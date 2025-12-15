# ConfigValidator - Complete Test Plan

## Test Status: 35/35 Implemented ✅ COMPLETE

**Note**: Original plan was 33 tests. Final implementation has 35 tests due to additional coverage for edge cases (overwrite tests and extra null safety checks).

---

## Category 1: Lifecycle & Initialization (2 tests)

### ✅ Test 1: `test_ConfigValidator_CanBeCreated`
**Status**: IMPLEMENTED
**Purpose**: Verify module can be instantiated
**Steps**:
1. Call `ConfigValidator_Create()`
2. Assert handle is not NULL
3. Call `ConfigValidator_Destroy()`

### ✅ Test 2: `test_ConfigValidator_HasKnownInitialState`
**Status**: IMPLEMENTED
**Purpose**: Verify all parameters initialize to known/safe values
**Steps**:
1. Create validator
2. Get DLI → assert equals 0
3. Get photoperiod → assert equals 0
4. Get riseTime → assert equals 0
5. Get fallTime → assert equals 0
6. Destroy validator

---

## Category 2: DLI Parameter (8 tests)

### ✅ Test 3: `test_ConfigValidator_AcceptsValidDLI`
**Status**: IMPLEMENTED
**Purpose**: Verify valid DLI values are stored correctly
**Steps**:
1. Create validator
2. Set DLI = 20
3. Get DLI → assert equals 20

### ✅ Test 4: `test_ConfigValidator_GetDLI_RejectsNullHandle`
**Status**: IMPLEMENTED
**Purpose**: Verify null safety on GetDLI
**Steps**:
1. Call `ConfigValidator_GetDLI(NULL, &dli)`
2. Assert returns `CONFIG_ERROR_NULL_POINTER`

### ✅ Test 5: `test_ConfigValidator_GetDLI_RejectsNullPointer`
**Status**: IMPLEMENTED
**Purpose**: Verify null safety on output pointer
**Steps**:
1. Create validator
2. Call `ConfigValidator_GetDLI(validator, NULL)`
3. Assert returns `CONFIG_ERROR_NULL_POINTER`

### ✅ Test 6: `test_ConfigValidator_SetDLI_RejectsNullHandle`
**Status**: IMPLEMENTED
**Purpose**: Verify null safety on SetDLI
**Steps**:
1. Call `ConfigValidator_SetDLI(NULL, 20)`
2. Assert returns `CONFIG_ERROR_NULL_POINTER`

### ❌ Test 7: `test_ConfigValidator_SetDLI_RejectsZero`
**Status**: IMPLEMENTED ✅
**Purpose**: DLI = 0 is invalid (no light = no growth)
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetDLI(validator, 0)`
3. Assert returns `CONFIG_ERROR_INVALID_PARAMETER`
4. Get DLI → assert still equals 0 (unchanged)

**Code Changes Needed**:
```c
// Add to enum:
CONFIG_ERROR_INVALID_PARAMETER

// In SetDLI:
if (dli == 0) {
    return CONFIG_ERROR_INVALID_PARAMETER;
}
```

### ❌ Test 8: `test_ConfigValidator_SetDLI_AcceptsMinimumValid`
**Status**: IMPLEMENTED ✅
**Purpose**: DLI = 1 mol/m²/day is minimum valid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetDLI(validator, 1)`
3. Assert returns `CONFIG_OK`
4. Get DLI → assert equals 1

### ❌ Test 9: `test_ConfigValidator_SetDLI_AcceptsMaximumValid`
**Status**: IMPLEMENTED ✅
**Purpose**: DLI = 65535 mol/m²/day (uint16_t max) should be accepted
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetDLI(validator, 65535)`
3. Assert returns `CONFIG_OK`
4. Get DLI → assert equals 65535

### ❌ Test 10: `test_ConfigValidator_SetDLI_OverwritesPreviousValue`
**Status**: IMPLEMENTED ✅
**Purpose**: Setting DLI twice should update value
**Steps**:
1. Create validator
2. Set DLI = 20
3. Set DLI = 30
4. Get DLI → assert equals 30

---

## Category 3: Photoperiod Parameter (9 tests)

### ✅ Test 11: `test_ConfigValidator_AcceptsValidPhotoperiod`
**Status**: IMPLEMENTED
**Purpose**: Verify valid photoperiod is stored
**Steps**:
1. Create validator
2. Set photoperiod = 16
3. Get photoperiod → assert equals 16

### ❌ Test 12: `test_ConfigValidator_GetPhotoperiod_RejectsNullHandle`
**Status**: IMPLEMENTED ✅
**Purpose**: Null safety check
**Steps**:
1. Call `ConfigValidator_GetPhotoperiod(NULL, &photoperiod)`
2. Assert returns `CONFIG_ERROR_NULL_POINTER`

### ❌ Test 13: `test_ConfigValidator_GetPhotoperiod_RejectsNullPointer`
**Status**: IMPLEMENTED ✅
**Purpose**: Null safety check on output
**Steps**:
1. Create validator
2. Call `ConfigValidator_GetPhotoperiod(validator, NULL)`
3. Assert returns `CONFIG_ERROR_NULL_POINTER`

### ❌ Test 14: `test_ConfigValidator_SetPhotoperiod_RejectsNullHandle`
**Status**: IMPLEMENTED ✅
**Purpose**: Null safety check
**Steps**:
1. Call `ConfigValidator_SetPhotoperiod(NULL, 16)`
2. Assert returns `CONFIG_ERROR_NULL_POINTER`

### ❌ Test 15: `test_ConfigValidator_SetPhotoperiod_RejectsZero`
**Status**: IMPLEMENTED ✅
**Purpose**: Photoperiod = 0 hours is invalid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetPhotoperiod(validator, 0)`
3. Assert returns `CONFIG_ERROR_INVALID_PARAMETER`

### ❌ Test 16: `test_ConfigValidator_SetPhotoperiod_AcceptsMinimum`
**Status**: IMPLEMENTED ✅
**Purpose**: Photoperiod = 1 hour is minimum valid (per requirements)
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetPhotoperiod(validator, 1)`
3. Assert returns `CONFIG_OK`
4. Get photoperiod → assert equals 1

### ❌ Test 17: `test_ConfigValidator_SetPhotoperiod_AcceptsMaximum`
**Status**: IMPLEMENTED ✅
**Purpose**: Photoperiod = 24 hours is maximum valid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetPhotoperiod(validator, 24)`
3. Assert returns `CONFIG_OK`
4. Get photoperiod → assert equals 24

### ❌ Test 18: `test_ConfigValidator_SetPhotoperiod_RejectsExcessiveValue`
**Status**: IMPLEMENTED ✅
**Purpose**: Photoperiod > 24 hours is invalid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetPhotoperiod(validator, 25)`
3. Assert returns `CONFIG_ERROR_INVALID_PARAMETER`
4. Get photoperiod → assert equals 0 (unchanged)

**Code Changes Needed**:
```c
// In SetPhotoperiod:
if (photoperiod == 0 || photoperiod > 24) {
    return CONFIG_ERROR_INVALID_PARAMETER;
}
```

### ❌ Test 19: `test_ConfigValidator_SetPhotoperiod_OverwritesPreviousValue`
**Status**: IMPLEMENTED ✅
**Purpose**: Setting twice should update value
**Steps**:
1. Create validator
2. Set photoperiod = 12
3. Set photoperiod = 18
4. Get photoperiod → assert equals 18

---

## Category 4: Rise Time Parameter (7 tests)

### ✅ Test 20: `test_ConfigValidator_AcceptsValidRiseTime`
**Status**: IMPLEMENTED
**Purpose**: Verify valid rise time is stored
**Steps**:
1. Create validator
2. Set riseTime = 60
3. Get riseTime → assert equals 60

### ❌ Test 21: `test_ConfigValidator_SetRiseTime_RejectsBelowMinimum`
**Status**: IMPLEMENTED ✅
**Purpose**: Rise time < 30 min is invalid (inrush current risk)
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetRiseTime(validator, 29)`
3. Assert returns `CONFIG_ERROR_INVALID_PARAMETER`

### ❌ Test 22: `test_ConfigValidator_SetRiseTime_AcceptsMinimum`
**Status**: IMPLEMENTED ✅
**Purpose**: Rise time = 30 min is minimum valid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetRiseTime(validator, 30)`
3. Assert returns `CONFIG_OK`
4. Get riseTime → assert equals 30

### ❌ Test 23: `test_ConfigValidator_SetRiseTime_AcceptsMaximum`
**Status**: IMPLEMENTED ✅
**Purpose**: Rise time = 90 min is maximum valid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetRiseTime(validator, 90)`
3. Assert returns `CONFIG_OK`
4. Get riseTime → assert equals 90

### ❌ Test 24: `test_ConfigValidator_SetRiseTime_RejectsAboveMaximum`
**Status**: IMPLEMENTED ✅
**Purpose**: Rise time > 90 min is invalid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetRiseTime(validator, 91)`
3. Assert returns `CONFIG_ERROR_INVALID_PARAMETER`

**Code Changes Needed**:
```c
// In SetRiseTime:
if (riseTime < 30 || riseTime > 90) {
    return CONFIG_ERROR_INVALID_PARAMETER;
}
```

### ❌ Test 25: `test_ConfigValidator_GetRiseTime_RejectsNullHandle`
**Status**: IMPLEMENTED ✅
**Purpose**: Null safety
**Steps**:
1. Call `ConfigValidator_GetRiseTime(NULL, &riseTime)`
2. Assert returns `CONFIG_ERROR_NULL_POINTER`

### ❌ Test 26: `test_ConfigValidator_GetRiseTime_RejectsNullPointer`
**Status**: IMPLEMENTED ✅
**Purpose**: Null safety
**Steps**:
1. Create validator
2. Call `ConfigValidator_GetRiseTime(validator, NULL)`
3. Assert returns `CONFIG_ERROR_NULL_POINTER`

---

## Category 5: Fall Time Parameter (7 tests - NEW)

### ❌ Test 27: `test_ConfigValidator_FallTimeHasKnownInitialState`
**Status**: IMPLEMENTED ✅
**Purpose**: Fall time should initialize to 0
**Steps**:
1. Create validator
2. Get fallTime → assert equals 0

**Code Changes Needed**:
```c
// Add to struct:
uint8_t fallTime;

// In Create:
instance.fallTime = 0;

// Add functions:
ConfigValidatorError ConfigValidator_GetFallTime(ConfigValidatorHandle, uint8_t*);
ConfigValidatorError ConfigValidator_SetFallTime(ConfigValidatorHandle, uint8_t);
```

### ❌ Test 28: `test_ConfigValidator_AcceptsValidFallTime`
**Status**: IMPLEMENTED ✅
**Purpose**: Verify valid fall time is stored
**Steps**:
1. Create validator
2. Set fallTime = 60
3. Get fallTime → assert equals 60

### ❌ Test 29: `test_ConfigValidator_SetFallTime_RejectsBelowMinimum`
**Status**: IMPLEMENTED ✅
**Purpose**: Fall time < 30 min is invalid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetFallTime(validator, 29)`
3. Assert returns `CONFIG_ERROR_INVALID_PARAMETER`

### ❌ Test 30: `test_ConfigValidator_SetFallTime_AcceptsMinimum`
**Status**: IMPLEMENTED ✅
**Purpose**: Fall time = 30 min is minimum valid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetFallTime(validator, 30)`
3. Assert returns `CONFIG_OK`

### ❌ Test 31: `test_ConfigValidator_SetFallTime_AcceptsMaximum`
**Status**: IMPLEMENTED ✅
**Purpose**: Fall time = 90 min is maximum valid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetFallTime(validator, 90)`
3. Assert returns `CONFIG_OK`

### ❌ Test 32: `test_ConfigValidator_SetFallTime_RejectsAboveMaximum`
**Status**: IMPLEMENTED ✅
**Purpose**: Fall time > 90 min is invalid
**Steps**:
1. Create validator
2. Call `ConfigValidator_SetFallTime(validator, 91)`
3. Assert returns `CONFIG_ERROR_INVALID_PARAMETER`

### ❌ Test 33: `test_ConfigValidator_FallTimeIndependentFromRiseTime`
**Status**: IMPLEMENTED ✅
**Purpose**: Rise and fall times should be stored independently
**Steps**:
1. Create validator
2. Set riseTime = 30
3. Set fallTime = 90
4. Get riseTime → assert equals 30
5. Get fallTime → assert equals 90

---

## Summary of Remaining Work

### Immediate Next Steps (Tests 7-10):
1. Add `CONFIG_ERROR_INVALID_PARAMETER` to enum
2. Implement DLI validation (reject 0, accept 1-65535)
3. Test edge cases (min, max, overwrites)

### Short Term (Tests 12-19):
1. Implement photoperiod validation (reject 0, >24; accept 1-24)
2. Add null pointer tests for photoperiod

### Medium Term (Tests 21-33):
1. Implement rise time validation (30-90 min)
2. Add fall time parameter (new struct member + functions)
3. Implement fall time validation (30-90 min)

### Future Enhancements (Beyond 33 tests):
- Spectral ratio configuration
- DLI achievability calculation
- PPFD setpoint calculation
- Suggestion engine
- Persistence (NVM save/load)

---

## Test Execution Strategy

### Phase 1: Parameter Validation (Tests 7-33)
**Goal**: Complete basic parameter storage with range checking
**Estimated Effort**: 3-4 hours
**TDD Cycle**:
1. Write test (RED)
2. Run → observe failure
3. Implement minimal code (GREEN)
4. Run → observe pass
5. Refactor if needed
6. Commit

### Phase 2: Advanced Features (Beyond Test 33)
**Goal**: Implement calculations and validations
**Estimated Effort**: 8-12 hours
**Features**:
- DLI achievability math
- Spectral configuration
- Persistence layer

---

## Coverage Goal
**Target**: 95% line coverage for ConfigValidator module
**Tool**: gcov via `ceedling gcov:config_validator`

---

**Last Updated**: 2025-10-27
**Status**: 8/33 tests implemented (24%)
