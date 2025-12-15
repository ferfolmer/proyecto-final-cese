#include "control_unit.h"
#include <stdbool.h>
#include <string.h>

#define PPFD_MAX 4000 // Maximum PPFD value (µmol/m²/s)

struct ControlUnit {
    bool initialized;
    ConfigValidatorHandle config;
    SensorDriverHandle sensor;
    OutputDriverHandle output;

    ControlMode mode;
    ControlState state;
    uint16_t setpoint_umol;
    PIParameters piParams;

    // Controller state
    float outputPercent;
    float lastError;
    float integralTerm;
};

static struct ControlUnit instance;

static bool isValidHandle(ControlUnitHandle handle) {
    return handle != NULL;
}

static bool isValidPointer(const void * ptr) {
    return ptr != NULL;
}

static bool isValidSetpoint(uint16_t ppfd) {
    return ppfd <= PPFD_MAX;
}

// Clamp value to range [min, max]
static float Clamp(float value, float min, float max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

ControlUnitHandle ControlUnit_Create(ConfigValidatorHandle config, SensorDriverHandle sensor,
                                     OutputDriverHandle output) {
    if (config == NULL || sensor == NULL || output == NULL) {
        return NULL;
    }

    memset(&instance, 0, sizeof(instance));
    instance.initialized = false;
    instance.config = config;
    instance.sensor = sensor;
    instance.output = output;

    // Default values
    instance.mode = CONTROL_MODE_MANUAL;
    instance.state = CONTROL_STATE_IDLE;
    instance.setpoint_umol = 0;

    // Default PI parameters (proportional only for Option A)
    instance.piParams.Kp = 0.1f;
    instance.piParams.Ki = 0.0f; // Not used in Option A
    instance.piParams.integralLimit = 100.0f;

    instance.outputPercent = 0.0f;
    instance.lastError = 0.0f;
    instance.integralTerm = 0.0f;

    return &instance;
}

void ControlUnit_Destroy(ControlUnitHandle handle) {
    (void)handle;
}

ControlError ControlUnit_Init(ControlUnitHandle handle) {
    if (!isValidHandle(handle)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    handle->initialized = true;
    handle->state = CONTROL_STATE_IDLE;

    return CONTROL_OK;
}

ControlError ControlUnit_SetMode(ControlUnitHandle handle, ControlMode mode) {
    if (!isValidHandle(handle)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    handle->mode = mode;

    // Reset state when changing modes
    if (mode == CONTROL_MODE_MANUAL) {
        handle->state = CONTROL_STATE_IDLE;
        handle->integralTerm = 0.0f;
    } else if (mode == CONTROL_MODE_AUTOMATIC) {
        handle->state = CONTROL_STATE_STEADY_STATE;
    }

    return CONTROL_OK;
}

ControlError ControlUnit_GetMode(ControlUnitHandle handle, ControlMode * mode) {
    if (!isValidHandle(handle) || !isValidPointer(mode)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    *mode = handle->mode;
    return CONTROL_OK;
}

ControlError ControlUnit_SetSetpoint(ControlUnitHandle handle, uint16_t ppfd_umol) {
    if (!isValidHandle(handle)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    if (!isValidSetpoint(ppfd_umol)) {
        return CONTROL_ERROR_INVALID_SETPOINT;
    }

    handle->setpoint_umol = ppfd_umol;
    return CONTROL_OK;
}

ControlError ControlUnit_GetSetpoint(ControlUnitHandle handle, uint16_t * ppfd_umol) {
    if (!isValidHandle(handle) || !isValidPointer(ppfd_umol)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    *ppfd_umol = handle->setpoint_umol;
    return CONTROL_OK;
}

ControlError ControlUnit_SetPIParameters(ControlUnitHandle handle, PIParameters params) {
    if (!isValidHandle(handle)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    handle->piParams = params;
    return CONTROL_OK;
}

ControlError ControlUnit_GetPIParameters(ControlUnitHandle handle, PIParameters * params) {
    if (!isValidHandle(handle) || !isValidPointer(params)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    *params = handle->piParams;
    return CONTROL_OK;
}

ControlError ControlUnit_SetManualOutput(ControlUnitHandle handle, OutputChannel channel,
                                         float percent) {
    if (!isValidHandle(handle)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    // Set output directly (works in any mode, but mainly for manual)
    OutputDriverError result = OutputDriver_SetChannel(handle->output, channel, percent);

    if (result != OUTPUT_OK) {
        return CONTROL_ERROR_OUTPUT_FAULT;
    }

    // Store output percentage for status monitoring
    if (channel == OUTPUT_CHANNEL_0) {
        handle->outputPercent = percent;
    }

    return CONTROL_OK;
}

ControlError ControlUnit_Update(ControlUnitHandle handle) {
    if (!isValidHandle(handle)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    if (!handle->initialized) {
        return CONTROL_ERROR_NOT_INITIALIZED;
    }

    // Only run control loop in automatic mode
    if (handle->mode != CONTROL_MODE_AUTOMATIC) {
        return CONTROL_OK;
    }

    // Read current PPFD from sensor
    uint16_t measured_ppfd = 0;
    SensorDriverError sensorResult = SensorDriver_ReadPPFD(handle->sensor, &measured_ppfd);

    if (sensorResult != SENSOR_OK) {
        handle->state = CONTROL_STATE_ERROR;
        return CONTROL_ERROR_SENSOR_FAULT;
    }

    // Calculate error: setpoint - measured
    float error = (float)handle->setpoint_umol - (float)measured_ppfd;

    // Proportional control: output = Kp * error
    // (Option A: no integral term yet)
    float newOutput = handle->piParams.Kp * error;

    // Add current output as bias (incremental control)
    newOutput += handle->outputPercent;

    // Clamp output to valid range [0, 100%]
    newOutput = Clamp(newOutput, 0.0f, 100.0f);

    // Apply output to channel 0 (primary channel)
    OutputDriverError outputResult =
        OutputDriver_SetChannel(handle->output, OUTPUT_CHANNEL_0, newOutput);

    if (outputResult != OUTPUT_OK) {
        handle->state = CONTROL_STATE_ERROR;
        return CONTROL_ERROR_OUTPUT_FAULT;
    }

    // Update state
    handle->outputPercent = newOutput;
    handle->lastError = error;
    handle->state = CONTROL_STATE_STEADY_STATE;

    return CONTROL_OK;
}

ControlError ControlUnit_GetStatus(ControlUnitHandle handle, ControlStatus * status) {
    if (!isValidHandle(handle) || !isValidPointer(status)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    // Read current measured value
    uint16_t measured = 0;
    SensorDriver_ReadPPFD(handle->sensor, &measured);

    // Fill status structure
    status->mode = handle->mode;
    status->state = handle->state;
    status->setpoint_umol = handle->setpoint_umol;
    status->measured_umol = measured;
    status->outputPercent = handle->outputPercent;
    status->error = handle->lastError;
    status->integralTerm = handle->integralTerm;

    return CONTROL_OK;
}

ControlError ControlUnit_Reset(ControlUnitHandle handle) {
    if (!isValidHandle(handle)) {
        return CONTROL_ERROR_NULL_POINTER;
    }

    // Reset controller state
    handle->state = CONTROL_STATE_IDLE;
    handle->outputPercent = 0.0f;
    handle->lastError = 0.0f;
    handle->integralTerm = 0.0f;

    // Turn off all outputs
    OutputDriver_SetChannel(handle->output, OUTPUT_CHANNEL_0, 0.0f);

    return CONTROL_OK;
}
