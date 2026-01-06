/*
 *  FILE
 *      utils.h - header de utils.c
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header header de utils.c.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "ocpp_cs.h"

// struct per les recepcions de requests
struct req_rx {
    char *header;
    char *payload;
};

// struct per tractar els elements del header
struct header_st {
    int message_type_id;
    char *unique_id;
    char *action;
};

struct req_rx *split_message(struct req_rx *dest, char *src);
struct header_st *split_header(struct req_rx *src, struct header_st *dest);
char *remove_spaces(char *json);
char *remove_quotes(char *str);
bool check_id_tag(char *id_tag);
bool check_concurrent_tx_id_tag(char *id_tag, ChargerVars *vars);
bool check_cp_model(const char *charge_point_model);
bool check_cp_vendor(const char *charge_point_vendor);
bool check_transaction_id(int64_t transaction_id, const ChargerVars *vars);
void delete_transaction_id(int64_t transaction_id, ChargerVars *vars);
char *ocpp_strptime(const char *s, const char *format, struct tm *tm, size_t len);

#endif
