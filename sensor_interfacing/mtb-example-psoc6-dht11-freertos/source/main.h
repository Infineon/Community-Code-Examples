/******************************************************************************
* File Name:  main.h
*
* Description:  This file provides common constants and variables accessible
*               by both the tasks.
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


#ifndef SOURCE_MAIN_H_
#define SOURCE_MAIN_H_

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Pin used for the DATA signal can be changed here */
#define DATA_PIN 	P6_3

/* Error Codes */
#define SUCCESS 				0x00
#define DHT_CONNECTION_ERROR 	0x01
#define DHT_INCORRECT_VALUE		0x02


/* Task handles for each task */
TaskHandle_t DHT_Task_handle, Print_Task_handle;

/* Structure to store temperature and humidity readings */
struct readings
{
	float humidity;
	float temperature;
	uint8 result_code;
};

#endif /* SOURCE_MAIN_H_ */
