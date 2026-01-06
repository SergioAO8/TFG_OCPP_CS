#ifndef _AUTHORIZEREQJSON_H_
#define _AUTHORIZEREQJSON_H_

#include <cJSON.h>

struct AuthorizeReq {
    char * id_tag;
};

struct AuthorizeReq * cJSON_ParseAuthorizeReq(const char * s);
char * cJSON_PrintAuthorizeReq(const struct AuthorizeReq * x);

#endif
