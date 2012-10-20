/******************************************************************************
 *
 *                  Application Common
 *
 ******************************************************************************
 * FileName:        App_Common.h
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
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *****************************************************************************/

#ifndef _APP_COMMON_H_
#define _APP_COMMON_H_

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
/* None */

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
/* None */

/*****************************************************************************
  Data Structures
 *****************************************************************************/
/* None */

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/

/***************************************************************************************
 * Function:        BYTE GetMACByte(BYTE *ptr)
 *
 * PreCondition:    None
 *
 * Input:           ptr-Pointer to the HEX bytes which needs to be converted into BYTE.
 *
 * Output:          converted BYTE
 *
 * Side Effects:    None
 *
 * Overview:        2 HEX digits(in ascii format) converted into a BYTE
 *
 * Note:            None
 *
 ***************************************************************************************/
BYTE GetMACByte(BYTE *ptr);

/***************************************************************************************
 * Function:        BYTE GetMACByteDec(BYTE *ptr)
 *
 * PreCondition:    None
 *
 * Input:           ptr-Pointer to the Decimal values which needs to be converted into BYTE.
 *
 * Output:          converted BYTE
 *
 * Side Effects:    None
 *
 * Overview:        decimal values(in ascii format) converted into a BYTE
 *
 * Note:            None
 *
 ***************************************************************************************/
void GetMACByteDec(void *value, BYTE *ptr, BYTE dataTypeSize);
/***************************************************************************************
 * Function:        void HandleUARTRequests()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handle requests issued by terminal
 *
 * Note:            None
 *
 ***************************************************************************************/
void HandleUARTRequests(void);

/******************************************************************************
 * Function:        void AppSendData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData)
 *
 * PreCondition:    None
 *
 * Input:           direction - direction of the command - server to client
 *                                                         or client to server
 *                  clusterID - cluster id
 *                  cmdId - command id
 *                  asduLength - length of the payload
 *                  pData - payload
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function allocates memory.It fills up App Data Request
*                   structure and queues it up for AIL
 *****************************************************************************/
void AppSendData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData);

/******************************************************************************
 * Function:        void AppSendGroupData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData)
 *
 * PreCondition:    None
 *
 * Input:           direction - direction of the command - server to client
 *                                                         or client to server
 *                  clusterID - cluster id
 *                  cmdId - command id
 *                  asduLength - length of the payload
 *                  pData - payload
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to format the Data in the required format
 *                  for updating the data request structure.
 *****************************************************************************/
void AppSendGroupData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData);


BYTE App_FindDestEndpointFromBindings(SHORT_ADDR shortAddr, WORD clusterId);

#endif /* _APP_COMMON_H_ */
