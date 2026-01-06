/*
 *  FILE
 *      ws_server.c - servidor WebSocket del sistema de control
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Fitxer on es configura i es crea el servidor WebSocket del sistema de control.
 *      També es gestiona l'enviament de peticions al carregador per part de l'usuari.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <ws.h>
#include "ws_server.h"
#include "ocpp_cs.h"
#include "BootNotificationConfJSON.h"

#define RESET   "\e[0m"
#define YELLOW  "\e[0;33m"
#define RED     "\e[0;31m"
#define BLUE    "\e[0;34m"
#define CYAN    "\e[0;36m"
#define GREEN   "\e[0;32m"

ChargerVars charger_vars[MAX_CHARGERS + 1];

// Prototips de les funcions
static void onopen(ws_cli_conn_t client);
static void onclose(ws_cli_conn_t client);
static void onmessage(ws_cli_conn_t client, const unsigned char *msg, uint64_t size, int type);
static void select_request(ChargerVars *vars, const char *operation);
static int get_charger_index(int charger_id);

/*
 *  NAME
 *      main - main() del servidor i del sistema de control.
 *  SYNOPSIS
 *      int main(int argc, char* argv[])
 *  DESCRIPTION
 *      main() del servidor i del sistema de control. El thread principal
 *      del procés s'encarrega de les recepcions de peticions.
 *      A part, es crea un altre thread per encarregar-se de l'enviament
 *      de peticions.
 *  RETURN VALUE
 *      Res.
 */
int main(int argc, char* argv[])
{
    // configuro el syslog
    int loglevel = LOG_DEBUG;
    setlogmask(LOG_UPTO(loglevel));
    openlog(NULL, LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);

    // Inicialitzo algunes variables
    for (int i = 0; i <= MAX_CHARGERS; i++) {
        memset(&charger_vars[i], 0, sizeof(ChargerVars));
        charger_vars[i].client = -1;
        charger_vars[i].current_transaction_id = 0;
        charger_vars[i].current_unique_id = 0;
        charger_vars[i].charger_id = i;

        for (int index = 1; index < (NUM_CONNECTORS + 1); index++) {
            charger_vars[i].connectors_status[index] = CONN_UNKNOWN;
        }
    }

    // crea un thread per cada connexió, aquest s'encarrega de rebre les peticions del carregador i els missatges de la web
    ws_socket(&(struct ws_server){
        .host = "localhost",
        .port = 8080,
        .thread_loop   = 0, // d'aquesta manera ws_socket() és bloquejant
        .timeout_ms    = 1000,
        .evs.onopen    = &onopen,
        .evs.onclose   = &onclose,
        .evs.onmessage = &onmessage
    });

    return 0;
}

/*
 *  NAME
 *      onopen - Inicialitza cada connexió que s'obre.
 *  SYNOPSIS
 *      onopen(ws_cli_conn_t client);
 *  DESCRIPTION
 *      Inicialitza la connexió amb el client i el sistema de control. S'executa en obrir una connexió.
 *  RETURN VALUE
 *      Res.
 */
static void onopen(ws_cli_conn_t client)
{
    char *cli;
    cli = ws_getaddress(client);
    syslog(LOG_NOTICE, "Connection opened, addr: %s\n", cli);

    // busco el primer index de carregador disponible i l'assigno si hi ha espai
    int index = get_charger_index(-1);
    if (index != -1) {
        charger_vars[index].client = client;

        init_system(&charger_vars[index]); // inicialitzo el sistema
    }
    else
        syslog(LOG_WARNING, "%s: Warning: No hi ha espai per més carregadors\n", __func__);
}

/*
 *  NAME
 *      onclose - Tanca la connexió amb el client.
 *  SYNOPSIS
 *      void onclose(ws_cli_conn_t client);
 *  DESCRIPTION
 *      Tanca la connexió amb el client. S'executa en tancar una connexió.
 *  RETURN VALUE
 *      Res.
 */
static void onclose(ws_cli_conn_t client)
{
    int index = get_charger_index(client);
    if (index != -1) {
        syslog(LOG_DEBUG, "%s: index = %d\n", __func__, index);
        char *cli;
        cli = ws_getaddress(client);
        syslog(LOG_NOTICE, "Connection closed, addr: %s\n", cli);

        charger_vars[index].client = -1;
        snprintf(charger_vars[index].current_vendor, 20, "%s", "");
        snprintf(charger_vars[index].current_model, 20, "%s", "");

        for (int i = 1; i <= MAX_CHARGERS; i++) {
            // Formo el missatge per enviar a la web l'estat dels carregadors
            char information[1024];
            snprintf(information, sizeof(information), "{\"charger\": \"%d\", \"type\": \"stopTransaction\", \"connector1\": 9,"
                " \"connector2\": 9, \"idTag1\": \"%s\", \"idTag2\": \"%s\", \"transactionId1\": %ld, \"transactionId2\": %ld}",
                charger_vars[index].charger_id, charger_vars->current_id_tags[1], charger_vars->current_id_tags[2],
                charger_vars->transaction_list[1], charger_vars->transaction_list[2]);

            // Envio el missatge a la web
            ws_send("WEB", information, charger_vars[0].client);

            // Formo el missatge per enviar a la web
            char information_2[1024];
            snprintf(information_2, sizeof(information_2), "{\"charger\": \"%d\", \"type\": \"bootNotification\", \"general\": 2, \"vendor\": \"\", "
                "\"model\": \"\"}", charger_vars[index].charger_id);

            // Envio el missatge a la web
            ws_send("WEB", information_2, charger_vars[0].client);

            memset(information, 0, 1024);
            memset(information_2, 0, 1024);
        }
    }
    else
        syslog(LOG_WARNING, "%s: Warning: no s'ha trobat el carregador\n", __func__);
}

/*
 *  NAME
 *      onmessage - Rep els missatges del carregador.
 *  SYNOPSIS
 *      void onmessage(ws_cli_conn_t client, const unsigned char *msg, uint64_t size, int type);
 *  DESCRIPTION
 *      Rep els missatges del carregador i els envia al fitxer del sistema de control per gestionar-los.
 *  RETURN VALUE
 *      Res.
 */
static void onmessage(ws_cli_conn_t client, const unsigned char *msg, uint64_t size, int type)
{
    if (strcmp((char *)msg, "Flask client") == 0) { // missatge d'inicialització del servidor web
        syslog(LOG_NOTICE, "Flask connectat\n");

        int index = get_charger_index(client); // busco el index del client web
        charger_vars[0].client = client; // assigno el client web al charger_vars[0]
        if (index != -1)
            charger_vars[index].client = -1; // resetejo aquest charger_vars per a que si pugui connectar un carregador després

        // Formo el missatge per enviar a la web l'estat dels carregadors
        char information[1024];
        for (int i = 1; i <= MAX_CHARGERS; i++) {
            snprintf(information, sizeof(information), "{\"charger\": \"%d\", \"type\": \"stopTransaction\", \"connector1\": %ld,"
                " \"connector2\": %ld, \"idTag1\": \"no_charging\", \"idTag2\": \"no_charging\", \"transactionId1\": -1, \"transactionId2\": -1}",
                charger_vars[i].charger_id, charger_vars[i].connectors_status[1], charger_vars[i].connectors_status[2]);

            // Envio el missatge a la web
            ws_send("WEB", information, charger_vars[0].client);

            // Formo el missatge per enviar a la web
            char information_2[1024];
            snprintf(information_2, sizeof(information_2), "{\"charger\": \"%d\", \"type\": \"bootNotification\", \"general\": %d, \"vendor\": \"%s\", "
                "\"model\": \"%s\"}", charger_vars[i].charger_id, charger_vars[i].boot.status, charger_vars[i].current_vendor, charger_vars[i].current_model);

            // Envio el missatge a la web
            ws_send("WEB", information, charger_vars[0].client);

            // netejo els continguts
            memset(information, 0, 1024);
            memset(information_2, 0, 1024);
        }
    }
    else {
        char message[1024];
        snprintf(message, sizeof(message), "%s", msg);
        char *rest = message;

        if (strcmp(strtok_r(rest, ":", &rest), "Flask") == 0) { // un usuari vol enviar una petició
            syslog(LOG_INFO, "%sRECEIVED MESSAGE: %s (%lu), from: %s\n", BLUE, msg, size, RESET);

            char *rest2 = rest;
            char *charger = strtok_r(rest2, ":", &rest2);
            char num_charger = charger[7];
            select_request(&charger_vars[num_charger - '0'], rest2);
        }
        else { // missatge d'un carregador
            int index = get_charger_index(client); // busca quin carregador és
            if (index != -1) {
                char *cli;
                cli = ws_getaddress(client);
                syslog(LOG_INFO, "%sRECEIVED MESSAGE: %s (%lu), from: %s%s\n", BLUE, msg,
                    size, cli, RESET);

                system_on_receive((char *) msg, &charger_vars[index]);
            }
            else
                syslog(LOG_ERR, "%s: Error: no s'ha trobat el carregador\n", __func__);
        }
    }
}

/*
 *  NAME
 *      ws_send - Envia els missatges al carregador o al servidor web.
 *  SYNOPSIS
 *      void ws_send(const char *option, char *text, ws_cli_conn_t client)
 *  DESCRIPTION
 *      Envia els missatges al carregador o al servidor web. Segons el tipus
 *      de missatge a enviar, s'imprimeix d'un color diferent al terminal.
 *  RETURN VALUE
 *      Res.
 */
void ws_send(const char *option, char *text, ws_cli_conn_t client)
{
    if (strcmp(option, "CALL") == 0) {
        ws_sendframe_txt(client, text);
        syslog(LOG_INFO, "%sSENDING REQUEST: %s%s\n", CYAN, text, RESET);
    }
    else if (strcmp(option, "CALL RESULT") == 0) {
        ws_sendframe_txt(client, text);
        syslog(LOG_INFO, "%sSENDING CONFIRMATION: %s%s\n\n", YELLOW, text, RESET);
    }
    else if (strcmp(option, "CALL ERROR") == 0) {
        ws_sendframe_txt(client, text);
        syslog(LOG_INFO, "%sSENDING ERROR: %s%s\n", RED, text, RESET);
    }
    else if (strcmp(option, "WEB") == 0) {
        ws_sendframe_txt(charger_vars[0].client, text);
        syslog(LOG_INFO, "%sSENDING TO WEB: %s%s\n", GREEN, text, RESET);
    }
}

/*
 *  NAME
 *      select_request - Permet a l'usuari enviar peticions al carregador.
 *  SYNOPSIS
 *      void *select_request(ChargerVars *vars, const char *operation)
 *  DESCRIPTION
 *      L'usuari escull quin missatge enviar, s'envia mòdul ocpp_cs
 *      per gestionar-lo, i aquest posteriorment l'envia al carregador.
 *  RETURN VALUE
 *      Res.
 */
static void select_request(ChargerVars *vars, const char *operation)
{
    char message[1024];
    memset(message, 0, 1024); // netejo el buffer
    snprintf(message, sizeof(message), "%s", operation);

    // s'analitza el missatge del servidor web per saber quina operació s'ha d'enviar
    char *action = strtok(message, ":");
    syslog(LOG_DEBUG, "action: %s\n", action);
    if (strcmp(action, "changeAvailability") == 0) {
        char *request = strtok(0, "");
        send_request('1', request, vars);
    }
    else if (strcmp(action, "clearCache") == 0) {
        char *request = strtok(0, "");
        send_request('2', request, vars);
    }
    else if (strcmp(action, "dataTransfer") == 0) {
        char *request = strtok(0, "");
        send_request('3', request, vars);
    }
    else if (strcmp(action, "getConfiguration") == 0) {
        char *request = strtok(0, "");
        send_request('4', request, vars);
    }
    else if (strcmp(action, "remoteStartTransaction") == 0) {
        char *request = strtok(0, "");
        send_request('5', request, vars);
    }
    else if (strcmp(action, "remoteStopTransaction") == 0) {
        char *request = strtok(0, "");
        send_request('6', request, vars);
    }
    else if (strcmp(action, "reset") == 0) {
        char *request = strtok(0, "");
        send_request('7', request, vars);
    }
    else if (strcmp(action, "unlockConnector") == 0) {
        char *request = strtok(0, "");
        send_request('8', request, vars);
    }
    else
        syslog(LOG_DEBUG, "desconegut\n");

    memset(message, 0, 1024); // netejo el buffer
}


/*
 *  NAME
 *      get_charger_index - retorna l'índex dels carregadors
 *  SYNOPSIS
 *      int get_charger_index(int client);
 *  DESCRIPTION
 *      Retorna la primera posició lliure de l'array de carregadors si client és -1,
 *      en cas contari retorna la posició de client.
 *  RETURN VALUE
 *      Si tot va bé retorna la primera posició lliure de l'array de carregadors o la posició que busquem.
 *      En cas contrari, retorna -1.
 */
static int get_charger_index(int client)
{
    int i;
    if (client == -1) {
        for (i = 1; i <= MAX_CHARGERS; i++) {
            syslog(LOG_DEBUG, "charger nº%d: %ld\n", i, charger_vars[i].client);
            if (charger_vars[i].client == -1)
                return i;
        }
    }
    else {
        for (i = 0; i <= MAX_CHARGERS; i++) {
            syslog(LOG_DEBUG, "searching: charger nº%d: %ld\n", i, charger_vars[i].client);
            if (charger_vars[i].client == client)
                return i;
        }
    }

    return -1; // no hi ha posicions lliures
}
