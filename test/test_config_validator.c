#include "unity.h"
#include "config_validator.h"

static ConfigValidatorHandle validator;

void setUp(void) {
    validator = ConfigValidator_Create();
}

void tearDown(void) {
}

void test_ConfigValidator_CanBeCreated(void) {
    TEST_ASSERT_NOT_NULL(validator);
}

void test_ConfigValidator_HasKnownInitialState(void) {
    uint16_t dli = 999;

    ConfigValidatorError result = ConfigValidator_GetDLI(validator, &dli);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);
    TEST_ASSERT_EQUAL_UINT16(0, dli);
}

void test_ConfigValidator_AcceptsValidDLI(void) {
    uint16_t dli = 0;

    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 20);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(20, dli);
}

void test_ConfigValidator_AcceptsValidPhotoperiod(void) {
    uint8_t photoperiod = 0;

    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(validator, 16);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    ConfigValidator_GetPhotoperiod(validator, &photoperiod);
    TEST_ASSERT_EQUAL_UINT8(16, photoperiod);
}

void test_ConfigValidator_AcceptsValidRiseTime(void) {
    uint8_t riseTime = 0;

    ConfigValidatorError result = ConfigValidator_SetRiseTime(validator, 60);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    ConfigValidator_GetRiseTime(validator, &riseTime);
    TEST_ASSERT_EQUAL_UINT8(60, riseTime);
}

void test_ConfigValidator_GetDLI_RejectsNullHandle(void) {
    uint16_t dli = 0;
    ConfigValidatorError result = ConfigValidator_GetDLI(NULL, &dli);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

void test_ConfigValidator_GetDLI_RejectsNullPointer(void) {
    ConfigValidatorError result = ConfigValidator_GetDLI(validator, NULL);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

void test_ConfigValidator_SetDLI_RejectsNullHandle(void) {
    ConfigValidatorError result = ConfigValidator_SetDLI(NULL, 20);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

// ========== Test 7: Reject DLI = 0 (invalid) ==========
void test_ConfigValidator_SetDLI_RejectsZero(void) {
    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 0);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    // Verify DLI was NOT changed from initial state
    uint16_t dli = 999;
    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(0, dli);
}

// ========== Test 8: Accept minimum valid DLI ==========
void test_ConfigValidator_SetDLI_AcceptsMinimumValid(void) {
    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 1);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint16_t dli = 0;
    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(1, dli);
}

// ========== Test 9: Accept maximum valid DLI ==========
void test_ConfigValidator_SetDLI_AcceptsMaximumValid(void) {

    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 65535);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint16_t dli = 0;
    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(65535, dli);
}

// ========== Test 10: Overwrite previous DLI value ==========
void test_ConfigValidator_SetDLI_OverwritesPreviousValue(void) {

    ConfigValidator_SetDLI(validator, 20);
    ConfigValidatorError result = ConfigValidator_SetDLI(validator, 30);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint16_t dli = 0;
    ConfigValidator_GetDLI(validator, &dli);
    TEST_ASSERT_EQUAL_UINT16(30, dli);
}

// ========== Test 11: Reject photoperiod = 0 ==========
void test_ConfigValidator_SetPhotoperiod_RejectsZero(void) {

    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(validator, 0);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    // Verify photoperiod was NOT changed
    uint8_t photoperiod = 99;
    ConfigValidator_GetPhotoperiod(validator, &photoperiod);
    TEST_ASSERT_EQUAL_UINT8(0, photoperiod);
}

// ========== Test 12: GetPhotoperiod rejects null handle ==========
void test_ConfigValidator_GetPhotoperiod_RejectsNullHandle(void) {
    uint8_t photoperiod = 0;
    ConfigValidatorError result = ConfigValidator_GetPhotoperiod(NULL, &photoperiod);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

// ========== Test 13: GetPhotoperiod rejects null pointer ==========
void test_ConfigValidator_GetPhotoperiod_RejectsNullPointer(void) {
    ConfigValidatorError result = ConfigValidator_GetPhotoperiod(validator, NULL);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

// ========== Test 14: SetPhotoperiod rejects null handle ==========
void test_ConfigValidator_SetPhotoperiod_RejectsNullHandle(void) {
    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(NULL, 16);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

// ========== Test 15: Accept minimum photoperiod (1 hour) ==========
void test_ConfigValidator_SetPhotoperiod_AcceptsMinimum(void) {

    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(validator, 1);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint8_t photoperiod = 0;
    ConfigValidator_GetPhotoperiod(validator, &photoperiod);
    TEST_ASSERT_EQUAL_UINT8(1, photoperiod);
}

// ========== Test 16: Accept maximum photoperiod (24 hours) ==========
void test_ConfigValidator_SetPhotoperiod_AcceptsMaximum(void) {

    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(validator, 24);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint8_t photoperiod = 0;
    ConfigValidator_GetPhotoperiod(validator, &photoperiod);
    TEST_ASSERT_EQUAL_UINT8(24, photoperiod);
}

// ========== Test 17: Reject photoperiod > 24 hours ==========
void test_ConfigValidator_SetPhotoperiod_RejectsExcessiveValue(void) {

    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(validator, 25);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    // Verify photoperiod was NOT changed
    uint8_t photoperiod = 99;
    ConfigValidator_GetPhotoperiod(validator, &photoperiod);
    TEST_ASSERT_EQUAL_UINT8(0, photoperiod);
}

// ========== Test 18: Photoperiod overwrite works ==========
void test_ConfigValidator_SetPhotoperiod_OverwritesPreviousValue(void) {

    ConfigValidator_SetPhotoperiod(validator, 12);
    ConfigValidatorError result = ConfigValidator_SetPhotoperiod(validator, 18);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint8_t photoperiod = 0;
    ConfigValidator_GetPhotoperiod(validator, &photoperiod);
    TEST_ASSERT_EQUAL_UINT8(18, photoperiod);
}

// ========== Test 19: GetRiseTime rejects null handle ==========
void test_ConfigValidator_GetRiseTime_RejectsNullHandle(void) {
    uint8_t riseTime = 0;
    ConfigValidatorError result = ConfigValidator_GetRiseTime(NULL, &riseTime);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

// ========== Test 20: GetRiseTime rejects null pointer ==========
void test_ConfigValidator_GetRiseTime_RejectsNullPointer(void) {
    ConfigValidatorError result = ConfigValidator_GetRiseTime(validator, NULL);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

// ========== Test 21: SetRiseTime rejects null handle ==========
void test_ConfigValidator_SetRiseTime_RejectsNullHandle(void) {
    ConfigValidatorError result = ConfigValidator_SetRiseTime(NULL, 60);
    TEST_ASSERT_EQUAL(CONFIG_ERROR_NULL_POINTER, result);
}

// ========== Test 22: Reject rise time < 30 min (inrush current) ==========
void test_ConfigValidator_SetRiseTime_RejectsBelowMinimum(void) {

    ConfigValidatorError result = ConfigValidator_SetRiseTime(validator, 29);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    // Verify riseTime was NOT changed
    uint8_t riseTime = 99;
    ConfigValidator_GetRiseTime(validator, &riseTime);
    TEST_ASSERT_EQUAL_UINT8(0, riseTime);
}

// ========== Test 23: Accept minimum rise time (30 min) ==========
void test_ConfigValidator_SetRiseTime_AcceptsMinimum(void) {

    ConfigValidatorError result = ConfigValidator_SetRiseTime(validator, 30);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint8_t riseTime = 0;
    ConfigValidator_GetRiseTime(validator, &riseTime);
    TEST_ASSERT_EQUAL_UINT8(30, riseTime);
}

// ========== Test 24: Accept maximum rise time (90 min) ==========
void test_ConfigValidator_SetRiseTime_AcceptsMaximum(void) {

    ConfigValidatorError result = ConfigValidator_SetRiseTime(validator, 90);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint8_t riseTime = 0;
    ConfigValidator_GetRiseTime(validator, &riseTime);
    TEST_ASSERT_EQUAL_UINT8(90, riseTime);
}

// ========== Test 25: Reject rise time > 90 min ==========
void test_ConfigValidator_SetRiseTime_RejectsAboveMaximum(void) {

    ConfigValidatorError result = ConfigValidator_SetRiseTime(validator, 91);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    // Verify riseTime was NOT changed
    uint8_t riseTime = 99;
    ConfigValidator_GetRiseTime(validator, &riseTime);
    TEST_ASSERT_EQUAL_UINT8(0, riseTime);
}

// ========== Test 26: RiseTime overwrite works ==========
void test_ConfigValidator_SetRiseTime_OverwritesPreviousValue(void) {

    ConfigValidator_SetRiseTime(validator, 30);
    ConfigValidatorError result = ConfigValidator_SetRiseTime(validator, 90);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint8_t riseTime = 0;
    ConfigValidator_GetRiseTime(validator, &riseTime);
    TEST_ASSERT_EQUAL_UINT8(90, riseTime);
}

// ========== Test 27: FallTime has known initial state ==========
void test_ConfigValidator_FallTimeHasKnownInitialState(void) {
    uint8_t fallTime = 99;
    ConfigValidatorError result = ConfigValidator_GetFallTime(validator, &fallTime);
    TEST_ASSERT_EQUAL(CONFIG_OK, result);
    TEST_ASSERT_EQUAL_UINT8(0, fallTime);
}

// ========== Test 28: Accept valid fall time ==========
void test_ConfigValidator_AcceptsValidFallTime(void) {
    uint8_t fallTime = 0;

    ConfigValidatorError result = ConfigValidator_SetFallTime(validator, 60);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    ConfigValidator_GetFallTime(validator, &fallTime);
    TEST_ASSERT_EQUAL_UINT8(60, fallTime);
}

// ========== Test 29: Reject fall time < 30 min ==========
void test_ConfigValidator_SetFallTime_RejectsBelowMinimum(void) {
    ConfigValidatorError result = ConfigValidator_SetFallTime(validator, 29);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    uint8_t fallTime = 99;
    ConfigValidator_GetFallTime(validator, &fallTime);
    TEST_ASSERT_EQUAL_UINT8(0, fallTime);
}

// ========== Test 30: Accept minimum fall time (30 min) ==========
void test_ConfigValidator_SetFallTime_AcceptsMinimum(void) {
    ConfigValidatorError result = ConfigValidator_SetFallTime(validator, 30);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint8_t fallTime = 0;
    ConfigValidator_GetFallTime(validator, &fallTime);
    TEST_ASSERT_EQUAL_UINT8(30, fallTime);
}

// ========== Test 31: Accept maximum fall time (90 min) ==========
void test_ConfigValidator_SetFallTime_AcceptsMaximum(void) {
    ConfigValidatorError result = ConfigValidator_SetFallTime(validator, 90);

    TEST_ASSERT_EQUAL(CONFIG_OK, result);

    uint8_t fallTime = 0;
    ConfigValidator_GetFallTime(validator, &fallTime);
    TEST_ASSERT_EQUAL_UINT8(90, fallTime);
}

// ========== Test 32: Reject fall time > 90 min ==========
void test_ConfigValidator_SetFallTime_RejectsAboveMaximum(void) {
    ConfigValidatorError result = ConfigValidator_SetFallTime(validator, 91);

    TEST_ASSERT_EQUAL(CONFIG_ERROR_INVALID_PARAMETER, result);

    uint8_t fallTime = 99;
    ConfigValidator_GetFallTime(validator, &fallTime);
    TEST_ASSERT_EQUAL_UINT8(0, fallTime);
}

// ========== Test 33: FallTime independent from RiseTime ==========
void test_ConfigValidator_FallTimeIndependentFromRiseTime(void) {
    ConfigValidator_SetRiseTime(validator, 30);
    ConfigValidator_SetFallTime(validator, 90);

    uint8_t riseTime = 0;
    uint8_t fallTime = 0;

    ConfigValidator_GetRiseTime(validator, &riseTime);
    ConfigValidator_GetFallTime(validator, &fallTime);

    TEST_ASSERT_EQUAL_UINT8(30, riseTime);
    TEST_ASSERT_EQUAL_UINT8(90, fallTime);
}
