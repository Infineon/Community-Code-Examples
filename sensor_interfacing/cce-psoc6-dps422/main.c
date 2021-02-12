/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for DPS422 interfacing with PSoC 6
*              example.
*
* Related Document: See README.md
*
*
*******************************************************************************
* (c) 2021, Cypress Semiconductor Corporation. All rights reserved.
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

#include "cy_retarget_io.h"
#include "dps422.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define SWITCH_INTR_PRIORITY    (3u)


/*******************************************************************************
* Function Prototypes
********************************************************************************/
void Switch_ISR(void *handler_arg, cyhal_gpio_irq_event_t event);


/*******************************************************************************
 * Global variable
 *******************************************************************************/
uint32_t interrupt_flag = false;


/*******************************************************************************
* Function Name: Switch_ISR
********************************************************************************
*
* Summary:
*  This function is executed when GPIO interrupt is triggered. It sets the
*  interrupt flag to TRUE.
*
*******************************************************************************/
void Switch_ISR(void *handler_arg, cyhal_gpio_irq_event_t event)
{
    /* Set interrupt flag */
    interrupt_flag = true;
}


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  System entrance point.
*  1. This function configures and initializes the GPIO interrupt.
*  2. Initializes the UART and I2C.
*  3. Initializes the pressure sensor.
*  4. Displays the most recent temperature and pressure reading upon every button
*     press.
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    __enable_irq();

    /* Initialize the user button */
    result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);

    /* Configure GPIO interrupt */
    cyhal_gpio_register_callback(CYBSP_USER_BTN, Switch_ISR, NULL);
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, SWITCH_INTR_PRIORITY, true);

    /* Initialize UART */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 115200);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    printf("\x1b[2J\x1b[;H \r");
    printf("**********************************************************************\r\n");
    printf("**          Interfacing DPS422 Pressure Sensor with PSoC 6          **\r\n");
    printf("**********************************************************************\r\n");

    /* Initialize I2C */
    cyhal_i2c_t I2C_DPS_422;
    result = cyhal_i2c_init(&I2C_DPS_422, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("\r\nError: I2C initialization failed\r\n");
        CY_ASSERT(0);
    }

    printf("**                   Initializing Pressure Sensor                   **\r\n");

    /* Initialize DPS422 Pressure Sensor */
    DPS_422_Init(&I2C_DPS_422);

    printf("**                  Sensor initialization complete                  **\r\n");
    printf("**********************************************************************\r\n");

    /* Read and display Product ID of DPS422 Pressure Sensor */
    int16_t prodId = DPS_422_readByte(registers[PROD_ID].regAddress);
    printf("**                         Product ID : 0x%x                        **\r\n",prodId);
    printf("**********************************************************************\r\n");

    printf("\r\nPress SW2 on the kit to receive the Temperature and Pressure reading\r\n");

    float temperature = 0;
    float pressure = 0;
    uint8_t oversampling = 5;
    int16_t ret;

    for (;;)
    {

        if(interrupt_flag == true)
        {

            /* Read and display temperature value */
            ret = DPS_422_measureTempOnceOversamplingRate(&temperature, oversampling);
            if (ret != 0)
            {
                printf("Failed to read temperature data from sensor\r\n");
            }
            else
            {
                printf("\r\nTemperature  : %f C\r\n", temperature);
            }

            /* Read and display pressure value */
            ret = DPS_422_measurePressureOnceOversamplingRate(&pressure, oversampling);
            if (ret != 0)
            {
                printf("Failed to read pressure data from sensor\r\n");
            }
            else
            {
                printf("Pressure     : %f Pascal\r\n", pressure);
            }
            printf("End\r\n");

            interrupt_flag = false;
        }
    }
}


/* [] END OF FILE */
