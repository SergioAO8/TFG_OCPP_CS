/*
 *  FILE
 *      heartbeat.h - header del fitxer heartbeat.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del fitxer heartbeat.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _HEARTBEAT_H_
#define _HEARTBEAT_H_

#include "utils.h"
#include "ocpp_cs.h"

void proc_heartbeat(struct header_st *header, char *payload, ChargerVars *vars);

#endif
