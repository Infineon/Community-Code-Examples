/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSoC 6 Deep Sleep Application
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

/* LED blink timer clock value in Hz  */
#define LED_BLINK_TIMER_CLOCK_HZ            (1000)

/* LED blink timer period value */
#define LED_BLINK_TIMER_PERIOD              (999)

/* Macro for interrupt priority */
#define TIMER_INTERRUPT_PRIORITIY           (7)
#define GPIO_INTERRUPT_PRIORITY             (7)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
/* Timer initialization function */
void timer_init(void);

/* Functions to disable hardware blocks */
void disable_ram(void);
void disable_udb(void);

/* Functions to enable hardware blocks */
void enable_ram(void);
void enable_udb(void);

/* ISR Functions */
static void isr_timer(void *callback_arg, cyhal_timer_event_t event);
static void isr_button(void *callback_arg, cyhal_gpio_irq_event_t event);

/* Power callbacks */
bool timer_power_callback(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void *arg);

/*******************************************************************************
* Global Variables
*******************************************************************************/
static bool timer_interrupt_flag    = false;
static bool button_interrupt_flag   = false;
static bool button_pressed_once     = false;

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

#ifdef CYBSP_LED_RGB_GREEN
    /* Initializing Green LED if supported by the target device */
    result = cyhal_gpio_init(CYBSP_LED_RGB_GREEN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        /* GPIO init failed. Stop program execution */
        CY_ASSERT(0);
    }
#endif

    timer_init();

    /* Callback declaration for Power Modes */
    cyhal_syspm_callback_data_t deepsleep_callback_data = { timer_power_callback,               /* Callback function */
                                                            CYHAL_SYSPM_CB_CPU_DEEPSLEEP,       /* Power States supported */
                                                            (cyhal_syspm_callback_mode_t) NULL, /* Modes to ignore */
                                                            NULL,                               /* Callback Argument */
                                                            NULL};                              /* For internal use */

    /* Power Management Callback registration */
    cyhal_syspm_register_callback(&deepsleep_callback_data);

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

            if (cyhal_syspm_get_system_state() == CYHAL_SYSPM_SYSTEM_NORMAL)
            {
                /* Switch to System ULP Power state */
                result = cyhal_syspm_set_system_state(CYHAL_SYSPM_SYSTEM_LOW);
                if (result != CY_RSLT_SUCCESS)
                {
                    /* Transition to System ULP mode failed. Handle error if code comes to this point */
                    CY_ASSERT(0);
                }
            }

            /* Enter Deep Sleep mode.
             * Please refer to timer_power_callback callback function or Design and Implementation section of 
             * Readme.md for information regarding the return state. 
             */
            result = cyhal_syspm_deepsleep();
            if (result != CY_RSLT_SUCCESS)
            {
                if(!button_pressed_once)
                {
                	/* Change flag to indicate the USER BUTTON was pressed once */
                	button_pressed_once = true;
                }
                else
                {
                    /* Transition to System Deep Sleep mode failed unexpectedly. Handle error if code comes to this point */
                    CY_ASSERT(0);
                }
                
            }
            else
            {
            	/* Change flag to indicate the USER BUTTON was pressed the second time */
            	button_pressed_once = false;
            }

            button_interrupt_flag = false;
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

    /* timer init failed. Stop program execution */
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
*    event            Timer/counter interrupt triggers
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
* Function Name: isr_button
********************************************************************************
* Summary:
* This is the interrupt handler function for the user button interrupt.
*
* Parameters:
*    callback_arg    Arguments passed to the interrupt callback
*    event            GPIO interrupt triggers
*
*******************************************************************************/
static void isr_button(void *callback_arg, cyhal_gpio_irq_event_t  event)
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
*  bool  - Returns false if button is pressed for the first time. If button is
*  		   pressed for the second time, returns ture.
*
*******************************************************************************/
bool timer_power_callback(cyhal_syspm_callback_state_t state, cyhal_syspm_callback_mode_t mode, void *arg)
{
    (void) arg;
    cy_rslt_t result;


    if (mode == CYHAL_SYSPM_CHECK_READY)
    {
    	/* Device is in CHECK_READY mode */
    	if(!button_pressed_once)
    	{
    		/* Button pressed for the first time */
#ifdef CYBSP_LED_RGB_GREEN
            /* Notify that the transition to Deep Sleep failed */
            cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
            cyhal_system_delay_ms(2000);
            cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
#endif
            /* Return false as USER BUTTON is pressed for first time */
    		return false;
    	}

    	/* Button pressed for the second time. Stop timer before checking
    	 * if device is ready for deep sleep transition
    	 */
         cyhal_timer_stop(&led_blink_timer);
         cyhal_system_delay_ms(1);
    }
    else if (mode == CYHAL_SYSPM_BEFORE_TRANSITION)
    {
    	/* Device is in BEFORE_TRANSITION mode */
        if (state == CYHAL_SYSPM_CB_CPU_DEEPSLEEP)
        {
            /* Disable RAM pages */
            disable_ram();

            /* Disable UDB */
            disable_udb();

#ifdef CYBSP_LED_RGB_GREEN
            /* Notify that the CPU is transitioning to Deep Sleep mode */
            cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
            cyhal_system_delay_ms(100);
            cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
#endif
        }
    }
    else if (mode == CYHAL_SYSPM_AFTER_TRANSITION)
    {
    	/* Device is in AFTER_TRANSITION mode */
#ifdef CYBSP_LED_RGB_GREEN        
        /* Notify that the CPU is transitioning to Active mode */
        cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
        cyhal_system_delay_ms(100);
        cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
#endif

        /* Enable RAM pages */
        enable_ram();

        /* Enable UDB */
        enable_udb();

        /* Start timer again */
        result = cyhal_timer_start(&led_blink_timer);
        if (result != CY_RSLT_SUCCESS)
        {
            /* Timer start failed. Stop program execution */
            CY_ASSERT(0);
        }
    }
    else if (mode == CYHAL_SYSPM_CHECK_FAIL)
    {
        /* Transition to System Deep Sleep mode failed. Handle error if code comes to this point */
    	CY_ASSERT(0);
    }

    return true;
}

/*******************************************************************************
* Function Name: disable_ram
********************************************************************************
* Summary:
*  Function to retain only 256 KB of SRAM. A custom linker script with modified
*  RAM length is used in this application.
*
* Note:
*  Setting the RAMx_PWR_CTL register to 0x05FA0000 disables the RAM controller
*  thereby disabling all the RAM pages in that controller. But setting certain
*  indices of the RAMx_PWR_MACRO_CTLregister array to 0x05FA0000 disables only
*  those RAM pages under the corresponding RAM controller.
*
*******************************************************************************/
void disable_ram(void)
{
    for(int32_t i = 8; i < 9; i++)
    {
        /* Disable 9th page in SRAM0 */
        CPUSS->RAM0_PWR_MACRO_CTL[i]= 0x05FA0000;

        /* If the application uses/requires any system calls, such as flash
         * write/erase, it must not disable the last page of SRAM (9th
         * page in PSoC 6 MCU devices with 288 KB RAM).
         */
    }

    /* Disable SRAM1 */
    CPUSS->RAM1_PWR_CTL= 0x05FA0000;
    /* Disable SRAM2 */
    CPUSS->RAM2_PWR_CTL= 0x05FA0000;
}

/*******************************************************************************
* Function Name: disable_udb
********************************************************************************
* Summary:
*  Disable UDBs to avoid any leakage of current through them. This function needs
*  to be removed if the device does not support UDBs.
*
*******************************************************************************/
void disable_udb(void)
{
    /* Disable UDBs */
    CPUSS->UDB_PWR_CTL = 0x05FA0000;
}

/*******************************************************************************
* Function Name: enable_ram
********************************************************************************
* Summary:
*  Function to enable the already disabled RAM. The function writes the default 
*  value into the respective power control registers. Please refer to the device
*  specific register TRM for more information regarding this.
*
*******************************************************************************/
void enable_ram(void)
{
    for(int32_t i = 8; i < 9; i++)
    {
        /* Enable 9th page in SRAM0 */
        CPUSS->RAM0_PWR_MACRO_CTL[i]= 0x05FA0001;
    }

    /* Enable SRAM1 */
    CPUSS->RAM1_PWR_CTL= 0x05FA0001;
    /* Enable SRAM2 */
    CPUSS->RAM2_PWR_CTL= 0x05FA0001;
}

/*******************************************************************************
* Function Name: enable_udb
********************************************************************************
* Summary:
*  Function to enable the already disabled UDBs. The function writes the default 
*  value into the respective power control registers. Please refer to the device
*  specific register TRM for more information regarding this.
*
*******************************************************************************/
void enable_udb(void)
{
    /* Enable UDBs */
    CPUSS->UDB_PWR_CTL = 0x05FA0001;
}

/* [] END OF FILE */
