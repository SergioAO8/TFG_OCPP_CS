#ifndef _REMOTESTARTTRANSACTIONREQSJON_H_
#define _REMOTESTARTTRANSACTIONREQSJON_H_

#include <cJSON.h>
#include <stdint.h>

enum ChargingProfileKind {
    CHARGINGPROFILEKIND_ABSOLUTE,
    CHARGINGPROFILEKIND_RECURRING,
    CHARGINGPROFILEKIND_RELATIVE,
};

enum ChargingProfilePurpose {
    CHARGINGPROFILEPURPOSE_CHARGE_POINT_MAX_PROFILE,
    CHARGINGPROFILEPURPOSE_TX_DEFAULT_PROFILE,
    CHARGINGPROFILEPURPOSE_TX_PROFILE,
};

enum ChargingRateUnit {
    CHARGINGRATEUNIT_A,
    CHARGINGRATEUNIT_W,
};

struct ChargingSchedulePeriod {
    double limit;
    int64_t * number_phases;
    int64_t start_period;
};

struct ChargingSchedule {
    enum ChargingRateUnit charging_rate_unit;
    list_t * charging_schedule_period;
    int64_t * duration;
    double * min_charging_rate;
    char * start_schedule;
};

enum RecurrencyKind {
    RECURRENCYKIND_DAILY,
    RECURRENCYKIND_WEEKLY,
};

struct ChargingProfile {
    int64_t charging_profile_id;
    enum ChargingProfileKind charging_profile_kind;
    enum ChargingProfilePurpose charging_profile_purpose;
    struct ChargingSchedule * charging_schedule;
    enum RecurrencyKind * recurrency_kind;
    int64_t stack_level;
    int64_t * transaction_id;
    char * valid_from;
    char * valid_to;
};

struct RemoteStartTransactionReq {
    struct ChargingProfile * charging_profile;
    int64_t * connector_id;
    char * id_tag;
};

struct ChargingSchedulePeriod * cJSON_ParseChargingSchedulePeriod(const char * s);
char * cJSON_PrintChargingSchedulePeriod(const struct ChargingSchedulePeriod * x);
struct ChargingSchedule * cJSON_ParseChargingSchedule(const char * s);
char * cJSON_PrintChargingSchedule(const struct ChargingSchedule * x);
struct ChargingProfile * cJSON_ParseChargingProfile(const char * s);
char * cJSON_PrintChargingProfile(const struct ChargingProfile * x);
struct RemoteStartTransactionReq * cJSON_ParseRemoteStartTransactionReq(const char * s);
char * cJSON_PrintRemoteStartTransactionReq(const struct RemoteStartTransactionReq * x);

#endif
