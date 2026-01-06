/*
 *  FILE
 *      heartbeat.c - Mòdul amb la funció per gestionar la petició Heartbeat
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Mòdul amb la funció per gestionar la petició Heartbeat.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "heartbeat.h"
#include "HeartbeatReqJSON.h"
#include "HeartbeatConfJSON.h"
#include "ocpp_cs.h"
#include "ws_server.h"
#include "error_messages.h"
#include "utils.h"

/*
 *  NAME
 *      proc_heartbeat - gestiona la petició de Heartbeat
 *  SYNOPSIS
 *      void proc_heartbeat(struct header_st *header, char *payload, ChargerVars *vars);
 *  DESCRIPTION
 *      Controla els possibles errors en la petició i envia el respectiu error en cas que hi hagi.
 *      Si no hi ha errors, es gestiona la petició (es modifiquen les variables globals del
 *      sistema de control que calguin), i s'envia la resposta.
 *  RETURN VALUE
 *      Res.
 */
void proc_heartbeat(struct header_st *header, char *payload, ChargerVars *vars)
{
    // Passo el string a struct JSON
    struct HeartbeatReq *heartbeat_req = cJSON_ParseHeartbeatReq(payload);

    // Comprovo errors abans d'enviar la resposta
    if (heartbeat_req == NULL) { // Error: FormationViolation
        send_formation_violation(header->unique_id, vars->client);
    }
    else if (strcmp(payload, "{}") != 0) { // Error: ProtocolError
        send_protocol_error(header->unique_id, vars->client);
    }
    else { // No errors -> CALLRESULT
        struct HeartbeatConf heartbeat_conf;

        // timestamp
        time_t t = time(NULL);
        struct tm *currentTime = localtime(&t);
        heartbeat_conf.current_time = malloc(64);
        snprintf(heartbeat_conf.current_time, 64, "\%04d-%02d-%02dT%02d:%02d:%02dZ",
            currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
            currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);

        // Formo el missatge
        char message[256];
        snprintf(message, sizeof(message), "[3,%s,%s]", header->unique_id,
            remove_spaces(cJSON_PrintHeartbeatConf(&heartbeat_conf)));

        // Envio el missatge al carregador
        ws_send("CALL RESULT", message, vars->client);
    }

    // Allibero la memòria
    free(heartbeat_req);
}
