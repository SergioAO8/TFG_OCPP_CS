#ifndef _TRIGGERMESSAGECONFJSON_H_
#define _TRIGGERMESSAGECONFJSON_H_

#include <cJSON.h>

enum Status_Message {
    STATUS_MESSAGE_ACCEPTED,
    STATUS_MESSAGE_NOT_IMPLEMENTED,
    STATUS_MESSAGE_REJECTED,
};

struct TriggerMessageConf {
    enum Status_Message status;
};

struct TriggerMessageConf * cJSON_ParseTriggerMessageConf(const char * s);
char * cJSON_PrintTriggerMessageConf(const struct TriggerMessageConf * x);

#endif
