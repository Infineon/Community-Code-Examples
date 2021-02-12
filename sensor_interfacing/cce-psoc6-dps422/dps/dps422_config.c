/******************************************************************************
* File Name:  dps422_config.c
*
* Description:  This file contains constant structures for sensor registers.
*
*******************************************************************************/

#include "dps422_config.h"

const RegMask_t registers[DPS422_NUM_OF_REGMASKS] = {
    // flags
    {0x08, 0x40, 6}, 		// CONT_FLAG
    {0x08, 0x80, 7}, 		// INIT_DONE
    // interrupt config
    {0x09, 0xF0, 4}, 		// INTR_SEL
    {0x09, 0x80, 3}, 		// INTR_POL
    // /fifo config
    {0x0B, 0x1F, 0}, 		// WM
    {0x0D, 0x80, 7}, 		// FIFO_FL
    {0x0C, 0x01, 0}, 		// FIFO_EMPTY
    {0x0C, 0x02, 1}, 		// FIFO_FULL
    {0x09, 0x04, 2}, 		// FIFO_FULL_CONF
    {0x0C, 0xFC, 2}, 		// FIFO_FILL_LEVEL
    // misc
    {0x1D, 0x0F, 0}, 		// PROD_ID
    {0x1D, 0xF0, 0}, 		// REV_ID
    {0x09, 0x01, 0}, 		// SPI_MODE
    {0x0D, 0x0F, 0}, 		// SOFT_RESET
    {0x07, 0x80, 7}, 		// MUST_SET
};

const RegMask_t config_registers[NUM_OF_COMMON_REGMASKS] = {
    {0x07, 0x70, 4}, 		// TEMP_MR
    {0x07, 0x07, 0}, 		// TEMP_OSR
    {0x06, 0x70, 4}, 		// PRS_MR
    {0x06, 0x07, 0}, 		// PRS_OSR
    {0x08, 0x07, 0}, 		// MSR_CTRL
    {0x09, 0x02, 1}, 		// FIFO_EN

    {0x08, 0x20, 5}, 		// TEMP_RDY
    {0x08, 0x10, 4}, 		// PRS_RDY
    {0x0A, 0x04, 2}, 		// INT_FLAG_FIFO
    {0x0A, 0x02, 1}, 		// INT_FLAG_TEMP
    {0x0A, 0x01, 0}, 		// INT_FLAG_PRS
};

const RegBlock_t coeffBlocks[4] = {
    {0x20, 3},
    {0x26, 20},
};

const RegBlock_t registerBlocks[2] = {
    {0x03, 3},
    {0x00, 3},
};

