#ifndef CONFIG_VALIDATOR_H
#define CONFIG_VALIDATOR_H

#include <stddef.h>
#include <stdint.h>

typedef struct ConfigValidator * ConfigValidatorHandle;

typedef enum { CONFIG_OK = 0 } ConfigValidatorError;

ConfigValidatorHandle ConfigValidator_Create(void);
void ConfigValidator_Destroy(ConfigValidatorHandle handle);
ConfigValidatorError ConfigValidator_GetDLI(ConfigValidatorHandle handle, uint16_t * dli);
ConfigValidatorError ConfigValidator_SetDLI(ConfigValidatorHandle handle, uint16_t dli);
ConfigValidatorError ConfigValidator_GetPhotoperiod(ConfigValidatorHandle handle,
                                                    uint8_t * photoperiod);
ConfigValidatorError ConfigValidator_SetPhotoperiod(ConfigValidatorHandle handle,
                                                    uint8_t photoperiod);
ConfigValidatorError ConfigValidator_GetRiseTime(ConfigValidatorHandle handle, uint8_t * riseTime);
ConfigValidatorError ConfigValidator_SetRiseTime(ConfigValidatorHandle handle, uint8_t riseTime);
#endif
