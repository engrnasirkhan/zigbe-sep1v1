// *****************************************************************************
//
//  Software License Agreement
//
//  Copyright (c) 2004-2007 Microchip Technology Inc.
//  All rights reserved.
//
//  Microchip licenses to you the right to use, copy and distribute Software
//  only when embedded on a Microchip microcontroller or digital signal
//  controller and used with a Microchip radio frequency transceiver, which
//  are integrated into your product or third party product (pursuant to the
//  sublicense terms in the accompanying license agreement).  You may NOT
//  modify or create derivative works of the Software.
//
//  If you intend to use this Software in the development of a product for
//  sale, you must be a member of the ZigBee Alliance.  For more
//  information, go to www.zigbee.org.
//
//  You should refer to the license agreement accompanying this Software for
//  additional information regarding your rights and obligations.
//
//  SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY
//  WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A
//  PARTICULAR PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE
//  LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY,
//  CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY
//  DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY
//  INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST
//  PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS,
//  TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT
//  LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
//
//  1/31/07
//
// *****************************************************************************

// Created by ZENA(TM) Version 2.0.1.0, 10/7/2007, 16:10:03
 /****************************************************************************
   Includes
 *****************************************************************************/
/* Configuration Files */
#include "zigbee.def"

/* Stack Related Files */
#include "zNWK.h"
#include "zZDO.h"

/* ZCL Related Files */
#include "ZCL_Interface.h"
#include "ZCL_Basic.h"
#include "ZCL_Identify.h"
#include "ZCL_Alarm.h"
#include "ZCL_Commissioning.h"
#include "ZCL_Power_Configuration.h"
#include "ZCL_Key_Establishment.h"
#include "ZCL_Time.h"
#include "ZCL_Metering.h"
#include "ZCL_Price.h"
#include "ZCL_Message.h"

#if defined(I_SUPPORT_GROUP_ADDRESSING)
	#include "ZCL_Groups.h"
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if defined(I_SUPPORT_SCENES)
	#include "ZCL_Scenes.h"
#endif /*I_SUPPORT_SCENES*/

/* SE Related Files */
#include "SE_Interface.h"
#include "SE_Profile.h"
#include "SE_MTR.h"

/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/

// Default ZigBee Router cannot be Trust Center and Network Manager
#define SERVER_MASK_PRIMARY_TRUST_CENTER_BIT    0x00
#define SERVER_MASK_NETWORK_MANAGER_BIT         0x00

#define SERVER_MASK SERVER_MASK_PRIMARY_TRUST_CENTER_BIT | SERVER_MASK_NETWORK_MANAGER_BIT

/*****************************************************************************
   Customizable Macros
 *****************************************************************************/
#define METER_NUMBER_OF_MANDATORY_OUTPUT_CLUSTERS     0x04
#define METER_NUMBER_OF_MANDATORY_INPUT_CLUSTERS      0x06

#if I_SUPPORT_CBKE == 1
    #define METER_CBKE_OUTPUT_CLUSTER                 0x01
    #define METER_CBKE_INPUT_CLUSTER                  0x01
#else
    #define METER_CBKE_OUTPUT_CLUSTER                 0x00
    #define METER_CBKE_INPUT_CLUSTER                  0x00
#endif

#if defined(I_SUPPORT_GROUP_ADDRESSING)
	#define METER_GROUPS_OUTPUT_CLUSTER               0x01
    #define METER_GROUPS_INPUT_CLUSTER                0x01
#else
    #define METER_GROUPS_OUTPUT_CLUSTER               0x00
    #define METER_GROUPS_INPUT_CLUSTER                0x00
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)
	#define METER_SCENES_OUTPUT_CLUSTER               0x01
    #define METER_SCENES_INPUT_CLUSTER                0x01
#else
    #define METER_SCENES_OUTPUT_CLUSTER               0x00
    #define METER_SCENES_INPUT_CLUSTER                0x00
#endif /*I_SUPPORT_SCENES*/

#define    METER_OUTPUT_CLUSTERS                  METER_NUMBER_OF_MANDATORY_OUTPUT_CLUSTERS +\
                                                METER_CBKE_OUTPUT_CLUSTER +\
                                                METER_GROUPS_OUTPUT_CLUSTER +\
                                                METER_SCENES_OUTPUT_CLUSTER
                                                
#define    METER_INPUT_CLUSTERS                   METER_NUMBER_OF_MANDATORY_INPUT_CLUSTERS +\
                                                METER_CBKE_INPUT_CLUSTER +\
                                                METER_GROUPS_INPUT_CLUSTER +\
                                                METER_SCENES_INPUT_CLUSTER

/*****************************************************************************
   Data Structures
 *****************************************************************************/
 /* None */

/*****************************************************************************
   Variable Definitions
 *****************************************************************************/
//ROM NODE_DESCRIPTOR Config_Node_Descriptor =
//{
//    0x01,                 // ZigBee Router
//    0x00,                 // (reserved)
//    0x00,                 // (APS Flags, not currently used)
//    0x08,                 // Frequency Band 2400
//    MY_CAPABILITY_INFO,   // Capability Information
//    #if !defined(__C32__)
//        {{0x00, 0x00}},       // Manufacturer Code
//    #else
//        0x0000,
//    #endif
//#ifdef I_SUPPORT_SECURITY
//    0x4C,                 // Max Buffer Size
//#else
//    0x5E,                 // Max Buffer Size
//#endif
//    #if !defined(__C32__)
//        {{0xFF, 0x00}},       // Max Incoming Transfer Size
//    #else
//        0x00ff,
//    #endif
//
//    #if !defined(__C32__)
//        {{SERVER_MASK, 0x00}},// ServerMask fields
//    #else
//        {0x0000 | SERVER_MASK},
//    #endif
//    
//    #if !defined(__C32__)
//        {{0x00, 0x04}},       // Max Outgoting Transfer Size
//    #else
//        0x0400,
//    #endif
//    {0x00}                // NodeDescriptorCapability Information
//};

//ROM NODE_POWER_DESCRIPTOR Config_Power_Descriptor =
//{
//    0x00, //Power mode: RxOn
//    0x01, //Available power: Mains DispBatt
//    0x01, //Current power: Mains
//    0x0c  //Fill in current power level
//};

ROM NODE_SIMPLE_DESCRIPTOR Config_Simple_Descriptors_MTR[] =
{
//--------------------------------------
// ZigBee Device Object Endpoint
// DO NOT MODIFY THIS DESCRIPTOR!!!
//--------------------------------------
    {
        EP_ZDO,
        #if !defined(__C32__)
            {{0x00, 0x00}},  // ZDO Profile ID
        #else
            0x0000,
        #endif
        
        #if !defined(__C32__)
            {{0x00, 0x00}},  // ZDO Device
        #else
            0x0000,
        #endif
        0x00,          // ZDO Version
        NO_OTHER_DESCRIPTOR_AVAILABLE,
        ZDO_INPUT_CLUSTERS,
        {
            NWK_ADDR_req,
            IEEE_ADDR_req,
            NODE_DESC_req,
            POWER_DESC_req,
            SIMPLE_DESC_req,
            ACTIVE_EP_req,
            MATCH_DESC_req
        },
        ZDO_OUTPUT_CLUSTERS,
        {
            NWK_ADDR_rsp,
            IEEE_ADDR_rsp,
            NODE_DESC_rsp,
            POWER_DESC_rsp,
            SIMPLE_DESC_rsp,
            ACTIVE_EP_rsp,
            MATCH_DESC_rsp
        }
    }
,
    {
        METER_ENDPOINT_ID,
        #if !defined(__C32__)
            {SE_PROF_ID},
        #else
            {SE_PROFILE_ID},
        #endif
        #if defined(__C32__)
            {METER_ID_DEVICE},
        #else
            {METER_DEVICE_ID},  // Meter Device
        #endif
        0x00,            // SE Version
        NO_OTHER_DESCRIPTOR_AVAILABLE,
        METER_INPUT_CLUSTERS,
        {
            /* General Clusters */
            ZCL_BASIC_CLUSTER,
            #if I_SUPPORT_CBKE == 1
                ZCL_KEY_ESTABLISHMENT_CLUSTER,
            #endif /*#if I_SUPPORT_CBKE*/
            ZCL_POWER_CONFIGURATION_CLUSTER,
            ZCL_ALARM_CLUSTER,
            #ifdef I_SUPPORT_GROUP_ADDRESSING
				ZCL_GROUPS_CLUSTER,
			#endif /*I_SUPPORT_GROUP_ADDRESSING*/
			#ifdef I_SUPPORT_SCENES
				ZCL_SCENES_CLUSTER,
			#endif /*I_SUPPORT_SCENES*/
            ZCL_COMMISSIONING_CLUSTER,
            ZCL_IDENTIFY_CLUSTER,
			/* SE Specific clusters */
            ZCL_METERING_CLUSTER
        },
        METER_OUTPUT_CLUSTERS,
        {
            /* General Clusters */
            #if I_SUPPORT_CBKE == 1
                ZCL_KEY_ESTABLISHMENT_CLUSTER ,
            #endif /*#if I_SUPPORT_CBKE*/
            
            ZCL_COMMISSIONING_CLUSTER,
            
			#ifdef I_SUPPORT_GROUP_ADDRESSING
				ZCL_GROUPS_CLUSTER,
			#endif /*I_SUPPORT_GROUP_ADDRESSING*/
			
			#ifdef I_SUPPORT_SCENES			
				ZCL_SCENES_CLUSTER,
			#endif /*I_SUPPORT_SCENES*/

            /* SE Specific clusters */
            ZCL_TIME_CLUSTER,
            ZCL_PRICE_CLUSTER,
            ZCL_MESSAGING_CLUSTER
        }
    }
};

ROM _Config_NWK_Mode_and_Params Config_NWK_Mode_and_Params_MTR =
{
    nwkcProtocolVersion,        //Protocol Version
    PROFILE_myStackProfileId,   //Stack Profile ID
    MAC_PIB_macBeaconOrder,     //Beacon Order
    MAC_PIB_macSuperframeOrder, //Superframe Order
    MAC_PIB_macBattLifeExt,     //Battery Life Extension
    PROFILE_nwkSecurityLevel,   //Security Level
    ALLOWED_CHANNELS_PRE_CONFIG            //Channels to scan
};

/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
/* None */

/*****************************************************************************
  Private Functions
 *****************************************************************************/
 /* None */

/*****************************************************************************
  Public Functions
 *****************************************************************************/
 /* None */

