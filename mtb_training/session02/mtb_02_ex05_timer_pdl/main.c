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
 * In this project, one Timer Block is initialized and run using PDL APIs. The
 * clock is set to 10KHz and Period to 9999 to give 1s terminal count interrupt.
 * The flag is set in the interrupt which is checked periodically in main
 * to blink the LED.
 *
 * Timer Block ---> Connected to USER LED
 *
 * Connections: Not required
 *
 * Output:
 *        USER LED blinks every 1s
 */

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_sysint.h"


cy_stc_sysint_t timer_isr_config = {
		.intrSrc = TIMER_IRQ,
		.intrPriority = 3
};

/* Gets triggered every 1s on overflow or underflow event (together called terminal count) */
void timer_isr()
{
	/* Toggle LED */
	cyhal_gpio_toggle(CYBSP_USER_LED);

	/* Clear interrupt */
	Cy_TCPWM_ClearInterrupt(TIMER_HW, TIMER_NUM, CY_TCPWM_INT_ON_TC);
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

    /* Configure a LED */
    cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    /* Initialize the interrupt */
    Cy_SysInt_Init(&timer_isr_config, timer_isr);
    NVIC_EnableIRQ(timer_isr_config.intrSrc);

    /* Start the timer */
    Cy_TCPWM_Counter_Init(TIMER_HW, TIMER_NUM, &TIMER_config);
    Cy_TCPWM_Counter_Enable(TIMER_HW, TIMER_NUM);
    Cy_TCPWM_TriggerStart(TIMER_HW, TIMER_MASK);

    for (;;)
    {
    }
}

/* [] END OF FILE */
