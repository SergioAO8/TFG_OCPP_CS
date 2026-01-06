#ifndef _CHANGEAVAILABILITYREQJSON_H_
#define _CHANGEAVAILABILITYREQJSON_H_

#include <cJSON.h>
#include <stdint.h>

enum Type {
    TYPE_INOPERATIVE,
    TYPE_OPERATIVE,
};

struct ChangeAvailabilityReq {
    int64_t connector_id;
    enum Type type;
};

struct ChangeAvailabilityReq * cJSON_ParseChangeAvailabilityReq(const char * s);
char * cJSON_PrintChangeAvailabilityReq(const struct ChangeAvailabilityReq * x);

#endif
