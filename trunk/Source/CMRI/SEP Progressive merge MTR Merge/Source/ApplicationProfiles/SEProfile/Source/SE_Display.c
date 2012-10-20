/*********************************************************************
 *
 *                  SE Display
 *
 *********************************************************************
 * FileName:        SE_Display.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v2.05 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright © 2004-2011 Microchip Technology Inc.  All rights reserved.
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *****************************************************************************/

 /****************************************************************************
   Includes
 *****************************************************************************/
/*Configuration files */
#include "zigbee.def"

/* Generic files */
#include "generic.h"
#include "rtcc.h"

/*ZCL Related Files*/
#include "ZCL_Interface.h"
#include "ZCL_Foundation.h"
#include "ZCL_Utility.h"
#include "ZCL_Basic.h"
#include "ZCL_Identify.h"
#include "ZCL_Message.h"
#include "ZCL_Time.h"
#include "ZCL_DRLC.h"
#include "ZCL_Metering.h"
#include "ZCL_Power_Configuration.h"
#include "ZCL_Alarm.h"
#include "ZCL_Price.h"
#include "ZCL_Commissioning.h"
#if I_SUPPORT_CBKE == 1
    #include "ZCL_Key_Establishment.h"
#endif /*#if I_SUPPORT_CBKE*/

/* SE Related Files */
#include "SE_Interface.h"
#include "SE_Profile.h"
#if(I_AM_IPD == 1)
    #include "SE_IPD.h"
#endif
#if(I_AM_PCT == 1)
    #include "SE_PCT.h"
#endif

#if(I_AM_LCD == 1)
    #include "SE_LCD.h"
#endif
//#if(I_AM_ESP == 1)		//MSEB
    #include "SE_ESP.h"
//#endif

//#if(I_AM_MTR == 1)		//MSEB
    #include "SE_MTR.h"
    #include "Menu_MTR.h"
//#endif

#include "SE_Display.h"

/* Libraries */
#if defined(__C30__)
    #include "TLS2130.h"
#else
    #include "lcd PIC32.h"
#endif

/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/
 /*None*/

/*****************************************************************************
   Customizable Macros
 *****************************************************************************/
 #define INVALID_START_TIME                              0xFFFFFFFF

/*****************************************************************************
   Data Structures
 *****************************************************************************/
 /*None*/

/*****************************************************************************
   Variable Definitions
 *****************************************************************************/
BYTE displayState = displayTime;
Display_Timer displayTimer = {0x00, 0x00000000};

#if(I_AM_IPD == 1)
    extern ZCL_Message_CancelMessageCmdPayload cancelMessage;
    extern ZCL_MeteringGetProfileRspCmdPayload MeteringGetProfileResponse;
    extern BYTE meterReadingValue[];
    extern BYTE *reportAttributeValue;
#endif

#if( I_AM_IPD == 1 )
extern BYTE TiermeterReadingValue[6];
#endif /* I_AM_IPD == 1 */

#if defined(I_SUPPORT_GROUP_ADDRESSING)
DisplayGroupsCmds   displayGroups;
#endif

/*****************************************************************************
   Function Prototypes
 *****************************************************************************/

/***************************************************************************************
 * Function:        static void Display_Message( void )
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
#if(I_AM_IPD == 1)
static void Display_Message( void );
#endif

/***************************************************************************************
 * Function:        static void Display_GetProfileResponse()
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
#if(I_AM_IPD == 1)
static void Display_GetProfileResponse();
#endif

/***************************************************************************************
 * Function:        static void Display_PublishPrice()
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
//#if(I_AM_IPD == 1) || ( I_AM_ESP == 1 )		//MSEB
static void Display_PublishPrice_ESP();
//#endif

/***************************************************************************************
 * Function:        static void Display_DRLCEvent()
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
#if(I_AM_IPD == 1)
static void Display_DRLCEvent();
#endif

/***************************************************************************************
 * Function:        static void Display_CancelMessage()
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
#if(I_AM_IPD == 1)
static void Display_CancelMessage();
#endif

/***************************************************************************************
 * Function:        static void Display_CancelDRLC(void)
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
#if(I_AM_IPD == 1)
static void Display_CancelDRLC(void);
#endif

/***************************************************************************************
 * Function:        static void Display_CancelAllDRLC(void)
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
#if(I_AM_IPD == 1)
static void Display_CancelAllDRLC(void);
#endif

/***************************************************************************************
 * Function:        static void  Display_reportAttribute( void )
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
#if(I_AM_IPD == 1)
static void Display_reportAttribute(void);
#endif

#if( I_AM_IPD == 1 )
static void DisplayTimeOfUse
(
    BYTE *pmeterReadingValue,
    BYTE display_State
);
#endif

#if defined(I_SUPPORT_GROUP_ADDRESSING)
static void display_AddGroup(void);
static void display_RemoveGroup(void);
static void display_ViewRspGroup(void);
#endif

/*****************************************************************************
  Private Functions
 *****************************************************************************/

/***************************************************************************************
 * Function:        static void Display_Message( void )
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
#if(I_AM_IPD == 1)
static void Display_Message( void )
{
    BYTE i;

    lcdClear();
    lcdPutStr("MSG:");
    if (displayMessagesTable.confirmationRequired)
    {
        lcdPutCur(1,13);
        lcdPutStr("CNF");
    }
    for (i = 0; i < displayMessagesTable.displayMessage.aDisplayMessage[0]; i++)
    {
        lcdPutCur(1, i);
        lcdPutChar( displayMessagesTable.displayMessage.aDisplayMessage[i + 1]);
    }
}
#endif

/***************************************************************************************
 * Function:        static void Display_GetProfileResponse()
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
#if(I_AM_IPD == 1)
static void Display_GetProfileResponse()
{
    lcdClear();
    lcdPutStr("METER VAL:");
    lcdPutCur(1,0);
    display_Byte( &MeteringGetProfileResponse.intervals[0].metringInterval[2] );
    lcdPutCur(1,2);
    display_Byte( &MeteringGetProfileResponse.intervals[0].metringInterval[1] );
    lcdPutCur(1,4);
    display_Byte( &MeteringGetProfileResponse.intervals[0].metringInterval[0] );
}
#endif

/***************************************************************************************
 * Function:        static void Display_PublishPrice()
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
//#if(I_AM_IPD == 1) || ( I_AM_ESP == 1 )		//MSEB
static void Display_PublishPrice_ESP()
{
    BYTE *temp;
    elapsedTimeInfo elapsedTime;
    BYTE tempVal;

    DWORD startTime;

    if ( PublishPriceInfo.StartTime == 0x00000000 )
    {
        startTime = ZCL_Callback_GetCurrentTimeInSeconds();
    }
    else
    {
        startTime = PublishPriceInfo.StartTime;
    }

    ZCL_Utility_Time_ConvertSecondsToYear( startTime, &elapsedTime );

    lcdClear();
    lcdPutStr("S:");
    lcdPutCur(0, 2);

    tempVal = elapsedTime.hour / 10;
    elapsedTime.hour += tempVal * 6;

    display_Byte( &elapsedTime.hour );
    lcdPutCur(0,4);
    lcdPutStr(":");
    lcdPutCur(0,5);
    tempVal = elapsedTime.minute / 10;
    elapsedTime.minute += tempVal * 6;
    display_Byte( &elapsedTime.minute );
    lcdPutCur(0,8);

    tempVal = elapsedTime.day / 10;
    elapsedTime.day += tempVal * 6;
    display_Byte( &elapsedTime.day );
    lcdPutCur(0,10);
    lcdPutStr("/");
    lcdPutCur(0,11);
    tempVal = elapsedTime.month / 10;
    elapsedTime.month += tempVal * 6;
    display_Byte( &elapsedTime.month );
    lcdPutCur(0,13);
    lcdPutStr("/");
    lcdPutCur(0,14);
    tempVal = elapsedTime.year / 10;
    elapsedTime.year += tempVal * 6;
    display_Byte( &elapsedTime.year );

    lcdPutCur(1,0);
    lcdPutStr("D:");
    lcdPutCur(1,2);
    temp = (BYTE *)&PublishPriceInfo.DurationInMinutes;
    temp += 1;
    display_Byte( temp );
    lcdPutCur(1,4);

    temp = (BYTE *)&PublishPriceInfo.DurationInMinutes;
    display_Byte( temp );

    lcdPutCur(1,8);
    temp = (BYTE *)&PublishPriceInfo.Price;
    temp += 3;
    display_Byte( temp );
    lcdPutCur(1,10);
    temp = (BYTE *)&PublishPriceInfo.Price;
    temp += 2;
    display_Byte( temp );
    lcdPutCur(1,12);
    temp = (BYTE *)&PublishPriceInfo.Price;
    temp += 1;
    display_Byte( temp );
    lcdPutCur(1,14);
    temp = (BYTE *)&PublishPriceInfo.Price;
    display_Byte( temp );
}
//#endif
/***************************************************************************************
 * Function:        static void Display_DRLCEvent()
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
#if(I_AM_IPD == 1)
static void Display_DRLCEvent()
{
    BYTE temp;
    BYTE i;
    BYTE temp1[5];
    WORD temp2;
    elapsedTimeInfo elapsedTime;

    if( LoadControlEventTable.startTime == 0x00000000 )
    {
        ZCL_Utility_Time_ConvertSecondsToYear( ZCL_Callback_GetCurrentTimeInSeconds(), &elapsedTime);
    }
    else
    {
        ZCL_Utility_Time_ConvertSecondsToYear( LoadControlEventTable.startTime, &elapsedTime);
    }
    lcdClear();
    lcdPutCur(0,0);
    lcdPutStr("S:");
    lcdPutCur(0,2);
    temp = elapsedTime.hour;
    if( temp > 19 )
    {
        temp += 12;
    }
    else if( temp > 9)
    {
        temp += 6;
    }
    display_Byte( &temp );

    temp2 = LoadControlEventTable.durationInMinutes;
    for (i = 4; i > 0; i--)
    {
        temp1[i] = temp2 % 10;
        temp2 /= 10;
        temp1[i] <<= 4;

    }
    temp1[0] = temp2 % 10;
    lcdPutCur(0,5);
    lcdPutStr("DM:");

    for (i = 0; i < 5; i++)
    {
        lcdPutCur(0, (8 + i) );
        display_Byte( &temp1[i] );
    }
    lcdPutCur(0,13);
    lcdPutStr(" ");
    lcdPutCur(1,0);
    lcdPutStr("G:");
    lcdPutCur(1,2);
    display_Byte( &LoadControlEventTable.utilityEnrolementGroup );

    lcdPutCur(1,5);
    lcdPutStr("D:");
    lcdPutCur(1,7);
    temp = ( ( LoadControlEventTable.deviceClass & 0xFF00 ) >> 8 );
    display_Byte( &temp );
    lcdPutCur(1,9);
    temp = ( LoadControlEventTable.deviceClass & 0x00FF );
    display_Byte( &temp );
}
#endif

/***************************************************************************************
 * Function:        static void Display_CancelMessage()
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
#if(I_AM_IPD == 1)
static void Display_CancelMessage()
{
    lcdPutCur(0,0);
    lcdPutStr("Cancel Message");
}
#endif

/***************************************************************************************
 * Function:        static void Display_CancelDRLC(void)
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
#if(I_AM_IPD == 1)
static void Display_CancelDRLC(void)
{
    lcdClear();
    lcdPutCur(0,0);
    lcdPutStr("Cancel DRLC");
}
#endif

/***************************************************************************************
 * Function:        static void Display_CancelAllDRLC(void)
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
#if(I_AM_IPD == 1)
static void Display_CancelAllDRLC(void)
{
    lcdClear();
    lcdPutCur(0,0);
    lcdPutStr("Cancel All DRLC");
}
#endif
/***************************************************************************************
 * Function:        static void  Display_reportAttribute( void )
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
#if(I_AM_IPD == 1)
static void Display_reportAttribute(void)
{
    BYTE length;
    BYTE i;

    lcdClear();
    lcdPutCur(0, 0);
    lcdPutStr("AI:");
    lcdPutCur(0, 3);
    display_Byte(&reportAttributeValue[1]);
    lcdPutCur(0, 5);
    display_Byte(&reportAttributeValue[0]);
    lcdPutCur(1, 0);
    lcdPutStr("AD:");

    length = ZCL_GetDataLengthForAttributeDataType(reportAttributeValue[2], 0x00);
    for( i = 0; i < length; i++ )
    {
        lcdPutCur(1, 3 + i * 2);
        display_Byte( &reportAttributeValue[length + 2 - i] );
    }
}
#endif /* (I_AM_IPD == 1) */

/***************************************************************************************
 * Function:        static void display_AddGroup(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the received Groups Commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if defined(I_SUPPORT_GROUP_ADDRESSING)
static void display_AddGroup(void)
{
    BYTE length;
    BYTE i;
    
    lcdClear();
    lcdPutCur (0, 0);
    lcdPutStr("ADD GRP:");
    lcdPutCur (0, 8);
    display_Byte(&displayGroups.groupID.v[1]);
    lcdPutCur (0, 10);
    display_Byte(&displayGroups.groupID.v[0]);
    lcdPutCur (1, 0);
    length = displayGroups.groupName[0];
    for (i=1; i<=length; i++)
    {
        lcdPutCur (1, i-1);
        lcdPutChar(displayGroups.groupName[i]);
    }
}
#endif

/***************************************************************************************
 * Function:        static void display_RemoveGroup(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the received Groups Commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if defined(I_SUPPORT_GROUP_ADDRESSING)
static void display_RemoveGroup(void)
{
    lcdClear();
    lcdPutCur (0, 0);
    lcdPutStr("RMV GRP:");
    lcdPutCur (0, 8);
    display_Byte(&displayGroups.groupID.v[1]);
    lcdPutCur (0, 10);
    display_Byte(&displayGroups.groupID.v[0]);
}
#endif

/***************************************************************************************
 * Function:        static void display_ViewRspGroup(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the received Groups Commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if defined(I_SUPPORT_GROUP_ADDRESSING)
static void display_ViewRspGroup(void)
{
    BYTE length;
    BYTE i;
    
    lcdClear();
    lcdPutCur (0, 0);
    lcdPutStr("VIEW GRP:");
    lcdPutCur (0, 9);
    display_Byte(&displayGroups.groupID.v[1]);
    lcdPutCur (0, 11);
    display_Byte(&displayGroups.groupID.v[0]);
    lcdPutCur (1, 0);
    length = displayGroups.groupName[0];
    for (i=1; i<=length; i++)
    {
        lcdPutCur (1, i-1);
        lcdPutChar(displayGroups.groupName[i]);
    }
}
#endif

/***************************************************************************************
 * Function:        static void display_AddScene(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the received Scene Commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if (I_SUPPORT_SCENES == 1)
static void display_AddScene(void)
{
    //BYTE length;
    //BYTE i;
    
    lcdClear();
    lcdPutCur (0, 0);
    lcdPutStr("ADD SCN:");
    /*lcdPutCur (0, 8);
    display_Byte(&displayGroups.groupID.v[1]);
    lcdPutCur (0, 10);
    display_Byte(&displayGroups.groupID.v[0]);
    lcdPutCur (1, 0);
    length = displayGroups.groupName[0];
    for (i=1; i<=length; i++)
    {
        lcdPutCur (1, i-1);
        lcdPutChar(displayGroups.groupName[i]);
    }*/
}
#endif

/***************************************************************************************
 * Function:        static void display_RemoveScene(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the received Scene Commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if (I_SUPPORT_SCENES == 1)
static void display_RemoveScene(void)
{
    lcdClear();
    lcdPutCur (0, 0);
    lcdPutStr("RMV SCN:");
    /*lcdPutCur (0, 8);
    display_Byte(&displayGroups.groupID.v[1]);
    lcdPutCur (0, 10);
    display_Byte(&displayGroups.groupID.v[0]);*/
}
#endif

/***************************************************************************************
 * Function:        static void display_ViewRspScene(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the received Scene Commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if (I_SUPPORT_SCENES == 1)
static void display_ViewRspScene(void)
{
    //BYTE length;
    //BYTE i;
    
    lcdClear();
    lcdPutCur (0, 0);
    lcdPutStr("VIEW SCN:");
    /*lcdPutCur (0, 9);
    display_Byte(&displayGroups.groupID.v[1]);
    lcdPutCur (0, 11);
    display_Byte(&displayGroups.groupID.v[0]);
    lcdPutCur (1, 0);
    length = displayGroups.groupName[0];
    for (i=1; i<=length; i++)
    {
        lcdPutCur (1, i-1);
        lcdPutChar(displayGroups.groupName[i]);
    }*/
}
#endif

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
 * Overview:        This function will display the Events received (like publish price
 *                  or metering) on LCD of the device.
 *
 * Note:            None
 *
 ***************************************************************************************/
//#if(I_AM_ESP == 1)		//MSEB
void HandleESPDisplay( void )
{
    static BYTE previousState;
    DWORD currentTime;
    BYTE i;
    if( displayState == displayTime )
    {
        Display_Time();
        previousState = displayTime;
    }
    else if( displayState == receivedPublishPrice )
    {
        if ( previousState != receivedPublishPrice )
        {
			if(NOW_I_AM_A_ROUTER())
				Display_PublishPrice_MTR();
			else if (NOW_I_AM_A_CORDINATOR())
				Display_PublishPrice_ESP();
            
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedPublishPrice;
        }
    }
    #if defined(I_SUPPORT_GROUP_ADDRESSING)
    else if( displayState == receivedAddGroupCmd )
    {
        if ( previousState != receivedAddGroupCmd )
        {
            display_AddGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddGroupCmd;
        }
    }
    else if( displayState == receivedRemoveGroupCmd )
    {
        if ( previousState != receivedRemoveGroupCmd )
        {
            display_RemoveGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveGroupCmd;
        }
    }
    else if( displayState == receivedViewGroupRspCmd )
    {
        if ( previousState != receivedViewGroupRspCmd )
        {
            display_ViewRspGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewGroupRspCmd;
        }
    }
    #endif
    #if (I_SUPPORT_SCENES == 1)
    else if( displayState == receivedAddSceneCmd )
    {
        if ( previousState != receivedAddSceneCmd )
        {
            display_AddScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddSceneCmd;
        }
    }
    else if( displayState == receivedRemoveSceneCmd )
    {
        if ( previousState != receivedRemoveSceneCmd )
        {
            display_RemoveScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveSceneCmd;
        }
    }
    else if( displayState == receivedViewSceneRspCmd )
    {
        if ( previousState != receivedViewSceneRspCmd )
        {
            display_ViewRspScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewSceneRspCmd;
        }
    }
    #endif
    else
    {
        for( i=0x00;i<25;i++)
        {
            if( displayState == meterAttributes_DisplayStateTable[i].displayState )
            {
                if ( previousState != meterAttributes_DisplayStateTable[i].displayState )
                {
                    DisplayForMeterFormattingAttributes
                    (
                        displayQuotient,
                        remainderArray,
                        displayState
                    );
                    displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                    displayTimer.timerInProgress = TRUE;
                    previousState = displayState;
                }
            }
        }
    }
    if ( displayTimer.timerInProgress )
    {
        currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
        if ( (currentTime - displayTimer.time) > 30 )
        {
            displayState = displayTime;
            displayTimer.timerInProgress = FALSE;
            lcdClear();
        }
    }
}
//#endif
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
#if(I_AM_IPD == 1)
void HandleIPDDisplay( void )
{
    static BOOL startDisplay;
    static WORD previousRemainingMinutes = 0x00;
    static BYTE previousState;
    DWORD currentTime;

    if( (displayState != previousState) && (displayState != displayMessage))
    {
        displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
        displayTimer.timerInProgress = TRUE;
    }

    if
    (
        ( displayState != displayMessage ) &&
        ( previousState == displayMessage ) &&
        ( displayState != receivedCancelMessage )
    )
    {
        displayMessagesTable.confirmationRequired = FALSE;
    }

    switch( displayState )
    {
        case displayTime:
        {
            Display_Time();
            displayTimer.timerInProgress = FALSE;
            previousState = displayTime;
        }
        break;
        case displayMessage:
        {
            static BOOL confRequired;

            currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
            if ( displayMessagesTable.displayMessage.startTime > currentTime )
            {
                Display_Time();
                break;
            }
            else
            {
                if( !startDisplay )
                {
                    WORD elapsedTime;
                    elapsedTime = (( currentTime - displayMessagesTable.displayMessage.startTime )/60 );
                    startDisplay = TRUE;
                    /* Remove the duration in minutes which are already elapsed */
                    if( displayMessagesTable.displayMessage.durationInMinutes != DURATION_UNTIL_CHANGED )
                    {
                        if( elapsedTime > displayMessagesTable.displayMessage.durationInMinutes)
                        {
                            displayMessagesTable.displayMessage.durationInMinutes = 0;
                            displayState = displayTime;
                            confRequired = FALSE;
                            lcdClear();
                            return;
                        }
                        else
                        {
                            displayMessagesTable.displayMessage.durationInMinutes -= elapsedTime;
                        }
                    }
                }
            }

            if( displayMessage != previousState )
            {
                Display_Message();
                lcdPutCur(0,5);
                lcdPutStr("DUR:FFFF");
                displayTimer.timerInProgress = TRUE;
                displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                confRequired = displayMessagesTable.confirmationRequired;
                previousState = displayMessage;
            }

            if (confRequired && !displayMessagesTable.confirmationRequired)
            {
                displayState = displayTime;
                confRequired = FALSE;
                lcdClear();
                return;
            }
            if( displayMessagesTable.displayMessage.durationInMinutes != DURATION_UNTIL_CHANGED )
            {
                if ( displayTimer.timerInProgress )
                {
                    currentTime = ZCL_Callback_GetCurrentTimeInSeconds();

                    WORD remainingMinutes =
                        displayMessagesTable.displayMessage.durationInMinutes - ((currentTime - displayTimer.time) / 60 );

                    if( previousRemainingMinutes != remainingMinutes )
                    {
                        BYTE *temp;
                        previousRemainingMinutes = remainingMinutes;
                        temp = (BYTE *)&remainingMinutes;
                        temp += 1;
                        lcdPutCur(0,9);
                        display_Byte(temp);
                        temp = (BYTE *)&remainingMinutes;
                        lcdPutCur(0,11);
                        display_Byte(temp);
                    }
                    if ( (currentTime - displayTimer.time) >
                        displayMessagesTable.displayMessage.durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE )
                    {
                        displayState = displayTime;
                        displayTimer.timerInProgress = FALSE;
                        displayMessagesTable.confirmationRequired = FALSE;
                        lcdClear();
                    }
                }
            }
        }
        break;
        case receivedMessage:
        {
            previousState = receivedMessage;
            displayState = displayMessage;
            startDisplay = FALSE;
            lcdClear();
            previousRemainingMinutes = 0x00;
        }
        break;

        case receivedGetProfileResponse:
        {
            if( displayState != previousState )
            {
                Display_GetProfileResponse();
                previousState = displayState;
            }
        }
        break;

        case receivedPublishPrice:
        {
            if( receivedPublishPrice != previousState )
            {
				if(NOW_I_AM_A_ROUTER())
					Display_PublishPrice_MTR();
				else if (NOW_I_AM_A_CORDINATOR())
					Display_PublishPrice_ESP();
                previousState = displayState;
            }
        }
        break;

        case receivedDRLCEvent:
        {
            if( receivedDRLCEvent != previousState )
            {
                Display_DRLCEvent();
                previousState = displayState;
            }
        }
        break;

        case receivedCancelMessage:
        {
            static BOOL confRequired;

            if( receivedCancelMessage != previousState )
            {
                lcdClear();
                Display_CancelMessage();

                if( displayMessagesTable.confirmationRequired == TRUE )
                {
                    confRequired = TRUE;
                    lcdPutCur(1,13);
                    lcdPutStr("CNF");

                }
                else
                {
                    confRequired = FALSE;
                }
            }

            if (confRequired && !(displayMessagesTable.confirmationRequired) )
            {
                displayState = displayTime;
                confRequired = FALSE;
                lcdClear();
                return;
            }
            previousState = displayState;
        }
        break;
        case receivedCancelDRLC:
        {
            if( receivedCancelDRLC != previousState )
            {
                Display_CancelDRLC();
                previousState = displayState;
            }
        }
        break;
        case receivedCancelAllDRLC:
        {
            if( receivedCancelAllDRLC != previousState )
            {
                Display_CancelAllDRLC();
                previousState = displayState;
            }
        }
        break;
        case reportAttribute:
        {
            if( reportAttribute != previousState )
            {
                Display_reportAttribute();
                previousState = displayState;
            }
        }
        break;
        case receivedTier1MeterReading:
        case receivedTier2MeterReading:
        case receivedTier3MeterReading:
        case receivedTier4MeterReading:
        case receivedTier5MeterReading:
        case receivedTier6MeterReading:
        {
            if( displayState != previousState )
            {
                DisplayTimeOfUse( TiermeterReadingValue, displayState );
                previousState = displayState;
            }
        }
        break;
        
        #if defined(I_SUPPORT_GROUP_ADDRESSING)
        case receivedAddGroupCmd:
            if( receivedAddGroupCmd != previousState )
            {
                display_AddGroup();
                previousState = displayState;
            }
            break;
        case receivedRemoveGroupCmd:
            if( receivedRemoveGroupCmd != previousState )
            {
                display_RemoveGroup();
                previousState = displayState;
            }
            break;
        case receivedViewGroupRspCmd:
            if( receivedViewGroupRspCmd != previousState )
            {
                display_ViewRspGroup();
                previousState = displayState;
            }
            break;
        #endif
        
        #if (I_SUPPORT_SCENES == 1)
        case receivedAddSceneCmd:
        {
            if ( previousState != receivedAddSceneCmd )
            {
                display_AddScene();
                previousState = receivedAddSceneCmd;
            }
        }
        case receivedRemoveSceneCmd:
        {
            if ( previousState != receivedRemoveSceneCmd )
            {
                display_RemoveScene();
                previousState = receivedRemoveSceneCmd;
            }
        }
        case receivedViewSceneRspCmd:
        {
            if ( previousState != receivedViewSceneRspCmd )
            {
                display_ViewRspScene();
                previousState = receivedViewSceneRspCmd;
            }
        }
        #endif
        
        default:
        break;
    }
    if ( (displayTimer.timerInProgress) && (displayState != displayMessage))
    {
        currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
        if ( (currentTime - displayTimer.time) > 30 )
        {
            if( displayState == receivedCancelMessage )
            {
                displayMessagesTable.confirmationRequired = FALSE;
            }
            displayState = displayTime;
            displayTimer.timerInProgress = FALSE;
            lcdClear();
        }
    }
}
#endif /* (I_AM_IPD == 1) */

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
void HandleLCDDisplay( void )
{
    static BYTE previousState;
    DWORD currentTime;
    if( displayState == displayTime )
    {
        Display_Time();
        previousState = displayTime;
    }
    #if defined(I_SUPPORT_GROUP_ADDRESSING)
    else if( displayState == receivedAddGroupCmd )
    {
        if ( previousState != receivedAddGroupCmd )
        {
            display_AddGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddGroupCmd;
        }
    }
    else if( displayState == receivedRemoveGroupCmd )
    {
        if ( previousState != receivedRemoveGroupCmd )
        {
            display_RemoveGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveGroupCmd;
        }
    }
    else if( displayState == receivedViewGroupRspCmd )
    {
        if ( previousState != receivedViewGroupRspCmd )
        {
            display_ViewRspGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewGroupRspCmd;
        }
    }
    #endif
    #if (I_SUPPORT_SCENES == 1)
    else if( displayState == receivedAddSceneCmd )
    {
        if ( previousState != receivedAddSceneCmd )
        {
            display_AddScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddSceneCmd;
        }
    }
    else if( displayState == receivedRemoveSceneCmd )
    {
        if ( previousState != receivedRemoveSceneCmd )
        {
            display_RemoveScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveSceneCmd;
        }
    }
    else if( displayState == receivedViewSceneRspCmd )
    {
        if ( previousState != receivedViewSceneRspCmd )
        {
            display_ViewRspScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewSceneRspCmd;
        }
    }
    #endif
    if ( displayTimer.timerInProgress )
    {
        currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
        if ( (currentTime - displayTimer.time) > 30 )
        {
            displayState = displayTime;
            displayTimer.timerInProgress = FALSE;
            lcdClear();
        }
    }
}
#endif

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
void HandleMeterDisplay( void )
{
    static BYTE previousState;
    switch( displayState )
    {
        case displayTime:
            if( displayMessagesTable.confirmationRequired)
            {
                displayState = displayMessage;
            }
            Display_Time();
            previousState = displayTime;
            displayTimer.timerInProgress = FALSE;
        break;
        case displayMessage:
        {
            DWORD currentTime;
            static BOOL confRequired;

            if( displayMessage != previousState )
            {
                Display_Message();
                if( displayMessagesTable.confirmationRequired)
                {
                    confRequired = TRUE;
                }
                else
                {
                    confRequired = FALSE;
                    displayTimer.timerInProgress = TRUE;
                    displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                }

                previousState = displayMessage;
            }

            if (confRequired && !displayMessagesTable.confirmationRequired)
            {
                displayState = displayTime;
                confRequired = FALSE;
            }
            if ( displayTimer.timerInProgress )
            {
                currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
                if ( (currentTime - displayTimer.time) >
                    displayMessagesTable.displayMessage.durationInMinutes * 60 )
                {
                    displayState = displayTime;
                    lcdClear();
                }
            }
        }
        break;
        case receivedMessage:
            previousState = receivedMessage;
            displayState = displayMessage;
        break;
        case receivedPublishPrice:
        {
            if ( previousState != receivedPublishPrice )
            {
				if(NOW_I_AM_A_ROUTER())
					Display_PublishPrice_MTR();
				else if (NOW_I_AM_A_CORDINATOR())
					Display_PublishPrice_ESP();
                displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                displayTimer.timerInProgress = TRUE;
                previousState = receivedPublishPrice;
            }
        }
        break;
        
        #if defined(I_SUPPORT_GROUP_ADDRESSING)
        case receivedAddGroupCmd:
            if( receivedAddGroupCmd != previousState )
            {
                display_AddGroup();
                displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                displayTimer.timerInProgress = TRUE;
                previousState = displayState;
            }
            break;
        case receivedRemoveGroupCmd:
            if( receivedRemoveGroupCmd != previousState )
            {
                display_RemoveGroup();
                displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                displayTimer.timerInProgress = TRUE;
                previousState = displayState;
            }
            break;
        case receivedViewGroupRspCmd:
            if( receivedViewGroupRspCmd != previousState )
            {
                display_ViewRspGroup();
                displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                displayTimer.timerInProgress = TRUE;
                previousState = displayState;
            }
            break;
        #endif
        
        #if (I_SUPPORT_SCENES == 1)
        case receivedAddSceneCmd:
        {
            if ( previousState != receivedAddSceneCmd )
            {
                display_AddScene();
                displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                displayTimer.timerInProgress = TRUE;
                previousState = receivedAddSceneCmd;
            }
        }
        case receivedRemoveSceneCmd:
        {
            if ( previousState != receivedRemoveSceneCmd )
            {
                display_RemoveScene();
                displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                displayTimer.timerInProgress = TRUE;
                previousState = receivedRemoveSceneCmd;
            }
        }
        case receivedViewSceneRspCmd:
        {
            if ( previousState != receivedViewSceneRspCmd )
            {
                display_ViewRspScene();
                displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
                displayTimer.timerInProgress = TRUE;
                previousState = receivedViewSceneRspCmd;
            }
        }
        #endif

        default:
        break;

    }
    if ( displayTimer.timerInProgress )
    {
        DWORD currentTime;
        
        currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
        if ( (currentTime - displayTimer.time) > 30 )
        {
            displayState = displayTime;
            displayTimer.timerInProgress = FALSE;
            lcdClear();
        }
    }
}
//#endif

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
void HandlePCTDisplay( void )
{
    static BYTE previousState;
    DWORD currentTime;
    if( displayState == displayTime )
    {
        Display_Time();
        previousState = displayTime;
    }
    #if defined(I_SUPPORT_GROUP_ADDRESSING)
    else if( displayState == receivedAddGroupCmd )
    {
        if ( previousState != receivedAddGroupCmd )
        {
            display_AddGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddGroupCmd;
        }
    }
    else if( displayState == receivedRemoveGroupCmd )
    {
        if ( previousState != receivedRemoveGroupCmd )
        {
            display_RemoveGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveGroupCmd;
        }
    }
    else if( displayState == receivedViewGroupRspCmd )
    {
        if ( previousState != receivedViewGroupRspCmd )
        {
            display_ViewRspGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewGroupRspCmd;
        }
    }
    #endif
    
    #if (I_SUPPORT_SCENES == 1)
    else if( displayState == receivedAddSceneCmd )
    {
        if ( previousState != receivedAddSceneCmd )
        {
            display_AddScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddSceneCmd;
        }
    }
    else if( displayState == receivedRemoveSceneCmd )
    {
        if ( previousState != receivedRemoveSceneCmd )
        {
            display_RemoveScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveSceneCmd;
        }
    }
    else if( displayState == receivedViewSceneRspCmd )
    {
        if ( previousState != receivedViewSceneRspCmd )
        {
            display_ViewRspScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewSceneRspCmd;
        }
    }
    #endif
    
    #if (I_SUPPORT_SCENES == 1)
    else if( displayState == receivedAddSceneCmd )
    {
        if ( previousState != receivedAddSceneCmd )
        {
            display_AddScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddSceneCmd;
        }
    }
    else if( displayState == receivedRemoveSceneCmd )
    {
        if ( previousState != receivedRemoveSceneCmd )
        {
            display_RemoveScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveSceneCmd;
        }
    }
    else if( displayState == receivedViewSceneRspCmd )
    {
        if ( previousState != receivedViewSceneRspCmd )
        {
            display_ViewRspScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewSceneRspCmd;
        }
    }
    #endif
    if ( displayTimer.timerInProgress )
    {
        currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
        if ( (currentTime - displayTimer.time) > 30 )
        {
            displayState = displayTime;
            displayTimer.timerInProgress = FALSE;
            lcdClear();
        }
    }
}
#endif

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
void HandleREDDisplay( void )
{
    static BYTE previousState;
    DWORD currentTime;
    if( displayState == displayTime )
    {
        Display_Time();
        previousState = displayTime;
    }
    #if defined(I_SUPPORT_GROUP_ADDRESSING)
    else if( displayState == receivedAddGroupCmd )
    {
        if ( previousState != receivedAddGroupCmd )
        {
            display_AddGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddGroupCmd;
        }
    }
    else if( displayState == receivedRemoveGroupCmd )
    {
        if ( previousState != receivedRemoveGroupCmd )
        {
            display_RemoveGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveGroupCmd;
        }
    }
    else if( displayState == receivedViewGroupRspCmd )
    {
        if ( previousState != receivedViewGroupRspCmd )
        {
            display_ViewRspGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewGroupRspCmd;
        }
    }
    #endif
    
    #if (I_SUPPORT_SCENES == 1)
    else if( displayState == receivedAddSceneCmd )
    {
        if ( previousState != receivedAddSceneCmd )
        {
            display_AddScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddSceneCmd;
        }
    }
    else if( displayState == receivedRemoveSceneCmd )
    {
        if ( previousState != receivedRemoveSceneCmd )
        {
            display_RemoveScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveSceneCmd;
        }
    }
    else if( displayState == receivedViewSceneRspCmd )
    {
        if ( previousState != receivedViewSceneRspCmd )
        {
            display_ViewRspScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewSceneRspCmd;
        }
    }
    #endif
    
    if ( displayTimer.timerInProgress )
    {
        currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
        if ( (currentTime - displayTimer.time) > 30 )
        {
            displayState = displayTime;
            displayTimer.timerInProgress = FALSE;
            lcdClear();
        }
    }
}
#endif

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
void HandleSAPDisplay( void )
{
    static BYTE previousState;
    DWORD currentTime;
    if( displayState == displayTime )
    {
        Display_Time();
        previousState = displayTime;
    }
    #if defined(I_SUPPORT_GROUP_ADDRESSING)
    else if( displayState == receivedAddGroupCmd )
    {
        if ( previousState != receivedAddGroupCmd )
        {
            display_AddGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddGroupCmd;
        }
    }
    else if( displayState == receivedRemoveGroupCmd )
    {
        if ( previousState != receivedRemoveGroupCmd )
        {
            display_RemoveGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveGroupCmd;
        }
    }
    else if( displayState == receivedViewGroupRspCmd )
    {
        if ( previousState != receivedViewGroupRspCmd )
        {
            display_ViewRspGroup();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewGroupRspCmd;
        }
    }
    #endif
    
    #if (I_SUPPORT_SCENES == 1)
    else if( displayState == receivedAddSceneCmd )
    {
        if ( previousState != receivedAddSceneCmd )
        {
            display_AddScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedAddSceneCmd;
        }
    }
    else if( displayState == receivedRemoveSceneCmd )
    {
        if ( previousState != receivedRemoveSceneCmd )
        {
            display_RemoveScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedRemoveSceneCmd;
        }
    }
    else if( displayState == receivedViewSceneRspCmd )
    {
        if ( previousState != receivedViewSceneRspCmd )
        {
            display_ViewRspScene();
            displayTimer.time = ZCL_Callback_GetCurrentTimeInSeconds();
            displayTimer.timerInProgress = TRUE;
            previousState = receivedViewSceneRspCmd;
        }
    }
    #endif
    
    if ( displayTimer.timerInProgress )
    {
        currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
        if ( (currentTime - displayTimer.time) > 30 )
        {
            displayState = displayTime;
            displayTimer.timerInProgress = FALSE;
            lcdClear();
        }
    }
}
#endif

/******************************************************************************
* Function:        static void DisplayTimeOfUse
*                  (
*                      BYTE *pmeterReadingValue,
*                      BYTE display_State
*                  );
* PreCondition:     None
*
* Input:            None
*
* Output:           None
*
* Return :
*
* Side Effects:     None
*
* Overview:         None
* Note:             None
*****************************************************************************/
#if( I_AM_IPD == 1 )
static void DisplayTimeOfUse
(
    BYTE *pmeterReadingValue,
    BYTE display_State
)
{
    BYTE i;
    BYTE pos = 4;
    BYTE priceTier;
    BYTE priceArray[4];
    lcdClear();
    lcdPutCur (0, 0);
    switch(display_State)
    {
        case receivedTier1MeterReading :
            lcdPutStr("CD1:");
            priceTier = 1;
            break;

        case receivedTier2MeterReading :
            lcdPutStr("CD2:");
            priceTier = 2;
            break;

        case receivedTier3MeterReading :
            lcdPutStr("CD3:");
            priceTier = 3;
            break;

        case receivedTier4MeterReading :
            lcdPutStr("CD4:");
            priceTier = 4;
            break;

        case receivedTier5MeterReading :
            lcdPutStr("CD5:");
            priceTier = 5;
            break;

        case receivedTier6MeterReading :
            lcdPutStr("CD6:");
            priceTier = 6;
            break;


        default :
         return;


    }
    for( i = 0x00; i<0x06; i++ )
    {
        lcdPutCur (0, pos+(i*2));
        display_Byte( &(pmeterReadingValue[5-i]) );

    }

    lcdPutCur (1, 0);
    lcdPutStr("PRICE:");
    priceArray[0]=(BYTE)ListOfPriceEvents[priceTier - 1].price;
    priceArray[1]=(BYTE)(ListOfPriceEvents[priceTier - 1].price>>8);
    priceArray[2]=(BYTE)(ListOfPriceEvents[priceTier - 1].price>>16);
    priceArray[3]=(BYTE)(ListOfPriceEvents[priceTier - 1].price>>24);
    /*memcpy
    (
        &priceArray[0],
        (BYTE *) &(ListOfPriceEvents[priceTier - 1].price),
        0x04
    );*/
    pos = 6;
    for( i = 0x00; i < 4; i++ )
    {
        lcdPutCur (1, pos+(i*2));
        display_Byte( &(priceArray[3-i]) );
    }


}
#endif /*I_AM_IPD==1*/

/*****************************************************************************
  Public Functions
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
*                  LCD
*
* Note:            None
*****************************************************************************/
void Display_Time( void )
{
    BYTE i = 0x00;
    /*arrays to hold the characters that will be displayed*/
    char array1[20];
    char array2[20];
    /*store the previous second,Only if atleast one second is elapsed,
    the function should read RTC and display the time
    in the yy-mm-dd format*/
    static BYTE previousSecond = 0x00;
    /*variable to holds the value read from RTC and converting it to
    ascii*/
    BYTE localVariable;
    /*atleast more than 1 second should have been elapsed, or the second
    should be 0x00*/

    if( mRTCCGetSec() != previousSecond )
    {
        /*update the previous second*/
        previousSecond =  mRTCCGetSec();
        /*fetch hour from RTC and convert it to ascii*/
        localVariable =  ( mRTCCGetHour() );
        array1[i++] = ( ( localVariable & 0xF0 ) >> 4 ) + '0';
        array1[i++] =  ( localVariable & 0x0F ) + '0';
        array1[i++] = ':';
        /*fetch minute from RTC and convert it to ascii*/
        localVariable = (mRTCCGetMin());
        array1[i++] = ( ( localVariable & 0xF0 ) >> 4 ) + 0x30;
        array1[i++] =  ( localVariable & 0x0F ) + 0x30;
        array1[i++] = ':';
        /*fetch second from RTC and convert it to ascii,
        since RTC will be running, to get the latest second is
        retrieved is again*/
        localVariable = (mRTCCGetSec());
        array1[i++] = ( ( localVariable & 0xF0 ) >> 4 ) + 0x30;
        array1[i++] =  ( localVariable & 0x0F ) + 0x30;
        array1[i] = '\0';
        lcdPutCur(2,0);
        /*place the string in LCD*/
        lcdPutStr(array1);
        /*fetch day from RTC and convert it to ascii*/
        /* Reversing This to reflect American style dates mm-day-year */
        //localVariable = (mRTCCGetDay());
        localVariable = (mRTCCGetMonth());
        i = 0x00;
        array2[i++] = ( ( localVariable & 0xF0 ) >> 4 ) + 0x30;
        array2[i++] =  ( localVariable & 0x0F ) + 0x30;

        array2[i++] = '/';
        /*fetch month from RTC and convert it to ascii*/
        //localVariable = (mRTCCGetMonth());
        localVariable = (mRTCCGetDay());
        array2[i++] = ( ( localVariable & 0xF0 ) >> 4 ) + 0x30;
        array2[i++] =  ( localVariable & 0x0F ) + 0x30;
        array2[i++] = '/';
        /*fetch year from RTC and convert it to ascii*/
        localVariable = (mRTCCGetYear());
        array2[i++] = ( ( localVariable & 0xF0 ) >> 4 ) + 0x30;
        array2[i++] =  ( localVariable & 0x0F ) + 0x30;
        array2[i] = '\0';
        lcdPutCur(1,0);
        /*place the string in LCD*/
        lcdPutStr(array2);
    }
}

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
 * Overview:        Display a byte on LCD of the device.
 *
 * Note:            None
 *
 ***************************************************************************************/
void display_Byte(BYTE *val)
{
    BYTE x;

    x = ( (*val & 0xF0) >> 4);
    if (x > 9)
    {
        x += 0x37;
    }
    else
    {
        x += 0x30;
    }
    lcdPutChar(x);
    x = ( *val & 0x0F);
    if (x > 9)
    {
        x += 0x37;
    }
    else
    {
        x += 0x30;
    }
    lcdPutChar(x);
}

