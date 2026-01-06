#ifndef _GETCONFIGURATIONCONFJSON_H_
#define _GETCONFIGURATIONCONFJSON_H_

#include <cJSON.h>
#include <list.h>
#include <stdbool.h>

struct ConfigurationKey {
    char * key;
    bool readonly;
    char * value;
};

struct GetConfigurationConf {
    list_t * configuration_key;
    list_t * unknown_key;
};

char * cJSON_PrintConfigurationKey(const struct ConfigurationKey * x);
struct ConfigurationKey * cJSON_ParseConfigurationKey(const char * s);
struct GetConfigurationConf * cJSON_ParseGetConfigurationConf(const char * s);
char * cJSON_PrintGetConfigurationConf(const struct GetConfigurationConf * x);

#endif
