/*********************************************************************
 *
 *                Menu Options for Energy Service Portal (ESP)
 *
 *********************************************************************
 * FileName:        Menu.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v2.05 or higher
 *
 * Software License Agreement
 *
 * Copyright � 2004-2007 Microchip Technology Inc.  All rights reserved.
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
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY OF ANY
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
 *
 ********************************************************************/

/****************************************************************************
   Includes
 *****************************************************************************/
/* Configuration files */
#include "zigbee.def"

/* Generic Files */
#include "generic.h"
#include "sralloc.h"
#include "Console.h"
#include "zNVM.h"

/* Stack Related Files */
#include "zAIL.h"
#include "zAPS.h"
#include "zAPL.h"

/* ZCL Related Files */
#include "ZCL_Interface.h"
#include "ZCL_Foundation.h"
#include "ZCL_Identify.h"
#include "ZCL_Basic.h"
#include "ZCL_Time.h"
#include "ZCL_Message.h"
#include "ZCL_Price.h"
#include "ZCL_DRLC.h"
#include "ZCL_Metering.h"
#include "ZCL_Power_Configuration.h"
#include "ZCL_Alarm.h"
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
#include "SE_ESP.h"
#include "SE_MTR.h"
#include "SE_Display.h"
#if I_SUPPORT_CBKE == 1
    #include "eccapi.h"
    #include "SE_CBKE.h"
#endif /*#if I_SUPPORT_CBKE*/

/* App Related Files */
#include "Menu_ESP.h"
#include "App_Common.h"
#include "App_ESP.h"
#include "App_MTR.h"
/* Libraries */
#if defined(__C30__)
    #include "TLS2130.h"
#else
    #include "lcd PIC32.h"
#endif

#include "msdlclusterhanddle.h"
#include "MSDCL_Commissioning.h"

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
extern ZIGBEE_PRIMITIVE currentPrimitive_ESP;
extern ZIGBEE_PRIMITIVE currentPrimitive_MTR;
extern ZIGBEE_PRIMITIVE    *currentPrimitive_PTR;

extern ALARM_AlarmTable AlarmEntries[];

/* Menu System */
/*First Level Menu Options- This menu gives a list of commands
that can be executed over the ESP device.
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
    to/on other devices.Example: Display message command
7: Metering Cluster Commands - This option enables the user to execute some of the
    metering cluster commands.Example: Get Profile Command
8: Price Cluster Commands - This option enables the user to execute some of the
    Price cluster commands.Example Publish Price command
9: DRLC Cluster Commands - This option enables the user to send some of the DRLC
    cluster commands.
a: Inter-PAN - This enables the user to perform data transmission through Inter-PAN.

*/
ROM char * const menu_ESP =
    "\r\n     1: Local Settings"
    "\r\n     2: Generic Commands"
    "\r\n     3: Basic Cluster Commands"
    "\r\n     4: Identify Cluster Commands"
    "\r\n     5: Commissioning Cluster Commands"
    "\r\n     6: Message Cluster Commands"
    "\r\n     7: Metering Cluster Commands"
    "\r\n     8: Price Cluster Commands"
    "\r\n     9: DRLC Cluster Commands"
    "\r\n     a: Inter-PAN"
    "\r\n     c: Display"
    "\r\n     d: Alarm Cluster Commands"
    "\r\n     e: Groups Cluster Commands"
    "\r\n     f: Scenes Cluster Commands"
    "\r\n     g: Test Fragmentation"
    "\r\n     h: Link Status Command Testing"
	"\r\n	  m: MSEDCL Cluster test"
	"\r\n	  n: Find Number of Node connected"
	"\r\n     o: Make ESP as MTR"
	"\n\r     q: Discover Route"
	"\r\n     p: Make MTR as ESP with Globle strrings"
    ; // This semicolon line is required

/*Cluster Specific Menu Options
The time time settings enables the user to set the current system time in terms of
year, month, day, hour,  minute, and second. All the data are in decimal. The hour
takes a value in the 24 hour format
*/
ROM char * const settingsMenu_ESP =
    "\r\n  You Have Selected Settings Option"
    "\r\n     1: Time Settings"
    "\r\n     2: Update Price Settings"
    "\r\n     3: Local Write for Single Attr"
    "\r\n     4: Local Binding"
    "\r\n     5: Set DRLC Event"
    "\r\n     6: Enable/Disable Permit Join"
    "\r\n     7: Update Start Time of Price Tiers"
    "\r\n     8: Set Destination End Point"
    "\r\n     9: Request Customer Specific Cluster Info"
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
    report command.This command will be destined to Meter device. This configures the
    meter for one attribute to report periodically at an interval 10 seconds.
5: Read Report Command - This option enables the user to read the configured
    report entry on the meter device.
*/
ROM char * const genericCommandsMenu_ESP =
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
ROM char * const basicClusterMenu_ESP =
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
ROM char * const identifyClusterMenu_ESP =
    "\r\n  You Have Selected Identify Cluster Commands"
    "\r\n     1: Identify Command"
    "\r\n     2: Identify Query Command"
    ; // This semicolon line is required

/*commissioningClusterMenu - This is enabled only on ESP device.
This option enables the user to commission other devices in the network.
1: Commission the Device - This enables the user to commission the start up
    parameters on the remote device.
2: Restart Device Command - This option enables the remote device to restart the
    device using the commissioned start up parameters.
3: Save SAS Command - This option enables the user to save the Start up parameters
    which was commissioned into NVM.
4: Restore SAS Command - This option enables the user to restores the Start up parameters
    which was commissioned from NVM to ZCL commissioning attribute set.
5: Reset SAS Command - This option enables the user to reset the start up parameters
    ( can be zcl commissioning attribute set or stored in NVM) to default values.
*/
ROM char * const commissioningClusterMenu =
    "\r\n  You Have Selected Commissioning Cluster Commands"
    "\r\n     1: Commission the Device"
    "\r\n     2: Restart Device Command"
    "\r\n     3: Save SAS Command"
    "\r\n     4: Restore SAS Command"
    "\r\n     5: Reset SAS Command"
    ; // This semicolon line is required

/*messageClusterMenu
This option enables the device to execute Message cluster commands.
1: Display Message Command - This device enables the user to send display message
    command to IPD device. The payload for this command consists of predefined
    values.
2: Cancel Message Command - This device enables the user to cancel the message
    command sent to IPD device.

Note: The commands listed here are applicable for ESP. The commands that are not
    applicable to ESP are not listed here.
*/
ROM char * const messageClusterMenu =
    "\r\n  You Have Selected Message Cluster Commands"
    "\r\n     1: Display Message Command"
    "\r\n     2: Cancel Message Command"
    ; // This semicolon line is required

/*meteringClusterMenu
This option enables the user to execute metering cluster commands.
1: Get Profile Request Command - This option enables the user to send
    get profile command to Meter device.
Note: The commands listed here are applicable for ESP. The commands that are not
    applicable to ESP are not listed here.
*/
ROM char * const meteringClusterMenu_ESP =
    "\r\n  You Have Selected Metering Cluster Commands"
    "\r\n     1: Get Profile Request Command"
    ; // This semicolon line is required

/*priceClusterMenu
This option enables the user to issue publish price command to other
devices.
1: Publish Price Command - This option enables the user to send publish price
    command to the destination address. The payload for this command consists of
    predefined values.
Note: The commands listed here are applicable for ESP. The commands that are not
    applicable to ESP are not listed here.
*/
ROM char * const priceClusterMenu_ESP =
    "\r\n  You Have Selected Price Cluster Commands"
    "\r\n     1: Publish Price Command"
    ; // This semicolon line is required

/*drlcClusterMenu
This option enables the user to send DRLC cluster commands to IPD device.
1: Load Control Event Command - This option enables the user to send load control
    event command to IPD device.
2: Cancel Load Control Event Command -This option enables the user to send cancel load control
    event command to IPD device.
3: Cancel All Load Control Events Command - This option enables the user to send cancel all
    load control event command to IPD device.
Note: The commands listed here are applicable for ESP. The commands that are not
    applicable to ESP are not listed here.
*/
ROM char * const drlcClusterMenu =
    "\r\n  You Have Selected DRLC Cluster Commands"
    "\r\n     1: Load Control Event Command"
    "\r\n     2: Cancel Load Control Event Command"
    "\r\n     3: Cancel All Load Control Events Command"
    ; // This semicolon line is required

/*interPANMenu
This option enables the user to issue commands that can be sent over inter PAN
1: Inter-PAN Publish Price Command - This option enables the user to send publish price
    command to other devices over inter-PAN.
*/
ROM char * const interPANMenu_ESP =
    "\r\n  You Have Selected Inter-PAN Communication"
    "\r\n     1: Inter-PAN Publish Price Command"
    "\r\n     2: Display Message Command"
    "\r\n     3: Cancel Message Command"
    ; // This semicolon line is required

/*displayMenu
This option enables the user to view some of the information.
User has to decide on what information is required to be displayed
*/
#if I_SUPPORT_CBKE == 1
    ROM char * const displayMenu_ESP =
    "\r\n  You Have Selected Display Option"
    "\r\n     1: Price Events"
    "\r\n     2: Application Link Key (CBKE)"
    ; // This semicolon line is required
#else
    ROM char * const displayMenu_ESP =
    "\r\n  You Have Selected Display Option"
    "\r\n     1: Price Events"
    ; // This semicolon line is required
#endif

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
ROM char * const alarmClusterMenu_ESP =
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
ROM char * const groupsClusterMenu_ESP =
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
ROM char * const scenesClusterMenu_ESP =
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
ROM char * const fragmentationMenu_ESP =
    "\r\n  You Have Opted for fragmentation testing "
    "\r\n     1: Initialize fragmentation parameters"
    "\r\n     2: Send data by fragmentation"
    ; // This semicolon line is required
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*linkStatusTestingMenu
This option enables the user to test the NVM store of age parameter of the
link status command frame when ever there is an updation
*/
#if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
ROM char * const linkStatusTestingMenu = 
    "\r\n  You have opted for link status command testing "
    "\r\n     1. Enter the Link Status Details: "
;
#endif//#if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )

INPUT_BUFFER_T_ESP inputBuf_ESP;
BYTE menuLevel1;
BYTE menuLevel2;
BYTE menuLevel3;
BYTE inputIndex = 0;
BYTE asduData[80];


extern unsigned int NumberOfDevicesConnected;

#if (I_SUPPORT_SCENES == 1)
BYTE totalExtensionFieldLength = 0x00;
BYTE extensionFieldIndex;
BYTE NumberOfExtensionFields;
#endif

/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
extern void AppSendData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData);

static void HandleSettingsMenuOptions(void);
static void HandleGenericCommandsMenuOptions(void);
static void HandleBasicClusterMenuOptions(void);
static void HandleIdentifyClusterMenuOptions(void);
static void HandleCommissioningClusterMenuOptions(void);
static void HandleMessageClusterMenuOptions(void);
static void HandleMeteringClusterMenuOptions(void);
static void HandlePriceClusterMenuOptions(void);
static void HandleDRLCClusterMenuOptions(void);
static void HandleInterPANMenuOptions(void);
static void HandleDisplayMenuOptions(void);
static void HandleAlarmClusterMenuOptions(void);
void HandleMettringClusterMenuOptions(void);
void HandleNetworkInformation(void);

void HandleESPDefault(void);
void HandleESPtoMtr(void);

#ifdef I_SUPPORT_GROUP_ADDRESSING
static void HandleGroupsClusterMenuOptions(void);
#endif
#if (I_SUPPORT_SCENES == 1)
static void HandleScenesClusterMenuOptions(void);
#endif

#if (I_SUPPORT_FRAGMENTATION == 1)
static void HandleFragmentationMenuOptions(void);
#endif //(I_SUPPORT_FRAGMENTATION == 1)

#if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
static void HandleLinkStatusTesting( void );
#endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )

static void ResetMenu(void);

/*****************************************************************************
  Private Functions
 *****************************************************************************/
/* None */

/*****************************************************************************
  Public Functions
 *****************************************************************************/

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
void ProcessMenu_ESP( void )
{
    DISABLE_WDT();

    /*This part of the code is for accepting the First Level Menu Options*/
    if (menuLevel1 == 0xFF || menuLevel1 == 'z')
    {
        if ( menuLevel1 == 0xFF )
        {
            menuLevel1 = inputBuf_ESP.buffer[0];
            menuLevel2 = 0xFF;
            menuLevel3 = 0x01;
        }
        printf("\r\n");
        switch (menuLevel1)
        {
            case '1': // Settings
                ConsolePutROMString(settingsMenu_ESP);
                break;
            case '2': //
                ConsolePutROMString(genericCommandsMenu_ESP);
                break;
            case '3': // Basic Cluster
                ConsolePutROMString(basicClusterMenu_ESP);
                break;
            case '4':
                ConsolePutROMString(identifyClusterMenu_ESP);
                break;
            case '5':
                ConsolePutROMString(commissioningClusterMenu);
                break;
            case '6':
                ConsolePutROMString(messageClusterMenu);
                break;
            case '7':
                ConsolePutROMString(meteringClusterMenu_ESP);
                break;
            case '8':
                ConsolePutROMString(priceClusterMenu_ESP);
                break;
            case '9':
                ConsolePutROMString(drlcClusterMenu);
                break;
            case 'a':
                ConsolePutROMString(interPANMenu_ESP);
                break;
            case 'c':
                ConsolePutROMString(displayMenu_ESP);
                break;
            case 'd':
                ConsolePutROMString(alarmClusterMenu_ESP);
                break;
            #ifdef I_SUPPORT_GROUP_ADDRESSING
            case 'e':
                ConsolePutROMString(groupsClusterMenu_ESP);
                break;
            #endif
            #if (I_SUPPORT_SCENES == 1)
            case 'f':
                ConsolePutROMString(scenesClusterMenu_ESP);
                break;
            #endif
            #if (I_SUPPORT_FRAGMENTATION == 1)
            case 'g':
                ConsolePutROMString( fragmentationMenu_ESP);
                break;
            #endif //(I_SUPPORT_FRAGMENTATION == 1)
            #if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
            case 'h':
                ConsolePutROMString(linkStatusTestingMenu);
                break;
            #endif//(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )

			case 'm':
                HandleMettringClusterMenuOptions();
                break;
			case 'n':
				HandleNetworkInformation();
				break;
			case 'q':
				HandleDiscoverRoute();
				break;
            case 'z':
                {
                    LONG_ADDR MACExtendedAddress;

                    if ( menuLevel2 == 0xFF )
                    {
                        menuLevel2 = inputBuf_ESP.buffer[0];
                    }

                    switch(menuLevel2)
                    {
                        case '0':
                            startMode = 0;
                            currentPrimitive_ESP = APP_START_DEVICE_request;
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
                            currentPrimitive_ESP = APP_START_DEVICE_request;
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
                                    MACExtendedAddress.v[7] = GetMACByte(&inputBuf_ESP.buffer[0]);
                                    MACExtendedAddress.v[6] = GetMACByte(&inputBuf_ESP.buffer[2]);
                                    MACExtendedAddress.v[5] = GetMACByte(&inputBuf_ESP.buffer[4]);
                                    MACExtendedAddress.v[4] = GetMACByte(&inputBuf_ESP.buffer[6]);
                                    MACExtendedAddress.v[3] = GetMACByte(&inputBuf_ESP.buffer[8]);
                                    MACExtendedAddress.v[2] = GetMACByte(&inputBuf_ESP.buffer[10]);
                                    MACExtendedAddress.v[1] = GetMACByte(&inputBuf_ESP.buffer[12]);
                                    MACExtendedAddress.v[0] = GetMACByte(&inputBuf_ESP.buffer[14]);
                                    PutMACAddress((BYTE *)&MACExtendedAddress);
                                    APLEnable();
                                    startMode = 1;
                                    currentPrimitive_ESP = APP_START_DEVICE_request;
                                    menuLevel1 = 0xFF;
                                    menuLevel2 = 0xFF;
                                    menuLevel3 = 0x01;
                                    App_Reset();
                                break;
                            }
                            break;
                        */
                        default:
                            menuLevel1 = 0xFF;
                            menuLevel2 = 0xFF;
                            PrintMenu_ESP();
                            break;
                    }
                }
                /*Return is used instead of Break here.
                This is because to avoid priniting the Menu Level2 choices*/
                return;
//			case 'm':
//                HandleMettringClusterMenuOptions();
//                break;
//			case 'n':
//				HandleNetworkInformation();
//				break;
			case 'o':
				HandleESPtoMtr();
				break;
			case 'p':
				HandleESPDefault();	
				break;
            default:
                menuLevel1 = 0xFF;
                PrintMenu_ESP();
                break;
        }
        menuLevel2 = 0xFF;
        printf("\r\nEnter the Menu Choice:");
    }
    else
    {
        if (menuLevel2 == 0xFF)
        {
            menuLevel2 = inputBuf_ESP.buffer[0];
        }
        inputBuf_ESP.used = 1;

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
            case '5':
                HandleCommissioningClusterMenuOptions();
                break;
            case '6':
                HandleMessageClusterMenuOptions();
                break;
            case '7':
                HandleMeteringClusterMenuOptions();
                break;
            case '8':
                HandlePriceClusterMenuOptions();
                break;
            case '9':
                HandleDRLCClusterMenuOptions();
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
            #if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
            case 'h':
                HandleLinkStatusTesting();
                break;
            #endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
			case 'm':
                HandleMettringClusterMenuOptions();
                break;
			case 'n':
				HandleNetworkInformation();
				break;			
			case 'q':
				HandleDiscoverRoute();
				break;
            default:
                break;
        }
    }
    ENABLE_WDT();
}


void HandleESPDefault(void)
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

void HandleESPtoMtr(void)
{

	ConsolePutROMString((ROM char * const)"\r\nEnter the ExtendedPANId in Hex 0th First:");
	MSDCL_Commission.ExtendedPANId[0] = GetHexByte();
	MSDCL_Commission.ExtendedPANId[1] = GetHexByte();
	MSDCL_Commission.ExtendedPANId[2] = GetHexByte();
	MSDCL_Commission.ExtendedPANId[3] = GetHexByte();
	MSDCL_Commission.ExtendedPANId[4] = GetHexByte();
	MSDCL_Commission.ExtendedPANId[5] = GetHexByte();
	MSDCL_Commission.ExtendedPANId[6] = GetHexByte();
	MSDCL_Commission.ExtendedPANId[7] = GetHexByte();
	
	
	ConsolePutROMString((ROM char * const)"\r\nEnter the StartUp Control in Hex (01-Form N/W, 02-Join N/W):");
	MSDCL_Commission.StartupStatus = GetHexByte(); //STARTUP_CONTROL_FORM_NEW_NETWORK; //STARTUP_CONTROL_JOIN_NEW_NETWORK; // Always rejoin to specified Extended PANId
	
	
	asduData[20] = (unsigned char)(ALLOWED_CHANNELS);
	asduData[21] = (unsigned char)(ALLOWED_CHANNELS>>8);
	asduData[22] = (unsigned char)(ALLOWED_CHANNELS>>16);
	asduData[23] = (unsigned char)(ALLOWED_CHANNELS>>24);
	

	ConsolePutROMString((ROM char * const)"\r\nEnter the PreConfigured Link Key zeroth byte first:");
	MSDCL_Commission.LinkKey[0] = 0x5a;
	MSDCL_Commission.LinkKey[1] = 0x69;
	MSDCL_Commission.LinkKey[2] = 0x67;
	MSDCL_Commission.LinkKey[3] = 0x42;
	MSDCL_Commission.LinkKey[4] = 0x65;
	MSDCL_Commission.LinkKey[5] = 0x65;
	MSDCL_Commission.LinkKey[6] = 0x41;
	MSDCL_Commission.LinkKey[7] = 0x6c;
	MSDCL_Commission.LinkKey[8] = 0x6c;
	MSDCL_Commission.LinkKey[9] = 0x69;
	MSDCL_Commission.LinkKey[10] = 0x61;
	MSDCL_Commission.LinkKey[11] = 0x6e;
	MSDCL_Commission.LinkKey[12] = 0x63;
	MSDCL_Commission.LinkKey[13] = 0x65;
	MSDCL_Commission.LinkKey[14] = 0x30;
	MSDCL_Commission.LinkKey[15] = 0x39;
	ConsolePutROMString((ROM char * const)"\r\nEnd of Data Entry \r\n");

	NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
    Reset();
	

}

/******************************************************************************
* Function:        BYTE HandleSettingsMenuOptions
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
        case '1'://Time setting
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Yr (in Decimal - yy):");
                    menuLevel3 = 2;
                    break;
                case 2:
                    /*copy the year. The year is specified in 1 byte*/
                    GetMACByteDec(&asduData[0], &inputBuf_ESP.buffer[0], 1);
                    menuLevel3 = 3;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Mon (in Decimal - mm):");
                    break;

                case 3:
                    /*copy the month as enetered in the terminal*/
                    GetMACByteDec(&asduData[1], &inputBuf_ESP.buffer[0], 1);
                    menuLevel3 = 4;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the day (in Decimal - dd):");
                    break;

                case 4:
                    /*copy the day*/
                    GetMACByteDec(&asduData[2], &inputBuf_ESP.buffer[0], 1);
                    menuLevel3 = 5;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Hr (24 Hrs format: in Decimal - hh):");
                    break;

                case 5:
                    /*copy the hour*/
                    GetMACByteDec(&asduData[3], &inputBuf_ESP.buffer[0], 1);
                    menuLevel3 = 6;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Min (in Decimal):");
                    break;

                case 6:
                    /*copy the second*/
                    GetMACByteDec(&asduData[4], &inputBuf_ESP.buffer[0], 1);
                    /*set the system time in the RTC and in the time attribute of
                    time cluster*/
                    App_Set_SystemTime(asduData[0], asduData[1], asduData[2],
                         asduData[3], asduData[4], 0x00);
                    ConsolePutROMString((ROM char * const)"\r\n");
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;
            }
            break;

        case '2'://Price Events
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the IssuerEventId (in Hex) (4 Bytes):");
                    menuLevel3 = 2;
                    break;
                case 2:
                    /*copy the IssuerEventId*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[6]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[4]);
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 3;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the New Price (in Hex) (4 Bytes):");
                    break;

                case 3:
                    /*copy the Price*/
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[6]);
                    asduData[5] = GetMACByte(&inputBuf_ESP.buffer[4]);
                    asduData[6] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[7] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 4;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Year (in Decimal - yy):");
                    break;

                case 4:
                    /*copy the year. The year is specified in 1 byte*/
                    GetMACByteDec(&asduData[8], &inputBuf_ESP.buffer[0], 1);
                    menuLevel3 = 5;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Month (in Decimal - mm):");
                    break;

                case 5:
                    /*copy the month as enetered in the terminal*/
                    GetMACByteDec(&asduData[9], &inputBuf_ESP.buffer[0], 1);
                    menuLevel3 = 6;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - day (in Decimal - dd):");
                    break;

                case 6:
                    /*copy the day*/
                    GetMACByteDec(&asduData[10], &inputBuf_ESP.buffer[0], 1);
                    menuLevel3 = 7;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Hour (24 Hrs format: in Decimal - hh):");
                    break;

                case 7:
                    /*copy the hour*/
                    GetMACByteDec(&asduData[11], &inputBuf_ESP.buffer[0], 1);
                    menuLevel3 = 8;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartTime - Minute (in Decimal):");
                    break;

                case 8:
                    /*copy the second*/
                    GetMACByteDec(&asduData[12], &inputBuf_ESP.buffer[0], 1);
                    asduData[13] = 0x00; /*Assume Seconds as always Zero*/
                    menuLevel3 = 9;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the DurationInMinutes (in Hex) (2 Bytes)");
                    break;

                case 9:
                    /*copy the Duration In Minutes*/
                    asduData[14] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[15] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 10;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Tier Value (from 01 to 06):");
                    break;

                case 10:
                    /*copy the Tier Value*/
                    asduData[16] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 11;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Rate Label Length (Max 11 Characters)(In Hex) (1 Byte):");
                    break;

                case 11:
                    /*copy the Rate Label Length*/
                    asduData[17] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 12;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Rate Label :");
                    break;

                case 12:
                    {
                        BYTE i;
                        elapsedTimeInfo elapsedTime;

                        /*copy the Rate Label*/
                        for (i=0; i<asduData[17]; i++)
                        {
                            asduData[18 + i] = inputBuf_ESP.buffer[i];
                        }

                        memcpy
                        (
                            (BYTE *)&PublishPriceInfo,
                            (BYTE *)&PublishPriceCommandPayloadDefaultValue,
                            sizeof(ZCL_Price_PublishPriceInfo)
                        );

                        memcpy
                        (
                            (BYTE *)&PublishPriceInfo.IssuerEventId,
                            &asduData[0],
                            4
                        );

                        memcpy
                        (
                            (BYTE *)&PublishPriceInfo.Price,
                            &asduData[4],
                            4
                        );

                        elapsedTime.year = asduData[8];
                        elapsedTime.month = asduData[9];
                        elapsedTime.day = asduData[10];
                        elapsedTime.hour = asduData[11];
                        elapsedTime.minute = asduData[12];
                        elapsedTime.second = asduData[13];

                        PublishPriceInfo.StartTime = ZCL_Utility_Time_ConvertYearToSeconds (&elapsedTime);
                        //PublishPriceInfo.StartTime += (DWORD)(asduData[11] * 60 * 60) + (DWORD)(asduData[12] * 60);

                        memcpy
                        (
                            (BYTE *)&PublishPriceInfo.DurationInMinutes,
                            &asduData[14],
                            2
                        );

                        PublishPriceInfo.PriceTrailingDigitAndPriceTier = asduData[16];
                        PublishPriceInfo.NumOfPriceTiersAndRegisterTier &= 0xF0;
                        PublishPriceInfo.NumOfPriceTiersAndRegisterTier |= asduData[16];

                        memcpy
                        (
                            (BYTE *)&PublishPriceInfo.RateLabel[0],
                            &asduData[17],
                            asduData[17] + 1
                        );

                        App_UpdatePriceEvents_ESP();

                        ResetMenu();
                    }
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 3;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the attributeId (in Hex) (2 Bytes):");
                    break;

                case 3:
                    /*copy the AttributeID*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 4;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Attribute Data Type(in Hex) (1 Byte):");
                    break;

                case 4:
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 5;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the attribute Data length(in Hex) (1 Byte):");
                    break;

                case 5:
                    asduData[5] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 6;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the attribute Data :");
                    break;
                case 6:
                    for( i=0x00;i<asduData[5];i++)
                    {
                        asduData[6 + i] = GetMACByte(&inputBuf_ESP.buffer[((asduData[5]-1)*2) -( i * 2 )]);
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
                        ESP_ENDPOINT_ID,//EndPoint
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
                    asduData[9] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[10] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 3;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Destination Address(in Hex)(8 Bytes)):");
                    break;

                case 3:
                    /*Updating Binding Entry  DestAddress*/
                    asduData[12] = GetMACByte(&inputBuf_ESP.buffer[14]);
                    asduData[13] = GetMACByte(&inputBuf_ESP.buffer[12]);
                    asduData[14] = GetMACByte(&inputBuf_ESP.buffer[10]);
                    asduData[15] = GetMACByte(&inputBuf_ESP.buffer[8]);
                    asduData[16] = GetMACByte(&inputBuf_ESP.buffer[6]);
                    asduData[17] = GetMACByte(&inputBuf_ESP.buffer[4]);
                    asduData[18] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[19] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    menuLevel3 = 4;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the DestEndpoint:");
                    break;
               case 4:
               {
                    /*Updating Binding Entry  DestEndpoint*/
                    asduData[20] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
        case '5':
        {
            LCEReqFrameVal.LCECmdPayload[0].startTime =
                (ZCL_Callback_GetCurrentTimeInSeconds() + 60);
            LCEReqFrameVal.LCECmdPayload[0].heatingTempSetPoint = 0x1F4;/* 5 degree C temp */
            LCEReqFrameVal.LCECmdPayload[0].heatingTempOffset = 0x14; /* 2 degree C offset */
        }
        ResetMenu();
        break;

        /* Enable or Disable Joining by other devices */
        case '6':
        {
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\n00=Disable Join, FF=Enable, XX=Timed(sec) : ");
                    menuLevel3 = 2;
                    break;
                case 2:
                    /*Get the Permit Duration entry */
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    
                    addr1[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    addr1[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
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
                    destinationEndPoint = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ResetMenu();
                    break;
                
                default:
                    ResetMenu();
                    break;
            }
        }
        break;
        
        /* CUSTOMER manufactor specific command  */
        case '9':   /*Customer specific cluster command */
        {
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"Destination Address of Request: ");
                    menuLevel3 = 2;
                    break;
                  
                case 2:
                {
                    /* Get the Destination Short Address to send the command */
                    WORD_VAL ZDODstAddr;
                    BYTE     addr1[2];
                    menuLevel3 = 2;
                    
                    addr1[0]        = GetMACByte(&inputBuf_ESP.buffer[2]);
                    addr1[1]        = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
                    ZDODstAddr.Val  =  ( (addr1[1] << 8) | addr1[0] );
                    asduData[0] = 0x05; /* ZCL frame Control */
                    asduData[1] = 0x5f; /* manufacutor specific code 0x105f */
                    asduData[2] = 0x10;
                    asduData[3] = 0x00; /* transaction number */
                    asduData[4] = 0x01; /* Command */
                    
                    SendAPPLRequest( ZDODstAddr, CUSTOMER_CLUSTER_ID1, asduData, 0x05);
                    ResetMenu(); 
                }
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
* Function:        BYTE HandleGenericCommandsMenuOptions
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
 BYTE attribute_count;
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Cluster Id: ");
                    ConsolePutROMString((ROM char * const)"\r\n0000 : Basic 000A : Time");
                    ConsolePutROMString((ROM char * const)"\r\n");                                                           
                    menuLevel3 = 0x03;
                    break;
                    
                case 3:
                    /*copy the Cluster id*/
                    asduData[40] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[41] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the number of attributes to be read :");                    
                    menuLevel3 = 0x04;           
                    break;
                
                case 4:
                    /*Read the total number of attributes*/
                    attribute_count = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                            asduData[count + 2] = GetMACByte(&inputBuf_ESP.buffer[(count*2)+ 2]);
                            asduData[count + 3] = GetMACByte(&inputBuf_ESP.buffer[count*2]);
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
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the detination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
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

        case '4': /* Configure Report Command - configure the meter device to report attribute
                  for current summation deleivered at an interval of 10 seconds*/
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\n\r\n0702 - Metering Cluster");
                    ConsolePutROMString((ROM char * const)"\r\n0000 - Basic Cluster");
                    ConsolePutROMString((ROM char * const)"\r\nEnter one of the above ClusterID:");
                    menuLevel3 = 0x03;
                    break;
                
                case 3:
                    /*copy the ClusterID*/
                    asduData[20] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[21] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\n\r\nFor Metering Cluster:");
                    ConsolePutROMString((ROM char * const)"\r\n0000 - CurrentSummationDelivered:");
                    ConsolePutROMString((ROM char * const)"\r\n0100 - CurrentTier1SummationDelivered:");
                    ConsolePutROMString((ROM char * const)"\r\n\r\nFor Basic Cluster:");
                    ConsolePutROMString((ROM char * const)"\r\n0000 - BasicVersion:");
                    ConsolePutROMString((ROM char * const)"\r\n0001 - ApplicationVersion:");
                    ConsolePutROMString((ROM char * const)"\r\n\r\nEnter one of the above AttributeID based on Cluster:");
                    menuLevel3 = 0x04;
                    break;
                    
                case 4:
                    /*attribute id CurrentSummationDelivered -  to be reported */
                    /*Configuring the Meter Device to Send Report*/
                    asduData[2] = 0x00; // Direction is Send Reports
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[2]); // attribute id LSB - CurrentSummationDelivered
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[0]); // attribute id MSB
                    ConsolePutROMString((ROM char * const)"\r\n\r\nFor Metering Cluster:");
                    ConsolePutROMString((ROM char * const)"\r\n25 (CurrentSummationDelivered & CurrentTier1SummationDelivered)");
                    ConsolePutROMString((ROM char * const)"\r\n\r\nFor Basic Cluster:");
                    ConsolePutROMString((ROM char * const)"\r\n20 (BasicVersion & ApplicationVersion)");
                    ConsolePutROMString((ROM char * const)"\r\n\r\nEnter one of the above AttributeDataType:");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                    /* attribute data type of attribute id - CurrentSummationDelivered*/
                    asduData[5] = GetMACByte(&inputBuf_ESP.buffer[0]); // Attribute Data Type
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Minimum Interval (2 Bytes):");
                    menuLevel3 = 0x06;
                    break;
                case 6:
                    /*minimum reporting interval at which the report attribute
                    command need to be sent*/
                    asduData[6] = GetMACByte(&inputBuf_ESP.buffer[2]); // Minimum Interval - LSB (3 Seconds)
                    asduData[7] = GetMACByte(&inputBuf_ESP.buffer[0]); // Minimum Interval - MSB
                    ConsolePutROMString((ROM char * const)"\r\nEnter valid Maximum Interval value to report or FFFF to Stop already reporting entry");
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Maximum Interval (2 Bytes):");
                    menuLevel3 = 0x07;
                    break;
                case 7:
                    /*maximum reporting interval at which the report attribute
                    command need to be sent*/
                    asduData[8] = GetMACByte(&inputBuf_ESP.buffer[2]); // Maximum Interval - LSB (10 Seconds)
                    asduData[9] = GetMACByte(&inputBuf_ESP.buffer[0]); // Maximum Interval - MSB
                    ConsolePutROMString((ROM char * const)"\r\n\r\nFor Metering Cluster - 6 Byte Value");
                    ConsolePutROMString((ROM char * const)"\r\nFor Basic Cluster - 1 Byte Value");
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Reportable Change:");
                    menuLevel3 = 0x08;
                    break;
                case 8:
                {
                    BYTE i;
                    WORD_VAL localClusterId;
                    BYTE attributeDataLength = ZCL_GetDataLengthForAttributeDataType
                                    ( asduData[5], 0x00 );
                    /*Reportable change*/
                    for (i=0; i<attributeDataLength; i++ )
                    {
                        asduData[10+i] = GetMACByte(&inputBuf_ESP.buffer[2*(attributeDataLength - 1 - i)]);
                    }
                    /*Cluster Id*/                   
                    localClusterId.v[0] = asduData[20];
                    localClusterId.v[1] = asduData[21];
                    
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_GenericCommand,
                        localClusterId.Val,
                        ZCL_Configure_Reporting_Cmd,
                        8+attributeDataLength,
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

        case '5': // Read Report Command - reads the reporting configuration command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /* copy the destination address */
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /* Configuring the Meter Device to Send Report */
                    asduData[2] = 0x00; // Direction is Send Reports
                    /* attribute id  - CurrentSummationDelivered */
                    asduData[3] = 0x00; // attribute id LSB - CurrentSummationDelivered
                    asduData[4] = 0x00; // attribute id MSB
                    /*create read reporting configuration command for
                    CurrentSummationDelivered, 0x03 - gives the length of the
                    read reporting configuration payload*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_GenericCommand,
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
* Function:        BYTE HandleBasicClusterMenuOptions
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
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
* Function:        BYTE HandleIdentifyClusterMenuOptions
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*Identify Time interval is set to 30 Seconds by default*/
                    asduData[2] = 0x1E;
                    asduData[3] = 0x00;
                    
                    ConsolePutROMString((ROM char * const)"\r\nEnter Mode of Transmission (01 for Group, 02 for Unicast):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Mode of Transmission (01 for Group, 02 for Unicast):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
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
* Function:        BYTE HandleCommissioningClusterMenuOptions
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
* Overview:        This enables the user to perform commissioning cluster specific
*                   commands such as restart device, save SAS etc
*
* Note:            None
 *****************************************************************************/
static void HandleCommissioningClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Write Attribute Command is issued to Commission the Device.
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
                    /*extended pan id attribute 0x0001*/
                    asduData[2] = (unsigned char)(ZCL_COMMISSIONING_EXTENDED_PANID); //0x01// Extended PANId Attribute LSB
                    asduData[3] = (unsigned char)(ZCL_COMMISSIONING_EXTENDED_PANID>>8); //0x00// Extended PANId Attribute MSB
                    /*attribute data type of extended attribute id*/
                    asduData[4] = DATA_TYPE_IEEE_ADDRESS;	//0xF0
                    /*attribute data for the attribute*/
                    ConsolePutROMString((ROM char * const)"\r\nEnter the ExtendedPANId in Hex:");
                    asduData[12] = GetHexByte();
                    asduData[11] = GetHexByte();
                    asduData[10] = GetHexByte();
                    asduData[9] = GetHexByte();
                    asduData[8] = GetHexByte();
                    asduData[7] = GetHexByte();
                    asduData[6] = GetHexByte();
                    asduData[5] = GetHexByte();
                    
                    /*start up control attribute id*/
                    asduData[13] = (unsigned char)(ZCL_COMMISSIONING_STARTUP_CONTROL); //0x06// Startup Control Attribute LSB
                    asduData[14] = (unsigned char)(ZCL_COMMISSIONING_STARTUP_CONTROL>>8); //0x00// Startup Control Attribute MSB
                    /*datatype of start up control attribute*/
                    asduData[15] = DATA_TYPE_8_ENUMERATION;	//0x30
                    /*attribute data for start up control - always rejoin*/
                    ConsolePutROMString((ROM char * const)"\r\nEnter the StartUp Control in Hex (01-Form N/W, 02-Join N/W):");
                    asduData[16] = GetHexByte(); //STARTUP_CONTROL_FORM_NEW_NETWORK; //STARTUP_CONTROL_JOIN_NEW_NETWORK; // Always rejoin to specified Extended PANId

                     /*Channel Mask id attribute */
                    asduData[17] = (unsigned char)(ZCL_COMMISSIONING_CHANNEL_MASK); // Channel Mask Attribute LSB
                    asduData[18] = (unsigned char)(ZCL_COMMISSIONING_CHANNEL_MASK>>8); // Channel Mask Attribute MSB
                    /*attribute data type of Channel Mask attribute id*/
                    asduData[19] = DATA_TYPE_32_BITMAP;
                    /*attribute data for the attribute*/
                    asduData[20] = (unsigned char)(ALLOWED_CHANNELS);
                    asduData[21] = (unsigned char)(ALLOWED_CHANNELS>>8);
                    asduData[22] = (unsigned char)(ALLOWED_CHANNELS>>16);
                    asduData[23] = (unsigned char)(ALLOWED_CHANNELS>>24);
                    
                     /*Preconfigured Link Key id attribute */
                    asduData[24] = (unsigned char)(ZCL_COMMISSIONING_PRE_CONFIGURED_LINK_KEY); // Preconfigured Link Key Attribute LSB
                    asduData[25] = (unsigned char)(ZCL_COMMISSIONING_PRE_CONFIGURED_LINK_KEY>>8); // Preconfigured Link Key Attribute MSB
                    /*attribute data type of Preconfigured Link Key attribute id*/
                    asduData[26] = DATA_TYPE_128_SECURITY_KEY;
                    /*attribute data for the attribute*/
                    ConsolePutROMString((ROM char * const)"\r\nEnter the PreConfigured Link Key zeroth byte first:");
                    asduData[42] = 0x5a;
                    asduData[41] = 0x69;
                    asduData[40] = 0x67;
                    asduData[39] = 0x42;
                    asduData[38] = 0x65;
                    asduData[37] = 0x65;
                    asduData[36] = 0x41;
                    asduData[35] = 0x6c;
                    asduData[34] = 0x6c;
                    asduData[33] = 0x69;
                    asduData[32] = 0x61;
                    asduData[31] = 0x6e;
                    asduData[30] = 0x63;
                    asduData[29] = 0x65;
                    asduData[28] = 0x30;
                    asduData[27] = 0x39;
					ConsolePutROMString((ROM char * const)"\r\nEnd of Commissioning Data Entry \r\n");
                    /*create write attribute command for three attributes namely
                    extended PANID attribute, start up control, Trust Center address*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_GenericCommand,
                        ZCL_COMMISSIONING_CLUSTER,
                        ZCL_Write_Attributes_Cmd,
                        41,
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Restart SAS
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*options - restart the device using the parameters set in
                    ZCL commissioning attribute set*/
                    asduData[2] = 0x00;
                    /*the delay in seconds  to be applied before restarting device*/
                    asduData[3] = 0x14; // Delay in seconds (20 seconds)
                    /*the delay in milliseconds to be applied before restarting device*/
                    asduData[4] = 0x00; // Jitter in miliseconds
                    /*create restart device command. The length of the payload for
                    this command is 0x03 bytes*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_COMMISSIONING_CLUSTER,
                        ZCL_CommissioningRestartDevice,
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

        case '3': // Save SAS
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*reserved - all bits are reserved*/
                    asduData[2] = 0x00; // Options
                    /*SAS index to where the comissioned parameters are to be saved*/
                    asduData[3] = 0x01; // SAS index (only 0x01 or 0x02 is acceptable)
                    /*create save sas command. The length of the payload is 0x02.*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_COMMISSIONING_CLUSTER,
                        ZCL_CommissioningSaveStartupParameters,
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

        case '4': // Restore SAS
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*Options - reserved*/
                    asduData[2] = 0x00;
                    /*restore commisioned parameters from SAS index*/
                    asduData[3] = 0x01; // SAS index (only 0x01 or 0x02 is acceptable)
                    /*create Restore SAS command with payload length 0x02*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_COMMISSIONING_CLUSTER,
                        ZCL_CommissioningRestoreStartupParameters,
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

        case '5': // Reset SAS
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\n01: Reset Curt SAS:");
                    ConsolePutROMString((ROM char * const)"\r\n02: Reset All SAS:");
                    ConsolePutROMString((ROM char * const)"\r\n04: Erase Index:");
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Option:");
                    menuLevel3 = 0x03;
                    break;

                case 3:
                    /*copy the option*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*SAS index to be reset when option is erase index(0x04)*/
                    asduData[3] = 0x01; // SAS index (only 0x01 or 0x02 is acceptable)
                    /*create reset startup parameters command*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_COMMISSIONING_CLUSTER,
                        ZCL_CommissioningResetStartupParameters,
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

        default:
            ResetMenu();
        break;
    }
}

/******************************************************************************
* Function:        BYTE HandleMessageClusterMenuOptions
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
* Overview:        This enables the user to perform message cluster specific
*                   commands such as display message, cancel message command.
*
* Note:            None
 *****************************************************************************/
static void HandleMessageClusterMenuOptions(void)
{

    switch (menuLevel2)
    {
        case '1': // Display Message Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Len of the Msg:");
                    menuLevel3 = 0x03;
                    break;

                case 3:
                    /*copy the length of the message*/
                    asduData[13] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Msg:");
                    menuLevel3 = 0x04;
                    break;

                case 4:
                    {
                        BYTE i;
                        /*copy the message*/
                        for( i=0 ; i < asduData[13]; i++ )
                        {
                            asduData[14 + i] =  inputBuf_ESP.buffer[i];
                        }
                        /*fill the display message table contents*/
                        ZCL_Message_DisplayMessageTable.startTime = ZCL_Callback_GetCurrentTimeInSeconds();
                        /*In display message, duration is fixed as 0x0001 minutes*/
                        displayMessagesTable.pDisplayMessage->durationInMinutes = 0x0001;

                        /*copying the destination address*/
                        memcpy
                        (
                            (BYTE*)&( displayMessagesTable.destaddress),
                            asduData,
                            0x02
                         );
                         /*update the endpoint*/
                        displayMessagesTable.destEndPoint = 0x01;
                        /*update the message id*/
                        (displayMessagesTable.pDisplayMessage->messageId)++;
                        /*update the message control to default value with no confirmation required*/
                        displayMessagesTable.pDisplayMessage->messageControl =
                           0x80;

                        /*fetch the length of the message and update the message table*/
                        displayMessagesTable.pDisplayMessage->aDisplayMessage[0] = asduData[13];
                        /*copy the message*/
                        memcpy
                        (
                            &(ZCL_Message_DisplayMessageTable.aDisplayMessage[1]),
                            &(asduData[14]),
                            asduData[13]
                        );
                        /*set the background task for display message to 1*/
                        espStatus.flags.bits.bPendingDisplayMessageCmd = 1;
                        /*initialise the start time for display message command*/
                        espStatus.displayMessageCmdTime =
                                    ZCL_Message_DisplayMessageTable.startTime;
                        /*copy the message id*/
                        memcpy
                        (
                            &asduData[2],
                            (BYTE *)&(displayMessagesTable.pDisplayMessage->messageId),
                            11
                        );
                        /*create display message command and queue it up for AIL*/
                        AppSendData
                        (
                            ZCL_ServerToClient | ZCL_ClusterSpecificCommand,
                            ZCL_MESSAGING_CLUSTER,
                            ZCL_Message_DisplayMessageCmd,
                            12 + asduData[13],
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

        case '2': // Cancel Message Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*Message id*/
                    asduData[2] = 0x11;
                    asduData[3] = 0x11;
                    asduData[4] = 0x11;
                    asduData[5] = 0x11;
                    /*message control*/
                    asduData[6] = 0x80;
                    /*clear the background task while sending cancel message command*/
                    espStatus.flags.bits.bPendingDisplayMessageCmd = 0x00;
                    ZCL_Message_DisplayMessageTable.startTime = 0xFFFFFFFF;
                    /*create cancel message command with length as 5 bytes*/
                    AppSendData
                    (
                        ZCL_ServerToClient | ZCL_ClusterSpecificCommand,
                        ZCL_MESSAGING_CLUSTER,
                        ZCL_Message_CancelMessageCmd,
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

        default:
            ResetMenu();
        break;
    }
}

/******************************************************************************
* Function:        BYTE HandleMeteringClusterMenuOptions
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
        case '1': // Get Profile Request Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*interval for get profile command is fixed as delivered*/
                    asduData[2] = ZCL_MeteringConsumptionDelivered;

                    // EndTime as CurrentTime.
                    asduData[3] = 0x00;
                    asduData[4] = 0x00;
                    asduData[5] = 0x00;
                    asduData[6] = 0x00;

                    /*include the number of periods included as 0x03*/
                    asduData[7] = 0x03; // 3 Periods intervals are requested
                    /*create Get profile command for Metering cluster with
                    length of the payload as 6 bytes*/
                    AppSendData
                    (
                        ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
                        ZCL_METERING_CLUSTER,
                        ZCL_MeteringGetProfile,
                        0x06,
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
* Function:        BYTE HandlePriceClusterMenuOptions
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
*                   commands such as Publish price command
*
* Note:            None
 *****************************************************************************/
static void HandlePriceClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Publish Price Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Tier Info(01 to 06):");
                    menuLevel3 = 0x03;
                    break;

                case 3:
                    {
                        BYTE i;
                        BYTE lenOfRateLabel;
                        BYTE payloadLength;
                        /*copy the tier info*/
                        asduData[2] = GetMACByte(&inputBuf_ESP.buffer[0]);

                        if ( ( asduData[2] > MAX_TIERS_SUPPORTED ) ||
                             ( asduData[2] == 0x00 ) )
                        {
                            ResetMenu();
                            return;
                        }

                        for (i=0; i<MAX_TIERS_SUPPORTED; i++)
                        {
                            if (asduData[2] == ListOfPriceEvents[i].tierInfo)
                            {
                                break;
                            }
                        }

                        payloadLength = 4; // 4 Bytes for ProviderId
                        memcpy
                        (
                            &asduData[2],
                            (BYTE*)&PublishPriceCommandPayloadDefaultValue.ProviderId,
                            0x04
                        );

                        /*Since, this is a octet string, first byte specifies the
                        length of the Rate Label.*/
                        lenOfRateLabel = ListOfPriceEvents[i].rateLabel[0];
                        memcpy
                        (
                            &PublishPriceCommandPayloadDefaultValue.RateLabel[0],
                            &ListOfPriceEvents[i].rateLabel[0],
                            lenOfRateLabel + 1 // 1 is added because of the first byte which specifies the length
                        );
                        memcpy
                        (
                            &asduData[6],
                            &ListOfPriceEvents[i].rateLabel[0],
                            lenOfRateLabel + 1 // 1 is added because of the first byte which specifies the length
                        );

                        /*Since the RateLable is of variable length, we have to
                        calculate the total lenght of the payload dynamically.*/
                        payloadLength = payloadLength + lenOfRateLabel + 1;

                        PublishPriceCommandPayloadDefaultValue.IssuerEventId =
                            ListOfPriceEvents[i].issuerEventId;

                        /*maximum of 6 tiers*/
                        PublishPriceCommandPayloadDefaultValue.NumOfPriceTiersAndRegisterTier = 0x60 |
                            ListOfPriceEvents[i].tierInfo;
                        PublishPriceCommandPayloadDefaultValue.PriceTrailingDigitAndPriceTier &= 0xF0;
                        PublishPriceCommandPayloadDefaultValue.PriceTrailingDigitAndPriceTier |=
                            ListOfPriceEvents[i].tierInfo;
                        PublishPriceCommandPayloadDefaultValue.Price = ListOfPriceEvents[i].price;
                        PublishPriceCommandPayloadDefaultValue.StartTime = ListOfPriceEvents[i].UTCStartTime;
                        //GetStartTime( ListOfPriceEvents[i].tierInfo,
                        //&(PublishPriceCommandPayloadDefaultValue.DurationInMinutes) );
                        PublishPriceCommandPayloadDefaultValue.DurationInMinutes = ListOfPriceEvents[i].durationInMinutes;
                            //( ListOfPriceEvents[i].endTime - ListOfPriceEvents[i].startTime ) / 60;

                        PublishPriceCommandPayloadDefaultValue.CurrentTime = ZCL_Callback_GetCurrentTimeInSeconds();

                        // 35 Bytes for the remaining payload which is fixed
                        payloadLength = payloadLength + PUBLISH_PRICE_CMD_LENGTH_FROM_ISSUER_EVENT_ID;

                        memcpy
                        (
                            &asduData[6 + lenOfRateLabel + 1 ],
                            (BYTE*)&PublishPriceCommandPayloadDefaultValue.IssuerEventId,
                            PUBLISH_PRICE_CMD_LENGTH_FROM_ISSUER_EVENT_ID
                        );
                        /*create publish price command*/
                        AppSendData
                        (
                            ZCL_ServerToClient | ZCL_ClusterSpecificCommand,
                            ZCL_PRICE_CLUSTER,
                            ZCL_Price_PublishPriceCmd,
                            payloadLength,
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
* Function:        void HandleDRLCClusterMenuOptions
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
* Overview:        This enables the user to perform DRLC cluster specific
*                   commands such as Load control event command.
*
* Note:            None
 *****************************************************************************/
static void HandleDRLCClusterMenuOptions(void)
{
    switch (menuLevel2)
    {
        case '1': // Load Control Event Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*copy the default values for load control event command
                    from the table*/
                    LoadControlEventCommandPayloadDefaultValue[0].startTime = 0x00000000;
                    memcpy
                    (
                        &asduData[2],
                        (BYTE*)&LCEReqFrameVal.LCECmdPayload[0],
                        sizeof(DRLC_LoadControlEvent)
                    );
                    /*create load control event command for DRLC cluster*/
                    AppSendData
                    (
                        ZCL_ServerToClient | ZCL_ClusterSpecificCommand,
                        ZCL_DRLC_CLUSTER,
                        ZCL_LoadControlEventCmd,
                        sizeof(DRLC_LoadControlEvent),
                        asduData
                    );
                    ResetMenu();
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Cancel Load Control Event Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*copy the cancel load control event command payload*/
                    memcpy
                    (
                        &asduData[2],
                        (BYTE*)&LCEReqFrameVal.LCECmdPayload[0],
                        7
                    );
                    /*no randomization for start and stop time*/
                    asduData[9] = 0x00; // No Randomization
                    /*set the start time to now*/
                    asduData[10] = 0x00; // effective time - Now
                    asduData[11] = 0x00; // effective time - Now
                    asduData[12] = 0x00; // effective time - Now
                    asduData[13] = 0x00; // effective time - Now
                    /*create cancel load control event command*/
                    AppSendData
                    (
                        ZCL_ServerToClient | ZCL_ClusterSpecificCommand,
                        ZCL_DRLC_CLUSTER,
                        ZCL_CancelLoadControlEventCmd,
                        sizeof(DRLC_CancelLoadControlEvent),
                        asduData
                    );
                    ResetMenu();;
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '3': // Cancel All Load Control Event Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /* No Randomization*/
                    asduData[2] = 0x00;
                    /*create cancel all load control events command*/
                    AppSendData
                    (
                        ZCL_ServerToClient | ZCL_ClusterSpecificCommand,
                        ZCL_DRLC_CLUSTER,
                        ZCL_CancelAllLoadControlEventCmds,
                        sizeof(DRLC_CancelAllLoadControlEvents),
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
        case '1': // Publish Price Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr mode:");
                    ConsolePutROMString((ROM char * const)"\r\n02: Short Addr");
                    ConsolePutROMString((ROM char * const)"\r\n03: Extended Addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the addressing mode*/
                    PublishPriceRequestFrameValue.destAddrMode = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x03;
                    break;

                case 3:
                    /*based on the addressing mode, copy the address*/
                    if ( PublishPriceRequestFrameValue.destAddrMode == APS_ADDRESS_16_BIT )
                    {
                        PublishPriceRequestFrameValue.destAddr[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        PublishPriceRequestFrameValue.destAddr[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    }
                    else
                    {
                        PublishPriceRequestFrameValue.destAddr[0] = GetMACByte(&inputBuf_ESP.buffer[14]);
                        PublishPriceRequestFrameValue.destAddr[1] = GetMACByte(&inputBuf_ESP.buffer[12]);
                        PublishPriceRequestFrameValue.destAddr[2] = GetMACByte(&inputBuf_ESP.buffer[10]);
                        PublishPriceRequestFrameValue.destAddr[3] = GetMACByte(&inputBuf_ESP.buffer[8]);
                        PublishPriceRequestFrameValue.destAddr[4] = GetMACByte(&inputBuf_ESP.buffer[6]);
                        PublishPriceRequestFrameValue.destAddr[5] = GetMACByte(&inputBuf_ESP.buffer[4]);
                        PublishPriceRequestFrameValue.destAddr[6] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        PublishPriceRequestFrameValue.destAddr[7] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    }
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest PAN Id (2 Bytes):");
                    menuLevel3 = 0x04;
                    break;

                case 4:
                    {
                        PAN_ADDR dstPANId;
                        /*copy the destination pan id*/
                        dstPANId.v[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        dstPANId.v[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                        PublishPriceRequestFrameValue.destInterPANId = dstPANId.Val;
                        ConsolePutROMString((ROM char * const)"\r\nEnter the Tier Info(01 to 06):");
                        menuLevel3 = 0x05;
                    }
                    break;

                case 5:
                    {
                        BYTE i;
                        BYTE lenOfRateLabel;

                        asduData[0] = GetMACByte(&inputBuf_ESP.buffer[0]);

                        if ( asduData[0] > 0x06 || asduData[0] <= 0x00 )
                        {
                            ResetMenu();
                            return;
                        }

                        for (i=0; i<MAX_TIERS_SUPPORTED; i++)
                        {
                            if (asduData[0] == ListOfPriceEvents[i].tierInfo)
                            {
                                break;
                            }
                        }

                        /*Since, this is a octet string, first byte specifies the
                        length of the Rate Label.*/
                        lenOfRateLabel = ListOfPriceEvents[i].rateLabel[0];
                        memcpy
                        (
                            &PublishPriceCommandPayloadDefaultValue.RateLabel[0],
                            &ListOfPriceEvents[i].rateLabel[0],
                            lenOfRateLabel + 1 // 1 is added because of the first byte which specifies the length
                        );

                        PublishPriceCommandPayloadDefaultValue.IssuerEventId =
                            ListOfPriceEvents[i].issuerEventId;

                        /*maximum of 6 tiers*/
                        PublishPriceCommandPayloadDefaultValue.NumOfPriceTiersAndRegisterTier = 0x60 |
                            ListOfPriceEvents[i].tierInfo;
                        PublishPriceCommandPayloadDefaultValue.PriceTrailingDigitAndPriceTier &= 0xF0;
                        PublishPriceCommandPayloadDefaultValue.PriceTrailingDigitAndPriceTier |=
                            ListOfPriceEvents[i].tierInfo;
                        PublishPriceCommandPayloadDefaultValue.Price = ListOfPriceEvents[i].price;
                        PublishPriceCommandPayloadDefaultValue.StartTime = ListOfPriceEvents[i].UTCStartTime;
                        //GetStartTime( ListOfPriceEvents[i].tierInfo,
                        //&(PublishPriceCommandPayloadDefaultValue.DurationInMinutes) );
                        PublishPriceCommandPayloadDefaultValue.DurationInMinutes = ListOfPriceEvents[i].durationInMinutes;
                            //( ListOfPriceEvents[i].endTime - ListOfPriceEvents[i].startTime ) / 60;
                        /*send publish price command through inter pan*/
                        App_SendPublishPriceCommandOnInterPANNetwork();

                        ResetMenu();
                    }
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '2': // Display Message
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr mode:");
                    ConsolePutROMString((ROM char * const)"\r\n02: Short Addr");
                    ConsolePutROMString((ROM char * const)"\r\n03: Extended Addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the addressing mode*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x03;
                    break;

                case 3:
                    /*based on the addressing mode, copy the address*/
                    if ( asduData[0] == APS_ADDRESS_16_BIT )
                    {
                        asduData[1] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        asduData[2] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    }
                    else
                    {
                        asduData[1] = GetMACByte(&inputBuf_ESP.buffer[14]);
                        asduData[2] = GetMACByte(&inputBuf_ESP.buffer[12]);
                        asduData[3] = GetMACByte(&inputBuf_ESP.buffer[10]);
                        asduData[4] = GetMACByte(&inputBuf_ESP.buffer[8]);
                        asduData[5] = GetMACByte(&inputBuf_ESP.buffer[6]);
                        asduData[6] = GetMACByte(&inputBuf_ESP.buffer[4]);
                        asduData[7] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        asduData[8] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    }
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest PAN Id (2 Bytes):");
                    menuLevel3 = 0x04;
                    break;

                case 4:
                    /*copy the destination pan id*/
                    asduData[9] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[10] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Len of the Msg (Max 14 Char):");
                    menuLevel3 = 0x05;
                    break;

                case 5:
                    /*copy the length of the message*/
                    asduData[11] = GetMACByte(&inputBuf_ESP.buffer[0]);

                    if ( ( asduData[11] <= 0 ) ||  ( asduData[11] > 14  ) )
                    {
                        ConsolePutROMString((ROM char * const)"\r\nInvalid Msg Length");
                        ResetMenu();
                        return;
                    }

                    ConsolePutROMString((ROM char * const)"\r\nEnter the Msg:");
                    menuLevel3 = 0x06;
                    break;

                case 6:
                    {
                        BYTE i;
                        BYTE *pData;

                        /*copy the message*/
                        for( i=0 ; i < asduData[11]; i++ )
                        {
                            asduData[12 + i] =  inputBuf_ESP.buffer[i];
                        }

                        interPANDataRequest.destAddrMode = asduData[0];
                        if ( interPANDataRequest.destAddrMode == APS_ADDRESS_64_BIT )
                        {
                            interPANDataRequest.asduLength = MAX_ADDR_LENGTH;
                        }
                        else
                        {
                            interPANDataRequest.asduLength = SHORT_ADDRESS_LENGTH;
                        }
                        memcpy
                        (
                            ( BYTE *)&(interPANDataRequest.destAddr[0]),
                            ( BYTE *)&(asduData[1]),
                            interPANDataRequest.asduLength
                        );
                        memcpy
                        (
                            ( BYTE *)&(interPANDataRequest.destInterPANId),
                            ( BYTE *)&(asduData[9]),
                            2 /*DestinationPANId is always 2 Bytes*/
                        );

                        interPANDataRequest.profileId = SE_PROFILE_ID;
                        interPANDataRequest.clusterID = ZCL_MESSAGING_CLUSTER;

                        /*Allocate memory for Display Message Command Payload + the ZCL Header (3 Bytes)*/
                        pData =  (BYTE *)SRAMalloc(sizeof(ZCL_Message_DisplayMessageCmdPayload) + 0x03);
                        if( pData == NULL )
                        {
                            return;
                        }

                        interPANDataRequest.pAsdu = pData;

                        /*fill the display message table contents*/
                        ZCL_Message_DisplayMessageTable.startTime = ZCL_Callback_GetCurrentTimeInSeconds();
                        /*In display message, duration is fixed as 0x0005 minutes*/
                        ZCL_Message_DisplayMessageTable.durationInMinutes = 0x0005;
                        /*fill the MessageId*/
                        /*For this Sample Application, MessageId is fixed to 0x11111111*/
                        ZCL_Message_DisplayMessageTable.messageId = 0x11111111;
                        /*update the message control to enable Anonymous Inter-PAN Transmission*/
                        ZCL_Message_DisplayMessageTable.messageControl = ANONYMOUS_INTERPAN_ONLY;
                        /*update the message id*/
                        /*fetch the length of the message and update the message table*/
                        ZCL_Message_DisplayMessageTable.aDisplayMessage[0] = asduData[11];

                        /*copy the message*/
                        memcpy
                        (
                            &(ZCL_Message_DisplayMessageTable.aDisplayMessage[1]),
                            &(asduData[12]),
                            asduData[11]
                        );

                        /*Note: asduLength is reused here. Earlier this field was used for copying the
                        destination address.*/
                        interPANDataRequest.asduLength = ZCL_CreateDisplayMessageCommand
                                                         (
                                                             interPANDataRequest.pAsdu,
                                                             &ZCL_Message_DisplayMessageTable ,
                                                             appNextSeqNum
                                                         );

                        /*ASDUHandle is a unique number. This should be given by the application. Since,
                        TransactionSeqNum is also unique and changes when every packet gets transmitted,
                        the same value is used as the ASDUHandle*/
                        interPANDataRequest.asduHandle = appNextSeqNum;

                        /*Increment the TransactionSequenceNumber*/
                        appNextSeqNum++;

                        App_SendDataOverInterPAN();

                        /*set the background task for display message to 1*/
                        espStatus.flags.bits.bPendingDisplayMessageCmd = 1;
                        /*initialise the start time for display message command*/
                        espStatus.displayMessageCmdTime = ZCL_Message_DisplayMessageTable.startTime;

                        ResetMenu();
                    }
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;

        case '3': // Cancel Message Command
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr mode:");
                    ConsolePutROMString((ROM char * const)"\r\n02: Short Addr");
                    ConsolePutROMString((ROM char * const)"\r\n03: Extended Addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the addressing mode*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x03;
                    break;

                case 3:
                    /*based on the addressing mode, copy the address*/
                    if ( asduData[0] == APS_ADDRESS_16_BIT )
                    {
                        asduData[1] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        asduData[2] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    }
                    else
                    {
                        asduData[1] = GetMACByte(&inputBuf_ESP.buffer[14]);
                        asduData[2] = GetMACByte(&inputBuf_ESP.buffer[12]);
                        asduData[3] = GetMACByte(&inputBuf_ESP.buffer[10]);
                        asduData[4] = GetMACByte(&inputBuf_ESP.buffer[8]);
                        asduData[5] = GetMACByte(&inputBuf_ESP.buffer[6]);
                        asduData[6] = GetMACByte(&inputBuf_ESP.buffer[4]);
                        asduData[7] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        asduData[8] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    }
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest PAN Id (2 Bytes):");
                    menuLevel3 = 0x04;
                    break;

                case 4:
                    {
                        BYTE *pData;

                        /*copy the destination pan id*/
                        asduData[9] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        asduData[10] = GetMACByte(&inputBuf_ESP.buffer[0]);

                        interPANDataRequest.destAddrMode = asduData[0];
                        if ( interPANDataRequest.destAddrMode == APS_ADDRESS_64_BIT )
                        {
                            interPANDataRequest.asduLength = MAX_ADDR_LENGTH;
                        }
                        else
                        {
                            interPANDataRequest.asduLength = SHORT_ADDRESS_LENGTH;
                        }
                        memcpy
                        (
                            ( BYTE *)&(interPANDataRequest.destAddr[0]),
                            ( BYTE *)&(asduData[1]),
                            interPANDataRequest.asduLength
                        );
                        memcpy
                        (
                            ( BYTE *)&(interPANDataRequest.destInterPANId),
                            ( BYTE *)&(asduData[9]),
                            2 /*DestinationPANId is always 2 Bytes*/
                        );

                        interPANDataRequest.profileId = SE_PROFILE_ID;
                        interPANDataRequest.clusterID = ZCL_MESSAGING_CLUSTER;

                        /*Allocate memory for Cancel Message Command Payload + the ZCL Header (3 Bytes)*/
                        pData =  (BYTE *)SRAMalloc(sizeof(ZCL_Message_CancelMessageCmdPayload) + 0x03);
                        if( pData == NULL )
                        {
                            return;
                        }

                        interPANDataRequest.pAsdu = pData;

                        /*Create the ZCL Header.*/
                        /*Cluster specific command, Manufacturer specific bit is zero,
                        direction is Server to Client */
                        *pData++ = ZCL_ClusterSpecificCommand | ZCL_ServerToClient; /*ZCL FrameControl Field*/
                        *pData++ = appNextSeqNum; /*TransactionSequenceNumber Field*/
                        *pData++ = ZCL_Message_CancelMessageCmd; /*CommandId*/

                        /*Create the ZCL Payload*/
                        /*Update the MessageId Field*/
                        /*For this Sample Application, MessageId is fixed to 0x11111111*/
                        *pData++ = 0x11;
                        *pData++ = 0x11;
                        *pData++ = 0x11;
                        *pData++ = 0x11;

                        /*Update the MessageControl Field to enable Anonymous Inter-PAN Transmission*/
                        *pData++ = ANONYMOUS_INTERPAN_ONLY;

                        /*Note: asduLength is reused here. Earlier this field was used for copying the
                        destination address.*/
                        interPANDataRequest.asduLength = 0x08; /*3 Bytes of ZCL Header and 5 Bytes of ZCL Payload*/

                        /*ASDUHandle is a unique number. This should be given by the application. Since,
                        TransactionSeqNum is also unique and changes when every packet gets transmitted,
                        the same value is used as the ASDUHandle*/
                        interPANDataRequest.asduHandle = appNextSeqNum;

                        /*Increment the TransactionSequenceNumber*/
                        appNextSeqNum++;

                        App_SendDataOverInterPAN();

                        /*Reset the StartTime. This is required to avoid sending
                        the Display Message command when Get Last Message command
                        is received.*/
                        ZCL_Message_DisplayMessageTable.startTime = 0xFFFFFFFF;

                        /*clear the background task while sending cancel message command*/
                        espStatus.flags.bits.bPendingDisplayMessageCmd = 0x00;

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
                    ConsolePutROMString((ROM char * const)"\r\nTier1(01) - Tier6(06)");
                    ConsolePutROMString((ROM char * const)"\r\nSelect any one of the option:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    {
                        BYTE i;

                        /*copy the Tier Info*/
                        asduData[0] = GetMACByte(&inputBuf_ESP.buffer[0]);

                        /*Loop through the Price Events*/
                        for (i=0; i<MAX_TIERS_SUPPORTED; i++)
                        {
                            /*Display on Hyper Terminal*/

                            /*Check whether specific tier information is required*/
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
                    ConsolePutROMString((ROM char * const)"\r\nEnter the IEEE addr of partner:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    {
                        BYTE i;
                        TC_LINK_KEY_TABLE   tcLinkKeyTable;
                        LONG_ADDR aIEEEAddress;
                        /*copy the destination address*/
                        asduData[0] = GetMACByte(&inputBuf_ESP.buffer[14]);
                        asduData[1] = GetMACByte(&inputBuf_ESP.buffer[12]);
                        asduData[2] = GetMACByte(&inputBuf_ESP.buffer[10]);
                        asduData[3] = GetMACByte(&inputBuf_ESP.buffer[8]);
                        asduData[4] = GetMACByte(&inputBuf_ESP.buffer[6]);
                        asduData[5] = GetMACByte(&inputBuf_ESP.buffer[4]);
                        asduData[6] = GetMACByte(&inputBuf_ESP.buffer[2]);
                        asduData[7] = GetMACByte(&inputBuf_ESP.buffer[0]);

                        memcpy( aIEEEAddress.v,asduData,0x08 );
                        if( SearchForTCLinkKey( aIEEEAddress, &tcLinkKeyTable))
                        {
                            ConsolePutROMString((ROM char * const)"\r\nApp Link Key is: ");
                            for( i = 0x00; i < 16; i++ )
                            {
                                PrintChar( tcLinkKeyTable.LinkKey.v[i]);
                            }
                        }
                        else
                        {
                            ConsolePutROMString((ROM char * const)"\r\n No App Link Key");
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
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;
                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    ConsolePutROMString((ROM char * const)"\r\nEnter the dest addr:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /*copy the destination address*/
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group addr:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Length of the Name String (1 Byte in Hex):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                    /*copy the Length of the Name String*/
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Name String (Max 16 Characters):");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                {
                    BYTE i;
                    /*copy the Name String*/
                    for( i=0 ; i < asduData[4]; i++ )
                    {
                        asduData[5 + i] =  inputBuf_ESP.buffer[i];
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group addr:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group Count (1 Byte):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group Count*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                        //asduData[3 + i] =  inputBuf_ESP.buffer[i];
                        asduData[3 + i + 1] = GetMACByte(&inputBuf_ESP.buffer[j]);
                        asduData[3 + i] = GetMACByte(&inputBuf_ESP.buffer[j+2]);
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group addr:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group addr:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Length of the Name String (1 Byte in Hex):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                    /*copy the Length of the Name String*/
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Name String (Max 16 Characters):");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                {
                    BYTE i;
                    /*copy the Name String*/
                    for( i=0 ; i < asduData[4]; i++ )
                    {
                        asduData[5 + i] =  inputBuf_ESP.buffer[i];
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group ID (2 Byte):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Scene ID (1 Byte):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                    /*copy the Scene ID*/
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Transition Time (2 Bytes):");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                    /*copy the Transition Time*/
                    asduData[5] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[6] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Length of the Scene Name (1 to 16):");
                    menuLevel3 = 0x06;
                    break;
                case 6:
                    /*copy the Length of the Scene Name*/
                    asduData[7] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Name String (Max 16 Characters):");
                    menuLevel3 = 0x07;
                    break;
                case 7:
                {
                    BYTE i;
                    /*copy the Scene Name*/
                    for( i=0 ; i < asduData[7]; i++ )
                    {
                        asduData[8 + i] =  inputBuf_ESP.buffer[i];
                    }
                    extensionFieldIndex = 8 + asduData[7];
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Number of Extension Fields required:");
                    menuLevel3 = 0x08;
                }    
                    break;

                case 8:
                    /*copy the Number of Extension Fields*/
                    NumberOfExtensionFields = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                    asduData[extensionFieldIndex++] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[extensionFieldIndex++] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    totalExtensionFieldLength = totalExtensionFieldLength + 2;
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Length:");
                    menuLevel3 = 10;
                    break;
                    
                case 10:
                    /*copy the Length of the Extension field*/
                    asduData[extensionFieldIndex++] = GetMACByte(&inputBuf_ESP.buffer[0]);
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
                        asduData[extensionFieldIndex++] =  inputBuf_ESP.buffer[i];
                    }*/
                    for( i=0 ; i < (extensionFieldLen*2); i=i+2 )
                    {
                        asduData[extensionFieldIndex++] =  GetMACByte(&inputBuf_ESP.buffer[i]);
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group ID(2 Bytes):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Scene ID(1 Byte):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                    /*copy the Scene ID*/
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Mode of Transmission (01 for Group, 02 for Unicast):");
                    menuLevel3 = 0x05;
                    break;
                case 5:
                {
                    BYTE    cmdID = 0x00;
                    
                    /*copy the Mode of Transmission*/
                    asduData[5] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Group ID(2 Bytes):");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Group address*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter Mode of Transmission (01 for Group, 02 for Unicast):");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                {
                    BYTE    cmdID = 0x00;
                    
                    /*copy the Mode of Transmission*/
                    asduData[4] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Attribute ID");
                    ConsolePutROMString((ROM char * const)"\r\n0011: OccupiedCoolingSetPoint");
                    ConsolePutROMString((ROM char * const)"\r\n0012: OccupiedHeatingSetPoint");
                    ConsolePutROMString((ROM char * const)"\r\n001C: SystemMode");
                    ConsolePutROMString((ROM char * const)"\r\nSelect any of the above or any of your choice:");
                    menuLevel3 = 0x03;
                    break;
                case 3:
                    /*copy the Attribute ID*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Cluster ID");
                    ConsolePutROMString((ROM char * const)"\r\n0201 - for Thermostat Cluster");
                    ConsolePutROMString((ROM char * const)"\r\nSelect the above or any of your choice:");
                    menuLevel3 = 0x04;
                    break;
                case 4:
                {
                    WORD_VAL clusterId;
                    
                    /*copy the Cluster ID*/
                    clusterId.v[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    clusterId.v[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
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
                    asduData[0] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1] = GetMACByte(&inputBuf_ESP.buffer[0]);
                   
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
 * Function:        void HandleLinkStatusTesting( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Prints the Linkstatusmenu options for testing the feature 
 *
 ***************************************************************************************/
#if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
WORD Addr;
BYTE table_index;
static void HandleLinkStatusTesting( void )
{
    switch (menuLevel2)
    {
        case '1':
        {
            switch (menuLevel3)
            {
                case 1: 
                {
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Short Addr of the Neighbor:");
                    menuLevel3 = 0x02;
                    break;
                }
                
                case 2:
                {    
                    /*copy the neighbor address*/
                    Addr = ( 0x00FF & (WORD)GetMACByte(&inputBuf_ESP.buffer[2]) );
                    Addr |= ( (WORD)GetMACByte(&inputBuf_ESP.buffer[0]) << 8 );
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Incoming Cost to be updated: ");
                    menuLevel3 = 0x03;
                    break;
                }    
                    
                case 3: 
                {
                    for( table_index = 0 ; table_index < MAX_NEIGHBORS ; table_index++ )
                    {
                        if( RAMNeighborTable[table_index].shortAddr.Val == Addr )
                        break;
                    }
                    RAMNeighborTable[table_index].linkStatusInfo.flags.bits.bIncomingCost = GetMACByte(&inputBuf_ESP.buffer[0]);
                    ConsolePutROMString((ROM char * const)"\r\nEnter the Outgoing Cost to be updated: ");
                    menuLevel3 = 0x04;
                    break;
                }    
                    
                case 4:
                {
                    RAMNeighborTable[table_index].linkStatusInfo.flags.bits.bOutgoingCost = GetMACByte(&inputBuf_ESP.buffer[0]); 
                    ResetMenu();
                    break;
                }    
                    
                default:
                    ResetMenu();
                    break;
            }    
            break;
        }
        default:
            ResetMenu();
            break;
    }
}                
#endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )

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
void PrintMenu_ESP( void )
{
    ConsolePutROMString( (ROM char *)"\r\n" );
    if( NOW_I_AM_A_CORDINATOR() ){//#if (I_AM_ESP == 1)
        ConsolePutROMString( (ROM char *)"\r\nShort Address of ESP:" );
    }//#endif
    #if (I_AM_IPD == 1)
        ConsolePutROMString( (ROM char *)"\r\nShort Address of IPD:" );
    #endif
    if( NOW_I_AM_A_ROUTER() ){//#if (I_AM_METER == 1)
        ConsolePutROMString( (ROM char *)"\r\nShort Address of Meter:" );
    }//#endif
    #if (I_AM_LCD == 1)
        ConsolePutROMString( (ROM char *)"\r\nShort Address of LCD:" );
    #endif
    #if (I_AM_PCT == 1)
        ConsolePutROMString( (ROM char *)"\r\nShort Address of PCT:" );
    #endif
    #if (I_AM_SA == 1)
        ConsolePutROMString( (ROM char *)"\r\nShort Address of Smart Appliance:" );
    #endif
    #if (I_AM_RED == 1)
        ConsolePutROMString( (ROM char *)"\r\nShort Address of Range Extender:" );
    #endif
    PrintChar( macPIB.macShortAddress.byte.MSB );
    PrintChar( macPIB.macShortAddress.byte.LSB );
    ConsolePutROMString(menu_ESP);
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
    PrintMenu_ESP();
}

extern struct __EndDeviceAnounceTable	EndDeviceAnounceTable;
extern struct __EndDeviceAnounceTable	PerMissionEndDeviceAnounceTable;


void HandleMettringClusterMenuOptions(void)
{

		WORD_VAL ZDODstAddr;
						int Seq,Cmd;

	switch (menuLevel2)
    {
        case '1': // Read Attribute Command
		{
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /* Get the Destination Short Address to send the command */
                    
                    menuLevel3 = 2;
                    
                    asduData[0]        = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[1]        = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
                    
                    menuLevel3 = 0x03;
                    ConsolePutROMString((ROM char * const)"\r\nSEQ_METER_INFO_CMD_SERIAL_NUMBER						0000");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_METER_INFO_CMD_MAKE_CODE							0001");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_COMU_ACTIVE_ENERGY_CMD_COMU_ACTIVE_ENERGY		0100");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_COMU_ACTIVE_ENERGY_CMD_COMU_ACTIVE_ENERGY_M1		0101");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_COMU_ACTIVE_ENERGY_CMD_COMU_ACTIVE_ENERGY_M2		0102");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_COMU_ACTIVE_ENERGY_CMD_COMU_ACTIVE_ENERGY_M3		0103");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_COMU_ACTIVE_ENERGY_CMD_COMU_ACTIVE_ENERGY_M4		0104");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_COMU_ACTIVE_ENERGY_CMD_COMU_ACTIVE_ENERGY_M5		0105");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_COMU_ACTIVE_ENERGY_CMD_COMU_ACTIVE_ENERGY_M6		0106");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_CUR					0200");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_CUR_TIME				0201");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M1						0202");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M1_TIME				0203");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M2						0204");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M2_TIME				0205");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M3						0206");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M3_TIME				0207");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M4						0208");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M4_TIME				0209");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M5						020a");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M5_TIME				020b");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M6						020c");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_MAX_DEMAND_CMD_MAX_DEMAND_M6_TIME				020d");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_1				0300");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_1				0310");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_2				0301");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_2				0311");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_3				0302");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_3				0312");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_4				0303");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_4				0313");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_5				0304");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_5				0314");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_6				0305");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_6				0315");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_7				0306");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_7				0316");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_8				0307");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_8				0317");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_9				0308");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_9				0318");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_ST_10				0309");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_RP_RN_SP_10				0319");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_1					0320");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_1					0330");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_2					0321");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_2					0331");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_3					0322");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_3					0332");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_4					0323");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_4					0333");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_5					0324");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_5					0334");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_6					0325");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_6					0335");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_7					0326");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_7					0336");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_8					0327");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_8					0337");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_9					0328");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_9					0338");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_ST_10					0329");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_LE_SP_10					0339");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_1					0340");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_1					0350");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_2					0341");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_2					0351");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_3					0342");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_3					0352");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_4					0343");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_4					0353");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_5					0344");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_5					0354");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_6					0345");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_6					0355");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_7					0346");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_7					0356");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_8					0347");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_8					0357");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_9					0348");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_9					0358");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_ST_10					0349");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_ND_SP_10					0359");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_1					0360");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_1					0370");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_2					0361");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_2					0371");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_3					0362");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_3					0372");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_4					0363");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_4					0373");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_5					0364");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_5					0374");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_6					0365");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_6					0375");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_7					0366");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_7					0376");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_8					0367");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_8					0377");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_9					0368");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_9					0378");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_ST_10					0369");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_MT_SP_10					0379");
					ConsolePutROMString((ROM char * const)"\r\nSEQ_TAMPER_INFO_CMD_TAMPER_CO						0380");
					ConsolePutROMString((ROM char * const)"\r\nChose Any ONe Of Above:");
                    menuLevel3 = 0x03;
                    break;
                case 3:

					

                    /*copy the attribute to be read as in the terminal*/
                    asduData[2] = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    /*create read attribute command for basic cluster and*/

					ZDODstAddr.Val  =  ( (asduData[1] << 8) | asduData[0] );

					Seq = asduData[3];
					Cmd = asduData[2];
	
                   	asduData[0] = 0x00; /* ZCL frame Control */
                    asduData[1] = 0x01; /* manufacutor specific code 0x105f */
                    asduData[2] = 0x00;
                    asduData[3] = Seq; /* transaction number */
                    asduData[4] = Cmd; /* Command */
                    
                    SendAPPLRequest( ZDODstAddr, CUSTOMER_CLUSTER_ID1, asduData, 0x05);
                    ResetMenu(); 
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
		}
		case '2':
		{
			switch( menuLevel3 )
            {
				static unsigned char AddressofDevice = 0;
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address:");
					menuLevel3 = 2;
					break;
				case 2:
				case 3:
                    AddressofDevice        = GetMACByte(&inputBuf_ESP.buffer[0]);
				
					asduData[0]        = EndDeviceAnounceTable.EndDevAddr[AddressofDevice].shortaddress.v[0];
                    asduData[1]        = EndDeviceAnounceTable.EndDevAddr[AddressofDevice].shortaddress.v[1];
					/*copy the attribute to be read as in the terminal*/
                    asduData[2] = 0;
                    asduData[3] = 0;
                    /*create read attribute command for basic cluster and*/

					ZDODstAddr.Val  =  ( (asduData[1] << 8) | asduData[0] );
					xprintf("\n\r Device Number =");
					PrintChar(AddressofDevice);

					xprintf("\n\r Data asking Short Address =");
					PrintChar(ZDODstAddr.v[1]);
					PrintChar(ZDODstAddr.v[0]);

					Seq = asduData[3];
					Cmd = asduData[2];
	
                   	asduData[0] = 0x00; /* ZCL frame Control */
                    asduData[1] = 0x01; /* manufacutor specific code 0x105f */
                    asduData[2] = 0x00;
                    asduData[3] = Seq; /* transaction number */
                    asduData[4] = Cmd; /* Command */
                    
                    SendAPPLRequest( ZDODstAddr, CUSTOMER_CLUSTER_ID1, asduData, 0x05);
                    ResetMenu(); 
                    break;
                default:
                    ResetMenu();
                    break;
			}


		}
		break;
	}


}


void HandleNetworkInformation(void)
{
	unsigned int Tp;
	xprintf("\n\r Number Of Device Found :- %u \n\r",EndDeviceAnounceTable.Counter);
	if(EndDeviceAnounceTable.Counter>0)
	{
		for(Tp=0;Tp<EndDeviceAnounceTable.Counter;Tp++)
		{
			int Tp2;
			
			xprintf("\n\r Device Number= %d\n\r",Tp+1);
			xprintf("\n\r Short Address =");
			PrintChar(EndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[1]);
			PrintChar(EndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[0]);
			
			xprintf("\n\r Long Address =");
			
			for(Tp2=7;Tp2>=0;Tp2--)
			{
				PrintChar(EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[Tp2]);
			}
			printf("\n\r");
		}	
	}
	else
	{
		xprintf("\n\r No Device Connected \n\r");
	}


	if(PerMissionEndDeviceAnounceTable.Counter>0)
	{
		unsigned int NumberDevice = 0;
		xprintf("\n\r Device Without End Device Declered---- \n\r");

		for(Tp=0;Tp<PerMissionEndDeviceAnounceTable.Counter;Tp++)
		{
			int Tp2;
			int Tp3;
			BOOL FoundDevice = FALSE;
			for(Tp3=0;Tp3<EndDeviceAnounceTable.Counter;Tp3++)
			{

				if( EndDeviceAnounceTable.EndDevAddr[Tp3].longaddress.v[7] == PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[7] &&
	                EndDeviceAnounceTable.EndDevAddr[Tp3].longaddress.v[6] == PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[6] &&
	                EndDeviceAnounceTable.EndDevAddr[Tp3].longaddress.v[5] == PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[5] &&
	                EndDeviceAnounceTable.EndDevAddr[Tp3].longaddress.v[4] == PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[4] &&
	                EndDeviceAnounceTable.EndDevAddr[Tp3].longaddress.v[3] == PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[3] &&
	                EndDeviceAnounceTable.EndDevAddr[Tp3].longaddress.v[2] == PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[2] &&
	                EndDeviceAnounceTable.EndDevAddr[Tp3].longaddress.v[1] == PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[1] &&
	                EndDeviceAnounceTable.EndDevAddr[Tp3].longaddress.v[0] == PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[0])
	                {
	                    FoundDevice = TRUE;
						break;
	                }
			}
			
			if(FoundDevice == FALSE)	
			{
				xprintf("\n\r Device Number= %d\n\r",NumberDevice+1);
				xprintf("\n\r Short Address =");
				PrintChar(PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[1]);
				PrintChar(PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[0]);
				
				xprintf("\n\r Long Address =");
				
				for(Tp2=7;Tp2>=0;Tp2--)
				{
					PrintChar(PerMissionEndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[Tp2]);
				}
				printf("\n\r");
				NumberDevice++;
			}
		}
	}
	else
	{
		xprintf("\n\r No Device Permissted \n\r");
	}

	ResetMenu();
}

extern unsigned char RoLongFlage;
extern LONG_ADDR	RoLoAddr;

void HandleDiscoverRoute(void)
{

WORD_VAL ZDODstAddr;
int Seq,Cmd;
LONG_ADDR IeeeAddr;
	switch (menuLevel2)
    {
        case '1': // Read Attribute Command
		{
            switch( menuLevel3 )
            {
                case 1:
                    ConsolePutROMString((ROM char * const)"\r\nEnter the destination address last 3 bytes byte:");
                    menuLevel3 = 0x02;
                    break;

                case 2:
                    /* Get the Destination Short Address to send the command */
                    
                    menuLevel3 = 2;
                    
                    
                    asduData[5]        = GetMACByte(&inputBuf_ESP.buffer[4]);
					asduData[6]         = GetMACByte(&inputBuf_ESP.buffer[2]);
                    asduData[7]       = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
                    
                    menuLevel3 = 0x03;
					menuLevel3 = 0x03;
					
					
					
					ConsolePutROMString((ROM char * const)"\r\nEnter the layer type 1 TRUE 0 FALSE:");
                    break;
                case 3:

					

                    
                    asduData[3] = GetMACByte(&inputBuf_ESP.buffer[0]);
                    
					IeeeAddr.v[7] = 0x3c;
					IeeeAddr.v[6] = 0xC1;
					IeeeAddr.v[5] = 0xF6;
					IeeeAddr.v[4] = 0x04;
					IeeeAddr.v[3] = 0x00;
					IeeeAddr.v[2] = asduData[7] ;
					IeeeAddr.v[1] = asduData[6]; 
					IeeeAddr.v[0] = asduData[5]; 
					
					Seq = asduData[3];
					
					unsigned char Tp;
					xprintf("\n\rIeee Addr = ");
					for(Tp=0;Tp<8;Tp++)
					{
						PrintChar(IeeeAddr.v[Tp]);
					}
					xprintf("\n\r");
					RoLongFlage = 1;
                    MyAskForDeviceAddress(FALSE,IeeeAddr);
                    ResetMenu(); 
                    break;

                default:
                    ResetMenu();
                    break;

            }
            break;
		}
	}

}






