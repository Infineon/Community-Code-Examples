/*******************************************************************************
 * File Name: http_client_task.c
 *
 * Description: This file contains functions to implement a HTTP client to fetch
 * current date & time, weather forecast and location information from
 * a HTTP server.
 *
 * Related Document: See README.md
 *
 *******************************************************************************
 * Copyright (2020), Cypress Semiconductor Corporation.
 *******************************************************************************
 * This software, including source code, documentation and related materials
 * (“Software”), is owned by Cypress Semiconductor Corporation or one of its
 * subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
 * protection (United States and foreign), United States copyright laws and
 * international treaty provisions. Therefore, you may use this Software only
 * as provided in the license agreement accompanying the software package from
 * which you obtained this Software (“EULA”).
 *
 * If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
 * non-transferable license to copy, modify, and compile the Software source
 * code solely for use in connection with Cypress’s integrated circuit products.
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
 * significant property damage, injury or death (“High Risk Product”). By
 * including Cypress’s product in a High Risk Product, the manufacturer of such
 * system or application assumes all risk of such use and in doing so agrees to
 * indemnify Cypress against all liability.
 *****************************************​*************************************/

/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"

#include "cy_wcm.h"
#include "cy_lwip.h"

/* Middleware libraries */
#include "cy_json_parser.h"

/* Task header files */
#include "http_client_task.h"
#include "cy_http_client_api.h"

/* emWin include (for display) */
#include "DIALOG.h"


/* Standard C header files */
#include <stdlib.h>



#include "wifi_config.h"
#include "tft_task.h"
#include "icons.h"

/* Standard C header files */
#include <inttypes.h>

/******************************************************************************
* Macros
******************************************************************************/
/* Maximum number of connection retries to the HTTP server. */
#define MAX_HTTP_SERVER_CONN_RETRIES        (5u)

#define GET_GEO_LOC_URL                    "ip-api.com"
#define GET_GEO_LOC_PATH                   "/json/?fields=464"
#define GET_GEO_LOC_PORT                   ( 80 )
#define GET_TIME_URL                       "api.timezonedb.com"
#define GET_TIME_URL_PATH                   "/v2.1/get-time-zone?key=DAT40YY89DUY&format=json&by=zone&zone="
#define GET_TIME_PORT                         ( 80 )
#define GET_WEATHER_URL                    "api.weatherbit.io"
#define GET_WEATHER_URL_PATH               "/v2.0/forecast/daily?days=4&key=700bcd4bfa8b4aada5ca887866020c48&lat="
#define GET_WEATHER_PORT                   ( 80 )
#define TRANSPORT_SEND_RECV_TIMEOUT_MS     ( 5000 )
#define USER_BUFFER_LENGTH                 ( 5120 )

/* RTC related macros. */
#define RTC_DAYS_PER_WEEK                  (7u)
#define RTC_MONTHS_PER_YEAR                (12uL)
#define RTC_HOURS_PER_DAY                  (24uL)
#define RTC_HOURS_PER_HALF_DAY             (12uL)
#define RTC_SECONDS_PER_MINUTE             (60uL)
#define RTC_SECONDS_PER_HOUR               (3600uL)
#define RTC_SECONDS_PER_DAY                (24uL * 3600uL)
#define RTC_SECONDS_PER_LEAP_YEAR          (366uL * 24uL * 3600uL)
#define RTC_SECONDS_PER_NONLEAP_YEAR       (365uL * 24uL * 3600uL)
#define RTC_UNIX_TIME_PM                   ((12uL * 3600uL) + 1uL)

#define RTOS_TICKS_TO_WAIT                 (50u)
/* Unix time begins in 1970 year */
#define RTC_YEAR_0                         (1970u)

/* DST Month setting constants */
#define RTC__JAN 1
#define RTC__FEB 2
#define RTC__MAR 3
#define RTC__APR 4
#define RTC__MAY 5
#define RTC__JUN 6
#define RTC__JUL 7
#define RTC__AUG 8
#define RTC__SEP 9
#define RTC__OCT 10
#define RTC__NOV 11
#define RTC__DEC 12

#define RTC_JANUARY    (RTC__JAN)    /**< Sequential number of January in the year */
#define RTC_FEBRUARY   (RTC__FEB)    /**< Sequential number of February in the year */
#define RTC_MARCH      (RTC__MAR)    /**< Sequential number of March in the year */
#define RTC_APRIL      (RTC__APR)    /**< Sequential number of April in the year */
#define RTC_MAY        (RTC__MAY)    /**< Sequential number of May in the year */
#define RTC_JUNE       (RTC__JUN)    /**< Sequential number of June in the year */
#define RTC_JULY       (RTC__JUL)    /**< Sequential number of July in the year */
#define RTC_AUGUST     (RTC__AUG)    /**< Sequential number of August in the year */
#define RTC_SEPTEMBER  (RTC__SEP)    /**< Sequential number of September in the year */
#define RTC_OCTOBER    (RTC__OCT)    /**< Sequential number of October in the year */
#define RTC_NOVEMBER   (RTC__NOV)    /**< Sequential number of November in the year */
#define RTC_DECEMBER   (RTC__DEC)    /**< Sequential number of December in the year */

#define RTC_DAYS_IN_JANUARY     (31u)    /**< Number of days in January  */
#define RTC_DAYS_IN_FEBRUARY    (28u)    /**< Number of days in February */
#define RTC_DAYS_IN_MARCH       (31u)    /**< Number of days in March */
#define RTC_DAYS_IN_APRIL       (30u)    /**< Number of days in April */
#define RTC_DAYS_IN_MAY         (31u)    /**< Number of days in May */
#define RTC_DAYS_IN_JUNE        (30u)    /**< Number of days in June */
#define RTC_DAYS_IN_JULY        (31u)    /**< Number of days in July */
#define RTC_DAYS_IN_AUGUST      (31u)    /**< Number of days in August */
#define RTC_DAYS_IN_SEPTEMBER   (30u)    /**< Number of days in September */
#define RTC_DAYS_IN_OCTOBER     (31u)    /**< Number of days in October */
#define RTC_DAYS_IN_NOVEMBER    (30u)    /**< Number of days in November */
#define RTC_DAYS_IN_DECEMBER    (31u)    /**< Number of days in December */

#define RTC_12_HOURS_FORMAT    (0) /**< The 24 hour format */
#define RTC_24_HOURS_FORMAT    (1) /**< The 12 hour (AM/PM) format */

#define TEXT_BUFFER_SIZE                (100u)

/* Structure tm stores years since 1900 */
#define TM_YEAR_BASE                    (1900u)
/******************************************************************************
* Global Variables
*******************************************************************************/
/* Handle of the queue holding the HTTP commands. */
QueueHandle_t http_client_cmd_q;

/* TCP client socket handle */
cy_socket_t client_handle;

cy_http_client_t handle;

bool is_http_data_sync;

/* Variable to store time zone and location coordinates. */
char timezone[20], lat[10], lon[10];

/* Variable to store GET commands */
char get_weather_string[100] = GET_WEATHER_URL, get_time_string[100] = GET_TIME_URL;

const char rtc_days_in_month_table[RTC_MONTHS_PER_YEAR] = {RTC_DAYS_IN_JANUARY,
                                                      RTC_DAYS_IN_FEBRUARY,
                                                      RTC_DAYS_IN_MARCH,
                                                      RTC_DAYS_IN_APRIL,
                                                      RTC_DAYS_IN_MAY,
                                                      RTC_DAYS_IN_JUNE,
                                                      RTC_DAYS_IN_JULY,
                                                      RTC_DAYS_IN_AUGUST,
                                                      RTC_DAYS_IN_SEPTEMBER,
                                                      RTC_DAYS_IN_OCTOBER,
                                                      RTC_DAYS_IN_NOVEMBER,
                                                      RTC_DAYS_IN_DECEMBER};

weather_data_t weather_info[FORECAST_NO_OF_DAYS];
char city_info[CITY_NAME_LEN+1] = "Bengaluru";

/* Buffer to send and receive over TCP sockets */
static uint8_t* user_buffer;

SemaphoreHandle_t sema_http;

bool is_wifi_connected;

/* RTC related variables */
cyhal_rtc_t rtc_obj;
cyhal_alarm_active_t active_alarm;
struct tm current_time, prev_time;
struct tm alarm_time;

/* Default time. */
struct tm default_time = {
        .tm_sec = 0,
        .tm_min = 0,
        .tm_hour = 0,
        .tm_mday = 1,
        .tm_mon = 0,
        .tm_year = 2000 - TM_YEAR_BASE,
        .tm_wday = 6,
        .tm_yday = 1,
        .tm_isdst = 0
};

const char months[12][4] = { "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
};

const char *weather_text[MAX_WEATHER_TEXT] = {
        "Thunderstorm",
        "Rain",
        "Snow",
        "Fog",
        "Clear sky",
        "Overcast",
        "Partly cloudy"
};

/* Variable to check if displayed time needs an update. */
bool display_time_update = 0;

/* Variable to check if loc and weather forecast needs an update. */
bool display_loc_weather_update = 0;

bool display_time = true;


/******************************************************************************
* Function Prototypes
*******************************************************************************/
static void unix_to_date_time(struct tm* date_time, uint64 unix_time, uint32_t time_format);
static cy_rslt_t json_parser_location_cb(cy_JSON_object_t* json_object, void *arg);
static cy_rslt_t json_parser_time_cb(cy_JSON_object_t* json_object, void *arg);
static cy_rslt_t json_parser_weather_cb(cy_JSON_object_t* json_object, void *arg);
static cy_rslt_t send_http_get_request(http_get_t get_type);
static cy_rslt_t connect_to_wifi_ap(void);
static void rtc_cb(void *callback_arg, cyhal_rtc_event_t event);
void update_display_loc_weather();

/******************************************************************************
 * Function Name: http_client_task
 ******************************************************************************
 * Summary:
 *  Task for managing HTTP client connection to fetch location, time, and
 *  weather information from HTTP server.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void http_client_task(void *pvParameters)
{
    BaseType_t rtos_api_result;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    http_cmd_t http_client_cmd;

    char text[TEXT_BUFFER_SIZE];

    cy_wcm_config_t wifi_config = { .interface = WIFI_INTERFACE_TYPE };

    /* To avoid compiler warnings */
    (void)pvParameters;

    http_client_cmd_q = xQueueCreate(1, sizeof(http_cmd_t));

    /* Initialize Wi-Fi connection manager. */
    result = cy_wcm_init(&wifi_config);

	if (result != CY_RSLT_SUCCESS)
	{
		printf("Wi-Fi Connection Manager initialization failed! Error code: 0x%08"PRIx32"\n", (uint32_t)result);
		CY_ASSERT(0);
	}
	printf("Wi-Fi Connection Manager initialized.\r\n");

	/* Connect to Wi-Fi AP */
	result = connect_to_wifi_ap();
	if(result!= CY_RSLT_SUCCESS )
	{
		printf("\n Failed to connect to Wi-Fi AP! Error code: 0x%08"PRIx32"\n", (uint32_t)result);
		CY_ASSERT(0);
	}
	else
	{
		is_wifi_connected = true;
		IMAGE_SetBitmap(wifi_status_handle, &bmwifi_conn);
		printf("Wi-Fi Connected\n");
	}

    /* Initialize the HTTP Client Library. */
    result = cy_http_client_init();
    if( result != CY_RSLT_SUCCESS )
    {
        printf("Failed to init HTTP client library\n");
        CY_ASSERT(0);
    }

    /* Initialize the default time and alarm time. */
    	current_time = default_time;
    	alarm_time.tm_sec = 0;
    	alarm_time.tm_mday = 1;

    	/* Initialize RTC. */
    	result = cyhal_rtc_init(&rtc_obj);
    	if(result != CY_RSLT_SUCCESS)
    	{
    		printf("Failed to initialize RTC!\n");
    	}

    	/* Set the default time to RTC. */
    	cyhal_rtc_write(&rtc_obj, &default_time);

    	/* Enable match of seconds to trigger RTC alarm callback function and
    	 * set the RTC alarm.
    	 */
    	active_alarm.en_sec = 1;
    	cyhal_rtc_set_alarm(&rtc_obj, &alarm_time, active_alarm);

    	/* Register and enable RTC alarm callback. The callback is invoked
    	 * every minute */
    	cyhal_rtc_register_callback(&rtc_obj, rtc_cb, NULL);
    	cyhal_rtc_enable_event(&rtc_obj, CYHAL_RTC_ALARM, 5, true);

    http_client_cmd = GET_CMD;
    xQueueSend(http_client_cmd_q, &http_client_cmd, RTOS_TICKS_TO_WAIT);

    while(true)
    {
        rtos_api_result = xQueueReceive(http_client_cmd_q, &http_client_cmd, RTOS_TICKS_TO_WAIT);

        if(rtos_api_result == pdTRUE)
        {
            switch(http_client_cmd)
            {
                case GET_CMD:
                {
                    is_http_data_sync = true;
                    memset(get_weather_string, 0, sizeof(get_weather_string) );
                    memcpy(get_weather_string, GET_WEATHER_URL_PATH, strlen(GET_WEATHER_URL_PATH));
                    memset(get_time_string, 0, sizeof(get_time_string));
                    memcpy(get_time_string, GET_TIME_URL_PATH, strlen(GET_TIME_URL_PATH));

                    result = send_http_get_request(GET_LOCATION);

                    /* Weather and time are fetched based on the location coordinates.
                     * So retrieve the data only if location coordinates are fetched successfully.
                     */
                    if(result == CY_RSLT_SUCCESS)
                    {
                        result = send_http_get_request(GET_WEATHER);
                        if(result != CY_RSLT_SUCCESS)
                        {
                            printf("Failed to retrieve weather data\n");
                            is_http_data_sync = false;
                            break;
                        }
                        result = send_http_get_request(GET_TIME);
                        if(result != CY_RSLT_SUCCESS)
                        {
                            printf("Failed to retrieve time data\n");
                            is_http_data_sync = false;
                            break;
                        }

                    }
                    else
                    {
                        printf("Could not fetch the location, weather and time info!\n");
                        is_http_data_sync = false;
                    }

                    break;
                }

                default:
                    break;
            }
        }

        if(display_time_update && display_time)
		{
			display_time_update = false;
			cyhal_rtc_read(&rtc_obj, &current_time);
			sprintf(text,"%02d:%02d", current_time.tm_hour, current_time.tm_min);
			TEXT_SetFont(text_handle, GUI_FONT_D32);
			TEXT_SetTextColor(text_handle, GUI_ORANGE);
			TEXT_SetText(text_handle, text);

			switch(current_time.tm_wday)
			{
			 case 0:
				 sprintf(text,"Sun, %s %02d", months[current_time.tm_mon], current_time.tm_mday);
				 break;
			 case 1:
				 sprintf(text,"Mon, %s %02d", months[current_time.tm_mon], current_time.tm_mday);
				 break;
			 case 2:
				 sprintf(text,"Tue, %s %02d", months[current_time.tm_mon], current_time.tm_mday);
				 break;
			 case 3:
				 sprintf(text,"Wed, %s %02d", months[current_time.tm_mon], current_time.tm_mday);
				 break;
			 case 4:
				 sprintf(text,"Thu, %s %02d", months[current_time.tm_mon], current_time.tm_mday);
				 break;
			 case 5:
				 sprintf(text,"Fri, %s %02d", months[current_time.tm_mon], current_time.tm_mday);
				 break;
			 case 6:
				 sprintf(text,"Sat, %s %02d", months[current_time.tm_mon], current_time.tm_mday);
				 break;
			}
			TEXT_SetFont(date_handle, GUI_FONT_16B_1);
			TEXT_SetTextColor(date_handle, GUI_ORANGE);
			TEXT_SetText(date_handle, text);

			/* Update City */
			TEXT_SetFont(loc_handle, GUI_FONT_20B_1);
			TEXT_SetTextColor(loc_handle, GUI_WHITE);
			TEXT_SetText(loc_handle, city_info);
		}

		if(display_loc_weather_update && (!display_time))
		{
			update_display_loc_weather();
		}

    }
}

/******************************************************************************
 * Function Name: send_http_get_request
 ******************************************************************************
 * Summary:
 *  This function send HTTP GET command to HTTP servers and fetches
 *  location/time/weather based on the HTTP GET command type.
 *
 * Parameters:
 *  http_get_t get_type : HTTP GET command type. See http_get_t
 *
 * Return:
 *  cy_rslt_t : result of HTTP GET operation.
 *
 ******************************************************************************/
static cy_rslt_t send_http_get_request(http_get_t get_type)
{
    /* Status variables for various operations. */
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_awsport_server_info_t server_info;
    cy_http_client_request_header_t request;
    cy_http_client_header_t header;
    uint32_t num_header;
    cy_http_client_response_t response;

    /*Pointer to HTTP GET response. */
    char* http_data = NULL;

    /* Variable to store number of bytes received and sent over TCP socket. */
    uint32_t bytes_received = 0;

    switch(get_type)
      {
          case GET_LOCATION:
          {
              server_info.host_name = GET_GEO_LOC_URL;
              server_info.port = GET_GEO_LOC_PORT;
              request.resource_path = GET_GEO_LOC_PATH;
              /* Register callback to process the json response packet*/
              cy_JSON_parser_register_callback(json_parser_location_cb, NULL);

              break;
          }

          case GET_TIME:
          {
              server_info.host_name = GET_TIME_URL;
              server_info.port = GET_TIME_PORT;
              request.resource_path = get_time_string;
              /* Register callback to process the json response packet*/
              cy_JSON_parser_register_callback(json_parser_time_cb, NULL);

              break;
          }

          case GET_WEATHER:
          {
              server_info.host_name = GET_WEATHER_URL;
              server_info.port = GET_WEATHER_PORT;
              request.resource_path = get_weather_string;
              /* Register callback to process the json response packet*/
              cy_JSON_parser_register_callback(json_parser_weather_cb, NULL);

              break;
          }

          default:
          break;
      }

      /* Create an instance of the HTTP client. */
      result = cy_http_client_create(NULL, &server_info, NULL, NULL, &handle);
      if( result != CY_RSLT_SUCCESS )
      {
          printf("HTTP client creation failed\n");
      }

      /* Connect the HTTP client to server. */
      result = cy_http_client_connect(handle, TRANSPORT_SEND_RECV_TIMEOUT_MS, TRANSPORT_SEND_RECV_TIMEOUT_MS);
      if( result != CY_RSLT_SUCCESS )
      {
          printf("HTTP client connect failed\n");
      }

      /* Allocate memory for send and receive buffers. */
      user_buffer = (uint8_t*)malloc(USER_BUFFER_LENGTH);

      request.buffer = user_buffer;
      request.buffer_len = USER_BUFFER_LENGTH;
      request.headers_len = 0;
      request.method = CY_HTTP_CLIENT_METHOD_GET;
      request.range_end = -1;
      request.range_start = 0;
      header.field = "Connection";
      header.field_len = strlen("Connection");
      header.value = "keep-alive";
      header.value_len = strlen("keep-alive");
      num_header = 1;

      /* Generate the standard header and user-defined header and update in the request structure. */
      result = cy_http_client_write_header(handle, &request, &header, num_header);
      if( result != CY_RSLT_SUCCESS )
      {
          printf("HTTP header creation failed\n");
      }

      /* Send the HTTP request and body to the server and receive the response from it. */
      result = cy_http_client_send(handle, &request, NULL, 0, &response);
      if( result != CY_RSLT_SUCCESS )
      {
          printf("HTTP send request and read response failed\n");
      }

      http_data = (char*)response.body;
      bytes_received = response.body_len;

      while ((*http_data != '{') && (bytes_received > 0) )
      {
          http_data++;
          bytes_received--;
      }
      if(bytes_received > 0)
      {
          /* Parse the json data. */
          cy_JSON_parser(http_data, bytes_received);
      }
      else
      {
          printf("No valid data found!\n");
          is_http_data_sync = false;
      }

      /* Disconnect the HTTP client from the server. */
      result = cy_http_client_disconnect(handle);

      /* Delete the instance of the HTTP client. */
      result = cy_http_client_delete(handle);

      /* Free the send and receive buffers. */
      free(user_buffer);

      return result;
}

/*******************************************************************************
* Function Name: rtc_leap_year
****************************************************************************//**
*
* Checks whether the year passed through the parameter is leap or no.
*
* \param year
* The year to be checked.
*
* \return
* 0u - The year is not leap <br> 1u - The year is leap.
*
*******************************************************************************/
static __inline uint32_t rtc_leap_year(uint32_t year)
{
    uint32_t ret_val;

    if(((0u == (year % 4Lu)) && (0u != (year % 100Lu))) || (0u == (year % 400Lu)))
    {
        ret_val = 1uL;
    }
    else
    {
        ret_val = 0uL;
    }

    return(ret_val);
}

/*******************************************************************************
* Function Name: rtc_days_in_month
****************************************************************************//**
*
* Returns a number of days in a month passed through the parameters.
*
* \param month
* A month of a year, see \ref group_rtc_month.
*
* \param year
* A year value.
*
* \return
* A number of days in a month in the year passed through the parameters
*
*******************************************************************************/
static uint32_t rtc_days_in_month(uint32_t month, uint32_t year)
{
    uint32_t ret_val;

    ret_val = rtc_days_in_month_table[month - 1u];
    if((uint32_t)RTC_FEBRUARY == month)
    {
        if(0u != rtc_leap_year(year))
        {
            ret_val++;
        }
    }

    return(ret_val);
}

/*******************************************************************************
* Function Name: rtc_get_day_of_week
****************************************************************************//**
* \internal
*
*  Returns a day of the week for a year, month, and day of month that are passed
*  through parameters. Zeller's congruence is used to calculate the day of
*  the week.
*
*  For the Georgian calendar, Zeller's congruence is:
*  h = (q + [13 * (m + 1)] + K + [K/4] + [J/4] - 2J) mod 7;
*
*  h - The day of the week (0 = Saturday, 1 = Sunday, 2 = Monday, ...,
*  6 = Friday).
*  q - The day of the month.
*  m - The month (3 = March, 4 = April, 5 = May, ..., 14 = February).
*  K - The year of the century (year \mod 100).
*  J - The zero-based century (actually [year/100]) For example, the zero-based
*  centuries for 1995 and 2000 are 19 and 20 respectively (not to be
*  confused with the common ordinal century enumeration which indicates
*  20th for both cases).
*
* \note
* In this algorithm January and February are counted as months 13 and 14
* of the previous year.
*
* \param day
* The day of the month(1..31)
*
* \param month
* The month of the year, see \ref group_rtc_month
*
* \param year
* The year value.
*
* \return
* Returns a day of the week, see \ref group_rtc_day_of_the_week.
*
* \endinternal
*******************************************************************************/
static __inline uint32_t rtc_get_day_of_week(uint32_t day, uint32_t month, uint32_t year)
{
    uint32_t ret_val;

    /* Converts month number from regular convention
     * (1=January,..., 12=December) to convention required for this
     * algorithm(January and February are counted as months 13 and 14 of
     * previous year).
    */
    if(month < (uint32_t)RTC_MARCH)
    {
        month = 12u + month;
        year--;
    }

    /* Calculates Day of Week using Zeller's congruence algorithms */
    ret_val = (day + (((month + 1u) * 26u) / 10u) + year + (year / 4u) + (6u * (year / 100u)) + (year / 400u)) % 7u;

    /* Makes correction for Saturday. Saturday number should be 7 instead of 0. */
    if(0u == ret_val)
    {
        ret_val = (uint32_t)7;
    }

    return(ret_val-1);
}

/*******************************************************************************
 * Function Name: unix_to_date_time
 *******************************************************************************
 * Summary:
 *  Function to connect to TCP server.
 *
 * Parameters:
 *  struct tm* date_time: Pointer to time structure.
 *  uint64 unix_time: unix time
 *  uint32_t time_format: time format
 *
 * Return:
 *  None
 *
 *******************************************************************************/
static void unix_to_date_time(struct tm* date_time, uint64 unix_time, uint32_t time_format)
{
    uint32_t tmp_minute;
    uint32_t tmp_hour;
    uint32_t tmp_day;
    uint32_t tmp_month;
    uint32_t tmp_year;
    uint32_t tmp_var;
    tmp_year = RTC_YEAR_0;
    tmp_var = RTC_SECONDS_PER_NONLEAP_YEAR;

    /* Calculates current year value. Variable tmp_year
    *  increments while it contains value greater than number
    *  of seconds in current year.
    */
    while(unix_time >= tmp_var)
    {
        unix_time -= tmp_var;
        tmp_year++;

        if(0u != rtc_leap_year(tmp_year))
        {
            tmp_var = RTC_SECONDS_PER_LEAP_YEAR;
        }
        else
        {
            tmp_var = RTC_SECONDS_PER_NONLEAP_YEAR;
        }
    }

    /* Calculates current month value. The tmp_month variable increments while
    *  unix_time variable value is greater than time interval from beginning
    *  of current year to beginning of current month
    */
    tmp_month = (uint32_t)RTC_JANUARY;
    tmp_var = rtc_days_in_month(tmp_month, tmp_year) * RTC_SECONDS_PER_DAY;

    while(unix_time >= tmp_var)
    {
        unix_time -= tmp_var;
        tmp_month++;
        tmp_var = rtc_days_in_month(tmp_month, tmp_year) * RTC_SECONDS_PER_DAY;
    }

    /* Calculates current day value */
    tmp_day = (uint32_t) (unix_time / RTC_SECONDS_PER_DAY);
    tmp_var = tmp_day * RTC_SECONDS_PER_DAY;
    unix_time -= (unix_time >= tmp_var) ? tmp_var : 0u;
    tmp_day += 1u;

    /* Calculates current hour value. If function works in 12-Hour mode,
     * it converts time to 12-Hours mode and calculates AmPm status */
    tmp_hour = (uint32_t) (unix_time / RTC_SECONDS_PER_HOUR);
    tmp_var  = tmp_hour * RTC_SECONDS_PER_HOUR;
    if((uint32_t)RTC_24_HOURS_FORMAT != time_format)
    {
        if(unix_time > RTC_UNIX_TIME_PM)
        {
            tmp_hour = (tmp_hour > 12u) ? (tmp_hour - 12u) : tmp_hour;
        }
        else
        {
            tmp_hour = (0u != tmp_hour) ? tmp_hour : 12u;
        }
    }
    unix_time -= (unix_time >= tmp_var) ? tmp_var : 0u;

    /* Calculates current minute */
    tmp_minute = (uint32_t) (unix_time / RTC_SECONDS_PER_MINUTE);
    tmp_var = tmp_minute * RTC_SECONDS_PER_MINUTE;

    /* Calculates current second */
    unix_time -= (unix_time >= (uint64) tmp_var) ? (uint64) tmp_var : 0ul;

    date_time->tm_mon = tmp_month-1;
    date_time->tm_mday = tmp_day;
    date_time->tm_year = tmp_year-1900;
    date_time->tm_sec = unix_time;
    date_time->tm_min = tmp_minute;
    date_time->tm_hour = tmp_hour;
    date_time->tm_wday = rtc_get_day_of_week(tmp_day, tmp_month, tmp_year);
}

/*******************************************************************************
 * Function Name: json_parser_location_cb
 *******************************************************************************
 * Summary:
 *  Function to connect to TCP server.
 *
 * Parameters:
 *  cy_JSON_object_t* json_object: Pointer to json object.
 *  void *arg: unused argument
 *
 * Return:
 *  cy_rslt_t result
 *
 *******************************************************************************/
static cy_rslt_t json_parser_location_cb(cy_JSON_object_t* json_object, void *arg)
{
    cy_rslt_t result = 0;
    int32_t count = 0;

    if(memcmp("lat", json_object->object_string, json_object->object_string_length) == 0)
    {
        printf("Lat: ");
        for(count = 0; count < json_object->value_length; count++)
        {
            printf("%c", json_object->value[count]);
            lat[count]=json_object->value[count];
        }
        lat[count]='\0';
        strcat(get_weather_string,lat);
        printf("\n");
    }
    else if(memcmp("lon", json_object->object_string, json_object->object_string_length) == 0)
    {
        printf("Lon: ");
        for(count = 0; count < json_object->value_length; count++)
        {
            printf("%c", json_object->value[count]);
            lon[count]=json_object->value[count];
        }
        lon[count]='\0';
        strcat(get_weather_string, "&lon=");
        strcat(get_weather_string,lon);
        printf("\n");
    }
    else if(memcmp("timezone", json_object->object_string, json_object->object_string_length) == 0)
    {
        printf("timezone: ");
        for(count = 0; count < json_object->value_length; count++)
        {
            printf("%c", json_object->value[count]);
            timezone[count]=json_object->value[count];
        }
        timezone[count]='\0';
        strcat(get_time_string, timezone);
        printf("\n");
    }

    return result;
}

/*******************************************************************************
 * Function Name: json_parser_weather_cb
 *******************************************************************************
 * Summary:
 *  Function to connect to TCP server.
 *
 * Parameters:
 *  cy_JSON_object_t* json_object: Pointer to json object.
 *  void *arg: unused argument
 *
 * Return:
 *  cy_rslt_t result
 *
 *******************************************************************************/
static cy_rslt_t json_parser_weather_cb(cy_JSON_object_t* json_object, void *arg)
{
    cy_rslt_t result = 0;
    int32_t count = 0;
    static int32_t day = 0;

    if((json_object->object_string[0] == 'i') || (json_object->object_string[0] == 'c')
        || (json_object->object_string[0] == 't'))
    {
        if(memcmp("icon", json_object->object_string, json_object->object_string_length) == 0)
        {
            switch(json_object->value[0])
            {
            case 't':
                weather_info[day].icon = THUNDERSTORM;
                break;
            case 'd':
            case 'r':
            case 'u':
                weather_info[day].icon = RAIN;
                break;
            case 's':
                weather_info[day].icon = SNOW;
                break;
            case 'a':
                weather_info[day].icon = FOG;
                break;
            case 'c':
                if('1' == json_object->value[2])
                {
                    weather_info[day].icon = CLEAR;
                }
                else if(('2' == json_object->value[2]) || ('3' == json_object->value[2]))
                {
                    weather_info[day].icon = SCATTERED_CLOUDS;
                }
                else if('4' == json_object->value[2])
                {
                    weather_info[day].icon = OVERCAST;
                }
                break;
            default:
                break;
            }
        }
        else if(memcmp("temp", json_object->object_string, json_object->object_string_length) == 0)
        {
            printf("Temp day%lu: ", day);
            for(count = 0; count < json_object->value_length; count++)
            {
                printf("%c", json_object->value[count]);

                if(count < 2)
                {
                    weather_info[day].temp[count] = json_object->value[count];
                }
                else if(count == 2)
                {
                    weather_info[day].temp[count] = '\0';
                }
            }
            printf("\n");

            day++;

            if(day == FORECAST_NO_OF_DAYS)
            {
                day = 0;
            }
        }
        else if(memcmp("city_name", json_object->object_string, json_object->object_string_length) == 0)
        {
            printf("city_name: ");
            for(count = 0; count < json_object->value_length; count++)
            {
                printf("%c", json_object->value[count]);
                city_info[count] = json_object->value[count];

                if(count == CITY_NAME_LEN)
                {
                    break;
                }
            }
            city_info[count] = '\0';
            display_loc_weather_update = true;
            printf("\n");
        }
    }

    return result;
}

/*******************************************************************************
 * Function Name: json_parser_time_cb
 *******************************************************************************
 * Summary:
 *  Function to connect to TCP server.
 *
 * Parameters:
 *  cy_JSON_object_t* json_object: Pointer to json object.
 *  void *arg: unused argument
 *
 * Return:
 *  cy_rslt_t result
 *
 *******************************************************************************/
static cy_rslt_t json_parser_time_cb(cy_JSON_object_t* json_object, void *arg)
{
    cy_rslt_t result = 0;
    int32_t count = 0;
    uint32_t unix_time = 0;

    if(memcmp("timestamp", json_object->object_string, json_object->object_string_length) == 0)
    {
        printf("Time: ");
        for(count = 0; count < json_object->value_length; count++)
        {
            printf("%c", json_object->value[count]);
            unix_time = unix_time*10 + (json_object->value[count] -'0');
        }
        printf("\n");

        unix_to_date_time(&current_time, unix_time, RTC_24_HOURS_FORMAT);
        cyhal_rtc_write(&rtc_obj, &current_time);
        prev_time = current_time;
        display_time_update = 1;
    }

    return result;
}

/*******************************************************************************
 * Function Name: connect_to_wifi_ap()
 *******************************************************************************
 * Summary:
 *  Connects to Wi-Fi AP using the user-configured credentials, retries up to a
 *  configured number of times until the connection succeeds.
 *
 *******************************************************************************/
cy_rslt_t connect_to_wifi_ap(void)
{
    cy_rslt_t result;

    /* Variables used by Wi-Fi connection manager.*/
    cy_wcm_connect_params_t wifi_conn_param;

    cy_wcm_ip_address_t ip_address;

     /* Set the Wi-Fi SSID, password and security type. */
    memset(&wifi_conn_param, 0, sizeof(cy_wcm_connect_params_t));
    memcpy(wifi_conn_param.ap_credentials.SSID, WIFI_SSID, sizeof(WIFI_SSID));
    memcpy(wifi_conn_param.ap_credentials.password, WIFI_PASSWORD, sizeof(WIFI_PASSWORD));
    wifi_conn_param.ap_credentials.security = WIFI_SECURITY_TYPE;

    printf("Connecting to Wi-Fi Network: %s\n", WIFI_SSID);

    /* Join the Wi-Fi AP. */
    for(uint32_t conn_retries = 0; conn_retries < MAX_WIFI_CONN_RETRIES; conn_retries++ )
    {
        result = cy_wcm_connect_ap(&wifi_conn_param, &ip_address);

        if(result == CY_RSLT_SUCCESS)
        {
            printf("Successfully connected to Wi-Fi network '%s'.\n",
                                wifi_conn_param.ap_credentials.SSID);
            printf("IP Address Assigned: %s\n",
                    ip4addr_ntoa((const ip4_addr_t *)&ip_address.ip.v4));
            return result;
        }

        printf("Connection to Wi-Fi network failed with error code %d."
               "Retrying in %d ms...\n", (int)result, WIFI_CONN_RETRY_INTERVAL_MSEC);

        vTaskDelay(pdMS_TO_TICKS(WIFI_CONN_RETRY_INTERVAL_MSEC));
    }

    /* Stop retrying after maximum retry attempts. */
    printf("Exceeded maximum Wi-Fi connection attempts\n");

    return result;
}

/******************************************************************************
 * Function Name: rtc_cb
 ******************************************************************************
 * Summary: Callback function to handle RTC alarm interrupt. The interrupt is
 * triggered every 1 minute.
 *
 * Parameters:
 *  void *callback_arg - unused
 *  cyhal_rtc_event_t event - unused
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void rtc_cb(void *callback_arg, cyhal_rtc_event_t event)
{
    /* To remove compiler warnings. */
    (void)callback_arg;
    (void)event;

    display_time_update = true;

    cyhal_rtc_read(&rtc_obj, &current_time);

    if((current_time.tm_mday != prev_time.tm_mday))
    {
        is_http_data_sync = false;
        prev_time = current_time;
    }
}

/******************************************************************************
 * Function Name: update_display_loc_weather
 ******************************************************************************
 * Summary: Function updates the display with latest location and weather
 * forecast.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void update_display_loc_weather()
{
    char text[TEXT_BUFFER_SIZE];

    /* Update City */
    TEXT_SetFont(loc_handle, GUI_FONT_20B_1);
    TEXT_SetTextColor(loc_handle, GUI_WHITE);
    TEXT_SetText(loc_handle, city_info);

    /* Update weather info */
    sprintf(text,"%s\nTemp: %s %cC",weather_text[weather_info[0].icon], weather_info[0].temp, 176);
    TEXT_SetFont(text_handle, GUI_FONT_20B_1);
    TEXT_SetTextColor(text_handle, GUI_LIGHTBLUE);
    TEXT_SetText(text_handle, text);

    display_loc_weather_update = false;
}


/* [] END OF FILE */
