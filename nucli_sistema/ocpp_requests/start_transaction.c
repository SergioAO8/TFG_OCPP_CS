/*
 *  FILE
 *      start_transaction.c - Mòdul amb la funció per gestionar la petició StartTransaction
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul amb la funció per gestionar la petició StartTransaction.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list.h>
#include <syslog.h>
#include "start_transaction.h"
#include "StartTransactionReqJSON.h"
#include "StartTransactionConfJSON.h"
#include "ocpp_cs.h"
#include "ws_server.h"
#include "error_messages.h"
#include "utils.h"

/*
 *  NAME
 *      proc_start_transaction - gestiona la petició de StartTransaction
 *  SYNOPSIS
 *      void proc_start_transaction(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Controla els possibles errors en la petició i envia el respectiu error en cas que hi hagi.
 *      Si no hi ha errors, es gestiona la petició (es modifiquen les variables globals del
 *      sistema de control que calguin), i s'envia la resposta.
 *  RETURN VALUE
 *      Res.
 */
void proc_start_transaction(struct header_st *header, char *payload, ChargerVars *vars)
{
    // Passo el string a struct JSON
    struct StartTransactionReq *start_transaction_req = cJSON_ParseStartTransactionReq(payload);

    struct tm timestamp_st;
    memset(&timestamp_st, 0, sizeof(timestamp_st));

    // Comprovo errors abans d'enviar la resposta
    if (start_transaction_req == NULL) { // Error: FormationViolation
        send_formation_violation(header->unique_id, vars->client);
    }
    else if (start_transaction_req->connector_id == -1 ||
             start_transaction_req->id_tag == NULL ||
             strcmp(start_transaction_req->id_tag, "") == 0 ||
             start_transaction_req->meter_start == -1 ||
             start_transaction_req->timestamp == NULL ||
             strcmp(start_transaction_req->timestamp, "") == 0) { // Error: ProtocolError

        send_protocol_error(header->unique_id, vars->client);
    }
    else if (start_transaction_req->connector_id < 0 ||
             start_transaction_req->meter_start < 0 ||
             (start_transaction_req->id_tag && strcmp(start_transaction_req->id_tag, "err") == 0) ||
             (start_transaction_req->reservation_id && *start_transaction_req->reservation_id < 0) ||
             (start_transaction_req->timestamp && strcmp(start_transaction_req->timestamp, "err") == 0)) { // Error: TypeConstraintViolation

        send_type_constraint_violation(header->unique_id, vars->client);
    }
    else if ((start_transaction_req->reservation_id && *start_transaction_req->reservation_id == -1) ||
              start_transaction_req->connector_id > NUM_CONNECTORS ||
              start_transaction_req->connector_id == 0 ||
              ocpp_strptime(start_transaction_req->timestamp, "%Y-%m-%dT%H:%M:%S%z", &timestamp_st, 19) == NULL) { // Error: PropertyConstraintViolation

        send_property_constraint_violation(header->unique_id, vars->client);
    }
    else if (strlen(start_transaction_req->id_tag) > 20) { // Error: OccurrenceConstraintViolation
        send_occurrence_constraint_violation(header->unique_id, vars->client);
    }
    else { // No errors -> CALLRESULT
        struct StartTransactionConf start_transaction_conf;
        struct IdTagInfo_Start info;

        // Comprovo si el idTag es el del authorize i si es troba a la auth_list
        if (check_id_tag(start_transaction_req->id_tag) &&
            (strcasecmp(start_transaction_req->id_tag, vars->current_id_tag)) == 0) { // idTag vàlid

            // comprovo si el connector ja està amb una transacció activa
            if (vars->transaction_list[start_transaction_req->connector_id] != -1 ||
                check_concurrent_tx_id_tag(start_transaction_req->id_tag, vars)) { // connector ja amb una transacció activa -> ConcurrentTx
                // Afegeixo l'idTagInfo
                info.status = STATUS_START_CONCURRENT_TX;
                info.expiry_date = NULL;
                info.parent_id_tag = NULL;
                start_transaction_conf.id_tag_info = &info;
                start_transaction_conf.transaction_id = ++vars->current_transaction_id;
                syslog(LOG_WARNING, "%s: concurrentTx", __func__);
            }
            else if (vars->connectors_status[0] == CONN_UNAVAILABLE ||
                vars->connectors_status[start_transaction_req->connector_id] == CONN_FAULTED ||
                vars->connectors_status[start_transaction_req->connector_id] == CONN_SUSPENDED_EV ||
                vars->connectors_status[start_transaction_req->connector_id] == CONN_SUSPENDED_EVSE ||
                vars->connectors_status[start_transaction_req->connector_id] == CONN_UNAVAILABLE) {

                // Afegeixo l'idTagInfo
                info.status = STATUS_START_INVALID;
                info.expiry_date = NULL;
                info.parent_id_tag = NULL;
                start_transaction_conf.id_tag_info = &info;
                start_transaction_conf.transaction_id = ++vars->current_transaction_id;
                syslog(LOG_WARNING, "%s: connector no disponible", __func__);
            }
            else { // connector vàlid per carregar -> Accepted
                // Afegeixo l'idTagInfo
                info.status = STATUS_START_ACCEPTED;
                info.expiry_date = NULL;
                info.parent_id_tag = NULL;
                start_transaction_conf.id_tag_info = &info;
                start_transaction_conf.transaction_id = ++vars->current_transaction_id;
                snprintf(vars->current_id_tags[start_transaction_req->connector_id], ID_TAG_LEN, "%s", start_transaction_req->id_tag); // Guardo el idTag a la respectiva posicio
                                                                                                  // del connector a current_id_tags per quan es pari la transacció
                syslog(LOG_DEBUG, "%s: Accepted", __func__);
            }
        }
        else { // idTag no reconegut -> Invalid
            // Afegexo l'idTagInfo
            info.status = STATUS_START_INVALID;
            info.expiry_date = NULL;
            info.parent_id_tag = NULL;
            start_transaction_conf.id_tag_info = &info;
            start_transaction_conf.transaction_id = ++vars->current_transaction_id;
            syslog(LOG_WARNING, "%s: idTag no vàlid", __func__);
        }

        // Formo el missatge
        char message[256];
        snprintf(message, sizeof(message), "[3,%s,%s]", header->unique_id,
            remove_spaces(cJSON_PrintStartTransactionConf(&start_transaction_conf)));

        // Envio el missatge al carregador
        ws_send("CALL RESULT", message, vars->client);
    }

    // Formo el missatge per enviar a la web
    char information[1024];
    snprintf(information, sizeof(information), "{\"charger\": \"%d\", \"type\": \"startTransaction\", \"connector1\": %ld, \"connector2\": %ld, \"idTag1\": \"%s\", "
        "\"idTag2\": \"%s\", \"transactionId1\": %ld, \"transactionId2\": %ld}", vars->charger_id, vars->connectors_status[1],
        vars->connectors_status[2] , vars->current_id_tags[1], vars->current_id_tags[2],
        vars->transaction_list[1], vars->transaction_list[2]);

    // Envio el missatge a la web
    ws_send("WEB", information, vars->client);

    // Allibero la memòria
    free(start_transaction_req);
}
