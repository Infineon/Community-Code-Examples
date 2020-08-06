/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSoC 6 Hibernate Application
*              for ModusToolbox.
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Hibernate wakeup pin. Change according to the device */
#define HIBERNATE_WAKEUP_PIN                (CYBSP_USER_BTN)

/* Custom pin for entering hibernate mode */
#define CUSTOM_USER_PIN                     (P6_0)

/* Macro for GPIO states */
#define CYBSP_GPIO_STATE_LOW                (0)
#define CYBSP_GPIO_STATE_HIGH               (1)

/* Macro for interrupt priority */
#define TIMER_INTERRUPT_PRIORITIY           (7)
#define GPIO_INTERRUPT_PRIORITY             (7)

/* LED blink timer clock value in Hz  */
#define LED_BLINK_TIMER_CLOCK_HZ            (1000)

/* LED blink timer period value */
#define LED_BLINK_TIMER_PERIOD              (999)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
/* Timer initialization function */
void timer_init(void);

/* Hibernate wakeup pin initialization function */
void setup_hib_wakeup_interrupt(void);

/* ISR Functions */
static void isr_timer(void *callback_arg, cyhal_timer_event_t event);
static void isr_custom_user_pin(void *callback_arg, cyhal_gpio_irq_event_t event);

/* Power callbacks */
bool timer_power_callback(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void *arg);

/*******************************************************************************
* Global Variables
*******************************************************************************/
static bool timer_interrupt_flag    = false;
static bool button_interrupt_flag   = false;

/* Timer object used for blinking the LED */
cyhal_timer_t led_blink_timer;

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

    /* Initializing the Custom User pin */
    result = cyhal_gpio_init(CUSTOM_USER_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_GPIO_STATE_LOW);
    if (result != CY_RSLT_SUCCESS)
    {
        /* GPIO init failed. Stop program execution */
        CY_ASSERT(0);
    }

    /* Register callback function - isr_custom_user_pin */
    cyhal_gpio_register_callback(CUSTOM_USER_PIN, isr_custom_user_pin, NULL);
    
    /* Enable falling edge interrupt event with interrupt priority set to GPIO_INTERRUPT_PRIORITY */
    cyhal_gpio_enable_event(CUSTOM_USER_PIN, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);

    /* Initializing the USER LED GPIO */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        /* GPIO init failed. Stop program execution */
        CY_ASSERT(0);
    }

#ifdef CYBSP_LED_RGB_GREEN
    /* Initializing Green LED if supported by the target device */
    result = cyhal_gpio_init(CYBSP_LED_RGB_GREEN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        /* GPIO init failed. Stop program execution */
        CY_ASSERT(0);
    }

    /* Toggle green LED if device is reset due to hibernate wakeup */
    if(cyhal_system_get_reset_reason() == CYHAL_SYSTEM_RESET_HIB_WAKEUP )
    {
        cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
        cyhal_system_delay_ms(500);
        cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
        cyhal_system_delay_ms(500);
        cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
        cyhal_system_delay_ms(500);
        cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
        cyhal_system_delay_ms(500);
    }
#endif

    timer_init();

    /* Callback declaration for Power Modes */
    cyhal_syspm_callback_data_t hibernate_callback_data = { timer_power_callback,               /* Callback function */
                                                            CYHAL_SYSPM_CB_SYSTEM_HIBERNATE,    /* Power States supported */
                                                            (cyhal_syspm_callback_mode_t) NULL, /* Modes to ignore */
                                                            NULL,                               /* Callback Argument */
                                                            NULL};                              /* For internal use */

    /* Power Management Callback registration */
    cyhal_syspm_register_callback(&hibernate_callback_data);

    for (;;)
    {
        if(timer_interrupt_flag == true)
        {
            /* Toggle USER LED when timer interrupt occurs */
            cyhal_gpio_toggle(CYBSP_USER_LED);
            timer_interrupt_flag = false;
        }

        if(button_interrupt_flag == true)
        {
            /* Setup hibernate wakeup source */
            setup_hib_wakeup_interrupt();

            /* Enter Hibernate mode */
            result = cyhal_syspm_hibernate(CYHAL_SYSPM_HIBERNATE_PINB_LOW);

            /* Code should not enter here. Handle error if code comes to this point */
            CY_ASSERT(0);
        }
    }
}

/*******************************************************************************
* Function Name: timer_init
********************************************************************************
* Summary:
* This function creates and configures a Timer object. The timer ticks 
* continuously and produces a periodic interrupt on every terminal count 
* event. The period is defined by the 'period' and 'compare_value' of the 
* timer configuration structure 'led_blink_timer_cfg'. Without any changes, 
* this application is designed to produce an interrupt every 1 second.
*
* Parameters:
*  none
*
*******************************************************************************/

void timer_init(void)
 {
    cy_rslt_t result;

    /* Initialize the timer object. Does not use pin output ('pin' is NC) and
     * does not use a pre-configured clock source ('clk' is NULL). */
    result = cyhal_timer_init(&led_blink_timer, NC, NULL);

    if (result != CY_RSLT_SUCCESS)
    {
        /* Timer init failed. Stop program execution */
        CY_ASSERT(0);
    }

    const cyhal_timer_cfg_t led_blink_timer_cfg = 
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = LED_BLINK_TIMER_PERIOD,   /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = true,              /* Run timer indefinitely */
        .value = 0                          /* Initial value of counter */
    };
    /* Configure timer period and operation mode such as count direction, 
       duration */
    cyhal_timer_configure(&led_blink_timer, &led_blink_timer_cfg);

    /* Set the frequency of timer's clock source */
    cyhal_timer_set_frequency(&led_blink_timer, LED_BLINK_TIMER_CLOCK_HZ);

    /* Assign the ISR to execute on timer interrupt */
    cyhal_timer_register_callback(&led_blink_timer, isr_timer, NULL);

    /* Set the event on which timer interrupt occurs and enable it */
    cyhal_timer_enable_event(&led_blink_timer, CYHAL_TIMER_IRQ_TERMINAL_COUNT,
                              TIMER_INTERRUPT_PRIORITIY, true);

    /* Start the timer with the configured settings */
    cyhal_timer_start(&led_blink_timer);


 }

/*******************************************************************************
* Function Name: isr_timer
********************************************************************************
* Summary:
* This is the interrupt handler function for the timer interrupt.
*
* Parameters:
*    callback_arg    Arguments passed to the interrupt callback
*    event           Timer/counter interrupt triggers
*
*******************************************************************************/
static void isr_timer(void *callback_arg, cyhal_timer_event_t event)
{
    (void) callback_arg;
    (void) event;

    /* Set the interrupt flag and process it from the main loop */
    timer_interrupt_flag = true;
}

/*******************************************************************************
* Function Name: isr_custom_user_pin
********************************************************************************
* Summary:
* This is the interrupt handler function for the user button interrupt.
*
* Parameters:
*    callback_arg    Arguments passed to the interrupt callback
*    event           GPIO interrupt triggers
*
*******************************************************************************/
static void isr_custom_user_pin(void *callback_arg, cyhal_gpio_irq_event_t  event)
{
    (void) callback_arg;
    (void) event;

    /* Set the interrupt flag and process it from the main loop */
    button_interrupt_flag = true;
}

/*******************************************************************************
* Function Name: timer_power_callback
********************************************************************************
* Summary:
*  Callback implementation for the Timer block. It changes the blinking pattern
*  based on the power state and MCU state.
*
* Parameters:
*  state - state the system or CPU is being transitioned into
*  mode  - callback mode
*  arg   - user argument (not used)
*
* Return:
*  Always true
*
*******************************************************************************/
bool timer_power_callback(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void *arg)
{
    (void) arg;

    if (mode == CYHAL_SYSPM_CHECK_READY)
    {
        /* Device is in CHECK_READY mode. Stop timer before checking if
         * device is ready for hibernate transition.
         */
        cyhal_timer_stop(&led_blink_timer);
        cyhal_system_delay_ms(1);
    }
    else if (mode == CYHAL_SYSPM_BEFORE_TRANSITION)
    {
        /* Device is in BEFORE_TRANSITION mode */
        if (state == CYHAL_SYSPM_CB_SYSTEM_HIBERNATE)
        {

#ifdef CYBSP_LED_RGB_GREEN
            /* Notify that the CPU is transitioning to Hibernate mode */
            cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
            cyhal_system_delay_ms(100);
            cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
            /* Free Green LED GPIO */
            cyhal_gpio_free(CYBSP_LED_RGB_GREEN);
#endif
            /* Free CUSTOM_USER_PIN and USER_LED GPIO */
            cyhal_gpio_free(CUSTOM_USER_PIN);
            cyhal_gpio_free(CYBSP_USER_LED);
        }
    }
    /* After transition callback is not executed as device resets when it wakes
     * up from hibernate mode.
     */
    else if (mode == CYHAL_SYSPM_CHECK_FAIL)
    {
        /* Transition to hibernate mode failed. Handle error if code comes to this point */
        CY_ASSERT(0);
    }

    return true;
}

/*******************************************************************************
* Function Name: setup_hib_wakeup_interrupt
********************************************************************************
* Summary:
*  Function to initialize the hibernate wakeup pin and enable interrupt on
*  rising edge event.
*
*******************************************************************************/
void setup_hib_wakeup_interrupt(void)
{
    cy_rslt_t result;

    /* Initializing the Hibernate wakeup pin */
    result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        /* GPIO init failed. Stop program execution */
        CY_ASSERT(0);
    }

    /* Enable rising edge interrupt event with interrupt priority set to GPIO_INTERRUPT_PRIORITY */
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
}

/* [] END OF FILE */
