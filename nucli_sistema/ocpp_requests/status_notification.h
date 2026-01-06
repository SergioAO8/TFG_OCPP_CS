/*
 *  FILE
 *      status_notification.h - header del fitxer status_notification.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del fitxer status_notification.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _STATUS_NOTIFICATION_H_
#define _STATUS_NOTIFICATION_H_

#include "utils.h"
#include "ocpp_cs.h"

void proc_status_notification(struct header_st *header, char *payload, ChargerVars *vars);

#endif
