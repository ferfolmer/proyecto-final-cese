#include "config_validator.h"

struct ConfigValidator {
    uint16_t dli;
    uint8_t photoperiod;
    uint8_t riseTime;
};

static struct ConfigValidator instance;

ConfigValidatorHandle ConfigValidator_Create(void) {
    instance.dli = 0;
    instance.photoperiod = 0;
    instance.riseTime = 0;
    return &instance;
}

void ConfigValidator_Destroy(ConfigValidatorHandle handle) {
    (void)handle;
}

ConfigValidatorError ConfigValidator_GetDLI(ConfigValidatorHandle handle, uint16_t * dli) {
    if (handle == NULL || dli == NULL) {
        return CONFIG_OK;
    }
    *dli = handle->dli;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_SetDLI(ConfigValidatorHandle handle, uint16_t dli) {
    if (handle == NULL) {
        return CONFIG_OK;
    }
    handle->dli = dli;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_GetPhotoperiod(ConfigValidatorHandle handle,
                                                    uint8_t * photoperiod) {
    if (handle == NULL || photoperiod == NULL) {
        return CONFIG_OK;
    }
    *photoperiod = handle->photoperiod;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_SetPhotoperiod(ConfigValidatorHandle handle,
                                                    uint8_t photoperiod) {
    if (handle == NULL) {
        return CONFIG_OK;
    }
    handle->photoperiod = photoperiod;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_GetRiseTime(ConfigValidatorHandle handle, uint8_t * riseTime) {
    if (handle == NULL || riseTime == NULL) {
        return CONFIG_OK;
    }
    *riseTime = handle->riseTime;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_SetRiseTime(ConfigValidatorHandle handle, uint8_t riseTime) {
    if (handle == NULL) {
        return CONFIG_OK;
    }
    handle->riseTime = riseTime;
    return CONFIG_OK;
}
