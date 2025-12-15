#include "calibration_manager.h"
#include <stdbool.h>
#include <string.h>

struct CalibrationManager {
    bool initialized;
    SensorDriverHandle sensor;
    CalibrationInfo calibration;
};

static struct CalibrationManager instance;

static bool isValidHandle(CalibrationManagerHandle handle) {
    return handle != NULL;
}

static bool isValidPointer(const void * ptr) {
    return ptr != NULL;
}

static bool isValidGain(float gain) {
    return gain > 0.0f;
}

CalibrationManagerHandle CalibrationManager_Create(SensorDriverHandle sensor) {
    if (sensor == NULL) {
        return NULL;
    }

    memset(&instance, 0, sizeof(instance));
    instance.initialized = false;
    instance.sensor = sensor;

    // Initialize with default calibration (no correction)
    instance.calibration.data.offset_umol = 0.0f;
    instance.calibration.data.gain = 1.0f;
    instance.calibration.data.timestamp_ms = 0;
    instance.calibration.data.crc = 0;
    instance.calibration.valid = false;

    return &instance;
}

void CalibrationManager_Destroy(CalibrationManagerHandle handle) {
    (void)handle;
}

CalibrationError CalibrationManager_Init(CalibrationManagerHandle handle) {
    if (!isValidHandle(handle)) {
        return CALIBRATION_ERROR_NULL_POINTER;
    }

    handle->initialized = true;
    return CALIBRATION_OK;
}

CalibrationError CalibrationManager_SetCalibration(CalibrationManagerHandle handle,
                                                   float offset_umol, float gain,
                                                   uint32_t timestamp_ms) {
    if (!isValidHandle(handle)) {
        return CALIBRATION_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return CALIBRATION_ERROR_NOT_INITIALIZED;
    }
    if (!isValidGain(gain)) {
        return CALIBRATION_ERROR_INVALID_GAIN;
    }

    // Store calibration data
    handle->calibration.data.offset_umol = offset_umol;
    handle->calibration.data.gain = gain;
    handle->calibration.data.timestamp_ms = timestamp_ms;
    handle->calibration.valid = true;

    return CALIBRATION_OK;
}

CalibrationError CalibrationManager_GetCalibration(CalibrationManagerHandle handle,
                                                   CalibrationInfo * calibration) {
    if (!isValidHandle(handle) || !isValidPointer(calibration)) {
        return CALIBRATION_ERROR_NULL_POINTER;
    }

    // Return current calibration data
    *calibration = handle->calibration;

    return CALIBRATION_OK;
}

CalibrationError CalibrationManager_ApplyToSensor(CalibrationManagerHandle handle) {
    if (!isValidHandle(handle)) {
        return CALIBRATION_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return CALIBRATION_ERROR_NOT_INITIALIZED;
    }

    // Apply to sensor driver (use the CalibrationData directly)
    SensorDriverError result =
        SensorDriver_SetCalibration(handle->sensor, handle->calibration.data);

    if (result != SENSOR_OK) {
        return CALIBRATION_ERROR_DRIVER_ERROR;
    }

    return CALIBRATION_OK;
}

CalibrationError CalibrationManager_IsValid(CalibrationManagerHandle handle, bool * isValid) {
    if (!isValidHandle(handle) || !isValidPointer(isValid)) {
        return CALIBRATION_ERROR_NULL_POINTER;
    }

    *isValid = handle->calibration.valid;

    return CALIBRATION_OK;
}

CalibrationError CalibrationManager_ClearCalibration(CalibrationManagerHandle handle) {
    if (!isValidHandle(handle)) {
        return CALIBRATION_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return CALIBRATION_ERROR_NOT_INITIALIZED;
    }

    // Reset to default values (no correction)
    handle->calibration.data.offset_umol = 0.0f;
    handle->calibration.data.gain = 1.0f;
    handle->calibration.data.timestamp_ms = 0;
    handle->calibration.data.crc = 0;
    handle->calibration.valid = false;

    return CALIBRATION_OK;
}

// ========== Future Phase 2: Semi-automatic calibration ==========
// TODO: Implement when reference sensor is available
//
// CalibrationError CalibrationManager_StartProcedure(CalibrationManagerHandle handle,
//                                                     float outputPercent) {
//     // Set LED output to specified level
//     // Store reference output level
//     // Return CALIBRATION_OK
// }
//
// CalibrationError CalibrationManager_SetReference(CalibrationManagerHandle handle,
//                                                   float referencePPFD) {
//     // Store reference PPFD value from external sensor
//     // Return CALIBRATION_OK
// }
//
// CalibrationError CalibrationManager_Calculate(CalibrationManagerHandle handle) {
//     // Read current sensor PPFD
//     // Calculate: gain = referencePPFD / measuredPPFD
//     // Store calculated calibration
//     // Return CALIBRATION_OK
// }

// ========== Future Phase 3: Performance tracking ==========
// TODO: Implement for drift detection
//
// CalibrationError CalibrationManager_RecordBaseline(CalibrationManagerHandle handle) {
//     // Store current PPFD reading as baseline
//     // Save timestamp
//     // Return CALIBRATION_OK
// }
//
// CalibrationError CalibrationManager_CheckDrift(CalibrationManagerHandle handle,
//                                                 float * driftPercent) {
//     // Compare current reading to baseline
//     // Calculate drift percentage
//     // Return drift value
// }
