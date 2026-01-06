/*
 *  FILE
 *      ws_server.h - header del servidor WebSocket
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del servidor WebSocket.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <ws.h>

#ifndef _SERVER_H_
#define _SERVER_H_

#define DATABASE_PATH "../../servidor_web/base_dades/base_dades.db"
#define MAX_CHARGERS 4

void ws_send(const char *option, char *text, ws_cli_conn_t client);

#endif
