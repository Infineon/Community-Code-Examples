/******************************************************************************
* File Name:  print_task.c
*
* Description:  This file contains all the functions and variables required for
*               proper operation of Print Task.
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


#include "print_task.h"

/*******************************************************************************
* Function Name: Print_Task
****************************************************************************//**
*
* Task function to print the temperature and humidity values on to a serial
* terminal after reading from the queue.
*
* \param pvParameters
* Void pointer which points to the queue handle
*
*******************************************************************************/

void Print_Task(void* pvParameters)
{
	/* Variable to store the queue handle */
	QueueHandle_t print_queue;
	print_queue = (QueueHandle_t) pvParameters;

	/* Variable to check if connection error message is already displayed */
	bool conn_err_displayed = false;

	/* Variable to store temperature and humidity values */
	struct readings DHT_reading = {0, 0};    /* Variables to store temperature and humidity values */

	for(;;)
	{
		/* *************************************************************
		 * Receive the temperature and humidity values from the queue.
		 * If the queue is empty, enter blocked state and wait for the
		 * DHT_Task to send the value to the queue. As Print_Task is
		 * given the least priority, it prints data only when all the
		 * other tasks are in blocked state.
		 * *************************************************************/
		xQueueReceive(print_queue, &DHT_reading, portMAX_DELAY);

		/* Print the DHT sensor readings if the values are valid */
		if(DHT_reading.result_code == SUCCESS)
		{
			printf("\r\nHumidity  =   %.2f\r\n",DHT_reading.humidity);
			printf("\r\nTemperature  =   %.2f\r\n",DHT_reading.temperature);
			conn_err_displayed = false;
		}

		/* *************************************************************
		 * If there is a connection error notify the user once that the
		 * sensor is not connected. The printing resumes once connection
		 * is established.
		 * *************************************************************/
		else if(DHT_reading.result_code == DHT_CONNECTION_ERROR)
		{
			if(conn_err_displayed == false)
			{
				printf("\r\nDHT Sensor Connection Failed\r\n");
				conn_err_displayed = true;
			}
		}
		else
		{
			/* **************************************************************
			 * This application does not do anything if the value read from
			 * the sensor is incorrect. The printing resumes once valid
			 * values are obtained. But any error handling code can be placed
			 * here based on the use case.
			 * **************************************************************/
		}
	}

}


