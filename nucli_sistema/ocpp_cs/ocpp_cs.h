/*
 *  FILE
 *      ocpp_cs.h - header d'ocpp_cs.c.
 *  PROJECT
 *      TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
 *  DESCRIPTION
 *      Header de'ocpp_cs.c, el mòdul principal del nucli del sistema de control.
 *  AUTHOR
 *      Sergio Abate
 *  OPERATING SYSTEM
 *      Linux
 */

#ifndef _SISTEMA_CONTROL_H_
#define _SISTEMA_CONTROL_H_

#define HEARTBEAT_INTERVAL 86400
#define RESEND_BOOT_NOTIFICATION_INTERVAL 300
#define NUM_CONNECTORS 2

#define ID_TAG_LEN 20 // mida establerta pel protocol

// possibles estats dels connectors
#define CONN_AVAILABLE 0
#define CONN_CHARGING 1
#define CONN_FAULTED 2
#define CONN_FINISHING 3
#define CONN_PREPARING 4
#define CONN_RESERVED 5
#define CONN_SUSPENDED_EV 6
#define CONN_SUSPENDED_EVSE 7
#define CONN_UNAVAILABLE 8
#define CONN_UNKNOWN 9

#include <stdint.h>
#include <ws.h>
#include "BootNotificationConfJSON.h"

/* enum per indicar l'estat de l'enviament de peticions:
 * si està ready deixa enviar en qualsevol moment
 * si és sent no deixa enviar una altra request fins a rebre la resposta de l'anterior
*/
enum tx_state_t {
    ready_to_send,
    sent
};

// claus de configuració del punt de càrrega
typedef struct {
    char *AuthorizeRemoteTxRequests;
    char *ClockAlignedDataInterval;
    char *ConnectionTimeOut;
    char *ConnectorPhaseRotation;
    char *GetConfigurationMaxKeys;
    char *HeartbeatInterval;
    char *LocalAuthorizeOffline;
    char *LocalPreAuthorize;
    char *MeterValuesAlignedData;
    char *MeterValuesSampledData;
    char *MeterValueSampleInterval;
    char *NumberOfConnectors;
    char *ResetRetries;
    char *StopTransactionOnEVSideDisconnect;
    char *StopTransactionOnInvalidId;
    char *StopTxnAligneData;
    char *StopTxnSampledData;
    char *SupportedFeatureProfiles;
    char *TransactionMessageAtempts;
    char *TransactionMessageRetryInterval;
    char *UnlockConnectorOnEVSideDisconnect;
} ConfigurationKeys;

// estrcutura amb les variables de cada punt de càrrega
typedef struct {
    int charger_id;                                       // identificador del carregador
    ws_cli_conn_t client;                                 // identifiador del client ws
    int64_t connectors_status[NUM_CONNECTORS + 1];        // aqui aniran els status de cada connector, els quals poden ser qualsevol dels defines CONN_<>
    char current_id_tags[NUM_CONNECTORS + 1][ID_TAG_LEN]; // idTag de les transaccions actives
    struct BootNotificationConf boot;                     // per veure el status general del carregador
    char current_id_tag[ID_TAG_LEN];                      // idTag rebut en l'autentificació per acceptar o no transaccions
    char current_vendor[20];                              // per veure el vendor qual está connectat
    char current_model[20];                               // per veure el model qual está connectat
    int64_t transaction_list[NUM_CONNECTORS + 1];         // aqui aniran els trasnactionId dels connectors que estan en una transacció activa
    int64_t current_transaction_id;                       // l'últim transactionId que s'ha utilitzat
    char current_tx_request[32];                          // la request activa que s'ha transmès al carregador per verificar la respectiva resposta
    uint64_t current_unique_id;                           // unique_id actual que va incrementant cada vegada que el sistema envia una request
    enum tx_state_t tx_state;                             // estat del sistema
    ConfigurationKeys conf_keys;                          // claus de configuració del punt de càrrega
} ChargerVars;

// llista d'idTags, els quals es podran autoritzar
extern char *auth_list[];

// llista de chargePointModels, els quals podran fer bootNotification
extern char *cp_models[];

// llista de chargePointVendors, els quals podran fer bootNotification
extern char *cp_vendors[];

void init_system(ChargerVars *vars);
void system_on_receive(char *req, ChargerVars *vars);
void send_request(int option, char *payload, ChargerVars *vars);

#endif
