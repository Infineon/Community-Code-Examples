/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Crypto Uart Receiver Application
*              for ModusToolbox.
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

/* Header file includes */
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "rsa.h"

/* Macro to interchange UART SCB Peripheral used */
#define UART_INVERT 0

#define MAX_MESSAGE_SIZE                     (100u)

/* Size of the message block that can be processed by Crypto hardware for
 * AES encryption */
#define AES128_ENCRYPTION_LENGTH             (uint32_t)(16u)

#define AES128_KEY_LENGTH                    (uint32_t)(16u)

/* Hash escape character size */
#define ESCAPE_CHAR_SIZE    5u

/* End of packet token size */
#define EOP_SIZE            3u

/* Size of the message digest for SHA-256 encryption */
#define MESSAGE_DIGEST_SIZE                 (32u)

/* Size of the AES encrypted message */
#define ENCRYPTED_MESSAGE_SIZE    ((((MAX_MESSAGE_SIZE + ESCAPE_CHAR_SIZE + \
                                    MESSAGE_DIGEST_SIZE) / AES128_ENCRYPTION_LENGTH) \
                                    + 1) * AES128_ENCRYPTION_LENGTH) + EOP_SIZE

/* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
#define CLEAR_SCREEN         "\x1b[2J\x1b[;H"

#define END_OF_PROCESS "\r\n***************************************************************************\r\n"

cy_stc_crypto_aes_state_t aes_state;
cyhal_uart_t uart_obj;
/* OK - Success */
uint8_t OK_buffer[2] = {'O', 'K'};
/* NC - Failed */
uint8_t NC_buffer[2] = {'N', 'C'};
/* Buffer to save the Receiver data */
uint8_t receive_buffer[2];

/* Key used for AES encryption*/
CY_ALIGN(4) uint8_t AES_Key[AES128_KEY_LENGTH];

/* Variables to hold the user message and the corresponding encrypted message */
CY_ALIGN(4) uint8_t message[MAX_MESSAGE_SIZE];

/* Buffer to store the encrypted and decrypted message */
CY_ALIGN(4) uint8_t encrypted_msg[ENCRYPTED_MESSAGE_SIZE] ;
CY_ALIGN(4) uint8_t decrypted_msg[ENCRYPTED_MESSAGE_SIZE];

uint8_t message_pt[MAX_MESSAGE_SIZE + 1];

/* Escape character for hash comparison */
uint8_t escapeChar[] = "HASH:";

/* End of Packet */
uint8_t EOP_char[] = "EOP";

/* Buffer to store the hash received and calculated to validate message integrity */
CY_ALIGN(4) uint8_t hash_decoded[MESSAGE_DIGEST_SIZE];
CY_ALIGN(4) uint8_t hash[MESSAGE_DIGEST_SIZE];

/* Buffer to hold the output of RSA decryption */
CY_ALIGN(4) uint8_t rsaOutput[RSA_MODULO_DATA_SIZE];

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void uart_init();
cy_rslt_t key_exchange();
void decrypt_message(uint8_t* message, uint8_t size);
void print_data(uint8_t* data, uint16_t len);
void uart_send_data(uint8_t* data_buffer, uint16_t buffer_length);
void uart_get_data(uint8_t* data_buffer, uint16_t buffer_length, uint32_t timeout);

/*******************************************************************************
 * Function Name: main
 *******************************************************************************
 * Summary: Application execution starts here. This function starts the UART and
 *  Crypto blocks. The AES key is received from the sender and this key is 
 *  decrypted using the RSA private key.
 *  Once the AES key is received successfully, encrypted message transfer is 
 *  started between the sender and receiver and the message is decoded using AES
 *
 ******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    cy_en_crypto_status_t cryptoStatus;
    uint32 message_index = 0;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    
    /* Initialize the UART peripheral */
    uart_init();

    __enable_irq();

    /* Enable the Crypto block */
    cryptoStatus = Cy_Crypto_Core_Enable(CRYPTO);
    if(cryptoStatus != CY_CRYPTO_SUCCESS)
	{
		CY_ASSERT(0);
	}

    printf(CLEAR_SCREEN);
    printf(END_OF_PROCESS);

    printf("\r\nStarting Crypto Receiver Application\r\n");

    /* Receive the key used for AES decryption */
    while(CY_RSLT_SUCCESS != key_exchange());    

    printf("\r\nKey exchange successful!!!\r\n");
    printf(END_OF_PROCESS);

    for (;;)
    {
        message_index = 0;
        memset(encrypted_msg, 0, MAX_MESSAGE_SIZE);
        memset(decrypted_msg, 0, MAX_MESSAGE_SIZE);

        /* Receive the AT command before receiving the message */
        uart_get_data(receive_buffer, 2, 0);
        while(receive_buffer[0] != 'A' || receive_buffer[1] != 'T')
        {
            printf("\r\nCommunication failed!\r\n");
            uart_get_data(receive_buffer, 2, 0);
        }

        /* Send OK to Sender */
        uart_send_data(OK_buffer, 2);
        printf("OK sent...\r\nRecieving encrypted Message... \r\n");
        
        /* Receive the AES encrypted message */
        cyhal_uart_getc(&uart_obj, &encrypted_msg[message_index++], 0);
        cyhal_uart_getc(&uart_obj, &encrypted_msg[message_index++], 0);
        cyhal_uart_getc(&uart_obj, &encrypted_msg[message_index++], 0);
        while(encrypted_msg[message_index - 1] != EOP_char[2] ||
              encrypted_msg[message_index - 2] != EOP_char[1] ||
              encrypted_msg[message_index - 3] != EOP_char[0])
        {
            if(message_index < ENCRYPTED_MESSAGE_SIZE)
            {
                cyhal_uart_getc(&uart_obj, &encrypted_msg[message_index++], 0);
            }
            else
            {
                printf("Message Size exceeded!!!\r\n");
                message_index = 0xFFFF;
                break;
            }
        }
        if(message_index <= ENCRYPTED_MESSAGE_SIZE)
        {
            printf("Message Recieved\r\n");
            print_data(encrypted_msg, message_index);

            /* Decrypt the message */
            decrypt_message(encrypted_msg, message_index-3);
        }
        uart_send_data(OK_buffer, 2);
        printf(END_OF_PROCESS);
    }
}

/*******************************************************************************
 * Function Name: uart_init
 *******************************************************************************
 * Summary: Function to initialize the UART blocks. Two UART blocks are 
 *  initialized. One for retarget IO to print the data to user and the other for
 *  encrypted communication with the sender application.
 *
 ******************************************************************************/
void uart_init()
{
    cy_rslt_t result;

    /* Initialize the UART block for retarget IO */
    #if UART_INVERT
        result = cy_retarget_io_init(P12_1, P12_0, 115200);
    #else
        result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 115200);
    #endif
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize UART for communication with Sender */
    const cyhal_uart_cfg_t uart_config =
    {
        .data_bits = 8,
        .stop_bits = 1,
        .parity = CYHAL_UART_PARITY_NONE,
        .rx_buffer = NULL,
        .rx_buffer_size = 0,
    };

    #if UART_INVERT
        result = cyhal_uart_init(&uart_obj, CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, NULL, &uart_config);
    #else
        result = cyhal_uart_init(&uart_obj, P12_1, P12_0, NULL, &uart_config);
    #endif
    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_uart_set_baud(&uart_obj, 115200, NULL);
    }
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

/*******************************************************************************
 * Function Name: key_exchange
 *******************************************************************************
 * Summary: Function to exchange the key between sender and receiver. The sender
 *  has the RSA public key of the receiver and encrypts the randomly generated 
 *  AES key. The receiver receives this and decrypts it using its private key.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  cy_rslt_t - returns if the message was successfully received or not.
 *
 ******************************************************************************/
cy_rslt_t key_exchange()
{    
    printf("Waiting for AT command from sender...\r\n");
    uart_get_data(receive_buffer, 2, 100);
    while(receive_buffer[0] != 'A' || receive_buffer[1] != 'T')
    {
        uart_get_data(receive_buffer, 2, 100);
        CyDelay(500);
    }

    uart_send_data(OK_buffer, 2);
    printf("OK sent...\r\nReceiving encrypted Key... \r\n");

    /* Recieve the RSA encrypted Key */
    uart_get_data(encrypted_key, RSA_MODULO_DATA_SIZE, 10000);
    printf ("\r\nReceived Key =");
    print_data(encrypted_key, RSA_MODULO_DATA_SIZE);

    /* Decrypt the message using RSA to obtain the AES key */
    rsa_decrypt_key();
    printf("\r\nRSA Decryption successful.\r\n");

    /* Copy the key to AES_KEY buffer */
    for(uint8 i = 0; i < AES128_KEY_LENGTH; i++)
    {
        AES_Key[i] = rsaOutput[i];
    }

    /* Calculate the hash of the key and transmit it to the sender
     * for validating the key exchange */
    Cy_Crypto_Core_Sha(CRYPTO, AES_Key, AES128_KEY_LENGTH, hash, CY_CRYPTO_MODE_SHA256);
    uart_send_data(hash, sizeof(hash));
    
    uart_get_data(receive_buffer, 2, 0);
    if(receive_buffer[0] != 'O' || receive_buffer[1] != 'K')
    {
        if(receive_buffer[0] != 'N' || receive_buffer[1] != 'C')
        {
            printf("Hash error!!\r\nRetrying Key exchange\r\n");
        }
        else
        {
            printf("\r\nCommunication failed!\r\n");
        }
        return CY_RSLT_TYPE_ERROR;
    }
    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
 * Function Name: decrypt_message
 *******************************************************************************
 * Summary: Function used to decrypt the message. Once decoded, it checks for
 *  the escape character "HASH:" and compares the hash of the message to check 
 *  the integrity of the message.
 *
 * Parameters:
 *  char * message - pointer to the message to be decrypted
 *  uint8_t size   - size of message to be decrypted.
 *
 ******************************************************************************/
void decrypt_message(uint8_t* message, uint8_t size)
{
    uint8_t AES_block_count = 0;
    uint8 message_size = 0;
    int i = 0, j = 0;

    AES_block_count =  (size % AES128_ENCRYPTION_LENGTH == 0) ?
                       (size / AES128_ENCRYPTION_LENGTH)
                       : (1 + size / AES128_ENCRYPTION_LENGTH);

    /* Initializes the AES operation by setting key and key length */
    Cy_Crypto_Core_Aes_Init(CRYPTO, AES_Key, CY_CRYPTO_KEY_AES_128, &aes_state);

    /* Start decryption operation*/
    for (i = 0; i < AES_block_count ; i++)
    {
        /* Perform AES ECB Decryption mode of operation */
        Cy_Crypto_Core_Aes_Ecb(CRYPTO, CY_CRYPTO_DECRYPT,
                               (decrypted_msg + AES128_ENCRYPTION_LENGTH * i),
                               (message + AES128_ENCRYPTION_LENGTH * i),
                               &aes_state);

        /* Wait for Crypto Block to be available */
        Cy_Crypto_Core_WaitForReady(CRYPTO);
    }

    decrypted_msg[size]='\0';
     
    /* Print the decrypted message on the UART terminal */
    printf("\r\nResult of Decryption:\r\n\n");
    print_data(decrypted_msg, size);

    /* Check for the location of the escape sequence and retrieve the message 
     * and the hash.The escape sequence in this example is "HASH".
     */
    for(i = 0; i < size - ESCAPE_CHAR_SIZE; i++)
    {
        for(j = 0; j < ESCAPE_CHAR_SIZE; j++)
        {
            if (decrypted_msg[i+j] != escapeChar[j])
            {
                break;
            }
        }
        if(j == ESCAPE_CHAR_SIZE)
        {
            message_size = i;
            for (uint8 k = 0; k < i; k++)
            {
                message_pt[k] = decrypted_msg[k]; 
            }
            for (uint8 k = i + ESCAPE_CHAR_SIZE, j=0; k < size + 1; k++)
            {
                hash_decoded [j++]= decrypted_msg[k]; 
            }
            message_pt[i] = '\0';
            break;
        }
    }

    /* Calculate the Hash of the message that was received */
    Cy_Crypto_Core_Sha(CRYPTO, message_pt, message_size, 
                       hash, CY_CRYPTO_MODE_SHA256);
    
    /* Compare the calculated hash with the hash value received with the message
     * to check for integrity of the message 
     */
    for(i = 0; i < MESSAGE_DIGEST_SIZE; i++)
    {
        if(hash[i] != hash_decoded[i])
        {
            printf ("\r\n Hash Error!!!");
            return;
        }
    }

    /* Hash validation was successful */
    printf(END_OF_PROCESS);

    printf("\r\nMessage: \r\n");
    printf("%s \r\n", message_pt);

    Cy_Crypto_Core_Aes_Free(CRYPTO, &aes_state);
}

/*******************************************************************************
 * Function Name: print_data
 *******************************************************************************
 * Summary: Function used to display the data in hexadecimal format
 *
 * Parameters:
 *  data - Pointer to location of data to be printed
 *  len  - length of data to be printed
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void print_data(uint8_t* data, uint16_t len)
{
    char print[10];
    for (uint32 i=0; i < len; i++)
    {
        if ((i % 16) == 0)
        {
            printf("\r\n");
        }
        sprintf(print,"0x%02X ", *(data+i));
        printf("%s", print);
    }
    printf("\r\n");
}

/*******************************************************************************
 * Function Name: rsa_decrypt_key
 *******************************************************************************
 * Summary: Function used to decrypt the message. Once decoded, it checks for
 *          the escape character "HASH:" and compares the hash of the message to
 *          check the integrity of the message.
 *
 * Parameters:
 *  char * message - pointer to the message to be decrypted
 *  uint8_t size   - size of message to be decrypted.
 *
 ******************************************************************************/
void rsa_decrypt_key(void)
{
    cy_en_crypto_status_t           cryptoStatus;

    /* Key modulus and exponents are placed as BIG-endian data. Invert the data
     * before using.
     */
    Cy_Crypto_Rsa_InvertEndianness(modulus,         RSA_MODULO_DATA_SIZE);
    Cy_Crypto_Rsa_InvertEndianness(privateExponent, RSA_MODULO_DATA_SIZE);

    /* Copy the data from the buffer to the relevant structures */
    cy_privateKey.keyStruct.pubExpPtr       = privateExponent;
    cy_privateKey.keyStruct.pubExpLength    = RSA_MODULO_LENGTH;
    cy_privateKey.keyStruct.moduloPtr       = modulus;

    cy_privateKey.keyStruct.barretCoefPtr    = cy_privateKey.barretCoeff;
    cy_privateKey.keyStruct.inverseModuloPtr = cy_privateKey.invModulo;
    cy_privateKey.keyStruct.rBarPtr          = cy_privateKey.rBarData;
    
    /* Calculate key's coefficients to speedup RSA processing for long (multiblock) 
     * messages */
    cryptoStatus = Cy_Crypto_Core_Rsa_Coef(CRYPTO, &cy_privateKey.keyStruct);
    if(cryptoStatus != CY_CRYPTO_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Decrypt using RSA */
    cryptoStatus = Cy_Crypto_Core_Rsa_Proc(CRYPTO,
                        (cy_stc_crypto_rsa_pub_key_t *)&cy_privateKey.keyStruct,
                        encrypted_key, sizeof(encrypted_key),
                        rsaOutput);
    
    if(cryptoStatus != CY_CRYPTO_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

/*******************************************************************************
 * Function Name: uart_send_data
 *******************************************************************************
 * Summary:
 *  Function to send data to the receiver.
 *
 * Parameters:
 *  data_buffer - buffer containing the data to be sent
 *  buffer_length  - number of bytes to send
 *
 ******************************************************************************/
void uart_send_data(uint8_t* data_buffer, uint16_t buffer_length)
{
    for(int i = 0; i < buffer_length; i++)
    {
        cyhal_uart_putc(&uart_obj, data_buffer[i]);
    }
}

/*******************************************************************************
 * Function Name: uart_get_data
 ********************************************************************************
 * Summary:
 *  Function to receive data from the receiver.
 *
 * Parameters:
 *  data_buffer - buffer to store received data
 *  buffer_length  - number of bytes to receive
 *  timeout - UART timeout. Wait indefinitely if set to 0
 *
 *******************************************************************************/

void uart_get_data(uint8_t* data_buffer, uint16_t buffer_length, uint32_t timeout)
{
    uint8_t recv_value = 0;
    for(int i = 0; i < buffer_length; i++)
    {
        cyhal_uart_getc(&uart_obj, &recv_value, timeout);
        data_buffer[i] = recv_value;
    }
}

/* [] END OF FILE */