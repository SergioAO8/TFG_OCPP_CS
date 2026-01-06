/*
 *  FILE
 *      boot_notification.h - header del fitxer boot_notification.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header del fitxer boot_notification.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _BOOT_NOTIFICATION_H_
#define _BOOT_NOTIFICATION_H_

#include "utils.h"
#include "ocpp_cs.h"

void proc_boot_notification(struct header_st *header, char *payload, ChargerVars *vars);

#endif
