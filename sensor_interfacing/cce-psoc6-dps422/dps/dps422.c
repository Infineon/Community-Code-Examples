/******************************************************************************
* File Name:  dps422.c
*
* Description:  This file contains the function definitions for all DPS422
*               APIs.
*
*******************************************************************************/

#include "dps422.h"

/* Calculation factor kP associated with each oversampling rate */
const int32_t DPS_422_scaling_facts[DPS__NUM_OF_SCAL_FACTS] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

/*******************************************************************************
* Function Name: DPS_422_Init
********************************************************************************
*
* Summary:
*  This function initializes the sensor. It reads the pressure calibration
*  coefficients, sets the necessary bits of TEMP_CFG register, configures
*  measurement and oversampling rate for temperature and pressure measurement
*  and performs necessary correction for ICs with fuse bit problem.
*
*******************************************************************************/
void DPS_422_Init (cyhal_i2c_t *I2C_DPS_422)
{
	I2C_ptr = I2C_DPS_422;
	/* This flag will show if the initialization was successful */
	m_initFail = 0U;

	/* Set I2C bus connection */
	m_SpiI2c = 1U;

	cyhal_system_delay_ms(50);

	/* Read pressure calibration coefficients */
	/* Sets TEMP_CFG[7] to '1' to configure the temperature measurement correctly */
	if (DPS_422_readcoeffs() < 0 || DPS_422_writeByteBitfield_reg(0x01, registers[MUST_SET]) < 0)
	{
		m_initFail = 1U;
		return;
	}
	DPS_422_configTemp(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);
	DPS_422_configPressure(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);

	DPS_422_correctTemp();
}

/*******************************************************************************
* Function Name: DPS_422_standby
********************************************************************************
*
* Summary:
*  This function sets the operation mode of the device to standby.
*
*******************************************************************************/
int16_t DPS_422_standby(void)
{
	/* Abort if initialization failed */
	if (m_initFail)
	{
		return DPS_FAIL;
	}
	/* Set device to idling mode */
	int16_t ret = DPS_422_setOpMode(IDLE);
	if (ret != DPS_SUCCESS)
	{
		return ret;
	}
	ret = DPS_422_disableFIFO();
	return ret;
}

/*******************************************************************************
* Function Name: DPS_422_setOpMode
********************************************************************************
*
* Summary:
*  This function sets the operation mode of the device as per the parameter
*  received.
*
*******************************************************************************/
int16_t DPS_422_setOpMode(uint8_t opMode)
{
	if (DPS_422_writeByteBitfield_reg(opMode, config_registers[MSR_CTRL]) == -1)
	{
		return DPS_FAIL;
	}
	m_opMode = (enum Mode)opMode;
	return DPS_SUCCESS;
}

/*******************************************************************************
* Function Name: DPS_422_getTwosComplement
********************************************************************************
*
* Summary:
*  This function calculates the 2's complement of the parameter received.
*
*******************************************************************************/
void DPS_422_getTwosComplement(int32_t *raw, uint8_t length)
{
	if (*raw & ((uint32_t)1 << (length - 1)))
	{
		*raw -= (uint32_t)1 << length;
	}
}

/*******************************************************************************
* Function Name: DPS_422_configTemp
********************************************************************************
*
* Summary:
*  This function sets the temperature measurement and oversampling rate.
*
*******************************************************************************/
int16_t DPS_422_configTemp(uint8_t tempMr, uint8_t tempOsr)
{
	tempMr &= 0x07;
	tempOsr &= 0x07;
	/* Two accesses to the same register; for readability */
	int16_t ret = DPS_422_writeByteBitfield_reg(tempMr, config_registers[TEMP_MR]);
	ret = DPS_422_writeByteBitfield_reg(tempOsr, config_registers[TEMP_OSR]);

	/* Abort immediately on fail */
	if (ret != DPS_SUCCESS)
	{
		return DPS_FAIL;
	}
	m_tempMr = tempMr;
	m_tempOsr = tempOsr;

	return ret;
}

/*******************************************************************************
* Function Name: DPS_422_configPressure
********************************************************************************
*
* Summary:
*  This function sets the pressure measurement and oversampling rate.
*
*******************************************************************************/
int16_t DPS_422_configPressure(uint8_t prsMr, uint8_t prsOsr)
{
	prsMr &= 0x07;
	prsOsr &= 0x07;
	int16_t ret = DPS_422_writeByteBitfield_reg(prsMr, config_registers[PRS_MR]);
	ret = DPS_422_writeByteBitfield_reg(prsOsr, config_registers[PRS_OSR]);

	/* Abort immediately on fail */
	if (ret != DPS_SUCCESS)
	{
		return DPS_FAIL;
	}
	m_prsMr = prsMr;
	m_prsOsr = prsOsr;

	return ret;
}

/*******************************************************************************
* Function Name: DPS_422_calcBusyTime
********************************************************************************
*
* Summary:
*  This function calculates the measurement time.
*
*******************************************************************************/
uint16_t DPS_422_calcBusyTime(uint16_t mr, uint16_t osr)
{
	return ((uint32_t)20U << mr) + ((uint32_t)32U << (osr + mr));
}

/*******************************************************************************
* Function Name: DPS_422_correctTemp
********************************************************************************
*
* Summary:
*  This function performs necessary register writes to fix ICs with a fuse bit
*  problem, which lead to a wrong temperature. It should not affect ICs without
*  this problem.
*
*******************************************************************************/
int16_t DPS_422_correctTemp(void)
{
	if (m_initFail)
	{
		return DPS_FAIL;
	}
	DPS_422_writeByte(0x0E, 0xA5, 0);
	DPS_422_writeByte(0x0F, 0x96, 0);
	DPS_422_writeByte(0x62, 0x02, 0);
	DPS_422_writeByte(0x0E, 0x00, 0);
	DPS_422_writeByte(0x0F, 0x00, 0);

	/* Perform a first temperature measurement (again) */
	/* The most recent temperature will be saved internally */
	/* and used for compensation when calculating pressure */
	float trash;
	DPS_422_measureTempOnceOversamplingRate(&trash, m_tempOsr);

	return DPS_SUCCESS;
}

/*******************************************************************************
* Function Name: DPS_422_calcTemp
********************************************************************************
*
* Summary:
*  This function calculates final temperature value from raw sensor data.
*
*******************************************************************************/
float DPS_422_calcTemp(int32_t raw)
{
	m_lastTempScal = (float)raw / 1048576;
	float u = m_lastTempScal / (1 + DPS422_ALPHA * m_lastTempScal);
	return ((a_prime * u) + b_prime);
}

/*******************************************************************************
* Function Name: DPS_422_calcPressure
********************************************************************************
*
* Summary:
*  This function calculates final pressure value from raw sensor data.
*
*******************************************************************************/
float DPS_422_calcPressure(int32_t raw_prs)
{
	float prs = raw_prs;
	prs /= DPS_422_scaling_facts[m_prsOsr];

	float temp = (8.5 * m_lastTempScal) / (1 + 8.8 * m_lastTempScal);

	prs = m_c00 + m_c10 * prs + m_c01 * temp + m_c20 * prs * prs + m_c02 * temp * temp + m_c30 * prs * prs * prs +
		  m_c11 * temp * prs + m_c12 * prs * temp * temp + m_c21 * prs * prs * temp;
	return prs;
}

/*******************************************************************************
* Function Name: DPS_422_getSingleResult
********************************************************************************
*
* Summary:
*  This function provides the final temperature/pressure value. It reads the
*  ready bit from the configuration registers to check if the sensor reading is
*  ready. If the ready bit set, the raw data from the sensor is read and final
*  temperature/pressure value is calculated.
*
*******************************************************************************/
int16_t DPS_422_getSingleResult(float *result)
{
	/* Abort if initialization failed */
	if (m_initFail)
	{
		return DPS_FAIL;
	}

	/* Read finished bit for current opMode */
	int16_t rdy;
	switch (m_opMode)
	{
	case CMD_TEMP: /* temperature */
		rdy = DPS_422_readByteBitfield(config_registers[TEMP_RDY]);
		break;
	case CMD_PRS: /* pressure */
		rdy = DPS_422_readByteBitfield(config_registers[PRS_RDY]);
		break;
	default: /* DPS422 not in command mode */
		return DPS_FAIL;
	}
	/* Read new measurement result */
	switch (rdy)
	{
		case DPS_FAIL: /* Could not read ready flag */
			return DPS_FAIL;
		case 0: /* Ready flag not set, measurement still in progress */
			return DPS_FAIL;
		case 1: /* Measurement ready, expected case */
		{
			enum Mode oldMode = m_opMode;
			m_opMode = IDLE; /* Operation mode was automatically reseted by DPS422 */
			int32_t raw_val = 0;
			switch (oldMode)
			{
			case CMD_TEMP: /* temperature */
				DPS_422_getRawResult(&raw_val, registerBlocks[TEMP]);
				*result = DPS_422_calcTemp(raw_val);
				return DPS_SUCCESS;
			case CMD_PRS: /* pressure */
				DPS_422_getRawResult(&raw_val, registerBlocks[PRS]);
				*result = DPS_422_calcPressure(raw_val);
				return DPS_SUCCESS;
			default:
				return DPS_FAIL; /* Should already be filtered above */
			}
		}
	}
	return DPS_FAIL;
}

/*******************************************************************************
* Function Name: DPS_422_getRawResult
********************************************************************************
*
* Summary:
*  This function reads raw data from the sensor and calculates 2's complement.
*
*******************************************************************************/
int16_t DPS_422_getRawResult(int32_t *raw, RegBlock_t reg)
{
	uint8_t buffer[DPS__RESULT_BLOCK_LENGTH] = {0};
	if (DPS_422_readBlock(reg, buffer) != DPS__RESULT_BLOCK_LENGTH)
		return DPS_FAIL;

	*raw = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];
	DPS_422_getTwosComplement(raw, 24);
	return DPS_SUCCESS;
}

/*******************************************************************************
* Function Name: DPS_422_measureTempOnceOversamplingRate
********************************************************************************
*
* Summary:
*  This function sets the operation mode of the sensor to one shot temperature
*  measurement and reads the temperature data.
*
*******************************************************************************/
int16_t DPS_422_measureTempOnceOversamplingRate(float *result, uint8_t oversamplingRate)
{
	/* Start measurement */
	int16_t ret = DPS_422_setModeTempMeasurementOneShot(oversamplingRate);
	if (ret != DPS_SUCCESS)
	{
		return ret;
	}

	/* Wait until measurement is finished */
	cyhal_system_delay_ms(DPS_422_calcBusyTime(0U, m_tempOsr) / DPS__BUSYTIME_SCALING);
	cyhal_system_delay_ms(DPS_422__BUSYTIME_FAILSAFE);

	ret = DPS_422_getSingleResult(result);
	if (ret != DPS_SUCCESS)
	{
		DPS_422_standby();
	}
	return ret;
}

/*******************************************************************************
* Function Name: DPS_422_setModeTempMeasurementOneShot
********************************************************************************
*
* Summary:
*  This function sets the operation mode of the sensor to one shot temperature
*  measurement.
*
*******************************************************************************/
int16_t DPS_422_setModeTempMeasurementOneShot(uint8_t oversamplingRate)
{
	/* Abort if initialization failed */
	if (m_initFail)
	{
		return DPS_FAIL;
	}
	/* Abort if device is not in idling mode */
	if (m_opMode != IDLE)
	{
		return DPS_FAIL;
	}

	if (oversamplingRate != m_tempOsr)
	{
		/* Configuration of oversampling rate, and lowest measure rate to avoid conflicts */
		if (DPS_422_configTemp(0U, oversamplingRate) != DPS_SUCCESS)
		{
			return DPS_FAIL;
		}
	}

	/* Set device to temperature measuring mode */
	return DPS_422_setOpMode(CMD_TEMP);
}

/*******************************************************************************
* Function Name: DPS_422_measurePressureOnceOversamplingRate
********************************************************************************
*
* Summary:
*  This function sets the operation mode of the sensor to one shot pressure
*  measurement and reads the pressure data.
*
*******************************************************************************/
int16_t DPS_422_measurePressureOnceOversamplingRate(float* result, uint8_t oversamplingRate)
{
	/* Start the measurement */
	/* Set oversampling rate */
	/* Set mode of operation to one shot pressure measurement */
	int16_t ret = DPS_422_setModePressureMeasurementOneShot(oversamplingRate);
	if (ret != DPS_SUCCESS)
	{
		return ret;
	}

	/* Wait until measurement is finished */
	cyhal_system_delay_ms(DPS_422_calcBusyTime(0U, m_prsOsr) / DPS__BUSYTIME_SCALING);
	cyhal_system_delay_ms(DPS_422__BUSYTIME_FAILSAFE);

	ret = DPS_422_getSingleResult(result);
	if (ret != DPS_SUCCESS)
	{
		DPS_422_standby();
	}
	return ret;
}

/*******************************************************************************
* Function Name: DPS_422_setModePressureMeasurementOneShot
********************************************************************************
*
* Summary:
*  This function sets the operation mode of the sensor to one shot pressure
*  measurement.
*
*******************************************************************************/
int16_t DPS_422_setModePressureMeasurementOneShot(uint8_t oversamplingRate)
{
	/* Abort if initialization failed */
	if (m_initFail)
	{
		return DPS_FAIL;
	}
	/* Abort if device is not in idling mode */
	if (m_opMode != IDLE)
	{
		return DPS_FAIL;
	}
	/* Configuration of oversampling rate, and lowest measure rate to avoid conflicts */
	if (oversamplingRate != m_prsOsr)
	{
		if (DPS_422_configPressure(0U, oversamplingRate))
		{
			return DPS_FAIL;
		}
	}
	/* Set device to pressure measuring mode */
	return DPS_422_setOpMode(CMD_PRS);
}

/*******************************************************************************
* Function Name: DPS_422_flushFIFO
********************************************************************************
*
* Summary:
*  This function sets the FIFO flush bit of RESET register that clears all data
*  in the measurement results FIFO.
*
*******************************************************************************/
int16_t DPS_422_flushFIFO()
{
	return DPS_422_writeByteBitfield_reg(1U, registers[FIFO_FL]);
}

/*******************************************************************************
* Function Name: DPS_422_disableFIFO
********************************************************************************
*
* Summary:
*  This function disables the FIFO. Old results are not stored. Pressure and
*  temperature results are henceforth, stored in respective results registers.
*
*******************************************************************************/
int16_t DPS_422_disableFIFO()
{
	int16_t ret = DPS_422_flushFIFO();
	ret = DPS_422_writeByteBitfield_reg(0U, config_registers[FIFO_EN]);
	return ret;
}

/*******************************************************************************
* Function Name: DPS_422_writeByteBitfield_reg
********************************************************************************
*
* Summary:
*  This function writes to specific bit fields of registers.
*
*******************************************************************************/
int16_t DPS_422_writeByteBitfield_reg(uint8_t data, RegMask_t regMask)
{
	return DPS_422_writeByteBitfield(data, regMask.regAddress, regMask.mask, regMask.shift, 0U);
}

/*******************************************************************************
* Function Name: DPS_422_writeByteBitfield
********************************************************************************
*
* Summary:
*  This function writes to sensor registers.
*
*******************************************************************************/
int16_t DPS_422_writeByteBitfield(uint8_t data, uint8_t regAddress, uint8_t mask, uint8_t shift, uint8_t check)
{
	int16_t old = DPS_422_readByte(regAddress);
	if (old < 0)
	{
		/* Fail while reading */
		return old;
	}
	return DPS_422_writeByte(regAddress, ((uint8_t)old & ~mask) | ((data << shift) & mask), check);
}

/*******************************************************************************
* Function Name: DPS_422_writeByte
********************************************************************************
*
* Summary:
*  This function performs the I2C write operation.
*
*******************************************************************************/
int16_t DPS_422_writeByte(uint8_t regAddress, uint8_t data, uint8_t check)
{
	cy_rslt_t result;
	result = cyhal_i2c_master_mem_write(I2C_ptr, DPS__STD_SLAVE_ADDRESS, regAddress, 0x01, &data, 1, 200);
	if(result == CY_RSLT_SUCCESS)
	{
		if (check == 0)
			return DPS_SUCCESS;					      /* No checking */
		else
		{
			if (DPS_422_readByte(regAddress) == data) /* Check if desired by calling function */
			{
				return DPS_SUCCESS;
			}
			else
			{
				return DPS_FAIL;
			}
		}
	}
	else
		return result;
}

/*******************************************************************************
* Function Name: DPS_422_readByteBitfield
********************************************************************************
*
* Summary:
*  This function reads from sensor registers and returns specific bit fields.
*
*******************************************************************************/
int16_t DPS_422_readByteBitfield(RegMask_t regMask)
{
	int16_t ret = DPS_422_readByte(regMask.regAddress);
	if (ret < 0)
	{
		return ret;
	}
	return (((uint8_t)ret) & regMask.mask) >> regMask.shift;
}

/*******************************************************************************
* Function Name: DPS_422_readByte
********************************************************************************
*
* Summary:
*  This function performs the I2C read operation.
*
*******************************************************************************/
int16_t DPS_422_readByte(uint8_t regAddress)
{
	cy_rslt_t result;
	uint8_t value;
	result = cyhal_i2c_master_mem_read(I2C_ptr, DPS__STD_SLAVE_ADDRESS, regAddress, 1, &value, 1, 200);
	if(result == CY_RSLT_SUCCESS)
	{
		return value;
	}
	else
		return DPS_FAIL;
}

/*******************************************************************************
* Function Name: DPS_422_readcoeffs
********************************************************************************
*
* Summary:
*  This function reads calibration coefficients from sensor registers.
*
*******************************************************************************/
int16_t DPS_422_readcoeffs(void)
{
	uint8_t buffer_temp[3];
	uint8_t buffer_prs[20];
	DPS_422_readBlock(coeffBlocks[TEMP], buffer_temp);
	DPS_422_readBlock(coeffBlocks[PRS], buffer_prs);

	// refer to datasheet
	// 1. read T_Vbe, T_dVbe and T_gain
	int32_t t_gain = buffer_temp[0];													 // 8 bits
	int32_t t_dVbe = (uint32_t)buffer_temp[1] >> 1;										 // 7 bits
	int32_t t_Vbe = ((uint32_t)buffer_temp[1] & 0x01) | ((uint32_t)buffer_temp[2] << 1); // 9 bits

	DPS_422_getTwosComplement(&t_gain, 8);
	DPS_422_getTwosComplement(&t_dVbe, 7);
	DPS_422_getTwosComplement(&t_Vbe, 9);

	// 2. Vbe, dVbe and Aadc
	float Vbe = t_Vbe * 1.05031e-4 + 0.463232422;
	float dVbe = t_dVbe * 1.25885e-5 + 0.04027621;
	float Aadc = t_gain * 8.4375e-5 + 0.675;
	// 3. Vbe_cal and dVbe_cal
	float Vbe_cal = Vbe / Aadc;
	float dVbe_cal = dVbe / Aadc;
	// 4. T_calib
	float T_calib = DPS422_A_0 * dVbe_cal - 273.15;
	// 5. Vbe_cal(T_ref): Vbe value at reference temperature
	float Vbe_cal_tref = Vbe_cal - (T_calib - DPS422_T_REF) * DPS422_T_C_VBE;
	// 6. alculate PTAT correction coefficient
	float k_ptat = (DPS422_V_BE_TARGET - Vbe_cal_tref) * DPS422_K_PTAT_CORNER + DPS422_K_PTAT_CURVATURE;
	// 7. calculate A' and B'
	a_prime = DPS422_A_0 * (Vbe_cal + DPS422_ALPHA * dVbe_cal) * (1 + k_ptat);
	b_prime = -273.15 * (1 + k_ptat) - k_ptat * T_calib;

	// c00, c01, c02, c10 : 20 bits
	// c11, c12: 17 bits
	// c20: 15 bits; c21: 14 bits; c30 12 bits
	m_c00 = ((uint32_t)buffer_prs[0] << 12) | ((uint32_t)buffer_prs[1] << 4) | (((uint32_t)buffer_prs[2] & 0xF0) >> 4);
	m_c10 = ((uint32_t)(buffer_prs[2] & 0x0F) << 16) | ((uint32_t)buffer_prs[3] << 8) | (uint32_t)buffer_prs[4];
	m_c01 = ((uint32_t)buffer_prs[5] << 12) | ((uint32_t)buffer_prs[6] << 4) | (((uint32_t)buffer_prs[7] & 0xF0) >> 4);
	m_c02 = ((uint32_t)(buffer_prs[7] & 0x0F) << 16) | ((uint32_t)buffer_prs[8] << 8) | (uint32_t)buffer_prs[9];
	m_c20 = ((uint32_t)(buffer_prs[10] & 0x7F) << 8) | (uint32_t)buffer_prs[11];
	m_c30 = ((uint32_t)(buffer_prs[12] & 0x0F) << 8) | (uint32_t)buffer_prs[13];
	m_c11 = ((uint32_t)buffer_prs[14] << 9) | ((uint32_t)buffer_prs[15] << 1) | (((uint32_t)buffer_prs[16] & 0x80) >> 7);
	m_c12 = (((uint32_t)buffer_prs[16] & 0x7F) << 10) | ((uint32_t)buffer_prs[17] << 2) | (((uint32_t)buffer_prs[18] & 0xC0) >> 6);
	m_c21 = (((uint32_t)buffer_prs[18] & 0x3F) << 8) | ((uint32_t)buffer_prs[19]);

	DPS_422_getTwosComplement(&m_c00, 20);
	DPS_422_getTwosComplement(&m_c01, 20);
	DPS_422_getTwosComplement(&m_c02, 20);
	DPS_422_getTwosComplement(&m_c10, 20);
	DPS_422_getTwosComplement(&m_c11, 17);
	DPS_422_getTwosComplement(&m_c12, 17);
	DPS_422_getTwosComplement(&m_c20, 15);
	DPS_422_getTwosComplement(&m_c21, 14);
	DPS_422_getTwosComplement(&m_c30, 12);

	return DPS_SUCCESS;
}

/*******************************************************************************
* Function Name: DPS_422_readBlock
********************************************************************************
*
* Summary:
*  This function performs I2C read operation over a block of sensor registers.
*
*******************************************************************************/
int16_t DPS_422_readBlock(RegBlock_t regBlock, uint8_t *buffer)
{
	//do not read if there is no buffer
	if (buffer == NULL)
	{
		return 0; //0 bytes read successfully
	}

	cy_rslt_t result;
	result = cyhal_i2c_master_mem_read(I2C_ptr, DPS__STD_SLAVE_ADDRESS, regBlock.regAddress, 1, buffer, regBlock.length, 200);
	if(result == CY_RSLT_SUCCESS)
	{
		return regBlock.length;
	}
	else
		return DPS_FAIL;
}


