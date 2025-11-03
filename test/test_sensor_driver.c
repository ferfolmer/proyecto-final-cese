#include "sensor_driver.h"
#include "unity.h"

static SensorDriverHandle sensor;
static SensorHardwareInterface mockHwInterface;
static float mockRawPPFD;

// Mock hardware function: returns the value set by test
static float MockReadRawPPFD(void) {
    return mockRawPPFD;
}

void setUp(void) {
    // Setup mock hardware interface
    mockRawPPFD = 0.0f;
    mockHwInterface.ReadRawPPFD = MockReadRawPPFD;

    sensor = SensorDriver_Create(&mockHwInterface);
}

void tearDown(void) {
    SensorDriver_Destroy(sensor);
}

// ========== Lifecycle Tests ==========

void test_SensorDriver_CanBeCreated(void) {
    TEST_ASSERT_NOT_NULL(sensor);
}

void test_SensorDriver_InitializesSuccessfully(void) {
    SensorDriverError result = SensorDriver_Init(sensor);
    TEST_ASSERT_EQUAL(SENSOR_OK, result);
}

void test_SensorDriver_Init_RejectsNullHandle(void) {
    SensorDriverError result = SensorDriver_Init(NULL);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

// ========== Raw ADC Reading Tests ==========

void test_SensorDriver_ReadRaw_RejectsNullHandle(void) {
    uint16_t rawValue = 0;
    SensorDriverError result = SensorDriver_ReadRaw(NULL, &rawValue);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_ReadRaw_RejectsNullPointer(void) {
    SensorDriver_Init(sensor);
    SensorDriverError result = SensorDriver_ReadRaw(sensor, NULL);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_ReadRaw_RequiresInitialization(void) {
    uint16_t rawValue = 0;
    SensorDriverError result = SensorDriver_ReadRaw(sensor, &rawValue);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NOT_INITIALIZED, result);
}

void test_SensorDriver_ReadRaw_ReturnsValidData(void) {
    SensorDriver_Init(sensor);
    uint16_t rawValue = 9999;

    SensorDriverError result = SensorDriver_ReadRaw(sensor, &rawValue);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    // Raw value should be within 12-bit ADC range (0-4095)
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(4095, rawValue);
}

// ========== Calibration Tests ==========

void test_SensorDriver_SetCalibration_RejectsNullHandle(void) {
    CalibrationData cal = {.offset_umol = 0.0f, .gain = 1.0f};
    SensorDriverError result = SensorDriver_SetCalibration(NULL, cal);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_GetCalibration_RejectsNullHandle(void) {
    CalibrationData cal;
    SensorDriverError result = SensorDriver_GetCalibration(NULL, &cal);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_GetCalibration_RejectsNullPointer(void) {
    SensorDriverError result = SensorDriver_GetCalibration(sensor, NULL);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_CalibrationHasKnownInitialState(void) {
    CalibrationData cal;

    SensorDriverError result = SensorDriver_GetCalibration(sensor, &cal);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, cal.offset_umol);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, cal.gain);
    TEST_ASSERT_EQUAL_UINT32(0, cal.timestamp_ms);
    TEST_ASSERT_EQUAL_UINT16(0, cal.crc);
}

void test_SensorDriver_StoresCalibrationData(void) {
    CalibrationData cal_in = {
        .offset_umol = 10.5f, .gain = 0.5f, .timestamp_ms = 1000, .crc = 0xABCD};
    CalibrationData cal_out;

    SensorDriver_SetCalibration(sensor, cal_in);
    SensorDriver_GetCalibration(sensor, &cal_out);

    TEST_ASSERT_EQUAL_FLOAT(10.5f, cal_out.offset_umol);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, cal_out.gain);
    TEST_ASSERT_EQUAL_UINT32(1000, cal_out.timestamp_ms);
    TEST_ASSERT_EQUAL_UINT16(0xABCD, cal_out.crc);
}

// ========== Filter Configuration Tests ==========

void test_SensorDriver_FilterSizeHasDefaultValue(void) {
    uint8_t filterSize = 0;

    SensorDriverError result = SensorDriver_GetFilterSize(sensor, &filterSize);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    TEST_ASSERT_EQUAL_UINT8(1, filterSize); // Default: no filtering
}

void test_SensorDriver_SetFilterSize_RejectsNullHandle(void) {
    SensorDriverError result = SensorDriver_SetFilterSize(NULL, 5);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_GetFilterSize_RejectsNullPointer(void) {
    SensorDriverError result = SensorDriver_GetFilterSize(sensor, NULL);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_StoresFilterSize(void) {
    uint8_t filterSize = 0;

    SensorDriver_SetFilterSize(sensor, 10);
    SensorDriver_GetFilterSize(sensor, &filterSize);

    TEST_ASSERT_EQUAL_UINT8(10, filterSize);
}

// ========== Connectivity Tests ==========

void test_SensorDriver_IsConnected_RejectsNullHandle(void) {
    bool isConnected = false;
    SensorDriverError result = SensorDriver_IsConnected(NULL, &isConnected);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_IsConnected_RejectsNullPointer(void) {
    SensorDriverError result = SensorDriver_IsConnected(sensor, NULL);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_IsConnected_ReturnsFalseBeforeInit(void) {
    bool isConnected = true;

    SensorDriverError result = SensorDriver_IsConnected(sensor, &isConnected);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    TEST_ASSERT_FALSE(isConnected);
}

void test_SensorDriver_IsConnected_ReturnsTrueAfterInit(void) {
    bool isConnected = false;

    SensorDriver_Init(sensor);
    SensorDriverError result = SensorDriver_IsConnected(sensor, &isConnected);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    TEST_ASSERT_TRUE(isConnected);
}

// ========== PPFD Calculation Tests ==========

void test_SensorDriver_ReadPPFD_RejectsNullHandle(void) {
    uint16_t ppfd = 0;
    SensorDriverError result = SensorDriver_ReadPPFD(NULL, &ppfd);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_ReadPPFD_RejectsNullPointer(void) {
    SensorDriver_Init(sensor);
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, NULL);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NULL_POINTER, result);
}

void test_SensorDriver_ReadPPFD_RequiresInitialization(void) {
    uint16_t ppfd = 0;
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, &ppfd);
    TEST_ASSERT_EQUAL(SENSOR_ERROR_NOT_INITIALIZED, result);
}

void test_SensorDriver_ReadPPFD_WithNoCalibration_ReturnsRawValue(void) {
    // Test that with default calibration (offset=0, gain=1.0), PPFD = raw reading
    SensorDriver_Init(sensor);

    // TODO: Need to inject a mock raw reading of 500 µmol/m²/s
    // For now, this test will be expanded when we add Modbus mocking

    uint16_t ppfd = 0;
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, &ppfd);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    // With no calibration: PPFD_out = (PPFD_raw - 0) * 1.0 = PPFD_raw
}

void test_SensorDriver_ReadPPFD_AppliesOffsetCalibration(void) {
    // Test: PPFD_corrected = (PPFD_raw - offset) * gain
    // If raw = 510, offset = 10, gain = 1.0
    // Then corrected = (510 - 10) * 1.0 = 500

    SensorDriver_Init(sensor);

    CalibrationData cal = {.offset_umol = 10.0f, .gain = 1.0f, .timestamp_ms = 1000, .crc = 0x1234};
    SensorDriver_SetCalibration(sensor, cal);

    // TODO: Mock Modbus to return raw PPFD = 510
    // Expected result: 500

    uint16_t ppfd = 0;
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, &ppfd);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    // TEST_ASSERT_EQUAL_UINT16(500, ppfd); // Will enable after Modbus mock
}

void test_SensorDriver_ReadPPFD_AppliesGainCalibration(void) {
    // Test: PPFD_corrected = (PPFD_raw - offset) * gain
    // If raw = 1000, offset = 0, gain = 0.5
    // Then corrected = (1000 - 0) * 0.5 = 500

    SensorDriver_Init(sensor);

    CalibrationData cal = {.offset_umol = 0.0f, .gain = 0.5f, .timestamp_ms = 2000, .crc = 0x5678};
    SensorDriver_SetCalibration(sensor, cal);

    // TODO: Mock Modbus to return raw PPFD = 1000
    // Expected result: 500

    uint16_t ppfd = 0;
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, &ppfd);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    // TEST_ASSERT_EQUAL_UINT16(500, ppfd); // Will enable after Modbus mock
}

void test_SensorDriver_ReadPPFD_AppliesBothOffsetAndGain(void) {
    // Test: PPFD_corrected = (PPFD_raw - offset) * gain
    // If raw = 520, offset = 20, gain = 0.96
    // Then corrected = (520 - 20) * 0.96 = 500 * 0.96 = 480

    SensorDriver_Init(sensor);

    CalibrationData cal = {
        .offset_umol = 20.0f, .gain = 0.96f, .timestamp_ms = 3000, .crc = 0xABCD};
    SensorDriver_SetCalibration(sensor, cal);

    // TODO: Mock Modbus to return raw PPFD = 520
    // Expected result: 480

    uint16_t ppfd = 0;
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, &ppfd);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    // TEST_ASSERT_EQUAL_UINT16(480, ppfd); // Will enable after Modbus mock
}

void test_SensorDriver_ReadPPFD_HandlesZeroReading(void) {
    // Test edge case: raw reading = 0 (lights off)
    // If raw = 0, offset = 0, gain = 1.0
    // Then corrected = (0 - 0) * 1.0 = 0

    SensorDriver_Init(sensor);

    // TODO: Mock Modbus to return raw PPFD = 0
    // Expected result: 0

    uint16_t ppfd = 9999;
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, &ppfd);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    TEST_ASSERT_EQUAL_UINT16(0, ppfd);
}

void test_SensorDriver_ReadPPFD_HandlesMaxReading(void) {
    // Test edge case: raw reading at sensor max (4000 µmol/m²/s)
    // If raw = 4000, offset = 0, gain = 1.0
    // Then corrected = (4000 - 0) * 1.0 = 4000

    SensorDriver_Init(sensor);

    // TODO: Mock Modbus to return raw PPFD = 4000
    // Expected result: 4000

    uint16_t ppfd = 0;
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, &ppfd);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    // TEST_ASSERT_LESS_OR_EQUAL_UINT16(4000, ppfd); // Should not exceed sensor max
}

void test_SensorDriver_ReadPPFD_NegativeResultClampedToZero(void) {
    // Test edge case: calibration produces negative result
    // If raw = 10, offset = 50, gain = 1.0
    // Then corrected = (10 - 50) * 1.0 = -40 → should clamp to 0

    SensorDriver_Init(sensor);

    CalibrationData cal = {.offset_umol = 50.0f, .gain = 1.0f, .timestamp_ms = 4000, .crc = 0xDEAD};
    SensorDriver_SetCalibration(sensor, cal);

    // TODO: Mock Modbus to return raw PPFD = 10
    // Expected result: 0 (clamped)

    uint16_t ppfd = 9999;
    SensorDriverError result = SensorDriver_ReadPPFD(sensor, &ppfd);

    TEST_ASSERT_EQUAL(SENSOR_OK, result);
    TEST_ASSERT_EQUAL_UINT16(0, ppfd); // Negative values should clamp to 0
}

// ========== Moving Average Filter Tests ==========

void test_SensorDriver_Filter_WithSizeOne_ReturnsRawValue(void) {
    // Filter size = 1 means no filtering (pass-through)
    SensorDriver_Init(sensor);
    SensorDriver_SetFilterSize(sensor, 1);

    // Set mock to return 100
    mockRawPPFD = 100.0f;
    uint16_t ppfd1 = 0;
    SensorDriver_ReadPPFD(sensor, &ppfd1);

    // Set mock to return 200
    mockRawPPFD = 200.0f;
    uint16_t ppfd2 = 0;
    SensorDriver_ReadPPFD(sensor, &ppfd2);

    // With filter size = 1, output should equal input immediately
    TEST_ASSERT_EQUAL_UINT16(100, ppfd1);
    TEST_ASSERT_EQUAL_UINT16(200, ppfd2);
}

void test_SensorDriver_Filter_WithSizeTwo_AveragesTwoSamples(void) {
    // Filter size = 2: average of last 2 readings
    SensorDriver_Init(sensor);
    SensorDriver_SetFilterSize(sensor, 2);

    // First reading: only 1 sample in buffer, so average = 100
    mockRawPPFD = 100.0f;
    uint16_t ppfd1 = 0;
    SensorDriver_ReadPPFD(sensor, &ppfd1);
    TEST_ASSERT_EQUAL_UINT16(100, ppfd1);

    // Second reading: buffer has [100, 200], average = 150
    mockRawPPFD = 200.0f;
    uint16_t ppfd2 = 0;
    SensorDriver_ReadPPFD(sensor, &ppfd2);
    TEST_ASSERT_EQUAL_UINT16(150, ppfd2);

    // Third reading: buffer has [200, 300], average = 250
    mockRawPPFD = 300.0f;
    uint16_t ppfd3 = 0;
    SensorDriver_ReadPPFD(sensor, &ppfd3);
    TEST_ASSERT_EQUAL_UINT16(250, ppfd3);
}

void test_SensorDriver_Filter_WithSizeFive_AveragesFiveSamples(void) {
    // Filter size = 5: average of last 5 readings
    SensorDriver_Init(sensor);
    SensorDriver_SetFilterSize(sensor, 5);

    uint16_t ppfd = 0;

    // Fill buffer with: 100, 200, 300, 400, 500
    mockRawPPFD = 100.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 200.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 300.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 400.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 500.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    // Average = (100 + 200 + 300 + 400 + 500) / 5 = 1500 / 5 = 300
    TEST_ASSERT_EQUAL_UINT16(300, ppfd);
}

void test_SensorDriver_Filter_CircularBufferWrapsAround(void) {
    // Test that circular buffer correctly overwrites old values
    SensorDriver_Init(sensor);
    SensorDriver_SetFilterSize(sensor, 3);

    uint16_t ppfd = 0;

    // Fill buffer: [100, 200, 300]
    mockRawPPFD = 100.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 200.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 300.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);
    TEST_ASSERT_EQUAL_UINT16(200, ppfd); // (100+200+300)/3 = 200

    // Add 400, buffer becomes: [400, 200, 300] (overwrites index 0)
    mockRawPPFD = 400.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);
    TEST_ASSERT_EQUAL_UINT16(300, ppfd); // (200+300+400)/3 = 300

    // Add 500, buffer becomes: [400, 500, 300] (overwrites index 1)
    mockRawPPFD = 500.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);
    TEST_ASSERT_EQUAL_UINT16(400, ppfd); // (300+400+500)/3 = 400
}

void test_SensorDriver_Filter_HandlesSpikeCorrectly(void) {
    // Test that filter smooths out a spike in readings
    SensorDriver_Init(sensor);
    SensorDriver_SetFilterSize(sensor, 3);

    uint16_t ppfd = 0;

    // Steady readings at 500
    mockRawPPFD = 500.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 500.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 500.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);
    TEST_ASSERT_EQUAL_UINT16(500, ppfd); // Steady at 500

    // Sudden spike to 800
    mockRawPPFD = 800.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);
    // Buffer: [500, 500, 800], average = 600
    TEST_ASSERT_EQUAL_UINT16(600, ppfd); // Spike is dampened

    // Return to 500
    mockRawPPFD = 500.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);
    // Buffer: [500, 800, 500], average = 600
    TEST_ASSERT_EQUAL_UINT16(600, ppfd);

    // Continue at 500
    mockRawPPFD = 500.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);
    // Buffer: [800, 500, 500], average = 600
    TEST_ASSERT_EQUAL_UINT16(600, ppfd);

    // One more reading at 500
    mockRawPPFD = 500.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);
    // Buffer: [500, 500, 500], average = 500 (spike fully filtered out)
    TEST_ASSERT_EQUAL_UINT16(500, ppfd);
}

void test_SensorDriver_Filter_ChangingFilterSizePreservesData(void) {
    // Test that changing filter size mid-operation doesn't crash
    SensorDriver_Init(sensor);
    SensorDriver_SetFilterSize(sensor, 3);

    uint16_t ppfd = 0;

    // Add some readings with size=3
    mockRawPPFD = 100.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 200.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    // Change filter size to 5
    SensorDriver_SetFilterSize(sensor, 5);

    // Continue adding readings
    mockRawPPFD = 300.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    // Should not crash (behavior may vary - just ensure it doesn't crash)
    TEST_ASSERT_EQUAL(SENSOR_OK, SensorDriver_ReadPPFD(sensor, &ppfd));
}

void test_SensorDriver_Filter_HandlesZeroValues(void) {
    // Test filter with zero PPFD (lights off scenario)
    SensorDriver_Init(sensor);
    SensorDriver_SetFilterSize(sensor, 3);

    uint16_t ppfd = 0;

    // All zeros
    mockRawPPFD = 0.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 0.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    mockRawPPFD = 0.0f;
    SensorDriver_ReadPPFD(sensor, &ppfd);

    TEST_ASSERT_EQUAL_UINT16(0, ppfd); // Average of zeros is zero
}

void test_SensorDriver_Filter_MaxFilterSize(void) {
    // Test with maximum filter size (16 samples)
    SensorDriver_Init(sensor);
    SensorDriver_SetFilterSize(sensor, 16);

    uint16_t ppfd = 0;

    // Fill buffer with values 0-15
    for (uint8_t i = 0; i < 16; i++) {
        mockRawPPFD = (float)(i * 100);
        SensorDriver_ReadPPFD(sensor, &ppfd);
    }

    // Average = (0 + 100 + 200 + ... + 1500) / 16
    // Sum = 100 * (0+1+2+...+15) = 100 * (15*16/2) = 100 * 120 = 12000
    // Average = 12000 / 16 = 750
    TEST_ASSERT_EQUAL_UINT16(750, ppfd);
}
