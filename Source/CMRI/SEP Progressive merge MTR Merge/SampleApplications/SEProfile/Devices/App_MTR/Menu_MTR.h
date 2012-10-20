/*********************************************************************
 *
 *                  Menu Options for Meter Device
 *
 *********************************************************************
 * FileName        : Menu.h
 * Dependencies    :
 * Processor       : PIC18 / PIC24 / PIC32
 * Complier        : MCC18 v3.00 or higher
 *                   MCC30 v2.05 or higher
 *                   MCC32
 * Company         : Microchip Technology, Inc.
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
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 ********************************************************************/
#ifndef _MENU_H
#define _MENU_H

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
  Data Structures
 *****************************************************************************/
typedef struct _Meter_Timer
{
    BOOL timerInProgress;
    DWORD time;
} Meter_Timer;

typedef struct INPUT_BUFFER_TAG
{
    BYTE buffer[200];
    BYTE used;
}INPUT_BUFFER_T;


/*****************************************************************************
 Variable definitions
*****************************************************************************/
extern INPUT_BUFFER_T inputBuf_MTR;
extern BYTE menuLevel1;
extern BYTE menuLevel2;
extern BYTE menuLevel3;
extern BYTE inputIndex;
extern BYTE asduData[80];
/*****************************************************************************
  Function Prototypes
 *****************************************************************************/


/***************************************************************************************
 * Function:        void Display_Message( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handles Display on Meter device.
 *
 * Note:            None
 *
 ***************************************************************************************/
void Display_Message( void );


/***************************************************************************************
 * Function:        void Display_PublishPrice()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handles Display on Meter device.
 *
 * Note:            None
 *
 ***************************************************************************************/
void Display_PublishPrice_MTR();

/*****************************************************************************
 * Function:        void ProcessButtonPress(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The function can be used to poll the on board push buttons
 *
 * Note:            None
 *****************************************************************************/
void ProcessButtonPress(void);

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
void PrintMenu_MTR( void );

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
void ProcessMenu_MTR( void );

#endif /*_MENU_H*/
