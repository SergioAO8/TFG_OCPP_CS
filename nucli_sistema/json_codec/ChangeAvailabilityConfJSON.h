#ifndef _CHANGEAVAILABILITYCONFJSON_H_
#define _CHANGEAVAILABILITYCONFJSON_H_

#include <cJSON.h>

enum Status_Availability {
    STATUS_AVAILABILITY_ACCEPTED,
    STATUS_AVAILABILITY_REJECTED,
    STATUS_AVAILABILITY_SCHEDULED,
};

struct ChangeAvailabilityConf {
    enum Status_Availability status;
};

struct ChangeAvailabilityConf * cJSON_ParseChangeAvailabilityConf(const char * s);
char * cJSON_PrintChangeAvailabilityConf(const struct ChangeAvailabilityConf * x);

#endif
