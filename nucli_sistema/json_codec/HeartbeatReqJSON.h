#ifndef _HEARTBEATREQJSON_h_
#define _HEARTBEATREQJSON_h_

#include <cJSON.h>

struct HeartbeatReq {
};

struct HeartbeatReq * cJSON_ParseHeartbeatReq(const char * s);
char * cJSON_PrintHeartbeatReq(const struct HeartbeatReq * x);

#endif
