#ifndef _UNLOCKCONNECTORCONFJSON_H_
#define _UNLOCKCONNECTORCONFJSON_H_

#include <cJSON.h>

enum Status_Unlock {
    STATUS_UNLOCK_NOT_SUPPORTED,
    STATUS_UNLOCK_UNLOCKED,
    STATUS_UNLOCK_UNLOCK_FAILED,
};

struct UnlockConnectorConf {
    enum Status_Unlock status;
};

struct UnlockConnectorConf * cJSON_ParseUnlockConnectorConf(const char * s);
char * cJSON_PrintUnlockConnectorConf(const struct UnlockConnectorConf * x);

#endif
