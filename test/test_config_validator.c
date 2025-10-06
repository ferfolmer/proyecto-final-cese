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
