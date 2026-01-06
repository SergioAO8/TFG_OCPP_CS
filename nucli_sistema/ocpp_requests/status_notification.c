/*
 *  FILE
 *      status_notification.c - Mòdul amb la funció per gestionar la petició StatusNotification
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul amb la funció per gestionar la petició StatusNotification.
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
#include "status_notification.h"
#include "StatusNotificationReqJSON.h"
#include "StatusNotificationConfJSON.h"
#include "ocpp_cs.h"
#include "ws_server.h"
#include "error_messages.h"
#include "utils.h"

/*
 *  NAME
 *      proc_status_notification - gestiona la petició de StatusNotification
 *  SYNOPSIS
 *      void proc_status_notification(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Controla els possibles errors en la petició i envia el respectiu error en cas que hi hagi.
 *      Si no hi ha errors, es gestiona la petició (es modifiquen les variables globals del
 *      sistema de control que calguin), i s'envia la resposta.
 *  RETURN VALUE
 *      Res.
 */
void proc_status_notification(struct header_st *header, char *payload, ChargerVars *vars)
{
    // Passo el string a struct JSON
    struct StatusNotificationReq *status_req = cJSON_ParseStatusNotificationReq(payload);

    // Comprovo errors abans d'enviar la resposta
    if (status_req == NULL) { // Error: FormationViolation
        send_formation_violation(header->unique_id, vars->client);
    }
    else if (status_req->connector_id == -1 || status_req->error_code == -1 || status_req->status == -1) { // Error: ProtocolError
        send_protocol_error(header->unique_id, vars->client);
    }
    else if ((status_req->connector_id && status_req->connector_id < 0) ||
             (status_req->error_code && status_req->error_code == -2) ||
             (status_req->status && status_req->status == -2) ||
             (status_req->vendor_error_code && strcmp(status_req->vendor_error_code, "err") == 0) ||
             (status_req->info && strcmp(status_req->info, "err") == 0) ||
             (status_req->timestamp && strcmp(status_req->timestamp, "err") == 0) ||
             (status_req->vendor_id && strcmp(status_req->vendor_id, "err") == 0)) { // Error: TypeConstraintViolation

        send_type_constraint_violation(header->unique_id, vars->client);
    }
    else if (status_req->connector_id > NUM_CONNECTORS ||
            (status_req->vendor_error_code && strcmp(status_req->vendor_error_code, "") == 0) ||
            (status_req->info && strcmp(status_req->info, "") == 0) ||
            (status_req->timestamp && strcmp(status_req->timestamp, "") == 0) ||
            (status_req->vendor_id && strcmp(status_req->vendor_id, "") == 0)) { // Error: PropertyConstraintViolation

        send_property_constraint_violation(header->unique_id, vars->client);
    }
    else if ((status_req->info && strlen(status_req->info) > 50) ||
             (status_req->vendor_id && strlen(status_req->vendor_id) > 255) ||
             (status_req->vendor_error_code && strlen(status_req->vendor_error_code) > 50)) { // Error: OccurrenceConstraintViolation

        send_occurrence_constraint_violation(header->unique_id, vars->client);
    }
    else { // No errors
        vars->connectors_status[status_req->connector_id] = status_req->status;

        // miro el error code per guardar-lo a la base de dades
        char error[32];
        switch (status_req->error_code) {
            case 0:
                snprintf(error, sizeof(error), "%s", "ConnectorLockFailure");
                break;
            case 1:
                snprintf(error, sizeof(error), "%s", "EVCommunicationError");
                break;
            case 2:
                snprintf(error, sizeof(error), "%s", "GroundFailure");
                break;
            case 3:
                snprintf(error, sizeof(error), "%s", "HighTemperature");
                break;
            case 4:
                snprintf(error, sizeof(error), "%s", "InternalError");
                break;
            case 5:
                snprintf(error, sizeof(error), "%s", "LocalListConflict");
                break;
            case 6:
                snprintf(error, sizeof(error), "%s", "NoError");
                break;
            case 7:
                snprintf(error, sizeof(error), "%s", "OtherError");
                break;
            case 8:
                snprintf(error, sizeof(error), "%s", "OverCurrentFailure");
                break;
            case 9:
                snprintf(error, sizeof(error), "%s", "OverVoltage");
                break;
            case 10:
                snprintf(error, sizeof(error), "%s", "PowerMeterFailure");
                break;
            case 11:
                snprintf(error, sizeof(error), "%s", "PowerSwitchFailure");
                break;
            case 12:
                snprintf(error, sizeof(error), "%s", "ReaderFailure");
                break;
            case 13:
                snprintf(error, sizeof(error), "%s", "ResetFailure");
                break;
            case 14:
                snprintf(error, sizeof(error), "%s", "UnderVoltage");
                break;
            case 15:
                snprintf(error, sizeof(error), "%s", "WeakSignal");
                break;
            default:
                snprintf(error, sizeof(error), "%s", "");
                break;
        }

        // miro l'estat per guardar-lo a la base de dades
        char estat[32];
        switch (status_req->status) {
            case 0:
                snprintf(estat, sizeof(estat), "%s", "Available");
                break;
            case 1:
                snprintf(estat, sizeof(estat), "%s", "Charging");
                break;
            case 2:
                snprintf(estat, sizeof(estat), "%s", "Faulted");
                break;
            case 3:
                snprintf(estat, sizeof(estat), "%s", "Finishing");
                break;
            case 4:
                snprintf(estat, sizeof(estat), "%s", "Preparing");
                break;
            case 5:
                snprintf(estat, sizeof(estat), "%s", "Reserved");
                break;
            case 6:
                snprintf(estat, sizeof(estat), "%s", "SuspendedEV");
                break;
            case 7:
                snprintf(estat, sizeof(estat), "%s", "SuspendedEVSE");
                break;
            case 8:
                snprintf(estat, sizeof(estat), "%s", "Unavailable");
                break;
            default:
                snprintf(estat, sizeof(estat), "%s", "");
                break;
        }

        // guardo l'hora actual per posar-la a la base de dades
        time_t t = time(NULL);
        struct tm *currentTime = localtime(&t);
        char *hora = malloc(64);
        snprintf(hora, 64, "\%04d-%02d-%02dT%02d:%02d:%02dZ",
        currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
        currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);

        // guardo l'estat a la base de dades
        sqlite3 *db;
        int rc;
        char *errmsg;

        rc = sqlite3_open(DATABASE_PATH, &db);
        if (rc != SQLITE_OK) {
            syslog(LOG_ERR, "%s: ERROR opening SQLite DB in memory: %s\n", __func__, sqlite3_errmsg(db));
        }

        char query[500];
        snprintf(query, sizeof(query), "INSERT INTO estats(charger_id, connector, estat, hora, error_code)"
            "VALUES(%d, %ld, '%s', '%s', '%s');", vars->charger_id, status_req->connector_id, estat, hora, error);
        rc = sqlite3_exec(db, query, 0, 0, &errmsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", errmsg);
            sqlite3_free(errmsg);
        } else {
            syslog(LOG_DEBUG, "%s: SQL statement executed successfully", __func__);
        }

        sqlite3_close(db);  // tanca la base de dades correctament

        if (status_req->status == STATUS_STATUS_AVAILABLE) {
            snprintf(vars->current_id_tags[status_req->connector_id], ID_TAG_LEN, "no_charging"); // actualitzo la current_id_tags
            vars->transaction_list[status_req->connector_id] = -1;
        }
        else if (status_req->status == STATUS_STATUS_CHARGING) {
            vars->transaction_list[status_req->connector_id] = vars->current_transaction_id; // Guardo el transactionId a la respectiva posicio
                                                                                             // del connector a transaction_list

            rc = sqlite3_open(DATABASE_PATH, &db);
            if (rc != SQLITE_OK) {
                syslog(LOG_ERR, "%s: ERROR opening SQLite DB in memory: %s\n", __func__, sqlite3_errmsg(db));
            }

            memset(query, 0, sizeof(query));
            snprintf(query, sizeof(query), "INSERT INTO transaccions(charger_id, estat, connector, hora, motiu)"
                "VALUES(%d, 'Start', %ld, '%s', '%s');", vars->charger_id, status_req->connector_id, hora, "");
            rc = sqlite3_exec(db, query, 0, 0, &errmsg);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", errmsg);
                sqlite3_free(errmsg);
            } else {
                syslog(LOG_DEBUG, "%s: SQL statement executed successfully", __func__);
            }

            sqlite3_close(db); // tanca la base de dades correctament
        }

        // Formo el missatge
        char message[256];
        snprintf(message, sizeof(message), "[3,%s,{}]", header->unique_id);

        // Envio el missatge al carregador
        ws_send("CALL RESULT", message, vars->client);
        free(hora);
    }

    // Formo el missatge per enviar a la web
    char information[1024];
    snprintf(information, sizeof(information), "{\"charger\": \"%d\", \"type\": \"statusNotification\", \"connector1\": %ld, \"connector2\": %ld, \"idTag1\": \"%s\", "
        "\"idTag2\": \"%s\", \"transactionId1\": %ld, \"transactionId2\": %ld}", vars->charger_id, vars->connectors_status[1],
        vars->connectors_status[2] , vars->current_id_tags[1], vars->current_id_tags[2],
        vars->transaction_list[1], vars->transaction_list[2]);

    // Envio el missatge a la web
    ws_send("WEB", information, vars->client);

    // Allibero la memòria
    free(status_req);
}
