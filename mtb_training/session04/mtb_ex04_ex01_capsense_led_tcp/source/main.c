/******************************************************************************
* File Name: main.c
*
*
* This code example uses FreeRTOS and lwIP
*
* Related Document: README.md
*
********************************************************************************
* (c) 2020, Cypress Semiconductor Corporation.
********************************************************************************
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
*****************************************​**************************************/


/******************************************************************************
* Header files includes
******************************************************************************/
#include "cybsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "capsense_task.h"
#include "led_task.h"
#include "tcp_client.h"
#include "cy_retarget_io.h"


/*******************************************************************************
 * Global constants
 ******************************************************************************/
/* Priorities of user tasks in this project. configMAX_PRIORITIES is defined in
 * the FreeRTOSConfig.h and higher priority numbers denote high priority tasks.
 */
#define TASK_CAPSENSE_PRIORITY      (configMAX_PRIORITIES - 1)
#define TASK_TCP_PRIORITY           (configMAX_PRIORITIES - 2)
#define TASK_LED_PRIORITY           (configMAX_PRIORITIES - 3)

/* Stack sizes of user tasks in this project */
#define TASK_CAPSENSE_STACK_SIZE    (1000)
#define TASK_TCP_STACK_SIZE         (1000)
#define TASK_LED_STACK_SIZE         (1000)

/* Queue lengths of message queues used in this project */
#define SINGLE_ELEMENT_QUEUE        (1u)

TaskHandle_t Capsense_task;
TaskHandle_t TCP_task;
TaskHandle_t LED_task;
/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
*  System entrance point. This function sets up user tasks and then starts
*  the RTOS scheduler.
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    
    /*Initialise the UART for UART debug prints*/
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                            CY_RETARGET_IO_BAUDRATE);

    /* Enable global interrupts */
    __enable_irq();

    printf("\x1b[2J\x1b[;H");
    printf("============================================================\n");
    printf("capsense led tcp \n");
    printf("============================================================\n\n");
    /* Create the queues. See the respective data-types for details of queue
     * contents
     */ 
    led_command_data_q  = xQueueCreate(SINGLE_ELEMENT_QUEUE,
                                       sizeof(led_command_data_t));
    capsense_command_q  = xQueueCreate(SINGLE_ELEMENT_QUEUE,
                                       sizeof(capsense_command_t));
    tcp_command_data_q  = xQueueCreate(SINGLE_ELEMENT_QUEUE,
                                       sizeof(tcp_command_data_t));

    /* Create the user tasks. See the respective task definition for more
     * details of these tasks.
     */
    xTaskCreate(task_capsense, "CapSense Task", TASK_CAPSENSE_STACK_SIZE,
                NULL, TASK_CAPSENSE_PRIORITY, Capsense_task);
    xTaskCreate(tcp_client_task, "TCP task", TASK_TCP_STACK_SIZE,
                NULL, TASK_TCP_PRIORITY, TCP_task);
    xTaskCreate(task_led, "Led Task", TASK_LED_STACK_SIZE,
                NULL, TASK_LED_PRIORITY, LED_task);
    /* Start the RTOS scheduler. This function should never return */
    vTaskStartScheduler();

    /*~~~~~~~~~~~~~~~~~~~~~ Should never get here! ~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /* RTOS scheduler exited */
    /* Halt the CPU if scheduler exits */
    CY_ASSERT(0);
    
    for(;;)
    {
    }
}


/* [] END OF FILE  */
