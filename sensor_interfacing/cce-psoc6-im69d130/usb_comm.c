/*******************************************************************************
* File Name: usb_comm.c
*
* Description: This file contains the implementation of the USB interface.
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

#include "usb_comm.h"

#include "cy_sysint.h"
#include "cycfg.h"
#include "cycfg_usbdev.h"

#include <stdio.h>
#include <string.h>

/*******************************************************************************
* Constants
*******************************************************************************/
#define USBCOMM_DEVICE_ID     0

/*******************************************************************************
* Local USB Callbacks
*******************************************************************************/
static cy_en_usb_dev_status_t usb_comm_request_received (cy_stc_usb_dev_control_transfer_t *transfer,
                                                         void *classContext,
                                                         cy_stc_usb_dev_context_t *devContext);

static cy_en_usb_dev_status_t usb_comm_request_completed(cy_stc_usb_dev_control_transfer_t *transfer,
                                                         void *classContext,
                                                         cy_stc_usb_dev_context_t *devContext);

static cy_en_usb_dev_status_t usb_comm_set_interface(uint32_t interface,
                                                     uint32_t alternate,
                                                     void *classContext,
                                                     cy_stc_usb_dev_context_t *devContext);

static cy_en_usb_dev_status_t usb_comm_set_configuration(uint32_t config,
                                                         void *classContext,
                                                         cy_stc_usb_dev_context_t *devContext);

/***************************************************************************
* Interrupt configuration
***************************************************************************/
static void usb_high_isr(void);
static void usb_medium_isr(void);
static void usb_low_isr(void);

/*******************************************************************************
* Global Variables
*******************************************************************************/
uint8_t usb_comm_mute;
uint8_t usb_comm_cur_volume[AUDIO_VOLUME_SIZE]; 
uint8_t usb_comm_min_volume[AUDIO_VOLUME_SIZE] = {CY_USB_DEV_AUDIO_VOLUME_MIN_LSB, CY_USB_DEV_AUDIO_VOLUME_MIN_MSB};
uint8_t usb_comm_max_volume[AUDIO_VOLUME_SIZE] = {CY_USB_DEV_AUDIO_VOLUME_MAX_LSB, CY_USB_DEV_AUDIO_VOLUME_MAX_MSB};
uint8_t usb_comm_res_volume[AUDIO_VOLUME_SIZE] = {AUDIO_VOL_RES_LSB, AUDIO_VOL_RES_MSB};

uint8_t usb_comm_ep_map[] = {0U, 0U, 1U};
uint8_t usb_comm_sample_frequency[AUDIO_STREAMING_EPS_NUMBER][AUDIO_SAMPLE_FREQ_SIZE];

volatile uint32_t usb_comm_new_sample_rate = 0;
volatile bool     usb_comm_enable_out_streaming = false;
volatile bool     usb_comm_enable_in_streaming = false;
volatile bool     usb_comm_enable_feedback = false;
volatile bool     usb_comm_clock_configured = false;

static usb_comm_interface_t usb_comm_interface = {
    .disable_in = NULL,
    .disable_out = NULL,
    .enable_in = NULL,
    .enable_out = NULL,
};

/* USB Interrupt Configuration */
const cy_stc_sysint_t usb_high_interrupt_cfg =
{
    .intrSrc = (IRQn_Type) usb_interrupt_hi_IRQn,
    .intrPriority = 5U,
};
const cy_stc_sysint_t usb_medium_interrupt_cfg =
{
    .intrSrc = (IRQn_Type) usb_interrupt_med_IRQn,
    .intrPriority = 6U,
};
const cy_stc_sysint_t usb_low_interrupt_cfg =
{
    .intrSrc = (IRQn_Type) usb_interrupt_lo_IRQn,
    .intrPriority = 7U,
};

/* USBFS Context Variables */
cy_stc_usbfs_dev_drv_context_t  usb_drvContext;
cy_stc_usb_dev_context_t        usb_devContext;
cy_stc_usb_dev_audio_context_t  usb_audioContext;

/*******************************************************************************
* Function Name: usb_comm_init
********************************************************************************
* Summary:
*   Initializes the USBFS hardware block and its interrupts.
*
*******************************************************************************/
void usb_comm_init(void)
{
    /* Start the USB Block */
    Cy_USB_Dev_Init(CYBSP_USBDEV_HW,
                    &CYBSP_USBDEV_config,
                    &usb_drvContext,
                    &usb_devices[USBCOMM_DEVICE_ID],
                    &usb_devConfig,
                    &usb_devContext);

    Cy_USB_Dev_Audio_Init(NULL,
                          &usb_audioContext,
                          &usb_devContext);

    /* Initialize the USB interrupts */
    Cy_SysInt_Init(&usb_high_interrupt_cfg,   &usb_high_isr);
    Cy_SysInt_Init(&usb_medium_interrupt_cfg, &usb_medium_isr);
    Cy_SysInt_Init(&usb_low_interrupt_cfg,    &usb_low_isr);   
}

/*******************************************************************************
* Function Name: usb_comm_connect
********************************************************************************
* Summary:
*   Starts USB enumeration.
*
*******************************************************************************/
void usb_comm_connect(void)
{
    /* Enable the USB interrupts */
    NVIC_EnableIRQ(usb_high_interrupt_cfg.intrSrc);
    NVIC_EnableIRQ(usb_medium_interrupt_cfg.intrSrc);
    NVIC_EnableIRQ(usb_low_interrupt_cfg.intrSrc);

    Cy_USB_Dev_Connect(true, CY_USB_DEV_WAIT_FOREVER, &usb_devContext);
}

/*******************************************************************************
* Function Name: usb_comm_is_ready
********************************************************************************
* Summary:
*   Verifies if the USB is enumerated.
*
*******************************************************************************/
bool usb_comm_is_ready(void)
{
    return (Cy_USB_Dev_GetConfiguration(&usb_devContext));
}

/*******************************************************************************
* Function Name: usb_comm_register_interface
********************************************************************************
* Summary:
*   Registers the functions to be executed on certain events.
*
*******************************************************************************/
void usb_comm_register_interface(usb_comm_interface_t *interface)
{
    usb_comm_interface.disable_in  = interface->disable_in;
    usb_comm_interface.disable_out = interface->disable_out;
    usb_comm_interface.enable_in   = interface->enable_in;
    usb_comm_interface.enable_out  = interface->enable_out;
}

/*******************************************************************************
* Function Name: usb_comm_register_usb_callbacks
********************************************************************************
* Summary:
*   Registers the USBFS callbacks.
*
*******************************************************************************/
void usb_comm_register_usb_callbacks(void)
{
    Cy_USB_Dev_Audio_RegisterUserCallback(usb_comm_request_received, usb_comm_request_completed, &usb_audioContext);
    Cy_USB_Dev_RegisterClassSetConfigCallback(usb_comm_set_configuration, Cy_USB_Dev_Audio_GetClass(&usb_audioContext));
    Cy_USB_Dev_RegisterClassSetInterfaceCallback(usb_comm_set_interface, Cy_USB_Dev_Audio_GetClass(&usb_audioContext));
}

/*******************************************************************************
* Function Name: usb_comm_get_sample_rate
********************************************************************************
* Summary:
*   Returns the current sample rate.
*
* Return:
*   Current sample rate.
*
*******************************************************************************/
uint32_t usb_comm_get_sample_rate(uint32_t endpoint)
{
    uint32_t newFrequency = (((uint32_t) usb_comm_sample_frequency[endpoint][2] << 16) |
                             ((uint32_t) usb_comm_sample_frequency[endpoint][1] << 8)  |
                             ((uint32_t) usb_comm_sample_frequency[endpoint][0]));

    return newFrequency;
}

/*******************************************************************************
* Function Name: usb_comm_request_received
********************************************************************************
* Summary:
*   Callback implementation for the Audio Request Received.
*
*******************************************************************************/
cy_en_usb_dev_status_t usb_comm_request_received(cy_stc_usb_dev_control_transfer_t *transfer,
                                                 void *classContext,
                                                 cy_stc_usb_dev_context_t *devContext)
{
    (void) classContext; (void) devContext;

    cy_en_usb_dev_status_t retStatus = CY_USB_DEV_REQUEST_NOT_HANDLED;

    if (transfer->setup.bmRequestType.type == CY_USB_DEV_CLASS_TYPE)
    {
        /* Feature Unit */
        if (AUDIO_CONTROL_FEATURE_UNIT == transfer->setup.wIndex)
        {
            /* Control selector */
            switch (CY_HI8(transfer->setup.wValue))
            {
                /* Handle the Audio Mute Control Request */
                case CY_USB_DEV_AUDIO_MUTE_CONTROL:
                {
                    /* Select channel number */
                    switch (CY_LO8(transfer->setup.wValue))
                    {
                        case AUDIO_FEATURE_UNIT_MASTER_CHANNEL:
                        {
                            switch (transfer->setup.bRequest)
                            {
                                case CY_USB_DEV_AUDIO_RQST_GET_CUR:
                                {
                                    /* Get current audio mute control */
                                    transfer->ptr       = &usb_comm_mute;
                                    transfer->remaining = sizeof(usb_comm_mute);

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_SET_CUR:
                                {
                                    /* Set audio mute control */
                                    transfer->ptr       = transfer->buffer;
                                    transfer->remaining = sizeof(usb_comm_mute);
                                    transfer->notify    = true;

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;
                            }
                        }
                        break;

                        default:
                            /* Only Master channel is supported */
                        break;
                    }
                }
                break;

                /* Handle the Audio Volume Control Request */
                case CY_USB_DEV_AUDIO_CS_VOLUME_CONTROL:
                {
                    /* Select channel number */
                    switch (CY_LO8(transfer->setup.wValue))
                    {
                        case AUDIO_FEATURE_UNIT_MASTER_CHANNEL:
                        {
                            switch (transfer->setup.bRequest)
                            {
                                case CY_USB_DEV_AUDIO_RQST_GET_CUR:
                                {
                                    /* Get current audio volume */
                                    transfer->ptr       = usb_comm_cur_volume;
                                    transfer->remaining = sizeof(usb_comm_cur_volume);

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_GET_MIN:
                                {
                                    /* Get current minimum volume */
                                    transfer->ptr       = usb_comm_min_volume;
                                    transfer->remaining = sizeof(usb_comm_min_volume);

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_GET_MAX:
                                {
                                    /* Get current maximum volume */
                                    transfer->ptr       = usb_comm_max_volume;
                                    transfer->remaining = sizeof(usb_comm_max_volume);

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_GET_RES:
                                {
                                    /* Get current volume resolution */
                                    transfer->ptr       = usb_comm_res_volume;
                                    transfer->remaining = sizeof(usb_comm_res_volume);

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_SET_CUR:
                                {
                                    /* Set audio volume  */
                                    transfer->remaining = AUDIO_VOLUME_SIZE;
                                    transfer->notify    = true;

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_SET_MIN:
                                {
                                    /* Set mininum volume */
                                    transfer->remaining = AUDIO_VOLUME_SIZE;
                                    transfer->notify    = true;

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_SET_MAX:
                                {
                                    /* Set maximum volume */
                                    transfer->remaining = AUDIO_VOLUME_SIZE;
                                    transfer->notify    = true;

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_SET_RES:
                                {
                                    /* Set volume resolution */
                                    transfer->remaining = AUDIO_VOLUME_SIZE;
                                    transfer->notify    = true;

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                default:
                                break;
                            } /* switch (transfer->setup.bRequest) */
                        }
                        break;

                        default:
                        break;
                    } /* switch (CY_LO8(transfer->setup.wValue)) */
                }
                break;

                default:
                break;
            }   /* switch (CY_HI8(transfer->setup.wValue)) */
        }
        /* Endpoint */
        else if ((AUDIO_STREAMING_OUT_ENDPOINT_ADDR == transfer->setup.wIndex) ||
                 (AUDIO_STREAMING_IN_ENDPOINT_ADDR  == transfer->setup.wIndex))
        {
            /* Switch control selector */
            switch (CY_HI8(transfer->setup.wValue))
            {
                /* Handle audio sampling frequency control */
                case CY_USB_DEV_AUDIO_CS_SAMPLING_FREQ_CTRL:
                {
                    switch (transfer->setup.bRequest)
                    {
                        case CY_USB_DEV_AUDIO_RQST_GET_CUR:
                        {
                            /* Get endpoint number */
                            uint32_t endpoint = usb_comm_ep_map[(transfer->setup.wIndex & 0x0FU)];

                            /* Get current sampling frequency */
                            transfer->ptr       = usb_comm_sample_frequency[endpoint];
                            transfer->remaining = AUDIO_SAMPLE_FREQ_SIZE;

                            retStatus = CY_USB_DEV_SUCCESS;
                        }
                        break;

                        case CY_USB_DEV_AUDIO_RQST_SET_CUR:
                        {
                            /* Set sampling frequency */
                            transfer->remaining = AUDIO_SAMPLE_FREQ_SIZE;
                            transfer->notify    = true;

                            retStatus = CY_USB_DEV_SUCCESS;
                        }
                        break;

                        default:
                        break;
                    } /* switch (transfer->setup.bRequest) */
                }
                break;

                default:
                break;
            } /* switch (CY_HI8(transfer->setup.wValue)) */
        }
        else
        {
            /* Unknown */
        }
    }

    return retStatus;
}

/*******************************************************************************
* Function Name: usb_comm_request_completed
********************************************************************************
* Summary:
*   Callback implementation for the Audio Request Completed.
*
*******************************************************************************/
cy_en_usb_dev_status_t usb_comm_request_completed(cy_stc_usb_dev_control_transfer_t *transfer,
                                                  void *classContext,
                                                  cy_stc_usb_dev_context_t *devContext)
{
    (void) classContext; (void) devContext;

    cy_en_usb_dev_status_t retStatus = CY_USB_DEV_REQUEST_NOT_HANDLED;

    if (transfer->setup.bmRequestType.type == CY_USB_DEV_CLASS_TYPE)
    {
        /* Feature Unit */
        if (AUDIO_CONTROL_FEATURE_UNIT == transfer->setup.wIndex)
        {
            /* Control selector */
            switch (CY_HI8(transfer->setup.wValue))
            {
                case CY_USB_DEV_AUDIO_CS_MUTE_CONTROL:
                {
                    /* Select channel number */
                    switch (CY_LO8(transfer->setup.wValue))
                    {
                        case AUDIO_FEATURE_UNIT_MASTER_CHANNEL:
                        {
                            switch (transfer->setup.bRequest)
                            {
                                case CY_USB_DEV_AUDIO_RQST_SET_CUR:
                                {
                                    /* Update mute control */
                                    memcpy(&usb_comm_mute, transfer->buffer, sizeof(usb_comm_mute));
                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;
                            }
                        }
                        break;

                        default:
                            /* Only Master channel is supported */
                        break;
                    }
                }
                break;

                case CY_USB_DEV_AUDIO_CS_VOLUME_CONTROL:
                {
                    /* Select channel number */
                    switch (CY_LO8(transfer->setup.wValue))
                    {
                        case AUDIO_FEATURE_UNIT_MASTER_CHANNEL:
                        {
                            switch (transfer->setup.bRequest)
                            {
                                case CY_USB_DEV_AUDIO_RQST_SET_CUR:
                                {
                                    /* Update audio volume */
                                    memcpy(usb_comm_cur_volume, transfer->buffer, sizeof(usb_comm_cur_volume));

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_SET_MIN:
                                {
                                    /* Update minimum volume */
                                    memcpy(usb_comm_min_volume, transfer->buffer, sizeof(usb_comm_min_volume));

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_SET_MAX:
                                {
                                    /* Update maximum volume */
                                    memcpy(usb_comm_max_volume, transfer->buffer, sizeof(usb_comm_max_volume));

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                case CY_USB_DEV_AUDIO_RQST_SET_RES:
                                {
                                    /* Update resolution volume */
                                    memcpy(usb_comm_res_volume, transfer->buffer, sizeof(usb_comm_res_volume));

                                    retStatus = CY_USB_DEV_SUCCESS;
                                }
                                break;

                                default:
                                break;
                            } /* switch (transfer->setup.bRequest) */
                        }
                        break;

                        default:
                        break;
                    } /* switch (CY_LO8(transfer->setup.wValue)) */
                }
                break;

                default:
                break;
            } /* switch (CY_HI8(transfer->setup.wValue)) */
        }
        else if ((AUDIO_STREAMING_OUT_ENDPOINT_ADDR == transfer->setup.wIndex) ||
                 (AUDIO_STREAMING_IN_ENDPOINT_ADDR  == transfer->setup.wIndex))
        {
            /* Switch control selector */
            switch (CY_HI8(transfer->setup.wValue))
            {
                case CY_USB_DEV_AUDIO_CS_SAMPLING_FREQ_CTRL:
                {
                    /* Get endpoint number */
                    uint32_t endpoint = usb_comm_ep_map[(transfer->setup.wIndex & 0x0FU)];

                    switch (transfer->setup.bRequest)
                    {
                        case CY_USB_DEV_AUDIO_RQST_SET_CUR:
                        {
                            memcpy(usb_comm_sample_frequency[endpoint], transfer->ptr, AUDIO_SAMPLE_FREQ_SIZE);

                            /* Configure feedback endpoint data */
                            usb_comm_new_sample_rate = usb_comm_get_sample_rate(endpoint);

                            retStatus = CY_USB_DEV_SUCCESS;
                        }
                        break;

                        default:
                        break;
                    }
                }
                break;

                default:
                break;
            } /* switch (CY_HI8(transfer->setup.wValue)) */
        }
        else
        {
            /* Unknown */
        }
    }

    return retStatus;
}

/*******************************************************************************
* Function Name: usb_comm_set_configuration
********************************************************************************
* Summary:
*   Callback implementation for the Audio Set Configuration.
*
*******************************************************************************/
cy_en_usb_dev_status_t usb_comm_set_configuration(uint32_t config,
                                                  void *classContext,
                                                  cy_stc_usb_dev_context_t *devContext)
{
    (void) classContext;
    (void) devContext;
    (void) config;

    return CY_USB_DEV_SUCCESS;
}

/*******************************************************************************
* Function Name: usb_comm_set_interface
********************************************************************************
* Summary:
*   Callback implementation for the Audio Set Interface.
*
*******************************************************************************/
cy_en_usb_dev_status_t usb_comm_set_interface(uint32_t interface,
                                              uint32_t alternate,
                                              void *classContext,
                                              cy_stc_usb_dev_context_t *devContext)
{
    (void) classContext;
    (void) devContext;

    if (AUDIO_STREAMING_OUT_INTERFACE == interface)
    {
        /* Check interface OUT Streaming alternate */
        usb_comm_enable_out_streaming = (AUDIO_STREAMING_OUT_ALTERNATE == alternate);

        if (usb_comm_enable_out_streaming)
        {
            if (NULL != usb_comm_interface.enable_out)
            {
                usb_comm_interface.enable_out();
            }
        }
        else
        {
            if (NULL != usb_comm_interface.disable_out)
            {
                usb_comm_interface.disable_out();
            }
        }
    }

    if (AUDIO_STREAMING_IN_INTERFACE == interface)
    {
        /* Check interface IN Streaming alternate */
        usb_comm_enable_in_streaming = (AUDIO_STREAMING_IN_ALTERNATE == alternate);

        if (usb_comm_enable_in_streaming)
        {
            if (NULL != usb_comm_interface.enable_in)
            {
                usb_comm_interface.enable_in();
            }
        }
        else
        {
            if (NULL != usb_comm_interface.disable_in)
            {
                usb_comm_interface.disable_in();
            }
        }
    }

    return CY_USB_DEV_SUCCESS;
}

/***************************************************************************
* Function Name: usb_high_isr
********************************************************************************
* Summary:
*  This function processes the high priority USB interrupts.
*
***************************************************************************/
static void usb_high_isr(void)
{
    /* Call interrupt processing */
    Cy_USBFS_Dev_Drv_Interrupt(CYBSP_USBDEV_HW, 
                               Cy_USBFS_Dev_Drv_GetInterruptCauseHi(CYBSP_USBDEV_HW), 
                               &usb_drvContext);
}

/***************************************************************************
* Function Name: usb_medium_isr
********************************************************************************
* Summary:
*  This function processes the medium priority USB interrupts.
*
***************************************************************************/
static void usb_medium_isr(void)
{
    /* Call interrupt processing */
    Cy_USBFS_Dev_Drv_Interrupt(CYBSP_USBDEV_HW, 
                               Cy_USBFS_Dev_Drv_GetInterruptCauseMed(CYBSP_USBDEV_HW), 
                               &usb_drvContext);
}

/***************************************************************************
* Function Name: usb_low_isr
********************************************************************************
* Summary:
*  This function processes the low priority USB interrupts.
*
**************************************************************************/
static void usb_low_isr(void)
{
    /* Call interrupt processing */
    Cy_USBFS_Dev_Drv_Interrupt(CYBSP_USBDEV_HW, 
                               Cy_USBFS_Dev_Drv_GetInterruptCauseLo(CYBSP_USBDEV_HW), 
                               &usb_drvContext);
}

/* [] END OF FILE */
