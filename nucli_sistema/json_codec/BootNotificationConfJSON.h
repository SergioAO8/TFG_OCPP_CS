#ifndef _BOOTNOTIFICATIONCONFJSON_H_
#define _BOOTNOTIFICATIONCONFJSON_H_

#include <cJSON.h>
#include <stdint.h>

enum Status_Boot {
    STATUS_BOOT_ACCEPTED,
    STATUS_BOOT_PENDING,
    STATUS_BOOT_REJECTED,
};

struct BootNotificationConf {
    char * current_time;
    int64_t interval;
    enum Status_Boot status;
};

struct BootNotificationConf * cJSON_ParseBootNotificationConf(const char * s);
char * cJSON_PrintBootNotificationConf(const struct BootNotificationConf * x);

#endif
