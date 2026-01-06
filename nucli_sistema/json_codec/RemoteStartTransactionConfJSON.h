#ifndef _REMOTESTARTTRANSACTIONCONFJSON_H_
#define _REMOTESTARTTRANSACTIONCONFJSON_H_

#include <cJSON.h>

enum Status_Remote_Start {
    STATUS_REMOTE_START_ACCEPTED,
    STATUS_REMOTE_START_REJECTED,
};

struct RemoteStartTransactionConf {
    enum Status_Remote_Start status;
};

struct RemoteStartTransactionConf * cJSON_ParseRemoteStartTransactionConf(const char * s);
char * cJSON_PrintRemoteStartTransactionConf(const struct RemoteStartTransactionConf * x);

#endif
