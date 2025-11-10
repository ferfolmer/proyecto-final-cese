#include "output_driver.h"
#include <stdbool.h>
#include <string.h>

#define OUTPUT_DAC_MAX_VALUE 4095 // 12-bit DAC resolution

typedef struct {
    float percent; // Current output percentage (0-100%)
    bool enabled;  // Channel on/off state
} ChannelState;

struct OutputDriver {
    bool initialized;
    OutputHardwareInterface * hwInterface;
    ChannelState channels[OUTPUT_CHANNEL_COUNT];
};

static struct OutputDriver instance;

static bool isValidHandle(OutputDriverHandle handle) {
    return handle != NULL;
}

static bool isValidPointer(const void * ptr) {
    return ptr != NULL;
}

static bool isValidChannel(OutputChannel channel) {
    return channel < OUTPUT_CHANNEL_COUNT;
}

static bool isValidPercentage(float percent) {
    return (percent >= 0.0f) && (percent <= 100.0f);
}

// Convert percentage (0-100) to raw DAC value (0-4095)
static uint16_t PercentToRaw(float percent) {
    float raw = (percent / 100.0f) * OUTPUT_DAC_MAX_VALUE;
    return (uint16_t)(raw + 0.5f); // Round to nearest integer
}

// Update hardware output for a channel
static void UpdateHardwareOutput(OutputDriverHandle handle, OutputChannel channel) {
    if (handle->hwInterface == NULL || handle->hwInterface->SetRawOutput == NULL) {
        return; // No hardware interface, skip
    }

    uint16_t rawValue;
    if (handle->channels[channel].enabled) {
        rawValue = PercentToRaw(handle->channels[channel].percent);
    } else {
        rawValue = 0; // Disabled channel outputs zero
    }

    handle->hwInterface->SetRawOutput(channel, rawValue);
}

OutputDriverHandle OutputDriver_Create(OutputHardwareInterface * hwInterface) {
    memset(&instance, 0, sizeof(instance));
    instance.initialized = false;
    instance.hwInterface = hwInterface;

    // Initialize all channels to 0%, enabled
    for (int i = 0; i < OUTPUT_CHANNEL_COUNT; i++) {
        instance.channels[i].percent = 0.0f;
        instance.channels[i].enabled = true;
    }

    return &instance;
}

void OutputDriver_Destroy(OutputDriverHandle handle) {
    (void)handle;
}

OutputDriverError OutputDriver_Init(OutputDriverHandle handle) {
    if (!isValidHandle(handle)) {
        return OUTPUT_ERROR_NULL_POINTER;
    }

    handle->initialized = true;

    // Set all outputs to 0% on initialization (safety)
    for (OutputChannel ch = 0; ch < OUTPUT_CHANNEL_COUNT; ch++) {
        UpdateHardwareOutput(handle, ch);
    }

    return OUTPUT_OK;
}

OutputDriverError OutputDriver_SetChannel(OutputDriverHandle handle, OutputChannel channel,
                                          float percent) {
    if (!isValidHandle(handle)) {
        return OUTPUT_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return OUTPUT_ERROR_NOT_INITIALIZED;
    }
    if (!isValidChannel(channel)) {
        return OUTPUT_ERROR_INVALID_CHANNEL;
    }
    if (!isValidPercentage(percent)) {
        return OUTPUT_ERROR_OUT_OF_RANGE;
    }

    // Store percentage
    handle->channels[channel].percent = percent;

    // Update hardware output (respects enabled/disabled state)
    UpdateHardwareOutput(handle, channel);

    return OUTPUT_OK;
}

OutputDriverError OutputDriver_GetChannel(OutputDriverHandle handle, OutputChannel channel,
                                          float * percent) {
    if (!isValidHandle(handle) || !isValidPointer(percent)) {
        return OUTPUT_ERROR_NULL_POINTER;
    }
    if (!isValidChannel(channel)) {
        return OUTPUT_ERROR_INVALID_CHANNEL;
    }

    *percent = handle->channels[channel].percent;
    return OUTPUT_OK;
}

OutputDriverError OutputDriver_EnableChannel(OutputDriverHandle handle, OutputChannel channel,
                                             bool enable) {
    if (!isValidHandle(handle)) {
        return OUTPUT_ERROR_NULL_POINTER;
    }
    if (!isValidChannel(channel)) {
        return OUTPUT_ERROR_INVALID_CHANNEL;
    }

    handle->channels[channel].enabled = enable;

    // Update hardware immediately (will output 0 if disabled)
    UpdateHardwareOutput(handle, channel);

    return OUTPUT_OK;
}

OutputDriverError OutputDriver_IsChannelEnabled(OutputDriverHandle handle, OutputChannel channel,
                                                bool * isEnabled) {
    if (!isValidHandle(handle) || !isValidPointer(isEnabled)) {
        return OUTPUT_ERROR_NULL_POINTER;
    }
    if (!isValidChannel(channel)) {
        return OUTPUT_ERROR_INVALID_CHANNEL;
    }

    *isEnabled = handle->channels[channel].enabled;
    return OUTPUT_OK;
}

OutputDriverError OutputDriver_SetAllChannels(OutputDriverHandle handle,
                                              float percentages[OUTPUT_CHANNEL_COUNT]) {
    if (!isValidHandle(handle) || !isValidPointer(percentages)) {
        return OUTPUT_ERROR_NULL_POINTER;
    }

    // Validate all percentages first (atomic operation - all or nothing)
    for (OutputChannel ch = 0; ch < OUTPUT_CHANNEL_COUNT; ch++) {
        if (!isValidPercentage(percentages[ch])) {
            return OUTPUT_ERROR_OUT_OF_RANGE;
        }
    }

    // All valid - now set all channels
    for (OutputChannel ch = 0; ch < OUTPUT_CHANNEL_COUNT; ch++) {
        handle->channels[ch].percent = percentages[ch];
        UpdateHardwareOutput(handle, ch);
    }

    return OUTPUT_OK;
}

OutputDriverError OutputDriver_EmergencyStop(OutputDriverHandle handle) {
    if (!isValidHandle(handle)) {
        return OUTPUT_ERROR_NULL_POINTER;
    }

    // Set all channels to 0% immediately
    for (OutputChannel ch = 0; ch < OUTPUT_CHANNEL_COUNT; ch++) {
        handle->channels[ch].percent = 0.0f;
        UpdateHardwareOutput(handle, ch);
    }

    return OUTPUT_OK;
}
