/******************************************************************************
* File Name: resource_map.h
*
* Description: This file gives the SPI pin map for all the supported kits.
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
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
* significant property damage, injury or death (“High Risk Product”). By
* including Cypress’s product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#ifndef RESOURCE_MAP_H_
#define RESOURCE_MAP_H_

#if defined (TARGET_CY8CPROTO_062_4343W)                    /* CY8CPROTO_062_4343W Kit*/
    #define sSPI_MOSI               (P9_0)
    #define sSPI_MISO               (P9_1)
    #define sSPI_SCLK               (P9_2)
    #define sSPI_SS                 (P9_3)
#else                                                       /* Other supported kits */
    #define sSPI_MOSI               (P10_0)
    #define sSPI_MISO               (P10_1)
    #define sSPI_SCLK               (P10_2)
    #define sSPI_SS                 (P10_3)
#endif

#if defined (TARGET_CY8CPROTO_063_BLE) || defined (TARGET_CY8CKIT_062S2_43012) || defined (TARGET_CYW9P62S1_43438EVB_01)     /* CY8CPROTO-063-BLE, CYW9P62S1_43438EVB_01 and CY8CKIT_062S2_43012 kit */
    #define mSPI_MOSI               (P9_0)
    #define mSPI_MISO               (P9_1)
    #define mSPI_SCLK               (P9_2)
    #define mSPI_SS                 (P9_3)
#else                                                       /* Other supported kits */
    #define mSPI_MOSI               (P6_0)
    #define mSPI_MISO               (P6_1)
    #define mSPI_SCLK               (P6_2)
    #define mSPI_SS                 (P6_3)
#endif

#endif /* RESOURCE_MAP_H_ */
