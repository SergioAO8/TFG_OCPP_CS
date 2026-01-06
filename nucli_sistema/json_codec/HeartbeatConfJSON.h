#ifndef _HEARTBEATREQJSON_H_
#define _HEARTBEATREQJSON_H_

#include <cJSON.h>

struct HeartbeatConf {
    char * current_time;
};

struct HeartbeatConf * cJSON_ParseHeartbeatConf(const char * s);
char * cJSON_PrintHeartbeatConf(const struct HeartbeatConf * x);

#endif
