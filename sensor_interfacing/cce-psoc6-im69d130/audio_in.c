/*******************************************************************************
* File Name: audio_in.c
*
*  Description: This file contains the Audio In path configuration and
*               processing code
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

#include "audio_in.h"
#include "audio.h"
#include "usb_comm.h"
#include "cy_retarget_io.h"

#include "cyhal.h"
#include "cycfg.h"
#include "cy_sysint.h"

/*******************************************************************************
* Local Constants
*******************************************************************************/
/* PDM/PCM Pins */
#define PDM_DATA                    P10_5
#define PDM_CLK                     P10_4

/* Decimation Rate of the PDM/PCM block */
#define DECIMATION_RATE             64u


/*******************************************************************************
* Local Functions
*******************************************************************************/
void audio_in_endpoint_callback(USBFS_Type *base, 
                                uint32_t endpoint,
                                uint32_t error_type, 
                                cy_stc_usbfs_dev_drv_context_t *context);

void convert_32_to_24_array(uint8_t *src, uint8_t *dst, uint32_t length);


/*******************************************************************************
* Audio In Variables
*******************************************************************************/
/* USB IN buffer data for Audio IN endpoint */
CY_USB_DEV_ALLOC_ENDPOINT_BUFFER(audio_in_usb_buffer, AUDIO_IN_ENDPOINT_SIZE + 1);

/* PCM buffer data (32-bits) */
uint8_t audio_in_pcm_buffer[4 * AUDIO_IN_ENDPOINT_SIZE/AUDIO_SAMPLE_DATA_SIZE];

/* Audio IN flags */
volatile bool audio_in_start_recording = false;
volatile bool audio_in_is_recording    = false;

/* Size of the frame */
volatile uint32_t audio_in_frame_size = AUDIO_FRAME_DATA_SIZE;

/* HAL object */
cyhal_pdm_pcm_t pdm_pcm;

/* HAL Config */
const cyhal_pdm_pcm_cfg_t pdm_pcm_cfg = 
{
    .sample_rate     = AUDIO_SAMPLING_RATE_48KHZ,
    .decimation_rate = DECIMATION_RATE,
    .mode            = CYHAL_PDM_PCM_MODE_STEREO,
    .word_length     = 24,  /* bits */
    .left_gain       = 0,   /* dB */
    .right_gain      = 0,   /* dB */
};

/*******************************************************************************
* Function Name: audio_in_init
********************************************************************************
* Summary:
*   Initialize the audio IN endpoint and PDM/PCM block.
*
*******************************************************************************/
void audio_in_init(void)
{
    /* Register Data Endpoint Callbacks */
    Cy_USBFS_Dev_Drv_RegisterEndpointCallback(CYBSP_USBDEV_HW, 
                                              AUDIO_STREAMING_IN_ENDPOINT, 
                                              audio_in_endpoint_callback, 
                                              &usb_drvContext);

    /* Initialize the PDM PCM block */
    cyhal_pdm_pcm_init(&pdm_pcm, PDM_DATA, PDM_CLK, NULL, &pdm_pcm_cfg);
}

/*******************************************************************************
* Function Name: audio_in_enable
********************************************************************************
* Summary:
*   Starts a recording session.
*
*******************************************************************************/
void audio_in_enable(void)
{
    audio_in_start_recording  = true;
}

/*******************************************************************************
* Function Name: audio_in_disable
********************************************************************************
* Summary:
*   Stops a recording session.
*
*******************************************************************************/
void audio_in_disable(void)
{
    audio_in_is_recording = false;
}

/*******************************************************************************
* Function Name: audio_in_process
********************************************************************************
* Summary:
*   Main task for the audio in endpoint. Periodically feeds the USB Audio IN
*   endpoint.
*
*******************************************************************************/
void audio_in_process(void)
{
    if (audio_in_start_recording)
    {
        audio_in_start_recording = false;
        audio_in_is_recording = true;

        /* Clear Audio In buffer */
        memset(audio_in_usb_buffer, 0, AUDIO_IN_ENDPOINT_SIZE);

        /* Clear PDM/PCM RX FIFO */
        cyhal_pdm_pcm_clear(&pdm_pcm);

        cyhal_pdm_pcm_start(&pdm_pcm);

        /* Start a transfer to the Audio IN endpoint */
        Cy_USB_Dev_WriteEpNonBlocking(AUDIO_STREAMING_IN_ENDPOINT,
                                      (uint8_t *) audio_in_usb_buffer,
                                      AUDIO_SAMPLE_DATA_SIZE * AUDIO_FRAME_DATA_SIZE,
                                      &usb_devContext);
    }
}

/*******************************************************************************
* Function Name: audio_in_endpoint_callback
********************************************************************************
* Summary:
*   Audio in endpoint callback implementation. It enables the Audio in DMA to
*   stream an audio frame.
*
* Parameters:
* base - The pointer to the USBFS instance.
* endpoint - Endpoint address
* error_type - Error type
* context - The pointer to the context structure allocated by user
*
*******************************************************************************/
void audio_in_endpoint_callback(USBFS_Type *base, 
                                uint32_t endpoint,
                                uint32_t error_type, 
                                cy_stc_usbfs_dev_drv_context_t *context)
{
    /* Set the count equal to the frame size */
    size_t audio_in_count = audio_in_frame_size;

    (void) error_type;
    (void) endpoint,
    (void) context;
    (void) base;

    /* Read all the data in the PDM/PCM buffer */
    cyhal_pdm_pcm_read(&pdm_pcm, (void *) audio_in_pcm_buffer, &audio_in_count);

    /* Limit the size to avoid overflow in the pcm buffer */
    if (audio_in_count > AUDIO_MAX_DATA_SIZE)
    {
        audio_in_count = AUDIO_MAX_DATA_SIZE;
    }

    /* Convert the PDM data array (32-bit) to USB data array (24-bit) */
    convert_32_to_24_array(audio_in_pcm_buffer, audio_in_usb_buffer, audio_in_count);

    /* Check if should keep recording */
    if (audio_in_is_recording == true)
    {
        Cy_USB_Dev_WriteEpNonBlocking(AUDIO_STREAMING_IN_ENDPOINT,
                                      (uint8_t *) audio_in_usb_buffer,
                                      audio_in_count*AUDIO_SAMPLE_DATA_SIZE,
                                      &usb_devContext);
    }
}


/*******************************************************************************
* Function Name: convert_32_to_24_array
********************************************************************************
* Summary:
*   Convert a 32-bit array to 24-bit array.
* 
* Parameters:
* src - Pointer to the source PDM - PCM buffer
* dst - Pointer to the destination USB buffer
* length - Length of the packet
*
*******************************************************************************/
void convert_32_to_24_array(uint8_t *src, uint8_t *dst, uint32_t length)
{
    while (0u != length--)
    {
        *(dst++) = *src++;
        *(dst++) = *src++;
        *(dst++) = *src++;
        src++;
    }
}

/* [] END OF FILE */
