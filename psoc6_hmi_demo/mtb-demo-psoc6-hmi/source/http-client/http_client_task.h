/******************************************************************************
* File Name:   http_client_task.h
*
* Description: This file is the public interface of http_client_task.c
*
* Related Document: See README.md
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
/*******************************************************************************
 *  Include guard
 ******************************************************************************/
#ifndef HTTP_CLIENT_TASK_H_
#define HTTP_CLIENT_TASK_H_

/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cy_http_client_api.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define CITY_NAME_LEN                                  (15u)
#define FORECAST_NO_OF_DAYS                            (4u)

/*******************************************************************************
* Typedefs
********************************************************************************/
/* Data type for weather icons. */
typedef enum {
    THUNDERSTORM,
    RAIN,
    SNOW,
    FOG,
    CLEAR,
    OVERCAST,
    SCATTERED_CLOUDS,
    MAX_WEATHER_TEXT
} icon_t;

/* Data type for weather. */
typedef struct {
    char temp[5];
    icon_t icon;
} weather_data_t;

/* Data type for HTTP client task commands. */
typedef enum {
    GET_CMD,
} http_cmd_t;

/* Data type for HTTP GET command. */
typedef enum {
    GET_LOCATION,
    GET_TIME,
    GET_WEATHER,
}http_get_t;

/*******************************************************************************
 * Extern variables
 ******************************************************************************/
extern weather_data_t weather_info[];
extern const char rtc_days_in_month_table[];
extern QueueHandle_t http_client_cmd_q;
extern TaskHandle_t http_task_handle;
extern char city_info[CITY_NAME_LEN+1];
extern bool is_http_data_sync;
extern cy_http_client_t handle;

extern TaskHandle_t tft_task_handle;
//extern struct tm current_time, prev_time, default_time;
//extern cyhal_rtc_t rtc_obj;
extern bool display_time_update;
extern bool display_loc_weather_update;
extern bool display_time;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void http_client_task(void* pvParameters);

#endif /* HTTP_CLIENT_TASK_H_ */

/* [] END OF FILE */
