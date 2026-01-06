/*
 *  FILE
 *      error_messages.c - funcions per enviar missatges d'error al carregador
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Aquí hi ha una funció per cada tipus de missatge d'error que es pot enviar al carregador.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "error_messages.h"
#include "ws_server.h"

/*
 *  NAME
 *      send_formation_violation - Envia l'error formationViolation
 *  SYNOPSIS
 *      void send_formation_violation(const char *unique_id, ws_cli_conn_t client);
 *  DESCRIPTION
 *      Envia el missatge d'error formationViolation al carregador.
 *  RETURN VALUE
 *      Res.
 */
void send_formation_violation(const char *unique_id, ws_cli_conn_t client)
{
    // Formo el missatge
    char message[256];
    snprintf(message, sizeof(message), "[4,%s,\"FormationViolation\",\"Payload for Action is syntactically incorrect"
    " or not conform the PDU structure for Action\",{}]", unique_id);

    // Envio el missatge al carregador
    ws_send("CALL ERROR", message, client);
}

/*
 *  NAME
 *      send_protocol_error - Envia l'error protocolError
 *  SYNOPSIS
 *      void send_protocol_error(const char *unique_id, ws_cli_conn_t client);
 *  DESCRIPTION
 *      Envia el missatge d'error protocolError al carregador.
 *  RETURN VALUE
 *      Res.
 */
void send_protocol_error(const char *unique_id, ws_cli_conn_t client)
{
    // Formo el missatge
    char message[256];
    snprintf(message, sizeof(message), "[4,%s,\"ProtocolError\",\"Payload for Action is incomplete\",{}]", unique_id);

    // Envio el missatge al carregador
    ws_send("CALL ERROR", message, client);
}

/*
 *  NAME
 *      send_property_constraint_violation - Envia l'error propertyConstraintViolation
 *  SYNOPSIS
 *      void send_property_constraint_violation(const char *unique_id, ws_cli_conn_t client);
 *  DESCRIPTION
 *      Envia el missatge d'error propertyConstraintViolation al carregador.
 *  RETURN VALUE
 *      Res.
 */
void send_property_constraint_violation(const char *unique_id, ws_cli_conn_t client)
{
    // Formo el missatge
    char message[256];
    snprintf(message, sizeof(message), "[4,%s,\"PropertyConstraintViolation\",\"Payload is syntactically correct but at least one "
        "field contains an invalid value\",{}]", unique_id);

    // Envio el missatge al carregador
    ws_send("CALL ERROR", message, client);
}

/*
 *  NAME
 *      send_occurrence_constraint_violation - Envia l'error ocurrenceConstraintViolation
 *  SYNOPSIS
 *      void send_occurrence_constraint_violation(const char *unique_id, ws_cli_conn_t client);
 *  DESCRIPTION
 *      Envia el missatge d'error ocurrenceConstraintViolation al carregador.
 *  RETURN VALUE
 *      Res.
 */
void send_occurrence_constraint_violation(const char *unique_id, ws_cli_conn_t client)
{
    // Formo el missatge
    char message[256];
    snprintf(message, sizeof(message), "[4,%s,\"OccurrenceConstraintViolation\",\"Payload for Action is syntactically "
        "correct but atleast one of the fields violates occurence constraints\",{}]", unique_id);

    // Envio el missatge al carregador
    ws_send("CALL ERROR", message, client);
}

/*
 *  NAME
 *      send_type_constraint_violation - Envia l'error typeConstraintViolation
 *  SYNOPSIS
 *      void send_type_constraint_violation(const char *unique_id, ws_cli_conn_t client);
 *  DESCRIPTION
 *      Envia el missatge d'error typeConstraintViolation al carregador.
 *  RETURN VALUE
 *      Res.
 */
void send_type_constraint_violation(const char *unique_id, ws_cli_conn_t client)
{
    // Formo el missatge
    char message[256];
    snprintf(message, sizeof(message), "[4,%s,\"TypeConstraintViolation\",\"Payload for Action is syntactically correct "
        "but at least one of the fields violates data type constraints (e.g. “somestring”: 12)\",{}]", unique_id);

    // Envio el missatge al carregador
    ws_send("CALL ERROR", message, client);
}

/*
 *  NAME
 *      send_generic_error - Envia l'error genericError
 *  SYNOPSIS
 *      void send_generic_error(const char *unique_id, ws_cli_conn_t client);
 *  DESCRIPTION
 *      Envia el missatge d'error genericError al carregador.
 *  RETURN VALUE
 *      Res.
 */
void send_generic_error(const char *unique_id, ws_cli_conn_t client)
{
    // Formo el missatge
    char message[256];
    snprintf(message, sizeof(message), "[4,%s,\"GenericError\",\"Generic Error\",{}]", unique_id);

    // Envio el missatge al carregador
    ws_send("CALL ERROR", message, client);
}
