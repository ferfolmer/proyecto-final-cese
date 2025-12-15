#ifndef CONTROL_UNIT_H
#define CONTROL_UNIT_H

#include "config_validator.h"
#include "output_driver.h"
#include "sensor_driver.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct ControlUnit * ControlUnitHandle;

// Operating modes
typedef enum {
    CONTROL_MODE_MANUAL = 0,    // Manual output control (no feedback)
    CONTROL_MODE_AUTOMATIC = 1, // Automatic PI control (closed-loop)
    CONTROL_MODE_DISABLED = 2   // All outputs off
} ControlMode;

// Control state
typedef enum {
    CONTROL_STATE_IDLE = 0,     // Not running
    CONTROL_STATE_RAMPING_UP,   // Output increasing to setpoint
    CONTROL_STATE_RAMPING_DOWN, // Output decreasing
    CONTROL_STATE_STEADY_STATE, // At setpoint, maintaining
    CONTROL_STATE_ERROR         // Fault condition
} ControlState;

// Error codes
typedef enum {
    CONTROL_OK = 0,
    CONTROL_ERROR_NULL_POINTER,
    CONTROL_ERROR_NOT_INITIALIZED,
    CONTROL_ERROR_INVALID_SETPOINT, // Setpoint out of range (0-4000)
    CONTROL_ERROR_INVALID_MODE,
    CONTROL_ERROR_SENSOR_FAULT,
    CONTROL_ERROR_OUTPUT_FAULT
} ControlError;

// PI controller parameters
typedef struct {
    float Kp;            // Proportional gain
    float Ki;            // Integral gain
    float integralLimit; // Anti-windup limit
} PIParameters;

// Control status (for monitoring)
typedef struct {
    ControlMode mode;
    ControlState state;
    uint16_t setpoint_umol; // Target PPFD (µmol/m²/s)
    uint16_t measured_umol; // Current measured PPFD
    float outputPercent;    // Current output (0-100%)
    float error;            // Current error (setpoint - measured)
    float integralTerm;     // PI integrator state
} ControlStatus;

// Lifecycle
ControlUnitHandle ControlUnit_Create(ConfigValidatorHandle config, SensorDriverHandle sensor,
                                     OutputDriverHandle output);
void ControlUnit_Destroy(ControlUnitHandle handle);
ControlError ControlUnit_Init(ControlUnitHandle handle);

// Mode control
ControlError ControlUnit_SetMode(ControlUnitHandle handle, ControlMode mode);
ControlError ControlUnit_GetMode(ControlUnitHandle handle, ControlMode * mode);

// Setpoint control
ControlError ControlUnit_SetSetpoint(ControlUnitHandle handle, uint16_t ppfd_umol);
ControlError ControlUnit_GetSetpoint(ControlUnitHandle handle, uint16_t * ppfd_umol);

// PI parameters
ControlError ControlUnit_SetPIParameters(ControlUnitHandle handle, PIParameters params);
ControlError ControlUnit_GetPIParameters(ControlUnitHandle handle, PIParameters * params);

// Manual mode output control
ControlError ControlUnit_SetManualOutput(ControlUnitHandle handle, OutputChannel channel,
                                         float percent);

// Main control loop (call periodically at 1 Hz)
ControlError ControlUnit_Update(ControlUnitHandle handle);

// Status monitoring
ControlError ControlUnit_GetStatus(ControlUnitHandle handle, ControlStatus * status);

// Reset controller (clears integral term, resets state)
ControlError ControlUnit_Reset(ControlUnitHandle handle);

#endif // CONTROL_UNIT_H
