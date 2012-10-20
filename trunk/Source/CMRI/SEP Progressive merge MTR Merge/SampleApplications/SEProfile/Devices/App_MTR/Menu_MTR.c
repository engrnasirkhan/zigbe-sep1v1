/*********************************************************************
 *
 *                Application Simple Metering Device
 *
 *********************************************************************
 * FileName:        SE_Meter.c
 * Dependencies:
 * Processor       : PIC18 / PIC24 / PIC32
 * Complier        : MCC18 v3.00 or higher
 *                   MCC30 v2.05 or higher
 *                   MCC32
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
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      10/16/07    Original
 ********************************************************************/

/****************************************************************************
   Includes
 *****************************************************************************/
/* Configuration Files */
#include "zigbee.def"

/* Generic Files */
#include <stdlib.h>
#include "generic.h"
#include "sralloc.h"
#include "zAIL.h"
#include "console.h"
#include "HAL.h"

/* Stack Related Files */
#include "ZigBeeTasks.h"
#include "zAPS.h"
#include "zAPL.h"
#include "zStack_Configuration.h"
#include "zStack_Profile.h"

/* ZCL Related Files */
#include "ZCL_Interface.h"
#include "ZCL_Foundation.h"
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
#endif /*I_SUPPORT_CBKE*/

#ifdef I_SUPPORT_GROUP_ADDRESSING
    #include "ZCL_Groups.h"
#endif

#if (I_SUPPORT_SCENES == 1)
	#include "ZCL_Scenes.h"
#endif /*I_SUPPORT_SCENES*/

/* SE Related Files */
#include "SE_Interface.h"
#include "SE_Profile.h"
#include "SE_Display.h"
#include "SE_MTR.h"
#if I_SUPPORT_CBKE == 1
    #include "eccapi.h"
    #include "SE_CBKE.h"
#endif /*#if I_SUPPORT_CBKE*/

/* App Related Files */
#include "Menu_MTR.h"
#include "App_Common.h"
#include "App_MTR.h"

/* Libraries */
#if defined(__C30__)
    #include "TLS2130.h"
#else
    #include "lcd PIC32.h"
#endif

#include "MSDCL_Commissioning.h"
#include "msdlclusterhanddle.h"
#include "Delay.h"
#include "monitor.h"

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
/* None */

/******************************************************************************
        Variable Definations
******************************************************************************/
extern BYTE destinationEndPoint;
extern ZIGBEE_PRIMITIVE currentPrimitive_MTR;
extern BYTE AllowJoin;
extern BYTE startMode;

/* Menu System */
/*First Level Menu Options*/
/* Menu System */
/*First Level Menu Options*/
/* Menu System */
/*First Level Menu Options- This menu gives a list of commands
that can be executed over the Meter device.
1: Local Settings - Time settings  - using this option, UTC time can be set on the device
2: Generic Command - This option enables the user to send some generic commands to
    other devices. For example: Read Attribute, Write Attribute command etc
3: Basic Cluster Commands - This option enables the user to send cluster specific commands
    specific to Basic cluster.Example: Reset to factory defaults command.
4: Identify Cluster Commands - This option enables the user to send Cluster specific
    commands specific to Identify cluster.
5: Commissioning Cluster Commands - This option enables the user to execute some
    commissioning cluster specific commands to commission the other device.
6: Message Cluster Commands - This option enables the user to send/cancel some messages
    to/on other devices.Example: Get Last message command
7: Metering Cluster Commands - This option enables the user to execute some of the
    metering cluster commands.Example: Get Profile Command
8: Price Cluster Commands - This option enables the user to execute some of the
    Price cluster commands.Example Get Current Price command
9: DRLC Cluster Commands - This option enables the user to send some of the DRLC
    cluster commands.
a: Inter-PAN - This enables the user to perform data transmission through Inter-PAN.

*/
ROM char * const menu_MTR =
    "\r\n     1: Local Settings"
    "\r\n     2: Generic Commands"
    "\r\n     3: Basic Cluster Commands"
    "\r\n     4: Identify Cluster Commands"
    "\r\n     7: Metering Cluster Commands"
    "\r\n     8: Price Cluster Commands"
    "\r\n     a: Inter-PAN"
    "\r\n     b: Establish Application Link Key With Neighbor Device"
    "\r\n     c: Display"
    "\r\n     d: Alarm Cluster Commands"
    "\r\n     e: Groups Cluster Commands"
    "\r\n     f: Scenes Cluster Commands"
    "\r\n     g: Test Fragmentation"
	"\r\n     m: MSEDCL Cluster Test"
	"\r\n     n: Number of Nodes connected"
    ; // This semicolon line is required

/*Cluster Specific Menu Options
This option enables the device to synchronize time with the time server. This
sends read attribute command of Time cluster - time attribute to ESP assuming
ESP is the time server. Upon receipt of the Read attribute response it
sets the UTC time in its time attribute of time cluster
*/
ROM char * const settingsMenu_MTR =
    "\r\n  You Have Selected Settings Option"
    "\r\n     1: Time Synchronization"
    "\r\n     3: Local Write for single attribute"
    "\r\n     4: Local Binding"
    "\r\n     6: Enable/Disable Permit Join"
    "\r\n     7: Update Start Time of Price Tiers"
    "\r\n     8: Set Destination End Point"
    ; // This semicolon line is required


/*Generic commands Menu
This option enables the user to execute some of the generic commands.Most of the
values are assumed while executing the commands.All generic commands can be performed on
all clusters.But this option limits the user to execute the generic commands
only on some of the clusters
1: Read Attribute Command - This option performs a read attribute command on basic cluster.
    This provides attribute list to select one of the attribute to be
    read namely Application Version, Hardware Version, and Location Description.
2: Write Attribute Command - This option enables the user to perform a write operation
    on basic cluster for Location description.
3: Discover Attribute Command - This option enables the user to discover attributes
    supported by the basic cluster on a particular node.
4: Configure Report Command - This option enables the user to execute configure
    report command.This command will be destined to ESP device. This configures the
    ESP for one attribute to accept report periodically within an interval of 15 seconds.
5: Read Report Command - This option enables the user to read the configured
    report entry on the ESP device.
*/
ROM char * const genericCommandsMenu_MTR =
    "\r\n  You Have Selected Generic Commands"
    "\r\n     1: Read Attribute Command"
    "\r\n     2: Write Attribute Command"
    "\r\n     3: Discover Attribute Command"
    "\r\n     4: Configure Report Command"
    "\r\n     5: Read Report Command"
    ; // This semicolon line is required

/*basicClusterMenu
This option enables the user to execute some of the basic cluster commands.
1: Reset To Factory Defaults Command - This option enables the user to reset all
    the attributes of all the clusters to its default values.
*/
ROM char * const basicClusterMenu_MTR =
    "\r\n  You Have Selected Basic Cluster Commands"
    "\r\n     1: Reset To Factory Defaults Command"
    ; // This semicolon line is required

/*identifyClusterMenu
This option enables the user to execute identify cluster commands.
1: Identify Command - This option enables the user to request other devices to
    identify themseleves.
2: Identify Query Command - This option enables the user to send identify
    query command to the specified destination device.
*/
ROM char * const identifyClusterMenu_MTR =
    "\r\n  You Have Selected Identify Cluster Commands"
    "\r\n     1: Identify Command"
    "\r\n     2: Identify Query Command"
    ; // This semicolon line is required

/*meteringClusterMenu
This option enables the user to execute metering cluster commands.
1: Request Mirror Command - This option enables the device to request mirroring
    functionality on ESP device.
2: Remove Mirror Command - This option enables the device to remove mirroring
    functionality on ESP device.
Note: The commands listed here are applicable for Meter. The commands that are not
    applicable to Meter are not listed here.
*/
ROM char * const meteringClusterMenu_MTR =
    "\r\n  You Have Selected Metering Cluster Commands"
    "\r\n     1: Request Mirror Command"
    "\r\n     2: Remove Mirror Command"
    ; // This semicolon line is required

/*priceClusterMenu
This option enables the user to issue publish price command to other
devices.
1: Get Current Price Command - This option enables the user to send Get current price to
    ESP device.
2: Get Scheduled Price Command - This option enables the user to send Get Scheduled price
    command to ESP device.
Note: The commands listed here are applicable for Meter. The commands that are not
    applicable to Meter are not listed here.
*/
ROM char * const priceClusterMenu_MTR =
    "\r\n  You Have Selected Price Cluster Commands"
    "\r\n     1: Get Current Price Command"
    "\r\n     2: Get Scheduled Price Command"
    ; // This semicolon line is required

/*interPANMenu
This option enables the user to issue commands that can be sent over inter PAN
1: Inter-PAN Get Current Price Command - This option enables the user to send
    Get current price command to ESP over inter-PAN.
2: Inter-PAN Get Scheduled Price Command - This option enables the user to send
    Get Scheduled price command to ESP over inter-PAN.
*/
ROM char * const interPANMenu_MTR =
    "\r\n  You Have Selected Inter-PAN Communication"
    "\r\n     1: Inter-PAN Get Current Price Command"
    "\r\n     2: Inter-PAN Get Scheduled Price Command"
    ; // This semicolon line is required

/*displayMenu
This option enables the user to view some of the information.
User has to decide on what information is required to be displayed
*/
#if I_SUPPORT_CBKE == 1
    ROM char * const displayMenu_MTR =
        "\r\n  You Have Selected Display Option"
        "\r\n     1: Price Events"
        "\r\n     2: Application Link Key (CBKE)"
    ; // This semicolon line is required
#else
    ROM char * const displayMenu_MTR =
        "\r\n  You Have Selected Display Option"
        "\r\n     1: Price Events"
    ; // This semicolon line is required
#endif /*#if I_SUPPORT_CBKE == 1*/

/*alarmClusterMenu
This option enables the user to send Alarm cluster commands to any SE device.
1: Reset Alarm Command - This option enables the user to send reset alarm command
                         to any SE device.
2: Reset All Alarm  Command -This option enables the user to send reset alarm all
                            command to any SE device.
3: Get Alarm Command - This option enables the user to send Get Alarm Command
                       to any SE device.
4. Reset Alarm Log Command - This option enables the user to send reset alarm log
                             Command to any SE device.

*/
ROM char * const alarmClusterMenu_MTR =
    "\r\n  You Have Selected Alarm Cluster Commands"
    "\r\n     1: Reset Alarm Command"
    "\r\n     2: Reset All Alarm Command"
    "\r\n     3: Get Alarm Command"
    "\r\n     4: Reset Alarm Log Command"
    ; // This semicolon line is required

/*groupsClusterMenu
This option enables the user to send Groups cluster commands to any SE device.
1: Add Group Command - This option allows the sending device to add group membership in a
                       particular group for one or more endpoints on the receiving device.
2: View Group Command - This option allows the sending device to request that the receiving
                        entity or entities respond with a view group response command containing the
                        application name string for a particular group.
3: Get Group Membership Command - This option allows the sending device to inquire about
                                  the group membership of the receiving device and endpoint in a number of ways.
4. Remove Group Command - This option allows the sender to request that the receiving entity
                          or entities remove their membership, if any, in a particular group.
5. Remove All Groups Command - This options allows the sending device to direct the receiving
                               entity or entities to remove all group associations.
6. Add Group If Identifying Command - This option allows the sending device to add group
                                      membership in a particular group for one or more endpoints on the receiving
                                      device, on condition that it is identifying itself.

*/
#ifdef I_SUPPORT_GROUP_ADDRESSING
ROM char * const groupsClusterMenu_MTR =
    "\r\n  You Have Selected Groups Cluster Commands"
    "\r\n     1: Add Group Command"
    "\r\n     2: View Group Command"
    "\r\n     3: Get Group Membership Command"
    "\r\n     4: Remove Group Command"
    "\r\n     5: Remove All Groups Command"
    "\r\n     6: Add Group If Identifying Command"
    ; // This semicolon line is required
#endif

/*scenesClusterMenu
This option enables the user to send Scenes cluster commands to any SE device.
1: Add Scene Command - This option allows the sending device to add scene on the receiving device.
2: View Scene Command - This option allows the sending device to request that the receiving
                        entity respond with a view scene response command.
3: Remove Scene Command - This option allows the sender to request that the receiving entity
                          or entities remove a scene.
4. Remove All Scenes Command - This options allows the sending device to direct the receiving
                               entity or entities to remove all scenes associated to the specified group.
5. Store Scene Command - This option allows the sending device to add scene based on the current value 
                         of the extension fields.
6. Recall Scene Command - This option allows the sending device to activate the scene.
7. Get Scene Membership Command - The Get Scene Membership command can be used to find an unused scene
                                  number within the group.
8. Read Scene Extension Attributes - This option is used to verify whether the scene extension attributes
                                     are set to expected values after recalling a scene.
*/
#if (I_SUPPORT_SCENES == 1)
ROM char * const scenesClusterMenu_MTR =
    "\r\n  You Have Selected Scenes Cluster Commands"
    "\r\n     1: Add Scene Command"
    "\r\n     2: View Scene Command"
    "\r\n     3: Remove Scene Command"
    "\r\n     4: Remove All Scenes Command"
    "\r\n     5: Store Scene Command"
    "\r\n     6: Recall Scene Command"
    "\r\n     7: Get Scene Membership Command"
    "\r\n     8: Read Scene Extension Attributes"
    ; // This semicolon line is required
#endif

/*fragmentationMenu
This option enables the user to test the fragmentation feature to send 
upto a maximum of 1k bytes
*/
#if (I_SUPPORT_FRAGMENTATION == 1)
ROM char * const fragmentationMenu_MTR =
    "\r\n  You Have Opted for fragmentation testing "
    "\r\n     1: Initialize fragmentation parameters"
    "\r\n     2: Send data by fragmentation"
    ; // This semicolon line is required
#endif //(I_SUPPORT_FRAGMENTATION == 1)
ZCL_MeteringGetProfileRspCmdPayload MeteringGetProfileResponse;

INPUT_BUFFER_T inputBuf_MTR;
extern BYTE menuLevel1;
extern BYTE menuLevel2;
extern BYTE menuLevel3;
//BYTE inputIndex = 0;
extern BYTE asduData[80];

#if (I_SUPPORT_SCENES == 1)
extern BYTE totalExtensionFieldLength;
extern BYTE extensionFieldIndex;
extern BYTE NumberOfExtensionFields;
#endif

/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
extern void AppSendData
(
    BYTE direction, 
    WORD clusterID, 
    BYTE cmdId, 
    BYTE asduLength, 
    BYTE *pData
);

static void HandleSettingsMenuOptions(void);
static void HandleGenericCommandsMenuOptions(void);
static void HandleBasicClusterMenuOptions(void);
static void HandleIdentifyClusterMenuOptions(void);
static void HandleMeteringClusterMenuOptions(void);
static void HandlePriceClusterMenuOptions(void);
static void HandleInterPANMenuOptions(void);
static void HandleDisplayMenuOptions(void);
static void HandleAlarmClusterMenuOptions(void);
void HandleMettringClusterMenuOptions(void);
void HandleNetworkInformation(void);
void HandleESPDefault_MTR(void);
#ifdef I_SUPPORT_GROUP_ADDRESSING
static void HandleGroupsClusterMenuOptions(void);
#endif
#if (I_SUPPORT_SCENES == 1)
static void HandleScenesClusterMenuOptions(void);
#endif
#if (I_SUPPORT_FRAGMENTATION == 1)
static void HandleFragmentationMenuOptions(void);
#endif //(I_SUPPORT_FRAGMENTATION == 1)
static void ResetMenu( void );

 /*****************************************************************************
  Private Functions
 *****************************************************************************/
/* None */

/*****************************************************************************
  Public Functions
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
 * Overview:        Handles Display on Meter device.
 *
 * Note:            None
 *
 ***************************************************************************************/
void Display_Message( void )
{
    lcdClear();
    lcdPutStr("MSG RECEIVED:");
    if (displayMessagesTable.confirmationRequired)
    {
        lcdPutCur(1,13);
        lcdPutStr("CNF");
    }
    lcdPutCur(1,0);
    lcdPutStr( (SBYTE *)&displayMessagesTable.displayMessage.aDisplayMessage[1]);
}
/***************************************************************************************
 * Function:        void Display_GetProfileResponse()
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
void Display_GetProfileResponse()
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
void Display_PublishPrice_MTR()
{
    BYTE *temp;
    elapsedTimeInfo elapsedTime;

    ZCL_Utility_Time_ConvertSecondsToYear( PublishPriceInfo.StartTime, &elapsedTime );

    lcdClear();
    lcdPutStr("DUR:");
    lcdPutCur(0,4);
    if(elapsedTime.hour > 19)
    {
        elapsedTime.hour += 12;
    }
    else if (elapsedTime.hour > 9)
    {
        elapsedTime.hour += 6;
    }

    display_Byte( &elapsedTime.hour );
    lcdPutCur(0,6);
    lcdPutStr(" To ");
    lcdPutCur(0,10);
    ZCL_Utility_Time_ConvertSecondsToYear( (PublishPriceInfo.StartTime +
        (PublishPriceInfo.DurationInMinutes*60)),
        &elapsedTime );
    if(elapsedTime.hour > 19)
    {
        elapsedTime.hour += 12;
    }
    else if (elapsedTime.hour > 9)
    {
        elapsedTime.hour += 6;
    }
    display_Byte( &elapsedTime.hour );
    lcdPutCur(1,0);
    lcdPutStr("Price:");
    lcdPutCur(1,6);
    temp = (BYTE *)&PublishPriceInfo.Price;
    temp += 3;
    display_Byte( temp );
    lcdPutCur(1,8);
    temp = (BYTE *)&PublishPriceInfo.Price;
    temp += 2;
    display_Byte( temp );
    lcdPutCur(1,10);
    temp = (BYTE *)&PublishPriceInfo.Price;
    temp += 1;
    display_Byte( temp );
    lcdPutCur(1,12);
    temp = (BYTE *)&PublishPriceInfo.Price;
    display_Byte( temp );
}


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
void ProcessMenu_MTR( void )
{
    DISABLE_WDT();

    /*This part of the code is for accepting the First Level Menu Options*/
    if (menuLevel1 == 0xFF || menuLevel1 == 'z' || menuLevel1 == 'b' )
    {
        if ( menuLevel1 == 0xFF )
        {
            menuLevel1 = inputBuf_MTR.buffer[0];
            menuLevel2 = 0xFF;
            menuLevel3 = 0x01;
        }
        printf("\r\n");
        switch (menuLevel1)
        {
            case '1': // Settings
                ConsolePutROMString(settingsMenu_MTR);
                break;
            case '2': //
                ConsolePutROMString(genericCommandsMenu_MTR);
                break;
            case '3': // Basic Cluster
                ConsolePutROMString(basicClusterMenu_MTR);
                break;
            case '4':
                ConsolePutROMString(identifyClusterMenu_MTR);
                break;
            case '7':
                ConsolePutROMString(meteringClusterMenu_MTR);
                break;
            case '8':
                ConsolePutROMString(priceClusterMenu_MTR);
                break;
            case 'a':
                ConsolePutROMString(interPANMenu_MTR);
                break;
            case 'b':
                {
                    BYTE i;

                    switch (menuLevel3)
                    {
                        case 1:
                            printf("\r\nPlease enter the Extended address of the Trust Center: ");
                            menuLevel3 = 0x02;
                            return;
                        case 2:
                            for (i = 0; i < 8; i++)
                            {
                                asduData[7 - i] = GetMACByte(&inputBuf_MTR.buffer[i * 2]);
                            }

                            printf("\r\nPlease enter the Extended address of the Partner device: ");
                            menuLevel3 = 0x03;
                            return;
                        case 3:
                            for (i = 0; i < 8; i++)
                            {
                                asduData[(7 - i) + 8] = GetMACByte(&inputBuf_MTR.buffer[i * 2]);
                            }
                            printf("\r\nPress Enter to Send Request key........... ");
                            menuLevel3 = 0x04;
                            return;

                        case 4:
                            for (i = 0; i < 8; i++)
                            {
                                params.APSME_REQUEST_KEY_request.DestAddress.v[i] = asduData[i];
                            }

                            params.APSME_REQUEST_KEY_request.KeyType = 0x02;

                            for (i = 0; i < 8; i++)
                            {
                                params.APSME_REQUEST_KEY_request.PartnerAddress.v[i] = asduData[i + 8];
                            }

                            currentPrimitive_MTR = APSME_REQUEST_KEY_request;
                            ResetMenu();
                            return;
                        default:
                            inputBuf_MTR.used = 1;
                            ResetMenu();
                            break;
                    }
                }
                break;
            case 'c':
                ConsolePutROMString(displayMenu_MTR);
                break;

            case 'd':
                ConsolePutROMString(alarmClusterMenu_MTR);
                break;

            #ifdef I_SUPPORT_GROUP_ADDRESSING
            case 'e':
                ConsolePutROMString(groupsClusterMenu_MTR);
                break;
            #endif
            
            #if (I_SUPPORT_SCENES == 1)
            case 'f':
                ConsolePutROMString(scenesClusterMenu_MTR);
                break;
            #endif
            #if (I_SUPPORT_FRAGMENTATION == 1)
            case 'g':
                ConsolePutROMString( fragmentationMenu_MTR);
                break;
            #endif //(I_SUPPORT_FRAGMENTATION == 1)		
            case 'z':
                {
                    LONG_ADDR MACExtendedAddress;

                    if ( menuLevel2 == 0xFF )
                    {
                        menuLevel2 = inputBuf_MTR.buffer[0];
                    }

                    switch(menuLevel2)
                    {
                        case '0':
                            startMode = 0;
                            currentPrimitive_MTR = APP_START_DEVICE_request;
                            menuLevel1 = 0xFF;
                            menuLevel2 = 0xFF;
                            menuLevel3 = 0x01;
                            break;

                        case '1':
                            printf("\r\nExtended address taken from zigbee.def file");
                            MACExtendedAddress.v[7] = MAC_LONG_ADDR_BYTE7;
                            MACExtendedAddress.v[6] = MAC_LONG_ADDR_BYTE6;
                            MACExtendedAddress.v[5] = MAC_LONG_ADDR_BYTE5;
                            MACExtendedAddress.v[4] = MAC_LONG_ADDR_BYTE4;
                            MACExtendedAddress.v[3] = MAC_LONG_ADDR_BYTE3;
                            MACExtendedAddress.v[2] = MAC_LONG_ADDR_BYTE2;
                            MACExtendedAddress.v[1] = MAC_LONG_ADDR_BYTE1;
                            MACExtendedAddress.v[0] = MAC_LONG_ADDR_BYTE0;
                            PutMACAddress((BYTE *)&MACExtendedAddress);
                            APLEnable();
                            startMode = 1;
                            currentPrimitive_MTR = APP_START_DEVICE_request;
                            menuLevel1 = 0xFF;
                            menuLevel2 = 0xFF;
                            menuLevel3 = 0x01;
                            App_Reset();
                            break;

                        /*case '2':
                            switch (menuLevel3)
                            {
                                case 1:
                                    printf("\r\nEnter the Extended address (8 Bytes):");
                                    menuLevel3 = 2;
                                    break;
                                case 2:
                                    MACExtendedAddress.v[7] = GetMACByte(&inputBuf_MTR.buffer[0]);
                                    MACExtendedAddress.v[6] = GetMACByte(&inputBuf_MTR.buffer[2]);
                                    MACExtendedAddress.v[5] = GetMACByte(&inputBuf_MTR.buffer[4]);
                                    MACExtendedAddress.v[4] = GetMACByte(&inputBuf_MTR.buffer[6]);
                                    MACExtendedAddress.v[3] = GetMACByte(&inputBuf_MTR.buffer[8]);
                                    MACExtendedAddress.v[2] = GetMACByte(&inputBuf_MTR.buffer[10]);
                                    MACExtendedAddress.v[1] = GetMACByte(&inputBuf_MTR.buffer[12]);
                                    MACExtendedAddress.v[0] = GetMACByte(&inputBuf_MTR.buffer[14]);
                                    PutMACAddress((BYTE *)&MACExtendedAddress);
                                    APLEnable();
                                    startMode = 1;
                                    currentPrimitive = APP_START_DEVICE_request;
                                    menuLevel1 = 0xFF;
                                    menuLevel2 = 0xFF;
                                    menuLevel3 = 0x01;
                                    App_Reset();
                                break;
                            }
                            break;*/

                        default:
                            menuLevel1 = 0xFF;
                            menuLevel2 = 0xFF;
                            PrintMenu_MTR();
                            break;
                    }
                }
                /*Return is used instead of Break here.
                This is because to avoid priniting the Menu Level2 choices*/
                return;

			case 'm':
                HandleMettringClusterMenuOptions();
                break;
			case 'n':
				HandleNetworkInformation();
				break;

            default:
                menuLevel1 = 0xFF;
                PrintMenu_MTR();
                break;
        }
        menuLevel2 = 0xFF;
        printf("\r\nEnter the Menu Choice:");
    }
    else
    {
        if (menuLevel2 == 0xFF)
        {
            menuLevel2 = inputBuf_MTR.buffer[0];
        }
        inputBuf_MTR.used = 1;

        switch (menuLevel1)
        {
            case '1':
                HandleSettingsMenuOptions();
                break;
            case '2':
                HandleGenericCommandsMenuOptions();
                break;
            case '3':
                HandleBasicClusterMenuOptions();
                break;
            case '4':
                HandleIdentifyClusterMenuOptions();
                break;
            case '7':
                HandleMeteringClusterMenuOptions();
                break;
            case '8':
                HandlePriceClusterMenuOptions();
                break;
            case 'a':
                HandleInterPANMenuOptions();
                break;
            case 'c':
                HandleDisplayMenuOptions();
                break;
            case 'd':
                HandleAlarmClusterMenuOptions();
                break;
            #ifdef I_SUPPORT_GROUP_ADDRESSING
            case 'e':
                HandleGroupsClusterMenuOptions();
                break;
            #endif
            #if (I_SUPPORT_SCENES == 1)
            case 'f':
                HandleScenesClusterMenuOptions();
                break;
            #endif
            #if (I_SUPPORT_FRAGMENTATION == 1) 
            case 'g':
                HandleFragmentationMenuOptions();
                break;
            #endif// (I_SUPPORT_FRAGMENTATION == 1)
            default:
                break;
        }
    }
    ENABLE_WDT();
}

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
void ProcessButtonPress(void)
{
    if( currentPrimitive_MTR == NO_PRIMITIVE )
    {
        if (ZigBeeStatus.flags.bits.bNetworkJoined )
        {
            if( SWITCH0 == FALSE )
            {
                while( SWITCH0 == FALSE );
                App_Message_Tick();
            }

            /* Handle sending Group data request when PB is pressed */
            if( SWITCH1 == FALSE )
            {
                while( SWITCH1 == FALSE );
                App_Message_Tick();
            }
        }
    }
}

void HandleESPDefault_MTR(void)
{
	ConsolePutROMString((ROM char * const)"\r\nTakeing Default values from zigbee.def");
	MSDCL_Commission.ExtendedPANId[0] = NWK_EXTENDED_PAN_ID_BYTE0;
	MSDCL_Commission.ExtendedPANId[1] = NWK_EXTENDED_PAN_ID_BYTE1;
	MSDCL_Commission.ExtendedPANId[2] = NWK_EXTENDED_PAN_ID_BYTE2;
	MSDCL_Commission.ExtendedPANId[3] = NWK_EXTENDED_PAN_ID_BYTE3;
	MSDCL_Commission.ExtendedPANId[4] = NWK_EXTENDED_PAN_ID_BYTE4;
	MSDCL_Commission.ExtendedPANId[5] = NWK_EXTENDED_PAN_ID_BYTE5;
	MSDCL_Commission.ExtendedPANId[6] = NWK_EXTENDED_PAN_ID_BYTE6;
	MSDCL_Commission.ExtendedPANId[7] = NWK_EXTENDED_PAN_ID_BYTE7;
	
	
	ConsolePutROMString((ROM char * const)"\r\nEnter the StartUp Control in Hex (01-Form N/W, 02-Join N/W):");
	MSDCL_Commission.StartupStatus = 0x01; //STARTUP_CONTROL_FORM_NEW_NETWORK; //STARTUP_CONTROL_JOIN_NEW_NETWORK; // Always rejoin to specified Extended PANId
	
	
	asduData[20] = (unsigned char)(ALLOWED_CHANNELS);
	asduData[21] = (unsigned char)(ALLOWED_CHANNELS>>8);
	asduData[22] = (unsigned char)(ALLOWED_CHANNELS>>16);
	asduData[23] = (unsigned char)(ALLOWED_CHANNELS>>24);
	

	ConsolePutROMString((ROM char * const)"\r\nEnter the PreConfigured Link Key zeroth byte first:");
	MSDCL_Commission.LinkKey[0] = PRECONFIGURED_LINK_KEY00;
	MSDCL_Commission.LinkKey[1] = PRECONFIGURED_LINK_KEY01;
	MSDCL_Commission.LinkKey[2] = PRECONFIGURED_LINK_KEY02;
	MSDCL_Commission.LinkKey[3] = PRECONFIGURED_LINK_KEY03;
	MSDCL_Commission.LinkKey[4] = PRECONFIGURED_LINK_KEY04;
	MSDCL_Commission.LinkKey[5] = PRECONFIGURED_LINK_KEY05;
	MSDCL_Commission.LinkKey[6] = PRECONFIGURED_LINK_KEY06;
	MSDCL_Commission.LinkKey[7] = PRECONFIGURED_LINK_KEY07;
	MSDCL_Commission.LinkKey[8] = PRECONFIGURED_LINK_KEY08;
	MSDCL_Commission.LinkKey[9] = PRECONFIGURED_LINK_KEY09;
	MSDCL_Commission.LinkKey[10] = PRECONFIGURED_LINK_KEY10;
	MSDCL_Commission.LinkKey[11] = PRECONFIGURED_LINK_KEY11;
	MSDCL_Commission.LinkKey[12] = PRECONFIGURED_LINK_KEY12;
	MSDCL_Commission.LinkKey[13] = PRECONFIGURED_LINK_KEY13;
	MSDCL_Commission.LinkKey[14] = PRECONFIGURED_LINK_KEY14;
	MSDCL_Commission.LinkKey[15] = PRECONFIGURED_LINK_KEY15;
	ConsolePutROMString((ROM char * const)"\r\nEnd of Data Entry \r\n");

	NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
    Reset();
}

/******************************************************************************
* Function:        void HandleSettingsMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform some settings on the device
*                   for example: Setting time etc.
*
* Note:            None
 *****************************************************************************/
static void HandleSettingsMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1':
            switch( menuLevel3 )
            {
                case 1:
                    asduData[0] = 0x00; // ESP Address LSB because it is the Time Server.
                    asduData[1] = 0x00; // ESP Address MSB because it is the Time Server.
                    asduData[2] = 0x00; // Time Attribute ID LSB
                    asduData[3] = 0x00; // Time Attribute ID MSB
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_GenericCommand,
                        ZCL_TIME_CLUSTER,
                        ZCL_Read_Attribute_Cmd,
                        0x02,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;
            }
            break;

        case '3'://Local Write for single attribute
        {
            WORD clusterId;
            WORD attributeId;
            BYTE i;
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the ClusterId (in Hex) (2Bytes):");
                    menuLevel3 = 2;
                    break;
                case 2:
                    /*copy the ClusterId*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    menuLevel3 = 3;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the attributeId (in Hex) (2 Bytes):");
                    break;

                case 3:
                    /*copy the AttributeID*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    menuLevel3 = 4;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Attribute Data Type(in Hex) (1 Byte):");
                    break;

                case 4:
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    menuLevel3 = 5;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the attribute Data length(in Hex) (1 Byte):");
                    break;

                case 5:
                    asduData[5] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    menuLevel3 = 6;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the attribute Data :");
                    break;
                case 6:
                    for( i=0x00;i<asduData[5];i++)
                    {
                        asduData[6 + i] = GetMACByte(&inputBuf_MTR.buffer[((asduData[5]-1)*2) -( i * 2 )]);
                    }
                    memcpy
                    (
                        (BYTE*)&clusterId,
                        (asduData + 0x00 ),
                        ZCL_CLUSTER_ID_LENGTH
                    );
                    memcpy
                    (
                        (BYTE*)&attributeId,
                        (asduData+ 0x02),
                        ZCL_ATTRIBUTE_ID_LENGTH
                    );
                    App_WriteSingleAttribute
                    (
                        METER_ENDPOINT_ID,//EndPoint
                        clusterId,
                        attributeId,
                        asduData[4],//attributeDataType
                        (asduData + 6)//attributeData
                    );
                    ResetMenu();
                break;

                default:
                    ResetMenu();
                    break;
            }
            break;
        }
        case '4'://Local Binding
        {
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the ClusterId (in Hex) (2Bytes):");
                    menuLevel3 = 2;
                    break;
                case 2:
                    /*Updating Binding Entry  ClusterId*/
                    asduData[9] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[10] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    menuLevel3 = 3;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Destination Address(in Hex)(8 Bytes)):");
                    break;

                case 3:
                    /*Updating Binding Entry  DestAddress*/
                    asduData[12] = GetMACByte(&inputBuf_MTR.buffer[14]);
                    asduData[13] = GetMACByte(&inputBuf_MTR.buffer[12]);
                    asduData[14] = GetMACByte(&inputBuf_MTR.buffer[10]);
                    asduData[15] = GetMACByte(&inputBuf_MTR.buffer[8]);
                    asduData[16] = GetMACByte(&inputBuf_MTR.buffer[6]);
                    asduData[17] = GetMACByte(&inputBuf_MTR.buffer[4]);
                    asduData[18] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[19] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    menuLevel3 = 4;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Destination Endpoint (1 Byte):");
                    break;
               case 4:
               {
                    /*Updating Binding Entry  DestEndpoint*/
                    asduData[20] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    LONG_ADDR myLongAddress;
                    WORD_VAL ZDODstAddr;
                    GetMACAddress(&myLongAddress);
                    /*Binding Entry needs to be created in Local Device.so ZDODstAddr
                      should be its Short Address*/
                    ZDODstAddr.Val = macPIB.macShortAddress.Val;
                    /*Updating Binding Entry Source Address(IEEE Address)*/
                    memcpy
                    (
                        asduData,
                        myLongAddress.v,
                        MAX_ADDR_LENGTH
                    );
                    /*Updating Src EndPoint*/
                    asduData[8] = 0x01;
                    /*Updating DestAddressMode*/
                    asduData[11] = APS_ADDRESS_64_BIT;
                    SendZDPRequest( ZDODstAddr, BIND_req, asduData, 0x15);
                    ResetMenu();
               }
                break;

                default:
                    ResetMenu();
                    break;
            }
            break;
        }
        /* Enable or Disable Joining by other devices */
        case '6':   /* permit joining for device */
        {
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\n00=Disable Join, FF=Enable, XX=Timed(sec) : ");
                    menuLevel3 = 2;
                    break;
                case 2:
                    /*Get the Permit Duration entry */
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Destination Short Address (2-bytes) : ");
                    
                    /* TC_Signifance is TRUE for all devices */
                    asduData[1] = TRUE;
                    menuLevel3 = 3;
                    break;                    
                case 3:
                {
                    /* Get the Destination Short Address to send the command */
                    WORD_VAL ZDODstAddr;
                    BYTE     addr1[2];
                    
                    addr1[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    addr1[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    
                    ZDODstAddr.Val =  ( (addr1[1] << 8) | addr1[0] );
                    SendZDPRequest( ZDODstAddr, MGMT_PERMIT_JOINING_req, asduData, 0x02);
                    ResetMenu(); 
                }
                break;
                
                default:
                    ResetMenu();
                    break;
            }
        }
        break;
        
        
        /* Update the start time of Price Tier Information*/       
        case '7':
        {
            BYTE i;
            DWORD currentTimeInSec = 0x00;
            elapsedTimeInfo currentTime;
            
            currentTimeInSec = ZCL_Callback_GetCurrentTimeInSeconds();
            
            ZCL_Utility_Time_ConvertSecondsToYear( currentTimeInSec, &currentTime);
            
            /*Tier and Price information should start from 12:00 AM 
            and the Duration of the Tier is 240 minutes. This is default
            configuration*/
            currentTime.hour = 0x00;
            currentTime.minute = 0x00;
            currentTime.second = 0x00;
            
            currentTimeInSec = ZCL_Utility_Time_ConvertYearToSeconds(&currentTime);
            
            for( i = 0x00; i < numberOfPriceEvents; i++)
            {
                ListOfPriceEvents[i].UTCStartTime = currentTimeInSec;
                currentTimeInSec = currentTimeInSec + (DWORD)( ListOfPriceEvents[i].durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE );
            }
            ResetMenu();
        }
        break;
        
        
        /* Buffer Test Request */
        case '9':   /* Buffer Test Request Primitive */
        {
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"How many bytes are you asking for(hex - max 33): ");
                    menuLevel3 = 2;
                    break;
                case 2:
                    /* number of requested bytes */
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Destination Short Address (2-bytes) : ");
                    menuLevel3 = 3;
                    break;                    
                case 3:
                {
                    /* Get the Destination Short Address to send the command */
                    WORD_VAL ZDODstAddr;
                    BYTE     addr1[2];
                    
                    addr1[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    addr1[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    
                    ZDODstAddr.Val =  ( (addr1[1] << 8) | addr1[0] );
                    SendAPPLRequest( ZDODstAddr, BUFFER_TEST_REQUEST_CLUSTER, asduData, 0x01);
                    ResetMenu(); 
                }
                break;
            }
        } 
        /* Set Destination End Point */
        case '8':
        {
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nSet the Destination End Point : ");
                    menuLevel3 = 2;
                    break;
                case 2:
                    destinationEndPoint = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ResetMenu();
                    break;
                
                default:
                    ResetMenu();
                    break;
            }
        }
        break;
        
        default:
            ResetMenu();
            break;            
    }
}

/******************************************************************************
* Function:        void HandleGenericCommandsMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform generic commands such as
*                   read attribute, write attribute command.
*
* Note:            None
 *****************************************************************************/
extern BYTE attribute_count;
static void HandleGenericCommandsMenuOptions(void)
{
    BYTE count = 0x00;
    BYTE attribute_count_copy;
    switch (menuLevel2)
    {
        case '1': // Read Attribute Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Cluster Id: ");
                    ConsolePutROMString((ROM char * const)"\r\n0000 : Basic 000A : Time");
                    ConsolePutROMString((ROM char * const)"\r\n");                                                           
                    menuLevel3 = 0x03;
                    break;

                case 3:
                    /*copy the Cluster id*/
                    asduData[40] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[41] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the number of attributes to be read :");                    
                    menuLevel3 = 0x04;           
                    break;
                
                case 4:
                    /*Read the total number of attributes*/
                    attribute_count = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the attribute ids");
                    ConsolePutROMString((ROM char * const)"\r\n\r\nFor Basic Cluster:");
                    ConsolePutROMString((ROM char * const)"\r\n0000: App Ver");
                    ConsolePutROMString((ROM char * const)"\r\n0003: Hardware Ver");
                    ConsolePutROMString((ROM char * const)"\r\n0010: Location Desc");                    
                    ConsolePutROMString((ROM char * const)"\r\n\r\nFor Time Cluster:");
                    ConsolePutROMString((ROM char * const)"\r\n0000: Time");
                    ConsolePutROMString((ROM char * const)"\r\n0001: TimeStatus");
                    ConsolePutROMString((ROM char * const)"\r\n0002: TimeZone");   
                    ConsolePutROMString((ROM char * const)"\r\n0003: DstStart");
                    ConsolePutROMString((ROM char * const)"\r\n0004: DstEnd");
                    ConsolePutROMString((ROM char * const)"\r\n0005: DstShift");   
                    ConsolePutROMString((ROM char * const)"\r\n0006: StandardTime");
                    ConsolePutROMString((ROM char * const)"\r\n0007: LocalTime\r\n");   
                    menuLevel3 = 0x05;           
                    break; 
                    
                case 5:
                    {
                        WORD_VAL localClusterId;
                        /*copy the attribute to be read as in the terminal*/
                        attribute_count_copy = attribute_count;
                        while(attribute_count_copy)
                        {                   
                            asduData[count + 2] = GetMACByte(&inputBuf_MTR.buffer[(count*2)+ 2]);
                            asduData[count + 3] = GetMACByte(&inputBuf_MTR.buffer[count*2]);
                            count += 2;
                            attribute_count_copy--;
                        }
                        
                        /*Copy Cluster Id*/                   
                        localClusterId.v[0] = asduData[40];
                        localClusterId.v[1] = asduData[41];
                        
                        /*create read attribute command and
                        queue it up for AIL.*/
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_GenericCommand,
                            localClusterId.Val,
                            ZCL_Read_Attribute_Cmd,
                            ( attribute_count * 2 ),
                            asduData
                        );
                    }
                    ResetMenu();
                    break;
                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Write Attribute Command for basic cluster - location description attribute
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the detination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*Attribute id - location description - 0x0010*/
                    asduData[2] = 0x10;
                    asduData[3] = 0x00;
                    /*attribute data type of location description*/
                    asduData[4] = 0x42;
                    /*length of the character string "Microchip"*/
                    asduData[5] = 0x09;
                    asduData[6] = 'M';
                    asduData[7] = 'i';
                    asduData[8] = 'c';
                    asduData[9] = 'r';
                    asduData[10] = 'o';
                    asduData[11] = 'c';
                    asduData[12] = 'h';
                    asduData[13] = 'i';
                    asduData[14] = 'p';
                    /*create write attribute command for basic cluster - location description
                    attribute and queue it up for AIL.*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_GenericCommand,
                        ZCL_BASIC_CLUSTER,
                        ZCL_Write_Attributes_Cmd,
                        0x0d,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '3': // Discover Attribute Command - discovers all the attributes for basic clsuter
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*start attribute id as 0x0000*/
                    asduData[2] = 0x00; // Start Attribute ID LSB
                    asduData[3] = 0x00; // Attribute ID MSB
                    /*fixing the maximum count as 0x05*/
                    asduData[4] = 0x05; // Maximum Count
                    /*create discover attribute command for basic cluster and send it to the
                    destination device*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_GenericCommand,
                        ZCL_BASIC_CLUSTER,
                        ZCL_Discover_Attributes_Cmd,
                        0x03,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '4': /* Configure Report Command - configure the esp device to accept attribute
                  for current summation deleivered within an interval of 15 seconds*/
            switch( menuLevel3 )
            {
                case 1:
                    asduData[0] = 0x00; // ESP Address LSB - We are fixing ESP to accept the report.
                    asduData[1] = 0x00; // ESP Address MSB - We are fixing ESP to accept the report.
                    /* Direction is Accept Reports*/
                    asduData[2] = 0x01;
                    /* CurrentSummationDelivered attribute id*/
                    asduData[3] = 0x00; // attribute id LSB - CurrentSummationDelivered
                    asduData[4] = 0x00; // attribute id MSB
                    /* timeout within which report attribute command should be received.*/
                    asduData[5] = 0x0F; // Timeout LSB
                    asduData[6] = 0x00; // Timeout MSB
                    /*create configure report attribute command for CurrentSummationDelivered
                    and send it to meter device*/
                    AppSendData
                    (
                        ZCL_ServerToClient | ZCL_GenericCommand,
                        ZCL_METERING_CLUSTER,
                        ZCL_Configure_Reporting_Cmd,
                        0x05,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '5': // Read Report Command - reads the reporting configuration command
            switch( menuLevel3 )
            {
                case 1:
                    // ESP Address LSB - Since ESP is configured to accept report, this command also should be sent to ESP.
                    asduData[0] = 0x00;
                    // ESP Address MSB - Since ESP is configured to accept report, this command also should be sent to ESP.
                    asduData[1] = 0x00;
                    // Direction is Accept Reports
                    asduData[2] = 0x01;
                    asduData[3] = 0x00; // attribute id LSB - CurrentSummationDelivered
                    asduData[4] = 0x00; // attribute id MSB
                    /*create read reporting configuration command for
                    CurrentSummationDelivered
                    0x03 - is the length of report configuration command payload*/
                    AppSendData
                    (
                        ZCL_ServerToClient | ZCL_GenericCommand,
                        ZCL_METERING_CLUSTER,
                        ZCL_Read_Reporting_Configuration_Cmd,
                        3,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        default:
            ResetMenu();
        break;
    }
}

/******************************************************************************
* Function:        void HandleBasicClusterMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform basic cluster specific commands
*                   such as Reset to factory defaults command.
*
* Note:            None
 *****************************************************************************/
static void HandleBasicClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Reset to Factory Defaults
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*create reset to factory defaults command. There is no
                    payload for this command, so the length 0 is passed to the function*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_BASIC_CLUSTER,
                        ZCL_BasicResetToFactoryDefaults,
                        0x00,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        default:
            ResetMenu();
        break;
    }
}

/******************************************************************************
* Function:        void HandleIdentifyClusterMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform identify cluster specific
*                   commands such as Identify, Identify query command.
*
* Note:            None
 *****************************************************************************/
static void HandleIdentifyClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Identify Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr (Unicast or Group):");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*Identify Time interval is set to 30 Seconds by default*/
                    asduData[2] = 0x1E;
                    asduData[3] = 0x00;
                    
                    ConsolePutROMString((ROM char * const)"\r\nEnter Mode of Transmission (01 for Group, 02 for Unicast):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    
                    if ( asduData[4] == 0x01 )
                    {
                        AppSendGroupData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_IDENTIFY_CLUSTER,
                            ZCL_IdentifyCmd,
                            0x02,
                            asduData
                        );
                    }
                    if ( asduData[4] == 0x02 )
                    {
                        /*create identify command, the length of the payload for
                        this command is 2 bytes - identify time.*/
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_IDENTIFY_CLUSTER,
                            ZCL_IdentifyCmd,
                            0x02,
                            asduData
                        );
                    }
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Identify Query Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr (Unicast or Group):");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Mode of Transmission (01 for Group, 02 for Unicast):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    
                    if ( asduData[2] == 0x01 )
                    {
                        /*create identify query command.*/
                        AppSendGroupData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_IDENTIFY_CLUSTER,
                            ZCL_IdentifyQueryCmd,
                            0x00,
                            asduData
                        );
                    }
                    if ( asduData[2] == 0x02 )
                    {
                        /*create identify query command.*/
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_IDENTIFY_CLUSTER,
                            ZCL_IdentifyQueryCmd,
                            0x00,
                            asduData
                        );
                    }
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        default:
            ResetMenu();
        break;
    }
}


/******************************************************************************
* Function:        void HandleMeteringClusterMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform metering cluster specific
*                   commands such as Get profile command
*
* Note:            None
 *****************************************************************************/
static void HandleMeteringClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Request Mirror Command
            switch( menuLevel3 )
            {
                case 1:
                    /*Mirroring command is always sent to ESP*/
                    asduData[0] = 0x00; // ESP Address LSB, Request Mirror command should always be sent to ESP
                    asduData[1] = 0x00; // ESP Address MSB, Request Mirror command should always be sent to ESP
                    /*create request mirror command to ESP device. There is no payload for this
                    command.Hence length is passed as 0 */
                    AppSendData
                    (
                        ZCL_ServerToClient | ZCL_ClusterSpecificCommand,
                        ZCL_METERING_CLUSTER,
                        ZCL_MeteringRequestMirror,
                        0x00,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Remove Mirror Command
            switch( menuLevel3 )
            {
                case 1:
                    /*mirroring is always sent to ESP*/
                    asduData[0] = 0x00; // ESP Address LSB, Request Mirror command should always be sent to ESP
                    asduData[1] = 0x00; // ESP Address MSB, Request Mirror command should always be sent to ESP
                    /*create remove mirror command to ESP device. There is no payload for this
                    command.Hence length is passed as 0 */
                    AppSendData
                    (
                        ZCL_ServerToClient | ZCL_ClusterSpecificCommand,
                        ZCL_METERING_CLUSTER,
                        ZCL_MeteringRemoveMirror,
                        0x00,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        default:
            ResetMenu();
        break;
    }
}

/******************************************************************************
* Function:        void HandlePriceClusterMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform price cluster specific
*                   commands such as Get current price command,
*                   Get scheduled prices command
*
* Note:            None
 *****************************************************************************/
static void HandlePriceClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Get Current Price Command
            switch( menuLevel3 )
            {
                case 1:
                    /*price command is always sent to ESP since price is supported
                    as server on ESP*/
                    asduData[0] = 0x00; // ESP Address LSB, as Price Cluster is supported as Server on ESP only
                    asduData[1] = 0x00; // ESP Address MSB, as Price Cluster is supported as Server on ESP only
                    /* Command Options (RxOnWhenIdle) - it is always true*/
                    asduData[2] = 0x01; // Command Options (RxOnWhenIdle) - it is always true
                    /*create Get current price command with length of the payload is
                    1 bytes*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_PRICE_CLUSTER,
                        ZCL_Price_GetCurrentPriceCmd,
                        0x01,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Get Scheduled Price Command
            switch( menuLevel3 )
            {
                case 1:
                    asduData[0] = 0x00; // ESP Address LSB, as Price Cluster is supported as Server on ESP only
                    asduData[1] = 0x00; // ESP Address MSB, as Price Cluster is supported as Server on ESP only
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Year (in Decimal - yy):");
                    menuLevel3 = 2;
                    break;

                case 2:
                    /*copy the year. The year is specified in 1 byte*/
                    GetMACByteDec(&asduData[2], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 3;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Month (in Decimal - mm):");
                    break;

                case 3:
                    /*copy the month as enetered in the terminal*/
                    GetMACByteDec(&asduData[3], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 4;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - day (in Decimal - dd):");
                    break;

                case 4:
                    /*copy the day*/
                    GetMACByteDec(&asduData[4], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 5;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Hour (24 Hrs format: in Decimal - hh):");
                    break;

                case 5:
                    /*copy the hour*/
                    GetMACByteDec(&asduData[5], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 6;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Minute (in Decimal):");
                    break;

                case 6:
                    /*copy the second*/
                    GetMACByteDec(&asduData[6], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 7;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Number of Events (Any One from 0x00 to 0x06):");
                    break;

                case 7:
                    {
                        elapsedTimeInfo elapsedTime;
                        DWORD UTCStartTime;

                        /*copy the number of events*/
                        asduData[7] = GetMACByte(&inputBuf_MTR.buffer[0]);

                        elapsedTime.year = asduData[2];
                        elapsedTime.month = asduData[3];
                        elapsedTime.day = asduData[4];
                        elapsedTime.hour = asduData[5];
                        elapsedTime.minute = asduData[6];
                        elapsedTime.second = asduData[7];

                        UTCStartTime = ZCL_Utility_Time_ConvertYearToSeconds (&elapsedTime);

                        memcpy
                        (
                            &asduData[2],
                            (BYTE *)&UTCStartTime,
                            4
                        );

                        /*Number of PriceEvents was taken in 7th index. This
                        should be put in 6th index. This index is the continuation
                        of the StartTime and payload creation in the AppSendData
                        requires the payload in the contiguous location*/
                        asduData[6] = asduData[7];

                        /*create Get scheduled price command with length 0x05 bytes*/
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_PRICE_CLUSTER,
                            ZCL_Price_GetScheduledPriceCmd,
                            0x05,
                            asduData
                        );
                        ResetMenu();
                    }
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        default:
            ResetMenu();
        break;
    }
}


/******************************************************************************
* Function:        void HandleInterPANMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to send price command through inter pan
*
* Note:            None
*****************************************************************************/
static void HandleInterPANMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Inter-PAN Get Current Price Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address mode:");
                    ConsolePutROMString((ROM char * const)"\r\n02: Short Addressing (2 Bytes)");
                    ConsolePutROMString((ROM char * const)"\r\n03: Extended Addressing (8 Bytes)");
                    ConsolePutROMString((ROM char * const)"\r\nSelect any one of the option:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination addressing mode*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    if ( asduData[0] == 0x02 )
                    {
                        // ESP Address LSB, as Price Cluster is supported as Server on ESP only
                        asduData[1] = 0x00;
                        // ESP Address MSB, as Price Cluster is supported as Server on ESP only
                        asduData[2] = 0x00;
                        ConsolePutROMString((ROM char * const)"\r\nEnter the Dest PAN Id (2 Bytes):");
                        menuLevel3 = 0x04;
                    }
                    else
                    {
                        ConsolePutROMString((ROM char * const)"\r\nEnter the Inter-PAN ESP Extended address (8 Bytes):");
                        menuLevel3 = 0x03;
                    }
                    break;

                case 3:
                    /*copy the destination address*/
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[14]);
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[12]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[10]);
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[8]);
                    asduData[5] = GetMACByte(&inputBuf_MTR.buffer[6]);
                    asduData[6] = GetMACByte(&inputBuf_MTR.buffer[4]);
                    asduData[7] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[8] = GetMACByte(&inputBuf_MTR.buffer[0]);

                    ConsolePutROMString((ROM char * const)"\r\nEnter the Dest PAN Id (2 Bytes):");
                    menuLevel3 = 0x04;
                    break;

                case 4:
                    {
                        BYTE *pData;
                        /*copy the destination pan id*/
                        params.INTRP_DATA_req.DstPANId.v[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                        params.INTRP_DATA_req.DstPANId.v[1] = GetMACByte(&inputBuf_MTR.buffer[0]);

                        params.INTRP_DATA_req.SrcAddrMode = 0x03; // Always Extended Addressing Mode
                        /*copy the destination addressing mode*/
                        params.INTRP_DATA_req.DstAddrMode = asduData[0];
                        memcpy
                        (
                            params.INTRP_DATA_req.DstAddress.v,
                            &asduData[1],
                            0x08
                        );
                        /*copy the profile id*/
                        params.INTRP_DATA_req.ProfileId.Val = SE_PROFILE_ID;
                        /*copy the cluster id*/
                        params.INTRP_DATA_req.ClusterId.Val = ZCL_PRICE_CLUSTER;
                        /*length of the payload for Get current price command is 4 bytes*/
                        params.INTRP_DATA_req.ASDULength = 0x04; // ZCL Header 3 bytes + 1 byte payload

                        pData =  (BYTE *)SRAMalloc(params.INTRP_DATA_req.ASDULength);
                        if( pData == NULL )
                        {
                            ResetMenu();
                            return;
                        }
                        /*ZCL Header*/
                        *pData = 0x01; // Frame control
                        *( pData + 1 ) = appNextSeqNum++; // Transaction Sequence Number
                        *( pData + 2 ) = ZCL_Price_GetCurrentPriceCmd; // Command Id

                        /*ZCL Payload*/
                        *( pData + 3 ) = 0x01; // Command Options (RxOnWhenIdle) - Always enabled

                        params.INTRP_DATA_req.ASDU = pData;
                        params.INTRP_DATA_req.ASDUHandle = appNextSeqNum; // Handle can be any number.

                        currentPrimitive_MTR = INTRP_DATA_request;

                        ResetMenu();
                    }
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Inter-PAN Get Scheduled Price Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address mode:");
                    ConsolePutROMString((ROM char * const)"\r\n02: Short Addressing (2 Bytes)");
                    ConsolePutROMString((ROM char * const)"\r\n03: Extended Addressing (8 Bytes)");
                    ConsolePutROMString((ROM char * const)"\r\nSelect any one of the option:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    if ( asduData[0] == 0x02 )
                    {
                        // ESP Address LSB, as Price Cluster is supported as Server on ESP only
                        asduData[1] = 0x00;
                        // ESP Address MSB, as Price Cluster is supported as Server on ESP only
                        asduData[2] = 0x00;
                        ConsolePutROMString((ROM char * const)"\r\nEnter the Dest PAN Id (2 Bytes):");
                        menuLevel3 = 0x04;
                    }
                    else
                    {
                        ConsolePutROMString((ROM char * const)"\r\nEnter the Inter-PAN ESP Extended address (8 Bytes):");
                        menuLevel3 = 0x03;
                    }
                    break;

                case 3:
                    /*copy the destination addresss*/
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[14]);
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[12]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[10]);
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[8]);
                    asduData[5] = GetMACByte(&inputBuf_MTR.buffer[6]);
                    asduData[6] = GetMACByte(&inputBuf_MTR.buffer[4]);
                    asduData[7] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[8] = GetMACByte(&inputBuf_MTR.buffer[0]);

                    ConsolePutROMString((ROM char * const)"\r\nEnter the Dest PAN Id (2 Bytes):");
                    menuLevel3 = 0x04;
                    break;

                case 4:
                    /*copy the Destination PANId*/
                    asduData[9] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[10] = GetMACByte(&inputBuf_MTR.buffer[0]);

                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Year (in Decimal - yy):");
                    menuLevel3 = 0x05;
                    break;



                case 5:
                    /*copy the year. The year is specified in 1 byte*/
                    GetMACByteDec(&asduData[11], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 0x06;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Month (in Decimal - mm):");
                    break;

                case 6:
                    /*copy the month as enetered in the terminal*/
                    GetMACByteDec(&asduData[12], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 0x07;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - day (in Decimal - dd):");
                    break;

                case 7:
                    /*copy the day*/
                    GetMACByteDec(&asduData[13], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 0x08;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Hour (24 Hrs format: in Decimal - hh):");
                    break;

                case 8:
                    /*copy the hour*/
                    GetMACByteDec(&asduData[14], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 0x09;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Minute (in Decimal):");
                    break;

                case 9:
                    /*copy the second*/
                    GetMACByteDec(&asduData[15], &inputBuf_MTR.buffer[0], 1);
                    menuLevel3 = 10;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Number of Events (Any One from 0x00 to 0x06):");
                    break;

                case 10:
                    {
                        elapsedTimeInfo elapsedTime;
                        DWORD UTCStartTime;
                        BYTE *pData;

                        /*copy the number of events*/
                        asduData[16] = GetMACByte(&inputBuf_MTR.buffer[0]);

                        elapsedTime.year = asduData[11];
                        elapsedTime.month = asduData[12];
                        elapsedTime.day = asduData[13];
                        elapsedTime.hour = asduData[14];
                        elapsedTime.minute = asduData[15];
                        elapsedTime.second = 0x00; /*Consider Seconds as always zero*/

                        UTCStartTime = ZCL_Utility_Time_ConvertYearToSeconds (&elapsedTime);

                        params.INTRP_DATA_req.DstPANId.v[0] = asduData[9];
                        params.INTRP_DATA_req.DstPANId.v[1] = asduData[10];

                        params.INTRP_DATA_req.SrcAddrMode = 0x03; // Always Extended Addressing Mode
                        params.INTRP_DATA_req.DstAddrMode = asduData[0];
                        memcpy
                        (
                            params.INTRP_DATA_req.DstAddress.v,
                            &asduData[1],
                            0x08
                        );
                        params.INTRP_DATA_req.ProfileId.Val = SE_PROFILE_ID;
                        params.INTRP_DATA_req.ClusterId.Val = ZCL_PRICE_CLUSTER;

                        /*ZCL Header*/
                        /*Since we have updated the params, we can overwrite for the payload*/
                        asduData[0] = 0x01; // Frame control
                        asduData[1] = appNextSeqNum++; // Transaction Sequence Number
                        asduData[2] = ZCL_Price_GetScheduledPriceCmd; // Command Id

                        /*ZCL Payload*/
                        /*Copy the Calculated StartTime*/
                        memcpy
                        (
                            &asduData[3],
                            (BYTE *)&UTCStartTime,
                            0x04
                        );
                        asduData[7] = asduData[16]; // Number of Events requested

                        params.INTRP_DATA_req.ASDULength = 0x08; // ZCL Header 3 bytes + 5 byte payload
                        pData =  (BYTE *)SRAMalloc(params.INTRP_DATA_req.ASDULength);
                        if( pData == NULL )
                        {
                            ResetMenu();
                            return;
                        }
                        memcpy
                        (
                            pData,
                            asduData,
                            0x08
                        );

                        params.INTRP_DATA_req.ASDU = pData;
                        params.INTRP_DATA_req.ASDUHandle = appNextSeqNum; // Handle can be any number.
                        /*inter pan data request*/
                        currentPrimitive_MTR = INTRP_DATA_request;

                        ResetMenu();
                    }
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        default:
            ResetMenu();
        break;
    }
}

/******************************************************************************
* Function:        void HandleDisplayMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to view some of the information
*
* Note:            None
*****************************************************************************/
static void HandleDisplayMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Price Events
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Tier Info:");
                    ConsolePutROMString((ROM char * const)"\r\nTier1(01) - Tier6(06): Specific Tier");
                    ConsolePutROMString((ROM char * const)"\r\nSelect any one of the option:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    {
                        BYTE i;

                        /*copy the Tier Info*/
                        asduData[0] = GetMACByte(&inputBuf_MTR.buffer[0]);

                        /*Loop through the Price Events*/
                        for (i=0; i<MAX_TIERS_SUPPORTED; i++)
                        {
                            /*Check whether the user has requested for specific
                            tier or all the tiers*/
                            if ( ( asduData[0] == ListOfPriceEvents[i].tierInfo ) ||
                                 ( asduData[0] == 0x00 ) )
                            {
                                /*Display on Hyper Terminal*/

                                /*If Specific Tier information is required, then break
                                out. Do not display further tier information.*/
                                if ( ListOfPriceEvents[i].tierInfo == asduData[0] )
                                {
                                    /*------------------------------------------------------------------*/
                                    /*This Block of the code is required to display the time in hours.
                                    This will be with respect to the current date and time*/
                                    DWORD currentTimeInSec = 0x00;
                                    elapsedTimeInfo elapsedTime;

                                    currentTimeInSec = ZCL_Callback_GetCurrentTimeInSeconds();
                                    ZCL_Utility_Time_ConvertSecondsToYear( currentTimeInSec, &elapsedTime );
                                    elapsedTime.hour = 0x00;
                                    elapsedTime.minute = 0x00;
                                    elapsedTime.second = 0x00;

                                    currentTimeInSec = ZCL_Utility_Time_ConvertYearToSeconds ( &elapsedTime );
                                    currentTimeInSec = currentTimeInSec + ListOfPriceEvents[i].startTime;
                                    /*------------------------------------------------------------------*/

                                    // Display the information on LCD
                                    memcpy
                                    (
                                        (BYTE *)&PublishPriceInfo.StartTime,
                                        (BYTE *)&currentTimeInSec,
                                        4
                                    );

                                    //PublishPriceInfo.DurationInMinutes = ( ListOfPriceEvents[i].endTime - ListOfPriceEvents[i].startTime ) / 60;
                                    PublishPriceInfo.DurationInMinutes = ListOfPriceEvents[i].durationInMinutes;

                                    memcpy
                                    (
                                        (BYTE *)&PublishPriceInfo.Price,
                                        (BYTE *)&ListOfPriceEvents[i].price,
                                        4
                                    );
                                    displayState = receivedPublishPrice;

                                    break;
                                }
                            }
                        }
                        ResetMenu();
                    }
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

    #if I_SUPPORT_CBKE == 1
        case '2': // Get the Application Link for the given IEEE address
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the IEEE address whose Application link Key is required:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    {
                        BYTE i;
                        TC_LINK_KEY_TABLE   tcLinkKeyTable;
                        LONG_ADDR aIEEEAddress;
                        /*copy the destination address*/
                        asduData[0] = GetMACByte(&inputBuf_MTR.buffer[14]);
                        asduData[1] = GetMACByte(&inputBuf_MTR.buffer[12]);
                        asduData[2] = GetMACByte(&inputBuf_MTR.buffer[10]);
                        asduData[3] = GetMACByte(&inputBuf_MTR.buffer[8]);
                        asduData[4] = GetMACByte(&inputBuf_MTR.buffer[6]);
                        asduData[5] = GetMACByte(&inputBuf_MTR.buffer[4]);
                        asduData[6] = GetMACByte(&inputBuf_MTR.buffer[2]);
                        asduData[7] = GetMACByte(&inputBuf_MTR.buffer[0]);

                        memcpy( aIEEEAddress.v,asduData,0x08 );
                        if( SearchForTCLinkKey( aIEEEAddress, &tcLinkKeyTable))
                        {
                            ConsolePutROMString((ROM char * const)"\r\nApplication Link Key is: ");
                            for( i = 0x00; i < 16; i++ )
                            {
                                PrintChar( tcLinkKeyTable.LinkKey.v[i]);
                            }
                        }
                        else
                        {
                            ConsolePutROMString((ROM char * const)"\r\n No Application Link Key");
                        }
                        ResetMenu();
                    }
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
    #endif /*#if I_SUPPORT_CBKE == 1*/

        default:
            ResetMenu();
        break;
    }
}
/******************************************************************************
* Function:        BYTE HandleAlarmClusterMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform ALARM cluster specific
*                  commands such as reset alarm,reset all alarm,get alarm,
*                  reset alarm log command.
*
* Note:            None
 *****************************************************************************/
static void HandleAlarmClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Reset Alarm Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*copy the alarm Code*/
                    asduData[2] = 0x01;
                    /*copy the ClusterID as Power Configuration Cluster*/
                    asduData[3] = 0x01;
                    asduData[4] = 0x00;
                    /*create Reset Alarm command, the length of the payload for
                    this command is 3 bytes - Alarm Code,ClusterId.*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_ALARM_CLUSTER,
                        ZCL_ResetAlarmCmd,
                        0x03,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Reset All Alarm Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*create Reset All Alarm command.*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_ALARM_CLUSTER,
                        ZCL_ResetAllAlarmsCmd,
                        0x00,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
        case '3': // Get Alarm Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*create Reset All Alarm command.*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_ALARM_CLUSTER,
                        ZCL_GetAlarmCmd,
                        0x00,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '4': // Reset Alarm Log Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*create Reset All Alarm command.*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_ALARM_CLUSTER,
                        ZCL_ResetAlarmLogCmd,
                        0x00,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        default:
            ResetMenu();
        break;
    }
}

/******************************************************************************
* Function:        BYTE HandleGroupsClusterMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform GROUPS cluster specific
*                  commands such as Add, View, Get Group Membership, Remove,
*                  Remove All Groups, Add Group if Identifying commands.
*
* Note:            None
 *****************************************************************************/
#ifdef I_SUPPORT_GROUP_ADDRESSING
static void HandleGroupsClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Add Group Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group addr:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Length of the Name String (1 Byte in Hex):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                    /*copy the Length of the Name String*/
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Name String (Max 16 Characters):");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                {
                    BYTE i;
                    /*copy the Name String*/
                    for( i=0 ; i < asduData[4]; i++ )
                    {
                        asduData[5 + i] =  inputBuf_MTR.buffer[i];
                    }
                    /*create and send command*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_GROUPS_CLUSTER,
                        ZCL_Groups_AddGroupCmd,
                        asduData[4] + 0x01 + 0x02, /*Name String (no.of bytes entered by user) + Groups Address (2 bytes)*/
                        asduData
                    );
                    ResetMenu();
                }    
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // View Group Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group addr:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*create and send command*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_GROUPS_CLUSTER,
                        ZCL_Groups_ViewGroupCmd,
                        0x02, /*Groups Address (2 bytes)*/
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
            
        case '3': // Get Group Membership Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group Count (1 Byte):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group Count*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    if ( asduData[2] == 0 )
                    {
                        /*create and send command*/
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_GROUPS_CLUSTER,
                            ZCL_Groups_GetGroupMembershipCmd,
                            0x01, /*Groups Count + No Groups Addresses*/
                            asduData
                        );
                        ResetMenu();
                    }
                    else
                    {
                        ConsolePutROMString((ROM char * const)"\r\nEnter the List of Group Addresses :");
                        menuLevel3 = 0x04;
                    }
                    break;

                case 4:
                {
                    /*copy the Group Addresses*/
                    BYTE i;
                    BYTE j=0;
                    /*copy the Name String*/
                    for( i=0 ; i < (asduData[2] * 2); i+=2 )
                    {
                        //asduData[3 + i] =  inputBuf_MTR.buffer[i];
                        asduData[3 + i + 1] = GetMACByte(&inputBuf_MTR.buffer[j]);
                        asduData[3 + i] = GetMACByte(&inputBuf_MTR.buffer[j+2]);
                        j = j + 4;
                    }
                    /*create and send command*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_GROUPS_CLUSTER,
                        ZCL_Groups_GetGroupMembershipCmd,
                        0x01 + (asduData[2] * 2), /*Group Count + List of Group Addresses(2 bytes)*/
                        asduData
                    );
                    ResetMenu();
                }
                    break;
                    
                default:
                    ResetMenu();
                    break;

            }
            break;

        case '4': // Remove Group Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group addr:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*create and send command*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_GROUPS_CLUSTER,
                        ZCL_Groups_RemoveGroupCmd,
                        0x02, /*Groups Address (2 bytes)*/
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
            
        case '5': // Remove All Groups Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    /*create and send command*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_GROUPS_CLUSTER,
                        ZCL_Groups_RemoveAllGroupsCmd,
                        0x00, /*No Payload*/
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
            
        case '6': // Add Group If Identifying Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group addr:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Length of the Name String (1 Byte in Hex):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                    /*copy the Length of the Name String*/
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Name String (Max 16 Characters):");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                {
                    BYTE i;
                    /*copy the Name String*/
                    for( i=0 ; i < asduData[4]; i++ )
                    {
                        asduData[5 + i] =  inputBuf_MTR.buffer[i];
                    }
                    /*create and send command*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_GROUPS_CLUSTER,
                        ZCL_Groups_AddGroupIfIdentifyingCmd,
                        asduData[4] + 0x01 + 0x02, /*Name String (no.of bytes entered by user) + Groups Address (2 bytes)*/
                        asduData
                    );
                    ResetMenu();
                }    
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
            
        default:
            ResetMenu();
        break;
    }
}
#endif

/******************************************************************************
* Function:        BYTE HandleScenesClusterMenuOptions
*                  (
*                      void
*                  );
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
* Overview:        This enables the user to perform SCENES cluster specific
*                  commands such as Add, View, Remove, Remove All Groups, 
*                  Store, Recall, Get Scene Membership commands.
*
* Note:            None
 *****************************************************************************/
#if (I_SUPPORT_SCENES == 1)
static void HandleScenesClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Add Scene Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group ID (2 Byte):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Scene ID (1 Byte):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                    /*copy the Scene ID*/
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Transition Time (2 Bytes):");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                    /*copy the Transition Time*/
                    asduData[5] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[6] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Length of the Scene Name (1 to 16):");
                    menuLevel3 = 0x06;
                    break;
                case 6:
                    /*copy the Length of the Scene Name*/
                    asduData[7] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Name String (Max 16 Characters):");
                    menuLevel3 = 0x07;
                    break;
                case 7:
                {
                    BYTE i;
                    /*copy the Scene Name*/
                    for( i=0 ; i < asduData[7]; i++ )
                    {
                        asduData[8 + i] =  inputBuf_MTR.buffer[i];
                    }
                    extensionFieldIndex = 8 + asduData[7];
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Number of Extension Fields required:");
                    menuLevel3 = 0x08;
                }    
                    break;

                case 8:
                    /*copy the Number of Extension Fields*/
                    NumberOfExtensionFields = GetMACByte(&inputBuf_MTR.buffer[0]);
                    if (NumberOfExtensionFields)
                    {
                        ConsolePutROMString((ROM char * const)"\r\nEnter the ClusterID:");
                        menuLevel3 = 0x09;
                    }    
                    else
                    {
                        /*create and send command*/
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_SCENES_CLUSTER,
                            ZCL_Scenes_AddSceneCmd,
                            5 + (asduData[7] + 1),
                            asduData
                        );
                        ResetMenu();
                    }
                    break;
                    
                case 9:
                    /*copy the ClusterID*/
                    asduData[extensionFieldIndex++] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[extensionFieldIndex++] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    totalExtensionFieldLength = totalExtensionFieldLength + 2;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Length:");
                    menuLevel3 = 10;
                    break;
                    
                case 10:
                    /*copy the Length of the Extension field*/
                    asduData[extensionFieldIndex++] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    totalExtensionFieldLength++;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Extension Field Values:");
                    menuLevel3 = 11;
                    break;
                    
                case 11:
                {
                    BYTE i;
                    BYTE extensionFieldLen;
                    
                    extensionFieldLen = asduData[extensionFieldIndex - 1];
                    totalExtensionFieldLength = totalExtensionFieldLength + extensionFieldLen;
                    
                    /*copy the Name String*/
                    /*for( i=0 ; i < extensionFieldLen; i++ )
                    {
                        asduData[extensionFieldIndex++] =  inputBuf_MTR.buffer[i];
                    }*/
                    for( i=0 ; i < (extensionFieldLen*2); i=i+2 )
                    {
                        asduData[extensionFieldIndex++] =  GetMACByte(&inputBuf_MTR.buffer[i]);
                    }
                    NumberOfExtensionFields--;
                    
                    if (NumberOfExtensionFields)
                    {
                        ConsolePutROMString((ROM char * const)"\r\nEnter the ClusterID:");
                        menuLevel3 = 9;
                    }
                    else
                    {
                        /*create and send command*/
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_SCENES_CLUSTER,
                            ZCL_Scenes_AddSceneCmd,
                            5 + (asduData[7] + 1) + totalExtensionFieldLength,
                            asduData
                        );
                        ResetMenu();
                        
                        totalExtensionFieldLength = 0x00;
                    }
                }    
                    break;
                    
                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // View Scene Command
        case '3': // Remove Scene Command
        case '5': // Store Scene Command
        case '6': // Recall Scene Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr (Unicast or Group):");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group ID(2 Bytes):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Scene ID(1 Byte):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                    /*copy the Scene ID*/
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Mode of Transmission (01 for Group, 02 for Unicast):");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                {
                    BYTE    cmdID = 0x00;
                    
                    /*copy the Mode of Transmission*/
                    asduData[5] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    
                    if (menuLevel2 == '2')
                    {
                        cmdID = ZCL_Scenes_ViewSceneCmd;
                    }
                    if (menuLevel2 == '3')
                    {
                        cmdID = ZCL_Scenes_RemoveSceneCmd;
                    }
                    if (menuLevel2 == '5')
                    {
                        cmdID = ZCL_Scenes_StoreSceneCmd;
                    }
                    if (menuLevel2 == '6')
                    {
                        cmdID = ZCL_Scenes_RecallSceneCmd;
                    }
                    
                    if ( asduData[5] == 0x01 )
                    {
                        AppSendGroupData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_SCENES_CLUSTER,
                            cmdID,
                            0x03, /*Groups ID (2 bytes) + Scene ID (1 Byte)*/
                            asduData
                        );
                    }
                    if ( asduData[5] == 0x02 )
                    {
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_SCENES_CLUSTER,
                            cmdID,
                            0x03, /*Groups ID (2 bytes) + Scene ID (1 Byte)*/
                            asduData
                        );
                    }
                    ResetMenu();
                }    
                    break;

                default:
                    ResetMenu();
                    break;
            }
            break;
            
        case '4': // Remove All Scenes Command
        case '7': // Get Group Membership Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr (Unicast or Group):");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group ID(2 Bytes):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Mode of Transmission (01 for Group, 02 for Unicast):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                {
                    BYTE    cmdID = 0x00;
                    
                    /*copy the Mode of Transmission*/
                    asduData[4] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    
                    if (menuLevel2 == '4')
                    {
                        cmdID = ZCL_Scenes_RemoveAllScenesCmd;
                    }
                    if (menuLevel2 == '7')
                    {
                        cmdID = ZCL_Scenes_GetSceneMembershipCmd;
                    }
                    
                    if ( asduData[4] == 0x01 )
                    {
                        AppSendGroupData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_SCENES_CLUSTER,
                            cmdID,
                            0x02, /*Groups ID (2 bytes)*/
                            asduData
                        );
                    }
                    if ( asduData[4] == 0x02 )
                    {
                        AppSendData
                        (
                            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                            ZCL_SCENES_CLUSTER,
                            cmdID,
                            0x02, /*Groups ID (2 bytes)*/
                            asduData
                        );
                    }
                    ResetMenu();
                }    
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
            
        case '8': // Read Scene Extension Attributes
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Attribute ID");
                    ConsolePutROMString((ROM char * const)"\r\n0011: OccupiedCoolingSetPoint");
                    ConsolePutROMString((ROM char * const)"\r\n0012: OccupiedHeatingSetPoint");
                    ConsolePutROMString((ROM char * const)"\r\n001C: SystemMode");
                    ConsolePutROMString((ROM char * const)"\r\nSelect any of the above or any of your choice:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Attribute ID*/
                    asduData[2] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Cluster ID");
                    ConsolePutROMString((ROM char * const)"\r\n0201 - for Thermostat Cluster");
                    ConsolePutROMString((ROM char * const)"\r\nSelect the above or any of your choice:");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                {
                    WORD_VAL clusterId;
                    
                    /*copy the Cluster ID*/
                    clusterId.v[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    clusterId.v[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                    
                    /*create read attribute command*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_GenericCommand,
                        clusterId.Val, 
                        ZCL_Read_Attribute_Cmd,
                        0x02,
                        asduData
                    );
                    ResetMenu();
                }    
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
            
        default:
            ResetMenu();
        break;
    }
}
#endif
/***************************************************************************************
 * Function:        void HandleFragmentationMenuOptions( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Prints the Fragmentation menu options 
 *
 ***************************************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1) 
static void HandleFragmentationMenuOptions( void )
{
    switch (menuLevel2)
    {
        case '1': // Initialize fragmentation parameters
            UpdateFragmentParams( 0x02, PROFILE_FRAGMENT_INTERFRAME_DELAY, 
                                              0x32, MAX_FRAGMENTATION_SIZE);
            ResetMenu();
            break;
            
        case '2': // Send data by fragmentation
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                {
                    WORD index;
                    
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_MTR.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_MTR.buffer[0]);
                   
                    /* copy message */
                    for( index =0 ; index < MAX_FRAGMENTATION_SIZE; index++ )
                    {                                      
                        fragmentData[index] = index;                          
                    }
                    
                    params.APSDE_DATA_request.ProfileId.Val = ZDP_PROFILE_ID;
                    params.APSDE_DATA_request.SrcEndpoint = SOURCE_ENDPOINT;
                    params.APSDE_DATA_request.DstEndpoint = 0x01;
                    params.APSDE_DATA_request.ClusterId.Val = ZCL_MESSAGING_CLUSTER;
                    params.APSDE_DATA_request.TxOptions.Val = 0x00;
                    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
                            
                    memcpy( (BYTE*)&params.APSDE_DATA_request.DstAddress.ShortAddr.Val,
                            asduData,
                            0x02 );
                                          
                    HandleFragmentDataRequest( MAX_FRAGMENTATION_SIZE, fragmentData);
                    ResetMenu();
                }    
                    break;                 
                                           
                default:
                    ResetMenu();
                    break;

            }
            break;
        
        default:
            ResetMenu();
            break;
    }
    
}
#endif //#if (I_SUPPORT_FRAGMENTATION == 1)

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
void PrintMenu_MTR( void )
{
    ConsolePutROMString( (ROM char *)"\r\n" );
    ConsolePutROMString( (ROM char *)"\r\nShort Address of MTR:" );

    PrintChar( macPIB.macShortAddress.byte.MSB );
    PrintChar( macPIB.macShortAddress.byte.LSB );

    ConsolePutROMString(menu_MTR);
    ConsolePutROMString( (ROM char * const) "\r\nEnter a menu choice: " );
}

/***************************************************************************************
 * Function:        static void ResetMenu( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Resets the Menu level and calls print menu .
 *
 ***************************************************************************************/
static void ResetMenu( void )
{
    menuLevel1 = 0xFF;
    menuLevel2 = 0xFF;
    menuLevel3 = 0x01;
    PrintMenu_MTR();
}

