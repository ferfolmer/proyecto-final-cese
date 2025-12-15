#include "system_manager.h"
#include <stdbool.h>
#include <stddef.h>

struct SystemManager {
    ConfigValidatorHandle configValidator;
    SensorDriverHandle sensorDriver;
    OutputDriverHandle outputDriver;
    CalibrationManagerHandle calibrationManager;
    ControlUnitHandle controlUnit;

    SystemState state;
    SystemError lastError;
    bool initialized;
    uint32_t uptime_ms;
};

static struct SystemManager instance;

static bool isValidHandle(SystemManagerHandle handle) {
    return handle != NULL;
}

// ========== Lifecycle ==========

SystemManagerHandle SystemManager_Create(SensorHardwareInterface * sensorHw,
                                         OutputHardwareInterface * outputHw) {
    if (sensorHw == NULL || outputHw == NULL) {
        return NULL;
    }

    instance.state = SYSTEM_STATE_UNINITIALIZED;
    instance.lastError = SYSTEM_OK;
    instance.initialized = false;
    instance.uptime_ms = 0;

    instance.configValidator = ConfigValidator_Create();
    instance.sensorDriver = SensorDriver_Create(sensorHw);
    instance.outputDriver = OutputDriver_Create(outputHw);
    instance.calibrationManager = CalibrationManager_Create(instance.sensorDriver);
    instance.controlUnit =
        ControlUnit_Create(instance.configValidator, instance.sensorDriver, instance.outputDriver);

    return &instance;
}

void SystemManager_Destroy(SystemManagerHandle handle) {
    if (isValidHandle(handle)) {
        handle->state = SYSTEM_STATE_UNINITIALIZED;
        handle->initialized = false;
    }
}

SystemError SystemManager_Init(SystemManagerHandle handle) {
    if (!isValidHandle(handle)) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    // Initialize all subsystems in dependency order
    // Note: We don't check ConfigValidator_Init return (it has no Init function)

    if (SensorDriver_Init(handle->sensorDriver) != SENSOR_OK) {
        handle->lastError = SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED;
        return SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED;
    }

    if (OutputDriver_Init(handle->outputDriver) != OUTPUT_OK) {
        handle->lastError = SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED;
        return SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED;
    }

    if (CalibrationManager_Init(handle->calibrationManager) != CALIBRATION_OK) {
        handle->lastError = SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED;
        return SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED;
    }

    if (ControlUnit_Init(handle->controlUnit) != CONTROL_OK) {
        handle->lastError = SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED;
        return SYSTEM_ERROR_SUBSYSTEM_INIT_FAILED;
    }

    // Initialization successful - transition to IDLE state
    handle->initialized = true;
    handle->state = SYSTEM_STATE_IDLE;
    handle->uptime_ms = 0;

    return SYSTEM_OK;
}

// ========== State Management ==========

SystemError SystemManager_GetState(SystemManagerHandle handle, SystemState * state) {
    if (!isValidHandle(handle) || state == NULL) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    *state = handle->state;
    return SYSTEM_OK;
}

// ========== Subsystem Access ==========

SystemError SystemManager_GetControlUnit(SystemManagerHandle handle,
                                         ControlUnitHandle * controlUnit) {
    if (!isValidHandle(handle) || controlUnit == NULL) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    *controlUnit = handle->controlUnit;
    return SYSTEM_OK;
}

SystemError SystemManager_GetCalibrationManager(SystemManagerHandle handle,
                                                CalibrationManagerHandle * calibrationMgr) {
    if (!isValidHandle(handle) || calibrationMgr == NULL) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    *calibrationMgr = handle->calibrationManager;
    return SYSTEM_OK;
}

SystemError SystemManager_GetConfigValidator(SystemManagerHandle handle,
                                             ConfigValidatorHandle * configValidator) {
    if (!isValidHandle(handle) || configValidator == NULL) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    *configValidator = handle->configValidator;
    return SYSTEM_OK;
}

SystemError SystemManager_GetSensorDriver(SystemManagerHandle handle,
                                          SensorDriverHandle * sensorDriver) {
    if (!isValidHandle(handle) || sensorDriver == NULL) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    *sensorDriver = handle->sensorDriver;
    return SYSTEM_OK;
}

SystemError SystemManager_GetOutputDriver(SystemManagerHandle handle,
                                          OutputDriverHandle * outputDriver) {
    if (!isValidHandle(handle) || outputDriver == NULL) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    *outputDriver = handle->outputDriver;
    return SYSTEM_OK;
}

// Helper: Check if state transition is valid
static bool isValidTransition(SystemState from, SystemState to) {
    // Always allow transition to ERROR (emergency stop)
    if (to == SYSTEM_STATE_ERROR) {
        return true;
    }

    // From UNINITIALIZED, can only go to INITIALIZING (via Init())
    if (from == SYSTEM_STATE_UNINITIALIZED) {
        return false; // SetState cannot be used from UNINITIALIZED
    }

    // From IDLE, can go to MANUAL, AUTOMATIC, or CALIBRATING
    if (from == SYSTEM_STATE_IDLE) {
        return (to == SYSTEM_STATE_MANUAL || to == SYSTEM_STATE_AUTOMATIC ||
                to == SYSTEM_STATE_CALIBRATING);
    }

    // From MANUAL, AUTOMATIC, or CALIBRATING, can only return to IDLE
    if (from == SYSTEM_STATE_MANUAL || from == SYSTEM_STATE_AUTOMATIC ||
        from == SYSTEM_STATE_CALIBRATING) {
        return (to == SYSTEM_STATE_IDLE);
    }

    // From ERROR, cannot use SetState (must use Reset())
    if (from == SYSTEM_STATE_ERROR) {
        return false;
    }

    return false;
}

SystemError SystemManager_SetState(SystemManagerHandle handle, SystemState state) {
    if (!isValidHandle(handle)) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    // Validate transition
    if (!isValidTransition(handle->state, state)) {
        return SYSTEM_ERROR_INVALID_STATE;
    }

    handle->state = state;
    return SYSTEM_OK;
}

SystemError SystemManager_EmergencyStop(SystemManagerHandle handle) {
    if (!isValidHandle(handle)) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    // Transition to ERROR state
    handle->state = SYSTEM_STATE_ERROR;

    // Disable all outputs for safety
    OutputDriver_EmergencyStop(handle->outputDriver);

    return SYSTEM_OK;
}

SystemError SystemManager_Reset(SystemManagerHandle handle) {
    if (!isValidHandle(handle)) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    // Can only reset from ERROR state
    if (handle->state != SYSTEM_STATE_ERROR) {
        return SYSTEM_ERROR_INVALID_STATE;
    }

    // Clear error and return to IDLE
    handle->lastError = SYSTEM_OK;
    handle->state = SYSTEM_STATE_IDLE;

    return SYSTEM_OK;
}

// ========== Main Execution Loop ==========

SystemError SystemManager_Run(SystemManagerHandle handle) {
    if (!isValidHandle(handle)) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    // Execute behavior based on current state
    switch (handle->state) {
    case SYSTEM_STATE_UNINITIALIZED:
    case SYSTEM_STATE_INITIALIZING:
        return SYSTEM_ERROR_NOT_INITIALIZED;

    case SYSTEM_STATE_IDLE:
        // No action - waiting for state transition
        break;

    case SYSTEM_STATE_MANUAL:
        // In manual mode, no automatic control
        // User sets outputs directly via ControlUnit
        break;

    case SYSTEM_STATE_AUTOMATIC:
        // Run automatic control loop
        if (ControlUnit_Update(handle->controlUnit) != CONTROL_OK) {
            handle->lastError = SYSTEM_ERROR_CONTROL_FAULT;
            handle->state = SYSTEM_STATE_ERROR;
            OutputDriver_EmergencyStop(handle->outputDriver);
            return SYSTEM_ERROR_CONTROL_FAULT;
        }
        break;

    case SYSTEM_STATE_CALIBRATING:
        // Calibration procedure placeholder
        break;

    case SYSTEM_STATE_ERROR:
        // In error state, do nothing
        // Must call Reset() to recover
        break;

    default:
        return SYSTEM_ERROR_INVALID_STATE;
    }

    return SYSTEM_OK;
}

// ========== Status and Error Reporting ==========

SystemError SystemManager_GetStatus(SystemManagerHandle handle, SystemStatus * status) {
    if (!isValidHandle(handle) || status == NULL) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    // Populate status structure
    status->state = handle->state;
    status->lastError = handle->lastError;

    // Check if control is running (in AUTOMATIC state)
    status->controlRunning = (handle->state == SYSTEM_STATE_AUTOMATIC);

    // Check calibration validity
    bool calibValid = false;
    CalibrationManager_IsValid(handle->calibrationManager, &calibValid);
    status->calibrationValid = calibValid;

    // Note: uptime_ms removed - timing is caller's responsibility
    status->uptime_ms = 0;

    return SYSTEM_OK;
}

SystemError SystemManager_GetLastError(SystemManagerHandle handle, SystemError * error) {
    if (!isValidHandle(handle) || error == NULL) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    *error = handle->lastError;
    return SYSTEM_OK;
}

SystemError SystemManager_ClearError(SystemManagerHandle handle) {
    if (!isValidHandle(handle)) {
        return SYSTEM_ERROR_NULL_POINTER;
    }

    handle->lastError = SYSTEM_OK;
    return SYSTEM_OK;
}
