/*
 *  FILE
 *      boot_notification.c - Mòdul amb la funció per gestionar la petició BootNotification
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul amb la funció per gestionar la petició BootNotification.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "boot_notification.h"
#include "BootNotificationReqJSON.h"
#include "BootNotificationConfJSON.h"
#include "ocpp_cs.h"
#include "ws_server.h"
#include "error_messages.h"
#include "utils.h"

/*
 *  NAME
 *      proc_boot_notification - gestiona la petició de BootNotification
 *  SYNOPSIS
 *      void proc_boot_notification(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Controla els possibles errors en la petició i envia el respectiu error en cas que hi hagi.
 *      Si no hi ha errors, es gestiona la petició (es modifiquen les variables globals del
 *      sistema de control que calguin), i s'envia la resposta.
 *  RETURN VALUE
 *      Res.
 */
void proc_boot_notification(struct header_st *header, char *payload, ChargerVars *vars)
{
    // Passo el string a struct JSON
    struct BootNotificationReq *boot_req_payload = cJSON_ParseBootNotificationReq(payload);

    // Comprovo errors abans d'enviar la resposta
    if (boot_req_payload == NULL) { // Error: FormationViolation
        send_formation_violation(header->unique_id, vars->client);
    }
    else if (boot_req_payload->charge_point_vendor == NULL  || strcmp(boot_req_payload->charge_point_vendor, "") == 0 ||
        boot_req_payload->charge_point_model == NULL || strcmp(boot_req_payload->charge_point_model, "") == 0) { // Error: ProtocolError

        send_protocol_error(header->unique_id, vars->client);
    }
    else if ((boot_req_payload->charge_point_model && strcmp(boot_req_payload->charge_point_model, "err") == 0) ||
        (boot_req_payload->charge_point_vendor && strcmp(boot_req_payload->charge_point_vendor, "err") == 0) ||
        (boot_req_payload->charge_point_serial_number && strcmp(boot_req_payload->charge_point_serial_number, "err") == 0) ||
        (boot_req_payload->charge_box_serial_number && strcmp(boot_req_payload->charge_box_serial_number, "err") == 0) ||
        (boot_req_payload->firmware_version && strcmp(boot_req_payload->firmware_version, "err") == 0)||
        (boot_req_payload->iccid && strcmp(boot_req_payload->iccid, "err") == 0)||
        (boot_req_payload->imsi && strcmp(boot_req_payload->imsi, "err") == 0)||
        (boot_req_payload->meter_type && strcmp(boot_req_payload->meter_type, "err") == 0)||
        (boot_req_payload->meter_serial_number && strcmp(boot_req_payload->meter_serial_number, "err") == 0)) { // Error: TypeConstraintViolation

        send_type_constraint_violation(header->unique_id, vars->client);
    }
    else if ((boot_req_payload->charge_point_serial_number && strcmp(boot_req_payload->charge_point_serial_number, "") == 0) ||
        (boot_req_payload->charge_box_serial_number && strcmp(boot_req_payload->charge_box_serial_number, "") == 0) ||
        (boot_req_payload->firmware_version && strcmp(boot_req_payload->firmware_version, "") == 0) ||
        (boot_req_payload->iccid && strcmp(boot_req_payload->iccid, "") == 0) ||
        (boot_req_payload->imsi && strcmp(boot_req_payload->imsi, "") == 0) ||
        (boot_req_payload->meter_type && strcmp(boot_req_payload->meter_type, "") == 0) ||
        (boot_req_payload->meter_serial_number && strcmp(boot_req_payload->meter_serial_number, "") == 0)) { // Error: PropertyConstraintViolation

            send_property_constraint_violation(header->unique_id, vars->client);
    }
    else if ((boot_req_payload->charge_point_vendor && strlen(boot_req_payload->charge_point_vendor) > 20) ||
        (boot_req_payload->charge_point_model && strlen(boot_req_payload->charge_point_model) > 20) ||
        (boot_req_payload->charge_point_serial_number && strlen(boot_req_payload->charge_point_serial_number) > 20) ||
        (boot_req_payload->charge_box_serial_number && strlen(boot_req_payload->charge_box_serial_number) > 20) ||
        (boot_req_payload->firmware_version && strlen(boot_req_payload->firmware_version) > 20) ||
        (boot_req_payload->iccid && strlen(boot_req_payload->iccid) > 20) ||
        (boot_req_payload->imsi && strlen(boot_req_payload->imsi) > 20) ||
        (boot_req_payload->meter_type && strlen(boot_req_payload->meter_type) > 20) ||
        (boot_req_payload->meter_serial_number && strlen(boot_req_payload->meter_serial_number) > 20)) { // Error: OccurrenceConstraintViolation

        send_occurrence_constraint_violation(header->unique_id, vars->client);
    }
    else { // No errors -> CALLRESULT
        struct BootNotificationConf boot_conf;

        // Obtinc el current time
        time_t t = time(NULL);
        struct tm *currentTime = localtime(&t);
        boot_conf.current_time = malloc(64);
        snprintf(boot_conf.current_time, 64, "\%04d-%02d-%02dT%02d:%02d:%02dZ",
            currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
            currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);

        /* PART OPCIONAL: normalment no es comproven els chargePointModels i chargePointVendors, pero estan disponibles
           en el cas que el desenvolupador del sistema de control vulgui fer llistes blanques/negres de models
           i vendors autoritzats/no autoritzats. En aquets cas no es fa la comprovació, però s'ha deixat comentat
           per si l'usuari del sistema de control la vol implementar.
        // comprovo el chargePointModel i el ChargePointVendor per veure si el reconec
        if (check_cp_model(boot_req_payload->charge_point_model)
            && check_cp_vendor(boot_req_payload->charge_point_vendor)) { // reconeguts -> ACCEPTED

            // Interval de Hearbeat i Status
            boot_conf.interval = HEARTBEAT_INTERVAL;
            boot_conf.status = STATUS_BOOT_ACCEPTED;
            boot.status = STATUS_BOOT_ACCEPTED; // actualitzo el status global
        }
        else { // no reconegut -> REJECTED (FALTA PENDING)
            // Interval de Hearbeat i Status
            boot_conf.interval = RESEND_BOOT_NOTIFICATION_INTERVAL;
            boot_conf.status = STATUS_BOOT_REJECTED;
        }
        */

        // Interval de Hearbeat i Status
        boot_conf.interval = HEARTBEAT_INTERVAL;
        boot_conf.status = STATUS_BOOT_ACCEPTED;
        vars->boot.status = STATUS_BOOT_ACCEPTED; // actualitzo el status global del carregador

        // Formo el missatge
        char message[256];
        snprintf(message, sizeof(message), "[3,%s,%s]", header->unique_id,
            remove_spaces(cJSON_PrintBootNotificationConf(&boot_conf)));

        // Envio el missatge al carregador
        ws_send("CALL RESULT", message, vars->client);

        snprintf(vars->current_vendor, 20, "%s", boot_req_payload->charge_point_vendor); // actualitzo el vendor del carregador
        snprintf(vars->current_model, 20, "%s", boot_req_payload->charge_point_model); // actualitzo el model del carregador

        // Allibero la memòria
        free(boot_conf.current_time);
    }

    // Formo el missatge per enviar a la web
    char information[1024];
    snprintf(information, sizeof(information), "{\"charger\": \"%d\", \"type\": \"bootNotification\", \"general\": %d, \"vendor\": \"%s\", "
        "\"model\": \"%s\"}", vars->charger_id, vars->boot.status, vars->current_vendor, vars->current_model);

    // Envio el missatge a la web
    ws_send("WEB", information, vars->client);

    // Allibero la memòria
    free(boot_req_payload);
}
