/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for the Empty PSoC6 Application
 *              for ModusToolbox.
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

/* Project Description
 *
 * In this project, one PWM Block is initialized and run using HAL APIs. The duty
 * cycle is incremented by 10 every 500ms to the max brightness value and then
 * decrements by 10 the same way to the lowest value.
 *
 * PWM Block ---> Connected to USER LED
 *
 * Connections: Not required
 *
 * Output:
 *        USER LED glows upto max brightness and then to its lowest and the cycle
 *        continues
 */

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"


int main(void)
{
	cy_rslt_t result;
	float duty_cycle = 0;
	int count_up_flag = 0;

	/* Initialize the device and board peripherals */
	result = cybsp_init() ;
	if (result != CY_RSLT_SUCCESS)
	{
		CY_ASSERT(0);
	}

	__enable_irq();

	cy_rslt_t rslt;

	cyhal_pwm_t pwm_obj;

	/* Initialize a left-aligned, continuous running PWM signal and assign normal and inverted outputs to pins */
	rslt = cyhal_pwm_init_adv(&pwm_obj, CYBSP_USER_LED, NC, CYHAL_PWM_LEFT_ALIGN, true, 0, false, NULL);

	/* Set an initial duty cycle of 0% on the normal PWM output with a frequency of 1kHz */
	rslt = cyhal_pwm_set_duty_cycle(&pwm_obj, 0, 1000);

	/* Start the PWM output */
	rslt = cyhal_pwm_start(&pwm_obj);

	for (;;)
	{
		if(duty_cycle == 100)
		{
			/* Change the flag to down counting mode */
			count_up_flag = 0;
		}

		if(duty_cycle == 0)
		{
			/* Change the flag to up counting mode */
			count_up_flag = 1;
		}

		/* Update new duty cycle */
		rslt = cyhal_pwm_set_duty_cycle(&pwm_obj, duty_cycle, 1000);

		/* Increase the duty cycle */
		count_up_flag ? (duty_cycle = duty_cycle + 10) : (duty_cycle = duty_cycle - 10);

		/* Delay to observe the output */
		cyhal_system_delay_ms(500);
	}
}

/* [] END OF FILE */
