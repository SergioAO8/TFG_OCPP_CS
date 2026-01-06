#ifndef _STATUSNOTIFICATIONCONFJSON_H_
#define _STATUSNOTIFICATIONCONFJSON_H_

#include <cJSON.h>

struct StatusNotificationConf {
};

struct StatusNotificationConf * cJSON_ParseStatusNotificationConf(const char * s);
char * cJSON_PrintStatusNotificationConf(const struct StatusNotificationConf * x);

#endif
