#include "config_validator.h"
#include "output_driver.h"
#include "sensor_driver.h"
#include "control_unit.h"
#include "unity.h"
#include <string.h>

// Mock dependencies
static ControlUnitHandle control;
static ConfigValidatorHandle mockConfig;
static SensorDriverHandle mockSensor;
static OutputDriverHandle mockOutput;

// Mock hardware interfaces
static SensorHardwareInterface mockSensorHw;
static OutputHardwareInterface mockOutputHw;

// Mock state
static float mockSensorPPFD;
static float mockOutputPercent[OUTPUT_CHANNEL_COUNT];

// Mock functions
static float MockSensorReadRaw(void) {
    return mockSensorPPFD;
}

static void MockOutputSetRaw(OutputChannel channel, uint16_t rawValue) {
    if (channel < OUTPUT_CHANNEL_COUNT) {
        mockOutputPercent[channel] = (rawValue / 4095.0f) * 100.0f;
    }
}

void setUp(void) {
    // Clear mock state
    mockSensorPPFD = 0.0f;
    memset(mockOutputPercent, 0, sizeof(mockOutputPercent));

    // Create mock dependencies
    mockConfig = ConfigValidator_Create();

    mockSensorHw.ReadRawPPFD = MockSensorReadRaw;
    mockSensor = SensorDriver_Create(&mockSensorHw);
    SensorDriver_Init(mockSensor);

    mockOutputHw.SetRawOutput = MockOutputSetRaw;
    mockOutput = OutputDriver_Create(&mockOutputHw);
    OutputDriver_Init(mockOutput);

    // Create control unit
    control = ControlUnit_Create(mockConfig, mockSensor, mockOutput);
}

void tearDown(void) {
    ControlUnit_Destroy(control);
    OutputDriver_Destroy(mockOutput);
    SensorDriver_Destroy(mockSensor);
    ConfigValidator_Destroy(mockConfig);
}

// ========== Lifecycle Tests ==========

void test_ControlUnit_CanBeCreated(void) {
    TEST_ASSERT_NOT_NULL(control);
}

void test_ControlUnit_Create_RejectsNullConfig(void) {
    ControlUnitHandle handle = ControlUnit_Create(NULL, mockSensor, mockOutput);
    TEST_ASSERT_NULL(handle);
}

void test_ControlUnit_Create_RejectsNullSensor(void) {
    ControlUnitHandle handle = ControlUnit_Create(mockConfig, NULL, mockOutput);
    TEST_ASSERT_NULL(handle);
}

void test_ControlUnit_Create_RejectsNullOutput(void) {
    ControlUnitHandle handle = ControlUnit_Create(mockConfig, mockSensor, NULL);
    TEST_ASSERT_NULL(handle);
}

void test_ControlUnit_InitializesSuccessfully(void) {
    ControlError result = ControlUnit_Init(control);
    TEST_ASSERT_EQUAL(CONTROL_OK, result);
}

void test_ControlUnit_Init_RejectsNullHandle(void) {
    ControlError result = ControlUnit_Init(NULL);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

// ========== Mode Control Tests ==========

void test_ControlUnit_DefaultModeIsManual(void) {
    ControlUnit_Init(control);

    ControlMode mode;
    ControlUnit_GetMode(control, &mode);

    TEST_ASSERT_EQUAL(CONTROL_MODE_MANUAL, mode);
}

void test_ControlUnit_SetMode_RejectsNullHandle(void) {
    ControlError result = ControlUnit_SetMode(NULL, CONTROL_MODE_AUTOMATIC);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_GetMode_RejectsNullHandle(void) {
    ControlMode mode;
    ControlError result = ControlUnit_GetMode(NULL, &mode);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_GetMode_RejectsNullPointer(void) {
    ControlUnit_Init(control);
    ControlError result = ControlUnit_GetMode(control, NULL);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_SetMode_SwitchesToAutomatic(void) {
    ControlUnit_Init(control);

    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);

    ControlMode mode;
    ControlUnit_GetMode(control, &mode);
    TEST_ASSERT_EQUAL(CONTROL_MODE_AUTOMATIC, mode);
}

void test_ControlUnit_SetMode_SwitchesToManual(void) {
    ControlUnit_Init(control);
    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);

    ControlUnit_SetMode(control, CONTROL_MODE_MANUAL);

    ControlMode mode;
    ControlUnit_GetMode(control, &mode);
    TEST_ASSERT_EQUAL(CONTROL_MODE_MANUAL, mode);
}

// ========== Setpoint Tests ==========

void test_ControlUnit_SetSetpoint_RejectsNullHandle(void) {
    ControlError result = ControlUnit_SetSetpoint(NULL, 500);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_GetSetpoint_RejectsNullHandle(void) {
    uint16_t setpoint;
    ControlError result = ControlUnit_GetSetpoint(NULL, &setpoint);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_GetSetpoint_RejectsNullPointer(void) {
    ControlUnit_Init(control);
    ControlError result = ControlUnit_GetSetpoint(control, NULL);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_SetSetpoint_StoresValue(void) {
    ControlUnit_Init(control);

    ControlUnit_SetSetpoint(control, 750);

    uint16_t setpoint = 0;
    ControlUnit_GetSetpoint(control, &setpoint);
    TEST_ASSERT_EQUAL_UINT16(750, setpoint);
}

void test_ControlUnit_SetSetpoint_RejectsValueAboveMax(void) {
    ControlUnit_Init(control);

    ControlError result = ControlUnit_SetSetpoint(control, 5000);

    TEST_ASSERT_EQUAL(CONTROL_ERROR_INVALID_SETPOINT, result);
}

// ========== PI Parameters Tests ==========

void test_ControlUnit_SetPIParameters_RejectsNullHandle(void) {
    PIParameters params = {.Kp = 1.0f, .Ki = 0.1f, .integralLimit = 100.0f};
    ControlError result = ControlUnit_SetPIParameters(NULL, params);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_GetPIParameters_RejectsNullHandle(void) {
    PIParameters params;
    ControlError result = ControlUnit_GetPIParameters(NULL, &params);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_GetPIParameters_RejectsNullPointer(void) {
    ControlUnit_Init(control);
    ControlError result = ControlUnit_GetPIParameters(control, NULL);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_PIParameters_HasDefaultValues(void) {
    ControlUnit_Init(control);

    PIParameters params;
    ControlUnit_GetPIParameters(control, &params);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, params.Kp); // Default Kp
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, params.Ki); // Ki not used yet
}

void test_ControlUnit_SetPIParameters_StoresValues(void) {
    ControlUnit_Init(control);

    PIParameters params_in = {.Kp = 0.5f, .Ki = 0.0f, .integralLimit = 50.0f};
    ControlUnit_SetPIParameters(control, params_in);

    PIParameters params_out;
    ControlUnit_GetPIParameters(control, &params_out);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, params_out.Kp);
}

// ========== Manual Mode Tests ==========

void test_ControlUnit_SetManualOutput_RejectsNullHandle(void) {
    ControlError result = ControlUnit_SetManualOutput(NULL, OUTPUT_CHANNEL_0, 50.0f);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_SetManualOutput_WorksInManualMode(void) {
    ControlUnit_Init(control);
    ControlUnit_SetMode(control, CONTROL_MODE_MANUAL);

    ControlError result = ControlUnit_SetManualOutput(control, OUTPUT_CHANNEL_0, 60.0f);

    TEST_ASSERT_EQUAL(CONTROL_OK, result);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 60.0f, mockOutputPercent[OUTPUT_CHANNEL_0]);
}

// ========== Automatic Mode Tests ==========

void test_ControlUnit_Update_RejectsNullHandle(void) {
    ControlError result = ControlUnit_Update(NULL);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_Update_RequiresInitialization(void) {
    ControlError result = ControlUnit_Update(control);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NOT_INITIALIZED, result);
}

void test_ControlUnit_Update_DoesNothingInManualMode(void) {
    ControlUnit_Init(control);
    ControlUnit_SetMode(control, CONTROL_MODE_MANUAL);
    mockSensorPPFD = 500.0f;

    ControlError result = ControlUnit_Update(control);

    TEST_ASSERT_EQUAL(CONTROL_OK, result);
    // Output should remain at 0 (not updated)
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, mockOutputPercent[OUTPUT_CHANNEL_0]);
}

void test_ControlUnit_Update_AutomaticMode_IncreasesOutputWhenBelowSetpoint(void) {
    ControlUnit_Init(control);
    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);
    ControlUnit_SetSetpoint(control, 500);

    // Sensor reads 400 (below setpoint)
    mockSensorPPFD = 400.0f;

    ControlUnit_Update(control);

    // Output should increase (proportional control: Kp * error)
    // error = 500 - 400 = 100
    // output = 0.1 * 100 = 10%
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 10.0f, mockOutputPercent[OUTPUT_CHANNEL_0]);
}

void test_ControlUnit_Update_AutomaticMode_DecreasesOutputWhenAboveSetpoint(void) {
    ControlUnit_Init(control);
    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);
    ControlUnit_SetSetpoint(control, 500);

    // Start with some output
    ControlUnit_SetManualOutput(control, OUTPUT_CHANNEL_0, 50.0f);
    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);

    // Sensor reads 600 (above setpoint)
    mockSensorPPFD = 600.0f;

    ControlUnit_Update(control);

    // Output should decrease
    // error = 500 - 600 = -100
    // New output should be less than 50%
    TEST_ASSERT_LESS_THAN(50.0f, mockOutputPercent[OUTPUT_CHANNEL_0]);
}

void test_ControlUnit_Update_AutomaticMode_MaintainsOutputAtSetpoint(void) {
    ControlUnit_Init(control);
    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);
    ControlUnit_SetSetpoint(control, 500);

    // Set initial output
    ControlUnit_SetManualOutput(control, OUTPUT_CHANNEL_0, 30.0f);
    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);

    // Sensor exactly at setpoint
    mockSensorPPFD = 500.0f;

    ControlUnit_Update(control);

    // Output should remain approximately same (error = 0)
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 30.0f, mockOutputPercent[OUTPUT_CHANNEL_0]);
}

// ========== Status Monitoring Tests ==========

void test_ControlUnit_GetStatus_RejectsNullHandle(void) {
    ControlStatus status;
    ControlError result = ControlUnit_GetStatus(NULL, &status);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_GetStatus_RejectsNullPointer(void) {
    ControlUnit_Init(control);
    ControlError result = ControlUnit_GetStatus(control, NULL);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_GetStatus_ReturnsCurrentState(void) {
    ControlUnit_Init(control);
    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);
    ControlUnit_SetSetpoint(control, 750);

    ControlStatus status;
    ControlUnit_GetStatus(control, &status);

    TEST_ASSERT_EQUAL(CONTROL_MODE_AUTOMATIC, status.mode);
    TEST_ASSERT_EQUAL_UINT16(750, status.setpoint_umol);
}

// ========== Reset Tests ==========

void test_ControlUnit_Reset_RejectsNullHandle(void) {
    ControlError result = ControlUnit_Reset(NULL);
    TEST_ASSERT_EQUAL(CONTROL_ERROR_NULL_POINTER, result);
}

void test_ControlUnit_Reset_ClearsState(void) {
    ControlUnit_Init(control);
    ControlUnit_SetMode(control, CONTROL_MODE_AUTOMATIC);
    ControlUnit_SetSetpoint(control, 500);

    ControlUnit_Reset(control);

    ControlStatus status;
    ControlUnit_GetStatus(control, &status);

    // After reset, should be back to idle state
    TEST_ASSERT_EQUAL(CONTROL_STATE_IDLE, status.state);
}
