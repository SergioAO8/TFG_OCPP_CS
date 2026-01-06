#ifndef _BOOTNOTIFICATIONREQJSON_H_
#define _BOOTNOTIFICATIONREQJSON_H_

#include <cJSON.h>

struct BootNotificationReq {
    char * charge_box_serial_number;
    char * charge_point_model;
    char * charge_point_serial_number;
    char * charge_point_vendor;
    char * firmware_version;
    char * iccid;
    char * imsi;
    char * meter_serial_number;
    char * meter_type;
};

struct BootNotificationReq * cJSON_ParseBootNotificationReq(const char * s);
char * cJSON_PrintBootNotificationReq(const struct BootNotificationReq * x);

#endif
