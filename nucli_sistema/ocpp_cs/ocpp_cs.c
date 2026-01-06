/*
 *  FILE
 *      ocpp_cs.c - mòdul principal del nucli del sistema de control
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul principal amb les utilitats bàsiques del sistema de control.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <list.h>
#include <ws.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include "ocpp_cs.h"
#include "ws_server.h"
#include "utils.h"
#include "error_messages.h"
#include "missatges_includes.h"
#include "lib_json_includes.h"

#define TIMEOUT_TIME 10 // temps de timeout per missatges sense resposta

// llista d'idTags, els quals es podran autoritzar
char *auth_list[] = {
    "12345",
    "D0431F35",
    "00FFFFFFFF",
    "idTag_Charger",
    "100"
};

// llista de chargePointModels
char *cp_models[] = {
    "MicroOcpp Simulator",
    "model2",
    "model3",
    "model4",
    "model5"
};

// llista de chargePointVendors
char *cp_vendors[] = {
    "MicroOcpp",
    "vendor2",
    "vendor3",
    "vendor4",
    "vendor5"
};

// Prototips de les funcions
static void proc_call(struct header_st *header, char *payload, ChargerVars *vars);
static void proc_call_result(const struct header_st *header, char *payload, ChargerVars *vars);

/*
 *  NAME
 *      init_system - Inicialitza el sistema
 *  SYNOPSIS
 *      void init_system(ChargerVars *vars);
 *  DESCRIPTION
 *      Inicialitza el sistema i les variables globals i no globals.
 *  RETURN VALUE
 *      Res.
 */
void init_system(ChargerVars *vars)
{
    vars->boot.status = STATUS_BOOT_REJECTED; /* fins que no arriba un BootNotification l'estat es REJECTED per no poder
                                            iniciar cap operació */

    vars->tx_state = ready_to_send; // es permet enviar peticions

    memset(&vars->conf_keys, 0, sizeof(vars->conf_keys)); // es netegen les claus

    // netejo el vendor i el model
    snprintf(vars->current_vendor, sizeof(vars->current_vendor), "%s", "");
    snprintf(vars->current_model, sizeof(vars->current_model), "%s", "");

    memset(vars->current_id_tag, 0, sizeof(vars->current_id_tag)); // inicialitzo el idTag per evitar errors

    memset(vars->transaction_list, -1, sizeof(vars->transaction_list)); // Inicializto la llista de transaccions a -1 inidicant que no n'hi ha cap

    for (int i = 0; i < (NUM_CONNECTORS + 1); i++) // Inicialitzo la llista dels idTags dels connectors a "no_charging", indicant que no estan carregant
        snprintf(vars->current_id_tags[i], ID_TAG_LEN, "%s", "no_charging");

    for (int i = 0; i < NUM_CONNECTORS + 1; i++)
        syslog(LOG_DEBUG, "idTag%d: %s", i, vars->current_id_tags[i]);

    for (int i = 0; i < (NUM_CONNECTORS + 1); i++) // Inicialitzo la llista dels status dels connectors a CONN_UNKNOWN, indicant que l'estat no és conegut
        vars->connectors_status[i] = CONN_UNKNOWN;

    for (int i = 0; i < NUM_CONNECTORS + 1; i++)
        syslog(LOG_DEBUG, "conn%d: %ld", i, vars->connectors_status[i]);
}

/*
 *  NAME
 *      system_on_receive - Gestiona els missatges rebuts
 *  SYNOPSIS
 *      void system_on_receive(char *req, ChargerVars *vars);
 *  DESCRIPTION
 *      Gestiona els missatges rebuts, filtrant per tipus de missatge
 *      (petició, resposta a petició enviada o missatge d'error).
 *      Depenent del tipus actua d'una manera o d'una altra.
 *  RETURN VALUE
 *      Res.
 */
void system_on_receive(char *req, ChargerVars *vars)
{
    // Formo el struct de recepció
    struct req_rx *request = malloc(sizeof(struct req_rx));
    request = split_message(request, req);

    // Formo el struct del header
    struct header_st *req_header = malloc(sizeof(struct header_st));
    req_header = split_header(request, req_header);

    // Comprovo el tipus de missatge
    switch (req_header->message_type_id) {
        case '2': // CALL
            proc_call(req_header, request->payload, vars); // es processa el missatge
            break;

        case '3': // CALLRESULT
            proc_call_result(req_header, request->payload, vars); // es processa el missatge
            break;

        case '4': // CALLERROR
            syslog(LOG_WARNING, "CALL ERROR RECEIVED");
            vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
            break;

        default: // NOT IMPLEMENTED
            // Envio el missatge al carregador
            ws_send("CALL ERROR", "[ERROR]: \"NotImplemented\",\"Requested Action is not known by receiver\"", vars->client);
    }

    // Allibero memòria
    free(req_header->action);
    free(req_header->unique_id);
    free(req_header);
    free(request->payload);
    free(request->header);
    free(request);
}

/*
 *  NAME
 *      proc_call - Gestiona les peticions rebudes.
 *  SYNOPSIS
 *      void proc_call(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Gestiona les peticions rebudes, filtrant pel tipus de petició
 *      (Authorize, BootNotification...). Depenent del tipus es delega
 *      la gestió del missatge a la funció corresponent al tipus de petició.
 *  RETURN VALUE
 *      Res.
 */
static void proc_call(struct header_st *header, char *payload, ChargerVars *vars)
{
    if (vars->boot.status == STATUS_BOOT_REJECTED && strcmp(header->action, "\"BootNotification\"") != 0) // carregador no incialitzat -> Error
        send_generic_error(header->unique_id, vars->client);
    else {
        if (strcmp(header->action, "\"Authorize\"") == 0) {
            proc_authorize(header, payload, vars);
        }
        else if (strcmp(header->action, "\"BootNotification\"") == 0) {
            proc_boot_notification(header, payload, vars);
        }
        else if (strcmp(header->action, "\"DataTransfer\"") == 0) {
            proc_data_transfer(header, payload, vars);
        }
        else if (strcmp(header->action, "\"Heartbeat\"") == 0) {
            proc_heartbeat(header, payload, vars);
        }
        else if (strcmp(header->action, "\"MeterValues\"") == 0) {
            proc_meter_values(header, payload, vars);
        }
        else if (strcmp(header->action, "\"StartTransaction\"") == 0) {
            proc_start_transaction(header, payload, vars);
        }
        else if (strcmp(header->action, "\"StopTransaction\"") == 0) {
            proc_stop_transaction(header, payload, vars);
        }
        else if (strcmp(header->action, "\"StatusNotification\"") == 0) {
            proc_status_notification(header, payload, vars);
        }
        else { // Not supported
            char message[256];
            snprintf(message, sizeof(message), "[4,%s,\"NotSupported\",\"Requested Action is recognized but not supported by the receiver\",{}]", header->unique_id);

            // Envio el missatge al carregador
            ws_send("CALL ERROR", message, vars->client);
        }
    }
}

/*
 *  NAME
 *      send_request - Gestiona l'enviament de peticions
 *  SYNOPSIS
 *      void send_request(int option, char *payload, ChargerVars *vars)
 *  DESCRIPTION
 *      Gestiona l'enviament de peticions, filtrant pel tipus de petició
 *      que s'ha d'enviar. Controla els errors dels missatges abans d'enviar-los,
 *      i en cas que no hi hagi envia la petició.
 *  RETURN VALUE
 *      Res.
 */
void send_request(int option, char *payload, ChargerVars *vars)
{
    time_t start = time(NULL); // aquí anirà l'hora a la que s'ha enviat la request
    char message[256];
    memset(message, 0, sizeof(message));
    switch (option) {
        case '1': // ChangeAvailability
            // Comprovo si el missatge que s'ha passat no està buit
            if (payload && (strlen(payload) > 1)) { // S'ha pogut llegir
                struct ChangeAvailabilityReq *request = cJSON_ParseChangeAvailabilityReq(payload); // Ho passo a struct per comprovar si els camps són correctes

                if (request == NULL || request->connector_id == -1 || request->type == -1) // Error sintàctic del missatge -> Error
                    syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");
                else { // Missatge escrit correctament -> Formo missatge complet i l'envio al carregador
                    snprintf(message, sizeof(message), "[2,\"%lu\",\"ChangeAvailability\",%s]", ++vars->current_unique_id, remove_spaces(payload));
                    ws_send("CALL", message, vars->client);
                    snprintf(vars->current_tx_request, sizeof(vars->current_tx_request), "\"ChangeAvailability\""); // actualitzo el tipus de missatge del qual espero la resposta
                    vars->tx_state = sent; // canvio l'estat a sent
                }
            }
            else // No s'ha pogut llegir -> Error
                syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");

            break;

        case '2': // ClearCache
            // En aquest cas no cal formar cap struct perquè el missatge és buit, es respon directament
            snprintf(message, sizeof(message), "[2,\"%lu\",\"ClearCache\",{}]", ++vars->current_unique_id);
            ws_send("CALL", message, vars->client);
            snprintf(vars->current_tx_request, sizeof(vars->current_tx_request), "\"ClearCache\""); // actualitzo el tipus de missatge del qual espero la resposta
            vars->tx_state = sent; // canvio l'estat a sent
            break;

        case '3': // DataTransfer
            // Comprovo si el missatge que s'ha passat no està buit
            if (payload && strlen(payload) > 1) { // S'ha pogut llegir
                struct DataTransferReq *request = cJSON_ParseDataTransferReq(payload); // Ho passo a struct per comprovar si els camps són correctes

                if (request == NULL || strcmp(request->vendor_id, "") == 0) { // Falta un camp obligatori -> Error
                    syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");
                }
                else { // Missatge escrit correctament -> Formo missatge complet i l'envio al carregador
                    snprintf(message, sizeof(message), "[2,\"%lu\",\"DataTransfer\",%s]", ++vars->current_unique_id, remove_spaces(payload));
                    ws_send("CALL", message, vars->client);
                    snprintf(vars->current_tx_request, sizeof(vars->current_tx_request), "\"DataTransfer\""); // actualitzo el tipus de missatge del qual espero la resposta
                    vars->tx_state = sent; // canvio l'estat a sent
                }
            }
            else // No s'ha pogut llegir -> Error
                syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");

            break;

        case '4': // GetConfiguration
            // Comprovo si el missatge que s'ha passat no està buit
            if (payload && strlen(payload) > 1) { // S'ha pogut llegir
                // Missatge escrit correctament -> Formo missatge complet i l'envio al carregador
                snprintf(message, sizeof(message), "[2,\"%lu\",\"GetConfiguration\",%s]", ++vars->current_unique_id, remove_spaces(payload));
                ws_send("CALL", message, vars->client);
                snprintf(vars->current_tx_request, sizeof(vars->current_tx_request), "\"GetConfiguration\""); // actualitzo el tipus de missatge del qual espero la resposta
                vars->tx_state = sent; // canvio l'estat a sent

            }
            else // No s'ha pogut llegir -> Error
                syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");

            break;

        case '5': // RemoteStartTransaction
            // Comprovo si el missatge que s'ha passat no està buit
            if (payload && strlen(payload) > 1) { // S'ha pogut llegir
                struct RemoteStartTransactionReq *request = cJSON_ParseRemoteStartTransactionReq(payload); // Ho passo a struct per comprovar si els camps són correctes

                if (request == NULL || strcmp(request->id_tag, "") == 0) { // Falta un camp obligatori -> Error
                    syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");
                }
                else { // Missatge escrit correctament -> Formo missatge complet i l'envio al carregador
                    snprintf(message, sizeof(message), "[2,\"%lu\",\"RemoteStartTransaction\",%s]", ++vars->current_unique_id, remove_spaces(payload));
                    ws_send("CALL", message, vars->client);
                    snprintf(vars->current_tx_request, sizeof(vars->current_tx_request), "\"RemoteStartTransaction\""); // actualitzo el tipus de missatge del qual espero la resposta
                    memset(vars->current_id_tag, 0, sizeof(vars->current_id_tag));
                    snprintf(vars->current_id_tag, sizeof(vars->current_id_tag), "%s", request->id_tag);
                    vars->tx_state = sent; // canvio l'estat a sent
                }
            }
            else // No s'ha pogut llegir -> Error
                syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");

            break;

        case '6': // RemoteStopTransaction
            // Comprovo si el missatge que s'ha passat no està buit
            if (payload && strlen(payload) > 1) { // S'ha pogut llegir
                struct RemoteStopTransactionReq *request = cJSON_ParseRemoteStopTransactionReq(payload); // Ho passo a struct per comprovar si els camps són correctes

                if (request == NULL || request->transaction_id == -1) { // Falta un camp obligatori -> Error
                    syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");
                }
                else { // Missatge escrit correctament -> Formo missatge complet i l'envio al carregador
                    snprintf(message, sizeof(message), "[2,\"%lu\",\"RemoteStopTransaction\",%s]", ++vars->current_unique_id, remove_spaces(payload));
                    ws_send("CALL", message, vars->client);
                    snprintf(vars->current_tx_request, sizeof(vars->current_tx_request), "\"RemoteStopTransaction\""); // actualitzo el tipus de missatge del qual espero la resposta
                    vars->tx_state = sent; // canvio l'estat a sent
                }
            }
            else // No s'ha pogut llegir -> Error
                syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");

            break;

        case '7': // Reset
            // Comprovo si el missatge que s'ha passat no està buit
            if (payload && strlen(payload) > 1) { // S'ha pogut llegir
                struct ResetReq *request = cJSON_ParseResetReq(payload); // Ho passo a struct per comprovar si els camps són correctes

                if (request == NULL || request->type == -1) { // Falta un camp obligatori -> Error
                    syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");
                }
                else { // Missatge escrit correctament -> Formo missatge complet i l'envio al carregador
                    snprintf(message, sizeof(message), "[2,\"%lu\",\"Reset\",%s]", ++vars->current_unique_id, remove_spaces(payload));
                    ws_send("CALL", message, vars->client);
                    snprintf(vars->current_tx_request, sizeof(vars->current_tx_request), "\"Reset\""); // actualitzo el tipus de missatge del qual espero la resposta
                    vars->tx_state = sent; // canvio l'estat a sent
                }
            }
            else // No s'ha pogut llegir -> Error
                syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");

            break;

        case '8': // UnlockConnector
            // Comprovo si el missatge que s'ha passat no està buit
            if (payload && strlen(payload) > 1) { // S'ha pogut llegir
                struct UnlockConnectorReq *request = cJSON_ParseUnlockConnectorReq(payload); // Ho passo a struct per comprovar si els camps són correctes

                if (request == NULL || request->connector_id == -1) { // Falta un camp obligatori -> Error
                    syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");
                }
                else { // Missatge escrit correctament -> Formo missatge complet i l'envio al carregador
                    snprintf(message, sizeof(message), "[2,\"%lu\",\"UnlockConnector\",%s]", ++vars->current_unique_id, remove_spaces(payload));
                    ws_send("CALL", message, vars->client);
                    snprintf(vars->current_tx_request, sizeof(vars->current_tx_request), "\"UnlockConnector\""); // actualitzo el tipus de missatge del qual espero la resposta
                    vars->tx_state = sent; // canvio l'estat a sent
                }
            }
            else // No s'ha pogut llegir -> Error
                syslog(LOG_WARNING, "Payload for Action is syntactically incorrect or not conform the PDU structure for Action");

            break;

        default:
            syslog(LOG_WARNING, "Invalid option");
    }

    // Timeout després d'enviar una petició
    struct timespec request = {0, 10000000}; // defineix un sleep de 10 ms per anar comprovant el temps de timeout
    // comprovo si ha passat el temps de timeout
    while (vars->tx_state == sent) { // si l'estat torna a ser ready_to_send, surt, sinó es posa a ready_to_send aquí després del temps de timeout
        time_t now = time(NULL); // temps actual
        if (difftime(now, start) >= TIMEOUT_TIME) { // ja ha passat el temps de timeout -> surto del bucle i deixo enviar una altra petició
            vars->tx_state = ready_to_send;
            syslog(LOG_WARNING, "Timeout");
        }
        nanosleep(&request, NULL); // deixo un temps de sleep per deixar treballar l'altre thread
    }
}

/*
 *  NAME
 *      proc_call_result - Gestiona la resposta de les peticions enviades.
 *  SYNOPSIS
 *      void proc_call_result(const struct header_st *header, char *payload, ChargerVars *vars)
 *  DESCRIPTION
 *      Gestiona la resposta de les peticions enviades, filtrant pel tipus de petició
 *      de la qual prové. Controla els errors dels missatges i actualitza les variables
 *      necessàries.
 *  RETURN VALUE
 *      Res.
 */
static void proc_call_result(const struct header_st *header, char *payload, ChargerVars *vars)
{
    char unique_id[32];
    snprintf(unique_id, sizeof(unique_id), "%s", header->unique_id); // faig una copia del uniqueId

    if (strtol(remove_quotes(unique_id), NULL, 10) != vars->current_unique_id) { // el uniqueId de la resposta no és el mateix que el de la petició -> Error
        syslog(LOG_WARNING, "The uniqueId of this response is not in accordance with the uniqueId of the request");
        vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
    }
    else { // el tipus de missatge i el uniqueId de la resposta corresponen amb el de la petició -> ara miro quin tipus de missatge és i el processo
        if (strcmp(vars->current_tx_request, "\"ChangeAvailability\"") == 0) {
            // Passo el string a struct JSON
            struct ChangeAvailabilityConf *change_availability_conf_payload = cJSON_ParseChangeAvailabilityConf(payload);

            // Comprovo errors abans d'enviar la resposta
            if (change_availability_conf_payload == NULL) { // Error: FormationViolation
                send_formation_violation(header->unique_id, vars->client);
            }
            else if (change_availability_conf_payload->status == -2) { // Error: TypeConstraintViolation
                send_type_constraint_violation(header->unique_id, vars->client);
            }
            else if (change_availability_conf_payload->status == -1) { // Error: ProtocolError
                send_protocol_error(header->unique_id, vars->client);
            }
            else { // No errors
                syslog(LOG_DEBUG, "ChangeAvailability: No errors");
                vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
            }
        }
        else if (strcmp(vars->current_tx_request, "\"ClearCache\"") == 0) {
            // Passo el string a struct JSON
            struct ClearCacheConf *clear_cache_conf_payload = cJSON_ParseClearCacheConf(payload);

            // Comprovo errors abans d'enviar la resposta
            if (clear_cache_conf_payload == NULL) { // Error: FormationViolation
                send_formation_violation(header->unique_id, vars->client);
            }
            else if (clear_cache_conf_payload->status == -1) { // Error: ProtocolError
                send_protocol_error(header->unique_id, vars->client);
            }
            else if (clear_cache_conf_payload->status == -2) { // Error: TypeConstraintViolation
                send_type_constraint_violation(header->unique_id, vars->client);
            }
            else { // No errors
                syslog(LOG_DEBUG, "ClearCache: No errors");
                vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
            }
        }
        else if (strcmp(vars->current_tx_request, "\"DataTransfer\"") == 0) {
            // Passo el string a struct JSON
            struct DataTransferConf *data_transfer_conf_payload = cJSON_ParseDataTransferConf(payload);

            // Comprovo errors abans d'enviar la resposta
            if (data_transfer_conf_payload == NULL) { // Error: FormationViolation
                send_formation_violation(header->unique_id, vars->client);
            }
            else if (data_transfer_conf_payload->status == -1) { // Error: ProtocolError
                send_protocol_error(header->unique_id, vars->client);
            }
            else if (data_transfer_conf_payload->status == -2 ||
                (data_transfer_conf_payload->data && strcmp(data_transfer_conf_payload->data, "err") == 0)) { // Error: TypeConstraintViolation

                send_type_constraint_violation(header->unique_id, vars->client);
            }
            else if ((data_transfer_conf_payload->data && strcmp(data_transfer_conf_payload->data, "") == 0)) {// Error: PropertyConstraintViolation
                send_property_constraint_violation(header->unique_id, vars->client);
            }
            else { // No errors
                syslog(LOG_DEBUG, "DataTransfer: No errors");
                vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
            }
        }
        else if (strcmp(vars->current_tx_request, "\"GetConfiguration\"") == 0) {
            // Passo el string a struct JSON
            struct GetConfigurationConf *get_configuration_conf_payload = cJSON_ParseGetConfigurationConf(payload);

            // Comprovo errors abans d'enviar la resposta
            if (get_configuration_conf_payload == NULL) { // Error: FormationViolation
                send_formation_violation(header->unique_id, vars->client);
                return;
            }

            if (get_configuration_conf_payload->configuration_key &&
                list_get_count(get_configuration_conf_payload->configuration_key)) {

                size_t len = list_get_count(get_configuration_conf_payload->configuration_key);
                for (int i = 0; i < len; i++) { // miro totes les configurationKeys que hi ha
                    struct ConfigurationKey *configuration_key = list_get_head(get_configuration_conf_payload->configuration_key);
                    list_remove_head(get_configuration_conf_payload->configuration_key);

                    if (configuration_key->key == NULL ||
                        strcmp(configuration_key->key, "") == 0) { // Error: ProtocolError

                        send_protocol_error(header->unique_id, vars->client);
                        return;
                    }
                    else if (configuration_key->key && strcmp(configuration_key->key, "err") == 0) { // Error: TypeConstraintViolation
                        send_type_constraint_violation(header->unique_id, vars->client);
                        return;
                    }
                    else if ((configuration_key->key && strlen(configuration_key->key) > 50) ||
                             (configuration_key->value && strlen(configuration_key->value) > 500)) { // Error: OccurrenceConstraintViolation

                        send_occurrence_constraint_violation(header->unique_id, vars->client);
                        return;
                    }
                    else { // No errors
                        if (strcmp(configuration_key->key, "AuthorizeRemoteTxRequests") == 0) {
                            free(vars->conf_keys.AuthorizeRemoteTxRequests);
                            vars->conf_keys.AuthorizeRemoteTxRequests = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.AuthorizeRemoteTxRequests, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "ClockAlignedDataInterval") == 0) {
                            free(vars->conf_keys.ClockAlignedDataInterval);
                            vars->conf_keys.ClockAlignedDataInterval = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.ClockAlignedDataInterval, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "ConnectionTimeOut") == 0) {
                            free(vars->conf_keys.ConnectionTimeOut);
                            vars->conf_keys.ConnectionTimeOut = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.ConnectionTimeOut, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "ConnectorPhaseRotation") == 0) {
                            free(vars->conf_keys.ConnectorPhaseRotation);
                            vars->conf_keys.ConnectorPhaseRotation = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.ConnectorPhaseRotation, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "GetConfigurationMaxKeys") == 0) {
                            free(vars->conf_keys.GetConfigurationMaxKeys);
                            vars->conf_keys.GetConfigurationMaxKeys = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.GetConfigurationMaxKeys, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "HeartbeatInterval") == 0) {
                            free(vars->conf_keys.HeartbeatInterval);
                            vars->conf_keys.HeartbeatInterval = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.HeartbeatInterval, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "LocalAuthorizeOffline") == 0) {
                            free(vars->conf_keys.LocalAuthorizeOffline);
                            vars->conf_keys.LocalAuthorizeOffline = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.LocalAuthorizeOffline, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "LocalPreAuthorize") == 0) {
                            free(vars->conf_keys.LocalPreAuthorize);
                            vars->conf_keys.LocalPreAuthorize = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.LocalPreAuthorize, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "MeterValuesAlignedData") == 0) {
                            free(vars->conf_keys.MeterValuesAlignedData);
                            vars->conf_keys.MeterValuesAlignedData = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.MeterValuesAlignedData, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "MeterValuesSampledData") == 0) {
                            free(vars->conf_keys.MeterValuesSampledData);
                            vars->conf_keys.MeterValuesSampledData = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.MeterValuesSampledData, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "MeterValueSampleInterval") == 0) {
                            free(vars->conf_keys.MeterValueSampleInterval);
                            vars->conf_keys.MeterValueSampleInterval = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.MeterValueSampleInterval, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "NumberOfConnectors") == 0) {
                            free(vars->conf_keys.NumberOfConnectors);
                            vars->conf_keys.NumberOfConnectors = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.NumberOfConnectors, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "ResetRetries") == 0) {
                            free(vars->conf_keys.ResetRetries);
                            vars->conf_keys.ResetRetries = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.ResetRetries, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "StopTransactionOnEVSideDisconnect") == 0) {
                            free(vars->conf_keys.StopTransactionOnEVSideDisconnect);
                            vars->conf_keys.StopTransactionOnEVSideDisconnect = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.StopTransactionOnEVSideDisconnect, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "StopTransactionOnInvalidId") == 0) {
                            free(vars->conf_keys.StopTransactionOnInvalidId);
                            vars->conf_keys.StopTransactionOnInvalidId = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.StopTransactionOnInvalidId, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "StopTxnAligneData") == 0) {
                            free(vars->conf_keys.StopTxnAligneData);
                            vars->conf_keys.StopTxnAligneData = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.StopTxnAligneData, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "StopTxnSampledData") == 0) {
                            free(vars->conf_keys.StopTxnSampledData);
                            vars->conf_keys.StopTxnSampledData = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.StopTxnSampledData, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "SupportedFeatureProfiles") == 0) {
                            free(vars->conf_keys.SupportedFeatureProfiles);
                            vars->conf_keys.SupportedFeatureProfiles = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.SupportedFeatureProfiles, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "TransactionMessageAtempts") == 0) {
                            free(vars->conf_keys.TransactionMessageAtempts);
                            vars->conf_keys.TransactionMessageAtempts = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.TransactionMessageAtempts, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "TransactionMessageRetryInterval") == 0) {
                            free(vars->conf_keys.TransactionMessageRetryInterval);
                            vars->conf_keys.TransactionMessageRetryInterval = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.TransactionMessageRetryInterval, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                        else if (strcmp(configuration_key->key, "UnlockConnectorOnEVSideDisconnect") == 0) {
                            free(vars->conf_keys.UnlockConnectorOnEVSideDisconnect);
                            vars->conf_keys.UnlockConnectorOnEVSideDisconnect = malloc(strlen(configuration_key->value) + 1);
                            snprintf(vars->conf_keys.UnlockConnectorOnEVSideDisconnect, strlen(configuration_key->value) + 1, "%s", configuration_key->value);
                        }
                    }
                }
            }

            if (get_configuration_conf_payload->unknown_key &&
                list_get_count(get_configuration_conf_payload->unknown_key)) {

                size_t len_n = list_get_count(get_configuration_conf_payload->unknown_key);
                for (int n = 0; n < len_n; n++) { // miro totes les unknownKeys que hi ha
                    char *unknown_key = list_get_head(get_configuration_conf_payload->unknown_key);
                    list_remove_head(get_configuration_conf_payload->unknown_key);

                    if (strlen(unknown_key) > 500) { // Error: OccurrenceConstraintViolation
                        send_occurrence_constraint_violation(header->unique_id, vars->client);
                        return;
                    }
                }
            }

            // No errors
            syslog(LOG_DEBUG, "GetConfiguration: No errors");
            vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
        }
        else if (strcmp(vars->current_tx_request, "\"RemoteStartTransaction\"") == 0) {
            // Passo el string a struct JSON
            struct RemoteStartTransactionConf *remote_start_conf_payload = cJSON_ParseRemoteStartTransactionConf(payload);

            // Comprovo errors abans d'enviar la resposta
            if (remote_start_conf_payload == NULL) { // Error: FormationViolation
                send_formation_violation(header->unique_id, vars->client);
            }
            else if (remote_start_conf_payload->status == -1) { // Error: ProtocolError
                send_protocol_error(header->unique_id, vars->client);
            }
            else if (remote_start_conf_payload->status == -2) { // Error: TypeConstraintViolation
                send_type_constraint_violation(header->unique_id, vars->client);
            }
            else { // No errors
                syslog(LOG_DEBUG, "RemoteStartTransaction: No errors");
                vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
            }
        }
        else if (strcmp(vars->current_tx_request, "\"RemoteStopTransaction\"") == 0) {
            // Passo el string a struct JSON
            struct RemoteStopTransactionConf *remote_stop_conf_payload = cJSON_ParseRemoteStopTransactionConf(payload);

            // Comprovo errors abans d'enviar la resposta
            if (remote_stop_conf_payload == NULL) { // Error: FormationViolation
                send_formation_violation(header->unique_id, vars->client);
            }
            else if (remote_stop_conf_payload->status == -1) { // Error: ProtocolError
                send_protocol_error(header->unique_id, vars->client);
            }
            else if (remote_stop_conf_payload->status == -2) { // Error: TypeConstraintViolation
                send_type_constraint_violation(header->unique_id, vars->client);
            }
            else { // No errors
                syslog(LOG_DEBUG, "RemoteStopTransaction: No errors");
                vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
            }
        }
        else if (strcmp(vars->current_tx_request, "\"Reset\"") == 0) {
            // Passo el string a struct JSON
            struct ResetConf *reset_conf_payload = cJSON_ParseResetConf(payload);

            // Comprovo errors abans d'enviar la resposta
            if (reset_conf_payload == NULL) { // Error: FormationViolation
                send_formation_violation(header->unique_id, vars->client);
            }
            else if (reset_conf_payload->status == -1) { // Error: ProtocolError
                send_protocol_error(header->unique_id, vars->client);
            }
            else if (reset_conf_payload->status == -2) { // Error: TypeConstraintViolation
                send_type_constraint_violation(header->unique_id, vars->client);
            }
            else { // No errors
                syslog(LOG_DEBUG, "Reset: No errors");
                vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
            }
        }
        else if (strcmp(vars->current_tx_request, "\"UnlockConnector\"") == 0) {
            // Passo el string a struct JSON
            struct UnlockConnectorConf *unlock_connector_conf_payload = cJSON_ParseUnlockConnectorConf(payload);

            // Comprovo errors abans d'enviar la resposta
            if (unlock_connector_conf_payload == NULL) { // Error: FormationViolation
                send_formation_violation(header->unique_id, vars->client);
            }
            else if (unlock_connector_conf_payload->status == -1) { // Error: ProtocolError
                send_protocol_error(header->unique_id, vars->client);
            }
            else if (unlock_connector_conf_payload->status == -2) { // Error: TypeConstraintViolation
                send_type_constraint_violation(header->unique_id, vars->client);
            }
            else { // No errors
                syslog(LOG_DEBUG, "UnlockConnector: No errors");
                vars->tx_state = ready_to_send; // canvio l'estat a disponible per enviar, ja que ha arribat la resposta -> es para el timeout i deixa enviar una altra petició
            }
        }
        // Not supported
        else { // Error: NotSupported
            char message[256];
            snprintf(message, sizeof(message), "[4,%s,\"NotSupported\",\"Requested Action is recognized but not supported by the receiver\",{}]", header->unique_id);

            // Envio el missatge al carregador
            ws_send("CALL ERROR", message, vars->client);
        }
    }
}

