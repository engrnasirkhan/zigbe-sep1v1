/******************************************************************************
 *
 *                  SE Display
 *
 ******************************************************************************
 * FileName:        SE_Display.h
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      07/04/09

 *****************************************************************************/

#ifndef _SE_DISPLAY_H_
#define _SE_DISPLAY_H_

/*****************************************************************************
  Includes
 *****************************************************************************/
#include "SE_ESP.h"
/*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/

/* None */

/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
enum display_state
{
    displayTime                         = 0x00,
    displayMessage                      = 0x01,
    receivedMessage                     = 0x03,
    receivedGetProfileResponse          = 0x04,
    receivedPublishPrice                = 0x05,
    receivedDRLCEvent                   = 0x06,
    receivedCancelMessage               = 0x07,
    receivedCancelDRLC                  = 0x08,
    receivedCancelAllDRLC               = 0x09,
    receivedMeterReading                = 0x0A,
    reportAttribute                     = 0x11,
    receiveCurrSummRecevMeterReading    = 0x21,
    receivedCurrMaxDemandMeterReading   = 0x22,
    receivedCurrDayConsumMeterReading   = 0x23,
    receivedTier1MeterReading           = 0x24,
    receivedTier2MeterReading           = 0x26,
    receivedTier3MeterReading           = 0x28,
    receivedTier4MeterReading           = 0x2A,
    receivedTier5MeterReading           = 0x2C,
    receivedTier6MeterReading           = 0x2E,
    
    receivedOnOffAttributeValueOFF      = 0x2F,
    receivedOnOffAttributeValueON       = 0x3F
    #if defined(I_SUPPORT_GROUP_ADDRESSING)
    , 
    receivedAddGroupCmd                 = 0x30,
    receivedRemoveGroupCmd              = 0x31,
    receivedViewGroupRspCmd             = 0x32
    #endif
    
    #if (I_SUPPORT_SCENES == 1)
    ,
    receivedAddSceneCmd                 = 0x40,
    receivedRemoveSceneCmd              = 0x41,
    receivedViewSceneRspCmd             = 0x42,
    receivedRemoveSceneRspCmd           = 0x43
    #endif
};

/*****************************************************************************
  Data Structures
 *****************************************************************************/
typedef struct __attribute__((packed,aligned(1))) _Display_Timer
{
    BOOL timerInProgress;
    DWORD time;
} Display_Timer;

typedef struct __attribute__((packed,aligned(1))) _DisplayGroupsCmds
{
    WORD_VAL groupID;
    BYTE groupName[16];
    BYTE status;
} DisplayGroupsCmds;

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
extern BYTE displayState;

#if ( (I_AM_IPD == 0x01 )  || (I_AM_PCT == 0x01) || (I_AM_LCD == 0x01) )
    extern displayMessageCmdTxDetails displayMessagesTable;
    extern DRLC_LoadControlEvent LoadControlEventTable;
#endif
//#if (I_AM_ESP == 1)		//MSEB
    extern Meter_FormattingAttributes meterFormatAttributes;
    extern MeterAttributes_DisplayState meterAttributes_DisplayStateTable[];
    extern DWORD displayQuotient;
    extern BYTE remainderArray[10];
//#endif  /*(I_AM_ESP == 1)*/

#if defined(I_SUPPORT_GROUP_ADDRESSING)
extern DisplayGroupsCmds displayGroups;
#endif

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/

/******************************************************************************
* Function:        void Display_Time( void );
*
* PreCondition:    None
*
* Input:           None
*
* Output:          None
*
*
* Return :         None
*
* Side Effects:    None
*
* Overview:        This function reads the RTC and displays the time in the
*                   LCD
*
* Note:            None
*****************************************************************************/
void Display_Time( void );
/***************************************************************************************
 * Function:        void display_Byte(BYTE *val)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handles Display on IPD device.
 *
 * Note:            None
 *
 ***************************************************************************************/
void display_Byte(BYTE *val);

/***************************************************************************************
 * Function:        void HandleESPDisplay( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handles Display on ESP device.
 *
 * Note:            None
 *
 ***************************************************************************************/
//#if ( I_AM_ESP == 1 )		//MSEB
void HandleESPDisplay( void );
//#endif /*( I_AM_ESP == 1 )*/

/***************************************************************************************
 * Function:        void HandleIPDDisplay( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handles Display on IPD device.
 *
 * Note:            None
 *
 ***************************************************************************************/
#if ( I_AM_IPD == 1 )
void HandleIPDDisplay( void );
#endif /* ( I_AM_IPD == 1 ) */

/***************************************************************************************
 * Function:        void HandleLCDDisplay( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function will display the Events received (like Group commands) 
 *                  on LCD of the device.
 *
 * Note:            None
 *
 ***************************************************************************************/
#if(I_AM_LCD == 1)
void HandleLCDDisplay( void );
#endif /* ( I_AM_LCD == 1 ) */

/***************************************************************************************
 * Function:        void HandleMeterDisplay( void )
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
//#if(I_AM_MTR == 1)		//MSEB
void HandleMeterDisplay( void );
//#endif /* ( I_AM_MTR == 1 ) */

/***************************************************************************************
 * Function:        void HandlePCTDisplay( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function will display the Events received (like Group commands) 
 *                  on LCD of the device.
 *
 * Note:            None
 *
 ***************************************************************************************/
#if(I_AM_PCT == 1)
void HandlePCTDisplay( void );
#endif /* ( I_AM_PCT == 1 ) */

/***************************************************************************************
 * Function:        void HandleREDDisplay( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function will display the Events received (like Group commands) 
 *                  on LCD of the device.
 *
 * Note:            None
 *
 ***************************************************************************************/
#if(I_AM_RED == 1)
void HandleREDDisplay( void );
#endif /* ( I_AM_RED == 1 ) */

/***************************************************************************************
 * Function:        void HandleSAPDisplay( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function will display the Events received (like Group commands) 
 *                  on LCD of the device.
 *
 * Note:            None
 *
 ***************************************************************************************/
#if(I_AM_SAP == 1)
void HandleSAPDisplay( void );
#endif /* ( I_AM_SAP == 1 ) */

#endif /* _SE_DISPLAY_H_ */

