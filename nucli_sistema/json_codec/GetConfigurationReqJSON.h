#ifndef _GETCONFIGURATIONREQJSON_H_
#define _GETCONFIGURATIONREQJSON_H_

#include <cJSON.h>
#include <list.h>

struct GetConfigurationReq {
    list_t * key;
};

struct GetConfigurationReq * cJSON_ParseGetConfigurationReq(const char * s);
char * cJSON_PrintGetConfigurationReq(const struct GetConfigurationReq * x);

#endif
