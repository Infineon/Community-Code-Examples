/******************************************************************************
* File Name:   main.c
*
* Description: This project demonstrates Advanced Sector Protection technique 
               in S25FL512S NOR Flash. It shows the behavior of flash in 
               Password Protection Mode.
*
* Related Document: See Readme.md
*
*
*******************************************************************************
* (c) 2019, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/


#include "Functions.h"

/*******************************************************************************
* Global Variable
********************************************************************************/
cyhal_qspi_t qspi_object;


int main(void)
{
    cy_rslt_t result;
    uint8_t rx_data[PACKET_SIZE];
    uint8_t tx_data[PACKET_SIZE];
    uint32_t addr_3B = 0x0C0000;
    uint8_t asp_reg[2];

    uint8_t i;

    /* Initializing the tx buffer */
    for(i=0;i<PACKET_SIZE;i++)
    {
        tx_data[i] = i;
    }

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    if(result != SUCCESS)
        printf("\n\rUART Initialized Failed\n\r");
    else if(result == SUCCESS)
        printf("\n\rUART Initialized\n\r");

    printf("\n\r*******************************START*******************************\n\r");

    /* Initialize QSPI peripheral */
    result = cyhal_qspi_init(&qspi_object, CYBSP_QSPI_D0, CYBSP_QSPI_D1, CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
                CYBSP_QSPI_SCK, CYBSP_QSPI_SS, QSPI_BUS_FREQUENCY_HZ, NULL);

    if(result != SUCCESS)
        printf("\n\rSMIF Initialized Failed\n\r");
    else if(result == SUCCESS)
        printf("\n\rSMIF Initialized\n\r");

    /* Read device ID to confirm that SPI communication is OK */
    result = Read_ID(rx_data);
    if(result != SUCCESS)
        printf("\n\rRead ID command Failed\n\r");
    else if(result == SUCCESS)
        printf("\n\rThe Device ID is : %x %x %x",rx_data[0],rx_data[1],rx_data[2]);

    /* Read Status Register to confirm no errors */
    result = Read_Status_Register1(rx_data);
    printf("\n\rThe SR value is : %x",rx_data[0]);

    /***********************************************************************************/
    /* This part of code should be removed when running the code example on a flash
     * device where password protection mode has already been enabled. Not removing this
     * part will cause erase error resulting in the E_ERR bit of Status Register getting
     * set.
     *
     * PPB bits are erased before enabling password protection to avoid unnecessary sector
     * protection
     * */
    /***********************************************************************************/
    printf("\n\rErasing all PPB bits");

    /* WREN */
    result = Write_Enable();

    /* Erase all PPB bits */
    if(result == SUCCESS)
        result = PPB_Erase();

    /* Polling WIP */
    if(result == SUCCESS)
        result = Polling_WIP();

    printf("\n\rAll PPB bits erased`");
    /***********************************************************************************/

    /* Read ASP register value */
    if(result == SUCCESS)
        result = ASP_Register_Read(asp_reg);
    printf("\n\rThe initial value of ASP Register is : %x %x",asp_reg[1], asp_reg[0]);

    /* Check if password protection mode is enabled or not */
    if((asp_reg[0] & 0x04) == 0x04)
    {
        printf("\n\rPassword Protection Mode is not enabled");

        /* Read password */
        if(result == SUCCESS)
            result = Password_Read(rx_data);
        printf("\n\rThe Current Password is : %x %x %x %x %x %x %x %x",rx_data[7],rx_data[6],rx_data[5],rx_data[4],
                                                            rx_data[3],rx_data[2],rx_data[1],rx_data[0]);

        printf("\n\rPassword is being set to : 00000000");

        /* WREN */
        if(result == SUCCESS)
            result = Write_Enable();

        /* Program new password*/
        if(result == SUCCESS)
            result = Password_Program();

        /* Polling WIP */
        if(result == SUCCESS)
            result = Polling_WIP();

        /* Read password to confirm */
        if(result == SUCCESS)
            result = Password_Read(rx_data);
        printf("\n\rThe New Password is : %x %x %x %x %x %x %x %x",rx_data[7],rx_data[6],rx_data[5],rx_data[4],
                                                            rx_data[3],rx_data[2],rx_data[1],rx_data[0]);

        /* WREN */
        if(result == SUCCESS)
            result = Write_Enable();

        /* Enable password protection mode */
        asp_reg[0] = asp_reg[0] & (0xFB);
        printf("\n\rASP Register is being programmed with : %x %x", asp_reg[1], asp_reg[0]);

        /* ASP register Program */
        if(result == SUCCESS)
            result = ASP_Program(asp_reg);

        /* Polling WIP */
        if(result == SUCCESS)
            result = Polling_WIP();

        printf("\n\rPassword Protection Mode enabled permanently");

        /* Read ASP register value to confirm */
        if(result == SUCCESS)
            result = ASP_Register_Read(asp_reg);
        printf("\n\rThe ASP Register value is : %x %x", asp_reg[1], asp_reg[0]);

    }
    else
    {
        printf("\n\rPassword Protection Mode is permanently enabled");

        /* Read PPB bits for SA03 */
        if(result == SUCCESS)
            result = PPB_Read(addr_3B, rx_data);
        printf("\n\rThe PPB Access Register value for SA03 is : %x. SA03 is unprotected.", rx_data[0]);

        /* WREN */
        if(result == SUCCESS)
            result = Write_Enable();

        /* Sector Erase operation on SA03 */
        if(result == SUCCESS)
            result = Sector_Erase(addr_3B);

        /* Polling WIP */
        if(result == SUCCESS)
            result = Polling_WIP();

        /* WREN */
        if(result == SUCCESS)
            result = Write_Enable();

        /* Page Program Operation on SA03 */
        if(result == SUCCESS)
            result = Page_Program(addr_3B, tx_data);

        /* Polling WIP */
        if(result == SUCCESS)
            result = Polling_WIP();

        /* Reading SA03 to confirm */
        if(result == SUCCESS)
            result = Read(addr_3B, rx_data);

        printf("\n\rThe data in SA03 is :");
        for(i=0;i<PACKET_SIZE;i++)
        {
            if(i%8 == 0)
                printf("\n\r");
            printf("%x  ",rx_data[i]);

        }
        printf("\n\r");

        /* Unlock PPB bits using password to protect sector SA03 */
        if(result == SUCCESS)
            result = Password_Unlock();

        /* Polling WIP */
        if(result == SUCCESS)
            result = Polling_WIP();

        printf("\n\rPPB bits unlocked using password");

        /* WREN */
        if(result == SUCCESS)
            result = Write_Enable();

        /* PPB Program */
        printf("\n\rProtecting SA03 by programming the PPB bit for SA03");
        if(result == SUCCESS)
            result = PPB_Program(addr_3B);

        /* Polling WIP */
        if(result == SUCCESS)
            result = Polling_WIP();

        /* WREN */
        if(result == SUCCESS)
            result = Write_Enable();

        /* Sector Erase operation on SA03 */
        printf("\n\rErasing SA03");
        if(result == SUCCESS)
            result = Sector_Erase(addr_3B);

        /* RDSR1 */
        if(result == SUCCESS)
            result = Read_Status_Register1(rx_data);

        printf("\n\rThe SR 1 value after erase operation is : %x indicating unsuccessful erase", rx_data[0]);

        /* Clear Status Register to reset error flags */
        if(result == SUCCESS)
            result = Clear_Status_Register();

        /* Reading SA03 to confirm */
        if(result == SUCCESS)
            result = Read(addr_3B, rx_data);

        printf("\n\rThe data in SA03 is :");
        for(i=0;i<PACKET_SIZE;i++)
        {
            if(i%8 == 0)
                printf("\n\r");
            printf("%x  ",rx_data[i]);

        }
        printf("\n\r");
        printf("\n\rThe above data read back from SA03 indicates unsuccessful erase");

        /* WREN */
        if(result == SUCCESS)
            result = Write_Enable();

        /* PPB Erase */
        printf("\n\rErasing all PPB bits");
        if(result == SUCCESS)
            result = PPB_Erase();

        /* Polling WIP */
        if(result == SUCCESS)
            result = Polling_WIP();

        /* WREN */
        if(result == SUCCESS)
            result = Write_Enable();

        /* Sector Erase operation on SA03 */
        if(result == SUCCESS)
            result = Sector_Erase(addr_3B);

        /* Polling WIP */
        if(result == SUCCESS)
            result = Polling_WIP();

        /* Reading SA03 to confirm */
        if(result == SUCCESS)
            result = Read(addr_3B, rx_data);

        printf("\n\rThe data in SA03 after Sector Erase Operation is :");
        for(i=0;i<PACKET_SIZE;i++)
        {
            if(i%8 == 0)
                printf("\n\r");
            printf("%x  ",rx_data[i]);

        }
        printf("\n\rindicating successful erase");
    }

    printf("\n\r*******************************************************************\n\r");

}


/* [] END OF FILE */
