/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for ADC UART example code.
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
* In this project,ADC is setup and input signal is sampled. The result is send through UART interface.
*
* (1) ADC is setup using default configurations
* (2) Channel 0 is configured with p10[0] as input
* (3) UART is set up using retarget_io library
* (4) ADC callback function is setup and end of scan event is also enabled
* (5) IIR filter is implemented in callback function to demonstrate post-processing
*
* Give input signal to pin 10[0]. Set UART baud rate to 115200 in terminal to see the result
*
*
* Connections:
* P10[0] ---> Input Signal to ADC

*
* Output: In terminal you should be able to see filtered ADC counts.
*
**********************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*Baudrate for UART*/
#define BAUD_RATE       (115200)
/*Wait time between samples in milliseconds*/
#define WAIT_TIME        (100)

/* IIR filter coefficients. Sum of the two coefficients (A & B) should be a power of 2.
 * IIR shift co-efficient should be LOG(COEFF_A + COEFF_B) base 2 */
#define IIR_COEFF_A    (32)
#define IIR_COEFF_B    (32)
#define IIR_SHIFT      (6)

/* Flag to indicate filtered data is available */
uint8_t adc_eos_flag=0;
/* variable to store Filter result */
int32_t filtered_result=0;
/* variable to store ADC conversion result */
int32_t adc_out;

/*******************************************************************************
 * Function Name: adc_iir_filter_callback
 *******************************************************************************
 *
 *  The adc_iir_filter_callback function performs the following actions:
 *   1. IIR fitler implementation to demonstrate post processing of ADC result
 *   2. Sets flag to print the result
 ******************************************************************************/
/* IIR fitler implementation to demonstrate post processing of ADC result  */
static void adc_iir_filter_callback(void* arg, cyhal_adc_event_t event)
{
    if(0u != (event & CYHAL_ADC_EOS))
    {
        /*Post-process result*/
        filtered_result = (adc_out * IIR_COEFF_A + filtered_result*IIR_COEFF_B) >> IIR_SHIFT;
        /*set the flag to print the result*/
        adc_eos_flag=1;
    }
}
/*******************************************************************************
 * Function Name: main
 *******************************************************************************
 *
 *  The main function performs the following actions:
 *   1. Initialize retarget-io to use degbug UART
 *   2. Initialize ADC and ADC channel
 *   3. Register callback function
 *   4. Subscribe to end of scan event
 *   5. Initiate repeated conversion with a delay
 *   6. Print the filtered result
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
    /* Enable global interrupts */
    __enable_irq();

    /*Initialize retarget-io to use debug UART port*/
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,BAUD_RATE);

    cyhal_adc_t adc_obj;
    cyhal_adc_channel_t adc_chan_0_obj;

    /* Initialize ADC. The ADC block which can connect to pin 10[0] is selected */
    result = cyhal_adc_init(&adc_obj, P10_0, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    /* Initialize ADC channel, allocate channel number 0 to pin 10[0] as this is the first channel initialized */
    const cyhal_adc_channel_config_t channel_config = { .enable_averaging = false, .min_acquisition_ns = 220, .enabled = true };
    result = cyhal_adc_channel_init_diff(&adc_chan_0_obj, &adc_obj, P10_0, CYHAL_ADC_VNEG, &channel_config);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    /* Register a callback  */
     cyhal_adc_register_callback(&adc_obj, &adc_iir_filter_callback, NULL);

    /* Subscribe to the end of scan event so that we we can process the results as each scan completes*/
    cyhal_adc_enable_event(&adc_obj, CYHAL_ADC_EOS, CYHAL_ISR_PRIORITY_DEFAULT, true);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    for (;;)
    {
        /*Check EOS triggered enable flag to update filtered data*/
        if(adc_eos_flag==1)
        {
            adc_eos_flag=0;
            /*Print Filtered result*/
            printf("ADC_Count = %ld\n\r",filtered_result);
        }
        /*Insert delay between sampling*/
        Cy_SysLib_Delay(WAIT_TIME);
        /*Read ADC conversion result for channel number 0*/
        adc_out = cyhal_adc_read(&adc_chan_0_obj);
    }
}

/* [] END OF FILE */
