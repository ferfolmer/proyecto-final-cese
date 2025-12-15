#ifndef CALIBRATION_MANAGER_H
#define CALIBRATION_MANAGER_H

#include "sensor_driver.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct CalibrationManager * CalibrationManagerHandle;

// Error codes
typedef enum {
    CALIBRATION_OK = 0,
    CALIBRATION_ERROR_NULL_POINTER,
    CALIBRATION_ERROR_NOT_INITIALIZED,
    CALIBRATION_ERROR_INVALID_GAIN, // Gain must be > 0
    CALIBRATION_ERROR_DRIVER_ERROR  // Underlying driver error
} CalibrationError;

// Note: CalibrationData is defined in sensor_driver.h
// We add an extended version with validation flag
typedef struct {
    CalibrationData data; // Sensor calibration data (offset, gain, timestamp, crc)
    bool valid;           // Calibration data is valid
} CalibrationInfo;

// Lifecycle
CalibrationManagerHandle CalibrationManager_Create(SensorDriverHandle sensor);
void CalibrationManager_Destroy(CalibrationManagerHandle handle);
CalibrationError CalibrationManager_Init(CalibrationManagerHandle handle);

// Manual calibration (Phase 1)
CalibrationError CalibrationManager_SetCalibration(CalibrationManagerHandle handle,
                                                   float offset_umol, float gain,
                                                   uint32_t timestamp_ms);

CalibrationError CalibrationManager_GetCalibration(CalibrationManagerHandle handle,
                                                   CalibrationInfo * calibration);

// Apply stored calibration to sensor driver
CalibrationError CalibrationManager_ApplyToSensor(CalibrationManagerHandle handle);

// Validation
CalibrationError CalibrationManager_IsValid(CalibrationManagerHandle handle, bool * isValid);

// Clear calibration (reset to defaults)
CalibrationError CalibrationManager_ClearCalibration(CalibrationManagerHandle handle);

// TODO: Phase 2 - Semi-automatic calibration with reference sensor
// CalibrationError CalibrationManager_StartProcedure(CalibrationManagerHandle handle,
//                                                     float outputPercent);
// CalibrationError CalibrationManager_SetReference(CalibrationManagerHandle handle,
//                                                   float referencePPFD);
// CalibrationError CalibrationManager_Calculate(CalibrationManagerHandle handle);

// TODO: Phase 3 - Performance tracking
// CalibrationError CalibrationManager_RecordBaseline(CalibrationManagerHandle handle);
// CalibrationError CalibrationManager_CheckDrift(CalibrationManagerHandle handle,
//                                                 float * driftPercent);

#endif // CALIBRATION_MANAGER_H
