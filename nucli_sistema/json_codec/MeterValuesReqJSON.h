#ifndef _METERVALUESREQJSON_H_
#define _METERVALUESREQJSON_H_

#include <cJSON.h>

enum Context {
    CONTEXT_INTERRUPTION_BEGIN,
    CONTEXT_INTERRUPTION_END,
    CONTEXT_OTHER,
    CONTEXT_SAMPLE_CLOCK,
    CONTEXT_SAMPLE_PERIODIC,
    CONTEXT_TRANSACTION_BEGIN,
    CONTEXT_TRANSACTION_END,
    CONTEXT_TRIGGER,
};

enum Format {
    FORMAT_RAW,
    FORMAT_SIGNED_DATA,
};

enum Location {
    LOCATION_BODY,
    LOCATION_CABLE,
    LOCATION_EV,
    LOCATION_INLET,
    LOCATION_OUTLET,
};

enum Measurand {
    MEASURAND_CURRENT_EXPORT,
    MEASURAND_CURRENT_IMPORT,
    MEASURAND_CURRENT_OFFERED,
    MEASURAND_ENERGY_ACTIVE_EXPORT_INTERVAL,
    MEASURAND_ENERGY_ACTIVE_EXPORT_REGISTER,
    MEASURAND_ENERGY_ACTIVE_IMPORT_INTERVAL,
    MEASURAND_ENERGY_ACTIVE_IMPORT_REGISTER,
    MEASURAND_ENERGY_REACTIVE_EXPORT_INTERVAL,
    MEASURAND_ENERGY_REACTIVE_EXPORT_REGISTER,
    MEASURAND_ENERGY_REACTIVE_IMPORT_INTERVAL,
    MEASURAND_ENERGY_REACTIVE_IMPORT_REGISTER,
    MEASURAND_FREQUENCY,
    MEASURAND_POWER_ACTIVE_EXPORT,
    MEASURAND_POWER_ACTIVE_IMPORT,
    MEASURAND_POWER_FACTOR,
    MEASURAND_POWER_OFFERED,
    MEASURAND_POWER_REACTIVE_EXPORT,
    MEASURAND_POWER_REACTIVE_IMPORT,
    MEASURAND_RPM,
    MEASURAND_SO_C,
    MEASURAND_TEMPERATURE,
    MEASURAND_VOLTAGE,
};

enum Phase {
    PHASE_L1,
    PHASE_L1_L2,
    PHASE_L1_N,
    PHASE_L2,
    PHASE_L2_L3,
    PHASE_L2_N,
    PHASE_L3,
    PHASE_L3_L1,
    PHASE_L3_N,
    PHASE_N,
};

enum Unit {
    UNIT_A,
    UNIT_CELCIUS,
    UNIT_CELSIUS,
    UNIT_FAHRENHEIT,
    UNIT_K,
    UNIT_KVAR,
    UNIT_KVARH,
    UNIT_K_VA,
    UNIT_K_W,
    UNIT_K_WH,
    UNIT_PERCENT,
    UNIT_V,
    UNIT_VA,
    UNIT_VAR,
    UNIT_VARH,
    UNIT_W,
    UNIT_WH,
};

struct SampledValue {
    enum Context * context;
    enum Format * format;
    enum Location * location;
    enum Measurand * measurand;
    enum Phase * phase;
    enum Unit * unit;
    char * value;
};

struct MeterValue {
    list_t * sampled_value;
    char * timestamp;
};

struct MeterValuesReq {
    int64_t connector_id;
    list_t * meter_value;
    int64_t * transaction_id;
};

struct SampledValue * cJSON_ParseSampledValue(const char * s);
struct MeterValue * cJSON_ParseMeterValue(const char * s);

struct MeterValuesReq * cJSON_ParseMeterValuesReq(const char * s);
char * cJSON_PrintMeterValuesReq(const struct MeterValuesReq * x);

#endif
