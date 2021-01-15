/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Crypto Sender Application
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "rsa.h"

/*******************************************************************************
* Macros
*******************************************************************************/

/* Macro to interchange UART SCB Peripheral used */
#define TEST_UART_INVERT 0

/* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
#define CLEAR_SCREEN                            "\x1b[2J\x1b[;H"

#define END_OF_PROCESS "\r\n***************************************************************************\r\n"

/* Size of the message digest for SHA-256 encryption */
#define MESSAGE_DIGEST_SIZE                     (32u)

/* Size of the message block that can be processed by Crypto hardware for AES encryption */
#define AES128_ENCRYPTION_LENGTH                (uint32_t)(16u)

/* Length of AES Key */
#define AES128_KEY_LENGTH                       (uint32_t)(16u)

/* Define to truncate TRNG value to 8 bits */
#define EIGHT_BIT_MASK                          (0xFF)

/* Number of bytes per line to be printed on the UART terminal */
#define BYTES_PER_LINE                          (16u)

/* Maximum size of message to be considered */
#define MAX_MESSAGE_SIZE                        (100u)

/* Size of EOM padding buffer */
#define END_OF_MSG_CHARACTER_SIZE               (5u)

/*******************************************************************************
* Global Variables
*******************************************************************************/

/* Key used for AES encryption*/
CY_ALIGN(4) uint8_t AES_Key[AES128_KEY_LENGTH];
CY_ALIGN(4) uint8_t AES_Key_hash[MESSAGE_DIGEST_SIZE];

/* RSA encrypted AES Key */
CY_ALIGN(4) uint8_t rsaOutput[RSA_MODULO_DATA_SIZE];

CY_ALIGN(4) uint8_t message[MAX_MESSAGE_SIZE];
CY_ALIGN(4) uint8_t hash[MESSAGE_DIGEST_SIZE];
CY_ALIGN(4) uint8_t payload[MAX_MESSAGE_SIZE + MESSAGE_DIGEST_SIZE + END_OF_MSG_CHARACTER_SIZE];

CY_ALIGN(4) uint8_t AES_encrypted_msg[MAX_MESSAGE_SIZE + MESSAGE_DIGEST_SIZE + END_OF_MSG_CHARACTER_SIZE];

/* Stores the block count of AES encrypted payload */
uint8_t AES_block_count_msg = 0;

/* Buffers to store command */
/* AT - To initiate communication with receiver */
uint8_t AT_buffer[2] = {'A', 'T'};
/* OK - Success */
uint8_t OK_buffer[2] = {'O', 'K'};
/* NC - Failed */
uint8_t NC_buffer[2] = {'N', 'C'};
/* EOP - End of payload */
uint8_t EOP_buffer[3] = {'E', 'O', 'P'};

/* Buffer to save the Receiver data */
uint8_t receive_buffer[MESSAGE_DIGEST_SIZE];

uint8 message_size = 0;
uint8 payload_size = 0;

cy_stc_crypto_aes_state_t aes_state;

cyhal_uart_t uart_obj;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void generate_key(void);
void rsa_encrypt_key(void);
void generate_hash(uint8_t* message, uint8_t size, uint8_t* hash);
void aes_encrypt_data(uint8_t* message, uint8_t size, uint8_t* AES_buffer, uint8_t* AES_block_count);

cy_rslt_t sender_key_exchange();
void create_payload(uint8_t* message, uint8_t msg_size, uint8_t* hash, uint8_t hash_size, uint8_t* payload, uint8_t* pld_size);

void uart_init();
void uart_send_data(uint8_t* data_buffer, uint16_t buffer_length);
cy_rslt_t uart_get_data(uint8_t* data_buffer, uint16_t buffer_length, uint32_t timeout);
void print_data(uint8_t* data, uint16_t len);

int main(void)
{
    cy_rslt_t result;
    bool message_complete = false;
    char temp;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    
    uart_init();


    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf(CLEAR_SCREEN);

    printf(END_OF_PROCESS);

    printf("\r\nStarting Crypto Sender Application\r\n");

    while(CY_RSLT_SUCCESS != sender_key_exchange());

    for (;;)
    {
        printf(END_OF_PROCESS);
        printf("\r\nPlease enter the message (less than 100 characters) to be sent:\r\n");
        message_complete = false;
        /* Read upto 100 characters */
        for(message_size = 0; message_size < MAX_MESSAGE_SIZE; message_size++)
        {
            temp = getchar();
            if(temp == '\r')
            {
                message_complete = true;
                message[message_size] = '\0';
                break;
            }
            else
            {
                message[message_size] = temp;
            }
        }
        if(!message_complete)
        {
            /* Ignore if more than 100 digits are typed */
            message[message_size] = '\0';
            while ((getchar()) != '\r');

        }

        /* Print the message */
        printf("\r\nMessage:\r\n%s\r\n", message);

        printf("\r\n");
        printf(END_OF_PROCESS);

        /* Generate HASH of the message */
        generate_hash(message, message_size, hash);

        printf("\r\nHash successfully generated\r\n");

        /* Print the generated Message HASH */
        printf("\r\nHash:\r\n");
        print_data(hash, MESSAGE_DIGEST_SIZE);

        printf(END_OF_PROCESS);

        /* Append Message with its HASH to create payload */
        create_payload(message, message_size, hash, MESSAGE_DIGEST_SIZE, payload, &payload_size);

        printf("\r\nPayload successfully created\r\n");

        /* Print the payload */
        printf("\r\nPayload:\r\n");
        print_data(payload, payload_size);
        printf(END_OF_PROCESS);

        /* Encrypt payload using AES*/
        aes_encrypt_data(payload, payload_size, AES_encrypted_msg, &AES_block_count_msg);

        printf("\r\nPayload Encryption Successful\r\n");

        /* Print the key used for encryption */
        printf("\r\nKey used for Encryption:\r\n");
        print_data(AES_Key, AES128_KEY_LENGTH);

        printf("\r\nResult of Encryption:\r\n");

        /* Print the encrypted payload */
        printf("\r\nEncrypted Payload:\r\n");
        print_data((uint8_t*) AES_encrypted_msg, AES_block_count_msg * AES128_ENCRYPTION_LENGTH);


        /* Send AT command and wait for Receiver to send OK */
        while(1)
        {
            uart_send_data(AT_buffer, 2);
            uart_get_data(receive_buffer, 2, 5000);
            if(receive_buffer[0] == 'O' && receive_buffer[1] == 'K')
            {
                printf("\r\nCommunication established!\r\n");
                break;
            }
            else
            {
                printf("\r\nCommunication failed! Retrying...");
            }
        }

        /* Send the encrypted payload to the Receiver */
        uart_send_data(AES_encrypted_msg, AES_block_count_msg * AES128_ENCRYPTION_LENGTH);

        /* Send the EOP characters to indicate end of payload */
        uart_send_data(EOP_buffer, 3);

        /* Wait for Receiver to send OK */
        uart_get_data(receive_buffer, 2, 0);
        if(receive_buffer[0] != 'O' || receive_buffer[1] != 'K')
        {
            printf("\r\nCommunication failed and terminated!\r\n");
            CY_ASSERT(0);
        }

        printf("\r\nMessage successfully sent to Receiver!!\r\n");
    }
}

/*******************************************************************************
* Function Name: sender_key_exchange
********************************************************************************
* Summary:
*  Function to create the AES key and send it to the receiver securely.
*
* Return:
*  result of the operation
*
*******************************************************************************/

cy_rslt_t sender_key_exchange()
{
    /* Generate AES Key */
    generate_key();
    printf("\r\nRandom AES128 Key generated\r\n");

    /* Encrypt the generated key using RSA */
    rsa_encrypt_key();
    printf("\r\nKey Encryption Successful\r\n");
    print_data(rsaOutput, 256);

    printf("\r\nWaiting for receiver...\r\n");
    /* Send AT command and wait for Receiver to send OK */
    while(1)
    {
        uart_send_data(AT_buffer, 2);
        uart_get_data(receive_buffer, 2, 5000);
        if(receive_buffer[0] == 'O' && receive_buffer[1] == 'K')
        {
            printf("\r\nCommunication established!\r\n");
            break;
        }
    }

    printf("\r\nSending key to receiver\r\n");

    /* Send AES Key after RSA encryption */
    uart_send_data(rsaOutput, 256);

    /* Generate HASH of the key */
    generate_hash(AES_Key, sizeof(AES_Key), AES_Key_hash);

    /* Receive HASH of AES key from receiver */
    uart_get_data(receive_buffer, MESSAGE_DIGEST_SIZE, 0);

    /* Compare the received HASH with the generated HASH */
    for(int k = 0; k < MESSAGE_DIGEST_SIZE; k++)
    {
        if(AES_Key_hash[k] != receive_buffer[k])
        {
            printf("\r\nAES Key Hash received does not match! Communication Terminated\r\n");
            /* Send NC command to Receiver */
            uart_send_data(NC_buffer, 2);
            return CY_RSLT_TYPE_ERROR;
        }
    }

    printf("\r\nAES Key Hash matched!\r\n");

    /* Send OK command to Receiver */
    uart_send_data(OK_buffer, 2);
    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: generate_key
********************************************************************************
* Summary:
*  Generate a 16 byte key using TRNG to ensure that the key is unique.
*
*******************************************************************************/

void generate_key(void)
{
    int8_t index;
    uint32_t random_val;

    cy_rslt_t result;

    cyhal_trng_t trng_obj;

    /* Initialize the TRNG generator block*/
    result = cyhal_trng_init(&trng_obj);

    if (result == CY_RSLT_SUCCESS)
    {
        for (index = 0; index < AES128_KEY_LENGTH; index++)
        {
            /* Generate a random number and truncate to a 8 bit number */
            random_val = cyhal_trng_generate(&trng_obj) & EIGHT_BIT_MASK;
            AES_Key[index] = random_val;
        }

        /* Free the TRNG generator block */
        cyhal_trng_free(&trng_obj);
    }
}

/*******************************************************************************
* Function Name: rsa_encrypt_key
********************************************************************************
* Summary:
*  Encrypt the generated AES key using the public key of the receiver with  
*  RSA algorithm.
*
*******************************************************************************/
void rsa_encrypt_key(void)
{
    cy_en_crypto_status_t cryptoStatus;

    /* Enable the Crypto block */
    cryptoStatus = Cy_Crypto_Core_Enable(CRYPTO);
    if(cryptoStatus != CY_CRYPTO_SUCCESS)
    {
        CY_ASSERT(0);
    }

    cy_publicKey.publicKeyStruct.moduloPtr   = cy_publicKey.moduloData;
    cy_publicKey.publicKeyStruct.pubExpPtr   = cy_publicKey.expData;
    cy_publicKey.publicKeyStruct.pubExpLength = RSA_MODULO_LENGTH;

    cy_publicKey.publicKeyStruct.barretCoefPtr = cy_publicKey.k1Data;
    cy_publicKey.publicKeyStruct.inverseModuloPtr = cy_publicKey.k2Data;
    cy_publicKey.publicKeyStruct.rBarPtr = cy_publicKey.k3Data;

    /* Calculate key's coefficients to speedup RSA processing for long (multiblock) messages */
    cryptoStatus = Cy_Crypto_Core_Rsa_Coef(CRYPTO, &cy_publicKey.publicKeyStruct);
    if(cryptoStatus != CY_CRYPTO_SUCCESS)
    {
        printf("RSA Coefficient calculation error!\r\n");
        CY_ASSERT(0);
    }

    /* Add message in the right format here */
    cryptoStatus = Cy_Crypto_Core_Rsa_Proc(CRYPTO,
                        (cy_stc_crypto_rsa_pub_key_t *)&cy_publicKey.publicKeyStruct,
                        AES_Key, sizeof(AES_Key),
                        rsaOutput);

    if(cryptoStatus != CY_CRYPTO_SUCCESS)
    {
        printf("RSA Calculation error!\r\n");
        CY_ASSERT(0);
    }

    /* Disable the Crypto block */
    Cy_Crypto_Core_Disable(CRYPTO);
}

/*******************************************************************************
* Function Name: generate_hash
********************************************************************************
* Summary:
*  Generate SHA HASH of the data passed to the function.
*
* Parameters:
*  message - input data for which hash is to be generated
*  size  - size of message
*  hash - buffer to store HASH of input data
*
*******************************************************************************/

void generate_hash(uint8_t* message, uint8_t size, uint8_t* hash)
{
    cy_en_crypto_status_t result;

    /* Enable the Crypto block */
    Cy_Crypto_Core_Enable(CRYPTO);

    /* Perform the message digest generation using SHA-256 algorithm */
    result = Cy_Crypto_Core_Sha(CRYPTO, message, size, hash, CY_CRYPTO_MODE_SHA256);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Disable the Crypto block */
    Cy_Crypto_Core_Disable(CRYPTO);
}

/*******************************************************************************
* Function Name: create_payload
********************************************************************************
* Summary:
*  Combine the message, end of message padding and message HASH into a payload.
*
* Parameters:
*  message - the original message to be sent to the receiver
*  msg_size  - size of message
*  hash - hash of message
*  hash_size - size of message
*  payload - buffer to store the combined payload
*  pld_size - size of payload 
*
*******************************************************************************/

void create_payload(uint8_t* message, uint8_t msg_size, uint8_t* hash, uint8_t hash_size, uint8_t* payload, uint8_t* pld_size)
{
    /* Append message to payload buffer */
    for(int i = 0; i < msg_size; i++)
    {
        payload[i] = message[i];
    }

    /* Append EOM padding to payload buffer */
    payload[msg_size]   = 'H';
    payload[msg_size+1] = 'A';
    payload[msg_size+2] = 'S';
    payload[msg_size+3] = 'H';
    payload[msg_size+4] = ':';

    /* Append message HASH to payload buffer */
    for(int i = 0; i < hash_size; i++)
    {
        payload[msg_size+5+i] = hash[i];
    }

    /* Calculate payload size */
    *pld_size = msg_size + 5 + hash_size;
}

/*******************************************************************************
* Function Name: aes_encrypt_data
********************************************************************************
* Summary:
*  Encrypt the data using the generated key with AES algorithm.
*
* Parameters:
*  message - data to be encrypted
*  size  - number of bytes to encrypt
*  AES_buffer - buffer to store the AES encrypted data
*  AES_block_count - variable to store the block count 
*
*******************************************************************************/

void aes_encrypt_data(uint8_t* message, uint8_t size, uint8_t* AES_buffer, uint8_t* AES_block_count)
{

    cy_en_crypto_status_t result;

    /* Enable the Crypto block */
    Cy_Crypto_Core_Enable(CRYPTO);

    /* Calculate the AES block count */
    *AES_block_count =  (size % AES128_ENCRYPTION_LENGTH == 0) ?
                       (size / AES128_ENCRYPTION_LENGTH)
                       : (1 + size / AES128_ENCRYPTION_LENGTH);

    /* Initializes the AES operation by setting key and key length */
    Cy_Crypto_Core_Aes_Init(CRYPTO, AES_Key, CY_CRYPTO_KEY_AES_128, &aes_state);

    for (int i = 0; i < *AES_block_count ; i++)
    {
        /* Perform AES ECB Encryption mode of operation */
        result = Cy_Crypto_Core_Aes_Ecb(CRYPTO, CY_CRYPTO_ENCRYPT,
                               (AES_buffer + AES128_ENCRYPTION_LENGTH * i),
                               (message + AES128_ENCRYPTION_LENGTH * i),
                                &aes_state);
        if (result != CY_RSLT_SUCCESS)
        {
            CY_ASSERT(0);
        }

        /* Wait for Crypto Block to be available */
        Cy_Crypto_Core_WaitForReady(CRYPTO);
    }
    Cy_Crypto_Core_Aes_Free(CRYPTO, &aes_state);
}

/*******************************************************************************
* Function Name: print_data
********************************************************************************
* Summary:
*  Function to print the buffer data in the right format.
*
* Parameters:
*  data - data buffer that stores data to be printed
*  len  - number of bytes to print
*
*******************************************************************************/

void print_data(uint8_t* data, uint16_t len)
{
    char print[10];
    for (uint32 i=0; i < len; i++)
    {
        if ((i % BYTES_PER_LINE) == 0)
        {
            printf("\r\n");
        }
        sprintf(print,"0x%02X ", *(data+i));
        printf("%s", print);
    }
    printf("\r\n");
}

/*******************************************************************************
* Function Name: uart_init
********************************************************************************
* Summary:
*  Function to initialize both the Debug and the communication UART peripherals.
*  The Debug UART is used to print the logs in to the serial terminal 
*  application and the Communication UART peripherals are used to exchange data
*  between the sender and the receiver.
*
*******************************************************************************/

void uart_init()
{
    cy_rslt_t result;
    #if TEST_UART_INVERT
        result = cy_retarget_io_init(P12_1, P12_0, 115200);
    #else
        result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, 115200);
    #endif
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize UART for communication with Receiver */
    const cyhal_uart_cfg_t uart_config =
    {
        .data_bits = 8,
        .stop_bits = 1,
        .parity = CYHAL_UART_PARITY_NONE,
        .rx_buffer = NULL,
        .rx_buffer_size = 0,
    };

    #if TEST_UART_INVERT
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
* Function Name: uart_send_data
********************************************************************************
* Summary:
*  Function to send data to the receiver.
*
* Parameters:
*  data_buffer - buffer containing data to be sent
*  buffer_length  - number of bytes to send
*
*******************************************************************************/

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
* Return:
*  result of the operation
*
*******************************************************************************/

cy_rslt_t uart_get_data(uint8_t* data_buffer, uint16_t buffer_length, uint32_t timeout)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint8_t recv_value = 0;
    for(int i = 0; i < buffer_length; i++)
    {
        result = cyhal_uart_getc(&uart_obj, &recv_value, timeout);
        if(result != CY_RSLT_SUCCESS)
        {
            return result;
        }
        data_buffer[i] = recv_value;
    }
    return result;
}

/* [] END OF FILE */
