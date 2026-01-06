/*
 *  FILE
 *      data_transfer.h - header del fitxer data_transfer.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del fitxer data_transfer.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _DATA_TRANSFER_H_
#define _DATA_TRANSFER_H_

#include "utils.h"
#include "ocpp_cs.h"

void proc_data_transfer(struct header_st *header, char *payload, ChargerVars *vars);

#endif
