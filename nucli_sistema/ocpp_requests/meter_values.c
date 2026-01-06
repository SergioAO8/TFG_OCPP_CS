/*
 *  FILE
 *      meter_values.c - Mòdul amb la funció per gestionar la petició MeterValues
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul amb la funció per gestionar la petició MeterValues.
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
#include <syslog.h>
#include <sqlite3.h>
#include "meter_values.h"
#include "MeterValuesReqJSON.h"
#include "MeterValuesConfJSON.h"
#include "ocpp_cs.h"
#include "ws_server.h"
#include "error_messages.h"
#include "utils.h"

/*
 *  NAME
 *      proc_meter_values - gestiona la petició de MeterValues
 *  SYNOPSIS
 *      void proc_meter_values(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Controla els possibles errors en la petició i envia el respectiu error en cas que hi hagi.
 *      Si no hi ha errors, es gestiona la petició (es modifiquen les variables globals del
 *      sistema de control que calguin), i s'envia la resposta.
 *  RETURN VALUE
 *      Res.
 */
void proc_meter_values(struct header_st *header, char *payload, ChargerVars *vars)
{
    // Passo el string a struct JSON
    struct MeterValuesReq *meter_values_req = cJSON_ParseMeterValuesReq(payload);

    // Comprovo errors abans d'enviar la resposta
    if (meter_values_req == NULL) { // Error: FormationViolation
        send_formation_violation(header->unique_id, vars->client);
        return;
    }
    else if (meter_values_req->connector_id == -1 ||
             list_get_count(meter_values_req->meter_value) == 0) { // Error: ProtocolError

        send_protocol_error(header->unique_id, vars->client);
        return;
    }

    if ((meter_values_req->connector_id && meter_values_req->connector_id < 0) ||
        (meter_values_req->transaction_id && *meter_values_req->transaction_id < 0)) { // Error: TypeConstraintViolation

        send_type_constraint_violation(header->unique_id, vars->client);
        return;
    }

    // guardo les variables que he de posar a la base de dades
    int64_t connector = meter_values_req->connector_id;
    int64_t transaccio = 0;
    if (meter_values_req->transaction_id)
        transaccio = *meter_values_req->transaction_id;

    if (meter_values_req->meter_value && list_get_count(meter_values_req->meter_value)) {
        size_t len = list_get_count(meter_values_req->meter_value);
        for (int i = 0; i < len; i++) { // analitzo cada meter_value
            struct MeterValue *meter_value = list_get_head(meter_values_req->meter_value);
            list_remove_head(meter_values_req->meter_value);

            if (meter_value->timestamp == NULL ||
                strcmp(meter_value->timestamp, "") == 0 ||
                list_get_count(meter_value->sampled_value) == 0) { // Error: ProtocolError

                send_protocol_error(header->unique_id, vars->client);
                return;
            }

            if ((meter_value->timestamp && strcmp(meter_value->timestamp, "err") == 0)) { // Error: TypeConstraintViolation
                send_type_constraint_violation(header->unique_id, vars->client);
                return;
            }

            struct tm timestamp_st;
            memset(&timestamp_st, 0, sizeof(timestamp_st));
            if (ocpp_strptime(meter_value->timestamp, "%Y-%m-%dT%H:%M:%S%z", &timestamp_st, 19) == NULL) { // timestamp mal format -> Error: PropertyConstraintViolation
                send_property_constraint_violation(header->unique_id, vars->client);
                return;
            }

            char *hora = meter_value->timestamp; // variable que he de posar a la base de dades

            if (meter_value->sampled_value && list_get_count(meter_value->sampled_value)) {
                size_t count = list_get_count(meter_value->sampled_value);
                for (int n = 0; n < count; n++) { // analitzo cada sampled_value
                    struct SampledValue *sampled_value = list_get_head(meter_value->sampled_value);
                    list_remove_head(meter_value->sampled_value);

                    if (sampled_value->value == NULL || strcmp(sampled_value->value, "") == 0) { // Error: ProtocolError
                        send_protocol_error(header->unique_id, vars->client);
                        return;
                    }

                    if ((sampled_value->value && strcmp(sampled_value->value, "err") == 0) ||
                        (sampled_value->context && *sampled_value->context == -2) ||
                        (sampled_value->format && *sampled_value->format == -2) ||
                        (sampled_value->measurand && *sampled_value->measurand == -2) ||
                        (sampled_value->phase && *sampled_value->phase == -2) ||
                        (sampled_value->location && *sampled_value->location == -2) ||
                        (sampled_value->unit && *sampled_value->unit == -2)) { // Error: TypeConstraintViolation

                        send_type_constraint_violation(header->unique_id, vars->client);
                        return;
                    }
                    else if ((sampled_value->context && *sampled_value->context == -1) ||
                             (sampled_value->format && *sampled_value->format == -1) ||
                             (sampled_value->measurand && *sampled_value->measurand == -1) ||
                             (sampled_value->phase && *sampled_value->phase == -1) ||
                             (sampled_value->location && *sampled_value->location == -1) ||
                             (sampled_value->unit && *sampled_value->unit == -1)) { // Error: PropertyConstraintViolation

                        send_property_constraint_violation(header->unique_id, vars->client);
                        return;
                    }
                    // guardo les variables que he de posar a la base de dades
                    char *valor = sampled_value->value;
                    char unit[16];
                    char measurand[32];
                    char context[32];
                    if (sampled_value->unit) {
                        switch (*sampled_value->unit) {
                            case 0:
                                snprintf(unit, sizeof(unit), "%s", "A");
                                break;
                            case 1:
                                snprintf(unit, sizeof(unit), "%s", "Celcius");
                                break;
                            case 2:
                                snprintf(unit, sizeof(unit), "%s", "Celsius");
                                break;
                            case 3:
                                snprintf(unit, sizeof(unit), "%s", "Fahrenheit");
                                break;
                            case 4:
                                snprintf(unit, sizeof(unit), "%s", "K");
                                break;
                            case 5:
                                snprintf(unit, sizeof(unit), "%s", "kvar");
                                break;
                            case 6:
                                snprintf(unit, sizeof(unit), "%s", "kvarh");
                                break;
                            case 7:
                                snprintf(unit, sizeof(unit), "%s", "kVa");
                                break;
                            case 8:
                                snprintf(unit, sizeof(unit), "%s", "kW");
                                break;
                            case 9:
                                snprintf(unit, sizeof(unit), "%s", "kwH");
                                break;
                            case 10:
                                snprintf(unit, sizeof(unit), "%s", "Percent");
                                break;
                            case 11:
                                snprintf(unit, sizeof(unit), "%s", "V");
                                break;
                            case 12:
                                snprintf(unit, sizeof(unit), "%s", "VA");
                                break;
                            case 13:
                                snprintf(unit, sizeof(unit), "%s", "var");
                                break;
                            case 14:
                                snprintf(unit, sizeof(unit), "%s", "varh");
                                break;
                            case 15:
                                snprintf(unit, sizeof(unit), "%s", "W");
                                break;
                            case 16:
                                snprintf(unit, sizeof(unit), "%s", "Wh");
                                break;
                            default:
                                snprintf(unit, sizeof(unit), "%s", "");
                                break;
                        }
                    }

                    if (sampled_value->measurand) {
                        switch (*sampled_value->measurand) {
                            case 0:
                                snprintf(measurand, sizeof(measurand), "%s", "Current.Export");
                                break;
                            case 1:
                                snprintf(measurand, sizeof(measurand), "%s", "Current.Import");
                                break;
                            case 2:
                                snprintf(measurand, sizeof(measurand), "%s", "Current.Offered");
                                break;
                            case 3:
                                snprintf(measurand, sizeof(measurand), "%s", "Energy.Active.Export.Interval");
                                break;
                            case 4:
                                snprintf(measurand, sizeof(measurand), "%s", "Energy.Active.Export.Register");
                                break;
                            case 5:
                                snprintf(measurand, sizeof(measurand), "%s", "Energy.Active.Import.Interval");
                                break;
                            case 6:
                                snprintf(measurand, sizeof(measurand), "%s", "Energy.Active.Import.Register");
                                break;
                            case 7:
                                snprintf(measurand, sizeof(measurand), "%s", "Energy.Reactive.Export.Interval");
                                break;
                            case 8:
                                snprintf(measurand, sizeof(measurand), "%s", "Energy.Reactive.Export.Register");
                                break;
                            case 9:
                                snprintf(measurand, sizeof(measurand), "%s", "Energy.Reactive.Import.Interval");
                                break;
                            case 10:
                                snprintf(measurand, sizeof(measurand), "%s", "Energy.Reactive.Import.Register");
                                break;
                            case 11:
                                snprintf(measurand, sizeof(measurand), "%s", "Frequency");
                                break;
                            case 12:
                                snprintf(measurand, sizeof(measurand), "%s", "Power.Active.Export");
                                break;
                            case 13:
                                snprintf(measurand, sizeof(measurand), "%s", "Power.Active.Import");
                                break;
                            case 14:
                                snprintf(measurand, sizeof(measurand), "%s", "Power.Factor");
                                break;
                            case 15:
                                snprintf(measurand, sizeof(measurand), "%s", "Power.Offered");
                                break;
                            case 16:
                                snprintf(measurand, sizeof(measurand), "%s", "Power.Reactive.Export");
                                break;
                            case 17:
                                snprintf(measurand, sizeof(measurand), "%s", "Power.Reactive.Import");
                                break;
                            case 18:
                                snprintf(measurand, sizeof(measurand), "%s", "RPM");
                                break;
                            case 19:
                                snprintf(measurand, sizeof(measurand), "%s", "SoC");
                                break;
                            case 20:
                                snprintf(measurand, sizeof(measurand), "%s", "Temperature");
                                break;
                            case 21:
                                snprintf(measurand, sizeof(measurand), "%s", "Voltage");
                                break;
                            default:
                                snprintf(measurand, sizeof(measurand), "%s", "");
                                break;
                        }
                    }

                    if (sampled_value->context) {
                        switch (*sampled_value->context) {
                            case 0:
                                snprintf(context, sizeof(context), "%s", "Interruption.Begin");
                                break;
                            case 1:
                                snprintf(context, sizeof(context), "%s", "Interruption.End");
                                break;
                            case 2:
                                snprintf(context, sizeof(context), "%s", "Other");
                                break;
                            case 3:
                                snprintf(context, sizeof(context), "%s", "Sample.Clock");
                                break;
                            case 4:
                                snprintf(context, sizeof(context), "%s", "Sample.Periodic");
                                break;
                            case 5:
                                snprintf(context, sizeof(context), "%s", "Transaction.Begin");
                                break;
                            case 6:
                                snprintf(context, sizeof(context), "%s", "Transaction.End");
                                break;
                            case 7:
                                snprintf(context, sizeof(context), "%s", "Trigger");
                                break;
                            default:
                                snprintf(context, sizeof(context), "%s", "");
                                break;
                        }
                    }

                    // guardo la informacó a la base de dades
                    sqlite3 *db;
                    int rc;
                    char *errmsg;

                    rc = sqlite3_open(DATABASE_PATH, &db);
                    if (rc != SQLITE_OK) {
                        syslog(LOG_ERR, "%s: ERROR opening SQLite DB in memory: %s\n", __func__, sqlite3_errmsg(db));
                    }

                    char query[500];
                    snprintf(query, sizeof(query), "INSERT INTO meter_values(charger_id, connector, transaccio, hora, "
                        "valor, unit, measurand, context) VALUES(%d, %ld, %ld, '%s', '%s', '%s', "
                        "'%s', '%s');", vars->charger_id, connector, transaccio, hora, valor, unit, measurand, context);
                    rc = sqlite3_exec(db, query, 0, 0, &errmsg);
                    if (rc != SQLITE_OK) {
                        sqlite3_free(errmsg);
                    } else {
                        syslog(LOG_ERR, "%s: SQL statement executed successfully", __func__);
                    }

                    sqlite3_close(db);  // tanca la base de dades correctament
                }
            }
            else { // Error: ProtocolError
                send_protocol_error(header->unique_id, vars->client);
                return;
            }
        }
    }
    else { // Error: ProtocolError
        send_protocol_error(header->unique_id, vars->client);
        return;
    }
    // No errors -> CALLRESULT

    // Formo el missatge
    char message[256];
    snprintf(message, sizeof(message), "[3,%s,{}]", header->unique_id);

    // Envio el missatge al carregador
    ws_send("CALL RESULT", message, vars->client);

    // Allibero la memòria
    free(meter_values_req);
}
