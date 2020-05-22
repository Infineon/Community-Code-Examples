/******************************************************************************
* File Name: main.c
*
* Description: This code example features a 5-segment CapSense slider and two
*              CapSense buttons. Button 0 turns the LED ON, Button 1 turns the
*              LED OFF and the slider controls the brightness of the LED. The
*              code example also features interfacing with Tuner GUI using I2C
*              interface.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright (2019), Cypress Semiconductor Corporation. All rights reserved.
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


/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cybsp.h"
#include "cyhal.h"
#include "cycfg_capsense.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define CAPSENSE_INTR_PRIORITY  (7u)
#define SLAVE_ADDRESS           (8u)
#define LED_ON                  (0u)
#define LED_OFF                 (1u)


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static cy_status initialize_capsense(void);
static void process_gesture(uint32_t gestureStatus);
static void initialize_capsense_tuner(void);
static void capsense_isr(void);
static cy_rslt_t initialize_led(void);


/*******************************************************************************
* Global Variables
*******************************************************************************/
cyhal_ezi2c_t ezi2c_obj;
uint32_t gestureStatus = 0;

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function performs
*  - initial setup of device
*  - initialize CapSense
*  - initialize tuner communication
*  - scan touch input continuously and update the LED accordingly.
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_status status;
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize the LED */
    initialize_led();

    /* Initialize the CapSense Firmware and hardware modules */
    status = initialize_capsense();

    /* Initialize the EZI2C block to communicate with the Tuner GUI  */
    initialize_capsense_tuner();

    if(CYRET_SUCCESS != status)
    {
        /* Halt the CPU if CapSense initialization failed */
        CY_ASSERT(0);
    }

    /* Start first scan */
    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

    for(;;)
    {
        if(CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context))
        {
        	/* Increments the timestamp register for the predefined timestamp interval */

        	Cy_CapSense_IncrementGestureTimestamp(&cy_capsense_context);
            /* Process all widgets */
            Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);

            /* Process gesture for the widget */
            gestureStatus = Cy_CapSense_DecodeWidgetGestures(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, &cy_capsense_context);

            process_gesture(gestureStatus);

            /* Establishes synchronized operation between the CapSense
             * middle ware and the CapSense Tuner tool.
             */
            Cy_CapSense_RunTuner(&cy_capsense_context);

            /* Start next scan */
            Cy_CapSense_ScanAllWidgets(&cy_capsense_context);
        }
    }
}


/*******************************************************************************
* Function Name: process_touch
********************************************************************************
* Summary:
*  Gets the details of touch position detected, processes the touch input
*  and updates the LED status.
*
*******************************************************************************/
static void process_gesture(uint32_t gestureStatus)
{
	uint16_t direction;

	/* If Two-finger Scroll gesture is detected */
	if (CY_CAPSENSE_GESTURE_NO_GESTURE != (gestureStatus & CY_CAPSENSE_GESTURE_ONE_FNGR_FLICK_MASK ))
	{
		direction = (gestureStatus >> CY_CAPSENSE_GESTURE_DIRECTION_OFFSET) &\
						  CY_CAPSENSE_GESTURE_DIRECTION_MASK_ONE_FLICK;

		switch(direction >> CY_CAPSENSE_GESTURE_DIRECTION_OFFSET_ONE_FLICK)
		{
			case CY_CAPSENSE_GESTURE_DIRECTION_LEFT:
				cyhal_gpio_toggle(CYBSP_LED_RGB_BLUE);
				break;
			case CY_CAPSENSE_GESTURE_DIRECTION_RIGHT:
				cyhal_gpio_toggle(CYBSP_LED_RGB_GREEN);
				break;
			default:
				break;
		}
	}

	/* Single click gesture is detected */

	if((CY_CAPSENSE_GESTURE_ONE_FNGR_SINGLE_CLICK_MASK & gestureStatus) != CY_CAPSENSE_GESTURE_NO_GESTURE)
	{
		cyhal_gpio_toggle(CYBSP_LED_RGB_RED);
	}
}


/*******************************************************************************
* Function Name: initialize_capsense
********************************************************************************
* Summary:
*  This function initializes the CapSense and configure the CapSense
*  interrupt.
*
*******************************************************************************/
static cy_status initialize_capsense(void)
{
    cy_status status;

    /* CapSense interrupt configuration */
    const cy_stc_sysint_t CapSense_interrupt_config =
    {
        .intrSrc = CYBSP_CSD_IRQ,
        .intrPriority = CAPSENSE_INTR_PRIORITY,
    };

    /* Capture the CSD HW block and initialize it to the default state. */
    status = Cy_CapSense_Init(&cy_capsense_context);

    if(CYRET_SUCCESS == status)
    {
        /* Initialize CapSense interrupt */
        Cy_SysInt_Init(&CapSense_interrupt_config, capsense_isr);
        NVIC_ClearPendingIRQ(CapSense_interrupt_config.intrSrc);
        NVIC_EnableIRQ(CapSense_interrupt_config.intrSrc);

        /* Initialize the CapSense firmware modules. */
        status = Cy_CapSense_Enable(&cy_capsense_context);
    }

    return status;
}


/*******************************************************************************
* Function Name: capsense_isr
********************************************************************************
* Summary:
*  Wrapper function for handling interrupts from CapSense block.
*
*******************************************************************************/
static void capsense_isr(void)
{
    Cy_CapSense_InterruptHandler(CYBSP_CSD_HW, &cy_capsense_context);
}


/*******************************************************************************
* Function Name: initialize_capsense_tuner
********************************************************************************
* Summary:
*  Initializes interface between Tuner GUI and PSoC 6 MCU.
*
*******************************************************************************/
static void initialize_capsense_tuner(void)
{
	/* Populate slave configuration structure slave_config1 */
	cyhal_ezi2c_slave_cfg_t slave_config1 =
	{
			.slave_address = SLAVE_ADDRESS,
			.buf = (uint8_t *)&cy_capsense_tuner,\
			.buf_size = sizeof(cy_capsense_tuner),
			.buf_rw_boundary = sizeof(cy_capsense_tuner)
	};

	/* Populate the EZI2C configuration structure */
	cyhal_ezi2c_cfg_t ezi2c_config =
	{
			.two_addresses = false,
			.enable_wake_from_sleep = true,
			.data_rate = CYHAL_EZI2C_DATA_RATE_400KHZ,
			.slave1_cfg = slave_config1,
			.sub_address_size = CYHAL_EZI2C_SUB_ADDR16_BITS
	};
	/* Initialize EZI2C */
	cyhal_ezi2c_init(&ezi2c_obj, P6_1, P6_0, NULL, &ezi2c_config);
}

static cy_rslt_t initialize_led(void)
{
    cy_rslt_t rslt;

    rslt = cyhal_gpio_init((cyhal_gpio_t)CYBSP_LED_RGB_RED, CYHAL_GPIO_DIR_OUTPUT,
               CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    rslt |= cyhal_gpio_init((cyhal_gpio_t)CYBSP_LED_RGB_GREEN, CYHAL_GPIO_DIR_OUTPUT,
               CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    rslt |= cyhal_gpio_init((cyhal_gpio_t)CYBSP_LED_RGB_BLUE, CYHAL_GPIO_DIR_OUTPUT,
               CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    return rslt;
}


/* [] END OF FILE */
