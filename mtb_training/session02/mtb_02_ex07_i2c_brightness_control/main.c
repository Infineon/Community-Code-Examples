/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the PSoC 6 MCU I2C Slave Using
*              Callbacks Example for ModusToolbox.

* Related Document: See Readme.md
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
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

/* Project Description
 *
 * Refer README.md file for information
 */

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* LED period in microseconds */
#define LED_PERIOD              (255u)
#define LED_INIT_PULSEWIDTH     (255u)

/* I2C slave address */
#define I2C_SLAVE_ADDRESS       (0x08u)

/* I2C slave bus frequency */
#define I2C_SLAVE_FREQ          (100000u)

/* I2C slave interrupt priority */
#define I2C_SLAVE_IRQ_PRIORITY  (7u)

/* Valid command packet size of three bytes */
#define PACKET_SIZE             (0x03u)

/* Master write and read buffer of size three bytes */
#define SL_RD_BUFFER_SIZE       (PACKET_SIZE)
#define SL_WR_BUFFER_SIZE       (PACKET_SIZE)

/* Start and end of packet markers */
#define PACKET_SOP              (0x01u)
#define PACKET_EOP              (0x17u)

/* Command valid status */
#define STS_CMD_DONE            (0x00u)
#define STS_CMD_FAIL            (0xFFu)

/* Packet positions */
#define PACKET_SOP_POS          (0x00u)
#define PACKET_STS_POS          (0x01u)
#define PACKET_LED_POS          (0x01u)
#define PACKET_EOP_POS          (0x02u)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void led_pwm_init(void);
void i2c_slave_init(void);
void execute_command(void);
void handle_i2c_slave_events(void *callback_arg, cyhal_i2c_event_t event);
void handle_error(void);

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* I2C read and write buffers - I2C master writes into i2c_write_buffer
 * and reads from i2c_read_buffer */
uint8_t i2c_read_buffer [SL_RD_BUFFER_SIZE] =
                                       {PACKET_SOP, STS_CMD_FAIL, PACKET_EOP};
uint8_t i2c_write_buffer[SL_WR_BUFFER_SIZE];

bool error_detected = false;
bool led_update_flag = false;

cyhal_pwm_t led_pwm;
cyhal_i2c_t i2c_slave;
cyhal_i2c_cfg_t i2c_slave_cfg = {true, I2C_SLAVE_ADDRESS, I2C_SLAVE_FREQ};

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  The main function performs the following actions:
*   1. Sets up I2C to be in I2C slave mode.
*   2. If initialization of I2C fails, system will be in infinite loop.
*   3. Initializes PWM to control the LED. If initialization of PWM fails,
*      system will be in infinite loop.
*   4. I2C slave receives packets from master and configures the PWM to
*      drive the LED.
*   5. Slave responds with the acknowledgment packet.
*
*\param
*  None
*
*\return
*  int
*
*******************************************************************************/
int main(void)
{
    /* Initialize the device and board peripherals */
    CY_ASSERT(cybsp_init() == CY_RSLT_SUCCESS);

    /* Initialize the PWM object */
    led_pwm_init();

    /* Initialize the I2C slave */
    i2c_slave_init();


    /* Enable global interrupts */
    __enable_irq();

    for(;;)
    {
        /* If error is detected in I2C slave operation, handle the error */
        if (error_detected == true)
        {
            handle_error();
        }
    }

    return (0);
}

/*******************************************************************************
* Function Name: led_pwm_init
********************************************************************************
* Summary:
* This function creates and configures a PWM object to drive PWM on
* pin CYBSP_USER_LED1.
*
*\param
*  None
*
*\return
*  None
*
*******************************************************************************/
void led_pwm_init(void)
{
    cy_rslt_t result;

    /* Allocate and initialize a TCPWM resource and auto select a clock */
    result = cyhal_pwm_init_adv(&led_pwm, CYBSP_USER_LED, NC,
                                CYHAL_PWM_RIGHT_ALIGN, true, 0, false, NULL);

    if (result == CY_RSLT_SUCCESS)
    {
        /* Set PWM period and initial pulse width */
        result = cyhal_pwm_set_period(&led_pwm,
                 LED_PERIOD, LED_INIT_PULSEWIDTH);
    }

    if (result == CY_RSLT_SUCCESS)
    {
        /* Start the PWM */
        result = cyhal_pwm_start(&led_pwm);
    }

    /* Handle error if PWM configuration failed */
    if (result != CY_RSLT_SUCCESS) handle_error();
 }


/*******************************************************************************
* Function Name: i2c_slave_init
********************************************************************************
*
* This function creates and configures an I2C slave object to communicate with
* I2C master.
*
*\param
*  None
*
*\return
*  None
*
*******************************************************************************/
void i2c_slave_init(void)
{
    cy_rslt_t result;

    /* Allocate and initialize a I2C resource and auto select a clock */
    result = cyhal_i2c_init(&i2c_slave, CYBSP_I2C_SDA,
                            CYBSP_I2C_SCL, NULL);

    if (result == CY_RSLT_SUCCESS)
    {
        /* Configure the I2C resource to be slave */
        result = cyhal_i2c_configure (&i2c_slave, &i2c_slave_cfg);
    }

    if (result == CY_RSLT_SUCCESS)
    {
        /* Configure I2C slave write buffer for I2C master to write into */
        result = cyhal_i2c_slave_config_read_buff(&i2c_slave, i2c_write_buffer,
                    SL_WR_BUFFER_SIZE);
    }

    if (result == CY_RSLT_SUCCESS)
    {
        /* Configure I2C slave read buffer for I2C master to read from */
        result = cyhal_i2c_slave_config_write_buff(&i2c_slave, i2c_read_buffer,
                  SL_RD_BUFFER_SIZE);
    }

    /* Handle error if I2C slave configuration failed */
    if (result != CY_RSLT_SUCCESS) handle_error();

    /* Register I2C slave event callback */
    cyhal_i2c_register_callback(&i2c_slave,
                    (cyhal_i2c_event_callback_t) handle_i2c_slave_events, NULL);

    /* Enable events */
    cyhal_i2c_enable_event(&i2c_slave, (cyhal_i2c_event_t)
                             (CYHAL_I2C_SLAVE_WR_CMPLT_EVENT \
                            | CYHAL_I2C_SLAVE_RD_CMPLT_EVENT \
                            | CYHAL_I2C_SLAVE_ERR_EVENT),    \
                            I2C_SLAVE_IRQ_PRIORITY, true);
}

/*******************************************************************************
* Function Name: handle_i2c_slave_events
********************************************************************************
*
* Handles slave events write and read completion events.
*
* \param callback_arg
*
* \param event
*   Event to be handled
*
* \return
*  None
*
*******************************************************************************/
void handle_i2c_slave_events(void *callback_arg, cyhal_i2c_event_t event)
{
    /* To remove unused variable warning */
    (void) callback_arg;

    /* Check write complete event */
    if (0UL != (CYHAL_I2C_SLAVE_WR_CMPLT_EVENT & event))
    {
        /* Check for errors */
        if (0UL == (CYHAL_I2C_SLAVE_ERR_EVENT & event))
        {
            /* Check start and end of packet markers */
            if ((i2c_write_buffer[PACKET_SOP_POS] == PACKET_SOP) &&
                (i2c_write_buffer[PACKET_EOP_POS] == PACKET_EOP))
            {
                /* Execute command and update reply status for received
                 * command */
                execute_command();
                i2c_read_buffer[PACKET_STS_POS] = STS_CMD_DONE;
            }
         }
         else
         {
             error_detected = true;
         }

         /* Configure write buffer for the next write */
         cyhal_i2c_slave_config_read_buff(&i2c_slave, i2c_write_buffer,
                                           SL_WR_BUFFER_SIZE);
     }

     /* Check read complete event */
     if (0UL != (CYHAL_I2C_SLAVE_RD_CMPLT_EVENT & event))
     {
         /* Configure read buffer for the next read */
         i2c_read_buffer[PACKET_STS_POS] = STS_CMD_FAIL;
         cyhal_i2c_slave_config_write_buff(&i2c_slave, i2c_read_buffer,
                                            SL_RD_BUFFER_SIZE);
     }
}

/*******************************************************************************
* Function Name: execute_command
********************************************************************************
*
* Sets the compare value for the LED PWM received by slave.
*
* \param
*  None
*
* \return
*  None
*
*******************************************************************************/
void execute_command(void)
{
    /* Sets the pulse width to control the brightness of the LED. */
    cyhal_pwm_set_period(&led_pwm, LED_PERIOD,
                         (i2c_write_buffer[PACKET_LED_POS]));
}

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
*
* This function processes unrecoverable errors such as any
* initialization errors etc. In case of such error the system will
* stay in the infinite loop of this function.
*
* \note
*  If error occurs interrupts are disabled.
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    /* Infinite loop. */
    while(1u);
}

/* [] END OF FILE */
