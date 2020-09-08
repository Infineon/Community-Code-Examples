/******************************************************************************
* File Name:  oled_task.c
*
* Description:  This file contains all the functions and variables required for
*               proper operation of OLED Task.
*
* Related Document: See Readme.md
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

#include "oled_task.h"

/*******************************************************************************
* Function Name: Show_Startup_Screen
****************************************************************************//**
*
* Function displays the Infineon + Cypress Logo.
*
*******************************************************************************/

void Show_Startup_Screen(void)
{
	/* ****************************************************
	 * To avoid task switching cyhal_system_delay_ms() is
	 * used instead of vTaskDelay(). This is done so that
	 * sensor does not start sampling before OLED is
	 * ready to display readings.
	 * ****************************************************/

    /* Set foreground and background color and font size */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();

    /* Display Infineon logo */
    GUI_DrawBitmap(&bmifx, 0, 0);
    cyhal_system_delay_ms(2000);
    GUI_Clear();

    /* Display '+' symbol */
    GUI_DrawBitmap(&bmplus, 0, 0);
    cyhal_system_delay_ms(2000);
    GUI_Clear();

    /* Display Cypress logo */
    GUI_DrawBitmap(&bmcy, 0, 0);
    cyhal_system_delay_ms(2000);
    GUI_Clear();
}

/*******************************************************************************
* Function Name: Show_Tag_Line
****************************************************************************//**
*
* Function displays the Infineon + Cypress tagline.
*
*******************************************************************************/

void Show_Tag_Line(void)
{
	/* ****************************************************
	 * To avoid task switching cyhal_system_delay_ms() is
	 * used instead of vTaskDelay(). This is done so that
	 * sensor does not start sampling before OLED is
	 * ready to display readings.
	 * ****************************************************/

    /* Set text mode */
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display the tagline */
    GUI_SetFont(GUI_FONT_8_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Strengthening the link", 64, 20);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("between the real and ", 64, 30);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("the digital world", 64, 40);

    cyhal_system_delay_ms(2000);

    /* Clear the display */
	GUI_Clear();
}

/*******************************************************************************
* Function Name: OLED_Task
****************************************************************************//**
*
* Task function to initialize the OLED panel and display the sensor values on
* the OLED panel after reading from the queue.
*
* \param pvParameters
* Void pointer which points to the queue handle
*
*******************************************************************************/

void OLED_Task(void* pvParameters)
{
	cy_rslt_t result;

	/* Initialize the I2C block */
	result = cyhal_i2c_init(&i2c_obj, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
	if (result != CY_RSLT_SUCCESS)
	{
		printf("\r\nError: I2C initialization failed\r\n");
		CY_ASSERT(0);
	}

	/* Initialize the OLED display library*/
	result = mtb_ssd1306_init_i2c(&i2c_obj);
	if (result != CY_RSLT_SUCCESS)
	{
		printf("\r\nError: OLED display library initialization failed\r\n");
		CY_ASSERT(0);
	}

	/* Initialize emWin Library */
	GUI_Init();

	/* **************************************************************
	 * If code execution does not reach this point, then please check
	 * the OLED display connections and ensure that the connections
	 * are correct.
	 * **************************************************************/

	/* Display Infineon + Cypress Logo */
	Show_Startup_Screen();

	/* Display tagline */
	Show_Tag_Line();

	/* Variable to store the queue handle */
	QueueHandle_t dht_reading_queue;
	dht_reading_queue = (QueueHandle_t) pvParameters;

	/* Variable to store temperature and humidity values */
	struct readings DHT_reading = {0, 0};

	for(;;)
	{
		/* ********************************************************************
		 * Block until DHT Task updates the queue and sets the corresponding
		 * event bit within the event group.
		 * ********************************************************************/
		xEventGroupWaitBits( /* The event group to read */
							dht_reading_notify_event_group,
							/* Bits to test */
							OLED_TASK_NOTIFY_EVENTBIT,
							/* Clear bits on exit if the unblock condition is met */
							pdTRUE,
							/* xWaitForAllBits does not matter as there is only 1bit */
							pdTRUE,
							/* Don't time out */
							portMAX_DELAY );

		/* *************************************************************
		 * Read the temperature and humidity values from the queue.
		 * xQueuePeek() copies the data from the queue to the structure
		 * without removing the data from the queue.
		 * *************************************************************/
		xQueuePeek(dht_reading_queue, &DHT_reading, portMAX_DELAY);

		/* Display the DHT sensor readings if the values are valid */
		if(DHT_reading.result_code == SUCCESS)
		{
			/* Clear the display */
			GUI_Clear();

			/* Set appropriate font, alignment, position and display the readings */
			GUI_SetFont(GUI_FONT_10_1);
			GUI_SetTextAlign(GUI_TA_LEFT);
			GUI_DispStringAt("Humidity: ", 2, 20);
			/* Display float value upto 5 characters */
			GUI_DispFloat(DHT_reading.humidity, 5);
			GUI_DispStringAt("Temperature: ", 2, 40);
			/* Display float value upto 5 characters */
			GUI_DispFloat(DHT_reading.temperature, 5);
		}
	}
}



