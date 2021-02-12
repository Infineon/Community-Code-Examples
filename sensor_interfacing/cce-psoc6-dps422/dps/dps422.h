/******************************************************************************
* File Name:  dps422.h
*
* Description:  This file contains the function prototypes for all DPS422
*               APIs and global variables required for sensor data processing.
*
*******************************************************************************/

#ifndef DPS_DPS422_H_
#define DPS_DPS422_H_

#include "dps422_config.h"

/*******************************************************************************
 * Global variable
 *******************************************************************************/
/* Flags */
uint8_t m_initFail;

/* Settings */
uint8_t m_tempMr;
uint8_t m_tempOsr;
uint8_t m_prsMr;
uint8_t m_prsOsr;

/* Compensation coefficients */
int32_t m_c00;
int32_t m_c10;
int32_t m_c01;
int32_t m_c11;
int32_t m_c20;
int32_t m_c21;
int32_t m_c30;

/* Compensation coefficients (for simplicity use 32 bits) */
float a_prime;
float b_prime;
int32_t m_c02;
int32_t m_c12;

/* Last measured scaled temperature (necessary for pressure compensation) */
float m_lastTempScal;

/* Bus specific */
uint8_t m_SpiI2c; 			/* 0=SPI, 1=I2C */

cyhal_i2c_t *I2C_ptr;

enum Mode m_opMode;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void DPS_422_Init (cyhal_i2c_t *I2C_DPS_422);
int16_t DPS_422_standby(void);
int16_t DPS_422_setOpMode(uint8_t opMode);
void DPS_422_getTwosComplement(int32_t *raw, uint8_t length);
int16_t DPS_422_configTemp(uint8_t tempMr, uint8_t tempOsr);
int16_t DPS_422_configPressure(uint8_t prsMr, uint8_t prsOsr);
uint16_t DPS_422_calcBusyTime(uint16_t mr, uint16_t osr);
int16_t DPS_422_correctTemp(void);
float DPS_422_calcTemp(int32_t raw);
float DPS_422_calcPressure(int32_t raw_prs);
int16_t DPS_422_getSingleResult(float *result);
int16_t DPS_422_getRawResult(int32_t *raw, RegBlock_t reg);
int16_t DPS_422_measureTempOnceOversamplingRate(float *result, uint8_t oversamplingRate);
int16_t DPS_422_setModeTempMeasurementOneShot(uint8_t oversamplingRate);
int16_t DPS_422_measurePressureOnceOversamplingRate(float* result, uint8_t oversamplingRate);
int16_t DPS_422_setModePressureMeasurementOneShot(uint8_t oversamplingRate);
int16_t DPS_422_flushFIFO();
int16_t DPS_422_disableFIFO();
int16_t DPS_422_writeByteBitfield_reg(uint8_t data, RegMask_t regMask);
int16_t DPS_422_writeByteBitfield(uint8_t data, uint8_t regAddress, uint8_t mask, uint8_t shift, uint8_t check);
int16_t DPS_422_writeByte(uint8_t regAddress, uint8_t data, uint8_t check);
int16_t DPS_422_readByteBitfield(RegMask_t regMask);
int16_t DPS_422_readByte(uint8_t regAddress);
int16_t DPS_422_readcoeffs(void);
int16_t DPS_422_readBlock(RegBlock_t regBlock, uint8_t *buffer);



#endif /* DPS_DPS422_H_ */
