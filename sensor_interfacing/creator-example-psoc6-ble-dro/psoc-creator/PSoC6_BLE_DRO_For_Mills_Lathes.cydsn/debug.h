/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
/* Include Files */
#ifndef DEBUG_H
#define DEBUG_H

/* Header file includes */
#include <project.h>
#include <stdio.h>  

#define DISABLED    0
#define ENABLED     (!DISABLED)

/***************************************
* Conditional Compilation Parameters
***************************************/
#define DEBUG_UART_ENABLED      DISABLED

/***************************************
*        External Function Prototypes
***************************************/
void ShowError(void);

/***************************************
*        Macros
***************************************/
#if DEBUG_UART_ENABLED

    __STATIC_INLINE void UART_DEBUG_START(void)              
    {
        (void) Cy_SCB_UART_Init(UART_DEB_HW, &UART_DEB_config, &UART_DEB_context);
        Cy_SCB_UART_Enable(UART_DEB_HW);
    }

    #define DEBUG_PRINTF(...)               (printf(__VA_ARGS__))

    #define UART_DEBUG_PUT_CHAR(...)        while(1UL != Cy_SCB_UART_Put(UART_DEBUG_HW, __VA_ARGS__))

    __STATIC_INLINE char8 UART_DEBUG_GET_CHAR(void)
    {
        uint32 rec;
        Cy_SCB_UART_Receive(UART_DEB_HW, &rec, 0x01, &UART_DEB_context);
        return((rec == CY_SCB_UART_RX_NO_DATA) ? 0u : (char8)(rec & 0xff));
    }
    #define UART_DEBUG_GET_TX_BUFF_SIZE(...)  (Cy_SCB_GetNumInTxFifo(UART_DEBUG_SCB__HW) + Cy_SCB_GetTxSrValid(UART_DEBUG_SCB__HW))

    #define DEBUG_WAIT_UART_TX_COMPLETE()     while(UART_DEBUG_GET_TX_BUFF_SIZE() != 0);

#else
    #define UART_DEB_START() 

    #define DEBUG_PRINTF(...)

    #define UART_DEB_PUT_CHAR(...)

    #define UART_DEB_GET_CHAR(...)              (0u)

    #ifndef UART_DEB_GET_TX_FIFO_SR_VALID
        #define UART_DEB_GET_TX_FIFO_SR_VALID   (0u)
    #endif

    #define UART_DEB_GET_TX_BUFF_SIZE(...)      (0u)

    #define DEBUG_WAIT_UART_TX_COMPLETE()

#endif /* (DEBUG_UART_ENABLED ) */    
#endif /* DEBUG_H */

/* [] END OF FILE */