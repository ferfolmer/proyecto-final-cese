# PPFD Controller - Development Roadmap

## Project Overview
Multi-channel PPFD (Photosynthetic Photon Flux Density) controller for indoor agriculture with DLI-based configuration and closed-loop control.

**Target**: STM32 Cortex-M microcontroller
**Methodology**: Test-Driven Development (TDD) with Ceedling/Unity/CMock
**Timeline**: ~628 hours over 14 months (Apr 2025 - Jun 2026)

---

## 📋 Development Phases

### **PHASE 1: ConfigValidator Module** ✅ COMPLETE
**Goal**: Complete configuration validation and storage layer
**Status**: 35/35 tests passing (100% - Base functionality complete)
**Priority**: HIGH - Foundation for all other modules

#### Completed ✅
- [x] Module lifecycle (Create/Destroy)
- [x] Initial state validation (DLI=0)
- [x] Basic DLI get/set with null checks
- [x] Basic Photoperiod get/set
- [x] Basic RiseTime get/set
- [x] Basic FallTime get/set

#### Phase 1A: Parameter Validation ✅ COMPLETE
- [x] **Test**: Reject DLI = 0 (invalid)
- [x] **Test**: Accept DLI in range 1-65535 mol/m²/day
- [x] **Test**: Reject photoperiod = 0 (invalid)
- [x] **Test**: Accept photoperiod 1-24 hours
- [x] **Test**: Reject photoperiod > 24 (invalid)
- [x] **Test**: Reject rise time < 30 min (inrush current risk)
- [x] **Test**: Accept rise time 30-90 min
- [x] **Test**: Reject rise time > 90 min (invalid)
- [x] **Code**: Add `CONFIG_ERROR_INVALID_PARAMETER` to enum
- [x] **Code**: Implement range checking in Set functions with `isInRange()` helper

#### Phase 1B: Fall Time Support ✅ COMPLETE
- [x] **Test**: FallTime has initial state = 0
- [x] **Test**: Accept valid fall time 30-90 min
- [x] **Test**: Reject fall time < 30 min
- [x] **Test**: Reject fall time > 90 min
- [x] **Test**: Null pointer checks for GetFallTime/SetFallTime
- [x] **Test**: FallTime independent from RiseTime
- [x] **Code**: Add `uint8_t fallTime` to struct
- [x] **Code**: Implement `ConfigValidator_GetFallTime()`
- [x] **Code**: Implement `ConfigValidator_SetFallTime()`
- [x] **Code**: Add constants `FALL_TIME_MIN` and `FALL_TIME_MAX`

#### Refactoring Completed ✅
- [x] Symbolic constants for all validation ranges
- [x] Helper function `isInRange()` for validation
- [x] setUp/tearDown in tests to eliminate repetitive code
- [x] All tests use shared validator instance

#### TODO - Phase 1C: Spectral Configuration
- [ ] **Test**: Spectral ratio initializes to 50:50 (or 0:0?)
- [ ] **Test**: Accept spectral ratio where sum = 100%
- [ ] **Test**: Reject spectral ratio where sum ≠ 100%
- [ ] **Test**: Accept individual channel percentages 0-100%
- [ ] **Code**: Add spectral ratio struct (e.g., `red_pct`, `blue_pct`)
- [ ] **Code**: Implement `ConfigValidator_SetSpectralRatio()`
- [ ] **Code**: Implement `ConfigValidator_GetSpectralRatio()`

#### TODO - Phase 1D: DLI Achievability Calculation
- [ ] **Test**: Calculate required PPFD from DLI + photoperiod + ramps
  - Example: DLI=20, photoperiod=16h, rise=60min, fall=60min
  - Effective time = 16h - 2h (ramps) = 14h = 50400s
  - PPFD_required = 20 mol/m²/day ÷ 50400s = ~396 µmol/m²·s
- [ ] **Test**: Validate DLI is achievable (PPFD < 2000 µmol/m²·s sensor max)
- [ ] **Test**: Return error if DLI not achievable
- [ ] **Test**: Calculate with zero ramp times (edge case)
- [ ] **Test**: Calculate with maximum ramp times (edge case)
- [ ] **Code**: Add `CONFIG_ERROR_DLI_NOT_ACHIEVABLE` to enum
- [ ] **Code**: Implement `ConfigValidator_CalculateRequiredPPFD()`
- [ ] **Code**: Implement `ConfigValidator_ValidateConfiguration()`

#### TODO - Phase 1E: Suggestion Engine
- [ ] **Test**: Suggest maximum achievable DLI given constraints
- [ ] **Test**: Suggest minimum photoperiod to achieve target DLI
- [ ] **Test**: Suggest reduced ramp times to achieve target DLI
- [ ] **Code**: Implement `ConfigValidator_GetMaxAchievableDLI()`
- [ ] **Code**: Implement `ConfigValidator_GetMinPhotoperiod()`
- [ ] **Code**: Implement `ConfigValidator_GetSuggestedRampTimes()`

#### TODO - Phase 1F: Persistence
- [ ] **Test**: Configuration persists across Create/Destroy cycles
- [ ] **Test**: Load defaults if NVM is empty/corrupted
- [ ] **Code**: Implement `ConfigValidator_SaveToNVM()`
- [ ] **Code**: Implement `ConfigValidator_LoadFromNVM()`
- [ ] **Code**: Add CRC/checksum validation for NVM data

---

### **PHASE 2: Hardware Abstraction Layer (HAL)** 🔜 NEXT
**Goal**: Abstract STM32 peripherals for testability
**Priority**: HIGH - Needed for control implementation

#### Module: SensorDriver
- [ ] **Design**: Define `sensor_driver.h` interface
- [ ] **Test**: Initialize ADC channel for PPFD sensor
- [ ] **Test**: Read raw ADC value (0-4095 for 12-bit ADC)
- [ ] **Test**: Apply calibration: `PPFD = (ADC - offset) * gain`
- [ ] **Test**: Apply moving average filter (N samples)
- [ ] **Test**: Saturate output at sensor max (2000 µmol/m²·s)
- [ ] **Test**: Handle ADC timeout/error conditions
- [ ] **Code**: Implement `SensorDriver_Init()`
- [ ] **Code**: Implement `SensorDriver_ReadRaw()`
- [ ] **Code**: Implement `SensorDriver_ReadCalibrated()`
- [ ] **Code**: Mock ADC HAL for host-side testing

#### Module: OutputDriver
- [ ] **Design**: Define `output_driver.h` interface
- [ ] **Test**: Initialize DAC/PWM channels (2+ outputs)
- [ ] **Test**: Set channel voltage 0-10V (or 0-100% PWM)
- [ ] **Test**: Map percentage to hardware value
- [ ] **Test**: Apply per-channel limits/clipping
- [ ] **Test**: Enable/disable channels independently
- [ ] **Test**: Handle simultaneous multi-channel update
- [ ] **Code**: Implement `OutputDriver_Init()`
- [ ] **Code**: Implement `OutputDriver_SetChannel()`
- [ ] **Code**: Implement `OutputDriver_EnableChannel()`
- [ ] **Code**: Mock DAC/PWM HAL for host-side testing

#### Module: CalibrationManager
- [ ] **Design**: Define `calibration.h` interface (Use Case #4)
- [ ] **Test**: Measure sensor offset with lights OFF
- [ ] **Test**: Measure sensor gain with reference lamp
- [ ] **Test**: Store calibration coefficients to NVM
- [ ] **Test**: Load calibration coefficients from NVM
- [ ] **Test**: Handle calibration failure (lamp doesn't turn on)
- [ ] **Test**: Validate calibration data CRC
- [ ] **Code**: Implement `Calibration_RunAutoCalibration()`
- [ ] **Code**: Implement `Calibration_GetOffsetGain()`

---

### **PHASE 3: Control Unit** 🎯 CORE ALGORITHM
**Goal**: Implement closed-loop PPFD regulation
**Priority**: HIGH - Core product functionality

#### Module: ControlUnit
- [ ] **Design**: Define `control_unit.h` interface
- [ ] **Test**: Initialize controller with setpoint
- [ ] **Test**: Calculate control error: `e = setpoint - measured`
- [ ] **Test**: Implement PI controller: `u = Kp*e + Ki*∫e`
- [ ] **Test**: Apply anti-windup (integral clamp)
- [ ] **Test**: Saturate output at 0-100%
- [ ] **Test**: Distribute output across channels by spectral ratio
- [ ] **Test**: Maintain control loop at 1Hz (per requirements)
- [ ] **Test**: Achieve <5% steady-state error (per requirements)
- [ ] **Test**: Enable/disable control (Use Case #2)
- [ ] **Code**: Implement `ControlUnit_Init()`
- [ ] **Code**: Implement `ControlUnit_SetSetpoint()`
- [ ] **Code**: Implement `ControlUnit_Update()` (called at 1Hz)
- [ ] **Code**: Implement `ControlUnit_Enable()` / `Disable()`

#### Ramp Profile Generator (if dynamic ramps required)
- [ ] **Design**: Define ramp profile structure
- [ ] **Test**: Generate sunrise ramp (0% → 100% over rise_time)
- [ ] **Test**: Generate sunset ramp (100% → 0% over fall_time)
- [ ] **Test**: Generate steady-state profile (flat 100%)
- [ ] **Test**: Transition between profile segments
- [ ] **Code**: Implement `ProfileGenerator_GenerateDaily()`
- [ ] **Code**: Implement `ProfileGenerator_GetCurrentSetpoint(time)`

---

### **PHASE 4: User Interface** 💬
**Goal**: Implement UART command interface
**Priority**: MEDIUM - Needed for field configuration

#### Module: UserInterface
- [ ] **Design**: Define `user_interface.h` interface
- [ ] **Design**: Define command protocol (text-based? binary?)
- [ ] **Test**: Parse command: "SET_DLI 20" (Use Case #1)
- [ ] **Test**: Parse command: "SET_PHOTOPERIOD 16"
- [ ] **Test**: Parse command: "SET_RATIO 70 30" (Use Case #3)
- [ ] **Test**: Parse command: "ENABLE_CONTROL" (Use Case #2)
- [ ] **Test**: Parse command: "DISABLE_CONTROL"
- [ ] **Test**: Parse command: "CALIBRATE" (Use Case #4)
- [ ] **Test**: Parse command: "GET_STATUS"
- [ ] **Test**: Handle invalid commands gracefully
- [ ] **Test**: Response timeout <500ms (per requirements)
- [ ] **Code**: Implement `UI_Init()`
- [ ] **Code**: Implement `UI_ProcessCommand()`
- [ ] **Code**: Implement `UI_SendResponse()`
- [ ] **Code**: Mock UART HAL for testing

#### Display Support (if HMI screen added)
- [ ] **Design**: Define display update protocol
- [ ] **Test**: Display current PPFD measurement
- [ ] **Test**: Display target PPFD setpoint
- [ ] **Test**: Display control status (ON/OFF)
- [ ] **Test**: Display spectral ratio
- [ ] **Code**: Implement `Display_Update()`

---

### **PHASE 5: Communication Interface** 🔌
**Goal**: Implement "Puerto G" protocol for ecosystem integration
**Priority**: MEDIUM - Required for Cannfeel integration

#### Module: CommunicationInterface
- [ ] **Design**: Define `comm_interface.h` interface
- [ ] **Design**: Understand Cannfeel "Puerto G" protocol spec
- [ ] **Test**: Send telemetry packet (PPFD, status, alarms)
- [ ] **Test**: Receive remote configuration command
- [ ] **Test**: Validate protocol checksum/CRC
- [ ] **Test**: Handle protocol timeout
- [ ] **Test**: Queue messages if bus is busy
- [ ] **Code**: Implement `CommInterface_Init()`
- [ ] **Code**: Implement `CommInterface_SendTelemetry()`
- [ ] **Code**: Implement `CommInterface_ReceiveCommand()`
- [ ] **Code**: Mock Puerto G hardware for testing

---

### **PHASE 6: System Integration** 🧩
**Goal**: Tie all modules together with state machine
**Priority**: HIGH - Final integration

#### Module: SystemManager
- [ ] **Design**: Define system states (IDLE, CALIBRATING, READ_ONLY, MANUAL, AUTOMATIC)
- [ ] **Test**: State transition: IDLE → CALIBRATING on command
- [ ] **Test**: State transition: IDLE → AUTOMATIC on "enable control"
- [ ] **Test**: State transition: AUTOMATIC → MANUAL on user override
- [ ] **Test**: State transition: * → READ_ONLY on sensor failure
- [ ] **Test**: Watchdog reset handling
- [ ] **Test**: Power-on initialization sequence
- [ ] **Test**: Error condition handling (sensor disconnect, actuator fault)
- [ ] **Code**: Implement `SystemManager_Init()`
- [ ] **Code**: Implement `SystemManager_Run()` (main loop)
- [ ] **Code**: Implement state machine logic

#### Integration Testing
- [ ] **Test**: End-to-end: Configure → Enable → Measure → Control → Verify
- [ ] **Test**: 30-day continuous operation (per requirements)
- [ ] **Test**: Performance: Control error <5% steady-state
- [ ] **Test**: Performance: Response time <1 second to user commands
- [ ] **Test**: Memory leak check (if dynamic allocation used)
- [ ] **Test**: Stack usage analysis
- [ ] **Code**: Create hardware-in-the-loop (HIL) test setup

---

### **PHASE 7: Documentation & Deployment** 📝
**Goal**: Finalize deliverables
**Priority**: MEDIUM

- [ ] Complete technical memory (report)
- [ ] Generate Doxygen API documentation
- [ ] Create wiring diagrams (sensor, actuators, power)
- [ ] Write calibration procedure document
- [ ] Write field testing report
- [ ] Prepare public presentation slides
- [ ] Archive code to repository with tags

---

## 🎯 Milestone Tracking

| Milestone | Target Date | Status |
|-----------|-------------|--------|
| ConfigValidator Complete | TBD | ⏳ 8/33 tests |
| HAL Complete | TBD | 🔜 Not started |
| Control Unit Complete | TBD | 🔜 Not started |
| User Interface Complete | TBD | 🔜 Not started |
| System Integration | TBD | 🔜 Not started |
| Field Testing | TBD | 🔜 Not started |
| Final Presentation | Jun 15, 2026 | 🔜 Not started |

---

## 🔧 Development Environment Setup

### Prerequisites
- [x] Ceedling 1.0.1 installed
- [x] STM32 development kit available
- [x] Git repository initialized
- [ ] CI/CD pipeline configured (optional)
- [ ] Code coverage reporting configured (gcov)

### Commands
```bash
# Run all tests
ceedling test:all

# Run specific module tests
ceedling test:config_validator

# Generate coverage report
ceedling gcov:all

# Clean build artifacts
ceedling clean
```

---

## 📊 Test Coverage Goals

| Module | Target Coverage | Current Coverage |
|--------|----------------|------------------|
| ConfigValidator | 95% | TBD |
| SensorDriver | 90% | TBD |
| OutputDriver | 90% | TBD |
| ControlUnit | 95% | TBD |
| UserInterface | 85% | TBD |
| CommInterface | 85% | TBD |
| SystemManager | 90% | TBD |
| **Overall** | **90%** | **TBD** |

---

## 🚨 Known Risks & Mitigation

| Risk | Severity | Mitigation |
|------|----------|------------|
| Component delivery delay | HIGH | Order early, identify alternatives |
| PCB design error | HIGH | Review checklist, peer review |
| Control instability | MEDIUM | Tune in simulation first |
| HMI complexity | MEDIUM | Start with UART, add display later |
| Puerto G protocol unknown | HIGH | Get spec from Cannfeel ASAP |

---

## 📚 References

- [Plan de Proyecto](./GdP_Folmer_Fernando_V5.pdf)
- [Especificación de Requerimientos](./Folmer-IdS-TP1-ERS-IEEE_830.pdf)
- [Casos de Uso](./Folmer-IdS-TP2-CU.pdf)
- [Arquitectura de Software](./Folmer-IdS-TP3-DAS.pdf)
- [Test-Driven Development for Embedded C](https://pragprog.com/titles/jgade/test-driven-development-for-embedded-c/) - James Grenning
- [Ceedling Documentation](http://www.throwtheswitch.org/ceedling)

---

**Last Updated**: 2025-10-27
**Maintained by**: Fernando Folmer
