# PPFD Controller - Architecture Layers

## Overview
This document describes the layered architecture of the PPFD controller system, showing how modules are organized and interact.

---

## 1. Layer Diagram (Top-Down View)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                                │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │                     SystemManager                              │ │
│  │  - Main state machine (INIT, IDLE, CALIBRATING, MANUAL, AUTO)  │ │
│  │  - Coordinates all subsystems                                  │ │
│  │  - Command routing and error handling                          │ │
│  └────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘
                                 │
                    ┌────────────┼────────────┐
                    ▼            ▼            ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    INTERFACE LAYER                                  │
│  ┌──────────────────┐      ┌──────────────────────────────────────┐│
│  │  UserInterface   │      │    CommunicationInterface            ││
│  │  - UART CLI      │      │    - Puerto G protocol              ││
│  │  - Commands      │      │    - Telemetry & remote commands    ││
│  │  - Status output │      │    - CRC validation                 ││
│  └──────────────────┘      └──────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────────┘
                                 │
                    ┌────────────┼────────────┐
                    ▼            ▼            ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    BUSINESS LOGIC LAYER                             │
│  ┌────────────────────┐    ┌────────────────────┐                   │
│  │   ControlUnit      │    │ CalibrationManager │                   │
│  │   - PI controller  │    │ - Auto-calibration │                   │
│  │   - Closed-loop    │    │ - Offset/gain calc │                   │
│  │   - Spectral dist. │    │ - Reference lamp   │                   │
│  │   - 1Hz update     │    │ - State machine    │                   │
│  └────────────────────┘    └────────────────────┘                   │
│            │                         │                              │
│            └─────────┬───────────────┘                              │
│                      ▼                                              │
│         ┌────────────────────────┐                                  │
│         │   ConfigValidator      │                                  │
│         │   - Parameter storage  │                                  │
│         │   - Range validation   │                                  │
│         │   - DLI/PPFD calc      │                                  │
│         └────────────────────────┘                                  │
└─────────────────────────────────────────────────────────────────────┘
                                 │
                    ┌────────────┼────────────┐
                    ▼            ▼            ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    HARDWARE ABSTRACTION LAYER (HAL)                 │
│  ┌────────────────────┐              ┌─────────────────────────┐    │
│  │   SensorDriver     │              │    OutputDriver         │    │
│  │   - ADC reading    │              │    - DAC/PWM control    │    │
│  │   - Calibration    │              │    - 0-10V output       │    │
│  │   - Filtering      │              │    - Multi-channel      │    │
│  │   - PPFD calc      │              │    - Safety limits      │    │
│  └────────────────────┘              └─────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    HARDWARE LAYER (STM32)                           │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌─────────────┐    │
│  │    ADC     │  │   DAC/PWM  │  │   UART     │  │    NVM      │    │
│  │  (Sensor)  │  │  (Outputs) │  │  (Comms)   │  │  (Config)   │    │
│  └────────────┘  └────────────┘  └────────────┘  └─────────────┘    │
│                                                                     │
│              STM32F4 Cortex-M4 + FreeRTOS                           │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Module Dependency Graph

```
Legend: [Module] ──uses──> [Dependency]

                            SystemManager
                                  │
              ┌───────────────────┼────────────────────┐
              ▼                   ▼                    ▼
       UserInterface    CommunicationInterface   ControlUnit
                                                       │
                                    ┌──────────────────┼──────────────┐
                                    ▼                  ▼              ▼
                            ConfigValidator    SensorDriver   OutputDriver
                                                       │              │
                                                       └──────┬───────┘
                                                              ▼
                                                   CalibrationManager
```

### Dependency Rules:
1. **No circular dependencies** - Lower layers never depend on upper layers
2. **HAL isolation** - Business logic never directly accesses hardware
3. **Testability** - Each layer can be tested independently with mocks

---

## 3. Data Flow (Closed-Loop Control Example)

```
User Command: "Enable automatic control with PPFD=500 µmol/m²·s"
    │
    ▼
┌─────────────────────┐
│  UserInterface      │  Parse command
│  "enable 500"       │
└─────────────────────┘
    │
    ▼
┌─────────────────────┐
│  SystemManager      │  Transition to AUTOMATIC state
└─────────────────────┘
    │
    ▼
┌─────────────────────┐
│  ConfigValidator    │  Validate PPFD=500 is achievable
│  Store setpoint     │
└─────────────────────┘
    │
    ▼
┌─────────────────────┐
│  ControlUnit        │  Set setpoint=500, start control loop
│  (1 Hz update)      │
└─────────────────────┘
    │
    ├──────────────────────────────┐
    ▼                              ▼
┌─────────────────────┐    ┌─────────────────────┐
│  SensorDriver       │    │  OutputDriver       │
│  ReadPPFD()         │    │  SetChannel()       │
│  measured=450       │    │  red=60%, blue=40%  │
└─────────────────────┘    └─────────────────────┘
    │                              ▲
    │                              │
    │    ┌─────────────────────┐   │
    └───>│  ControlUnit        │───┘
         │  PI Algorithm:      │
         │  error = 500-450=50 │
         │  output = f(error)  │
         └─────────────────────┘
```

---

## 4. Test Strategy by Layer

### Layer 1: Hardware Abstraction (HAL)
**Modules**: SensorDriver, OutputDriver

**Testing approach**:
- ✅ **Unit tests** with mocked hardware (CMock for STM32 HAL)
- ✅ **Stub ADC/DAC** values for deterministic tests
- ⚠️ **Hardware-in-the-loop (HIL)** tests on real STM32 (future)

**Example**:
```c
// Mock ADC to return 2048 (mid-scale)
Mock_HAL_ADC_GetValue_ExpectAndReturn(2048);

// Test that SensorDriver applies calibration correctly
SensorDriver_ReadPPFD(sensor, &ppfd);
TEST_ASSERT_EQUAL_UINT16(expected_ppfd, ppfd);
```

---

### Layer 2: Business Logic
**Modules**: ConfigValidator, ControlUnit, CalibrationManager

**Testing approach**:
- ✅ **Pure unit tests** (no hardware, no mocks needed for ConfigValidator)
- ✅ **Mocked dependencies** (ControlUnit mocks SensorDriver/OutputDriver)
- ✅ **Algorithm validation** (PI controller step response, DLI calculations)

**Example**:
```c
// Test PI controller response to step input
ControlUnit_SetSetpoint(ctrl, 500);
Mock_SensorDriver_ReadPPFD_ExpectAnyArgsAndReturn(SENSOR_OK);
Mock_SensorDriver_ReadPPFD_ReturnThruPtr_ppfd(&measured_value);

ControlUnit_Update(ctrl); // Run one control cycle

// Verify output increases when error is positive
ControlUnit_GetState(ctrl, &state);
TEST_ASSERT_GREATER_THAN(0, state.output_percent);
```

---

### Layer 3: Interface Layer
**Modules**: UserInterface, CommunicationInterface

**Testing approach**:
- ✅ **Command parsing tests** (string → command struct)
- ✅ **Protocol tests** (packet encoding/decoding, CRC)
- ✅ **Mocked UART** (no actual serial port needed)

**Example**:
```c
// Test command parsing
Command cmd;
UserInterface_ProcessInput(ui, "set_dli 25", &cmd);
TEST_ASSERT_EQUAL(CMD_SET_DLI, cmd.type);
TEST_ASSERT_EQUAL_UINT16(25, cmd.arg1);
```

---

### Layer 4: Application Layer
**Modules**: SystemManager

**Testing approach**:
- ✅ **State machine tests** (valid/invalid transitions)
- ✅ **Integration tests** (orchestration of multiple modules)
- ⚠️ **System tests** (end-to-end scenarios on target hardware)

**Example**:
```c
// Test state transition: IDLE → AUTOMATIC
SystemManager_SetState(sys, SYSTEM_STATE_AUTOMATIC);
SystemManager_GetState(sys, &state);
TEST_ASSERT_EQUAL(SYSTEM_STATE_AUTOMATIC, state);

// Verify control is actually running
Mock_ControlUnit_Update_Expect(ctrl_handle);
SystemManager_Run(sys); // Main loop iteration
```

---

## 5. Current Implementation Status

### ✅ Completed Layers

#### Hardware Abstraction Layer (Partial)
- **ConfigValidator** ✅ 35/35 tests passing
  - Basic parameter storage (DLI, photoperiod, rise/fall times)
  - Range validation with symbolic constants
  - Opaque handle pattern
  - Static allocation (embedded-friendly)

- **SensorDriver** 🚧 20/20 basic tests passing
  - Module lifecycle (Create/Destroy/Init)
  - Calibration data storage
  - Filter configuration
  - ⚠️ **TODO**: Actual PPFD calculation logic
  - ⚠️ **TODO**: Moving average filter implementation
  - ⚠️ **TODO**: ADC hardware mocking

#### Testing Infrastructure
- ✅ Ceedling/Unity/CMock framework configured
- ✅ Float support enabled in Unity
- ✅ Test coverage reporting (gcov)
- ✅ Pre-commit hooks (clang-format, trailing whitespace)

---

### 🚧 Work In Progress

#### Hardware Abstraction Layer
- **SensorDriver**: Basic structure complete, need PPFD calculation + filtering
- **OutputDriver**: Not started

---

### ⏳ Not Started

#### Business Logic Layer
- **ControlUnit**: Not started
- **CalibrationManager**: Not started

#### Interface Layer
- **UserInterface**: Not started
- **CommunicationInterface**: Not started

#### Application Layer
- **SystemManager**: Not started

---

## 6. Development Order (Bottom-Up)

Following the layer diagram, the recommended implementation order is:

### Phase 1: Foundation (HAL) ← **WE ARE HERE**
1. ✅ **ConfigValidator** - Complete
2. 🚧 **SensorDriver** - Basic structure done, need algorithm implementation
3. ⏳ **OutputDriver** - Next up
4. ⏳ **CalibrationManager** - Depends on SensorDriver + OutputDriver

### Phase 2: Core Logic
5. ⏳ **ControlUnit** - Depends on Phase 1
6. ⏳ Advanced ConfigValidator features (spectral ratio, DLI validation)

### Phase 3: Communication
7. ⏳ **UserInterface** - Can be done in parallel with Phase 2
8. ⏳ **CommunicationInterface** - Can be done in parallel with Phase 2

### Phase 4: Integration
9. ⏳ **SystemManager** - Depends on all above
10. ⏳ Integration testing
11. ⏳ Hardware-in-the-loop testing on STM32F4

---

## 7. Key Architectural Decisions

### ✅ Decisions Made

1. **Opaque Handle Pattern**
   - All modules use `typedef struct ModuleName * ModuleNameHandle`
   - Hides implementation details
   - Enables easy mocking in tests

2. **Static Memory Allocation**
   - No `malloc()` / `free()`
   - Singleton pattern where appropriate (ConfigValidator, SensorDriver)
   - Predictable memory usage for embedded systems

3. **Error Code Returns**
   - All functions return error enum
   - Output parameters via pointers
   - Consistent error handling across modules

4. **Test-Driven Development**
   - Write tests first (RED phase)
   - Implement minimal code to pass (GREEN phase)
   - Refactor for quality (REFACTOR phase)

5. **Layered Architecture**
   - Clear separation of concerns
   - No circular dependencies
   - Hardware abstraction for testability

---

### ⏳ Decisions Pending

1. **FreeRTOS Task Structure**
   - How many tasks? (Control loop, UI, Comms?)
   - Task priorities?
   - Inter-task communication (queues, semaphores)?

2. **Floating Point vs Fixed Point**
   - Currently using `float` for calibration
   - Consider fixed-point for control loop? (performance)

3. **NVM Storage Strategy**
   - Which NVM technology? (Flash, EEPROM)
   - Wear leveling needed?
   - Configuration versioning?

4. **Safety Requirements**
   - Watchdog timer implementation?
   - Redundant sensor readings?
   - Emergency shutdown logic?

---

## 8. Module Communication Patterns

### Pattern 1: Direct Function Calls (Synchronous)
```
ControlUnit ──calls──> SensorDriver_ReadPPFD()
            ──calls──> OutputDriver_SetChannel()
```
**Used for**: Tight coupling within control loop (1 Hz cycle)

---

### Pattern 2: Command/Response (Asynchronous)
```
UserInterface ──enqueues command──> SystemManager
SystemManager ──executes command──> ControlUnit
SystemManager ──sends response──> UserInterface
```
**Used for**: User commands, remote control

---

### Pattern 3: State Machine (Event-Driven)
```
SystemManager states:
    INIT → IDLE → CALIBRATING → IDLE
    IDLE → MANUAL or AUTOMATIC
    Any state → ERROR
```
**Used for**: Top-level system coordination

---

## 9. Testing Pyramid

```
                    ┌──────────────┐
                    │  System      │  ← Few, expensive, on hardware
                    │  Tests       │
                    └──────────────┘
                ┌────────────────────┐
                │  Integration       │  ← Some, with mocked HAL
                │  Tests             │
                └────────────────────┘
            ┌──────────────────────────┐
            │  Unit Tests              │  ← Many, fast, isolated
            │  (35 ConfigValidator +   │
            │   20 SensorDriver + ...) │
            └──────────────────────────┘
```

**Current status**: We're building a strong base of unit tests (bottom of pyramid)

---

## 10. Next Steps

### Immediate (This Session)
1. ✅ Complete SensorDriver basic structure
2. ⏳ Decide: Continue with SensorDriver PPFD calculation, or move to OutputDriver?

### Short Term (Next Few Sessions)
3. Complete OutputDriver module
4. Implement CalibrationManager
5. Start ControlUnit (PI controller)

### Medium Term
6. Add UserInterface (UART CLI)
7. Implement SystemManager state machine
8. Integration testing

### Long Term
9. CommunicationInterface (Puerto G protocol)
10. Hardware-in-the-loop testing
11. Real-time performance optimization

---

**Document Status**: Living document, updated as architecture evolves
**Last Updated**: 2025-11-03
