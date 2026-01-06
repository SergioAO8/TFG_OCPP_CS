#ifndef _STARTTRANSACTIONREQJSON_H_
#define _STARTTRANSACTIONREQJSON_H_

#include <cJSON.h>
#include <stdint.h>

struct StartTransactionReq {
    int64_t connector_id;
    char * id_tag;
    int64_t meter_start;
    int64_t * reservation_id;
    char * timestamp;
};

struct StartTransactionReq * cJSON_ParseStartTransactionReq(const char * s);
char * cJSON_PrintStartTransactionReq(const struct StartTransactionReq * x);

#endif
