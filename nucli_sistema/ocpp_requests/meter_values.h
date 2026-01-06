/*
 *  FILE
 *      meter_values.h - header del fitxer meter_values.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del fitxer meter_values.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _METER_VALUES_H_
#define _METER_VALUES_H_

#include "utils.h"
#include "ocpp_cs.h"

void proc_meter_values(struct header_st *header, char *payload, ChargerVars *vars);

#endif
