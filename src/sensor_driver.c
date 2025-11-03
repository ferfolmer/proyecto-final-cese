#include "sensor_driver.h"
#include <stdbool.h>
#include <string.h>

#define MAX_FILTER_SIZE 16

struct SensorDriver {
    bool initialized;
    SensorHardwareInterface * hwInterface;
    CalibrationData calibration;
    uint8_t filterSize;
    float filterBuffer[MAX_FILTER_SIZE];
    uint8_t filterIndex;
    bool filterFilled;
};

static struct SensorDriver instance;

static bool isValidHandle(SensorDriverHandle handle) {
    return handle != NULL;
}

static bool isValidPointer(const void * ptr) {
    return ptr != NULL;
}

static float ApplyMovingAverage(SensorDriverHandle handle, float newValue) {
    // Add new value to circular buffer
    handle->filterBuffer[handle->filterIndex] = newValue;
    handle->filterIndex = (handle->filterIndex + 1) % handle->filterSize;

    // Mark buffer as filled when we've wrapped around
    if (handle->filterIndex == 0) {
        handle->filterFilled = true;
    }

    // Calculate average over valid samples
    float sum = 0.0f;
    uint8_t count = handle->filterFilled ? handle->filterSize : handle->filterIndex;

    // Handle edge case: if filterIndex wrapped to 0, count should be filterSize
    if (count == 0 && handle->filterFilled) {
        count = handle->filterSize;
    }

    for (uint8_t i = 0; i < count; i++) {
        sum += handle->filterBuffer[i];
    }

    return (count > 0) ? (sum / count) : 0.0f;
}

SensorDriverHandle SensorDriver_Create(SensorHardwareInterface * hwInterface) {
    memset(&instance, 0, sizeof(instance));
    instance.initialized = false;
    instance.hwInterface = hwInterface;
    instance.filterSize = 1; // Default: no filtering
    instance.filterIndex = 0;
    instance.filterFilled = false;
    return &instance;
}

void SensorDriver_Destroy(SensorDriverHandle handle) {
    (void)handle;
}

SensorDriverError SensorDriver_Init(SensorDriverHandle handle) {
    if (!isValidHandle(handle)) {
        return SENSOR_ERROR_NULL_POINTER;
    }
    handle->initialized = true;
    return SENSOR_OK;
}

SensorDriverError SensorDriver_ReadRaw(SensorDriverHandle handle, uint16_t * rawValue) {
    if (!isValidHandle(handle) || !isValidPointer(rawValue)) {
        return SENSOR_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return SENSOR_ERROR_NOT_INITIALIZED;
    }
    // TODO: Actual ADC reading (will be mocked in tests)
    *rawValue = 0;
    return SENSOR_OK;
}

SensorDriverError SensorDriver_ReadPPFD(SensorDriverHandle handle, uint16_t * ppfd) {
    if (!isValidHandle(handle) || !isValidPointer(ppfd)) {
        return SENSOR_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return SENSOR_ERROR_NOT_INITIALIZED;
    }

    // Get raw PPFD value (in µmol/m²/s) from hardware interface
    float rawPPFD = 0.0f;
    if (handle->hwInterface != NULL && handle->hwInterface->ReadRawPPFD != NULL) {
        rawPPFD = handle->hwInterface->ReadRawPPFD();
    }

    // Apply calibration: PPFD_corrected = (PPFD_raw - offset) * gain
    // Use default gain=1.0 if calibration not set (gain==0)
    float gain = (handle->calibration.gain == 0.0f) ? 1.0f : handle->calibration.gain;
    float calibratedPPFD = (rawPPFD - handle->calibration.offset_umol) * gain;

    // Clamp negative values to zero
    if (calibratedPPFD < 0.0f) {
        calibratedPPFD = 0.0f;
    }

    // Apply filtering if filter size > 1
    float filteredPPFD = calibratedPPFD;
    if (handle->filterSize > 1) {
        filteredPPFD = ApplyMovingAverage(handle, calibratedPPFD);
    }

    // Convert to uint16_t with rounding
    *ppfd = (uint16_t)(filteredPPFD + 0.5f);

    return SENSOR_OK;
}

SensorDriverError SensorDriver_SetCalibration(SensorDriverHandle handle,
                                              CalibrationData calibration) {
    if (!isValidHandle(handle)) {
        return SENSOR_ERROR_NULL_POINTER;
    }
    handle->calibration = calibration;
    return SENSOR_OK;
}

SensorDriverError SensorDriver_GetCalibration(SensorDriverHandle handle,
                                              CalibrationData * calibration) {
    if (!isValidHandle(handle) || !isValidPointer(calibration)) {
        return SENSOR_ERROR_NULL_POINTER;
    }
    *calibration = handle->calibration;
    return SENSOR_OK;
}

SensorDriverError SensorDriver_SetFilterSize(SensorDriverHandle handle, uint8_t numSamples) {
    if (!isValidHandle(handle)) {
        return SENSOR_ERROR_NULL_POINTER;
    }

    // Clamp to maximum filter size
    if (numSamples > MAX_FILTER_SIZE) {
        numSamples = MAX_FILTER_SIZE;
    }

    // Reset filter state when size changes
    handle->filterSize = numSamples;
    handle->filterIndex = 0;
    handle->filterFilled = false;
    memset(handle->filterBuffer, 0, sizeof(handle->filterBuffer));

    return SENSOR_OK;
}

SensorDriverError SensorDriver_GetFilterSize(SensorDriverHandle handle, uint8_t * numSamples) {
    if (!isValidHandle(handle) || !isValidPointer(numSamples)) {
        return SENSOR_ERROR_NULL_POINTER;
    }
    *numSamples = handle->filterSize;
    return SENSOR_OK;
}

SensorDriverError SensorDriver_IsConnected(SensorDriverHandle handle, bool * isConnected) {
    if (!isValidHandle(handle) || !isValidPointer(isConnected)) {
        return SENSOR_ERROR_NULL_POINTER;
    }
    // TODO: Actual connectivity check
    *isConnected = handle->initialized;
    return SENSOR_OK;
}
