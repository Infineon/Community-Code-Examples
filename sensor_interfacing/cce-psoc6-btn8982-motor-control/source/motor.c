/*
 * motor.c
 *
 * Description: This file contains definition of functions related to
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

#include "motor.h"
#include "cyhal.h"
#include "cybsp.h"
#include <stdio.h>

/******************************************************************************
 *                             Global Static Variables
 ******************************************************************************/
static motor_max_speed_t motor_speed_config = {100, 100};
static motor_speed_t motor_speeds = {0, 0};

/******************************************************************************
 *                                Constants
 ******************************************************************************/
#define CLOCKWISE       1
#define ANTICLOCKWISE   0
#define MOTOR_PERIOD   (40)

/*******************************************************************************
 * Function Name: motor_init
 ********************************************************************************
 * Summary:
 * This function intializes the PWM and Smart-IO needed to operate the motors.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  cy_rslt_t: result
 *
 *******************************************************************************/
cy_rslt_t motor_init()
{
	/* Status variable */
	cy_rslt_t result;

	/* Initialize Motor1 PWM */
	result = Cy_TCPWM_PWM_Init(MOTOR1_PWM_HW, MOTOR1_PWM_NUM, &MOTOR1_PWM_config);
	Cy_TCPWM_PWM_Enable(MOTOR1_PWM_HW, MOTOR1_PWM_NUM);
	Cy_TCPWM_PWM_SetPeriod0(MOTOR1_PWM_HW, MOTOR1_PWM_NUM, MOTOR_PERIOD);

	/* Initialize Motor2 PWM */
	result = Cy_TCPWM_PWM_Init(MOTOR2_PWM_HW, MOTOR2_PWM_NUM, &MOTOR2_PWM_config);
	Cy_TCPWM_PWM_Enable(MOTOR2_PWM_HW, MOTOR2_PWM_NUM);
	Cy_TCPWM_PWM_SetPeriod0(MOTOR2_PWM_HW, MOTOR2_PWM_NUM, MOTOR_PERIOD);

	/* Start the PWMs */
	Cy_TCPWM_TriggerStart(MOTOR1_PWM_HW, MOTOR1_PWM_MASK);
	Cy_TCPWM_TriggerStart(MOTOR2_PWM_HW, MOTOR2_PWM_MASK);

	/* Initialize the Smart-IO Block */
	result = Cy_SmartIO_Init(SMARTIO_HW, &SMARTIO_config);
	Cy_SmartIO_Enable(SMARTIO_HW);

	printf("Motors Initialized!\r\n");

	return result;
}

/*******************************************************************************
 * Function Name: motor_set_max_speed
 ********************************************************************************
 * Summary:
 * This function sets the maximum speed for a particular motor.
 *
 * Parameters:
 *  Type of motor: motor_type_t motor (valid values: MOTOR_LEFT, MOTOR_RIGHT)
 *  Max speed of the motor: int speed (valid values: 0-100)
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void motor_set_max_speed(motor_type_t motor, int speed)
{
	/* Check for invalid params */
	if(speed > 100 || speed < 0){
		printf("Invalid speed parameters! Should be between 0-100 only!");
		CY_ASSERT(0);
	}

	if(motor == MOTOR_LEFT) motor_speed_config.motor_left_max_speed = speed;
	if(motor == MOTOR_RIGHT) motor_speed_config.motor_right_max_speed = speed;

	printf("Max speed set to %d\r\n", speed);
}

/*******************************************************************************
 * Function Name: motor_set_speed
 ********************************************************************************
 * Summary:
 * This function sets the speed for both motors.
 *
 * Parameters:
 *  Motor1 Speed: int motor1_speed (valid values: 0-100)
 *  Motor2 Speed: int motor2_speed (valid values: 0-100)
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void motor_set_speed(int motor1_speed, int motor2_speed){

	/* Check for invalid params */
	if(motor1_speed > 100 || motor2_speed > 100 || motor1_speed < 0 || motor2_speed < 0){
		printf("Invalid speed parameters! Should be between 0-100 only!");
		CY_ASSERT(0);
	}

	motor_speeds.motor1_speed = motor1_speed;
	motor_speeds.motor2_speed = motor2_speed;

	printf("Motor speed set to %d\r\n", motor1_speed);
}

/*******************************************************************************
 * Function Name: motor_drive
 ********************************************************************************
 * Summary:
 * This function drives the motor in a particular direction with set speed.
 *
 * Parameters:
 *  Movement Direction: motor_direction_t direction
 *  Valid Values:
 *  	Forward   - MOVE_FORWARD
 *  	Backward  - MOVE_BACKWARD
 *  	Right     - MOVE_RIGHT
 *  	Left      - MOVE_LEFT
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void motor_drive(motor_direction_t direction)
{
	/* Retrieve the speed set by BLE App */
	int motor1_speed = motor_speeds.motor1_speed;
	int motor2_speed = motor_speeds.motor2_speed;

	/* Scale the motor speeds based on motor max speed values */
	motor1_speed = ((motor1_speed * motor_speed_config.motor_left_max_speed) / 100);
	motor2_speed = ((motor2_speed * motor_speed_config.motor_right_max_speed) / 100);

	/* Scale the motor speeds to PWM Period range */
	uint32_t motor1_scaled_speed = (uint32_t)((motor1_speed * MOTOR_PERIOD)/100);
	uint32_t motor2_scaled_speed = (uint32_t)((motor1_speed * MOTOR_PERIOD)/100);

	/* Set the speed to 0 if STOP direction received */
	if(direction == MOVE_STOP){
		motor1_scaled_speed = 0;
		motor2_scaled_speed = 0;
	}

	/* ******************************************************************
	 *  __________________________________________________________
	 * 	| Direction         |               Motor Type            |
	 *  |                   |    Motor1        |    Motor2        |
	 *  |---------------------------------------------------------|
	 *  | Forward           |  Clockwise       |  Anticlockwise   |
	 *  | Backward          |  Anticlockwise   |  Clockwise       |
	 *  | Left              |  Clockwise       |  Clockwise       |
	 *  | Right             |  Anticlockwise   |  Anticlockwise   |
	 *  | Stop              |  NA              |  NA              |
	 *  |___________________|__________________|__________________|
	 * *******************************************************************/
	switch(direction)
	{
	case MOVE_FORWARD:
		printf("Moving forward!\r\n");
		Cy_GPIO_Write(MOTOR1_CONTROL_PORT, MOTOR1_CONTROL_NUM, CLOCKWISE);
		Cy_GPIO_Write(MOTOR2_CONTROL_PORT, MOTOR2_CONTROL_NUM, ANTICLOCKWISE);
		Cy_TCPWM_PWM_SetCompare0(MOTOR1_PWM_HW, MOTOR1_PWM_NUM, motor1_scaled_speed);
		Cy_TCPWM_PWM_SetCompare0(MOTOR2_PWM_HW, MOTOR2_PWM_NUM, motor2_scaled_speed);
		break;

	case MOVE_BACKWARD:
		printf("Moving backward!\r\n");
		Cy_GPIO_Write(MOTOR1_CONTROL_PORT, MOTOR1_CONTROL_NUM, ANTICLOCKWISE);
		Cy_GPIO_Write(MOTOR2_CONTROL_PORT, MOTOR2_CONTROL_NUM, CLOCKWISE);
		Cy_TCPWM_PWM_SetCompare0(MOTOR1_PWM_HW, MOTOR1_PWM_NUM, motor1_scaled_speed);
		Cy_TCPWM_PWM_SetCompare0(MOTOR2_PWM_HW, MOTOR2_PWM_NUM, motor2_scaled_speed);
		break;

	case MOVE_LEFT:
		printf("Moving left!\r\n");
		Cy_GPIO_Write(MOTOR1_CONTROL_PORT, MOTOR1_CONTROL_NUM, CLOCKWISE);
		Cy_GPIO_Write(MOTOR2_CONTROL_PORT, MOTOR2_CONTROL_NUM, CLOCKWISE);
		Cy_TCPWM_PWM_SetCompare0(MOTOR1_PWM_HW, MOTOR1_PWM_NUM, 0);
		Cy_TCPWM_PWM_SetCompare0(MOTOR2_PWM_HW, MOTOR2_PWM_NUM, motor2_scaled_speed);
		break;

	case MOVE_RIGHT:
		printf("Moving right!\r\n");
		Cy_GPIO_Write(MOTOR1_CONTROL_PORT, MOTOR1_CONTROL_NUM, ANTICLOCKWISE);
		Cy_GPIO_Write(MOTOR2_CONTROL_PORT, MOTOR2_CONTROL_NUM, ANTICLOCKWISE);
		Cy_TCPWM_PWM_SetCompare0(MOTOR1_PWM_HW, MOTOR1_PWM_NUM, motor1_scaled_speed);
		Cy_TCPWM_PWM_SetCompare0(MOTOR2_PWM_HW, MOTOR2_PWM_NUM, 0);
		break;

	case MOVE_STOP:
		Cy_TCPWM_PWM_SetCompare0(MOTOR1_PWM_HW, MOTOR1_PWM_NUM, motor1_scaled_speed);
		Cy_TCPWM_PWM_SetCompare0(MOTOR2_PWM_HW, MOTOR2_PWM_NUM, motor2_scaled_speed);
		printf("Motors Stopped!\r\n");
		break;

	default:
		/* Set the speeds to 0 if wrong direction recieved */
		Cy_TCPWM_PWM_SetCompare0(MOTOR1_PWM_HW, MOTOR1_PWM_NUM, 0);
		Cy_TCPWM_PWM_SetCompare0(MOTOR2_PWM_HW, MOTOR2_PWM_NUM, 0);
	}
}
