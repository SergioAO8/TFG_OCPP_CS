/*
 *  FILE
 *      error_messages.h - header d'error_messages.h
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header d'error_messages.h.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _ERROR_MESSAGES_H_
#define _ERROR_MESSAGES_H_

#include <ws.h>

void send_formation_violation(const char *unique_id, ws_cli_conn_t client);
void send_protocol_error(const char *unique_id, ws_cli_conn_t client);
void send_property_constraint_violation(const char *unique_id, ws_cli_conn_t client);
void send_occurrence_constraint_violation(const char *unique_id, ws_cli_conn_t client);
void send_type_constraint_violation(const char *unique_id, ws_cli_conn_t client);
void send_generic_error(const char *unique_id, ws_cli_conn_t client);

#endif
