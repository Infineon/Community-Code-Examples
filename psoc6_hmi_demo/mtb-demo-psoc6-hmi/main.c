/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSoC6 EmWin TFT FreeRTOS 
*                 Application  for ModusToolbox.
*
* Related Document: See Readme.md
*
*******************************************************************************
* (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
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
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

#include "FreeRTOS.h"
#include "task.h"

#include "tft_task.h"
#include "capsense_task.h"
#include "http_client_task.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define TFT_TASK_STACK_SIZE             (2*1024)
#define TFT_TASK_PRIORITY               (configMAX_PRIORITIES - 1)

#define CAPSENSE_TASK_STACK_SIZE        (256u)
#define CAPSENSE_TASK_PRIORITY          (configMAX_PRIORITIES - 1)

#define HTTP_CLIENT_TASK_STACK_SIZE     (2*1024)
#define HTTP_CLIENT_TASK_PRIORITY       (configMAX_PRIORITIES - 2)


/*******************************************************************************
* Global Variables
*******************************************************************************/
/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

/*******************************************************************************
* Function Name: int main(void)
********************************************************************************
*
* Summary: This is the main for this code example.  This function initializes
*          the BSP, creates the tft-task and starts the scheduler.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* This enables RTOS aware debugging in OpenOCD */
    uxTopUsedPriority = configMAX_PRIORITIES - 1 ;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                    CY_RETARGET_IO_BAUDRATE);
    CY_ASSERT(result == CY_RSLT_SUCCESS);
    
    /* To avoid compiler warning */
    (void)result;

    __enable_irq();
    
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("**********************************************************\r\n");
    printf("PSoC 6 MCU EmWin TFT\r\n");
    printf("**********************************************************\r\n");

    /* Create the TFT task */
    xTaskCreate(tft_task, "tftTask", TFT_TASK_STACK_SIZE,
                    NULL,  TFT_TASK_PRIORITY,  NULL);

    xTaskCreate(task_capsense, "CapSense Task", CAPSENSE_TASK_STACK_SIZE,
                    NULL, CAPSENSE_TASK_PRIORITY, NULL);

    xTaskCreate(http_client_task, "HTTP client task", HTTP_CLIENT_TASK_STACK_SIZE, NULL,
                        HTTP_CLIENT_TASK_PRIORITY, NULL);

    /* Start the FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Should never get here. */
    CY_ASSERT(0);
}

/* [] END OF FILE */
