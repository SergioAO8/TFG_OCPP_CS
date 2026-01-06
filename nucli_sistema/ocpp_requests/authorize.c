/*
 *  FILE
 *      authorize.c - Mòdul amb la funció per gestionar la petició Authorize
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul amb la funció per gestionar la petició Authorize.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include "authorize.h"
#include "AuthorizeReqJSON.h"
#include "AuthorizeConfJSON.h"
#include "ocpp_cs.h"
#include "ws_server.h"
#include "error_messages.h"
#include "utils.h"

/*
 *  NAME
 *      proc_authorize - gestiona la petició d'Authorize
 *  SYNOPSIS
 *      void proc_authorize(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Controla els possibles errors en la petició i envia el respectiu error en cas que hi hagi.
 *      Si no hi ha errors, es gestiona la petició (es modifiquen les variables globals del
 *      sistema de control que calguin), i s'envia la resposta.
 *  RETURN VALUE
 *      Res.
 */
void proc_authorize(struct header_st *header, char *payload, ChargerVars *vars)
{
    // Passo el string a struct JSON
    struct AuthorizeReq *auth_req_payload = cJSON_ParseAuthorizeReq(payload);

    // Comprovo errors abans d'enviar la resposta
    if (auth_req_payload == NULL) { // Error: FormationViolation
        send_formation_violation(header->unique_id, vars->client);
    }
    else if (strcmp(auth_req_payload->id_tag, "err") == 0) { // Error: TypeConstraintViolation
        send_type_constraint_violation(header->unique_id, vars->client);
    }
    else if (strcmp(auth_req_payload->id_tag, "") == 0) { // Error: ProtocolError
        send_protocol_error(header->unique_id, vars->client);
    }
    else if (auth_req_payload->id_tag && strlen(auth_req_payload->id_tag) > 20) { // Error: OccurrenceConstraintViolation
        send_occurrence_constraint_violation(header->unique_id, vars->client);
    }
    else { // No errors -> CALLRESULT
        struct AuthorizeConf auth_conf;
        struct IdTagInfo info;

        // Comprovo si l'idTag està a la auth_list per autoritzar o no
        if (check_id_tag(auth_req_payload->id_tag)) { // está a la llista -> ACCEPTED
            memset(vars->current_id_tag, 0, ID_TAG_LEN);
            snprintf(vars->current_id_tag, ID_TAG_LEN, "%s", auth_req_payload->id_tag); // Actualitzo el current idTag
            info.status = STATUS_ACCEPTED; // Afegexo l'idTagInfo
            syslog(LOG_DEBUG, "%s: Accepted", __func__);
        }
        else { // no està a la llista -> INVALID
            info.status = STATUS_INVALID; // Afegexo l'idTagInfo
            syslog(LOG_WARNING, "%s: Invalid", __func__);
        }

        info.expiry_date = NULL;
        info.parent_id_tag = NULL;
        auth_conf.id_tag_info = &info;

        // Formo el missatge
        char message[256];
        snprintf(message, sizeof(message), "[3,%s,%s]", header->unique_id,
                 remove_spaces(cJSON_PrintAuthorizeConf(&auth_conf)));

        // Envio el missatge al carregador
        ws_send("CALL RESULT", message, vars->client);
    }

    // Allibero la memòria
    free(auth_req_payload);
}
