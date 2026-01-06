#ifndef _RESETCONFJSON_H_
#define _RESETCONFJSON_H_

#include <cJSON.h>

enum Status_Reset {
    STATUS_RESET_ACCEPTED,
    STATUS_RESET_REJECTED,
};

struct ResetConf {
    enum Status_Reset status;
};

struct ResetConf * cJSON_ParseResetConf(const char * s);
char * cJSON_PrintResetConf(const struct ResetConf * x);

#endif
