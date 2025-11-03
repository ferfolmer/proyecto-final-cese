# PPFD Controller - Module Interface Proposals

This document defines the **public interfaces** (APIs) for each major software module in the system.

---

## 1. ConfigValidator Module

**File**: `inc/config_validator.h`
**Status**: ✅ IMPLEMENTED (partial - needs extensions)
**Purpose**: Validate and store system configuration parameters

### Current Interface

```c
typedef struct ConfigValidator * ConfigValidatorHandle;

typedef enum {
    CONFIG_OK = 0,
    CONFIG_ERROR_NULL_POINTER,
    CONFIG_ERROR_INVALID_PARAMETER,    // TODO: Add this
    CONFIG_ERROR_DLI_NOT_ACHIEVABLE    // TODO: Add this
} ConfigValidatorError;

// Lifecycle
ConfigValidatorHandle ConfigValidator_Create(void);
void ConfigValidator_Destroy(ConfigValidatorHandle handle);

// DLI configuration
ConfigValidatorError ConfigValidator_SetDLI(ConfigValidatorHandle handle, uint16_t dli);
ConfigValidatorError ConfigValidator_GetDLI(ConfigValidatorHandle handle, uint16_t * dli);

// Photoperiod configuration
ConfigValidatorError ConfigValidator_SetPhotoperiod(ConfigValidatorHandle handle, uint8_t photoperiod);
ConfigValidatorError ConfigValidator_GetPhotoperiod(ConfigValidatorHandle handle, uint8_t * photoperiod);

// Rise time configuration
ConfigValidatorError ConfigValidator_SetRiseTime(ConfigValidatorHandle handle, uint8_t riseTime);
ConfigValidatorError ConfigValidator_GetRiseTime(ConfigValidatorHandle handle, uint8_t * riseTime);

// Fall time configuration (TODO: Add these)
ConfigValidatorError ConfigValidator_SetFallTime(ConfigValidatorHandle handle, uint8_t fallTime);
ConfigValidatorError ConfigValidator_GetFallTime(ConfigValidatorHandle handle, uint8_t * fallTime);
```

### Proposed Extensions

```c
// Spectral ratio configuration
typedef struct {
    uint8_t red_percent;   // 0-100%
    uint8_t blue_percent;  // 0-100%
} SpectralRatio;

ConfigValidatorError ConfigValidator_SetSpectralRatio(ConfigValidatorHandle handle, SpectralRatio ratio);
ConfigValidatorError ConfigValidator_GetSpectralRatio(ConfigValidatorHandle handle, SpectralRatio * ratio);

// DLI achievability validation
ConfigValidatorError ConfigValidator_ValidateConfiguration(ConfigValidatorHandle handle, bool * isAchievable);
ConfigValidatorError ConfigValidator_GetRequiredPPFD(ConfigValidatorHandle handle, uint16_t * ppfd);

// Suggestion engine
ConfigValidatorError ConfigValidator_GetMaxAchievableDLI(ConfigValidatorHandle handle, uint16_t * maxDLI);
ConfigValidatorError ConfigValidator_GetMinPhotoperiod(ConfigValidatorHandle handle, uint16_t targetDLI, uint8_t * minPhotoperiod);

// Persistence
ConfigValidatorError ConfigValidator_SaveToNVM(ConfigValidatorHandle handle);
ConfigValidatorError ConfigValidator_LoadFromNVM(ConfigValidatorHandle handle);
ConfigValidatorError ConfigValidator_ResetToDefaults(ConfigValidatorHandle handle);
```

---

## 2. SensorDriver Module

**File**: `inc/sensor_driver.h` (TO BE CREATED)
**Purpose**: Abstract PPFD sensor hardware (ADC acquisition + calibration)

```c
#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct SensorDriver * SensorDriverHandle;

typedef enum {
    SENSOR_OK = 0,
    SENSOR_ERROR_NULL_POINTER,
    SENSOR_ERROR_NOT_INITIALIZED,
    SENSOR_ERROR_ADC_TIMEOUT,
    SENSOR_ERROR_ADC_OVERRUN,
    SENSOR_ERROR_CALIBRATION_INVALID
} SensorDriverError;

typedef struct {
    float offset_umol;     // Offset in µmol/m²·s
    float gain;            // Gain multiplier (unitless)
    uint32_t timestamp_ms; // When calibration was performed
    uint16_t crc;          // Data integrity check
} CalibrationData;

// Lifecycle
SensorDriverHandle SensorDriver_Create(void);
void SensorDriver_Destroy(SensorDriverHandle handle);
SensorDriverError SensorDriver_Init(SensorDriverHandle handle);

// Raw ADC reading (0-4095 for 12-bit ADC)
SensorDriverError SensorDriver_ReadRaw(SensorDriverHandle handle, uint16_t * rawValue);

// Calibrated PPFD reading (µmol/m²·s)
SensorDriverError SensorDriver_ReadPPFD(SensorDriverHandle handle, uint16_t * ppfd);

// Calibration management
SensorDriverError SensorDriver_SetCalibration(SensorDriverHandle handle, CalibrationData calibration);
SensorDriverError SensorDriver_GetCalibration(SensorDriverHandle handle, CalibrationData * calibration);

// Filtering configuration
SensorDriverError SensorDriver_SetFilterSize(SensorDriverHandle handle, uint8_t numSamples);
SensorDriverError SensorDriver_GetFilterSize(SensorDriverHandle handle, uint8_t * numSamples);

// Status
SensorDriverError SensorDriver_IsConnected(SensorDriverHandle handle, bool * isConnected);

#endif // SENSOR_DRIVER_H
```

---

## 3. OutputDriver Module

**File**: `inc/output_driver.h` (TO BE CREATED)
**Purpose**: Control 0-10V output channels for LED drivers

```c
#ifndef OUTPUT_DRIVER_H
#define OUTPUT_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct OutputDriver * OutputDriverHandle;

typedef enum {
    OUTPUT_OK = 0,
    OUTPUT_ERROR_NULL_POINTER,
    OUTPUT_ERROR_INVALID_CHANNEL,
    OUTPUT_ERROR_INVALID_VALUE,
    OUTPUT_ERROR_NOT_INITIALIZED,
    OUTPUT_ERROR_HARDWARE_FAULT
} OutputDriverError;

typedef enum {
    OUTPUT_CHANNEL_RED = 0,
    OUTPUT_CHANNEL_BLUE = 1,
    OUTPUT_CHANNEL_MAX = 2  // Extend for more channels
} OutputChannel;

// Lifecycle
OutputDriverHandle OutputDriver_Create(void);
void OutputDriver_Destroy(OutputDriverHandle handle);
OutputDriverError OutputDriver_Init(OutputDriverHandle handle);

// Channel control (percentage: 0-100%)
OutputDriverError OutputDriver_SetChannel(OutputDriverHandle handle, OutputChannel channel, uint8_t percent);
OutputDriverError OutputDriver_GetChannel(OutputDriverHandle handle, OutputChannel channel, uint8_t * percent);

// Voltage control (millivolts: 0-10000 mV)
OutputDriverError OutputDriver_SetChannelVoltage(OutputDriverHandle handle, OutputChannel channel, uint16_t millivolts);
OutputDriverError OutputDriver_GetChannelVoltage(OutputDriverHandle handle, OutputChannel channel, uint16_t * millivolts);

// Enable/disable individual channels
OutputDriverError OutputDriver_EnableChannel(OutputDriverHandle handle, OutputChannel channel);
OutputDriverError OutputDriver_DisableChannel(OutputDriverHandle handle, OutputChannel channel);
OutputDriverError OutputDriver_IsChannelEnabled(OutputDriverHandle handle, OutputChannel channel, bool * isEnabled);

// Safety limits
OutputDriverError OutputDriver_SetChannelLimit(OutputDriverHandle handle, OutputChannel channel, uint8_t maxPercent);
OutputDriverError OutputDriver_GetChannelLimit(OutputDriverHandle handle, OutputChannel channel, uint8_t * maxPercent);

// Emergency shutdown (all channels OFF)
OutputDriverError OutputDriver_EmergencyStop(OutputDriverHandle handle);

#endif // OUTPUT_DRIVER_H
```

---

## 4. ControlUnit Module

**File**: `inc/control_unit.h` (TO BE CREATED)
**Purpose**: Implement closed-loop PPFD regulation (PI/PID controller)

```c
#ifndef CONTROL_UNIT_H
#define CONTROL_UNIT_H

#include <stdint.h>
#include <stdbool.h>
#include "config_validator.h"
#include "sensor_driver.h"
#include "output_driver.h"

typedef struct ControlUnit * ControlUnitHandle;

typedef enum {
    CONTROL_OK = 0,
    CONTROL_ERROR_NULL_POINTER,
    CONTROL_ERROR_NOT_INITIALIZED,
    CONTROL_ERROR_INVALID_SETPOINT,
    CONTROL_ERROR_SENSOR_FAULT,
    CONTROL_ERROR_ACTUATOR_FAULT
} ControlUnitError;

typedef enum {
    CONTROL_MODE_DISABLED = 0,
    CONTROL_MODE_MANUAL,        // User sets output directly
    CONTROL_MODE_AUTOMATIC      // Closed-loop control active
} ControlMode;

typedef struct {
    float Kp;           // Proportional gain
    float Ki;           // Integral gain
    float Kd;           // Derivative gain (if PID)
    float integralMax;  // Anti-windup limit
    float integralMin;  // Anti-windup limit
} ControlGains;

typedef struct {
    uint16_t setpoint_ppfd;     // Target PPFD (µmol/m²·s)
    uint16_t measured_ppfd;     // Current PPFD
    float error;                // Setpoint - Measured
    float integral;             // Accumulated error
    float output_percent;       // Control output (0-100%)
    ControlMode mode;           // Current mode
    uint32_t timestamp_ms;      // Last update time
} ControlState;

// Lifecycle
ControlUnitHandle ControlUnit_Create(void);
void ControlUnit_Destroy(ControlUnitHandle handle);
ControlUnitError ControlUnit_Init(ControlUnitHandle handle,
                                   SensorDriverHandle sensor,
                                   OutputDriverHandle output,
                                   ConfigValidatorHandle config);

// Setpoint configuration
ControlUnitError ControlUnit_SetSetpoint(ControlUnitHandle handle, uint16_t ppfd);
ControlUnitError ControlUnit_GetSetpoint(ControlUnitHandle handle, uint16_t * ppfd);

// Controller tuning
ControlUnitError ControlUnit_SetGains(ControlUnitHandle handle, ControlGains gains);
ControlUnitError ControlUnit_GetGains(ControlUnitHandle handle, ControlGains * gains);

// Mode control
ControlUnitError ControlUnit_SetMode(ControlUnitHandle handle, ControlMode mode);
ControlUnitError ControlUnit_GetMode(ControlUnitHandle handle, ControlMode * mode);

// Main control loop (call at 1 Hz per requirements)
ControlUnitError ControlUnit_Update(ControlUnitHandle handle);

// Manual mode (bypass controller)
ControlUnitError ControlUnit_SetManualOutput(ControlUnitHandle handle, uint8_t percent);

// Status and diagnostics
ControlUnitError ControlUnit_GetState(ControlUnitHandle handle, ControlState * state);
ControlUnitError ControlUnit_GetError(ControlUnitHandle handle, float * error);
ControlUnitError ControlUnit_ResetIntegral(ControlUnitHandle handle);

#endif // CONTROL_UNIT_H
```

---

## 5. CalibrationManager Module

**File**: `inc/calibration.h` (TO BE CREATED)
**Purpose**: Auto-calibration procedure (Use Case #4)

```c
#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor_driver.h"
#include "output_driver.h"

typedef struct CalibrationManager * CalibrationManagerHandle;

typedef enum {
    CALIBRATION_OK = 0,
    CALIBRATION_ERROR_NULL_POINTER,
    CALIBRATION_ERROR_LIGHTS_ON,           // Can't calibrate with lights ON
    CALIBRATION_ERROR_REFERENCE_LAMP_FAIL, // Reference lamp didn't turn on
    CALIBRATION_ERROR_UNSTABLE_READING,    // Sensor readings too noisy
    CALIBRATION_ERROR_OUT_OF_RANGE         // Calculated gain/offset invalid
} CalibrationError;

typedef enum {
    CALIBRATION_STATE_IDLE = 0,
    CALIBRATION_STATE_MEASURING_OFFSET,    // Lights OFF
    CALIBRATION_STATE_MEASURING_GAIN,      // Reference lamp ON
    CALIBRATION_STATE_COMPLETE,
    CALIBRATION_STATE_FAILED
} CalibrationState;

// Lifecycle
CalibrationManagerHandle Calibration_Create(void);
void Calibration_Destroy(CalibrationManagerHandle handle);
CalibrationError Calibration_Init(CalibrationManagerHandle handle,
                                   SensorDriverHandle sensor,
                                   OutputDriverHandle output);

// Run auto-calibration procedure (blocking)
CalibrationError Calibration_Run(CalibrationManagerHandle handle);

// Get calibration results
CalibrationError Calibration_GetResult(CalibrationManagerHandle handle, CalibrationData * data);

// State machine (for non-blocking implementation)
CalibrationError Calibration_Start(CalibrationManagerHandle handle);
CalibrationError Calibration_Update(CalibrationManagerHandle handle);
CalibrationError Calibration_GetState(CalibrationManagerHandle handle, CalibrationState * state);
CalibrationError Calibration_Abort(CalibrationManagerHandle handle);

// Reference lamp control
CalibrationError Calibration_SetReferencePPFD(CalibrationManagerHandle handle, uint16_t knownPPFD);
CalibrationError Calibration_GetReferencePPFD(CalibrationManagerHandle handle, uint16_t * knownPPFD);

#endif // CALIBRATION_H
```

---

## 6. UserInterface Module

**File**: `inc/user_interface.h` (TO BE CREATED)
**Purpose**: Handle UART commands and user interaction

```c
#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct UserInterface * UserInterfaceHandle;

typedef enum {
    UI_OK = 0,
    UI_ERROR_NULL_POINTER,
    UI_ERROR_INVALID_COMMAND,
    UI_ERROR_BUFFER_OVERFLOW,
    UI_ERROR_UART_TIMEOUT
} UserInterfaceError;

typedef enum {
    CMD_SET_DLI,
    CMD_GET_DLI,
    CMD_SET_PHOTOPERIOD,
    CMD_GET_PHOTOPERIOD,
    CMD_SET_RATIO,
    CMD_GET_RATIO,
    CMD_ENABLE_CONTROL,
    CMD_DISABLE_CONTROL,
    CMD_CALIBRATE,
    CMD_GET_STATUS,
    CMD_RESET,
    CMD_HELP,
    CMD_UNKNOWN
} CommandType;

typedef struct {
    CommandType type;
    uint16_t arg1;
    uint16_t arg2;
    char buffer[64];
} Command;

// Lifecycle
UserInterfaceHandle UserInterface_Create(void);
void UserInterface_Destroy(UserInterfaceHandle handle);
UserInterfaceError UserInterface_Init(UserInterfaceHandle handle);

// Command processing
UserInterfaceError UserInterface_ProcessInput(UserInterfaceHandle handle, const char * input, Command * cmd);
UserInterfaceError UserInterface_SendResponse(UserInterfaceHandle handle, const char * response);

// Status display
UserInterfaceError UserInterface_DisplayPPFD(UserInterfaceHandle handle, uint16_t measured, uint16_t setpoint);
UserInterfaceError UserInterface_DisplayStatus(UserInterfaceHandle handle, const char * status);
UserInterfaceError UserInterface_DisplayError(UserInterfaceHandle handle, const char * error);

// Non-blocking I/O
UserInterfaceError UserInterface_Update(UserInterfaceHandle handle);
UserInterfaceError UserInterface_HasCommand(UserInterfaceHandle handle, bool * hasCommand);
UserInterfaceError UserInterface_GetCommand(UserInterfaceHandle handle, Command * cmd);

#endif // USER_INTERFACE_H
```

---

## 7. CommunicationInterface Module

**File**: `inc/comm_interface.h` (TO BE CREATED)
**Purpose**: Implement Cannfeel "Puerto G" protocol

```c
#ifndef COMM_INTERFACE_H
#define COMM_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct CommInterface * CommInterfaceHandle;

typedef enum {
    COMM_OK = 0,
    COMM_ERROR_NULL_POINTER,
    COMM_ERROR_NOT_INITIALIZED,
    COMM_ERROR_TIMEOUT,
    COMM_ERROR_CRC_MISMATCH,
    COMM_ERROR_INVALID_PACKET,
    COMM_ERROR_BUFFER_FULL
} CommInterfaceError;

typedef struct {
    uint16_t ppfd_measured;
    uint16_t ppfd_setpoint;
    uint8_t control_enabled;
    uint8_t alarm_flags;
    uint32_t uptime_seconds;
} TelemetryPacket;

typedef struct {
    uint8_t command_id;
    uint16_t param1;
    uint16_t param2;
    uint8_t checksum;
} RemoteCommand;

// Lifecycle
CommInterfaceHandle CommInterface_Create(void);
void CommInterface_Destroy(CommInterfaceHandle handle);
CommInterfaceError CommInterface_Init(CommInterfaceHandle handle);

// Telemetry transmission
CommInterfaceError CommInterface_SendTelemetry(CommInterfaceHandle handle, TelemetryPacket * packet);
CommInterfaceError CommInterface_SendAlarm(CommInterfaceHandle handle, uint8_t alarmCode);

// Remote command reception
CommInterfaceError CommInterface_ReceiveCommand(CommInterfaceHandle handle, RemoteCommand * cmd);
CommInterfaceError CommInterface_HasCommand(CommInterfaceHandle handle, bool * hasCommand);

// Protocol configuration
CommInterfaceError CommInterface_SetAddress(CommInterfaceHandle handle, uint8_t address);
CommInterfaceError CommInterface_GetAddress(CommInterfaceHandle handle, uint8_t * address);

// Non-blocking update
CommInterfaceError CommInterface_Update(CommInterfaceHandle handle);

#endif // COMM_INTERFACE_H
```

---

## 8. SystemManager Module

**File**: `inc/system_manager.h` (TO BE CREATED)
**Purpose**: Top-level system coordinator and state machine

```c
#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct SystemManager * SystemManagerHandle;

typedef enum {
    SYSTEM_OK = 0,
    SYSTEM_ERROR_NULL_POINTER,
    SYSTEM_ERROR_INIT_FAILED,
    SYSTEM_ERROR_INVALID_STATE
} SystemManagerError;

typedef enum {
    SYSTEM_STATE_INIT,           // Power-on initialization
    SYSTEM_STATE_IDLE,           // Waiting for commands
    SYSTEM_STATE_CALIBRATING,    // Running auto-calibration
    SYSTEM_STATE_READ_ONLY,      // Monitoring mode (no control)
    SYSTEM_STATE_MANUAL,         // Manual output control
    SYSTEM_STATE_AUTOMATIC,      // Closed-loop control active
    SYSTEM_STATE_ERROR           // Fault condition
} SystemState;

typedef struct {
    SystemState state;
    uint32_t uptime_seconds;
    uint16_t ppfd_measured;
    uint16_t ppfd_setpoint;
    uint8_t alarm_flags;
    bool control_enabled;
} SystemStatus;

// Lifecycle
SystemManagerHandle SystemManager_Create(void);
void SystemManager_Destroy(SystemManagerHandle handle);
SystemManagerError SystemManager_Init(SystemManagerHandle handle);

// Main loop (call in while(1))
SystemManagerError SystemManager_Run(SystemManagerHandle handle);

// State transitions
SystemManagerError SystemManager_SetState(SystemManagerHandle handle, SystemState newState);
SystemManagerError SystemManager_GetState(SystemManagerHandle handle, SystemState * state);

// Status
SystemManagerError SystemManager_GetStatus(SystemManagerHandle handle, SystemStatus * status);

// Command interface (from UI or remote)
SystemManagerError SystemManager_ExecuteCommand(SystemManagerHandle handle, uint8_t commandId, uint16_t param1, uint16_t param2);

// Error handling
SystemManagerError SystemManager_GetLastError(SystemManagerHandle handle, uint8_t * errorCode);
SystemManagerError SystemManager_ClearError(SystemManagerHandle handle);

#endif // SYSTEM_MANAGER_H
```

---

## Module Dependencies

```
SystemManager
    ├── ConfigValidator
    ├── ControlUnit
    │   ├── SensorDriver
    │   ├── OutputDriver
    │   └── ConfigValidator
    ├── CalibrationManager
    │   ├── SensorDriver
    │   └── OutputDriver
    ├── UserInterface
    └── CommInterface

Legend:
├── Direct dependency (module uses this)
```

---

## Compilation Order (Bottom-Up)

1. **ConfigValidator** (no dependencies)
2. **SensorDriver** (no dependencies)
3. **OutputDriver** (no dependencies)
4. **CalibrationManager** (depends on SensorDriver, OutputDriver)
5. **ControlUnit** (depends on SensorDriver, OutputDriver, ConfigValidator)
6. **UserInterface** (no dependencies)
7. **CommInterface** (no dependencies)
8. **SystemManager** (depends on ALL above)

---

## Testing Strategy

Each module should have:
- **Unit tests** (using Unity, with mocks for dependencies via CMock)
- **Integration tests** (test module interactions)
- **Hardware-in-the-loop tests** (for HAL modules)

Example test file structure:
```
test/
├── test_config_validator.c       (✅ exists)
├── test_sensor_driver.c          (TODO)
├── test_output_driver.c          (TODO)
├── test_calibration.c            (TODO)
├── test_control_unit.c           (TODO)
├── test_user_interface.c         (TODO)
├── test_comm_interface.c         (TODO)
└── test_system_manager.c         (TODO)
```

---

**Last Updated**: 2025-10-27
**Status**: Only ConfigValidator interface exists
