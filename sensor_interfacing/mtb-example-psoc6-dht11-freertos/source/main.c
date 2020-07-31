/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the DHT 11 FreeRTOS example
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

#include <dht_task.h>
#include <print_task.h>
#include "main.h"


int main(void)
{

    cy_rslt_t result;

    /* Variable to store the queue handle */
    QueueHandle_t print_queue;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the retarget-io middleware */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

    /* Initialize the DATA pin. The pin used can be changed in main.h file */
    result = cyhal_gpio_init(DATA_PIN, CYHAL_GPIO_DIR_BIDIRECTIONAL, CYHAL_GPIO_DRIVE_PULLUP, 1);
    if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

    /* Initialize the LED pin */
    result = cyhal_gpio_init(CYBSP_LED4, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

    __enable_irq();

    printf("****************** "
               "PSoC 6: Interfacing DHT-11 using ModusToolbox 2.1 "
               "****************** \r\n\n");

    /* Create a queue to store the sensor readings */
    print_queue = xQueueCreate(5, sizeof(struct readings));
    if(print_queue == NULL)
    {
    	CY_ASSERT(0);
    }

    /* Create tasks */
    xTaskCreate(DHT_Task, "DHT Task", configMINIMAL_STACK_SIZE, (void*) print_queue, 2, NULL);
    xTaskCreate(Print_Task, "Print Task", 3*configMINIMAL_STACK_SIZE, (void*) print_queue, 1, NULL);

    /* Start scheduler */
    vTaskStartScheduler();

    for (;;)
    {

    }
}

/* [] END OF FILE */
