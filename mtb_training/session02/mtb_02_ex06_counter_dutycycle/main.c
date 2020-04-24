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
 * In this project, one PWM Block is initialized and run using PDL APIs. It generates
 * a frequency of 250Hz. A counter block is initialized to capture this signal
 * and count the number of pulses being received. The captured pulses every second
 * provide the value of frequency which is output on the serial terminal using UART.
 *
 * PWM Block   ---> Output connected to Counter Block
 * Timer Block ---> Captures PWM signal in capture mode
 *
 * Connections: Not required
 *
 * Output:
 *        250Hz frequency printed on Serial Terminal
 */

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

static uint32_t captureVal = 0, capturePrev = 0;
int intFlag = 0, count = 0;

#define COUNTER_CLOCKFREQ 10000

cy_stc_sysint_t timer_isr_config = {
		.intrSrc = COUNTER_IRQ,
		.intrPriority = 3
};

/* Gets triggered every 1s on overflow or underflow event (together called terminal count) */
void timer_isr()
{
	intFlag = 1;

	captureVal = Cy_TCPWM_Counter_GetCapture(COUNTER_HW, COUNTER_NUM);

	/* Clear interrupt */
	Cy_TCPWM_ClearInterrupt(COUNTER_HW, COUNTER_NUM, CY_TCPWM_INT_ON_CC);
}

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

	cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 115200);

	printf("Uart working \r\n");

	/* Configure a LED */
	cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

	Cy_TCPWM_PWM_Init(PWM_HW, PWM_NUM, &PWM_config);
	Cy_TCPWM_PWM_Enable(PWM_HW, PWM_NUM);
	Cy_TCPWM_TriggerStart(PWM_HW, PWM_MASK);

	/* Initialize the interrupt */
	Cy_SysInt_Init(&timer_isr_config, timer_isr);
	NVIC_EnableIRQ(timer_isr_config.intrSrc);

	/* Start the timer */
	Cy_TCPWM_Counter_Init(COUNTER_HW, COUNTER_NUM, &COUNTER_config);
	Cy_TCPWM_Counter_Enable(COUNTER_HW, COUNTER_NUM);
	Cy_TCPWM_TriggerStart(COUNTER_HW, COUNTER_MASK);
	Cy_TCPWM_TriggerCaptureOrSwap(COUNTER_HW, COUNTER_MASK);

	for (;;)
	{
		char printBuf[128];
		uint16 freq;

		if (intFlag)    /* Interrupt was triggered, read the captured counter value now */
		{
			/* Increment count every time interrupt is triggered */
			count++;

			printf("After interrupt\r\n");

			/* Print # of interrupts (Count) and instantaneous value of TCPWM counter when it captured the PWM rising edge */
			sprintf(printBuf, "Count = %d  CaptureValue = %lu  ", count, captureVal);
			printf("%s\n\r", printBuf);

			/* Print the measured frequency */
			freq = COUNTER_CLOCKFREQ / (captureVal - capturePrev);
			sprintf(printBuf, "MeasuredFreq = %u Hz ", freq);
			printf("%s\r\n", printBuf);

			/* Current captured value becomes the next 'previous value' */
			capturePrev = captureVal;

			/* Reset flag */
			intFlag = 0;
		}
	}
}

/* [] END OF FILE */
