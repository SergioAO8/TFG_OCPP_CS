#ifndef _STOPTRANSACTIONCONFJSON_H_
#define _STOPTRANSACTIONCONFJSON_H_

#include <cJSON.h>

enum Status_Stop {
    STATUS_STOP_ACCEPTED,
    STATUS_STOP_BLOCKED,
    STATUS_STOP_CONCURRENT_TX,
    STATUS_STOP_EXPIRED,
    STATUS_STOP_INVALID,
};

struct IdTagInfo_Stop {
    char * expiry_date;
    char * parent_id_tag;
    enum Status_Stop status;
};

struct StopTransactionConf {
    struct IdTagInfo_Stop * id_tag_info;
};

struct IdTagInfo_Stop * cJSON_ParseIdTagInfo(const char * s);
char * cJSON_PrintIdTagInfo_Stop(const struct IdTagInfo_Stop * x);
struct StopTransactionConf * cJSON_ParseStopTransactionConf(const char * s);
char * cJSON_PrintStopTransactionConf(const struct StopTransactionConf * x);

#endif
