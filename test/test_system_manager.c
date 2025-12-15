#include "config_validator.h"
#include "sensor_driver.h"
#include "output_driver.h"
#include "calibration_manager.h"
#include "control_unit.h"
#include "system_manager.h"
#include "unity.h"
#include <string.h>

// System under test
static SystemManagerHandle system;

// Mock hardware interfaces
static SensorHardwareInterface mockSensorHw;
static OutputHardwareInterface mockOutputHw;

// Mock state
static float mockSensorPPFD;
static float mockOutputPercent[OUTPUT_CHANNEL_COUNT];

// Mock sensor hardware
static float MockSensorReadRaw(void) {
    return mockSensorPPFD;
}

// Mock output hardware
static void MockOutputSetRaw(OutputChannel channel, uint16_t rawValue) {
    if (channel < OUTPUT_CHANNEL_COUNT) {
        mockOutputPercent[channel] = (rawValue / 4095.0f) * 100.0f;
    }
}

void setUp(void) {
    // Clear mock state
    mockSensorPPFD = 0.0f;
    memset(mockOutputPercent, 0, sizeof(mockOutputPercent));

    // Setup mock hardware interfaces
    mockSensorHw.ReadRawPPFD = MockSensorReadRaw;
    mockOutputHw.SetRawOutput = MockOutputSetRaw;

    // Create system manager (it creates all subsystems internally)
    system = SystemManager_Create(&mockSensorHw, &mockOutputHw);
}

void tearDown(void) {
    SystemManager_Destroy(system);
}

// ========== Lifecycle Tests ==========

void test_SystemManager_CanBeCreated(void) {
    TEST_ASSERT_NOT_NULL(system);
}

void test_SystemManager_Create_RejectsNullSensorHw(void) {
    SystemManagerHandle handle = SystemManager_Create(NULL, &mockOutputHw);
    TEST_ASSERT_NULL(handle);
}

void test_SystemManager_Create_RejectsNullOutputHw(void) {
    SystemManagerHandle handle = SystemManager_Create(&mockSensorHw, NULL);
    TEST_ASSERT_NULL(handle);
}

void test_SystemManager_InitializesSuccessfully(void) {
    SystemError result = SystemManager_Init(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_SystemManager_Init_RejectsNullHandle(void) {
    SystemError result = SystemManager_Init(NULL);
    TEST_ASSERT_EQUAL(SYSTEM_ERROR_NULL_POINTER, result);
}

void test_SystemManager_InitialStateIsUninitialized(void) {
    SystemState state;
    SystemError result = SystemManager_GetState(system, &state);

    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_UNINITIALIZED, state);
}

void test_SystemManager_StateIsIdleAfterInit(void) {
    SystemManager_Init(system);

    SystemState state;
    SystemManager_GetState(system, &state);

    TEST_ASSERT_EQUAL(SYSTEM_STATE_IDLE, state);
}

void test_SystemManager_GetSubsystemsAfterCreate(void) {
    // Verify all subsystems were created
    ConfigValidatorHandle config = NULL;
    SensorDriverHandle sensor = NULL;
    OutputDriverHandle output = NULL;
    CalibrationManagerHandle calib = NULL;
    ControlUnitHandle control = NULL;

    TEST_ASSERT_EQUAL(SYSTEM_OK, SystemManager_GetConfigValidator(system, &config));
    TEST_ASSERT_EQUAL(SYSTEM_OK, SystemManager_GetSensorDriver(system, &sensor));
    TEST_ASSERT_EQUAL(SYSTEM_OK, SystemManager_GetOutputDriver(system, &output));
    TEST_ASSERT_EQUAL(SYSTEM_OK, SystemManager_GetCalibrationManager(system, &calib));
    TEST_ASSERT_EQUAL(SYSTEM_OK, SystemManager_GetControlUnit(system, &control));

    TEST_ASSERT_NOT_NULL(config);
    TEST_ASSERT_NOT_NULL(sensor);
    TEST_ASSERT_NOT_NULL(output);
    TEST_ASSERT_NOT_NULL(calib);
    TEST_ASSERT_NOT_NULL(control);
}

// ========== State Machine Transition Tests ==========

void test_SystemManager_TransitionIdleToManual(void) {
    SystemManager_Init(system);

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_MANUAL);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_MANUAL, state);
}

void test_SystemManager_TransitionIdleToAutomatic(void) {
    SystemManager_Init(system);

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_AUTOMATIC);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_AUTOMATIC, state);
}

void test_SystemManager_TransitionIdleToCalibrating(void) {
    SystemManager_Init(system);

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_CALIBRATING);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_CALIBRATING, state);
}

void test_SystemManager_TransitionManualToIdle(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_MANUAL);

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_IDLE);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_IDLE, state);
}

void test_SystemManager_TransitionAutomaticToIdle(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_AUTOMATIC);

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_IDLE);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_IDLE, state);
}

void test_SystemManager_TransitionCalibratingToIdle(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_CALIBRATING);

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_IDLE);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_IDLE, state);
}

void test_SystemManager_RejectsUninitializedToManual(void) {
    // Don't call Init() - stays in UNINITIALIZED

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_MANUAL);
    TEST_ASSERT_EQUAL(SYSTEM_ERROR_INVALID_STATE, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_UNINITIALIZED, state);
}

void test_SystemManager_RejectsManualToAutomatic(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_MANUAL);

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_AUTOMATIC);
    TEST_ASSERT_EQUAL(SYSTEM_ERROR_INVALID_STATE, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_MANUAL, state); // Should stay in MANUAL
}

void test_SystemManager_RejectsAutomaticToCalibrating(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_AUTOMATIC);

    SystemError result = SystemManager_SetState(system, SYSTEM_STATE_CALIBRATING);
    TEST_ASSERT_EQUAL(SYSTEM_ERROR_INVALID_STATE, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_AUTOMATIC, state); // Should stay in AUTOMATIC
}

void test_SystemManager_EmergencyStopTransitionsToError(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_AUTOMATIC);

    SystemError result = SystemManager_EmergencyStop(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_ERROR, state);
}

void test_SystemManager_ResetTransitionsFromErrorToIdle(void) {
    SystemManager_Init(system);
    SystemManager_EmergencyStop(system); // Force to ERROR state

    SystemError result = SystemManager_Reset(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_IDLE, state);
}

void test_SystemManager_SetState_RejectsNullHandle(void) {
    SystemError result = SystemManager_SetState(NULL, SYSTEM_STATE_IDLE);
    TEST_ASSERT_EQUAL(SYSTEM_ERROR_NULL_POINTER, result);
}

// ========== Module Coordination Tests (Run) ==========

void test_SystemManager_Run_SucceedsInIdleState(void) {
    SystemManager_Init(system);

    SystemError result = SystemManager_Run(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // State should remain IDLE
    SystemState state;
    SystemManager_GetState(system, &state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_IDLE, state);
}

void test_SystemManager_Run_SucceedsInManualState(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_MANUAL);

    SystemError result = SystemManager_Run(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_SystemManager_Run_SucceedsInAutomaticState(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_AUTOMATIC);

    // In AUTO mode, Run() should call ControlUnit_Update()
    SystemError result = SystemManager_Run(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_SystemManager_Run_SucceedsInCalibratingState(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_CALIBRATING);

    SystemError result = SystemManager_Run(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_SystemManager_Run_SucceedsInErrorState(void) {
    SystemManager_Init(system);
    SystemManager_EmergencyStop(system);

    // Run() should succeed even in ERROR state (no-op)
    SystemError result = SystemManager_Run(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_SystemManager_Run_RejectsNullHandle(void) {
    SystemError result = SystemManager_Run(NULL);
    TEST_ASSERT_EQUAL(SYSTEM_ERROR_NULL_POINTER, result);
}

// ========== Error Handling & Status Tests ==========

void test_SystemManager_GetStatus_ReportsCorrectState(void) {
    SystemManager_Init(system);
    SystemManager_SetState(system, SYSTEM_STATE_MANUAL);

    SystemStatus status;
    SystemError result = SystemManager_GetStatus(system, &status);

    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_MANUAL, status.state);
}

void test_SystemManager_GetStatus_ReportsCalibrationValid(void) {
    SystemManager_Init(system);

    SystemStatus status;
    SystemManager_GetStatus(system, &status);

    // Calibration initially invalid (no calibration set)
    TEST_ASSERT_FALSE(status.calibrationValid);
}

void test_SystemManager_GetLastError_ReturnsLastError(void) {
    SystemManager_Init(system);

    SystemError error;
    SystemError result = SystemManager_GetLastError(system, &error);

    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_OK, error); // No error initially
}

void test_SystemManager_ClearError_ResetsErrorCode(void) {
    SystemManager_Init(system);

    // Simulate an error being set (via EmergencyStop)
    SystemManager_EmergencyStop(system);

    // Clear the error
    SystemError result = SystemManager_ClearError(system);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify error was cleared
    SystemError lastError;
    SystemManager_GetLastError(system, &lastError);
    TEST_ASSERT_EQUAL(SYSTEM_OK, lastError);
}

void test_SystemManager_GetStatus_RejectsNullHandle(void) {
    SystemStatus status;
    SystemError result = SystemManager_GetStatus(NULL, &status);
    TEST_ASSERT_EQUAL(SYSTEM_ERROR_NULL_POINTER, result);
}

void test_SystemManager_GetStatus_RejectsNullPointer(void) {
    SystemManager_Init(system);
    SystemError result = SystemManager_GetStatus(system, NULL);
    TEST_ASSERT_EQUAL(SYSTEM_ERROR_NULL_POINTER, result);
}
