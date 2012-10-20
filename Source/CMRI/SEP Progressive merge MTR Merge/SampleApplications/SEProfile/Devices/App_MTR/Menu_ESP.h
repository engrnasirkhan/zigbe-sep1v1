/*********************************************************************
 *
 *                  Menu Options for Energy Service Portal (ESP)
 *
 *********************************************************************
 * FileName:        Menu.h
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

#ifndef _MENU_ESP_H_
#define _MENU_ESP_H_

/****************************************************************************
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
  Data Structures
 *****************************************************************************/


typedef struct INPUT_BUFFER_TAG_ESP
{
    BYTE buffer[25];
    BYTE used;
}INPUT_BUFFER_T_ESP;



 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
extern ROM char * const menu_ESP;

extern INPUT_BUFFER_T_ESP inputBuf_ESP;
extern BYTE menuLevel1;
extern BYTE menuLevel2;
extern BYTE menuLevel3;
extern BYTE inputIndex;
extern BYTE asduData[80];
extern BYTE startMode;

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/

/***************************************************************************************
 * Function:        void PrintMenu( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Prints the Menu options.
 *
 ***************************************************************************************/
void PrintMenu_ESP( void );

/***************************************************************************************
 * Function:        void ProcessMenu( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function will handle the requests from console.
 *
 ***************************************************************************************/
void ProcessMenu_ESP( void );

#endif /*_MENU_H_*/
