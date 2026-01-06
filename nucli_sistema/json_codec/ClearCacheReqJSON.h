#ifndef _CLEARCACHEREQJSON_H_
#define _CLEARCACHEREQJSON_H_

#include <cJSON.h>

struct ClearCacheReq {
};

struct ClearCacheReq * cJSON_ParseClearCacheReq(const char * s);
char * cJSON_PrintClearCacheReq(const struct ClearCacheReq * x);

#endif
