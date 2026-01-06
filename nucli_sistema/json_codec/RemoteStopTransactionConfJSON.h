#ifndef _REMOTESTOPTRANSACTIONCONFJSON_H_
#define _REMOTESTOPTRANSACTIONCONFJSON_H_

#include <cJSON.h>

enum Status_Remote_Stop {
    STATUS_REMOTE_STOP_ACCEPTED,
    STATUS_REMOTE_STOP_REJECTED,
};

struct RemoteStopTransactionConf {
    enum Status_Remote_Stop status;
};

struct RemoteStopTransactionConf * cJSON_ParseRemoteStopTransactionConf(const char * s);
char * cJSON_PrintRemoteStopTransactionConf(const struct RemoteStopTransactionConf * x);

#endif
