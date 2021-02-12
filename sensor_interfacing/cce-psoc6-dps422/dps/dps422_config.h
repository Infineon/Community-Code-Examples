/******************************************************************************
* File Name:  dps422_config.h
*
* Description:  This file contains the macros for temperature and pressure
*               calculation constants. It also contains the structures and
*               enums for sensor configuration registers.
*
*******************************************************************************/

#ifndef DPS_DPS422_CONFIG_H_
#define DPS_DPS422_CONFIG_H_

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define DPS_SUCCESS					 0
#define DPS_FAIL					 -1

/* Constants for temperature calculation */
#define DPS422_T_REF 				 27
#define DPS422_V_BE_TARGET 			 0.687027
#define DPS422_ALPHA 				 9.45
#define DPS422_T_C_VBE 				 -1.735e-3
#define DPS422_K_PTAT_CORNER		 -0.8
#define DPS422_K_PTAT_CURVATURE		  0.039
#define DPS422_A_0					  5030

#define DPS422_NUM_OF_REGMASKS       20

#define DPS422__PROD_ID              0x0A

#define DPS422__OSR_SE               3U

/* DPS422 has 10 milliseconds of spare time for each synchronous measurement / per second for asynchronous measurements
 * this is for error prevention on friday-afternoon-products :D
 * you can set it to 0 if you dare, but there is no warranty that it will still work
 */
#define DPS_422__BUSYTIME_FAILSAFE 	 10U
#define DPS310__MAX_BUSYTIME 		 ((1000U - DPS_422__BUSYTIME_FAILSAFE) * DPS__BUSYTIME_SCALING)

#define DPS__FIFO_SIZE               32
#define DPS__STD_SLAVE_ADDRESS       0x77U
#define DPS__RESULT_BLOCK_LENGTH 	 3
#define NUM_OF_COMMON_REGMASKS		 16

#define DPS__MEASUREMENT_RATE_1		 0
#define DPS__MEASUREMENT_RATE_2		 1
#define DPS__MEASUREMENT_RATE_4		 2
#define DPS__MEASUREMENT_RATE_8		 3
#define DPS__MEASUREMENT_RATE_16 	 4
#define DPS__MEASUREMENT_RATE_32	 5
#define DPS__MEASUREMENT_RATE_64	 6
#define DPS__MEASUREMENT_RATE_128	 7

#define DPS__OVERSAMPLING_RATE_1 	DPS__MEASUREMENT_RATE_1
#define DPS__OVERSAMPLING_RATE_2 	DPS__MEASUREMENT_RATE_2
#define DPS__OVERSAMPLING_RATE_4 	DPS__MEASUREMENT_RATE_4
#define DPS__OVERSAMPLING_RATE_8 	DPS__MEASUREMENT_RATE_8
#define DPS__OVERSAMPLING_RATE_16 	DPS__MEASUREMENT_RATE_16
#define DPS__OVERSAMPLING_RATE_32 	DPS__MEASUREMENT_RATE_32
#define DPS__OVERSAMPLING_RATE_64 	DPS__MEASUREMENT_RATE_64
#define DPS__OVERSAMPLING_RATE_128 	DPS__MEASUREMENT_RATE_128

/* We use 0.1 ms units for time calculations, so 10 units are one millisecond */
#define DPS__BUSYTIME_SCALING 		10U

#define DPS__NUM_OF_SCAL_FACTS		 8

/*******************************************************************************
 * Structure
 *******************************************************************************/
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

/*******************************************************************************
 * Enum
 *******************************************************************************/
enum Interrupt_source_420_e
{
    DPS422_NO_INTR = 0,
    DPS422_PRS_INTR = 1,
    DPS422_TEMP_INTR = 2,
    DPS422_BOTH_INTR = 3,
    DPS422_FIFO_WM_INTR = 4,
    DPS422_FIFO_FULL_INTR = 8,
};

enum Mode
{
    IDLE = 0x00,
    CMD_PRS = 0x01,
    CMD_TEMP = 0x02,
    CMD_BOTH = 0x03,
    CONT_PRS = 0x05,
    CONT_TMP = 0x06,
    CONT_BOTH = 0x07
};

enum Registers_e
{
    /* flags */
    CONT_FLAG = 0, 		/* continuous mode flag */
    INIT_DONE,     		/* set when initialisation procedure is complete */
    /* interrupt config */
    INTR_SEL, 			/* interrupt select */
    INTR_POL, 			/* interrupt active polarity */
    /* fifo config */
    WM,              	/* watermark level */
    FIFO_FL,         	/* FIFO flush */
    FIFO_EMPTY,      	/* FIFO empty */
    FIFO_FULL,       	/* if FIFO is full or reaches watermark level */
    FIFO_FULL_CONF,  	/* Configures FIFO behaviour when full */
    FIFO_FILL_LEVEL, 	/* contains the number of pressure and/or temperature measurements currently stored in FIFO */
    /* misc */
    PROD_ID,
    REV_ID,
    SPI_MODE, 			/* 4- or 3-wire SPI */
    SOFT_RESET,
    MUST_SET, 			/* bit 7 of TEMP_CFG, according to datasheet should always be set */
};

enum Config_Registers_e
{
    TEMP_MR = 0, 		/* temperature measure rate */
    TEMP_OSR,    		/* temperature measurement resolution */
    PRS_MR,      		/* pressure measure rate */
    PRS_OSR,     		/* pressure measurement resolution */
    MSR_CTRL,    		/* measurement control */
    FIFO_EN,

    TEMP_RDY,
    PRS_RDY,
    INT_FLAG_FIFO,
    INT_FLAG_TEMP,
    INT_FLAG_PRS,
};

enum RegisterBlocks_e
{
    TEMP,               /* compensation coefficients */
    PRS,
};

extern const RegMask_t config_registers[NUM_OF_COMMON_REGMASKS];
extern const RegMask_t registers[DPS422_NUM_OF_REGMASKS];
extern const RegBlock_t coeffBlocks[4];
extern const RegBlock_t registerBlocks[2];

#endif /* DPS_DPS422_CONFIG_H_ */
