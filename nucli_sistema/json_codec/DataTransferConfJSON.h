#ifndef _DATATRANSFERCONFJSON_H_
#define _DATATRANSFERCONFJSON_H_

#include <cJSON.h>

enum Status_Data_Transfer {
    STATUS_DATA_TRANSFER_ACCEPTED,
    STATUS_DATA_TRANSFER_REJECTED,
    STATUS_DATA_TRANSFER_UNKNOWN_MESSAGE_ID,
    STATUS_DATA_TRANSFER_UNKNOWN_VENDOR_ID,
};

struct DataTransferConf {
    char * data;
    enum Status_Data_Transfer status;
};

struct DataTransferConf * cJSON_ParseDataTransferConf(const char * s);
char * cJSON_PrintDataTransferConf(const struct DataTransferConf * x);

#endif
