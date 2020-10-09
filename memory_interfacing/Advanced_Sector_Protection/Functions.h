/******************************************************************************
* File Name:   Functions.h
*
* Description: This file contains the macro definitions, global variables and
*              function prototypes for function defined in Functions.c file.
*******************************************************************************/

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_
#endif /* FUNCTIONS_H_ */


#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cyhal_qspi.h"
#include "cy_serial_flash_qspi.h"
#include "cycfg_qspi_memslot.h"
#include "cy_smif.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define TRUE                       true
#define FALSE                      false
#define SUCCESS                    (0u)
#define MEM_SLOT_NUM               (0u)                  /* Slot number of the memory to use */
#define QSPI_BUS_FREQUENCY_HZ      (50000000lu)
#define PACKET_SIZE                (64u)                 /* Memory Read/Write size */
#define PASSWORD_LENGTH            (8U)
#define ADDRESS_LENGTH             (4U)
#define SR1_LENGTH                 (1U)
#define DEVICE_ID_LENGTH           (3U)
#define ASP_REGISTER_LENGTH        (2U)
#define PPB_LENGTH                 (1U)
#define FLASH_BUSY                 (0x01)

/* Commands */
#define RDID                       (0x9F)
#define RDSR1                      (0x05)
#define WREN                       (0x06)
#define SE                         (0xD8)
#define PP                         (0x02)
#define READ                       (0x03)
#define READ_4B                    (0x13)
#define CLSR                       (0x30)

#define ASPRD                      (0x2B)
#define ASPP                       (0x2F)

#define PASSP                      (0xE8)
#define PASSRD                     (0xE7)
#define PASSU                      (0xE9)

#define PPBRD                      (0xE2)
#define PPBP                       (0xE3)
#define PPBE                       (0xE4)






/*******************************************************************************
* Function Prototypes
********************************************************************************/
cy_rslt_t Read_ID(uint8_t *rx_data);
cy_rslt_t Read_Status_Register1(uint8_t *rx_data);
cy_rslt_t Polling_WIP();
cy_rslt_t Clear_Status_Register();
cy_rslt_t Write_Enable();
cy_rslt_t Read(uint32_t addr, uint8_t *rx_data);
cy_rslt_t Sector_Erase(uint32_t addr);
cy_rslt_t Page_Program(uint32_t addr, uint8_t *tx_data);
cy_rslt_t ASP_Register_Read(uint8_t *asp_reg);
cy_rslt_t ASP_Program(uint8_t *asp_reg);
cy_rslt_t Password_Program();
cy_rslt_t Password_Read(uint8_t *rx_data);
cy_rslt_t Password_Unlock();
cy_rslt_t PPB_Read(uint32_t addr, uint8_t *rx_data);
cy_rslt_t PPB_Program(uint32_t addr);
cy_rslt_t PPB_Erase();

