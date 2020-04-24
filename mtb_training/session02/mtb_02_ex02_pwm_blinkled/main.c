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
 * In this project, one PWM Block is initialized and run using HAL APIs and three
 * PWM Blocks are run using PDL APIs.
 *
 * PWM Block 1 ---> Connected to USER LED
 * PWM Block 2 ---> Connected to RED LED
 * PWM Block 3 ---> Connected to GREEN LED
 * PWM Block 4 ---> Connected to BLUE LED
 *
 * Connections: Not required
 *
 * Output:
 *        USER LED Blinks every 1Hz at 50% Duty Cycle
 *        RED LED Blinks every 1s at 50% Duty Cycle
 *        GREEN LED Blinks every 2s at 50% Duty Cycle
 *        BLUE LED Blinks every 4s at 50% Duty Cycle
 *
 *        Note: For PSoC6 Proto Kit, you will only see the output
 *        in USER LED since it doesn't have RGB.
 */

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

#define HAL_ENABLED (1U)
#define PDL_ENABLED (1U)

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

#if HAL_ENABLED

    cyhal_pwm_t pwm_obj;

    result = cyhal_pwm_init(&pwm_obj, CYBSP_USER_LED, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    cyhal_pwm_set_duty_cycle(&pwm_obj, 50, 1);

    cyhal_pwm_start(&pwm_obj);

    /* See output on logic analyzer */

#endif

#if PDL_ENABLED

    Cy_TCPWM_PWM_Init(RED_PWM_HW, RED_PWM_NUM, &RED_PWM_config);
    Cy_TCPWM_PWM_Init(GREEN_PWM_HW, GREEN_PWM_NUM, &GREEN_PWM_config);
    Cy_TCPWM_PWM_Init(BLUE_PWM_HW, BLUE_PWM_NUM, &BLUE_PWM_config);

    Cy_TCPWM_PWM_Enable(RED_PWM_HW, RED_PWM_NUM);
    Cy_TCPWM_PWM_Enable(GREEN_PWM_HW, GREEN_PWM_NUM);
    Cy_TCPWM_PWM_Enable(BLUE_PWM_HW, BLUE_PWM_NUM);

    Cy_TCPWM_TriggerStart(RED_PWM_HW, RED_PWM_MASK);
    Cy_TCPWM_TriggerStart(GREEN_PWM_HW, GREEN_PWM_MASK);
    Cy_TCPWM_TriggerStart(BLUE_PWM_HW, BLUE_PWM_MASK);

#endif

    for (;;)
    {

    }
}

/* [] END OF FILE */
