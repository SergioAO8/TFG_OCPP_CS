/*
 *  FILE
 *      utils.c - funcions utils del projecte
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Fitxer amb les funcions útils per la resta de mòduls.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#define _XOPEN_SOURCE // per trobar la funció de llibreria strptime()

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "utils.h"
#include "ocpp_cs.h"

/*
 *  NAME
 *      split_message - Divideix un missatge en header i payload
 *  SYNOPSIS
 *      struct req_rx *split_message(struct req_rx *dest, char *src);
 *  DESCRIPTION
 *      A partir d'un string, el divideix en header i payload.
 *  RETURN VALUE
 *      Retorna el resultat en una struct amb header i paylaod.
 */
struct req_rx *split_message(struct req_rx *dest, char *src)
{
    char tmp[1000];
    snprintf(tmp, sizeof(tmp), "%s", src + 1);

    // Obtinc el Header
    char* header = strtok(tmp, "{");
    size_t len = strlen(header);
    header[len - 1] = 0;
    dest->header = malloc(len + 1);
    snprintf(dest->header, len + 1, "%s", header);

    // Obtinc el Payload
    char *payload = src + (strlen(header) + 2);
    len = strlen(payload);
    payload[len - 1] = 0;
    dest->payload = malloc(len + 1);
    snprintf(dest->payload, len + 1, "%s", payload);

    return dest;
}

/*
 *  NAME
 *      split_header - Divideix un header en messageTypeId, uniqueId i action.
 *  SYNOPSIS
 *      struct header_st *split_header(struct req_rx *src, struct header_st *dest);
 *  DESCRIPTION
 *      A partir d'una struct de header i payload, agafa el header i
 *      divideix un header en messageTypeId, uniqueId i action.
 *  RETURN VALUE
 *      Retorna el resultat en una struct amb messageTypeId, uniqueId i action.
 */
struct header_st *split_header(struct req_rx *src, struct header_st *dest)
{
    // Obtinc el messageTypeId
    dest->message_type_id = src->header[0];

    char tmp[280];
    snprintf(tmp, sizeof(tmp), "%s", src->header + 2);

    // Obtinc el uniqueId
    char *unique_id = strtok(tmp, ",");
    dest->unique_id = malloc(strlen(unique_id) + 1);
    snprintf(dest->unique_id, strlen(unique_id) + 1, "%s", unique_id);

     // Obtinc l'action
    char *action = src->header + (strlen(unique_id) + 3);
    dest->action = malloc(strlen(action) + 1);
    snprintf(dest->action, strlen(action) + 1, "%s", action);

    return dest;
}

/*
 *  NAME
 *      remove_spaces - Elimina els espais i tabulacions d'un string
 *  SYNOPSIS
 *      char *remove_spaces(char *json);
 *  DESCRIPTION
 *      Elimina els espais i tabulacions d'un string.
 *  RETURN VALUE
 *      Retorna el resultat en un string.
 */
char *remove_spaces(char *json)
{
    int index = 0;

    for (int i = 0; i < strlen(json); i++) {
        if (json[i] != ' ' && json[i] != '\n' && json[i] != '\t') {
            json[index++] = json[i];
        }
    }

    json[index] = '\0';

    return json;
}

/*
 *  NAME
 *      remove_quotes - Elimina les cometes d'un string
 *  SYNOPSIS
 *      char *remove_quotes(char *str);
 *  DESCRIPTION
 *      Elimina les cometes d'un string.
 *  RETURN VALUE
 *      Retorna el resultat en un string.
 */
char *remove_quotes(char *str)
{
    int index = 0;

    for (int i = 0; i < strlen(str); i++) {
        if (str[i] != '"') {
            str[index++] = str[i];
        }
    }

    str[index] = '\0';

    return str;
}

/*
 *  NAME
 *      check_id_tag - Comprova si un idTag es troba a la auth_list
 *  SYNOPSIS
 *      bool check_id_tag(char *id_tag);
 *  DESCRIPTION
 *      Comprova si un idTag es troba a la auth_list del sistema de control, indicant si és vàlid o no.
 *  RETURN VALUE
 *      Retorna true si és vàlid.
 *      Retorna false en cas contrari.
 */
bool check_id_tag(char *id_tag)
{
    for (int i = 0; i < (strlen((char *)auth_list) - 1); i++) {
        if (strcasecmp(auth_list[i], id_tag) == 0)
            return true;
    }

    return false;
}

/*
 *  NAME
 *      check_concurrent_tx_id_tag - Comprova si ja s'ha iniciat una càrrega amb un idTag en concret.
 *  SYNOPSIS
 *      bool check_id_tag(char *id_tag);
 *  DESCRIPTION
 *      Comprova si ja s'ha iniciat una càrrega amb un idTag en concret, indicant si és vàlid o no.
 *  RETURN VALUE
 *      Retorna true si no és vàlid.
 *      Retorna false en cas contrari.
 */
bool check_concurrent_tx_id_tag(char *id_tag, ChargerVars *vars)
{
    for (int i = 0; i < (strlen((char *)vars->current_id_tags) - 1); i++) {
        if (strcasecmp(vars->current_id_tags[i], id_tag) == 0)
            return true;
    }

    return false;
}

/*
 *  NAME
 *      check_cp_model - Comprova si un chargePointModel es troba a la llista de models
 *  SYNOPSIS
 *      bool check_cp_model(const char *charge_point_model)
 *  DESCRIPTION
 *      Comprova si un chargePointModel es troba a la llista de models del sistema de control.
 *  RETURN VALUE
 *      Retorna true si és vàlid.
 *      Retorna false en cas contrari.
 */
bool check_cp_model(const char *charge_point_model)
{
    for (int i = 0; i < (strlen((char *)cp_models) - 1); i++) {
        if (strcmp(cp_models[i], charge_point_model) == 0)
            return true;
    }

    return false;
}

/*
 *  NAME
 *      check_cp_vendor - Comprova si un chargePointVendor es troba a la llista de vendors
 *  SYNOPSIS
        bool check_cp_vendor(const char *charge_point_vendor)
 *  DESCRIPTION
 *      Comprova si un chargePointVendor es troba a la llista de vendors del sistema de control.
 *  RETURN VALUE
 *      Retorna true si és vàlid.
 *      Retorna false en cas contrari.
 */
bool check_cp_vendor(const char *charge_point_vendor)
{
    for (int i = 0; i < (strlen((char *)cp_vendors) - 1); i++) {
        if (strcmp(cp_vendors[i], charge_point_vendor) == 0)
            return true;
    }

    return false;
}

/*
 *  NAME
 *      check_transaction_id - Comprova si un transactionId es troba a la transaction_list.
 *  SYNOPSIS
 *      bool check_transaction_id(int64_t transaction_id, const ChargerVars *vars)
 *  DESCRIPTION
 *      Comprova si un transactionId es troba a la transaction_list del sistema de control.
 *  RETURN VALUE
 *      Retorna true si és vàlid.
 *      Retorna false en cas contrari.
 */
bool check_transaction_id(int64_t transaction_id, const ChargerVars *vars)
{
    for (int i = 0; i <= NUM_CONNECTORS; i++) {
        if (vars->transaction_list[i] == transaction_id)
            return true;
    }

    return false;
}

/*
 *  NAME
 *      delete_transaction_id - Esborra el transactionId de la transactionList
 *  SYNOPSIS
 *      void delete_transaction_id(int64_t transaction_id);
 *  DESCRIPTION
 *      Esborra el transactionId de la transactionList del sistema de control.
 *  RETURN VALUE
 *      Res.
 */
void delete_transaction_id(int64_t transaction_id, ChargerVars *vars)
{
    for (int i = 0; i <= NUM_CONNECTORS; i++) {
        if (vars->transaction_list[i] == transaction_id)
            vars->transaction_list[i] = -1;
    }
}

/*
 *  NAME
 *      ocpp_strptime - funció strptime() personalitzada
 *  SYNOPSIS
 *      char *ocpp_strptime(const char *s, const char *format, struct tm *tm, size_t len);
 *  DESCRIPTION
 *      Funció strptime() personalitzada, permet els formats de timestamp del protocol OCPP.
 *  RETURN VALUE
 *      Si tot va bé escriu al data a la struct tm.
 *      En cas contrari, retorna NULL.
 */
char *ocpp_strptime(const char *s, const char *format, struct tm *tm, size_t len)
{
    // trec la part de data+hora+segons
    char part_one[32];
    memcpy(part_one, s, len);
    part_one[len] = 0;

    // trec la segona part
    char part_two[32];
    snprintf(part_two, sizeof(part_two), "%s", &s[strlen(part_one)]);

    // ajunto les dues parts i crido strptime()
    memset(tm, 0, sizeof(struct tm));
    char mys[32];
    char *mytimezone;
    if ((mytimezone = strchr(part_two, 'Z')) != NULL) { // cas en que hi ha una Z al final
        snprintf(mys, sizeof(mys), "%s%s", part_one, mytimezone);
        return strptime(mys, format, tm);
    }
    else if ((mytimezone = strchr(part_two, '+')) != NULL ||
        (mytimezone = strchr(part_two, '-')) != NULL) { // cas en que hi ha timezone explicitament

        snprintf(mys, sizeof(mys), "%s%s", part_one, mytimezone);
        return strptime(mys, format, tm);
    }

    // cas no reconegut -> Error
    return NULL;
}
