#ifndef _UNLOCKCONNECTORREQJSON_H_
#define _UNLOCKCONNECTORREQJSON_H_

#include <cJSON.h>

struct UnlockConnectorReq {
    int64_t connector_id;
};

struct UnlockConnectorReq * cJSON_ParseUnlockConnectorReq(const char * s);
char * cJSON_PrintUnlockConnectorReq(const struct UnlockConnectorReq * x);

#endif
