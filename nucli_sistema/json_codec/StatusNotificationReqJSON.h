#ifndef _STATUSNOTIFICATIONREQJSON_H_
#define _STATUSNOTIFICATIONREQJSON_H_

#include <cJSON.h>
#include <stdint.h>

enum ErrorCode {
    ERRORCODE_CONNECTOR_LOCK_FAILURE,
    ERRORCODE_EV_COMMUNICATION_ERROR,
    ERRORCODE_GROUND_FAILURE,
    ERRORCODE_HIGH_TEMPERATURE,
    ERRORCODE_INTERNAL_ERROR,
    ERRORCODE_LOCAL_LIST_CONFLICT,
    ERRORCODE_NO_ERROR,
    ERRORCODE_OTHER_ERROR,
    ERRORCODE_OVER_CURRENT_FAILURE,
    ERRORCODE_OVER_VOLTAGE,
    ERRORCODE_POWER_METER_FAILURE,
    ERRORCODE_POWER_SWITCH_FAILURE,
    ERRORCODE_READER_FAILURE,
    ERRORCODE_RESET_FAILURE,
    ERRORCODE_UNDER_VOLTAGE,
    ERRORCODE_WEAK_SIGNAL,
};

enum Status_Status {
    STATUS_STATUS_AVAILABLE,
    STATUS_STATUS_CHARGING,
    STATUS_STATUS_FAULTED,
    STATUS_STATUS_FINISHING,
    STATUS_STATUS_PREPARING,
    STATUS_STATUS_RESERVED,
    STATUS_STATUS_SUSPENDED_EV,
    STATUS_STATUS_SUSPENDED_EVSE,
    STATUS_STATUS_UNAVAILABLE,
};

struct StatusNotificationReq {
    int64_t connector_id;
    enum ErrorCode error_code;
    char * info;
    enum Status_Status status;
    char * timestamp;
    char * vendor_error_code;
    char * vendor_id;
};

struct StatusNotificationReq * cJSON_ParseStatusNotificationReq(const char * s);
char * cJSON_PrintStatusNotificationReq(const struct StatusNotificationReq * x);

#endif
