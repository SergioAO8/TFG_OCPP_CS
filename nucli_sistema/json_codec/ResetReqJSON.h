#ifndef _RESETREQJSON_H_
#define _RESETREQJSON_H_

#include <cJSON.h>

enum Type_Reset {
    TYPE_HARD,
    TYPE_SOFT,
};

struct ResetReq {
    enum Type_Reset type;
};

struct ResetReq * cJSON_ParseResetReq(const char * s);
char * cJSON_PrintResetReq(const struct ResetReq * x);

#endif
