/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.10
*
* Description: This is the source code for PSoC6 BLE DRO for Mills/Lathes. 
*              BLE component is configured with a custom DRO service. 
*              The BLE sends notification data(which are the values 
*              of all three axes(x-axis, y-axis and z-axis)) to the BLE
*              GATT client device(BLE App). 
*
* Hardware Dependency: CY8CKIT-062 PSoC6 BLE Pioneer Kit
* 
*******************************************************************************/

#include "project.h"
#include <stdio.h>
#include "debug.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define LED_OFF     1
#define LED_ON      (!LED_OFF)

#define DONT_UPDATE 0
#define UPDATE      (!DONT_UPDATE)

#define QUADRATURE_DECODER_ZERO 0x80000000
             
/*******************************************************************************
* Variables
*******************************************************************************/
uint8 XAxis_Notification_Enabled = 0, YAxis_Notification_Enabled = 0, ZAxis_Notification_Enabled = 0;
int32 Count_XAxis = 0, Count_YAxis = 0, Count_ZAxis = 0;
cy_stc_ble_gatt_handle_value_pair_t XAxis_Notify_Data, YAxis_Notify_Data, ZAxis_Notify_Data;
cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
cy_en_ble_gatt_err_code_t error;
uint8 Update_BLE_Client = DONT_UPDATE;

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/
void Timer_Tick_ISR(void);

/*******************************************************************************
* Function Name: StackEventHandler()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
*  event - the event code
*  *eventParam - the event parameters
*
* Return:
*   None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void *eventParam)
{
    #if DEBUG_UART_ENABLED
    cy_stc_ble_conn_handle_t connHandle;
    #endif
    cy_stc_ble_gatt_write_param_t *writeData;
   
    switch (event)
	{
        /**********************************************************
        *                       General Events
        ***********************************************************/
        
		/* This event is received when the BLE component is Started */
        case CY_BLE_EVT_STACK_ON:
        {
            DEBUG_PRINTF("CY_BLE_EVT_STACK_ON \r\n");
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);          
            if(apiResult != CY_BLE_SUCCESS)
            {
                DEBUG_PRINTF("Start Advertisement API Error: %d \r\n", apiResult);
            }
            else
            {
                DEBUG_PRINTF("Start Advertisement API Success: %d \r\n", apiResult);
            }
            break;
        }
        
        /* This event is received when there is a timeout. */
        case CY_BLE_EVT_TIMEOUT:
        {
            DEBUG_PRINTF("CY_BLE_EVT_TIMEOUT \r\n");
            break;
        }
        
        /* This event indicates that some internal HW error has occurred. */    
		case CY_BLE_EVT_HARDWARE_ERROR: 
        {
            DEBUG_PRINTF("CY_BLE_EVT_HARDWARE_ERROR \r\n");
			break;
        }
        
        /*  This event will be triggered by host stack if BLE stack is busy
            or has become free */
    	case CY_BLE_EVT_STACK_BUSY_STATUS:
        {
            DEBUG_PRINTF("CY_BLE_EVT_STACK_BUSY_STATUS: %x\r\n", *(uint8 *)eventParam);
            break;
        }
        
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
       
        /* This event indicates peripheral device has started/stopped
           advertising. */
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        {
            DEBUG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP: ");
            
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
            {
                printf("Advertising... \r\n");
                Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_ON);
                Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_OFF);
                Cy_GPIO_Write(Connect_LED_0_PORT, Connect_LED_0_NUM, LED_OFF);
            }
            else if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
            {
                printf("Advertisement Stopped \r\n");
                Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_OFF);
                Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_ON);
                Cy_GPIO_Write(Connect_LED_0_PORT, Connect_LED_0_NUM, LED_OFF);
            }
            break;
        }
        
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device. */
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED: 
		{
			DEBUG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED \r\n");
            break;
		}  
        
        /* This event is generated when disconnected from remote device or 
           failed to establish connection. */
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:   
		{
            DEBUG_PRINTF("\r\n CY_BLE_EVT_GAP_DEVICE_DISCONNECTED \r\n");
            printf("Device Disconnected \r\n");
              
            /* Device disconnected; restart advertisement */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);          
            if(apiResult != CY_BLE_SUCCESS)
            {
                DEBUG_PRINTF("Start Advertisement API Error: %d \r\n", apiResult);
            }
            else
            {
                printf("Restart Advertising... \r\n");
                DEBUG_PRINTF("Start Advertisement API Success: %d \r\n", apiResult);
            }
            break;
		}          
        
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        /* This event is generated at the GAP Peripheral end after a connection is completed
           with a peer Central device. */    
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            #if DEBUG_UART_ENABLED
            connHandle = *(cy_stc_ble_conn_handle_t *)eventParam;
            
            DEBUG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: attId=%X, bdHandle=%X \r\n", 
                        connHandle.attId, 
                        connHandle.bdHandle);
            #endif
            printf("Connected to Device\r\n");
            Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_OFF);
            Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_OFF);
            Cy_GPIO_Write(Connect_LED_0_PORT, Connect_LED_0_NUM, LED_ON);
            break;
        }
        
        /* This event is generated at the GAP Peripheral end after 
           disconnection. */
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
        {
            DEBUG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND \r\n");
            Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_OFF);
            Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_ON);
            Cy_GPIO_Write(Connect_LED_0_PORT, Connect_LED_0_NUM, LED_OFF);
            break;
        }
        
        /* This event is triggered when there is a write request from 
        the Client device */
        case CY_BLE_EVT_GATTS_WRITE_REQ: 
        {
            writeData = (cy_stc_ble_gatt_write_param_t *)eventParam;
            
            /* Extract the data and write to GATT database */
            Cy_BLE_GATTS_WriteAttributeValuePeer(&cy_ble_connHandle[0],
                                         &writeData->handleValPair);
            
            /* Send a Write Response to GATT Client */
            Cy_BLE_GATTS_WriteRsp(cy_ble_connHandle[0]);
        
            /* XAxis Notify is enabled in the client device */
            if(writeData->handleValPair.attrHandle == CY_BLE_DRO_XAXIS_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
            {  
                XAxis_Notification_Enabled = writeData->handleValPair.value.val[0];
            }
            /* YAxis Notify is enabled in the client device */
            else if(writeData->handleValPair.attrHandle == CY_BLE_DRO_YAXIS_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
            {
                YAxis_Notification_Enabled = writeData->handleValPair.value.val[0];
            }
            /* ZAxis Notify is enabled in the client device */
            else if(writeData->handleValPair.attrHandle == CY_BLE_DRO_ZAXIS_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
            {
                ZAxis_Notification_Enabled = writeData->handleValPair.value.val[0];
            }
            
            /* Reset the counter value to zero, if SET_ZERO characteristic is write with value 1 */
            if(writeData->handleValPair.attrHandle == CY_BLE_DRO_SET_ZERO_CHAR_HANDLE)
            {
                if(writeData->handleValPair.value.val[0] == 1)
                {   
                    Cy_TCPWM_QuadDec_SetCounter(QuadDec_XAxis_HW, QuadDec_XAxis_CNT_NUM, 0);
                    Cy_TCPWM_QuadDec_SetCounter(QuadDec_YAxis_HW, QuadDec_YAxis_CNT_NUM, 0);
                    Cy_TCPWM_QuadDec_SetCounter(QuadDec_ZAxis_HW, QuadDec_ZAxis_CNT_NUM, 0);
                }
            }
            break;
        }
       
        /**********************************************************
        *                       Other Events
        ***********************************************************/
        default:
        {
            DEBUG_PRINTF("OTHER event: %lx \r\n", (unsigned long) event);
            break;
        }
	}
}

/******************************************************************************
* Function Name: main()
*******************************************************************************
*
* Summary: This is the system entrance point for Cortex-M4.
* This function initializes the PSoC Components.
*
* Parameters:
*  None
*
* Return:
*  int
*
* Side Effects:
*  None  
*
******************************************************************************/
int main(void)
{
    __enable_irq();/* Enable global interrupts. */
    
    /* Start the BLE */
    Cy_BLE_Start(&StackEventHandler);
    
    /* Configure the Timer to generate an interrupt for every 200ms */
    Timer_Tick_Start();
    Cy_SysInt_Init(&ISR_Tick_cfg, Timer_Tick_ISR);
    NVIC_EnableIRQ(ISR_Tick_cfg.intrSrc);
    NVIC_ClearPendingIRQ(ISR_Tick_cfg.intrSrc);
   
    /* Start UART_DEB component */
    UART_DEB_Start();
    
    printf("***************** PSoC6 DRO for Mills/Lathes *******"\
               "**********\r\n");
    printf("*****************************************************************"\
               "*****************\r\n\n");
        
    /* Initialize with config set in component and enable QuadDec_XAxis */
    Cy_TCPWM_QuadDec_Init(QuadDec_XAxis_HW, QuadDec_XAxis_CNT_NUM, &QuadDec_XAxis_config);
    Cy_TCPWM_Enable_Multiple(QuadDec_XAxis_HW,QuadDec_XAxis_CNT_MASK);
    /* Start QuadDec_XAxis */
    Cy_TCPWM_TriggerReloadOrIndex(QuadDec_XAxis_HW, QuadDec_XAxis_CNT_MASK);
    
    /* Initialize with config set in component and enable QuadDec_YAxis */
    Cy_TCPWM_QuadDec_Init(QuadDec_YAxis_HW, QuadDec_YAxis_CNT_NUM, &QuadDec_YAxis_config);
    Cy_TCPWM_Enable_Multiple(QuadDec_YAxis_HW, QuadDec_YAxis_CNT_MASK);
    /* Start QuadDec_YAxis */
    Cy_TCPWM_TriggerReloadOrIndex(QuadDec_YAxis_HW, QuadDec_YAxis_CNT_MASK);
    
    /* Initialize with config set in component and enable QuadDec_ZAxis */
    Cy_TCPWM_QuadDec_Init(QuadDec_ZAxis_HW, QuadDec_ZAxis_CNT_NUM, &QuadDec_ZAxis_config);
    Cy_TCPWM_Enable_Multiple(QuadDec_ZAxis_HW, QuadDec_ZAxis_CNT_MASK);
    /* Start QuadDec_ZAxis */
    Cy_TCPWM_TriggerReloadOrIndex(QuadDec_ZAxis_HW, QuadDec_ZAxis_CNT_MASK);
    
    for(;;)
    {
        /* Cy_Ble_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
        
        /* Check if Timer_Flag is enabled */
        if(Update_BLE_Client == UPDATE)
        {
            /* Reset the Timer_Flag to 0 */
            Update_BLE_Client = DONT_UPDATE;
            
            /* Get the counter values */
            Count_XAxis = Cy_TCPWM_QuadDec_GetCounter(QuadDec_XAxis_HW, QuadDec_XAxis_CNT_NUM) -
                QUADRATURE_DECODER_ZERO;
            Count_YAxis = Cy_TCPWM_QuadDec_GetCounter(QuadDec_YAxis_HW, QuadDec_YAxis_CNT_NUM) - 
                QUADRATURE_DECODER_ZERO;
            Count_ZAxis = Cy_TCPWM_QuadDec_GetCounter(QuadDec_ZAxis_HW, QuadDec_ZAxis_CNT_NUM) - 
                QUADRATURE_DECODER_ZERO;

            /* Send XAxis_Notify_Data to the BLE App */
            if(XAxis_Notification_Enabled)
            {
                if(Cy_BLE_GATT_GetBusyStatus(cy_ble_connHandle[0].attId) ==\
                       CY_BLE_STACK_STATE_FREE)
                {
                    /* Update Notification packet with the data. */
                    XAxis_Notify_Data.attrHandle = CY_BLE_DRO_XAXIS_CHAR_HANDLE;
                    XAxis_Notify_Data.value.val  = (uint8 *) &Count_XAxis;
                    XAxis_Notify_Data.value.len  = 4;
                    apiResult = Cy_BLE_GATTS_SendNotification(&cy_ble_connHandle[0], &XAxis_Notify_Data);
                    
                    /* Check if the operation has been successful */
                    if(apiResult == CY_BLE_SUCCESS)
                    {
                        Cy_BLE_ProcessEvents();
                    }
                    else
                    {
                        DEBUG_PRINTF("Failure! : BLE - Sending notification, Error code: %x", apiResult);
                    }
                }
            }
            /* Send YAxis_Notify_Data to the BLE App */
            if(YAxis_Notification_Enabled)
            {
                if(Cy_BLE_GATT_GetBusyStatus(cy_ble_connHandle[0].attId) ==\
                   CY_BLE_STACK_STATE_FREE)
                {
                    /* Update Notification packet with the data. */
                    YAxis_Notify_Data.attrHandle = CY_BLE_DRO_YAXIS_CHAR_HANDLE;
                    YAxis_Notify_Data.value.val  = (uint8 *) &Count_YAxis;
                    YAxis_Notify_Data.value.len  = 4;
                    apiResult = Cy_BLE_GATTS_SendNotification(&cy_ble_connHandle[0], &YAxis_Notify_Data);
                    
                    /* Check if the operation has been successful */
                    if(apiResult == CY_BLE_SUCCESS)
                    {
                        Cy_BLE_ProcessEvents();
                    }
                    else
                    {
                        DEBUG_PRINTF("Failure! : BLE - Sending notification, Error code: %x", apiResult);
                    }
                }
            }
            /* Send ZAxis_Notify_Data to the BLE App */
            if(ZAxis_Notification_Enabled)
            {
                if(Cy_BLE_GATT_GetBusyStatus(cy_ble_connHandle[0].attId) ==\
                   CY_BLE_STACK_STATE_FREE)
                {
                    /* Update Notification packet with the data. */
                    ZAxis_Notify_Data.attrHandle = CY_BLE_DRO_ZAXIS_CHAR_HANDLE;
                    ZAxis_Notify_Data.value.val  = (uint8 *) &Count_ZAxis;
                    ZAxis_Notify_Data.value.len  = 4;
                    apiResult = Cy_BLE_GATTS_SendNotification(&cy_ble_connHandle[0], &ZAxis_Notify_Data);
                    
                    /* Check if the operation has been successful */
                    if(apiResult == CY_BLE_SUCCESS)
                    {
                        Cy_BLE_ProcessEvents();
                    }
                    else
                    {
                        DEBUG_PRINTF("Failure! : BLE - Sending notification, Error code: %x", apiResult);
                    }
                }
            }
        }
    }
}

/*******************************************************************************
* Function Name: Timer_Tick_Isr()
********************************************************************************
* Summary:
* Interrupt service routine for the timer block.
*
* Parameters:
* None
*
* Return:
* None
*
*******************************************************************************/
void Timer_Tick_ISR(void)
{
    /* Signal main for loop to update the connected BLE Client */
    Update_BLE_Client = UPDATE;
    
    /* Clears the Timer_Tick interrupt */
    Timer_Tick_ClearInterrupt(CY_TCPWM_INT_ON_TC);
    NVIC_ClearPendingIRQ(ISR_Tick_cfg.intrSrc);
}

/* [] END OF FILE */