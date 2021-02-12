/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the TLx4966 interfacing with PSoC 6 
*              Example
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "tlx4966.h"

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU. It sets up the TLx4966 sensor and 
* retarget IO. The speed and direction are periodically monitored and displayed 
* to the user whenever there is a change.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    uint8_t dir, prev_dir = 0;
    float speed, prev_speed = 0;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);

    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("********************************************************************\r\n"
           "Interfacing Infineon TLE4966K Direction and Speed Sensor with PSoC 6\r\n"
           "********************************************************************\r\n\n");

    /* Initialize resources required for Direction and Speed measurement from TLE4966K*/
    result = TLx4966_init(P10_0, P10_3, TLx4966_SPEED_COEF_RPM);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("Timer or GPIO initialization failed. Halting application execution\r\n");
        CY_ASSERT(0);
    }

    for (;;)
    {
        /* Read the current speed and direction */
        speed = TLx4966_readSpeed();
        dir = TLx4966_readDir();

        /* Print the data only when the speed or direction changes */
        if(speed != prev_speed || dir != prev_dir)
        {
            printf("Speed = %f %s   \r\nDirection = %d\r\n", speed, SPEED_UNIT, dir);
            prev_speed = speed;
            prev_dir = dir;
            /* Move cursor to previous line */
            printf("\x1b[1F\x1b[1F");
        }
    }
}

/* [] END OF FILE */
