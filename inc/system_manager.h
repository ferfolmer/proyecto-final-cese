#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "calibration_manager.h"
#include "config_validator.h"
#include "control_unit.h"
#include "output_driver.h"
#include "sensor_driver.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct SystemManager * SystemManagerHandle;

// System states (main state machine)
typedef enum {
    SYSTEM_STATE_UNINITIALIZED = 0, // Initial state before Init()
    SYSTEM_STATE_INITIALIZING,      // Hardware initialization in progress
    SYSTEM_STATE_IDLE,              // Ready, waiting for commands
    SYSTEM_STATE_CALIBRATING,       // Running calibration procedure
    SYSTEM_STATE_MANUAL,            // Manual control mode (user sets outputs)
    SYSTEM_STATE_AUTOMATIC,         // Automatic control mode (closed-loop)
    SYSTEM_STATE_ERROR              // Error condition, requires reset
} SystemState;

// Error codes
typedef enum {
    SYSTEM_OK = 0,
    SYSTEM_ERROR_NULL_POINTER,
    SYSTEM_ERROR_NOT_INITIALIZED,
    SYSTEM_ERROR_INVALID_STATE,         // Requested state transition not allowed
    SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED, // One or more subsystems failed Init()
    SYSTEM_ERROR_CONTROL_FAULT,         // Control unit reported error
    SYSTEM_ERROR_CALIBRATION_FAULT,     // Calibration manager reported error
    SYSTEM_ERROR_SENSOR_FAULT,          // Sensor driver reported error
    SYSTEM_ERROR_OUTPUT_FAULT           // Output driver reported error
} SystemError;

// System status (for monitoring)
typedef struct {
    SystemState state;
    bool controlRunning;   // ControlUnit is actively running
    bool calibrationValid; // Calibration data is valid
    uint32_t uptime_ms;    // Time since Init() in milliseconds
    SystemError lastError; // Last error that occurred
} SystemStatus;

// Lifecycle
// Hardware interfaces are required for creating SensorDriver and OutputDriver
SystemManagerHandle SystemManager_Create(SensorHardwareInterface * sensorHw,
                                         OutputHardwareInterface * outputHw);
void SystemManager_Destroy(SystemManagerHandle handle);
SystemError SystemManager_Init(SystemManagerHandle handle);

// State management
SystemError SystemManager_SetState(SystemManagerHandle handle, SystemState state);
SystemError SystemManager_GetState(SystemManagerHandle handle, SystemState * state);

// Main execution loop (call periodically, e.g., 1 Hz)
// This orchestrates all subsystems based on current state
SystemError SystemManager_Run(SystemManagerHandle handle);

// Status monitoring
SystemError SystemManager_GetStatus(SystemManagerHandle handle, SystemStatus * status);

// Error handling
SystemError SystemManager_GetLastError(SystemManagerHandle handle, SystemError * error);
SystemError SystemManager_ClearError(SystemManagerHandle handle);

// Subsystem access (for integration with Interface Layer)
SystemError SystemManager_GetControlUnit(SystemManagerHandle handle,
                                         ControlUnitHandle * controlUnit);
SystemError SystemManager_GetCalibrationManager(SystemManagerHandle handle,
                                                CalibrationManagerHandle * calibrationMgr);
SystemError SystemManager_GetConfigValidator(SystemManagerHandle handle,
                                             ConfigValidatorHandle * configValidator);
SystemError SystemManager_GetSensorDriver(SystemManagerHandle handle,
                                          SensorDriverHandle * sensorDriver);
SystemError SystemManager_GetOutputDriver(SystemManagerHandle handle,
                                          OutputDriverHandle * outputDriver);

// Emergency stop (transitions to ERROR state, disables all outputs)
SystemError SystemManager_EmergencyStop(SystemManagerHandle handle);

// Reset from ERROR state back to IDLE
SystemError SystemManager_Reset(SystemManagerHandle handle);

#endif // SYSTEM_MANAGER_H
