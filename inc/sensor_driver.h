#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct SensorDriver * SensorDriverHandle;

// Hardware abstraction: function pointer for reading raw PPFD from sensor
typedef float (*SensorReadRawPPFD_t)(void);

typedef struct {
    SensorReadRawPPFD_t ReadRawPPFD; // Function to read raw PPFD (µmol/m²/s)
} SensorHardwareInterface;

typedef enum {
    SENSOR_OK = 0,
    SENSOR_ERROR_NULL_POINTER,
    SENSOR_ERROR_NOT_INITIALIZED,
    SENSOR_ERROR_ADC_TIMEOUT,
    SENSOR_ERROR_ADC_OVERRUN,
    SENSOR_ERROR_CALIBRATION_INVALID
} SensorDriverError;

typedef struct {
    float offset_umol;     // Offset in µmol/m²·s
    float gain;            // Gain multiplier (unitless)
    uint32_t timestamp_ms; // When calibration was performed
    uint16_t crc;          // Data integrity check
} CalibrationData;

// Lifecycle
SensorDriverHandle SensorDriver_Create(SensorHardwareInterface * hwInterface);
void SensorDriver_Destroy(SensorDriverHandle handle);
SensorDriverError SensorDriver_Init(SensorDriverHandle handle);

// Raw ADC reading (0-4095 for 12-bit ADC)
SensorDriverError SensorDriver_ReadRaw(SensorDriverHandle handle, uint16_t * rawValue);

// Calibrated PPFD reading (µmol/m²·s)
SensorDriverError SensorDriver_ReadPPFD(SensorDriverHandle handle, uint16_t * ppfd);

// Calibration management
SensorDriverError SensorDriver_SetCalibration(SensorDriverHandle handle,
                                              CalibrationData calibration);
SensorDriverError SensorDriver_GetCalibration(SensorDriverHandle handle,
                                              CalibrationData * calibration);

// Filtering configuration
SensorDriverError SensorDriver_SetFilterSize(SensorDriverHandle handle, uint8_t numSamples);
SensorDriverError SensorDriver_GetFilterSize(SensorDriverHandle handle, uint8_t * numSamples);

// Status
SensorDriverError SensorDriver_IsConnected(SensorDriverHandle handle, bool * isConnected);

#endif // SENSOR_DRIVER_H
