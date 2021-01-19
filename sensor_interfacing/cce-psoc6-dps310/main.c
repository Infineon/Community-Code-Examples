/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the DPS310 pressure sensor Application
*
* Related Document: See Readme.md
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
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

/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "dps310.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define OVERSAMPLING            7
#define I2C_MASTER_FREQUENCY    1000000

/*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 * This is the main function for CM4 CPU
 *    1. Initializes the BSP
 *    2. Initializes retarget IO for UART debug printing
 *    3. Initializes I2C using HAL driver
 *    4. Initializes the DPS310 pressure sensor
 *    5. Measures the temperature and the pressure values and prints it on the
*        serial terminal.
 *
 * Return:
 *  int
 *
 *******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    
    cyhal_i2c_t mI2C;
    
    float temperature = 0;
    float pressure = 0;

    int16_t ret;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize the retarget-io */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 115200);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen. */
    printf("\x1b[2J\x1b[;H \r");
    printf("========================================================\n\r");
    printf("Interfacing Infineon DPS310 Pressure Sensor with PSoC 6 \r\n");
    printf("========================================================\n\n\r");

    /* Define the I2C master configuration structure */
    cyhal_i2c_cfg_t i2c_master_config = {CYHAL_I2C_MODE_MASTER, 0 /* address is not used for master mode */, I2C_MASTER_FREQUENCY};

    /* Initialize the I2C */
    result = cyhal_i2c_init(&mI2C, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("\r\nI2C initialization failed\r\n");
        CY_ASSERT(0);
    }

    /* Configure the I2C resource to be master */
    result = cyhal_i2c_configure(&mI2C, &i2c_master_config);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("\r\nFailed to configure I2C\r\n");
        CY_ASSERT(0);
    }

    /* Print message to the console */
    printf("Initializing DPS310 Pressure sensor\r\n");

    /* Initialize the pressure sensor */
    DPS310_init(&mI2C);

    printf("Initialization Complete\r\n");

    for (;;)
    {
        printf("\r\n");

        /* Lets the DPS310 perform a single temperature measurement with the last (or standard) configuration
        * The result will be written to the parameter temperature
        * Oversampling can be a value from 0 to 7
        * The DPS310 will perform 2^oversampling internal temperature measurements and combine them to one result with higher precision
        * measurements. Higher precision takes more time. Consult datasheet for more information.
        */
        ret = DPS310_measureTempOnce_oversample(&temperature, OVERSAMPLING);
        if (ret != 0)
        {
            printf("Failed to read temperature data from sensor\r\n");
        }
        else
        {
            printf("Temperature: %f degree Celsius\r\n", temperature);
        }

        /* Pressure measurement behaves similar to temperature measurement */
        ret = DPS310_measurePressureOnce_oversample(&pressure, OVERSAMPLING);
        if (ret != 0)
        {
            printf("Failed to read pressure data from sensor\r\n");
        }
        else
        {
            printf("Pressure: %f Pascal\r\n", pressure);
        }

        cyhal_system_delay_ms(1000);
    }
}

/* [] END OF FILE */
