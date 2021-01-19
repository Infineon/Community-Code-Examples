/******************************************************************************
* File Name:  dps310.c
*
* Description:  This file is a port of the DSP-310 Pressure Sensor library.
*
* Related Document: See https://github.com/Infineon/DPS310-Pressure-Sensor
*
*******************************************************************************/

#include "dps310.h"

const int32_t DPS310_scaling_facts[DPS__NUM_OF_SCAL_FACTS] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

void DPS310_init(cyhal_i2c_t* i2c_inst)
{
    i2c_ptr = i2c_inst;
    //this flag will show if the initialization was successful
    m_initFail = 0U;

    //Set I2C bus connection
    m_SpiI2c = 1U;

    cyhal_system_delay_ms(50);

    int16_t prodId = DPS310_readByteBitfield(registers[PROD_ID]);
    if (prodId < 0)
    {
        //Connected device is not a Dps310
        m_initFail = 1U;
        return;
    }
    m_productID = prodId;

    int16_t revId = DPS310_readByteBitfield(registers[REV_ID]);
    if (revId < 0)
    {
        m_initFail = 1U;
        return;
    }
    m_revisionID = revId;

    //find out which temperature sensor is calibrated with coefficients...
    int16_t sensor = DPS310_readByteBitfield(registers[TEMP_SENSORREC]);
    if (sensor < 0)
    {
        m_initFail = 1U;
        return;
    }

    //...and use this sensor for temperature measurement
    m_tempSensor = sensor;
    if (DPS310_writeByteBitfield_reg((uint8_t)sensor, registers[TEMP_SENSOR]) < 0)
    {
        m_initFail = 1U;
        return;
    }

    //read coefficients
    if (DPS310_readcoeffs() < 0)
    {
        m_initFail = 1U;
        return;
    }

    //set to standby for further configuration
    DPS310_standby();

    //set measurement precision and rate to standard values;
    DPS310_configTemp(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);
    DPS310_configPressure(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);

    //perform a first temperature measurement
    //the most recent temperature will be saved internally
    //and used for compensation when calculating pressure
    float trash;
    DPS310_measureTempOnce(&trash);

    //make sure the DPS310 is in standby after initialization
    DPS310_standby();

    // Fix IC with a fuse bit problem, which lead to a wrong temperature
    // Should not affect ICs without this problem
    DPS310_correctTemp();

}

int16_t DPS310_writeByteBitfield_reg(uint8_t data, RegMask_t regMask)
{
    return DPS310_writeByteBitfield(data, regMask.regAddress, regMask.mask, regMask.shift, 0U);
}

int16_t DPS310_writeByteBitfield(uint8_t data,
                                    uint8_t regAddress,
                                    uint8_t mask,
                                    uint8_t shift,
                                    uint8_t check)
{
    int16_t old = DPS310_readByte(regAddress);
    if (old < 0)
    {
        //fail while reading
        return old;
    }
    return DPS310_writeByte(regAddress, ((uint8_t)old & ~mask) | ((data << shift) & mask), check);
}

int16_t DPS310_writeByte(uint8_t regAddress, uint8_t data, uint8_t check)
{
    cy_rslt_t result;
    result = cyhal_i2c_master_mem_write(i2c_ptr, DPS310_I2C_SLAVE_ADDRESS, regAddress, 0x01, &data, 1, 200);
    if(result == CY_RSLT_SUCCESS)
    {
        if (check == 0)
            return DPS_SUCCEEDED;					  //no checking
        else
        {
            if (DPS310_readByte(regAddress) == data) //check if desired by calling function
            {
                return DPS_SUCCEEDED;
            }
            else
            {
                return DPS_FAILED;
            }
        }
    }
}

int16_t DPS310_readByteBitfield(RegMask_t regMask)
{
    int16_t ret = DPS310_readByte(regMask.regAddress);
    if (ret < 0)
    {
        return ret;
    }
    return (((uint8_t)ret) & regMask.mask) >> regMask.shift;
}

int16_t DPS310_readByte(uint8_t regAddress)
{
    cy_rslt_t result;
    uint8_t value;
    result = cyhal_i2c_master_mem_read(i2c_ptr, DPS310_I2C_SLAVE_ADDRESS, regAddress, 1, &value, 1, 200);
    if(result == CY_RSLT_SUCCESS)
    {
        return value;
    }
    else
        return DPS_FAILED;
}

int16_t DPS310_readcoeffs(void)
{
    // TODO: remove magic number
    uint8_t buffer[18];
    //read COEF registers to buffer
    int16_t ret = DPS310_readBlock(coeffBlock, buffer);

    //compose coefficients from buffer content
    m_c0Half = ((uint32_t)buffer[0] << 4) | (((uint32_t)buffer[1] >> 4) & 0x0F);
    DPS310_getTwosComplement(&m_c0Half, 12);
    //c0 is only used as c0*0.5, so c0_half is calculated immediately
    m_c0Half = m_c0Half / 2U;

    //now do the same thing for all other coefficients
    m_c1 = (((uint32_t)buffer[1] & 0x0F) << 8) | (uint32_t)buffer[2];
    DPS310_getTwosComplement(&m_c1, 12);
    m_c00 = ((uint32_t)buffer[3] << 12) | ((uint32_t)buffer[4] << 4) | (((uint32_t)buffer[5] >> 4) & 0x0F);
    DPS310_getTwosComplement(&m_c00, 20);
    m_c10 = (((uint32_t)buffer[5] & 0x0F) << 16) | ((uint32_t)buffer[6] << 8) | (uint32_t)buffer[7];
    DPS310_getTwosComplement(&m_c10, 20);

    m_c01 = ((uint32_t)buffer[8] << 8) | (uint32_t)buffer[9];
    DPS310_getTwosComplement(&m_c01, 16);

    m_c11 = ((uint32_t)buffer[10] << 8) | (uint32_t)buffer[11];
    DPS310_getTwosComplement(&m_c11, 16);
    m_c20 = ((uint32_t)buffer[12] << 8) | (uint32_t)buffer[13];
    DPS310_getTwosComplement(&m_c20, 16);
    m_c21 = ((uint32_t)buffer[14] << 8) | (uint32_t)buffer[15];
    DPS310_getTwosComplement(&m_c21, 16);
    m_c30 = ((uint32_t)buffer[16] << 8) | (uint32_t)buffer[17];
    DPS310_getTwosComplement(&m_c30, 16);
    return DPS_SUCCEEDED;
}

int16_t DPS310_readBlock(RegBlock_t regBlock, uint8_t *buffer)
{
    //do not read if there is no buffer
    if (buffer == NULL)
    {
        return 0; //0 bytes read successfully
    }

    cy_rslt_t result;
    result = cyhal_i2c_master_mem_read(i2c_ptr, DPS310_I2C_SLAVE_ADDRESS, regBlock.regAddress, 1, buffer, regBlock.length, 200);
    if(result == CY_RSLT_SUCCESS)
    {
        return regBlock.length;
    }
    else
        return DPS_FAILED;
}

void DPS310_getTwosComplement(int32_t *raw, uint8_t length)
{
    if (*raw & ((uint32_t)1 << (length - 1)))
    {
        *raw -= (uint32_t)1 << length;
    }
}

int16_t DPS310_standby(void)
{
    //abort if initialization failed
    if (m_initFail)
    {
        return DPS_FAILED;
    }
    //set device to idling mode
    int16_t ret = DPS310_setOpMode(IDLE);
    if (ret != DPS_SUCCEEDED)
    {
        return ret;
    }
    ret = DPS310_disableFIFO();
    return ret;
}

int16_t DPS310_setOpMode(uint8_t opMode)
{
    if (DPS310_writeByteBitfield_reg(opMode, config_registers[MSR_CTRL]) == -1)
    {
        return DPS_FAILED;
    }
    m_opMode = (enum Mode)opMode;
    return DPS_SUCCEEDED;
}

int16_t DPS310_disableFIFO()
{
    int16_t ret = DPS310_flushFIFO();
    ret = DPS310_writeByteBitfield_reg(0U, config_registers[FIFO_EN]);
    return ret;
}

int16_t DPS310_flushFIFO()
{
    return DPS310_writeByteBitfield_reg(1U, registers[FIFO_FL]);
}

int16_t DPS310_configTemp(uint8_t tempMr, uint8_t tempOsr)
{
    tempMr &= 0x07;
    tempOsr &= 0x07;
    // two accesses to the same register; for readability
    int16_t ret = DPS310_writeByteBitfield_reg(tempMr, config_registers[TEMP_MR]);
    ret = DPS310_writeByteBitfield_reg(tempOsr, config_registers[TEMP_OSR]);

    //abort immediately on fail
    if (ret != DPS_SUCCEEDED)
    {
        return DPS_FAILED;
    }
    m_tempMr = tempMr;
    m_tempOsr = tempOsr;

    DPS310_writeByteBitfield_reg(m_tempSensor, registers[TEMP_SENSOR]);
    //set TEMP SHIFT ENABLE if oversampling rate higher than eight(2^3)
    if (tempOsr > DPS310__OSR_SE)
    {
        ret = DPS310_writeByteBitfield_reg(1U, registers[TEMP_SE]);
    }
    else
    {
        ret = DPS310_writeByteBitfield_reg(0U, registers[TEMP_SE]);
    }
    return ret;
}

int16_t DPS310_configPressure(uint8_t prsMr, uint8_t prsOsr)
{
    prsMr &= 0x07;
    prsOsr &= 0x07;
    int16_t ret = DPS310_writeByteBitfield_reg(prsMr, config_registers[PRS_MR]);
    ret = DPS310_writeByteBitfield_reg(prsOsr, config_registers[PRS_OSR]);

    //abort immediately on fail
    if (ret != DPS_SUCCEEDED)
    {
        return DPS_FAILED;
    }
    m_prsMr = prsMr;
    m_prsOsr = prsOsr;
    //set PM SHIFT ENABLE if oversampling rate higher than eight(2^3)
    if (prsOsr > DPS310__OSR_SE)
    {
        ret = DPS310_writeByteBitfield_reg(1U, registers[PRS_SE]);
    }
    else
    {
        ret = DPS310_writeByteBitfield_reg(0U, registers[PRS_SE]);
    }
    return ret;
}

int16_t DPS310_measureTempOnce(float* result)
{
    return DPS310_measureTempOnce_oversample(result, m_tempOsr);
}

int16_t DPS310_measureTempOnce_oversample(float* result, uint8_t oversamplingRate)
{
    //Start measurement
    int16_t ret = DPS310_startMeasureTempOnce(oversamplingRate);
    if (ret != DPS_SUCCEEDED)
    {
        return ret;
    }

    //wait until measurement is finished
    uint16_t busy_time = DPS310_calcBusyTime(0U, m_tempOsr);
    cyhal_system_delay_ms(busy_time / DPS__BUSYTIME_SCALING);
    cyhal_system_delay_ms(DPS310__BUSYTIME_FAILSAFE);

    ret = DPS310_getSingleResult(result);
    if (ret != DPS_SUCCEEDED)
    {
        DPS310_standby();
    }
    return ret;
}

int16_t DPS310_startMeasureTempOnce_void(void)
{
    return DPS310_startMeasureTempOnce(m_tempOsr);
}

int16_t DPS310_startMeasureTempOnce(uint8_t oversamplingRate)
{
    //abort if initialization failed
    if (m_initFail)
    {
        return DPS_FAILED;
    }
    //abort if device is not in idling mode
    if (m_opMode != IDLE)
    {
        return DPS_FAILED;
    }

    if (oversamplingRate != m_tempOsr)
    {
        //configuration of oversampling rate
        if (DPS310_configTemp(0U, oversamplingRate) != DPS_SUCCEEDED)
        {
            return DPS_FAILED;
        }
    }

    //set device to temperature measuring mode
    return DPS310_setOpMode(CMD_TEMP);
}

uint16_t DPS310_calcBusyTime(uint16_t mr, uint16_t osr)
{
    //formula from datasheet (optimized)
    return ((uint32_t)20U << mr) + ((uint32_t)16U << (osr + mr));
}

int16_t DPS310_correctTemp(void)
{
    if (m_initFail)
    {
        return DPS_FAILED;
    }
    DPS310_writeByte(0x0E, 0xA5, 0);
    DPS310_writeByte(0x0F, 0x96, 0);
    DPS310_writeByte(0x62, 0x02, 0);
    DPS310_writeByte(0x0E, 0x00, 0);
    DPS310_writeByte(0x0F, 0x00, 0);

    //perform a first temperature measurement (again)
    //the most recent temperature will be saved internally
    //and used for compensation when calculating pressure
    float trash;
    DPS310_measureTempOnce(&trash);

    return DPS_SUCCEEDED;
}

int16_t DPS310_getSingleResult(float* result)
{
    //abort if initialization failed
    if (m_initFail)
    {
        return DPS_FAILED;
    }

    //read finished bit for current opMode
    int16_t rdy;
    switch (m_opMode)
    {
    case CMD_TEMP: //temperature
        rdy = DPS310_readByteBitfield(config_registers[TEMP_RDY]);
        break;
    case CMD_PRS: //pressure
        rdy = DPS310_readByteBitfield(config_registers[PRS_RDY]);
        break;
    default: //DPS310 not in command mode
        return DPS_FAILED;
    }
    //read new measurement result
    switch (rdy)
    {
        case DPS_FAILED: //could not read ready flag
            return DPS_FAILED;
        case 1: //measurement ready, expected case
        {
            enum Mode oldMode = m_opMode;
            m_opMode = IDLE; //opcode was automatically reseted by DPS310
            int32_t raw_val = 0;
            switch (oldMode)
            {
            case CMD_TEMP: //temperature
                if(DPS310_getRawResult(&raw_val, registerBlocks[TEMP]) != DPS_SUCCEEDED)
                {
                    printf("Failed to read data\r\n");
                    return DPS_FAILED;
                }
                *result = DPS310_calcTemp(raw_val);
                return DPS_SUCCEEDED; // TODO
            case CMD_PRS:			   //pressure
                DPS310_getRawResult(&raw_val, registerBlocks[PRS]);
                *result = DPS310_calcPressure(raw_val);
                return DPS_SUCCEEDED; // TODO
            default:
                return DPS_FAILED; //should already be filtered above
            }
        }
    }
    return DPS_FAILED;
}

int16_t DPS310_getRawResult(int32_t *raw, RegBlock_t reg)
{
    uint8_t buffer[DPS__RESULT_BLOCK_LENGTH] = {0};
    if (DPS310_readBlock(reg, buffer) != DPS__RESULT_BLOCK_LENGTH)
        return DPS_FAILED;

    *raw = (uint32_t)buffer[0] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[2];
    DPS310_getTwosComplement(raw, 24);
    return DPS_SUCCEEDED;
}

float DPS310_calcTemp(int32_t raw)
{
    float temp = raw;

    //scale temperature according to scaling table and oversampling
    temp /= DPS310_scaling_facts[m_tempOsr];

    //update last measured temperature
    //it will be used for pressure compensation
    m_lastTempScal = temp;

    //Calculate compensated temperature
    temp = m_c0Half + m_c1 * temp;

    return temp;
}

float DPS310_calcPressure(int32_t raw)
{
    float prs = raw;

    //scale pressure according to scaling table and oversampling
    prs /= DPS310_scaling_facts[m_prsOsr];

    //Calculate compensated pressure
    prs = m_c00 + prs * (m_c10 + prs * (m_c20 + prs * m_c30)) + m_lastTempScal * (m_c01 + prs * (m_c11 + prs * m_c21));

    //return pressure
    return prs;
}

int16_t DPS310_measurePressureOnce(float* result)
{
    return DPS310_measurePressureOnce_oversample(result, m_prsOsr);
}

int16_t DPS310_measurePressureOnce_oversample(float* result, uint8_t oversamplingRate)
{
    //start the measurement
    int16_t ret = DPS310_startMeasurePressureOnce(oversamplingRate);
    if (ret != DPS_SUCCEEDED)
    {
        return ret;
    }

    //wait until measurement is finished
    uint16_t busy_time = DPS310_calcBusyTime(0U, m_prsOsr);
    cyhal_system_delay_ms(busy_time / DPS__BUSYTIME_SCALING);
    cyhal_system_delay_ms(DPS310__BUSYTIME_FAILSAFE);

    ret = DPS310_getSingleResult(result);
    if (ret != DPS_SUCCEEDED)
    {
        DPS310_standby();
    }
    return ret;
}

int16_t DPS310_startMeasurePressureOnce_void(void)
{
    return DPS310_startMeasurePressureOnce(m_prsOsr);
}

int16_t DPS310_startMeasurePressureOnce(uint8_t oversamplingRate)
{
    //abort if initialization failed
    if (m_initFail)
    {
        return DPS_FAILED;
    }
    //abort if device is not in idling mode
    if (m_opMode != IDLE)
    {
        return DPS_FAILED;
    }
    //configuration of oversampling rate, lowest measure rate to avoid conflicts
    if (oversamplingRate != m_prsOsr)
    {
        if (DPS310_configPressure(0U, oversamplingRate))
        {
            return DPS_FAILED;
        }
    }
    //set device to pressure measuring mode
    return DPS310_setOpMode(CMD_PRS);
}
