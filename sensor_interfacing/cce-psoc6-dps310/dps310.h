/******************************************************************************
* File Name: dsp310.h
*
* Description: This file is a port of the DSP-310 Pressure Sensor library..
*
* Related Document: See https://github.com/Infineon/DPS310-Pressure-Sensor
*
*******************************************************************************/

#ifndef DPS310_H_
#define DPS310_H_

#include "dps310_config.h"
#include "cy_retarget_io.h"

//flags
uint8_t m_initFail;
uint8_t m_productID;
uint8_t m_revisionID;

//settings
uint8_t m_tempMr;
uint8_t m_tempOsr;
uint8_t m_prsMr;
uint8_t m_prsOsr;

// compensation coefficients for both dps310 and dps422
int32_t m_c00;
int32_t m_c10;
int32_t m_c01;
int32_t m_c11;
int32_t m_c20;
int32_t m_c21;
int32_t m_c30;

uint8_t m_tempSensor;

//compensation coefficients for dps310
int32_t m_c0Half;
int32_t m_c1;

// last measured scaled temperature (necessary for pressure compensation)
float m_lastTempScal;

//bus specific
uint8_t m_SpiI2c; //0=SPI, 1=I2C

cyhal_i2c_t* i2c_ptr;

enum Mode m_opMode;

void DPS310_init(cyhal_i2c_t* i2c_inst);
int16_t DPS310_standby(void);

int16_t DPS310_setOpMode(uint8_t opMode);
int16_t DPS310_configTemp(uint8_t tempMr, uint8_t tempOsr);
int16_t DPS310_configPressure(uint8_t prsMr, uint8_t prsOsr);
uint16_t DPS310_calcBusyTime(uint16_t mr, uint16_t osr);
void DPS310_getTwosComplement(int32_t *raw, uint8_t length);

int16_t DPS310_correctTemp(void);

float DPS310_calcTemp(int32_t raw);
float DPS310_calcPressure(int32_t raw);

int16_t DPS310_getSingleResult(float* result);
int16_t DPS310_getRawResult(int32_t *raw, RegBlock_t reg);

int16_t DPS310_measureTempOnce(float* result);
int16_t DPS310_measureTempOnce_oversample(float* result, uint8_t oversamplingRate);
int16_t DPS310_startMeasureTempOnce_void(void);
int16_t DPS310_startMeasureTempOnce(uint8_t oversamplingRate);

int16_t DPS310_measurePressureOnce(float* result);
int16_t DPS310_measurePressureOnce_oversample(float* result, uint8_t oversamplingRate);
int16_t DPS310_startMeasurePressureOnce_void(void);
int16_t DPS310_startMeasurePressureOnce(uint8_t oversamplingRate);

int16_t DPS310_disableFIFO();
int16_t DPS310_flushFIFO();

int16_t DPS310_readByte(uint8_t regAddress);
int16_t DPS310_readByteBitfield(RegMask_t regMask);
int16_t DPS310_readBlock(RegBlock_t regBlock, uint8_t *buffer);
int16_t DPS310_readcoeffs(void);

int16_t DPS310_writeByte(uint8_t regAddress, uint8_t data, uint8_t check);
int16_t DPS310_writeByteBitfield_reg(uint8_t data, RegMask_t regMask);
int16_t DPS310_writeByteBitfield(uint8_t data, uint8_t regAddress, uint8_t mask, uint8_t shift, uint8_t check);

#endif /* DPS310_H_ */
