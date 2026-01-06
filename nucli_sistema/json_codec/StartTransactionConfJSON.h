#ifndef _STARTTRANSACTIONCONFJSON_H_
#define _STARTTRANSACTIONCONFJSON_H_

#include <cJSON.h>
#include <stdint.h>

enum Status_Start {
    STATUS_START_ACCEPTED,
    STATUS_START_BLOCKED,
    STATUS_START_CONCURRENT_TX,
    STATUS_START_EXPIRED,
    STATUS_START_INVALID,
};

struct IdTagInfo_Start {
    char * expiry_date;
    char * parent_id_tag;
    enum Status_Start status;
};

struct StartTransactionConf {
    struct IdTagInfo_Start * id_tag_info;
    int64_t transaction_id;
};

struct IdTagInfo_Start * cJSON_ParseIdTagInfoStart(const char * s);
char * cJSON_PrintIdTagInfo(const struct IdTagInfo_Start * x);
struct StartTransactionConf * cJSON_ParseStartTransactionConf(const char * s);
char * cJSON_PrintStartTransactionConf(const struct StartTransactionConf * x);

#endif
