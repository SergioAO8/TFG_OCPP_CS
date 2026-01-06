#ifndef _AUTHORIZECONFJSON_H_
#define _AUTHORIZECONFJSON_H_

#include <cJSON.h>

enum Status {
    STATUS_ACCEPTED,
    STATUS_BLOCKED,
    STATUS_CONCURRENT_TX,
    STATUS_EXPIRED,
    STATUS_INVALID,
};

struct IdTagInfo {
    char * expiry_date;
    char * parent_id_tag;
    enum Status status;
};

struct AuthorizeConf {
    struct IdTagInfo * id_tag_info;
};

struct AuthorizeConf * cJSON_ParseAuthorizeConf(const char * s);
char * cJSON_PrintAuthorizeConf(const struct AuthorizeConf * x);

#endif
