/*
 * bot_motor_task.c
 *
 * Description: This file contains definition of tasks related to
 * motor control operation.
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

#include "motor_task.h"
#include "motor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cyhal.h"
#include <stdio.h>

/******************************************************************************
 *                                Constants
 ******************************************************************************/
#define MOTOR1_MAX_SPEED  (100)
#define MOTOR2_MAX_SPEED  (100)

/*******************************************************************************
 * Function Name: motor_task
 ********************************************************************************
 * Summary:
 * This FreeRTOS does the following:
 * 	(1) Initializes the motors
 * 	(2) Sets the max speed of the motors
 * 	(2) Waits for BLE App to notify the direction to move the motors
 *
 * Parameters:
 *  None
 *
 * Return:
 *  cy_rslt_t: result
 *
 *******************************************************************************/
void motor_task(){

	/* Notification values received from other tasks */
	uint32_t notifiedDirection;

	uint32_t result;

	/* Initialize the motors */
	result = motor_init();

	/* Halt if initialization fails */
	if(result != CY_RSLT_SUCCESS){
		printf("Motor initialization failed!");
		CY_ASSERT(0);
	}

	/* Set max speeds for the motors */
	motor_set_max_speed(MOTOR_LEFT, MOTOR1_MAX_SPEED);
	motor_set_max_speed(MOTOR_RIGHT, MOTOR2_MAX_SPEED);

	for(;;)
	{
		/* Wait for a notification from Mobile App */
		xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
				UINT32_MAX,         /* Reset the notification value to 0 on exit. */
				&notifiedDirection,   /* Notified value pass out in ulNotifiedValue. */
				portMAX_DELAY );    /* Block indefinitely. */

		/* Drive the motors */
		printf("Driving Motors...");
		motor_drive(notifiedDirection);
	}
}
