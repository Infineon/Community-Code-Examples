/******************************************************************************
* File Name:   tlx4966.c
*
* Description: This file contains the function definitions for the 
*              TLx4966 sensor.
*
*******************************************************************************
* (c) 2021, Cypress Semiconductor Corporation. All rights reserved.
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

#include "tlx4966.h"

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
 static cy_rslt_t timer_init(void);
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_irq_event_t event);

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Variable to hold the direction pin - Connected to Q1 of TLx4966l */
cyhal_gpio_t dir_gpio;

/* Time variables used to measure speed of rotation */
uint32_t curr_time, prev_time;

/* Variable to store the measured speed */
float speed;

/*******************************************************************************
* Function Name: TLx4966_init
********************************************************************************
* Summary:
*  This function initializes all the resources necessary for TLE4966K sensor's 
*  operation. The GPIO pins and interrupts are initialized, and the timer block 
*  configured and initialized for speed measurement.
* 
*
* Parameters:
*  speed_pin        GPIO connected to Q2 of TLx4966 indicating speed of rotation
*  dir_pin          GPIO connected to Q1 of TLx4966 indicating direction of 
*                   rotation
*  user_speed_unit  Denotes the unit of speed measurement. Can be set to Hz, Rads 
*                   or RPM
*
* Return:
*  cy_rslt_t
*******************************************************************************/
cy_rslt_t TLx4966_init(cyhal_gpio_t speed_pin, cyhal_gpio_t dir_pin, float user_speed_unit)
{
    cy_rslt_t result;

    speed_unit = user_speed_unit;

    /* Initialize the dirction and speed GPIO */
    result = cyhal_gpio_init(speed_pin, CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    result |= cyhal_gpio_init(dir_pin, CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    dir_gpio = dir_pin;
    /* Configure GPIO interrupt */
    cyhal_gpio_register_callback(speed_pin,
                                 gpio_interrupt_handler, NULL);
    cyhal_gpio_enable_event(speed_pin, CYHAL_GPIO_IRQ_RISE,
                                 GPIO_INTERRUPT_PRIORITY, true);

    /* Initialize timer */
    result |= timer_init();
    return result;
}

/*******************************************************************************
* Function Name: timer_init
********************************************************************************
* Summary:
*  This function creates and configures a Timer object. 
*
* Return:
*  cy_rslt_t
*
*******************************************************************************/
 static cy_rslt_t timer_init(void)
 {
    cy_rslt_t result;
    cyhal_clock_t clock_obj;

    result = cyhal_clock_allocate(&clock_obj, CY_SYSCLK_DIV_24_5_BIT);

    result |= cyhal_clock_set_frequency(&clock_obj, 1000, NULL);
    if (!cyhal_clock_is_enabled(&clock_obj))
    {
    	result |= cyhal_clock_set_enabled(&clock_obj, true, true);
    }

    const cyhal_timer_cfg_t timer_cfg =
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = TIMER_PERIOD,             /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = true,              /* Run timer indefinitely */
        .value = 0                          /* Initial value of counter */
    };

    /* Initialize the timer object. Does not use input pin ('pin' is NC),
     * but pre-configures the clock object to be used(clock_obj). */
    result |= cyhal_timer_init(&timer_obj, NC, &clock_obj);

    /* Configure timer period and operation mode such as count direction,
       duration */
    result |= cyhal_timer_configure(&timer_obj, &timer_cfg);

    /* Start the timer with the configured settings */
    result |= cyhal_timer_start(&timer_obj);

    return result;
 }

/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  handler_arg (unused)
*  event (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_irq_event_t event)
{
    curr_time = cyhal_timer_read(&timer_obj);
    speed = (float) speed_unit/(curr_time - prev_time);
    prev_time = curr_time;
}

/*******************************************************************************
* Function Name: TLx4966_readSpeed
********************************************************************************
* Summary:
*  Returns the current speed of rotation 
*
* Return:
*  float    speed of rotation measured
*
*******************************************************************************/
float TLx4966_readSpeed()
{
    return speed;
}

/*******************************************************************************
* Function Name: TLx4966_readDir
********************************************************************************
* Summary:
*  Returns the direction of rotation.
*
* Return:
*  uint8_t    direction of rotation measured
*
*******************************************************************************/
uint8_t TLx4966_readDir()
{
    return cyhal_gpio_read(dir_gpio);
}

/* [] END OF FILE */
