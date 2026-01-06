#ifndef _REMOTESTOPTRANSACTIONREQJSON_H_
#define _REMOTESTOPTRANSACTIONREQJSON_H_

#include <cJSON.h>
#include <stdint.h>

struct RemoteStopTransactionReq {
    int64_t transaction_id;
};

struct RemoteStopTransactionReq * cJSON_ParseRemoteStopTransactionReq(const char * s);
char * cJSON_PrintRemoteStopTransactionReq(const struct RemoteStopTransactionReq * x);

#endif
