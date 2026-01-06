/*
 *  FILE
 *      start_transaction.h - header del fitxer start_transaction.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del fitxer start_transaction.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _START_TRANSACTION_H_
#define _START_TRANSACTION_H_

#include "utils.h"
#include "ocpp_cs.h"

void proc_start_transaction(struct header_st *header, char *payload, ChargerVars *vars);

#endif
