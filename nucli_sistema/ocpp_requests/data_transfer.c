/*
 *  FILE
 *      data_transfer.c - Mòdul amb la funció per gestionar la petició DataTransfer
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul amb la funció per gestionar la petició DataTransfer.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "data_transfer.h"
#include "DataTransferReqJSON.h"
#include "DataTransferConfJSON.h"
#include "ocpp_cs.h"
#include "ws_server.h"
#include "error_messages.h"
#include "utils.h"

/*
 *  NAME
 *      proc_data_transfer - gestiona la petició de DataTransfer
 *  SYNOPSIS
 *      void proc_data_transfer(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Controla els possibles errors en la petició i envia el respectiu error en cas que hi hagi.
 *      Si no hi ha errors, es gestiona la petició (es modifiquen les variables globals del
 *      sistema de control que calguin), i s'envia la resposta.
 *  RETURN VALUE
 *      Res.
 */
void proc_data_transfer(struct header_st *header, char *payload, ChargerVars *vars)
{
    // Passo el string a struct JSON
    struct DataTransferReq *data_payload = cJSON_ParseDataTransferReq(payload);

    // Comprovo errors abans d'enviar la resposta
    if (data_payload == NULL) { // Error: FormationViolation
        send_formation_violation(header->unique_id, vars->client);
    }
    else if (data_payload->vendor_id == NULL ||
             strcmp(data_payload->vendor_id, "") == 0) { // Error: ProtocolError

        send_protocol_error(header->unique_id, vars->client);
    }
    else if ((data_payload->vendor_id && strcmp(data_payload->vendor_id, "err") == 0) ||
             (data_payload->message_id && (strcmp(data_payload->message_id, "err") == 0)) ||
             (data_payload->data && strcmp(data_payload->data, "err") == 0)) { // Error: TypeConstraintViolation

        send_type_constraint_violation(header->unique_id, vars->client);
    }
    else if ((data_payload->message_id && strcmp(data_payload->message_id, "") == 0) ||
             (data_payload->data && strcmp(data_payload->data, "") == 0)) { // Error: PropertyConstraintViolation

        send_property_constraint_violation(header->unique_id, vars->client);
    }
    else if (strlen(data_payload->vendor_id) > 255 ||
            (data_payload->message_id && strlen(data_payload->message_id) > 50)) { // Error: OccurrenceConstraintViolation

        send_occurrence_constraint_violation(header->unique_id, vars->client);
    }
    else { // No errors -> CALLRESULT
        struct DataTransferConf data_conf;

        // Status a desconegut, degut a que no hi ha cap fabricant de punt de càrrega amb el qual establir el significat d'aquesta petició.
        data_conf.status = STATUS_DATA_TRANSFER_UNKNOWN_MESSAGE_ID;
        data_conf.data = NULL;

        // Formo el missatge
        char message[256];
        snprintf(message, sizeof(message), "[3,%s,%s]", header->unique_id,
            remove_spaces(cJSON_PrintDataTransferConf(&data_conf)));

        // Envio el missatge al carregador
        ws_send("CALL RESULT", message, vars->client);
    }

    // Allibero la memòria
    free(data_payload);
}
