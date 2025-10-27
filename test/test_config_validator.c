#include "unity.h"
#include "config_validator.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_ConfigValidator_CanBeCreated(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();
    ConfigValidator_Destroy(validator);
    TEST_ASSERT_NOT_NULL(validator);
}

void test_ConfigValidator_HasKnownInitialState(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();
    uint16_t dli = 999;

    ConfigValidatorError result = ConfigValidator_GetDLI(validator, &dli);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);
    TEST_ASSERT_EQUAL_UINT16(0, dli);

    ConfigValidator_Destroy(validator);
}

void test_ConfigValidator_AcceptsValidDLI(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();
    uint16_t dli = 0;

    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 20);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(20, dli);

    ConfigValidator_Destroy(validator);
}

void test_ConfigValidator_AcceptsValidPhotoperiod(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();
    uint8_t photoperiod = 0;

    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(validator, 16);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    ConfigValidator_GetPhotoperiod(validator, &photoperiod);
    TEST_ASSERT_EQUAL_UINT8(16, photoperiod);

    ConfigValidator_Destroy(validator);
}

void test_ConfigValidator_AcceptsValidRiseTime(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();
    uint8_t riseTime = 0;

    ConfigValidatorError result = ConfigValidator_SetRiseTime(validator, 60);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    ConfigValidator_GetRiseTime(validator, &riseTime);
    TEST_ASSERT_EQUAL_UINT8(60, riseTime);

    ConfigValidator_Destroy(validator);
}

void test_ConfigValidator_GetDLI_RejectsNullHandle(void) {
    uint16_t dli = 0;
    ConfigValidatorError result = ConfigValidator_GetDLI(NULL, &dli);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

void test_ConfigValidator_GetDLI_RejectsNullPointer(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();
    ConfigValidatorError result = ConfigValidator_GetDLI(validator, NULL);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
    ConfigValidator_Destroy(validator);
}

void test_ConfigValidator_SetDLI_RejectsNullHandle(void) {
    ConfigValidatorError result = ConfigValidator_SetDLI(NULL, 20);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

// ========== Test 7: Reject DLI = 0 (invalid) ==========
void test_ConfigValidator_SetDLI_RejectsZero(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();

    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 0);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    // Verify DLI was NOT changed from initial state
    uint16_t dli = 999;
    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(0, dli);

    ConfigValidator_Destroy(validator);
}

// ========== Test 8: Accept minimum valid DLI ==========
void test_ConfigValidator_SetDLI_AcceptsMinimumValid(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();

    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 1);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint16_t dli = 0;
    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(1, dli);

    ConfigValidator_Destroy(validator);
}

// ========== Test 9: Accept maximum valid DLI ==========
void test_ConfigValidator_SetDLI_AcceptsMaximumValid(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();

    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 65535);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint16_t dli = 0;
    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(65535, dli);

    ConfigValidator_Destroy(validator);
}

// ========== Test 10: Overwrite previous DLI value ==========
void test_ConfigValidator_SetDLI_OverwritesPreviousValue(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();

    ConfigValidator_SetDLI(validator, 20);
    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 30);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint16_t dli = 0;
    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(30, dli);

    ConfigValidator_Destroy(validator);
}

// ========== Test 11: Reject photoperiod = 0 ==========
void test_ConfigValidator_SetPhotoperiod_RejectsZero(void) {
    ConfigValidatorHandle validator = ConfigValidator_Create();

    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(validator, 0);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    // Verify photoperiod was NOT changed
    uint8_t photoperiod = 99;
    ConfigValidator_GetPhotoperiod(validator, &photoperiod);
    TEST_ASSERT_EQUAL_UINT8(0, photoperiod);

    ConfigValidator_Destroy(validator);
}
