/******************************************************************************
*
*                  Certificate
*
******************************************************************************
* FileName:        SE_Certificate.h
* Dependencies:
* Processor:       PIC18 / PIC24 / PIC32
* Complier:        MCC18 v3.00 or higher
*                  MCC30 v2.05 or higher
*                  MCC32 v2.05 or higher
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

#ifndef _SE_CERTIFICATE_MTR_H_
#define _SE_CERTIFICATE_MTR_H_

#if I_SUPPORT_CBKE == 1
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

/******************************************************************************
* Function:        Function Name
*
* PreCondition:    None
*
* Input:           None
*
* Output:          None
*
* Side Effects:    None
*
* Return Value:    None
*
* Overview:
*
* Note:
*****************************************************************************/
/******************************************************************************
* Function:        BYTE* SE_GetCertificate( void )
*
* PreCondition:    None
*
* Input:           None
*
* Output:          None
*
* Side Effects:    None
*
* Return Value:    pointer to the certificate
*
* Overview:        This function returns the pointer to the certificate of the
*                   device.
*
* Note:
*****************************************************************************/
BYTE* SE_GetCertificate( void );

/******************************************************************************
* Function:        BYTE* SE_GetStaticPrivateKey( void )
*
* PreCondition:    None
*
* Input:           None
*
* Output:          None
*
* Side Effects:    None
*
* Return Value:    pointer to static private key
*
* Overview:        This function returns the pointer to the static private
*                   key of the device.
*
* Note:
*****************************************************************************/
BYTE* SE_GetStaticPrivateKey( void );

/******************************************************************************
* Function:        BYTE* SE_GetCAPublicKey( void )
*
* PreCondition:    None
*
* Input:           None
*
* Output:          None
*
* Side Effects:    None
*
* Return Value:    pointer to Certificate Authority's Public Key
*
* Overview:        This function returns the pointer to the CA Public key
*                    of the device.
*
* Note:
*****************************************************************************/
BYTE* SE_GetCAPublicKey( void );
#endif /*#if I_SUPPORT_CBKE == 1*/
#endif /* _SE_CERTIFICATE_H_ */
