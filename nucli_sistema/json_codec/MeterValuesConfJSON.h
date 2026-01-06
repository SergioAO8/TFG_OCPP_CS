#ifndef _METERVALUESCONFJSON_H_
#define _METERVALUESCONFJSON_H_

#include <cJSON.h>

struct MeterValuesConf {
};

struct MeterValuesConf * cJSON_ParseMeterValuesConf(const char * s);
char * cJSON_PrintMeterValuesConf(const struct MeterValuesConf * x);

#endif
