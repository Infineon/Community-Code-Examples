/******************************************************************************
* File Name: dps310_config.h
*
* Description: This file is a port of the DSP-310 Pressure Sensor library.
*
* Related Document: See https://github.com/Infineon/DPS310-Pressure-Sensor
*
*******************************************************************************/

#ifndef DPS310_CONFIG_H_
#define DPS310_CONFIG_H_

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

#define DPS_SUCCEEDED 0
#define DPS_FAILED -1

#define DPS310_NUM_OF_REGMASKS 16

/* The sensor's address is 0x77 (default) or 0x76 (if the SDO pin is pulled-down to GND) */
#define DPS310_I2C_SLAVE_ADDRESS 0x77U

#define NUM_OF_COMMON_REGMASKS 16

#define DPS310__PROD_ID 0x00
#define DPS310__SPI_WRITE_CMD 0x00U
#define DPS310__SPI_READ_CMD 0x80U
#define DPS310__SPI_RW_MASK 0x80U
#define DPS310__SPI_MAX_FREQ 1000000U

#define DPS310__OSR_SE 3U

// DPS310 has 10 milliseconds of spare time for each synchronous measurement / per second for asynchronous measurements
// this is for error prevention on friday-afternoon-products :D
// you can set it to 0 if you dare, but there is no warranty that it will still work
#define DPS310__BUSYTIME_FAILSAFE 10U
#define DPS310__MAX_BUSYTIME ((1000U - DPS310__BUSYTIME_FAILSAFE) * DPS__BUSYTIME_SCALING)

#define DPS310__REG_ADR_SPI3W 0x09U
#define DPS310__REG_CONTENT_SPI3W 0x01U

// slave address same for 422 and 310 (to be proved for future sensors)
#define DPS__FIFO_SIZE 32
#define DPS__STD_SLAVE_ADDRESS 0x77U
#define DPS__RESULT_BLOCK_LENGTH 3
#define NUM_OF_COMMON_REGMASKS 16

#define DPS__MEASUREMENT_RATE_1 0
#define DPS__MEASUREMENT_RATE_2 1
#define DPS__MEASUREMENT_RATE_4 2
#define DPS__MEASUREMENT_RATE_8 3
#define DPS__MEASUREMENT_RATE_16 4
#define DPS__MEASUREMENT_RATE_32 5
#define DPS__MEASUREMENT_RATE_64 6
#define DPS__MEASUREMENT_RATE_128 7

#define DPS__OVERSAMPLING_RATE_1 DPS__MEASUREMENT_RATE_1
#define DPS__OVERSAMPLING_RATE_2 DPS__MEASUREMENT_RATE_2
#define DPS__OVERSAMPLING_RATE_4 DPS__MEASUREMENT_RATE_4
#define DPS__OVERSAMPLING_RATE_8 DPS__MEASUREMENT_RATE_8
#define DPS__OVERSAMPLING_RATE_16 DPS__MEASUREMENT_RATE_16
#define DPS__OVERSAMPLING_RATE_32 DPS__MEASUREMENT_RATE_32
#define DPS__OVERSAMPLING_RATE_64 DPS__MEASUREMENT_RATE_64
#define DPS__OVERSAMPLING_RATE_128 DPS__MEASUREMENT_RATE_128

//we use 0.1 ms units for time calculations, so 10 units are one millisecond
#define DPS__BUSYTIME_SCALING 10U

#define DPS__NUM_OF_SCAL_FACTS 8

typedef struct
{
    uint8_t regAddress;
    uint8_t mask;
    uint8_t shift;
} RegMask_t;

typedef struct
{
    uint8_t regAddress;
    uint8_t length;
} RegBlock_t;

enum Interrupt_source_310_e
{
    DPS310_NO_INTR = 0,
    DPS310_PRS_INTR = 1,
    DPS310_TEMP_INTR = 2,
    DPS310_BOTH_INTR = 3,
    DPS310_FIFO_FULL_INTR = 4,
};

enum Mode
{
    IDLE = 0x00,
    CMD_PRS = 0x01,
    CMD_TEMP = 0x02,
    CMD_BOTH = 0x03, // only for DPS422
    CONT_PRS = 0x05,
    CONT_TMP = 0x06,
    CONT_BOTH = 0x07
};

enum Registers_e
{
    PROD_ID = 0,
    REV_ID,
    TEMP_SENSOR,    // internal vs external
    TEMP_SENSORREC, //temperature sensor recommendation
    TEMP_SE,        //temperature shift enable (if temp_osr>3)
    PRS_SE,         //pressure shift enable (if prs_osr>3)
    FIFO_FL,        //FIFO flush
    FIFO_EMPTY,     //FIFO empty
    FIFO_FULL,      //FIFO full
    INT_HL,
    INT_SEL,         //interrupt select
};

enum Config_Registers_e
{
    TEMP_MR = 0, // temperature measure rate
    TEMP_OSR,    // temperature measurement resolution
    PRS_MR,      // pressure measure rate
    PRS_OSR,     // pressure measurement resolution
    MSR_CTRL,    // measurement control
    FIFO_EN,

    TEMP_RDY,
    PRS_RDY,
    INT_FLAG_FIFO,
    INT_FLAG_TEMP,
    INT_FLAG_PRS,
};

enum RegisterBlocks_e
{
    PRS = 0, // pressure value
    TEMP,    // temperature value
};

extern const RegBlock_t registerBlocks[2];
extern const RegMask_t registers[DPS310_NUM_OF_REGMASKS];
extern const RegBlock_t coeffBlock;
extern const RegMask_t config_registers[NUM_OF_COMMON_REGMASKS];


#endif /* DPS310_CONFIG_H_ */
