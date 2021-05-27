/******************************************************************************
*
* File Name: tft_task.c
*
* Description: This file contains task and functions related to the tft-task
* that demonstrates controlling a tft display using the EmWin Graphics Library.
* The project displays a start up screen with Cypress logo and
* text "CYPRESS EMWIN GRAPHICS DEMO tft DISPLAY".
*
* The project then displays the following screens in a loop
*
*   1. A screen showing text alignment, styles, and modes
*   2. A screen showing text colors
*   3. A screen showing normal fonts
*   4. A screen showing bold fonts
*   5. A screen showing color gradients
*   6. A screen showing 2D graphics with horizontal lines, vertical lines
*       arcs and filled rounded rectangle
*   7. A screen showing 2D graphics with concentric circles and ellipses
*   8. A screen showing colorful concentric circles
*   9. A screen showing a bitmap image
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "GUI.h"
#include "mtb_st7789v.h"
#include "tft_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "icons.h"
#include "http_client_task.h"


#define GUI_DELAY                       (5u)

/******************************************************************************
* Global Variables
*******************************************************************************/
/* HTTP client task handle. */
TaskHandle_t tft_task_handle;

/* The pins are defined by the st7789v library. If the display is being used on
 * different hardware the mappings will be different.
 */
const mtb_st7789v_pins_t tft_pins =
{
    .db08 = CYBSP_J2_2,
    .db09 = CYBSP_J2_4,
    .db10 = CYBSP_J2_6,
    .db11 = CYBSP_J2_10,
    .db12 = CYBSP_J2_12,
    .db13 = CYBSP_D7,
    .db14 = CYBSP_D8,
    .db15 = CYBSP_D9,
    .nrd  = CYBSP_D10,
    .nwr  = CYBSP_D11,
    .dc   = CYBSP_D12,
    .rst  = CYBSP_D13
};


/******************************************************************************
* Function Prototypes
*******************************************************************************/
extern WM_HWIN CreateWindow(void);

/*******************************************************************************
* Function Name: void tft_task(void *arg)
********************************************************************************
*
* Summary:
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void tft_task(void *arg)
{
    cy_rslt_t result;

    /* Initialize the display controller */
    result = mtb_st7789v_init8(&tft_pins);
    CY_ASSERT(result == CY_RSLT_SUCCESS);
    
    /* To avoid compiler warning */
    (void)result;
    
    /* Initialize the emWin library */
    GUI_Init();

    /* Create the GUI window */
    CreateWindow();


    for(;;)
    {
        GUI_Delay(GUI_DELAY);
    }
}


/* END OF FILE */
