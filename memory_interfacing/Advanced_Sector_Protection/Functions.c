/******************************************************************************
* File Name:   Functions.c
*
* Description: This file contains the definitions for the custom functions
*              used in this project.
*******************************************************************************/

#include "Functions.h"

/*******************************************************************************
* Global Variable
********************************************************************************/
extern cyhal_qspi_t qspi_object;
const uint8_t password[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


cy_rslt_t Read_ID(uint8_t *rx_data)
{
    cy_rslt_t result;
    size_t length = 3;

    /* Defining QSPI command structure for READ ID command */
    cyhal_qspi_command_t read_id_command;
    read_id_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;            /* Bus width for the instruction */
    read_id_command.instruction.value = RDID;                                     /* Instruction value */
    read_id_command.instruction.disabled = FALSE;                                 /* Instruction phase skipped if disabled is set to true */
    read_id_command.address.disabled = TRUE;                                      /* Address phase skipped if disabled is set to true */
    read_id_command.mode_bits.disabled = TRUE;                                    /* Mode bits phase skipped if disabled is set to true */
    read_id_command.dummy_count = 0;                                              /* Dummy cycles count */
    read_id_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                   /* Bus width for data */

    /* HAL API for read operation */
    result = cyhal_qspi_read(&qspi_object, &read_id_command, rx_data, &length);

    return result;
}

cy_rslt_t Read_Status_Register1(uint8_t *rx_data)
{
    cy_rslt_t result;
    size_t length = SR1_LENGTH;

    /* Defining QSPI command structure for RDSR1 command */
    cyhal_qspi_command_t rdsr1_command;

    rdsr1_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;              /* Bus width for the instruction */
    rdsr1_command.instruction.value = RDSR1;                                      /* Instruction value */
    rdsr1_command.instruction.disabled = FALSE;                                   /* Instruction phase skipped if disabled is set to true */
    rdsr1_command.address.disabled = TRUE;                                        /* Address phase skipped if disabled is set to true */
    rdsr1_command.mode_bits.disabled = TRUE;                                      /* Mode bits phase skipped if disabled is set to true */
    rdsr1_command.dummy_count = 0;                                                /* Dummy cycles count */
    rdsr1_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                     /* Bus width for data */

    result = cyhal_qspi_read(&qspi_object, &rdsr1_command, rx_data, &length);

    return result;
}

cy_rslt_t Polling_WIP()
{
    cy_rslt_t result;
    uint8_t wip = FLASH_BUSY;
    uint8_t rx_data;
    while(wip)
    {
        /* RDSR1 */
        result = Read_Status_Register1(&rx_data);
        wip = rx_data & FLASH_BUSY;
    }

    return result;
}

cy_rslt_t Clear_Status_Register()
{
    cy_rslt_t result;

    /* Defining QSPI command structure for CLSR command */
    cyhal_qspi_command_t clsr_command;

    clsr_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;               /* Bus width for the instruction */
    clsr_command.instruction.value = CLSR;                                        /* Instruction value */
    clsr_command.instruction.disabled = FALSE;                                    /* Instruction phase skipped if disabled is set to true */
    clsr_command.address.disabled = TRUE;                                         /* Address phase skipped if disabled is set to true */
    clsr_command.mode_bits.disabled = TRUE;                                       /* Mode bits phase skipped if disabled is set to true */
    clsr_command.dummy_count = 0;                                                 /* Dummy cycles count */
    clsr_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                      /* Bus width for data */

    /* HAL API for TX-RX transaction used for sending CLSR command */
    result = cyhal_qspi_transfer(&qspi_object, &clsr_command, NULL, 0, NULL, 0);

    return result;
}

cy_rslt_t Write_Enable()
{
    cy_rslt_t result;

    /* Defining QSPI command structure for WREN command */
    cyhal_qspi_command_t wren_command;

    wren_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                /* Bus width for the instruction */
    wren_command.instruction.value = WREN;                                         /* Instruction value */
    wren_command.instruction.disabled = FALSE;                                     /* Instruction phase skipped if disabled is set to true */
    wren_command.address.disabled = TRUE;                                          /* Address phase skipped if disabled is set to true */
    wren_command.mode_bits.disabled = TRUE;                                        /* Mode bits phase skipped if disabled is set to true */
    wren_command.dummy_count = 0;                                                  /* Dummy cycles count */
    wren_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                       /* Bus width for data */

    /* HAL API for TX-RX transaction used for sending WREN command */
    result = cyhal_qspi_transfer(&qspi_object, &wren_command, NULL, 0, NULL, 0);

    return result;
}

cy_rslt_t Read(uint32_t addr, uint8_t *rx_data)
{
    cy_rslt_t result;
    size_t length = PACKET_SIZE;

    /* Defining QSPI command structure for READ command */
    cyhal_qspi_command_t read_command;

    read_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                /* Bus width for the instruction */
    read_command.instruction.value = READ;                                         /* Instruction value */
    read_command.instruction.disabled = FALSE;                                     /* Instruction phase skipped if disabled is set to true */
    read_command.address.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                    /* Bus width for the address */
    read_command.address.size = CYHAL_QSPI_CFG_SIZE_24;                            /* Address size in bits */
    read_command.address.value = addr;                                             /* Address value */
    read_command.address.disabled = FALSE;                                         /* Address phase skipped if disabled is set to true */
    read_command.mode_bits.disabled = TRUE;                                        /* Mode bits phase skipped if disabled is set to true */
    read_command.dummy_count = 0;                                                  /* Dummy cycles count */
    read_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                       /* Bus width for data */

    /* HAL API for read operation */
    result = cyhal_qspi_read(&qspi_object, &read_command, rx_data, &length);

    return result;
}

cy_rslt_t Sector_Erase(uint32_t addr)
{
    cy_rslt_t result;

    /* Defining QSPI command structure for SE command */
    cyhal_qspi_command_t se_command;

    se_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                  /* Bus width for the instruction */
    se_command.instruction.value = SE;                                             /* Instruction value */
    se_command.instruction.disabled = FALSE;                                       /* Instruction phase skipped if disabled is set to true */
    se_command.address.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                      /* Bus width for the address */
    se_command.address.size = CYHAL_QSPI_CFG_SIZE_24;                              /* Address size in bits */
    se_command.address.value = addr;                                               /* Address value */
    se_command.address.disabled = FALSE;                                           /* Address phase skipped if disabled is set to true */
    se_command.mode_bits.disabled = TRUE;                                          /* Mode bits phase skipped if disabled is set to true */
    se_command.dummy_count = 0;                                                    /* Dummy cycles count */
    se_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                         /* Bus width for data */

    /* HAL API for TX-RX transaction used for sending SE command */
    result = cyhal_qspi_transfer(&qspi_object, &se_command, NULL, 0, NULL, 0);

    return result;
}

cy_rslt_t Page_Program(uint32_t addr, uint8_t *tx_data)
{
    cy_rslt_t result;
    size_t length = PACKET_SIZE;

    /* Defining QSPI command structure for PP command */
    cyhal_qspi_command_t pp_command;

    pp_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                  /* Bus width for the instruction */
    pp_command.instruction.value = PP;                                             /* Instruction value */
    pp_command.instruction.disabled = FALSE;                                       /* Instruction phase skipped if disabled is set to true */
    pp_command.address.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                      /* Bus width for the address */
    pp_command.address.size = CYHAL_QSPI_CFG_SIZE_24;                              /* Address size in bits */
    pp_command.address.value = addr;                                               /* Address value */
    pp_command.address.disabled = FALSE;                                           /* Address phase skipped if disabled is set to true */
    pp_command.mode_bits.disabled = TRUE;                                          /* Mode bits phase skipped if disabled is set to true */
    pp_command.dummy_count = 0;                                                    /* Dummy cycles count */
    pp_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                         /* Bus width for data */

    /* HAL API for write operation */
    result = cyhal_qspi_write(&qspi_object, &pp_command, tx_data, &length);

    return result;
}

cy_rslt_t ASP_Register_Read(uint8_t *asp_reg)
{
    cy_rslt_t result;
    size_t length = ASP_REGISTER_LENGTH;

    /* Defining QSPI command structure for ASP Read command */
    cyhal_qspi_command_t asprd_command;

    asprd_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;               /* Bus width for the instruction */
    asprd_command.instruction.value = ASPRD;                                       /* Instruction value */
    asprd_command.instruction.disabled = FALSE;                                    /* Instruction phase skipped if disabled is set to true */
    asprd_command.address.disabled = TRUE;                                         /* Address phase skipped if disabled is set to true */
    asprd_command.mode_bits.disabled = TRUE;                                       /* Mode bits phase skipped if disabled is set to true */
    asprd_command.dummy_count = 0;                                                 /* Dummy cycles count */
    asprd_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;                      /* Bus width for data */

    /* HAL API for read operation */
    result = cyhal_qspi_read(&qspi_object, &asprd_command, asp_reg, &length);

    return result;
}

cy_rslt_t ASP_Program(uint8_t *asp_reg)
{
    cy_rslt_t result;
    size_t length = ASP_REGISTER_LENGTH;

    /* Defining QSPI command structure for ASP Program command */
    cyhal_qspi_command_t aspp_command;

    aspp_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    aspp_command.instruction.value = ASPP;
    aspp_command.instruction.disabled = FALSE;
    aspp_command.address.disabled = TRUE;
    aspp_command.mode_bits.disabled = TRUE;
    aspp_command.dummy_count = 0;
    aspp_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;

    /* HAL API for write operation */
    result = cyhal_qspi_write(&qspi_object, &aspp_command, asp_reg, &length);

    return result;

}

cy_rslt_t Password_Program()
{
    cy_rslt_t result;
    size_t length = PASSWORD_LENGTH;

    /* Defining QSPI command structure for Password Program command */
    cyhal_qspi_command_t passp_command;

    passp_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    passp_command.instruction.value = PASSP;
    passp_command.instruction.disabled = FALSE;
    passp_command.address.disabled = TRUE;
    passp_command.mode_bits.disabled = TRUE;
    passp_command.dummy_count = 0;
    passp_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;

    /* HAL API for write operation */
    result = cyhal_qspi_write(&qspi_object, &passp_command, password, &length);

    return result;
}

cy_rslt_t Password_Read(uint8_t *rx_data)
{
    cy_rslt_t result;
    size_t length = PASSWORD_LENGTH;

    /* Defining QSPI command structure for Password Read command */
    cyhal_qspi_command_t passrd_command;

    passrd_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    passrd_command.instruction.value = PASSRD;
    passrd_command.instruction.disabled = FALSE;
    passrd_command.address.disabled = TRUE;
    passrd_command.mode_bits.disabled = TRUE;
    passrd_command.dummy_count = 0;
    passrd_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;

    /* HAL API for read operation */
    result = cyhal_qspi_read(&qspi_object, &passrd_command, rx_data, &length);

    return result;
}

cy_rslt_t Password_Unlock()
{
    cy_rslt_t result;
    size_t length = PASSWORD_LENGTH;

    /* Defining QSPI command structure for Password Unlock command */
    cyhal_qspi_command_t passu_command;

    passu_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    passu_command.instruction.value = PASSU;
    passu_command.instruction.disabled = FALSE;
    passu_command.address.disabled = TRUE;
    passu_command.mode_bits.disabled = TRUE;
    passu_command.dummy_count = 0;
    passu_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;

    /* HAL API for write operation */
    result = cyhal_qspi_write(&qspi_object, &passu_command, password, &length);

    return result;
}

cy_rslt_t PPB_Read(uint32_t addr, uint8_t *rx_data)
{
    cy_rslt_t result;
    size_t length = PPB_LENGTH;

    /* Defining QSPI command structure for PPB Read command */
    cyhal_qspi_command_t ppbrd_command;

    ppbrd_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    ppbrd_command.instruction.value = PPBRD;
    ppbrd_command.instruction.disabled = FALSE;
    ppbrd_command.address.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    ppbrd_command.address.size = CYHAL_QSPI_CFG_SIZE_32;
    ppbrd_command.address.value = addr;
    ppbrd_command.address.disabled = FALSE;
    ppbrd_command.mode_bits.disabled = TRUE;
    ppbrd_command.dummy_count = 0;
    ppbrd_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;

    /* HAL API for read operation */
    result = cyhal_qspi_read(&qspi_object, &ppbrd_command, rx_data, &length);

    return result;
}

cy_rslt_t PPB_Program(uint32_t addr)
{
    cy_rslt_t result;

    /* Defining QSPI command structure for PPB Program command */
    cyhal_qspi_command_t ppbp_command;

    ppbp_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    ppbp_command.instruction.value = PPBP;
    ppbp_command.instruction.disabled = FALSE;
    ppbp_command.address.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    ppbp_command.address.size = CYHAL_QSPI_CFG_SIZE_32;
    ppbp_command.address.value = addr;
    ppbp_command.address.disabled = FALSE;
    ppbp_command.mode_bits.disabled = TRUE;
    ppbp_command.dummy_count = 0;
    ppbp_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;

    /* HAL API for TX-RX transaction used for sending PPBP command */
    result = cyhal_qspi_transfer(&qspi_object, &ppbp_command, NULL, 0, NULL, 0);

    return result;
}

cy_rslt_t PPB_Erase()
{
    cy_rslt_t result;

    /* Defining QSPI command structure for PPB Erase command */
    cyhal_qspi_command_t ppbe_command;

    ppbe_command.instruction.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;
    ppbe_command.instruction.value = PPBE;
    ppbe_command.instruction.disabled = FALSE;
    ppbe_command.address.disabled = TRUE;
    ppbe_command.mode_bits.disabled = TRUE;
    ppbe_command.dummy_count = 0;
    ppbe_command.data.bus_width = CYHAL_QSPI_CFG_BUS_SINGLE;

    /* HAL API for TX-RX transaction used for sending PPBP command */
    result = cyhal_qspi_transfer(&qspi_object, &ppbe_command, NULL, 0, NULL, 0);

    return result;
}
