# SensorDriver - Apogee SQ-522 Integration

## Overview
This document describes how the **SensorDriver** HAL module abstracts the Apogee SQ-522 quantum sensor for the PPFD controller system.

---

## Apogee SQ-522 Sensor Specifications

### Physical Characteristics
- **Model**: SQ-522 (Full-spectrum Quantum Sensor)
- **Output**: Modbus RTU (RS-232 or RS-485)
- **Measurement**: PPFD in µmol m⁻² s⁻¹
- **Range**: 0 to 4000 µmol m⁻² s⁻¹ (typ.)
- **Spectral Range**: 389 to 692 nm (±5 nm)
- **Operating Temp**: -40 to 70°C
- **Calibration Uncertainty**: ±5%
- **Measurement Repeatability**: <1%
- **Non-linearity**: <1%

### Electrical Characteristics
- **Supply Voltage**: 5.5 to 24 V DC
- **Current Draw**:
  - RS-232: 37 mA avg
  - RS-485: 37 mA quiescent, 42 mA active
- **Communication**: Modbus RTU
  - Baudrate: 19200 (default, configurable)
  - Parity: Even
  - Data bits: 8
  - Stop bits: 1
  - Slave address: 1 (default, configurable 1-247)

### Key Performance Metrics
- **Calibration Factor**: Custom per sensor, stored in firmware
- **Temperature Response**: -0.11 ± 0.04% per °C
- **Long-term Drift**: <2% per year
- **Field of View**: 180°
- **Directional Response**: ±2% at 45°, ±5% at 75° zenith angle

---

## Sensor Communication Interface

### Modbus Register Map (Relevant for Implementation)

#### Read-Only Registers (Function Code 0x03)

| Register | Type  | Description | Units |
|----------|-------|-------------|-------|
| 0-1      | Float | **Calibrated PPFD output** | µmol m⁻² s⁻¹ |
| 2-3      | Float | Detector millivolts | mV |
| 10-11    | Float | Device status | - |
| 40       | Int16 | Calibrated PPFD (scaled x10) | µmol m⁻² s⁻¹ |

**For our application, we only need register 0-1 (32-bit float PPFD reading).**

### Communication Protocol
```
Modbus RTU over RS-232 or RS-485
- Read Holding Registers (0x03)
- 32-bit IEEE 754 float format
- Big-Endian byte order
```

### Example Modbus Query
```c
// Request to read PPFD (registers 0-1)
uint8_t query[] = {
    0x01,        // Slave address (default)
    0x03,        // Function code (Read Holding Registers)
    0x00, 0x00,  // Starting address (register 0)
    0x00, 0x02,  // Number of registers (2 for 32-bit float)
    0xC4, 0x0B   // CRC16
};

// Expected response (8 bytes)
// [Address][Function][Byte Count][Data MSB][Data][Data][Data LSB][CRC H][CRC L]
```

---

## SensorDriver Abstraction Strategy

### Design Philosophy

The **SensorDriver** module provides a **hardware abstraction layer** that:

1. **Hides Modbus complexity** - Application code doesn't need to know about Modbus registers
2. **Provides calibration support** - Stores offset/gain for sensor calibration
3. **Implements filtering** - Moving average to smooth noisy readings
4. **Handles errors gracefully** - Timeout, CRC errors, sensor disconnection
5. **Enables testability** - Can be mocked for unit testing without hardware

### Architecture Layers

```
┌─────────────────────────────────────────────────────────┐
│           Application (ControlUnit)                      │
│   "Give me the current PPFD reading"                     │
└─────────────────────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────┐
│              SensorDriver (HAL)                          │
│   - ReadPPFD() → applies calibration + filtering        │
│   - ReadRaw() → raw ADC-like value (for debugging)      │
│   - SetCalibration() → offset + gain                    │
└─────────────────────────────────────────────────────────┘
                         │
         ┌───────────────┼───────────────┐
         ▼               ▼               ▼
┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│   Modbus    │  │ Calibration │  │   Filter    │
│   Driver    │  │   Engine    │  │   (MA)      │
│             │  │             │  │             │
│ - Query     │  │ - Offset    │  │ - Buffer    │
│ - Parse     │  │ - Gain      │  │ - Average   │
│ - CRC       │  │ - Formula   │  │ - Window    │
└─────────────┘  └─────────────┘  └─────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│        Hardware (UART + SQ-522 Sensor)                   │
└─────────────────────────────────────────────────────────┘
```

---

## Implementation Details

### 1. Raw Reading Acquisition

**Problem**: The SQ-522 outputs **calibrated PPFD** directly via Modbus, not a raw ADC value.

**Solution**: The sensor's internal "detector millivolts" (registers 2-3) can serve as a pseudo-raw value for:
- Debugging sensor responsiveness
- Verifying sensor is connected
- Monitoring signal strength

```c
// Sensor provides TWO relevant values:
// 1. Calibrated PPFD (registers 0-1) - what we use
// 2. Detector mV (registers 2-3) - for diagnostics

SensorDriverError SensorDriver_ReadRaw(SensorDriverHandle handle, uint16_t * rawValue) {
    // Option A: Read detector millivolts (0-5000 mV typical)
    // Option B: Read PPFD and scale to 12-bit range (0-4095)

    float ppfd_umol = Modbus_ReadFloat(handle->modbus, REG_PPFD);
    *rawValue = (uint16_t)(ppfd_umol * 4095.0f / 4000.0f); // Scale to ADC range

    return SENSOR_OK;
}
```

### 2. Calibration Application

**Problem**: The SQ-522 is **factory-calibrated** (±5% uncertainty). However, we may want to:
- Apply **field calibration** against a reference sensor
- Compensate for **aging drift** (<2% per year)
- Apply **user corrections** for specific conditions

**Calibration Formula**:
```
PPFD_corrected = (PPFD_sensor - offset) * gain
```

**Example Calibration Procedure**:
1. Place SQ-522 and reference sensor side-by-side under stable light
2. Record both readings (e.g., SQ-522 = 502 µmol/m²/s, Reference = 500 µmol/m²/s)
3. Calculate gain: `gain = 500 / 502 = 0.996`
4. Store in CalibrationData struct
5. All future readings multiplied by 0.996

```c
SensorDriverError SensorDriver_ReadPPFD(SensorDriverHandle handle, uint16_t * ppfd) {
    float raw_ppfd = Modbus_ReadFloat(handle->modbus, REG_PPFD);

    // Apply calibration
    float calibrated = (raw_ppfd - handle->calibration.offset_umol)
                       * handle->calibration.gain;

    // Apply filtering (if enabled)
    if (handle->filterSize > 1) {
        calibrated = ApplyMovingAverage(handle, calibrated);
    }

    *ppfd = (uint16_t)calibrated;
    return SENSOR_OK;
}
```

### 3. Moving Average Filter

**Purpose**: Reduce noise in PPFD measurements (especially useful under:
- Pulsed LED lighting (PWM dimming)
- Mains frequency flicker (50/60 Hz)
- Environmental vibration

**Implementation**:
```c
#define MAX_FILTER_SIZE 16

struct SensorDriver {
    bool initialized;
    CalibrationData calibration;
    uint8_t filterSize;      // 1 = no filtering, 2-16 = MA window
    float filterBuffer[MAX_FILTER_SIZE];
    uint8_t filterIndex;
    bool filterFilled;       // Has buffer been filled once?
};

float ApplyMovingAverage(SensorDriverHandle h, float new_value) {
    // Add new value to circular buffer
    h->filterBuffer[h->filterIndex] = new_value;
    h->filterIndex = (h->filterIndex + 1) % h->filterSize;

    if (h->filterIndex == 0) h->filterFilled = true;

    // Calculate average
    float sum = 0;
    uint8_t count = h->filterFilled ? h->filterSize : h->filterIndex;
    for (uint8_t i = 0; i < count; i++) {
        sum += h->filterBuffer[i];
    }

    return sum / count;
}
```

**Filter Performance**:
- **Window = 1**: No filtering (instant response)
- **Window = 5**: Good balance (0.2s @ 1Hz sampling)
- **Window = 10**: Heavy smoothing (1s lag @ 1Hz sampling)

### 4. Error Handling

**Modbus Communication Errors**:
```c
typedef enum {
    SENSOR_OK = 0,
    SENSOR_ERROR_NULL_POINTER,
    SENSOR_ERROR_NOT_INITIALIZED,
    SENSOR_ERROR_ADC_TIMEOUT,        // Modbus timeout (no response)
    SENSOR_ERROR_ADC_OVERRUN,        // Modbus CRC error
    SENSOR_ERROR_CALIBRATION_INVALID // Calibration data corrupt
} SensorDriverError;
```

**Timeout Handling**:
- Modbus query timeout: 500ms (configurable)
- Retry logic: 3 attempts before returning error
- Sensor considered "disconnected" after 3 consecutive failures

```c
bool SensorDriver_IsConnected(SensorDriverHandle h, bool * isConnected) {
    // Attempt to read device status register
    ModbusError err = Modbus_ReadRegister(h->modbus, REG_STATUS, &status);

    *isConnected = (err == MODBUS_OK);
    return SENSOR_OK;
}
```

---

## Testing Strategy

### Unit Testing (Host PC)

**Mock the Modbus Layer**:
```c
// In tests, replace real Modbus with mock
Mock_Modbus_ReadFloat_ExpectAndReturn(REG_PPFD, 500.0f);

SensorDriver_ReadPPFD(sensor, &ppfd);

TEST_ASSERT_EQUAL_UINT16(500, ppfd);
```

**Test Scenarios**:
1. ✅ Basic reading (no calibration, no filter)
2. ✅ Calibration applied correctly (offset + gain)
3. ✅ Moving average filter (fill buffer, verify average)
4. ✅ Timeout handling (mock returns timeout)
5. ✅ Sensor disconnect/reconnect
6. ✅ Invalid calibration data (CRC mismatch)

### Integration Testing (STM32 Hardware)

**With Real SQ-522 Sensor**:
1. Connect sensor via RS-232/RS-485
2. Verify Modbus communication
3. Compare readings to reference sensor
4. Test under varying light levels (0-2000 µmol/m²/s)
5. Verify temperature response (-10 to 40°C)

---

## Configuration Constants

```c
// Modbus communication
#define SENSOR_MODBUS_ADDRESS    1
#define SENSOR_MODBUS_BAUDRATE   19200
#define SENSOR_MODBUS_TIMEOUT_MS 500
#define SENSOR_MODBUS_RETRIES    3

// PPFD sensor range
#define SENSOR_PPFD_MIN          0      // µmol/m²/s
#define SENSOR_PPFD_MAX          4000   // µmol/m²/s
#define SENSOR_PPFD_NOMINAL      2000   // Typical max indoor

// Calibration defaults
#define SENSOR_DEFAULT_OFFSET    0.0f   // µmol/m²/s
#define SENSOR_DEFAULT_GAIN      1.0f   // Unitless

// Filter configuration
#define SENSOR_FILTER_DEFAULT    5      // 5-sample MA
#define SENSOR_FILTER_MAX        16     // Max buffer size
```

---

## Wiring Diagram (STM32 to SQ-522)

### Option A: RS-232 (Simple, Short Distances <20m)
```
STM32F4                    SQ-522 Sensor
---------                  -------------
UART TX  ──────────────────→ White (RX)
UART RX  ←──────────────────  Blue (TX)
GND      ──────────────────→ Black (GND)
+12V     ──────────────────→ Red (Power)
+12V     ──────────────────→ Green (RS-232 select)
Shield   ──────────────────→ Clear (Shield)
```

### Option B: RS-485 (Robust, Long Distances <1000m)
```
STM32F4                    SQ-522 Sensor
---------                  -------------
RS485 A  ──────────────────→ White (RS-485+)
RS485 B  ──────────────────→ Blue (RS-485-)
GND      ──────────────────→ Black (GND)
         ──────────────────→ Green (GND for RS-485)
+12V     ──────────────────→ Red (Power)
Shield   ──────────────────→ Clear (Shield)
```

**Recommendation**: Start with **RS-232** for simplicity, migrate to **RS-485** if cable length >20m or EMI issues.

---

## Current Implementation Status

### ✅ Completed
- Basic module structure (Create/Destroy/Init)
- Calibration data storage (Get/Set)
- Filter size configuration
- Error code definitions
- 20/20 basic tests passing

### ⏳ TODO
1. **Modbus Integration**:
   - Add Modbus RTU driver (or use existing library)
   - Implement register read functions
   - Add CRC16 calculation
   - Handle timeouts and retries

2. **PPFD Calculation**:
   - Implement `ReadPPFD()` with calibration formula
   - Add temperature compensation (optional, -0.11%/°C)

3. **Filtering**:
   - Implement moving average circular buffer
   - Add filter warm-up logic (handle partial buffer)

4. **Error Handling**:
   - Implement `IsConnected()` with Modbus health check
   - Add retry logic for transient errors

5. **Testing**:
   - Add PPFD calculation tests with known calibration values
   - Add filter tests with sequence of values
   - Add Modbus mock integration

---

## Next Steps

### Immediate (Continue SensorDriver):
1. Decide on Modbus library (options below)
2. Implement PPFD reading with mock Modbus
3. Add filtering logic
4. Write tests for calibration + filtering

### Modbus Library Options:
- **Option A**: Use existing STM32 Modbus library (FreeModbus, libmodbus)
- **Option B**: Write minimal Modbus RTU implementation (just Read Holding Registers)
- **Option C**: Mock Modbus interface for now, implement later

### Alternative (Move to OutputDriver):
- Complete OutputDriver module (similar complexity)
- Return to SensorDriver Modbus integration later
- Allows parallel progress on both HAL modules

---

**Document Status**: Living document, will be updated as implementation progresses
**Last Updated**: 2025-11-03
**Related Documents**:
- `ARCHITECTURE_LAYERS.md` - System architecture overview
- `MODULE_INTERFACES.md` - API specifications
- `SQ-522-manual.pdf` - Sensor datasheet
