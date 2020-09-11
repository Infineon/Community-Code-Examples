/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for PSoC 6 LPComp Hibernate wake-up example
*
* Related Document: See README.md
*
*******************************************************************************
(c) 2020, Cypress Semiconductor Corporation. All rights reserved.
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
* Project Description
********************************************************************************
* In this project,LPComp is setup as a wake-up source for the device which is
* transitioned to hibernate mode.
*
* (1) LPComp is set to Ultra-Low power mode
* (2) Local Vref of the LPComp is enabled and connected to negative terminal
* (3) LPComp positive terminal is connected to dedicated pin (Pin P5[6])
* (4) Device wake-up source is set to LPComp High Output
*
* Connections:
* P5[6] ---> Connect to Ground to keep the device in hibernate mode
*            Connect the pin to any voltage higher than local reference to wake-up
*            the device
* Note: Better not to leave the input pin floating.
*
* Output: Connect the input to GND pin to see the LED blinking, indicating the device
* continuously waking up from hibernate. The device current consumption around ~10mA
* range.When input is connected to a higher voltage(VDD for example) the LED stops blinking.
* In hibernate mode device current consumption is around ~3uA range.
**********************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Macros for defining LPComp and Channel */
#define MYLPCOMP_HW      LPCOMP
#define MYLPCOMP_CHANNEL CY_LPCOMP_CHANNEL_0
/* Macros to LED Pin GPIO */
#define LED_GREEN_PORT     GPIO_PRT1
#define LED_GREEN_PIN      1U

/* Configure the LPComp structure to operate in ULP mode */
const cy_stc_lpcomp_config_t MYLPCOMP_config =
{
    /*Select direct output of the comparator */
    .outputMode = CY_LPCOMP_OUT_DIRECT,
    /*Enable Hysteresis*/
    .hysteresis = CY_LPCOMP_HYST_ENABLE,
    /*Set Ultra-Low Power Mode*/
    .power = CY_LPCOMP_MODE_ULP,
    /*Disable interrupt for LPComp*/
    .intType = CY_LPCOMP_INTR_DISABLE,
};
/* Set the syspm callback parameters */
static cy_stc_syspm_callback_params_t LPCompHibernateCallbackParams =
{
    /* Base address of hardware instance */
    MYLPCOMP_HW,
    /* context of handler function*/
    NULL
};
/* Configure the syspm callback structure, check PDL documentation for more details */
static cy_stc_syspm_callback_t LPCompHibernateCallback =
{
    /*Callback handler function*/
    &Cy_LPComp_HibernateCallback,
    /*Callback type*/
    CY_SYSPM_HIBERNATE,
    /*Mask of modes to be skipped*/
    0u,
    /*Syspm callback parameters*/
    &LPCompHibernateCallbackParams,
    /*Previous list item*/
    NULL,
    /*Next list item*/
    NULL,
    *Callback execution order*/
    0
};

/*******************************************************************************
 * Function Name: main
 *******************************************************************************
 *
 *  The main function performs the following actions:
 *    1. On startup unfreeze the IO (In case of waking up from hibernate)
 *    2. Set the drive mode of the LED pin
 *    3. Set inputs and enable local reference for LPComp
 *    4. Initialize the LPComp to using predefined config structures
 *    5. Blink the LED twice
 *    6. Set LPComp output as the wakeup source from hibernate mode
 *    7. Register callback
 *    8. Transition the system to hibernate mode
 ******************************************************************************/
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
    if(Cy_SysPm_IoIsFrozen())
    {
        /* Restore the I/O configuration */
        Cy_SysPm_IoUnfreeze();
    }
    /* Initialize LED to the strong drive mode */
    Cy_GPIO_SetDrivemode(LED_GREEN_PORT, LED_GREEN_PIN, CY_GPIO_DM_STRONG_IN_OFF);

    /* Set LPComp Inputs*/
    Cy_LPComp_SetInputs(MYLPCOMP_HW, MYLPCOMP_CHANNEL, CY_LPCOMP_SW_GPIO, CY_LPCOMP_SW_LOCAL_VREF);

    /* Initialize LPComp */
    (void)Cy_LPComp_Init(MYLPCOMP_HW, MYLPCOMP_CHANNEL, &MYLPCOMP_config);

    /* Enable local reference for LPComp inputN */
    Cy_LPComp_ConnectULPReference(MYLPCOMP_HW, MYLPCOMP_CHANNEL);
    Cy_LPComp_UlpReferenceEnable(MYLPCOMP_HW);

    /* Power on the comparator */
    Cy_LPComp_SetPower(MYLPCOMP_HW, MYLPCOMP_CHANNEL, MYLPCOMP_config.power);

    /* Blink LED twice */
    for (uint32 i = 1UL; i <= 4UL; i++)
    {
        Cy_GPIO_Inv(LED_GREEN_PORT, LED_GREEN_PIN);
        Cy_SysLib_Delay(200/*msec*/);
    }

    /* Set the Hibernate wake-up source to the LPComp high output */
    Cy_SysPm_SetHibernateWakeupSource(CY_SYSPM_HIBERNATE_LPCOMP0_HIGH);

    /* Register a new syspm callback */
    (void)Cy_SysPm_RegisterCallback(&LPCompHibernateCallback);

    /* Enter into Hibernate mode */
    if (CY_SYSPM_SUCCESS != Cy_SysPm_SystemEnterHibernate())
    {
        /* Insert error handling */
    	CY_ASSERT(0);
    }

    for (;;)
    {
    }
}

/* [] END OF FILE */
