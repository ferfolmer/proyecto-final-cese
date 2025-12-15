#ifndef OUTPUT_DRIVER_H
#define OUTPUT_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct OutputDriver * OutputDriverHandle;

// LED channel enumeration
typedef enum {
    OUTPUT_CHANNEL_0 = 0, // Typically Red channel
    OUTPUT_CHANNEL_1 = 1, // Typically Blue channel
    OUTPUT_CHANNEL_2 = 2, // Optional: Far-Red or White
    OUTPUT_CHANNEL_3 = 3, // Optional: Far-Red or White
    OUTPUT_CHANNEL_COUNT = 4
} OutputChannel;

// Error codes
typedef enum {
    OUTPUT_OK = 0,
    OUTPUT_ERROR_NULL_POINTER,
    OUTPUT_ERROR_NOT_INITIALIZED,
    OUTPUT_ERROR_INVALID_CHANNEL,
    OUTPUT_ERROR_OUT_OF_RANGE // Percentage outside 0-100%
} OutputDriverError;

// Hardware abstraction: function pointer for setting raw output
// rawValue: 0-4095 (12-bit resolution)
//   - For DAC: 0=0V, 4095=3.3V (→ 0-10V after amplification)
//   - For PWM: 0=0% duty, 4095=100% duty
typedef void (*OutputSetRaw_t)(OutputChannel channel, uint16_t rawValue);

typedef struct {
    OutputSetRaw_t SetRawOutput; // Function to set raw hardware value
} OutputHardwareInterface;

// Lifecycle
OutputDriverHandle OutputDriver_Create(OutputHardwareInterface * hwInterface);
void OutputDriver_Destroy(OutputDriverHandle handle);
OutputDriverError OutputDriver_Init(OutputDriverHandle handle);

// Channel control (percentage 0.0-100.0%)
OutputDriverError OutputDriver_SetChannel(OutputDriverHandle handle, OutputChannel channel,
                                          float percent);

OutputDriverError OutputDriver_GetChannel(OutputDriverHandle handle, OutputChannel channel,
                                          float * percent);

// Enable/disable individual channels
OutputDriverError OutputDriver_EnableChannel(OutputDriverHandle handle, OutputChannel channel,
                                             bool enable);

OutputDriverError OutputDriver_IsChannelEnabled(OutputDriverHandle handle, OutputChannel channel,
                                                bool * isEnabled);

// Bulk operations (for synchronized multi-channel updates)
OutputDriverError OutputDriver_SetAllChannels(OutputDriverHandle handle,
                                              float percentages[OUTPUT_CHANNEL_COUNT]);

// Emergency stop: immediately set all channels to 0%
OutputDriverError OutputDriver_EmergencyStop(OutputDriverHandle handle);

#endif // OUTPUT_DRIVER_H
