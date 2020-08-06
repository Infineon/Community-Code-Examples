/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSoC6 Simple Deep Sleep
*              Application for ModusToolbox.
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Macro for interrupt priority */
#define GPIO_INTERRUPT_PRIORITY             (7)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
/* ISR Functions */
static void isr_button(void *callback_arg, cyhal_gpio_irq_event_t event);

/*******************************************************************************
* Global Variables
*******************************************************************************/
static bool button_interrupt_flag = false;

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        /* Board init failed. Stop program execution */
        CY_ASSERT(0);
    }

    __enable_irq();

    /* Initializing the User Button GPIO */
    result = cyhal_gpio_init(CYBSP_USER_BTN1, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        /* GPIO init failed. Stop program execution */
        CY_ASSERT(0);
    }

    /* Register callback function - isr_button */
    cyhal_gpio_register_callback(CYBSP_USER_BTN1, isr_button, NULL);

    /* Enable falling edge interrupt event with interrupt priority set to GPIO_INTERRUPT_PRIORITY */
    cyhal_gpio_enable_event(CYBSP_USER_BTN1, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);

    /* Initializing the USER LED GPIO */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        /* GPIO init failed. Stop program execution */
        CY_ASSERT(0);
    }

    for (;;)
    {
        /* Toggle USER LED after every one second delay*/
        cyhal_gpio_toggle(CYBSP_USER_LED);
        cyhal_system_delay_ms(1000);

        if(button_interrupt_flag == true)
        {
            /* Enter Deep Sleep mode */
            result = cyhal_syspm_deepsleep();
            if(result != CY_RSLT_SUCCESS)
            {
                /* Transition to System Deep Sleep mode failed. Handle error if code comes to this point */
                CY_ASSERT(0);
            }
            /* Wait a bit to avoid glitches from the button press */
            cyhal_system_delay_ms(100);
            button_interrupt_flag = false;
        }
    }
}

/*******************************************************************************
* Function Name: isr_button
********************************************************************************
* Summary:
* This is the interrupt handler function for the user button interrupt.
*
* Parameters:
*    callback_arg    Arguments passed to the interrupt callback
*    event           GPIO interrupt triggers
*
*******************************************************************************/
static void isr_button(void *callback_arg, cyhal_gpio_irq_event_t  event)
{
    (void) callback_arg;
    (void) event;

    /* Set the interrupt flag and process it from the main loop */
    button_interrupt_flag = true;
}

/* [] END OF FILE */
