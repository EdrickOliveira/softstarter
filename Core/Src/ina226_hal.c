#include "ina226_hal.h"
#include <math.h>

// -------- INA226 register map --------
#define INA226_CONFIGURATION    0x00
#define INA226_SHUNT_VOLTAGE    0x01
#define INA226_BUS_VOLTAGE      0x02
#define INA226_POWER            0x03
#define INA226_CURRENT          0x04
#define INA226_CALIBRATION      0x05
#define INA226_MASK_ENABLE      0x06
#define INA226_ALERT_LIMIT      0x07
#define INA226_MANUFACTURER     0xFE
#define INA226_DIE_ID           0xFF

// -------- Config bit masks --------
#define INA226_CONF_RESET_MASK  0x8000
#define INA226_CONF_AVERAGE_MASK 0x0E00
#define INA226_CONF_BUSVC_MASK   0x01C0
#define INA226_CONF_SHUNTVC_MASK 0x0038
#define INA226_CONF_MODE_MASK    0x0007

// ---- local helpers ----
static uint16_t rd16(INA226_Handle *dev, uint8_t reg);
static uint16_t wr16(INA226_Handle *dev, uint8_t reg, uint16_t value);

// ======================= Public API =========================

void INA226_Init(INA226_Handle *dev, I2C_HandleTypeDef *hi2c, uint8_t i2c_addr7)
{
    dev->hi2c = hi2c;
    dev->address = (uint8_t)(i2c_addr7 << 1); // HAL expects 8-bit addr in Mem APIs? We'll use Mem APIs so 7-bit, but keep shift=0 in Mem APIs.
    // Note: HAL_I2C_Mem_Read/Write uses 7-bit address left aligned by HAL. In Cube HAL, pass 0x40<<1. So we store <<1 here.
    dev->current_LSB = 0.0f;
    dev->maxCurrent = 0.0f;
    dev->shunt = 0.0f;
    dev->current_zero_offset = 0.0f;
    dev->bus_V_scaling_e4 = 10000;
    dev->error = 0;
    dev->i2c_timeout_ms = 10;
}

bool INA226_Begin(INA226_Handle *dev)
{
    return INA226_IsConnected(dev);
}

bool INA226_IsConnected(INA226_Handle *dev)
{
    // Probe by reading a 16-bit register (manufacturer ID)
    uint8_t buf[2];
    if (HAL_I2C_Mem_Read(dev->hi2c, dev->address, INA226_MANUFACTURER, I2C_MEMADD_SIZE_8BIT,
                         buf, 2, dev->i2c_timeout_ms) == HAL_OK)
    {
        return true;
    }
    return false;
}

uint8_t INA226_GetAddress(INA226_Handle *dev)
{
    return (dev->address >> 1); // return 7-bit
}

// -------- Measurements --------

float INA226_GetBusVoltage(INA226_Handle *dev)
{
    uint16_t raw = rd16(dev, INA226_BUS_VOLTAGE);
    float voltage = raw * 1.25e-3f; // 1.25 mV per bit
    if (dev->bus_V_scaling_e4 != 10000) {
        voltage *= (dev->bus_V_scaling_e4 * 1.0e-4f);
    }
    return voltage;
}

float INA226_GetShuntVoltage(INA226_Handle *dev)
{
    int16_t raw = (int16_t)rd16(dev, INA226_SHUNT_VOLTAGE);
    return (float)raw * 2.5e-6f; // 2.5 uV per bit
}

float INA226_GetCurrent(INA226_Handle *dev)
{
    int16_t raw = (int16_t)rd16(dev, INA226_CURRENT);
    return (float)raw * dev->current_LSB - dev->current_zero_offset;
}

float INA226_GetPower(INA226_Handle *dev)
{
    uint16_t raw = rd16(dev, INA226_POWER);
    return (float)raw * (dev->current_LSB * 25.0f); // fixed LSB: 25 * current_LSB W/bit
}

bool INA226_IsConversionReady(INA226_Handle *dev)
{
    uint16_t mask = rd16(dev, INA226_MASK_ENABLE);
    return (mask & INA226_CONVERSION_READY_FLAG) == INA226_CONVERSION_READY_FLAG;
}

bool INA226_WaitConversionReady(INA226_Handle *dev, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) <= timeout_ms) {
        if (INA226_IsConversionReady(dev)) return true;
        HAL_Delay(1);
    }
    return false;
}

// -------- Configuration --------

bool INA226_Reset(INA226_Handle *dev)
{
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    cfg |= INA226_CONF_RESET_MASK;
    uint16_t res = wr16(dev, INA226_CONFIGURATION, cfg);
    if (res != 0) return false;
    dev->current_LSB = 0.0f;
    dev->maxCurrent  = 0.0f;
    dev->shunt       = 0.0f;
    dev->current_zero_offset = 0.0f;
    dev->bus_V_scaling_e4 = 10000;
    return true;
}

bool INA226_SetAverage(INA226_Handle *dev, uint8_t avg)
{
    if (avg > 7) return false;
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    cfg &= (uint16_t)~INA226_CONF_AVERAGE_MASK;
    cfg |= (uint16_t)(avg << 9);
    wr16(dev, INA226_CONFIGURATION, cfg);
    return true;
}

uint8_t INA226_GetAverage(INA226_Handle *dev)
{
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    cfg &= INA226_CONF_AVERAGE_MASK;
    return (uint8_t)(cfg >> 9);
}

bool INA226_SetBusConvTime(INA226_Handle *dev, uint8_t bvct)
{
    if (bvct > 7) return false;
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    cfg &= (uint16_t)~INA226_CONF_BUSVC_MASK;
    cfg |= (uint16_t)(bvct << 6);
    wr16(dev, INA226_CONFIGURATION, cfg);
    return true;
}

uint8_t INA226_GetBusConvTime(INA226_Handle *dev)
{
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    cfg &= INA226_CONF_BUSVC_MASK;
    return (uint8_t)(cfg >> 6);
}

bool INA226_SetShuntConvTime(INA226_Handle *dev, uint8_t svct)
{
    if (svct > 7) return false;
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    cfg &= (uint16_t)~INA226_CONF_SHUNTVC_MASK;
    cfg |= (uint16_t)(svct << 3);
    wr16(dev, INA226_CONFIGURATION, cfg);
    return true;
}

uint8_t INA226_GetShuntConvTime(INA226_Handle *dev)
{
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    cfg &= INA226_CONF_SHUNTVC_MASK;
    return (uint8_t)(cfg >> 3);
}

// -------- Calibration --------

int INA226_SetMaxCurrentShunt(INA226_Handle *dev, float maxCurrent, float shunt, bool normalize)
{
    // Datasheet limit minus 0.02 mV to avoid overflow (matches original lib behavior)
    float shuntVoltage = maxCurrent * shunt;
    if (shuntVoltage > 0.08190f)           return INA226_ERR_SHUNTVOLTAGE_HIGH;
    if (maxCurrent < 0.001f)               return INA226_ERR_MAXCURRENT_LOW;
    if (shunt < INA226_MINIMAL_SHUNT_OHM)  return INA226_ERR_SHUNT_LOW;

    float current_LSB = maxCurrent * 3.0517578125e-5f; // maxCurrent/32768

    // Ensure calibration register fits when shunt is very small
    float minLSBfromShunt = 1.5625e-7f / shunt; // ~= 0.00000015625 / shunt
    if (minLSBfromShunt > current_LSB) current_LSB = minLSBfromShunt;

    if (normalize) {
        // Normalize to 1,2,5 * 10^n uA steps (as in the Arduino lib)
        uint16_t currentLSB_uA = (uint16_t)(current_LSB * 1e6f);
        currentLSB_uA++; // ceil approx
        uint16_t factor = 1;
        uint8_t i = 0;
        bool ok = false;
        do {
            if (1 * factor >= currentLSB_uA) { current_LSB = 1 * factor * 1e-6f; ok = true; }
            else if (2 * factor >= currentLSB_uA) { current_LSB = 2 * factor * 1e-6f; ok = true; }
            else if (5 * factor >= currentLSB_uA) { current_LSB = 5 * factor * 1e-6f; ok = true; }
            else { factor *= 10; i++; }
        } while ((i < 4) && (!ok));
        if (!ok) return INA226_ERR_NORMALIZE_FAILED;
    }

    // Compute calibration; auto-scale if needed to fit <= 32767
    float cal_f = 0.00512f / (current_LSB * shunt);
    uint32_t calib = (uint32_t)lroundf(cal_f);
    while (calib > 32767U) {
        current_LSB *= 2.0f;
        calib >>= 1;
    }

    // Write calibration
    wr16(dev, INA226_CALIBRATION, (uint16_t)calib);

    // Store params
    dev->current_LSB = current_LSB;
    dev->maxCurrent  = current_LSB * 32768.0f;
    dev->shunt       = shunt;
    dev->current_zero_offset = 0.0f;
    dev->bus_V_scaling_e4 = 10000;

    return INA226_ERR_NONE;
}

int INA226_Configure(INA226_Handle *dev, float shunt, float current_LSB_mA, float current_zero_offset_mA, uint16_t bus_V_scaling_e4)
{
    if (shunt < INA226_MINIMAL_SHUNT_OHM) return INA226_ERR_SHUNT_LOW;

    float current_LSB = current_LSB_mA * 1e-3f;
    float maxCurrent  = fminf((INA226_MAX_SHUNT_VOLTAGE / shunt), 32768.0f * current_LSB);

    if (maxCurrent < 0.001f) return INA226_ERR_MAXCURRENT_LOW;

    dev->shunt = shunt;
    dev->current_LSB = current_LSB;
    dev->current_zero_offset = current_zero_offset_mA * 1e-3f;
    dev->bus_V_scaling_e4 = bus_V_scaling_e4;
    dev->maxCurrent = maxCurrent;

    uint32_t calib = (uint32_t)lroundf(0.00512f / (dev->current_LSB * dev->shunt));
    wr16(dev, INA226_CALIBRATION, (uint16_t)calib);

    return INA226_ERR_NONE;
}

bool INA226_IsCalibrated(INA226_Handle *dev)
{
    return (dev->current_LSB != 0.0f);
}

// -------- Modes --------

bool INA226_SetMode(INA226_Handle *dev, uint8_t mode)
{
    if (mode > 7) return false;
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    cfg &= (uint16_t)~INA226_CONF_MODE_MASK;
    cfg |= mode;
    wr16(dev, INA226_CONFIGURATION, cfg);
    return true;
}

uint8_t INA226_GetMode(INA226_Handle *dev)
{
    uint16_t cfg = rd16(dev, INA226_CONFIGURATION);
    return (uint8_t)(cfg & INA226_CONF_MODE_MASK);
}

// -------- Alerts --------

bool INA226_SetAlertRegister(INA226_Handle *dev, uint16_t mask)
{
    uint16_t res = wr16(dev, INA226_MASK_ENABLE, (uint16_t)(mask & 0xFC00));
    return (res == 0);
}

uint16_t INA226_GetAlertFlag(INA226_Handle *dev)
{
    return (uint16_t)(rd16(dev, INA226_MASK_ENABLE) & 0x001F);
}

bool INA226_SetAlertLimit(INA226_Handle *dev, uint16_t limit)
{
    uint16_t res = wr16(dev, INA226_ALERT_LIMIT, limit);
    return (res == 0);
}

uint16_t INA226_GetAlertLimit(INA226_Handle *dev)
{
    return rd16(dev, INA226_ALERT_LIMIT);
}

// -------- Meta --------

uint16_t INA226_GetManufacturerID(INA226_Handle *dev)
{
    return rd16(dev, INA226_MANUFACTURER);
}

uint16_t INA226_GetDieID(INA226_Handle *dev)
{
    return rd16(dev, INA226_DIE_ID);
}

// -------- Debug / error --------

uint16_t INA226_GetRegister(INA226_Handle *dev, uint8_t reg)
{
    return rd16(dev, reg);
}

int INA226_GetLastError(INA226_Handle *dev)
{
    int e = dev->error;
    dev->error = 0;
    return e;
}

// ======================= Private I2C =========================

// Note: HAL_I2C_Mem_* expects the 7-bit address left-shifted by 1.
// We stored dev->address already shifted in Init.

static uint16_t rd16(INA226_Handle *dev, uint8_t reg)
{
    dev->error = 0;
    uint8_t buf[2] = {0};
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(dev->hi2c, dev->address, reg, I2C_MEMADD_SIZE_8BIT,
                                            buf, 2, dev->i2c_timeout_ms);
    if (st != HAL_OK) {
        dev->error = -1;
        return 0;
    }
    // INA226 is big-endian
    uint16_t val = ((uint16_t)buf[0] << 8) | buf[1];
    return val;
}

static uint16_t wr16(INA226_Handle *dev, uint8_t reg, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(value >> 8);
    buf[1] = (uint8_t)(value & 0xFF);
    HAL_StatusTypeDef st = HAL_I2C_Mem_Write(dev->hi2c, dev->address, reg, I2C_MEMADD_SIZE_8BIT,
                                             buf, 2, dev->i2c_timeout_ms);
    if (st != HAL_OK) {
        dev->error = -1;
        return (uint16_t)st;
    }
    return 0;
}
