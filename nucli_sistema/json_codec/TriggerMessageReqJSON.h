#ifndef _TRIGGERMESSAGEREQJSON_H_
#define _TRIGGERMESSAGEREQJSON_H_

#include <cJSON.h>

enum RequestedMessage {
    REQUESTEDMESSAGE_BOOT_NOTIFICATION,
    REQUESTEDMESSAGE_DIAGNOSTICS_STATUS_NOTIFICATION,
    REQUESTEDMESSAGE_FIRMWARE_STATUS_NOTIFICATION,
    REQUESTEDMESSAGE_HEARTBEAT,
    REQUESTEDMESSAGE_METER_VALUES,
    REQUESTEDMESSAGE_STATUS_NOTIFICATION,
};

struct TriggerMessageReq {
    int64_t * connector_id;
    enum RequestedMessage requested_message;
};

struct TriggerMessageReq * cJSON_ParseTriggerMessageReq(const char * s);
char * cJSON_PrintTriggerMessageReq(const struct TriggerMessageReq * x);

#endif
