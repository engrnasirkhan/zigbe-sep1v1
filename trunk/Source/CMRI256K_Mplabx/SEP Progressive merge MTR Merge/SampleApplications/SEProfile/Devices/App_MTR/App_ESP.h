/*********************************************************************
 *
 *                          Application ESP
 *
 *********************************************************************
 * FileName:        App_ESP.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v1.05 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright © 2004-2007 Microchip Technology Inc.  All rights reserved.
 *
 * Microchip licenses to you the right to use, copy and distribute Software
 * only when embedded on a Microchip microcontroller or digital signal
 * controller and used with a Microchip radio frequency transceiver, which
 * are integrated into your product or third party product (pursuant to the
 * sublicense terms in the accompanying license agreement).  You may NOT
 * modify or create derivative works of the Software.
 *
 * If you intend to use this Software in the development of a product for
 * sale, you must be a member of the ZigBee Alliance.  For more information,
 * go to www.zigbee.org.
 *
 * You should refer to the license agreement accompanying this Software for
 * additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED
 * UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF
 * WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR
 * EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT,
 * PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY
 * THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER
 * SIMILAR COSTS.
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 ********************************************************************/

#ifndef _APP_ESP_H_
#define _APP_ESP_H_

/*****************************************************************************
  Includes
 *****************************************************************************/
/* None */

/*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
/* None */

/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
#define ZDO_END_POINT                       0x00

#ifdef I_SUPPORT_SECURITY
    #define ZDP_DEFAULT_TX_OPTIONS          0x02 // NWK KEY ENABLED
#else
    #define ZDP_DEFAULT_TX_OPTIONS          0x00 // UNSECURED
#endif

/*****************************************************************************
  Data Structures
 *****************************************************************************/
/* None */

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
    /* Global static array to store the Fragmentation data payload */
    extern BYTE fragmentData[];
#endif

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/


#endif /* _APP_ESP_H_ */

