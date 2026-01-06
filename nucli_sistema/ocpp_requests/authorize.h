/*
 *  FILE
 *      authorize.h - header del fitxer authorize.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del fitxer authorize.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _AUTHORIZE_H_
#define _AUTHORIZE_H_

#include "utils.h"
#include "ocpp_cs.h"

void proc_authorize(struct header_st *header, char *payload, ChargerVars *vars);

#endif
