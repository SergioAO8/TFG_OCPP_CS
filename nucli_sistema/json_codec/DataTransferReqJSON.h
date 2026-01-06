#ifndef _DATATRANSFERREQJSON_H_
#define _DATATRANSFERREQJSON_H_

#include <cJSON.h>

struct DataTransferReq {
    char * data;
    char * message_id;
    char * vendor_id;
};

struct DataTransferReq * cJSON_ParseDataTransferReq(const char * s);
char * cJSON_PrintDataTransferReq(const struct DataTransferReq * x);

#endif
