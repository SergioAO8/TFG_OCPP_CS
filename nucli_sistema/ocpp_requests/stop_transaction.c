/*
 *  FILE
 *      stop_transaction.c - Mòdul amb la funció per gestionar la petició StopTransaction
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul amb la funció per gestionar la petició StopTransaction.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list.h>
#include <time.h>
#include <sqlite3.h>
#include <syslog.h>
#include "stop_transaction.h"
#include "StopTransactionReqJSON.h"
#include "StopTransactionConfJSON.h"
#include "ocpp_cs.h"
#include "ws_server.h"
#include "error_messages.h"
#include "utils.h"

/*
 *  NAME
 *      proc_stop_transaction - gestiona la petició de StopTransaction
 *  SYNOPSIS
 *      void proc_stop_transaction(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Controla els possibles errors en la petició i envia el respectiu error en cas que hi hagi.
 *      Si no hi ha errors, es gestiona la petició (es modifiquen les variables globals del
 *      sistema de control que calguin), i s'envia la resposta.
 *  RETURN VALUE
 *      Res.
 */
void proc_stop_transaction(struct header_st *header, char *payload, ChargerVars *vars)
{
    // Passo el string a struct JSON
    struct StopTransactionReq *stop_transaction_req = cJSON_ParseStopTransactionReq(payload);

    // Comprovo errors abans d'enviar la resposta
    if (stop_transaction_req == NULL) { // Error: FormationViolation
        send_formation_violation(header->unique_id, vars->client);
        return;
    }
    else if (stop_transaction_req->meter_stop == -1 ||
             stop_transaction_req->timestamp == NULL ||
             strcmp(stop_transaction_req->timestamp, "") == 0 ||
             stop_transaction_req->transaction_id == -1) { // Error: ProtocolError

        send_protocol_error(header->unique_id, vars->client);
        return;
    }

    struct tm timestamp_st;
    memset(&timestamp_st, 0, sizeof(timestamp_st));
    if (ocpp_strptime(stop_transaction_req->timestamp, "%Y-%m-%dT%H:%M:%S%z", &timestamp_st, 19) == NULL) { // timestamp mal format -> Error: PropertyConstraintViolation
        send_property_constraint_violation(header->unique_id, vars->client);
        return;
    }

    if ((stop_transaction_req->id_tag && strcmp(stop_transaction_req->id_tag, "err") == 0) ||
        (stop_transaction_req->meter_stop && stop_transaction_req->meter_stop < 0) ||
        (stop_transaction_req->timestamp && strcmp(stop_transaction_req->timestamp, "err") == 0) ||
        (stop_transaction_req->transaction_id && stop_transaction_req->transaction_id < 0) ||
        (stop_transaction_req->reason && *stop_transaction_req->reason == -2)) { // Error: TypeConstraintViolation

        send_type_constraint_violation(header->unique_id, vars->client);
        return;
    }
    else if ((stop_transaction_req->id_tag && strcmp(stop_transaction_req->id_tag, "") == 0) ||
             (stop_transaction_req->reason && *stop_transaction_req->reason == -1)) { // Error: PropertyConstraintViolation

        send_property_constraint_violation(header->unique_id, vars->client);
        return;
    }
    else if (stop_transaction_req->id_tag && strlen(stop_transaction_req->id_tag) > 20) { // Error: OccurrenceConstraintViolation
        send_occurrence_constraint_violation(header->unique_id, vars->client);
        return;
    }

    if (stop_transaction_req->transaction_data && list_get_count(stop_transaction_req->transaction_data)) {
        size_t count = list_get_count(stop_transaction_req->transaction_data);
        for (int i = 0; i < count; i++) { // analitzo cada transaction_data
            struct TransactionDatum *transaction_data = list_get_head(stop_transaction_req->transaction_data);

            if (transaction_data) {
                if (transaction_data->timestamp == NULL ||
                    strcmp(transaction_data->timestamp, "") == 0) { // Error: ProtocolError

                    send_protocol_error(header->unique_id, vars->client);
                    return;
                }

                if ((transaction_data->timestamp &&
                    strcmp(transaction_data->timestamp, "err") == 0)) { // Error: TypeConstraintViolation

                    send_type_constraint_violation(header->unique_id, vars->client);
                    return;
                }

                //struct tm timestamp_st;
                memset(&timestamp_st, 0, sizeof(timestamp_st));
                if (ocpp_strptime(transaction_data->timestamp, "%Y-%m-%dT%H:%M:%S%z", &timestamp_st, 19) == NULL) { // timestamp mal format -> Error: PropertyConstraintViolation
                    send_property_constraint_violation(header->unique_id, vars->client);
                    return;
                }

                if (list_get_count(transaction_data->sampled_value)) {
                    size_t count_2 = list_get_count(transaction_data->sampled_value);
                    for (int n = 0; n < count_2; n++) { // analitzo cada sampled_value
                        struct SampledValue_Stop *sampled_value_stop = list_get_head(transaction_data->sampled_value);
                        list_remove_head(transaction_data->sampled_value);

                        if ((sampled_value_stop && sampled_value_stop->value == NULL) ||
                            (sampled_value_stop && strcmp(sampled_value_stop->value, "") == 0)) { // Error: ProtocolError

                            send_protocol_error(header->unique_id, vars->client);
                            return;
                        }

                        if ((sampled_value_stop && sampled_value_stop->value && strcmp(sampled_value_stop->value, "err") == 0) ||
                            (sampled_value_stop && sampled_value_stop->context && *sampled_value_stop->context == -2) ||
                            (sampled_value_stop && sampled_value_stop->format && *sampled_value_stop->format == -2) ||
                            (sampled_value_stop && sampled_value_stop->measurand && *sampled_value_stop->measurand == -2) ||
                            (sampled_value_stop && sampled_value_stop->phase && *sampled_value_stop->phase == -2) ||
                            (sampled_value_stop && sampled_value_stop->location && *sampled_value_stop->location == -2) ||
                            (sampled_value_stop && sampled_value_stop->unit && *sampled_value_stop->unit == -2)) { // Error: TypeConstraintViolation

                            send_type_constraint_violation(header->unique_id, vars->client);
                            return;
                        }
                        else if ((sampled_value_stop && sampled_value_stop->context && *sampled_value_stop->context == -1) ||
                                 (sampled_value_stop && sampled_value_stop->format && *sampled_value_stop->format == -1) ||
                                 (sampled_value_stop && sampled_value_stop->measurand && *sampled_value_stop->measurand == -1) ||
                                 (sampled_value_stop && sampled_value_stop->phase && *sampled_value_stop->phase == -1) ||
                                 (sampled_value_stop && sampled_value_stop->location && *sampled_value_stop->location == -1) ||
                                 (sampled_value_stop && sampled_value_stop->unit && *sampled_value_stop->unit == -1)) { // Error: PropertyConstraintViolation

                            send_property_constraint_violation(header->unique_id, vars->client);
                            return;
                        }
                    }
                }
            }
        }
    }

    // No errors -> CALLRESULT

    struct StopTransactionConf stop_transaction_conf;
    struct IdTagInfo_Stop info;

    int connector = -1; // aqui posaré el connector d'aquesta transaccció

    // busco el connector d'aquesta transacció
    for (int i = 0; i <= NUM_CONNECTORS; i++) {
        if (stop_transaction_req->id_tag && (strcasecmp(stop_transaction_req->id_tag, vars->current_id_tags[i]) == 0))
            connector = i;
    }

    // busco el si el transactionId és correcte
    for (int i = 0; i <= NUM_CONNECTORS; i++) {
        if (stop_transaction_req->transaction_id == vars->transaction_list[i])
            connector = i;
    }

    if (connector < 0) // el transactionId no és correcte
        syslog(LOG_DEBUG, "%s: transationId no existent", __func__);

    // Comprovo si hi ha idTag
    if (stop_transaction_req->id_tag) { // hi ha idTag
        if (check_id_tag(stop_transaction_req->id_tag)) { // idTag a la auth list
            if (connector > 0 && (strcasecmp(stop_transaction_req->id_tag, vars->current_id_tags[connector]) == 0) &&
                (strcasecmp(stop_transaction_req->id_tag, vars->current_id_tag) == 0)) { // idTag vàlid
                info.status = STATUS_STOP_ACCEPTED;
                info.expiry_date = NULL;
                info.parent_id_tag = NULL;
                stop_transaction_conf.id_tag_info = &info;
                syslog(LOG_DEBUG, "%s: Accepted", __func__);
            }
            else { // l'idTag no és el mateix que el que s'ha fet servir per l'start -> no vàlid
                // Afegexo l'idTagInfo
                info.status = STATUS_STOP_INVALID;
                info.expiry_date = NULL;
                info.parent_id_tag = NULL;
                stop_transaction_conf.id_tag_info = &info;
                syslog(LOG_WARNING, "%s: Invalid: idTag diferent del auth", __func__);
            }
        }
        else { // idTag no està a la auth_list -> invàlid
            // Afegexo l'idTagInfo
            info.status = STATUS_STOP_INVALID;
            info.expiry_date = NULL;
            info.parent_id_tag = NULL;
            stop_transaction_conf.id_tag_info = &info;
            syslog(LOG_WARNING, "%s: Invalid: idTag no a la auth_list", __func__);
        }

        // Formo el missatge
        char message[256];
        snprintf(message, sizeof(message), "[3,%s,%s]", header->unique_id,
        remove_spaces(cJSON_PrintStopTransactionConf(&stop_transaction_conf)));

        // Envio el missatge al carregador
        ws_send("CALL RESULT", message, vars->client);
    }
    else { // no hi ha idTag
        syslog(LOG_DEBUG, "%s: Accepted", __func__);
        // Formo el missatge
        char message[256];
        snprintf(message, sizeof(message), "[3,%s,{}]", header->unique_id);

        // Envio el missatge al carregador
        ws_send("CALL RESULT", message, vars->client);
    }

    if (connector > 0) { // només en aquest cas guardo a la base de dades per evitar errors
        snprintf(vars->current_id_tags[connector], ID_TAG_LEN, "no_charging"); // actualitzo la current_id_tags
        vars->transaction_list[connector] = -1;

        char motiu[32];
        if (stop_transaction_req->reason) {
            switch (*stop_transaction_req->reason) {
                case 0:
                    snprintf(motiu, sizeof(motiu), "%s", "DeAuthorized");
                    break;
                case 1:
                    snprintf(motiu, sizeof(motiu), "%s", "EmergencyStop");
                    break;
                case 2:
                    snprintf(motiu, sizeof(motiu), "%s", "EVDisconnect");
                    break;
                case 3:
                    snprintf(motiu, sizeof(motiu), "%s", "HardReset");
                    break;
                case 4:
                    snprintf(motiu, sizeof(motiu), "%s", "Local");
                    break;
                case 5:
                    snprintf(motiu, sizeof(motiu), "%s", "Other");
                    break;
                case 6:
                    snprintf(motiu, sizeof(motiu), "%s", "PowerLoss");
                    break;
                case 7:
                    snprintf(motiu, sizeof(motiu), "%s", "Reboot");
                    break;
                case 8:
                    snprintf(motiu, sizeof(motiu), "%s", "Remote");
                    break;
                case 9:
                    snprintf(motiu, sizeof(motiu), "%s", "SoftReset");
                    break;
                case 10:
                    snprintf(motiu, sizeof(motiu), "%s", "UnlockCommand");
                    break;
                default:
                    snprintf(motiu, sizeof(motiu), "%s", "No especificat");
                    break;
            }
        }
        else
            snprintf(motiu, sizeof(motiu), "%s", "");

        // guardo l'hora actual per posar-la a la base de dades
        time_t t = time(NULL);
        struct tm *currentTime = localtime(&t);
        char *hora = malloc(64);
        snprintf(hora, 64, "\%04d-%02d-%02dT%02d:%02d:%02dZ",
        currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
        currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);

        // guardo la informacó a la base de dades
        sqlite3 *db;
        int rc;
        char *errmsg;

        rc = sqlite3_open(DATABASE_PATH, &db);
        if (rc != SQLITE_OK) {
            syslog(LOG_ERR, "%s: ERROR opening SQLite DB in memory: %s\n", __func__, sqlite3_errmsg(db));
        }

        char query[500];
        snprintf(query, sizeof(query), "INSERT INTO transaccions(charger_id, estat, connector, hora, motiu)"
            "VALUES(%d, 'Stop', '%d', '%s', '%s');", vars->charger_id, connector, hora, motiu);
        rc = sqlite3_exec(db, query, 0, 0, &errmsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", errmsg);
            sqlite3_free(errmsg);
        } else {
            syslog(LOG_DEBUG, "%s: SQL statement executed successfully", __func__);
        }

        sqlite3_close(db); // tanca la base de dades correctament
        free(hora);
    }

    // Esborro el transactionId de la transaction_list
    if (stop_transaction_req->transaction_id <= NUM_CONNECTORS)
        delete_transaction_id(stop_transaction_req->transaction_id, vars);

    // Formo el missatge per enviar a la web
    char information[1024];
    snprintf(information, sizeof(information), "{\"charger\": \"%d\", \"type\": \"stopTransaction\", \"connector1\": %ld, \"connector2\": %ld, \"idTag1\": \"%s\", "
        "\"idTag2\": \"%s\", \"transactionId1\": %ld, \"transactionId2\": %ld}", vars->charger_id, vars->connectors_status[1],
        vars->connectors_status[2] , vars->current_id_tags[1], vars->current_id_tags[2],
        vars->transaction_list[1], vars->transaction_list[2]);

    // Envio el missatge a la web
    ws_send("WEB", information, vars->client);

    // Allibero la memòria
    free(stop_transaction_req);
}
