#ifndef INA226_HAL_H
#define INA226_HAL_H

// Port of Rob Tillaart's INA226 Arduino library to C for STM32 HAL
// Target: STM32F446RE (CubeIDE + HAL)
// Author: (ported by ChatGPT for Edrick)
// Version: 0.6.4-compatible API

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// -------- Version tag (optional) --------
#define INA226_LIB_VERSION_E4 64  // 0.6.4 -> 64 (for your tracking)

// -------- Public error codes (same semantics as original) --------
#define INA226_ERR_NONE                   0x0000
#define INA226_ERR_SHUNTVOLTAGE_HIGH      0x8000
#define INA226_ERR_MAXCURRENT_LOW         0x8001
#define INA226_ERR_SHUNT_LOW              0x8002
#define INA226_ERR_NORMALIZE_FAILED       0x8003

// -------- Limits / helpers --------
#define INA226_MINIMAL_SHUNT_OHM          0.001f
#define INA226_MAX_WAIT_MS                600U
#define INA226_MAX_SHUNT_VOLTAGE          (81.92f / 1000.0f)  // 81.92 mV

// -------- Alert masks (for setAlertRegister) --------
#define INA226_SHUNT_OVER_VOLTAGE         0x8000
#define INA226_SHUNT_UNDER_VOLTAGE        0x4000
#define INA226_BUS_OVER_VOLTAGE           0x2000
#define INA226_BUS_UNDER_VOLTAGE          0x1000
#define INA226_POWER_OVER_LIMIT           0x0800
#define INA226_CONVERSION_READY           0x0400

// -------- Alert flags (for getAlertFlag) --------
#define INA226_ALERT_FUNCTION_FLAG        0x0010
#define INA226_CONVERSION_READY_FLAG      0x0008
#define INA226_MATH_OVERFLOW_FLAG         0x0004
#define INA226_ALERT_POLARITY_FLAG        0x0002
#define INA226_ALERT_LATCH_ENABLE_FLAG    0x0001

// -------- Averages --------
typedef enum {
    INA226_1_SAMPLE     = 0,
    INA226_4_SAMPLES    = 1,
    INA226_16_SAMPLES   = 2,
    INA226_64_SAMPLES   = 3,
    INA226_128_SAMPLES  = 4,
    INA226_256_SAMPLES  = 5,
    INA226_512_SAMPLES  = 6,
    INA226_1024_SAMPLES = 7
} ina226_average_t;

// -------- Conversion times --------
typedef enum {
    INA226_140_us  = 0,
    INA226_204_us  = 1,
    INA226_332_us  = 2,
    INA226_588_us  = 3,
    INA226_1100_us = 4,
    INA226_2100_us = 5,
    INA226_4200_us = 6,
    INA226_8300_us = 7
} ina226_timing_t;

// -------- Operating modes (same numeric values as original) --------
typedef enum {
    INA226_MODE_SHUTDOWN            = 0,
    INA226_MODE_SHUNT_TRIGGER       = 1,
    INA226_MODE_BUS_TRIGGER         = 2,
    INA226_MODE_SHUNT_BUS_TRIGGER   = 3,
    INA226_MODE_RESERVED4           = 4,
    INA226_MODE_SHUNT_CONTINUOUS    = 5,
    INA226_MODE_BUS_CONTINUOUS      = 6,
    INA226_MODE_SHUNT_BUS_CONTINUOUS= 7
} ina226_mode_t;

// -------- Driver handle --------
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;                 // 7-bit address (0x40..0x4F)
    float   current_LSB;             // A/bit
    float   shunt;                   // Ohm
    float   maxCurrent;              // A
    float   current_zero_offset;     // A
    uint16_t bus_V_scaling_e4;       // default 10000 (scale = 1.0000)
    int     error;                   // last I/O error
    uint32_t i2c_timeout_ms;         // HAL I2C timeout (default 10)
} INA226_Handle;

// -------- API --------
void    INA226_Init(INA226_Handle *dev, I2C_HandleTypeDef *hi2c, uint8_t i2c_addr7);
bool    INA226_Begin(INA226_Handle *dev);
bool    INA226_IsConnected(INA226_Handle *dev);
uint8_t INA226_GetAddress(INA226_Handle *dev);

// Core measurements
float   INA226_GetBusVoltage(INA226_Handle *dev);    // V
float   INA226_GetShuntVoltage(INA226_Handle *dev);  // V
float   INA226_GetCurrent(INA226_Handle *dev);       // A
float   INA226_GetPower(INA226_Handle *dev);         // W

// Convenience scales
static inline float INA226_GetBusVoltage_mV(INA226_Handle *d){ return INA226_GetBusVoltage(d)*1e3f; }
static inline float INA226_GetShuntVoltage_mV(INA226_Handle *d){ return INA226_GetShuntVoltage(d)*1e3f; }
static inline float INA226_GetCurrent_mA(INA226_Handle *d){ return INA226_GetCurrent(d)*1e3f; }
static inline float INA226_GetPower_mW(INA226_Handle *d){ return INA226_GetPower(d)*1e3f; }

bool    INA226_IsConversionReady(INA226_Handle *dev);
bool    INA226_WaitConversionReady(INA226_Handle *dev, uint32_t timeout_ms);

// Configuration
bool    INA226_Reset(INA226_Handle *dev);
bool    INA226_SetAverage(INA226_Handle *dev, uint8_t avg);   // 0..7
uint8_t INA226_GetAverage(INA226_Handle *dev);
bool    INA226_SetBusConvTime(INA226_Handle *dev, uint8_t bvct); // 0..7
uint8_t INA226_GetBusConvTime(INA226_Handle *dev);
bool    INA226_SetShuntConvTime(INA226_Handle *dev, uint8_t svct); // 0..7
uint8_t INA226_GetShuntConvTime(INA226_Handle *dev);

// Calibration
int     INA226_SetMaxCurrentShunt(INA226_Handle *dev, float maxCurrent, float shunt, bool normalize);
int     INA226_Configure(INA226_Handle *dev, float shunt, float current_LSB_mA, float current_zero_offset_mA, uint16_t bus_V_scaling_e4);
bool    INA226_IsCalibrated(INA226_Handle *dev);

static inline float INA226_GetCurrentLSB(INA226_Handle *d){ return d->current_LSB; }
static inline float INA226_GetCurrentLSB_mA(INA226_Handle *d){ return d->current_LSB*1e3f; }
static inline float INA226_GetCurrentLSB_uA(INA226_Handle *d){ return d->current_LSB*1e6f; }
static inline float INA226_GetShunt(INA226_Handle *d){ return d->shunt; }
static inline float INA226_GetMaxCurrent(INA226_Handle *d){ return d->maxCurrent; }

// Operating mode
bool    INA226_SetMode(INA226_Handle *dev, uint8_t mode); // 0..7
uint8_t INA226_GetMode(INA226_Handle *dev);
static inline bool INA226_Shutdown(INA226_Handle *d){ return INA226_SetMode(d, INA226_MODE_SHUTDOWN); }

// Alert
bool    INA226_SetAlertRegister(INA226_Handle *dev, uint16_t mask);
uint16_t INA226_GetAlertFlag(INA226_Handle *dev);
bool    INA226_SetAlertLimit(INA226_Handle *dev, uint16_t limit);
uint16_t INA226_GetAlertLimit(INA226_Handle *dev);

// Meta
uint16_t INA226_GetManufacturerID(INA226_Handle *dev);  // typical 0x5449
uint16_t INA226_GetDieID(INA226_Handle *dev);           // typical 0x2260

// Debug / error handling
uint16_t INA226_GetRegister(INA226_Handle *dev, uint8_t reg);
int      INA226_GetLastError(INA226_Handle *dev);

// Optional: override I2C timeout
static inline void INA226_SetI2CTimeout(INA226_Handle *d, uint32_t ms){ d->i2c_timeout_ms = ms; }

#ifdef __cplusplus
}
#endif
#endif // INA226_HAL_H
