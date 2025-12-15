#include "output_driver.h"
#include "unity.h"
#include <string.h>

static OutputDriverHandle output;
static OutputHardwareInterface mockHwInterface;
static uint16_t mockRawValues[OUTPUT_CHANNEL_COUNT];

// Mock hardware function: stores raw values for verification
static void MockSetRawOutput(OutputChannel channel, uint16_t rawValue) {
    if (channel < OUTPUT_CHANNEL_COUNT) {
        mockRawValues[channel] = rawValue;
    }
}

void setUp(void) {
    // Clear mock state
    memset(mockRawValues, 0, sizeof(mockRawValues));

    // Setup mock hardware interface
    mockHwInterface.SetRawOutput = MockSetRawOutput;

    output = OutputDriver_Create(&mockHwInterface);
}

void tearDown(void) {
    OutputDriver_Destroy(output);
}

// ========== Lifecycle Tests ==========

void test_OutputDriver_CanBeCreated(void) {
    TEST_ASSERT_NOT_NULL(output);
}

void test_OutputDriver_Create_AcceptsNullHardwareInterface(void) {
    // Should allow NULL for testing scenarios
    OutputDriverHandle handle = OutputDriver_Create(NULL);
    TEST_ASSERT_NOT_NULL(handle);
    OutputDriver_Destroy(handle);
}

void test_OutputDriver_InitializesSuccessfully(void) {
    OutputDriverError result = OutputDriver_Init(output);
    TEST_ASSERT_EQUAL(OUTPUT_OK, result);
}

void test_OutputDriver_Init_RejectsNullHandle(void) {
    OutputDriverError result = OutputDriver_Init(NULL);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

// ========== Channel Control Tests ==========

void test_OutputDriver_SetChannel_RejectsNullHandle(void) {
    OutputDriverError result = OutputDriver_SetChannel(NULL, OUTPUT_CHANNEL_0, 50.0f);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_SetChannel_RequiresInitialization(void) {
    OutputDriverError result = OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 50.0f);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NOT_INITIALIZED, result);
}

void test_OutputDriver_SetChannel_RejectsInvalidChannel(void) {
    OutputDriver_Init(output);

    OutputDriverError result = OutputDriver_SetChannel(output, OUTPUT_CHANNEL_COUNT, 50.0f);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_INVALID_CHANNEL, result);
}

void test_OutputDriver_SetChannel_RejectsNegativePercentage(void) {
    OutputDriver_Init(output);

    OutputDriverError result = OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, -10.0f);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_OUT_OF_RANGE, result);
}

void test_OutputDriver_SetChannel_RejectsPercentageAbove100(void) {
    OutputDriver_Init(output);

    OutputDriverError result = OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 110.0f);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_OUT_OF_RANGE, result);
}

void test_OutputDriver_SetChannel_ZeroPercent(void) {
    OutputDriver_Init(output);

    OutputDriverError result = OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 0.0f);

    TEST_ASSERT_EQUAL(OUTPUT_OK, result);
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_0]);
}

void test_OutputDriver_SetChannel_FiftyPercent(void) {
    OutputDriver_Init(output);

    OutputDriverError result = OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 50.0f);

    TEST_ASSERT_EQUAL(OUTPUT_OK, result);
    // 50% of 4095 = 2047.5 ≈ 2048
    TEST_ASSERT_UINT16_WITHIN(1, 2048, mockRawValues[OUTPUT_CHANNEL_0]);
}

void test_OutputDriver_SetChannel_HundredPercent(void) {
    OutputDriver_Init(output);

    OutputDriverError result = OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 100.0f);

    TEST_ASSERT_EQUAL(OUTPUT_OK, result);
    TEST_ASSERT_EQUAL_UINT16(4095, mockRawValues[OUTPUT_CHANNEL_0]);
}

void test_OutputDriver_SetChannel_MultipleChannelsIndependent(void) {
    OutputDriver_Init(output);

    // Set different values on different channels
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 25.0f);
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_1, 75.0f);

    // Channel 0: 25% of 4095 ≈ 1024
    TEST_ASSERT_UINT16_WITHIN(1, 1024, mockRawValues[OUTPUT_CHANNEL_0]);

    // Channel 1: 75% of 4095 ≈ 3071
    TEST_ASSERT_UINT16_WITHIN(1, 3071, mockRawValues[OUTPUT_CHANNEL_1]);

    // Other channels should remain at 0
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_2]);
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_3]);
}

void test_OutputDriver_GetChannel_RejectsNullHandle(void) {
    float percent = 0.0f;
    OutputDriverError result = OutputDriver_GetChannel(NULL, OUTPUT_CHANNEL_0, &percent);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_GetChannel_RejectsNullPointer(void) {
    OutputDriver_Init(output);
    OutputDriverError result = OutputDriver_GetChannel(output, OUTPUT_CHANNEL_0, NULL);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_GetChannel_ReturnsSetValue(void) {
    OutputDriver_Init(output);

    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 60.0f);

    float percent = 0.0f;
    OutputDriverError result = OutputDriver_GetChannel(output, OUTPUT_CHANNEL_0, &percent);

    TEST_ASSERT_EQUAL(OUTPUT_OK, result);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 60.0f, percent);
}

void test_OutputDriver_GetChannel_DefaultIsZero(void) {
    OutputDriver_Init(output);

    float percent = 99.0f;
    OutputDriver_GetChannel(output, OUTPUT_CHANNEL_0, &percent);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, percent);
}

// ========== Enable/Disable Channel Tests ==========

void test_OutputDriver_EnableChannel_RejectsNullHandle(void) {
    OutputDriverError result = OutputDriver_EnableChannel(NULL, OUTPUT_CHANNEL_0, true);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_EnableChannel_RejectsInvalidChannel(void) {
    OutputDriver_Init(output);

    OutputDriverError result = OutputDriver_EnableChannel(output, OUTPUT_CHANNEL_COUNT, true);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_INVALID_CHANNEL, result);
}

void test_OutputDriver_IsChannelEnabled_RejectsNullHandle(void) {
    bool enabled = false;
    OutputDriverError result = OutputDriver_IsChannelEnabled(NULL, OUTPUT_CHANNEL_0, &enabled);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_IsChannelEnabled_RejectsNullPointer(void) {
    OutputDriver_Init(output);
    OutputDriverError result = OutputDriver_IsChannelEnabled(output, OUTPUT_CHANNEL_0, NULL);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_ChannelsEnabledByDefault(void) {
    OutputDriver_Init(output);

    bool enabled = false;
    OutputDriver_IsChannelEnabled(output, OUTPUT_CHANNEL_0, &enabled);

    TEST_ASSERT_TRUE(enabled);
}

void test_OutputDriver_DisableChannel_SetsOutputToZero(void) {
    OutputDriver_Init(output);

    // Set channel to 50%
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 50.0f);
    TEST_ASSERT_UINT16_WITHIN(1, 2048, mockRawValues[OUTPUT_CHANNEL_0]);

    // Disable channel
    OutputDriver_EnableChannel(output, OUTPUT_CHANNEL_0, false);

    // Output should now be 0
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_0]);
}

void test_OutputDriver_DisabledChannel_IgnoresSetChannel(void) {
    OutputDriver_Init(output);

    // Disable channel first
    OutputDriver_EnableChannel(output, OUTPUT_CHANNEL_0, false);

    // Try to set channel to 50%
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 50.0f);

    // Output should remain at 0 (disabled)
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_0]);
}

void test_OutputDriver_ReEnableChannel_RestoresPreviousValue(void) {
    OutputDriver_Init(output);

    // Set channel to 75%
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 75.0f);

    // Disable channel
    OutputDriver_EnableChannel(output, OUTPUT_CHANNEL_0, false);
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_0]);

    // Re-enable channel
    OutputDriver_EnableChannel(output, OUTPUT_CHANNEL_0, true);

    // Should restore to 75%
    TEST_ASSERT_UINT16_WITHIN(1, 3071, mockRawValues[OUTPUT_CHANNEL_0]);
}

// ========== Bulk Operations Tests ==========

void test_OutputDriver_SetAllChannels_RejectsNullHandle(void) {
    float percentages[OUTPUT_CHANNEL_COUNT] = {0, 0, 0, 0};
    OutputDriverError result = OutputDriver_SetAllChannels(NULL, percentages);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_SetAllChannels_RejectsNullArray(void) {
    OutputDriver_Init(output);
    OutputDriverError result = OutputDriver_SetAllChannels(output, NULL);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_SetAllChannels_SetsAllChannelsSimultaneously(void) {
    OutputDriver_Init(output);

    float percentages[OUTPUT_CHANNEL_COUNT] = {25.0f, 50.0f, 75.0f, 100.0f};

    OutputDriverError result = OutputDriver_SetAllChannels(output, percentages);

    TEST_ASSERT_EQUAL(OUTPUT_OK, result);

    // Verify each channel
    TEST_ASSERT_UINT16_WITHIN(1, 1024, mockRawValues[OUTPUT_CHANNEL_0]); // 25%
    TEST_ASSERT_UINT16_WITHIN(1, 2048, mockRawValues[OUTPUT_CHANNEL_1]); // 50%
    TEST_ASSERT_UINT16_WITHIN(1, 3071, mockRawValues[OUTPUT_CHANNEL_2]); // 75%
    TEST_ASSERT_EQUAL_UINT16(4095, mockRawValues[OUTPUT_CHANNEL_3]);     // 100%
}

void test_OutputDriver_SetAllChannels_RejectsOutOfRangeValues(void) {
    OutputDriver_Init(output);

    float percentages[OUTPUT_CHANNEL_COUNT] = {25.0f, 50.0f, 110.0f, 100.0f}; // 110% invalid

    OutputDriverError result = OutputDriver_SetAllChannels(output, percentages);

    TEST_ASSERT_EQUAL(OUTPUT_ERROR_OUT_OF_RANGE, result);
}

// ========== Emergency Stop Tests ==========

void test_OutputDriver_EmergencyStop_RejectsNullHandle(void) {
    OutputDriverError result = OutputDriver_EmergencyStop(NULL);
    TEST_ASSERT_EQUAL(OUTPUT_ERROR_NULL_POINTER, result);
}

void test_OutputDriver_EmergencyStop_SetsAllChannelsToZero(void) {
    OutputDriver_Init(output);

    // Set all channels to various values
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 30.0f);
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_1, 60.0f);
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_2, 90.0f);
    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_3, 100.0f);

    // Trigger emergency stop
    OutputDriverError result = OutputDriver_EmergencyStop(output);

    TEST_ASSERT_EQUAL(OUTPUT_OK, result);

    // All channels should be 0
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_0]);
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_1]);
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_2]);
    TEST_ASSERT_EQUAL_UINT16(0, mockRawValues[OUTPUT_CHANNEL_3]);
}

void test_OutputDriver_EmergencyStop_GetChannelReturnsZero(void) {
    OutputDriver_Init(output);

    OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 80.0f);
    OutputDriver_EmergencyStop(output);

    float percent = 99.0f;
    OutputDriver_GetChannel(output, OUTPUT_CHANNEL_0, &percent);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, percent);
}

// ========== Edge Cases and Precision Tests ==========

void test_OutputDriver_SetChannel_VerySmallPercentage(void) {
    OutputDriver_Init(output);

    OutputDriverError result = OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, 0.1f);

    TEST_ASSERT_EQUAL(OUTPUT_OK, result);
    // 0.1% of 4095 ≈ 4
    TEST_ASSERT_UINT16_WITHIN(1, 4, mockRawValues[OUTPUT_CHANNEL_0]);
}

void test_OutputDriver_SetChannel_PrecisionTest(void) {
    OutputDriver_Init(output);

    // Test various percentages
    float testValues[] = {10.0f, 33.3f, 66.7f, 90.0f};
    uint16_t expectedRaw[] = {410, 1364, 2731, 3686};

    for (int i = 0; i < 4; i++) {
        OutputDriver_SetChannel(output, OUTPUT_CHANNEL_0, testValues[i]);
        TEST_ASSERT_UINT16_WITHIN(2, expectedRaw[i], mockRawValues[OUTPUT_CHANNEL_0]);
    }
}

void test_OutputDriver_HardwareInterfaceNotCalledWhenNull(void) {
    // Create driver with NULL hardware interface
    OutputDriverHandle nullHwOutput = OutputDriver_Create(NULL);
    OutputDriver_Init(nullHwOutput);

    // This should not crash (hardware function is NULL)
    OutputDriverError result = OutputDriver_SetChannel(nullHwOutput, OUTPUT_CHANNEL_0, 50.0f);

    // Should succeed even without hardware
    TEST_ASSERT_EQUAL(OUTPUT_OK, result);

    OutputDriver_Destroy(nullHwOutput);
}
