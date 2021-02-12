/*****************************************************************************
* File Name: usb_comm.h
*
* Description: This file contains the function prototypes and constants used in
*  usb_comm.c.
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

#ifndef USB_COMM_H
#define USB_COMM_H

#include <stdint.h>
#include <stdbool.h>

#include "cy_usb_dev.h"
#include "cy_usb_dev_audio.h"
#include "cy_usb_dev_audio_descr.h"

#include "audio.h"

/*******************************************************************************
* USB Communication Strucutres
*******************************************************************************/
typedef void (* usb_comm_interface_function_t)(void);

typedef struct
{
    usb_comm_interface_function_t enable_out;
    usb_comm_interface_function_t enable_in;
    usb_comm_interface_function_t disable_out;
    usb_comm_interface_function_t disable_in;
} usb_comm_interface_t;

/*******************************************************************************
* USB Communication Extern Global Variables
*******************************************************************************/
extern uint8_t usb_comm_mute;
extern uint8_t usb_comm_cur_volume[];
extern uint8_t usb_comm_min_volume[];
extern uint8_t usb_comm_max_volume[];
extern uint8_t usb_comm_res_volume[];

extern volatile uint32_t usb_comm_new_sample_rate;
extern volatile bool     usb_comm_enable_out_streaming;
extern volatile bool     usb_comm_enable_in_streaming;
extern volatile bool     usb_comm_out_streaming_start;
extern volatile bool     usb_comm_in_streaming_start;
extern volatile bool     usb_comm_out_streaming_stop;
extern volatile bool     usb_comm_in_streaming_stop;
extern volatile bool     usb_comm_enable_feedback;
extern volatile bool     usb_comm_clock_configured;

/* USBFS Context structures */
extern cy_stc_usbfs_dev_drv_context_t  usb_drvContext;
extern cy_stc_usb_dev_context_t        usb_devContext;
extern cy_stc_usb_dev_audio_context_t  usb_audioContext;

/*******************************************************************************
* USB Communication Functions
*******************************************************************************/
void     usb_comm_init(void);
void     usb_comm_connect(void);
bool     usb_comm_is_ready(void);
void     usb_comm_register_interface(usb_comm_interface_t *interface);
void     usb_comm_register_usb_callbacks(void);
uint32_t usb_comm_get_sample_rate(uint32_t endpoint);

#endif /* USB_COMM_H */

/* [] END OF FILE */
