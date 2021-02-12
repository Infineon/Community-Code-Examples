/******************************************************************************
* File Name:   tlx4966.h
*
* Description: This file contains the macros and function declarations for the 
*              TLx4966 sensor.
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Timer period value. Assign maximum period for the 32 bit counter (2^32) */
#define TIMER_PERIOD            (4294967295)

#define GPIO_INTERRUPT_PRIORITY (7u)

/* Speed Units */
#define TLx4966_SPEED_COEF_HZ      1000.0      /* Hertz - cps (1000 ms) */
#define TLx4966_SPEED_COEF_RADS    6283.2      /* Rad/s (2pi x 1000 ms) */
#define TLx4966_SPEED_COEF_RPM     60000.0     /* RPM  (60 x 1000 ms)   */

/* Macro to display the current speed unit in a printf statement */
#define SPEED_UNIT (speed_unit == TLx4966_SPEED_COEF_HZ ? "Hz" : \
                   (speed_unit == TLx4966_SPEED_COEF_RADS ? "Rads" : "RPM"))

/* Timer object used for blinking the LED */
cyhal_timer_t timer_obj;

/* Speed unit to measure */
float speed_unit;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
cy_rslt_t TLx4966_init(cyhal_gpio_t speed_pin, cyhal_gpio_t dir_pin, float userspeedUnit);
float TLx4966_readSpeed(void);
uint8_t TLx4966_readDir(void);

/* [] END OF FILE */
