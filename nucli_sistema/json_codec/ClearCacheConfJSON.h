#ifndef _CLEARCACHECONFJSON_H_
#define _CLEARCACHECONFJSON_H_

#include <cJSON.h>

enum Status_Cache {
    STATUS_CACHE_ACCEPTED,
    STATUS_CACHE_REJECTED,
};

struct ClearCacheConf {
    enum Status_Cache status;
};

struct ClearCacheConf * cJSON_ParseClearCacheConf(const char * s);
char * cJSON_PrintClearCacheConf(const struct ClearCacheConf * x);

#endif
