/******************************************************************************
* File Name:   tcp_client.c
*
* Description: This file contains task and functions related to TCP client
* operation.
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

/* Header file includes. */
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/* FreeRTOS header file. */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

/* Standard C header file. */
#include <string.h>

/* Cypress secure socket header file. */
#include "cy_secure_sockets.h"

/* Wi-Fi connection manager header files. */
#include "cy_wcm.h"
#include "cy_wcm_error.h"

/* TCP client task header file. */
#include "tcp_client.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* Maximum number of connection retries to the TCP server. */
#define MAX_TCP_SERVER_CONN_RETRIES        (5u)

/* Length of the TCP data packet. */
#define MAX_TCP_DATA_PACKET_LENGTH         (200)

/* Length of the LED ON/OFF command issued from the TCP server. */
#define TCP_LED_CMD_LEN                    (1)
#define TCP_DATA_ON                        '1'
#define TCP_STOP_DATA                      '0'
#define MSG_INVALID_CMD                    "Invalid command"

/*******************************************************************************
* Function Prototypes
********************************************************************************/
cy_rslt_t create_tcp_client_socket();
cy_rslt_t tcp_client_recv_handler(cy_socket_t socket_handle, void *arg);
cy_rslt_t tcp_disconnection_handler(cy_socket_t socket_handle, void *arg);
cy_rslt_t connect_to_tcp_server(cy_socket_sockaddr_t address);
cy_rslt_t connect_to_wifi_ap(void);

/*******************************************************************************
* Global Variables
********************************************************************************/
/* TCP client socket handle */
cy_socket_t client_handle;

/* Binary semaphore handle to keep track of TCP server connection. */
SemaphoreHandle_t connect_to_server;

/* Queue which has stored up the data from capsense slider */
QueueHandle_t tcp_command_data_q;

/* These two are made as global variables because if the variables are inside the client receive callback
   then they would get erased each time and the previous data would not be retained.
   The tcp_server previous variable stores the server permission and 
   the tcp_can_send stores the capsense transmission enable/disable data*/
bool tcp_can_send = false;
bool tcp_server_previous = false;
/*******************************************************************************
 * Function Name: tcp_client_task
 *******************************************************************************
 * Summary:
 *  Task used to establish a connection to a remote TCP server and
 *  control the LED state (ON/OFF) based on the command received from TCP server
 *  and send the data read from the capsense slider 
 * Parameters:
 *  void *args : Task parameter defined during task creation (unused).
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void tcp_client_task(void *arg)
{
    cy_rslt_t result ;
    printf("inside TCP task\n");
    /* IP address and TCP port number of the TCP server to which the TCP client
     * connects to. 
     */
    cy_socket_sockaddr_t tcp_server_address =
    {
        .ip_address.ip.v4 = TCP_SERVER_IP_ADDRESS,
        .ip_address.version = CY_SOCKET_IP_VER_V4,
        .port = TCP_SERVER_PORT
    };

    /* Create a binary semaphore to keep track of TCP server connection. */
    connect_to_server = xSemaphoreCreateBinary();

    /* Give the semaphore so as to connect to TCP server.  */
    xSemaphoreGive(connect_to_server);

    /* Connect to Wi-Fi AP */
    if(connect_to_wifi_ap() != CY_RSLT_SUCCESS )
    {
        printf("\n Failed to connect to Wi-FI AP.\n");
        CY_ASSERT(0);
    }

    /* Initialize secure socket library. */
    result = cy_socket_init();
    if (result != CY_RSLT_SUCCESS)
    {
        printf("Secure Socket initialization failed!\n");
        CY_ASSERT(0);
    }
    printf("Secure Socket initialized\n");

    for(;;)
    {
        /* Wait till semaphore is acquired so as to connect to a TCP server. */
        xSemaphoreTake(connect_to_server, portMAX_DELAY);

        /* Connect to the TCP server. If the connection fails, retry
         * to connect to the server for MAX_TCP_SERVER_CONN_RETRIES times.
         */
        printf("Connecting to TCP server...\n");
        result = connect_to_tcp_server(tcp_server_address);

        if(result != CY_RSLT_SUCCESS)
        {
            printf("Failed to connect to TCP server.\n");
            CY_ASSERT(0);
        }
    }
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

    cy_wcm_config_t wifi_config = { .interface = CY_WCM_INTERFACE_TYPE_STA };

    cy_wcm_ip_address_t ip_address;

     /* Initialize Wi-Fi connection manager. */
    result = cy_wcm_init(&wifi_config);

    if (result != CY_RSLT_SUCCESS)
    {
        printf("Wi-Fi Connection Manager initialization failed!\n");
        return result;
    }
    printf("Wi-Fi Connection Manager initialized.\r\n");

     /* Set the Wi-Fi SSID, password and security type. */
    memset(&wifi_conn_param, 0, sizeof(cy_wcm_connect_params_t));
    memcpy(wifi_conn_param.ap_credentials.SSID, WIFI_SSID, sizeof(WIFI_SSID));
    memcpy(wifi_conn_param.ap_credentials.password, WIFI_PASSWORD, sizeof(WIFI_PASSWORD));
    wifi_conn_param.ap_credentials.security = WIFI_SECURITY_TYPE;

    /* Join the Wi-Fi AP. */
    for(uint32_t conn_retries = 0; conn_retries < MAX_WIFI_CONN_RETRIES; conn_retries++ )
    {
        result = cy_wcm_connect_ap(&wifi_conn_param, &ip_address);

        if(result == CY_RSLT_SUCCESS)
        {
            printf("Successfully connected to Wi-Fi network '%s'.\n",
                                wifi_conn_param.ap_credentials.SSID);
            printf("IP Address Assigned: %d.%d.%d.%d\n", (uint8)ip_address.ip.v4,
                    (uint8)(ip_address.ip.v4 >> 8), (uint8)(ip_address.ip.v4 >> 16),
                    (uint8)(ip_address.ip.v4 >> 24));
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

/*******************************************************************************
 * Function Name: create_tcp_client_socket
 *******************************************************************************
 * Summary:
 *  Function to create a socket and set the socket options
 *  to set call back function for handling incoming messages, call back
 *  function to handle disconnection.
 *
 *******************************************************************************/
cy_rslt_t create_tcp_client_socket()
{
    cy_rslt_t result;

    /* Variables used to set socket options. */
    cy_socket_opt_callback_t tcp_recv_option;
    cy_socket_opt_callback_t tcp_disconnect_option;

    /* Create a new secure TCP socket. */
    result = cy_socket_create(CY_SOCKET_DOMAIN_AF_INET, CY_SOCKET_TYPE_STREAM,
                              CY_SOCKET_IPPROTO_TCP, &client_handle);

    if (result != CY_RSLT_SUCCESS)
    {
        printf("Failed to create socket!\n");
        return result;
    }

    /* Register the callback function to handle messages received from TCP server. */
    tcp_recv_option.callback = tcp_client_recv_handler;
    tcp_recv_option.arg = NULL;
    result = cy_socket_setsockopt(client_handle, CY_SOCKET_SOL_SOCKET,
                                  CY_SOCKET_SO_RECEIVE_CALLBACK,
                                  &tcp_recv_option, sizeof(cy_socket_opt_callback_t));
    if (result != CY_RSLT_SUCCESS)
    {
        printf("Set socket option: CY_SOCKET_SO_RECEIVE_CALLBACK failed\n");
        return result;
    }

    /* Register the callback function to handle disconnection. */
    tcp_disconnect_option.callback = tcp_disconnection_handler;
    tcp_disconnect_option.arg = NULL;

    result = cy_socket_setsockopt(client_handle, CY_SOCKET_SOL_SOCKET,
                                  CY_SOCKET_SO_DISCONNECT_CALLBACK,
                                  &tcp_disconnect_option, sizeof(cy_socket_opt_callback_t));
    if(result != CY_RSLT_SUCCESS)
    {
        printf("Set socket option: CY_SOCKET_SO_DISCONNECT_CALLBACK failed\n");
    }

    return result;
}

/*******************************************************************************
 * Function Name: connect_to_tcp_server
 *******************************************************************************
 * Summary:
 *  Function to connect to TCP server.
 *
 * Parameters:
 *  cy_socket_sockaddr_t address: Address of TCP server socket
 *
 * Return:
 *  cy_result result: Result of the operation
 *
 *******************************************************************************/
cy_rslt_t connect_to_tcp_server(cy_socket_sockaddr_t address)
{
    cy_rslt_t result = CY_RSLT_MODULE_SECURE_SOCKETS_TIMEOUT;
    cy_rslt_t conn_result;

    for(uint32_t conn_retries = 0; conn_retries < MAX_TCP_SERVER_CONN_RETRIES; conn_retries++)
    {
        /* Create a TCP socket */
        conn_result = create_tcp_client_socket();
        
        if(conn_result != CY_RSLT_SUCCESS)
        {
            printf("Socket creation failed!\n");
            CY_ASSERT(0);
        }

        conn_result = cy_socket_connect(client_handle, &address, sizeof(cy_socket_sockaddr_t));
        
        if (conn_result == CY_RSLT_SUCCESS)
        {
            printf("============================================================\n");
            printf("Connected to TCP server\n");

            return conn_result;
        }

        printf("Could not connect to TCP server.\n");
        printf("Trying to reconnect to TCP server... Please check if the server is listening\n");

        /* The resources allocated during the socket creation (cy_socket_create)
         * should be deleted.
         */
        cy_socket_delete(client_handle);
    }

     /* Stop retrying after maximum retry attempts. */
     printf("Exceeded maximum connection attempts to the TCP server\n");

     return result;
}

/*******************************************************************************
 * Function Name: tcp_client_recv_handler
 *******************************************************************************
 * Summary:
 *  Callback function to handle incoming TCP server messages and respond back 
 *  appropriately with the required data read from the capsense buttons/slider.
 *
 * Parameters:
 *  cy_socket_t socket_handle: Connection handle for the TCP client socket
 *  void *args : Parameter passed on to the function (unused)
 *
 * Return:
 *  cy_result result: Result of the operation
 *
 *******************************************************************************/
cy_rslt_t tcp_client_recv_handler(cy_socket_t socket_handle, void *arg)
{

    BaseType_t rtos_api_result;
    tcp_command_data_t tcp_data;
    /* Variable to store number of bytes send to the TCP server. */
    uint32_t bytes_sent = 0;
    /* Variable to store number of bytes received. */
    uint32_t bytes_received = 0;

    char message_buffer[MAX_TCP_DATA_PACKET_LENGTH];
    cy_rslt_t result ;

    printf("============================================================\n");
    result = cy_socket_recv(socket_handle, message_buffer, TCP_LED_CMD_LEN,
                            CY_SOCKET_FLAGS_NONE, &bytes_received);
    if(message_buffer[0] == TCP_DATA_ON)
        tcp_server_previous = true;
    else if(message_buffer[0] == TCP_STOP_DATA)
        tcp_server_previous = false;

    rtos_api_result = xQueueReceive(tcp_command_data_q, &tcp_data,
                                portMAX_DELAY);
    if(rtos_api_result == pdTRUE)
    {
        switch (tcp_data.command)
        {
            case TCP_send_data:
                // This case is for the data read from the capsense slider.
                if(tcp_server_previous && tcp_can_send)
                {
                    // Send the capsense slider data when the server is receiving.
                    printf("data = %d\n",tcp_data.TCP_data);
                    sprintf(message_buffer,"%d",(tcp_data.TCP_data));
                }
                else
                {
                    if(message_buffer[0] == TCP_STOP_DATA)
                    {
                        // Do not send the capsense slider data when data reception is not allowed by TCP server.
                        printf("Data not being sent as server isn't receiving\n");
                        sprintf(message_buffer,"Data not being sent as server isn't receiving");
                    }
                    else if(!tcp_can_send)
                    {
                        // Do not send the capsense slider data when trasmission is diabled by the capsense buttons.
                        printf("Data not being sent. Please enable in Capsense\n");
                        sprintf(message_buffer,"Data not being sent. Please enable in Capsense");
                    }
                    else
                    {
                        // This message is sent out when capsense slider is touched if commands other '1' or '0' is sent from server
                        printf("Invalid command\n");
                        sprintf(message_buffer, MSG_INVALID_CMD);
                    }

                }
                break;
            case TCP_stop_send:
                // This case is for the data trasmission to stop from slider.
                tcp_can_send = false;
                printf("Data send will be stopped. Enable in capsense to send again\n");
                sprintf(message_buffer, "Data send will be stopped. Enable in capsense to send again");
                break;
            case TCP_send_ready:
                // This case is for the data trasmission to start from slider.
                tcp_can_send = true;
                printf("Data will be sent from capsense\n");
                sprintf(message_buffer, "Capsense data will be sent");
                break;
            default:
                break;
        }
    }
    /* Send acknowledgement to the TCP server in receipt of the message received. */
    result = cy_socket_send(socket_handle, message_buffer, strlen(message_buffer),
                                CY_SOCKET_FLAGS_NONE, &bytes_sent);
        if(result == CY_RSLT_SUCCESS)
        {
            printf("Acknowledgement sent to TCP server\n");
        }

    return result;
}

/*******************************************************************************
 * Function Name: tcp_disconnection_handler
 *******************************************************************************
 * Summary:
 *  Callback function to handle TCP socket disconnection event.
 *
 * Parameters:
 *  cy_socket_t socket_handle: Connection handle for the TCP client socket
 *  void *args : Parameter passed on to the function (unused)
 *
 * Return:
 *  cy_result result: Result of the operation
 *
 *******************************************************************************/
cy_rslt_t tcp_disconnection_handler(cy_socket_t socket_handle, void *arg)
{
    cy_rslt_t result;

    /* Disconnect the TCP client. */
    result = cy_socket_disconnect(socket_handle, 0);

    /* Free the resources allocated to the socket. */
    cy_socket_delete(socket_handle);

    printf("Disconnected from the TCP server! \n");

    /* Give the semaphore so as to connect to TCP server.  */
    xSemaphoreGive(connect_to_server);

    return result;
}

/* [] END OF FILE */
