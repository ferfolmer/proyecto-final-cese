#include "calibration_manager.h"
#include "sensor_driver.h"
#include "unity.h"
#include <string.h>

static CalibrationManagerHandle calibMgr;
static SensorDriverHandle mockSensor;
static SensorHardwareInterface mockSensorHw;
static float mockSensorRawPPFD;

// Mock sensor hardware
static float MockSensorReadRaw(void) {
    return mockSensorRawPPFD;
}

void setUp(void) {
    // Setup mock sensor
    mockSensorRawPPFD = 0.0f;
    mockSensorHw.ReadRawPPFD = MockSensorReadRaw;
    mockSensor = SensorDriver_Create(&mockSensorHw);
    SensorDriver_Init(mockSensor);

    // Create calibration manager
    calibMgr = CalibrationManager_Create(mockSensor);
}

void tearDown(void) {
    CalibrationManager_Destroy(calibMgr);
    SensorDriver_Destroy(mockSensor);
}

// ========== Lifecycle Tests ==========

void test_CalibrationManager_CanBeCreated(void) {
    TEST_ASSERT_NOT_NULL(calibMgr);
}

void test_CalibrationManager_Create_RejectsNullSensor(void) {
    CalibrationManagerHandle handle = CalibrationManager_Create(NULL);
    TEST_ASSERT_NULL(handle);
}

void test_CalibrationManager_InitializesSuccessfully(void) {
    CalibrationError result = CalibrationManager_Init(calibMgr);
    TEST_ASSERT_EQUAL(CALIBRATION_OK, result);
}

void test_CalibrationManager_Init_RejectsNullHandle(void) {
    CalibrationError result = CalibrationManager_Init(NULL);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NULL_POINTER, result);
}

// ========== Set Calibration Tests ==========

void test_CalibrationManager_SetCalibration_RejectsNullHandle(void) {
    CalibrationError result = CalibrationManager_SetCalibration(NULL, 0.0f, 1.0f, 1000);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NULL_POINTER, result);
}

void test_CalibrationManager_SetCalibration_RequiresInitialization(void) {
    CalibrationError result = CalibrationManager_SetCalibration(calibMgr, 0.0f, 1.0f, 1000);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NOT_INITIALIZED, result);
}

void test_CalibrationManager_SetCalibration_RejectsZeroGain(void) {
    CalibrationManager_Init(calibMgr);

    CalibrationError result = CalibrationManager_SetCalibration(calibMgr, 0.0f, 0.0f, 1000);

    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_INVALID_GAIN, result);
}

void test_CalibrationManager_SetCalibration_RejectsNegativeGain(void) {
    CalibrationManager_Init(calibMgr);

    CalibrationError result = CalibrationManager_SetCalibration(calibMgr, 0.0f, -1.0f, 1000);

    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_INVALID_GAIN, result);
}

void test_CalibrationManager_SetCalibration_AcceptsValidData(void) {
    CalibrationManager_Init(calibMgr);

    CalibrationError result = CalibrationManager_SetCalibration(calibMgr, 5.0f, 0.98f, 12345);

    TEST_ASSERT_EQUAL(CALIBRATION_OK, result);
}

void test_CalibrationManager_SetCalibration_AcceptsZeroOffset(void) {
    CalibrationManager_Init(calibMgr);

    CalibrationError result = CalibrationManager_SetCalibration(calibMgr, 0.0f, 1.0f, 0);

    TEST_ASSERT_EQUAL(CALIBRATION_OK, result);
}

void test_CalibrationManager_SetCalibration_AcceptsNegativeOffset(void) {
    CalibrationManager_Init(calibMgr);

    CalibrationError result = CalibrationManager_SetCalibration(calibMgr, -10.0f, 1.0f, 0);

    TEST_ASSERT_EQUAL(CALIBRATION_OK, result);
}

// ========== Get Calibration Tests ==========

void test_CalibrationManager_GetCalibration_RejectsNullHandle(void) {
    CalibrationInfo cal;
    CalibrationError result = CalibrationManager_GetCalibration(NULL, &cal);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NULL_POINTER, result);
}

void test_CalibrationManager_GetCalibration_RejectsNullPointer(void) {
    CalibrationManager_Init(calibMgr);

    CalibrationError result = CalibrationManager_GetCalibration(calibMgr, NULL);

    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NULL_POINTER, result);
}

void test_CalibrationManager_GetCalibration_ReturnsStoredValues(void) {
    CalibrationManager_Init(calibMgr);
    CalibrationManager_SetCalibration(calibMgr, 10.5f, 0.96f, 9999);

    CalibrationInfo cal;
    CalibrationError result = CalibrationManager_GetCalibration(calibMgr, &cal);

    TEST_ASSERT_EQUAL(CALIBRATION_OK, result);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.5f, cal.data.offset_umol);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.96f, cal.data.gain);
    TEST_ASSERT_EQUAL_UINT32(9999, cal.data.timestamp_ms);
    TEST_ASSERT_TRUE(cal.valid);
}

void test_CalibrationManager_GetCalibration_InvalidByDefault(void) {
    CalibrationManager_Init(calibMgr);

    CalibrationInfo cal;
    CalibrationManager_GetCalibration(calibMgr, &cal);

    TEST_ASSERT_FALSE(cal.valid);
}

// ========== Apply to Sensor Tests ==========

void test_CalibrationManager_ApplyToSensor_RejectsNullHandle(void) {
    CalibrationError result = CalibrationManager_ApplyToSensor(NULL);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NULL_POINTER, result);
}

void test_CalibrationManager_ApplyToSensor_RequiresInitialization(void) {
    CalibrationError result = CalibrationManager_ApplyToSensor(calibMgr);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NOT_INITIALIZED, result);
}

void test_CalibrationManager_ApplyToSensor_WritesToSensorDriver(void) {
    CalibrationManager_Init(calibMgr);
    CalibrationManager_SetCalibration(calibMgr, 8.0f, 0.99f, 5000);

    CalibrationError result = CalibrationManager_ApplyToSensor(calibMgr);

    TEST_ASSERT_EQUAL(CALIBRATION_OK, result);

    // Verify sensor received calibration
    CalibrationData sensorCal;
    SensorDriver_GetCalibration(mockSensor, &sensorCal);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 8.0f, sensorCal.offset_umol);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.99f, sensorCal.gain);
}

void test_CalibrationManager_ApplyToSensor_WorksMultipleTimes(void) {
    CalibrationManager_Init(calibMgr);

    // First calibration
    CalibrationManager_SetCalibration(calibMgr, 5.0f, 0.95f, 1000);
    CalibrationManager_ApplyToSensor(calibMgr);

    // Second calibration
    CalibrationManager_SetCalibration(calibMgr, 10.0f, 1.05f, 2000);
    CalibrationManager_ApplyToSensor(calibMgr);

    // Verify latest calibration applied
    CalibrationData sensorCal;
    SensorDriver_GetCalibration(mockSensor, &sensorCal);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, sensorCal.offset_umol);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.05f, sensorCal.gain);
}

// ========== Validation Tests ==========

void test_CalibrationManager_IsValid_RejectsNullHandle(void) {
    bool isValid = false;
    CalibrationError result = CalibrationManager_IsValid(NULL, &isValid);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NULL_POINTER, result);
}

void test_CalibrationManager_IsValid_RejectsNullPointer(void) {
    CalibrationManager_Init(calibMgr);

    CalibrationError result = CalibrationManager_IsValid(calibMgr, NULL);

    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NULL_POINTER, result);
}

void test_CalibrationManager_IsValid_ReturnsFalseByDefault(void) {
    CalibrationManager_Init(calibMgr);

    bool isValid = true;
    CalibrationManager_IsValid(calibMgr, &isValid);

    TEST_ASSERT_FALSE(isValid);
}

void test_CalibrationManager_IsValid_ReturnsTrueAfterSetCalibration(void) {
    CalibrationManager_Init(calibMgr);
    CalibrationManager_SetCalibration(calibMgr, 0.0f, 1.0f, 0);

    bool isValid = false;
    CalibrationManager_IsValid(calibMgr, &isValid);

    TEST_ASSERT_TRUE(isValid);
}

// ========== Clear Calibration Tests ==========

void test_CalibrationManager_ClearCalibration_RejectsNullHandle(void) {
    CalibrationError result = CalibrationManager_ClearCalibration(NULL);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NULL_POINTER, result);
}

void test_CalibrationManager_ClearCalibration_RequiresInitialization(void) {
    CalibrationError result = CalibrationManager_ClearCalibration(calibMgr);
    TEST_ASSERT_EQUAL(CALIBRATION_ERROR_NOT_INITIALIZED, result);
}

void test_CalibrationManager_ClearCalibration_InvalidatesData(void) {
    CalibrationManager_Init(calibMgr);
    CalibrationManager_SetCalibration(calibMgr, 5.0f, 0.98f, 1000);

    CalibrationError result = CalibrationManager_ClearCalibration(calibMgr);

    TEST_ASSERT_EQUAL(CALIBRATION_OK, result);

    // Verify calibration is now invalid
    bool isValid = true;
    CalibrationManager_IsValid(calibMgr, &isValid);
    TEST_ASSERT_FALSE(isValid);
}

void test_CalibrationManager_ClearCalibration_ResetsToDefaults(void) {
    CalibrationManager_Init(calibMgr);
    CalibrationManager_SetCalibration(calibMgr, 10.0f, 0.95f, 5000);
    CalibrationManager_ClearCalibration(calibMgr);

    CalibrationInfo cal;
    CalibrationManager_GetCalibration(calibMgr, &cal);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, cal.data.offset_umol);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, cal.data.gain);
    TEST_ASSERT_EQUAL_UINT32(0, cal.data.timestamp_ms);
    TEST_ASSERT_FALSE(cal.valid);
}

// ========== Integration Tests ==========

void test_CalibrationManager_WorkflowIntegration(void) {
    // Complete workflow: Init → Set → Apply → Verify
    CalibrationManager_Init(calibMgr);

    // User provides calibration data
    CalibrationManager_SetCalibration(calibMgr, 7.5f, 0.97f, 12345);

    // Apply to sensor
    CalibrationManager_ApplyToSensor(calibMgr);

    // Verify sensor is using calibration
    mockSensorRawPPFD = 500.0f; // Sensor reads 500
    uint16_t ppfd = 0;
    SensorDriver_ReadPPFD(mockSensor, &ppfd);

    // Expected: (500 - 7.5) * 0.97 = 492.5 * 0.97 = 477.725 ≈ 478
    TEST_ASSERT_UINT16_WITHIN(1, 478, ppfd);
}

void test_CalibrationManager_MultipleCalibrationUpdates(void) {
    CalibrationManager_Init(calibMgr);

    // First calibration
    CalibrationManager_SetCalibration(calibMgr, 0.0f, 1.0f, 1000);
    CalibrationManager_ApplyToSensor(calibMgr);

    mockSensorRawPPFD = 500.0f;
    uint16_t ppfd1 = 0;
    SensorDriver_ReadPPFD(mockSensor, &ppfd1);
    TEST_ASSERT_EQUAL_UINT16(500, ppfd1); // No correction

    // Update calibration
    CalibrationManager_SetCalibration(calibMgr, 0.0f, 0.9f, 2000);
    CalibrationManager_ApplyToSensor(calibMgr);

    mockSensorRawPPFD = 500.0f;
    uint16_t ppfd2 = 0;
    SensorDriver_ReadPPFD(mockSensor, &ppfd2);
    TEST_ASSERT_EQUAL_UINT16(450, ppfd2); // 500 * 0.9 = 450
}

void test_CalibrationManager_ClearAndReapply(void) {
    CalibrationManager_Init(calibMgr);

    // Set calibration
    CalibrationManager_SetCalibration(calibMgr, 10.0f, 0.95f, 1000);
    CalibrationManager_ApplyToSensor(calibMgr);

    // Clear
    CalibrationManager_ClearCalibration(calibMgr);
    CalibrationManager_ApplyToSensor(calibMgr); // Apply cleared (default) calibration

    // Verify sensor uses default calibration (offset=0, gain=1.0)
    mockSensorRawPPFD = 500.0f;
    uint16_t ppfd = 0;
    SensorDriver_ReadPPFD(mockSensor, &ppfd);
    TEST_ASSERT_EQUAL_UINT16(500, ppfd); // No correction applied
}
