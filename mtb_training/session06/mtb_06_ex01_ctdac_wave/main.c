/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for CTDAC sine-wave generator example.
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
* In this project, the CTDAC has been used to generate a sine wave
*
* (1) 12 bit unsigned sinewave input is stored in a lookup table is transferred using DMA
* (2) The CTDAC voltage reference is internal bandgap reference (1.2V)
* (3) The internal reference is buffered through CTBm block before being supplied to CTDAC
*
*NOTE: Make sure you have replaced the design.modus file as explained in the documentation.
* To observe the outputs, connect pin 9[6] to an oscilloscope and see the waveform.
*
* Connections:
* P9[6] ---> CTDAC output pin

*
* Output: You should see a sine wave with amplitude ~1.2V(Internal Vref voltage)
*
* *********************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/* Lookup table for a sine wave in unsigned format. */
uint32_t sineWaveLUT[] = {0x7FF, 0x880, 0x900, 0x97F, 0x9FC, 0xA78, 0xAF1, 0xB67, 0xBD9, 0xC48,
                         0xCB2, 0xD18, 0xD79, 0xDD4, 0xE29, 0xE77, 0xEC0, 0xF01, 0xF3C, 0xF6F,
                         0xF9A, 0xFBE, 0xFDA, 0xFEE, 0xFFA, 0xFFF, 0xFFA, 0xFEE, 0xFDA, 0xFBE,
                         0xF9A, 0xF6F, 0xF3C, 0xF01, 0xEC0, 0xE77, 0xE29, 0xDD4, 0xD79, 0xD18,
                         0xCB2, 0xC48, 0xBD9, 0xB67, 0xAF1, 0xA78, 0x9FC, 0x97F, 0x900, 0x880,
                         0x7FF, 0x77E, 0x6FE, 0x67F, 0x602, 0x586, 0x50D, 0x497, 0x425, 0x3B6,
                         0x34C, 0x2E6, 0x285, 0x22A, 0x1D5, 0x187, 0x13E, 0x0FD, 0x0C2, 0x08F,
                         0x064, 0x040, 0x024, 0x010, 0x004, 0x000, 0x004, 0x010, 0x024, 0x040,
                         0x064, 0x08F, 0x0C2, 0x0FD, 0x13E, 0x187, 0x1D5, 0x22A, 0x285, 0x2E6,
                         0x34C, 0x3B6, 0x425, 0x497, 0x50D, 0x586, 0x602, 0x67F, 0x6FE, 0x77E};

/*******************************************************************************
 * Function Name: main
 *******************************************************************************
 *
 *  The main function performs the following actions:
 *    1. Initialize CTDAC, CTBm to device configurator configurations.
 *    2. Set DMA source and Destination addresses.
 *    3. Loopback DMA descriptor to itself
 *    4. Enable DMA
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

    /*Start AREF*/
    cy_en_sysanalog_status_t status_aref;
    status_aref = Cy_SysAnalog_Init(&Cy_SysAnalog_Fast_Local);


    /*Start CTDAC*/
    cy_en_ctdac_status_t status_ctdac;
    status_ctdac = Cy_CTDAC_Init(CTDAC0, &pass_0_ctdac_0_config);

    if (CY_CTDAC_SUCCESS == status_ctdac)
    {
        /* Turn on the CTDAC hardware block. */
        Cy_CTDAC_Enable(CTDAC0);
    }

    /*Start the Opamp*/
    cy_en_ctb_status_t status_ctb0;
    status_ctb0 = Cy_CTB_OpampInit (CTBM0, CY_CTB_OPAMP_1, &pass_0_ctb_0_oa_1_config);

    if (CY_CTB_SUCCESS == status_ctb0)
    {
        /* Turn on the CTBM hardware block. */
        Cy_CTB_Enable(CTBM0);
    }


    /*DMA Initialisation and Enable*/
    cy_stc_dma_descriptor_t descriptor;

    if (CY_DMA_SUCCESS != Cy_DMA_Descriptor_Init(&descriptor, &cpuss_0_dw0_0_chan_2_Descriptor_0_config))
    {
        /* Insert error handling */
        CY_ASSERT(0);
    }

    if (CY_DMA_SUCCESS != Cy_DMA_Channel_Init(DW0, cpuss_0_dw0_0_chan_2_CHANNEL, &cpuss_0_dw0_0_chan_2_channelConfig))
    {
        /* Insert error handling */
        CY_ASSERT(0);
    }
    /* Set source address as the LUT */
    Cy_DMA_Descriptor_SetSrcAddress(&descriptor, (uint32_t *) sineWaveLUT);

    /* Set destination address as the CTDAC buffer register */
    Cy_DMA_Descriptor_SetDstAddress(&descriptor, (uint32_t *) &CTDAC0->CTDAC_VAL_NXT);

    /* Loop back the descriptor to itself */
    Cy_DMA_Descriptor_SetNextDescriptor(&descriptor, &descriptor);

    /* Set the descriptor and enable */
    Cy_DMA_Channel_SetDescriptor(DW0, cpuss_0_dw0_0_chan_2_CHANNEL, &descriptor);
    Cy_DMA_Channel_Enable(DW0, cpuss_0_dw0_0_chan_2_CHANNEL);
    Cy_DMA_Enable(DW0);

    for (;;)
    {

    }
}

/* [] END OF FILE */
