/*****************************************************************************
* File Name: audio.h
*
* Description: This file contains the constants mapped to the USB descriptor.
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

#ifndef AUDIO_H
#define AUDIO_H

/*******************************************************************************
* Constants from USB Audio Descriptor
*******************************************************************************/
#define AUDIO_OUT_ENDPOINT_SIZE         (294U)  /* In bytes */
#define AUDIO_IN_ENDPOINT_SIZE          (294U)  /* In bytes */
#define AUDIO_FEEDBACK_ENDPOINT_SIZE    (3U)    /* In bytes */

#define AUDIO_FRAME_DATA_SIZE           (96u)   /* In words */
#define AUDIO_DELTA_VALUE               (2u)    /* In words */
#define AUDIO_MAX_DATA_SIZE             (AUDIO_FRAME_DATA_SIZE + AUDIO_DELTA_VALUE)

#define AUDIO_CONTROL_INTERFACE         (0x00U)
#define AUDIO_CONTROL_IN_ENDPOINT       (6U)
#define AUDIO_CONTROL_FEATURE_UNIT_IDX  (0x02U)
#define AUDIO_CONTROL_FEATURE_UNIT      ((AUDIO_CONTROL_FEATURE_UNIT_IDX << 8U) | (AUDIO_CONTROL_INTERFACE))

#define AUDIO_STREAMING_OUT_INTERFACE   (1U)
#define AUDIO_STREAMING_OUT_ALTERNATE   (1U)
#define AUDIO_STREAMING_IN_INTERFACE    (2U)
#define AUDIO_STREAMING_IN_ALTERNATE    (1U)

#define AUDIO_STREAMING_OUT_ENDPOINT    (1U)
#define AUDIO_STREAMING_IN_ENDPOINT     (2U)
#define AUDIO_FEEDBACK_IN_ENDPOINT      (3U)

#define AUDIO_STREAMING_OUT_ENDPOINT_ADDR   (0x01U)
#define AUDIO_STREAMING_IN_ENDPOINT_ADDR    (0x82U)
#define AUDIO_FEEDBACK_IN_ENDPOINT_ADDR     (0x83U)

#define AUDIO_STREAMING_EPS_NUMBER          (0x2U)
#define AUDIO_SAMPLE_FREQ_SIZE              (3U)    /* In bytes */
#define AUDIO_SAMPLE_DATA_SIZE              (3U)    /* In bytes */

#define AUDIO_FEATURE_UNIT_MASTER_CHANNEL   (0U)

#define AUDIO_VOLUME_SIZE                   (2U)    /* In bytes */

#define AUDIO_VOL_RES_MSB           (0x00u)         /* Volume Resolution */
#define AUDIO_VOL_RES_LSB           (0x01u)

#define AUDIO_SAMPLING_RATE_48KHZ   (48000U)
#define AUDIO_SAMPLING_RATE_44KHZ   (44100U)
#define AUDIO_SAMPLING_RATE_32KHZ   (32000U)
#define AUDIO_SAMPLING_RATE_22KHZ   (22050U)
#define AUDIO_SAMPLING_RATE_16KHZ   (16000U)

#endif /* AUDIO_H */

/* [] END OF FILE */
