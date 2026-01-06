/*
 *  FILE
 *      stop_transaction.h - header del fitxer stop_transaction.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del fitxer stop_transaction.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _STOP_TRANSACTION_H_
#define _STOP_TRANSACTION_H_

#include "utils.h"
#include "ocpp_cs.h"

void proc_stop_transaction(struct header_st *header, char *payload, ChargerVars *vars);

#endif
