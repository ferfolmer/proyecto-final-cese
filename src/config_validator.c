#include "config_validator.h"
#include <stdbool.h>

struct ConfigValidator {
    uint16_t dli;
    uint8_t photoperiod;
    uint8_t riseTime;
};

static struct ConfigValidator instance;

static bool isValidHandle(ConfigValidatorHandle handle) {
    return handle != NULL;
}

static bool isValidPointer(const void * ptr) {
    return ptr != NULL;
}

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
    if (!isValidHandle(handle) || !isValidPointer(dli)) {
        return CONFIG_ERROR_NULL_POINTER;
    }
    *dli = handle->dli;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_SetDLI(ConfigValidatorHandle handle, uint16_t dli) {
    if (!isValidHandle(handle)) {
        return CONFIG_ERROR_NULL_POINTER;
    }
    if (dli == 0) {
        return CONFIG_ERROR_INVALID_PARAMETER;
    }
    handle->dli = dli;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_GetPhotoperiod(ConfigValidatorHandle handle,
                                                    uint8_t * photoperiod) {
    if (!isValidHandle(handle) || !isValidPointer(photoperiod)) {
        return CONFIG_ERROR_NULL_POINTER;
    }
    *photoperiod = handle->photoperiod;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_SetPhotoperiod(ConfigValidatorHandle handle,
                                                    uint8_t photoperiod) {
    if (!isValidHandle(handle)) {
        return CONFIG_ERROR_NULL_POINTER;
    }
    if (photoperiod == 0 || photoperiod > 24) {
        return CONFIG_ERROR_INVALID_PARAMETER;
    }
    handle->photoperiod = photoperiod;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_GetRiseTime(ConfigValidatorHandle handle, uint8_t * riseTime) {
    if (!isValidHandle(handle) || !isValidPointer(riseTime)) {
        return CONFIG_ERROR_NULL_POINTER;
    }
    *riseTime = handle->riseTime;
    return CONFIG_OK;
}

ConfigValidatorError ConfigValidator_SetRiseTime(ConfigValidatorHandle handle, uint8_t riseTime) {
    if (!isValidHandle(handle)) {
        return CONFIG_ERROR_NULL_POINTER;
    }
    handle->riseTime = riseTime;
    return CONFIG_OK;
}
