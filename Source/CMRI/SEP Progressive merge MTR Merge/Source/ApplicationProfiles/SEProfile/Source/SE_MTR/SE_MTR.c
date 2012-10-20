/*********************************************************************
 *
 *                SE Simple Metering Device
 *
 *********************************************************************
 * FileName:        SE_Meter.c
 * Dependencies:
 * Processor       : PIC18 / PIC24 / PIC32
 * Complier        : MCC18 v3.00 or higher
 *                 : MCC30 v2.05 or higher
 *                 : MCC32 v1.05 or higher
 *
 * Company:        Microchip Technology, Inc.
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
 *
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
#include "HAL.h"

/* Stack Related Files */
#include "ZigBeeTasks.h"
#include "zAPS.h"
#include "zAPL.h"
#include "zAIL.h"
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
#endif /*#if I_SUPPORT_CBKE*/

#if defined(I_SUPPORT_GROUP_ADDRESSING)
	#include "ZCL_Groups.h"
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)
	#include "ZCL_Scenes.h"
#endif /*I_SUPPORT_SCENES*/

/* SE Related Files */
#include "SE_Interface.h"
#include "SE_Profile.h"
#include "SE_MTR.h"

#if I_SUPPORT_CBKE == 1
    #include "eccapi.h"
    #include "SE_CBKE.h"
#endif /*#if I_SUPPORT_CBKE*/

/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/
#define GET_HIGHER_NIBBLE                               0x04

#define GET_PROFILE_RESP_FIXED_FIELDS_LEN               0x0A
#define REQ_MIRROR_RESPONSE_BASE_INDEX                  0x03

#define SIZE_OF_UTC_TIME                                0x04
#define SIZE_OF_PROFILE_INTERVAL_VALUE                  0x03


/*These Macros define various profile Interval Periods.Meter can use any
one of the profile Interval Period */
#define DAILY_IN_SECONDS                                86400
#define SIXTY_MINUTES_IN_SECONDS                        3600
#define THIRTY_MINUTES_IN_SECONDS                       1800
#define FIFTEEN_MINUTES_IN_SECONDS                      900
#define TEN_MINUTES_IN_SECONDS                          600
#define SEVEN_AND_HALF_MINUTES_IN_SECONDS               450
#define FIVE_MINUTES_IN_SECONDS                         300
#define TWO_AND_HALF_MINUTES_IN_SECONDS                 150

/*****************************************************************************
   Customizable Macros
 *****************************************************************************/
/*It define no.of interval supported by the meter*/
#define METER_NUM_PROFILES_SUPPORTED                    0x0A

/*It defines Meter supporting profileIntervalPeriod.We can select any one of the
  above profile Interval periods.*/
#define PROFILE_INTERVAL_PERIOD                         TWO_AND_HALF_MINUTES_IN_SECONDS

/*****************************************************************************
   Data Structures
 *****************************************************************************/
typedef struct _App_ProfileMeteringIntervals
{
    DWORD utcTime;
    BYTE metringInterval[3];
} App_ProfileMeteringIntervals;

/******************************************************************************
        Variable Definations
******************************************************************************/
extern ZIGBEE_PRIMITIVE currentPrimitive_MTR;
extern BYTE AllowJoin;
extern BYTE startMode;
extern BYTE CommRestartTimerInProgress;
extern TICK CommRestartTime;
extern ALARM_AlarmTable AlarmEntries[];

extern BYTE asduData[80];

//#ifdef  I_AM_RFD        extern TICK startRFDSendTime;
//#endif 

#if I_SUPPORT_CBKE == 1
/*Supports CBKE key establishment suite*/
ROM _ZCL_KeyEstClusterAttributes Meter_KeyEstAttributeDefaultValue = { 0x0001 };
#endif /*I_SUPPORT_CBKE*/

ROM ZCL_IdentifyClusterAttributes Meter_IdentifyAttributeDefaultValue = { 0x0000 };

#ifdef I_SUPPORT_GROUP_ADDRESSING
/* Supports the group names */
ROM ZCL_GroupsClusterAttributes Meter_GroupsAttributeDefaultValue = { 0x80 };
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)
/*Default value for scenes cluster*/
ROM ZCL_ScenesClusterAttributes Meter_ScenesAttributeDefaultValue = 
{
	/* SceneCount */
	0x00,
	
	/* CurrentScene */
	0x00,
	
	/* CurrentGroup */
	0x00,

	/* SceneValid */
	0x00,

	/* NameSupport.MSB tells whether names are supported or not  */
	0x80
	#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
	,
		/* LastConfiguredBy */
		{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
	#endif
};
#endif /*I_SUPPORT_SCENES*/

ROM ZCL_BasicClusterAttributes Meter_BasicAttributeDefaultValue =
{
    /* ZCLVersion */
    0x02,
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1

    /* ApplicationVersion */
    0x05,

    /* StackVersion */
    0x05,

    /* HWVersion */
    0x01,

    /* ManufacturerName */
   {0x09,'M','i','c','r','o','c','h','i','p'},

    /* ModelIdentifier */
    {0x0a,'E','x','p','l','o','r','e','r','1','6'},

    /* DateCode */
    {0x06,'0','6','0','3','1','1'},
#endif /* I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES */

    /* PowerSource */
    0x00
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    ,
    /* LocationDescription */
    {0x09,'G','a','r','n','e','r',' ','N','C'},

    /* PhysicalEnvironment */
    0x00,

    /* DeviceEnabled */
    0x01,

    /* AlarmMask */
    0x00
#endif /*I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES*/
};

ROM ZCL_MeteringClusterAttributes Meter_MeteringAttributeDefaultValue =
{
    /* Reading Information Attribute Set */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    0x0000,
    0x00,
    0x00000000,
    0x00000000,
    0x00000000,
#endif
    /* TOU Information Attribute Set */
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
#endif

    /* Meter Status Attribute Set */
    0x00,

    /* Formatting Attribute Set */
    0x00,
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
#endif
    0x40,
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    0x40,
    0x40,
#endif
    0x00

    /* ESP Historical Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    ,
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    0x00000000,
    0x00000000,
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
#endif
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    /* Load Profile Configuration Attribute Set */
    0x18,
#endif
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    /* Supply Limit Attribute Set */
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    0x00,
    0x00
#endif
};

ROM Commissioning_Cluster_Attributes Meter_CommissioningAttributeDefaultValues =
{
    NWK_SHORT_ADDRESS,
    {
        NWK_EXTENDED_PAN_ID_BYTE0,
        NWK_EXTENDED_PAN_ID_BYTE1,
        NWK_EXTENDED_PAN_ID_BYTE2,
        NWK_EXTENDED_PAN_ID_BYTE3,
        NWK_EXTENDED_PAN_ID_BYTE4,
        NWK_EXTENDED_PAN_ID_BYTE5,
        NWK_EXTENDED_PAN_ID_BYTE6,
        NWK_EXTENDED_PAN_ID_BYTE7
    },
    MAC_DEFAULT_PANID,
    ALLOWED_CHANNELS_PRE_CONFIG,
    nwkcProtocolVersion,
    PROFILE_myStackProfileId,
    DEFAULT_STARTUP_CONTROL_MTR,
    {
        TRUST_CENTER_LONG_ADDR_BYTE0,
        TRUST_CENTER_LONG_ADDR_BYTE1,
        TRUST_CENTER_LONG_ADDR_BYTE2,
        TRUST_CENTER_LONG_ADDR_BYTE3,
        TRUST_CENTER_LONG_ADDR_BYTE4,
        TRUST_CENTER_LONG_ADDR_BYTE5,
        TRUST_CENTER_LONG_ADDR_BYTE6,
        TRUST_CENTER_LONG_ADDR_BYTE7

    },
    {
        TRUST_CENTER_MASTER_KEY0,
        TRUST_CENTER_MASTER_KEY1,
        TRUST_CENTER_MASTER_KEY2,
        TRUST_CENTER_MASTER_KEY3,
        TRUST_CENTER_MASTER_KEY4,
        TRUST_CENTER_MASTER_KEY5,
        TRUST_CENTER_MASTER_KEY6,
        TRUST_CENTER_MASTER_KEY7,
        TRUST_CENTER_MASTER_KEY8,
        TRUST_CENTER_MASTER_KEY9,
        TRUST_CENTER_MASTER_KEY10,
        TRUST_CENTER_MASTER_KEY11,
        TRUST_CENTER_MASTER_KEY12,
        TRUST_CENTER_MASTER_KEY13,
        TRUST_CENTER_MASTER_KEY14,
        TRUST_CENTER_MASTER_KEY15
    },
    {
        NETWORK_KEY_BYTE00,
        NETWORK_KEY_BYTE01,
        NETWORK_KEY_BYTE02,
        NETWORK_KEY_BYTE03,
        NETWORK_KEY_BYTE04,
        NETWORK_KEY_BYTE05,
        NETWORK_KEY_BYTE06,
        NETWORK_KEY_BYTE07,
        NETWORK_KEY_BYTE08,
        NETWORK_KEY_BYTE09,
        NETWORK_KEY_BYTE10,
        NETWORK_KEY_BYTE11,
        NETWORK_KEY_BYTE12,
        NETWORK_KEY_BYTE13,
        NETWORK_KEY_BYTE14,
        NETWORK_KEY_BYTE15
    },
    DEFAULTINSECUREJOIN,
    {
        PRECONFIGURED_LINK_KEY00,
        PRECONFIGURED_LINK_KEY01,
        PRECONFIGURED_LINK_KEY02,
        PRECONFIGURED_LINK_KEY03,
        PRECONFIGURED_LINK_KEY04,
        PRECONFIGURED_LINK_KEY05,
        PRECONFIGURED_LINK_KEY06,
        PRECONFIGURED_LINK_KEY07,
        PRECONFIGURED_LINK_KEY08,
        PRECONFIGURED_LINK_KEY09,
        PRECONFIGURED_LINK_KEY10,
        PRECONFIGURED_LINK_KEY11,
        PRECONFIGURED_LINK_KEY12,
        PRECONFIGURED_LINK_KEY13,
        PRECONFIGURED_LINK_KEY14,
        PRECONFIGURED_LINK_KEY15
    },
    NETWORK_KEY_SEQ,
    DEFAULT_NWK_KEY_TYPE,
    DEFAULT_NWK_MGR_ADDR,
    DEFAULT_SCAN_ATTEMPS,
    DEFAULT_TIME_BETWEEN_SCANS,
    DEFAULT_REJOIN_INTERVAL,
    DEFAULT_MAX_REJOIN_INTERVAL,

    DEFAULT_INDIRECT_POLL_RATE,
    DEFAULT_PARENT_RETRY_THRESHOLD,

    TRUE,
    DEFAULT_RADIUS,
    DEFAULT_CONC_DISC_TIME
};

/*Default value for the Power Configuration Cluster Attributes*/
ROM ZCL_PowerConfigurationClusterAttributes Meter_PowerConfigurationAttributeDefaultValue =
{
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    /*Mains Voltage*/
    0x0000,
    /*Mains Frequency*/
    0x00,
    /*Mains Alarm mask*/
    0x00,
    /*Mains Voltage Min Threshold Point*/
    0x0000,
    /*Mains Voltage Max Threshold Point*/
    0xFFFF,
    /*Mains Voltage Dwell Trip Point*/
    0x0000,
    /*Battery Voltage*/
    0x00,
    /*Battery Manfacturer Name*/
    {0x09,'M','i','c','r','o','c','h','i','p'},
    /*Battery Size*/
    0xFF,
    /*Battery A Hr rating*/
    0x0000,
    /*Battery Quantity*/
    0x00,
    /*Battery Rated Voltage*/
    0x00,
    /*Battery Alarm mask*/
    0x00,
    /*Battery Voltage Min Threshold*/
    0x00
#endif /* I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES */
};

/*Default value for the Alarm Cluster Attributes*/
ROM ZCL_AlarmClusterAttributes Meter_AlarmAttributeDefaultValue =
{
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
     /*Alarm Count*/
     0x0000
#endif
};

ROM ZCL_ClusterDefinition* pMeterClusterServerDefinition[] =
{
    &zcl_IdentifyClusterServerDefinition,
    &zcl_BasicClusterServerDefinition,
    &zcl_CommissioningClusterServerDefinition,
    &zcl_PowerConfigurationClusterServerDefinition,
    &zcl_AlarmClusterServerDefinition,
    &zcl_MeteringClusterServerDefinition
	#if I_SUPPORT_CBKE == 1
    ,
    &zcl_KeyEstClusterServerDefinition
    #endif /*I_SUPPORT_CBKE*/
    
    #ifdef I_SUPPORT_GROUP_ADDRESSING
	,	
		&zcl_GroupsClusterServerDefinition 	
	#endif /*I_SUPPORT_GROUP_ADDRESSING*/
	
	#if (I_SUPPORT_SCENES == 1)
	,	
		&zcl_ScenesClusterServerDefinition 	
	#endif /*I_SUPPORT_SCENES*/
};

ROM ZCL_ClusterDefinition* pMeterClusterClientDefinition[] =
{
    &zcl_IdentifyClusterClientDefinition,
    &zcl_BasicClusterClientDefinition,
    &zcl_TimeClusterClientDefinition,
    &zcl_CommissioningClusterClientDefinition,
    &zcl_PowerConfigurationClusterClientDefinition,
    &zcl_AlarmClusterClientDefinition,
    &zcl_PriceClusterClientDefinition,
    &zcl_MessageClusterClientDefinition
    #if I_SUPPORT_CBKE == 1
    ,
    &zcl_KeyEstClusterClientDefinition
    #endif /*I_SUPPORT_CBKE*/
    
	#ifdef I_SUPPORT_GROUP_ADDRESSING
	,	
		&zcl_GroupsClusterClientDefinition 	
	#endif /*I_SUPPORT_GROUP_ADDRESSING*/

    #if (I_SUPPORT_SCENES == 1)
	,	
		&zcl_ScenesClusterClientDefinition 	
	#endif /*I_SUPPORT_SCENES*/
};

ZCL_DeviceInfo Meter_DeviceServerinfo =
{
        METER_ENDPOINT_ID,
        NUMBER_OF_SERVER_CLUSTERS_SUPPORTED_MTR,
        pMeterClusterServerDefinition
};

/*SE Related*/
ZCL_DeviceInfo *pAppListOfDeviceServerInfo_MTR[] =
{
    &Meter_DeviceServerinfo
};

ZCL_DeviceInfo Meter_DeviceClientinfo =
{
        METER_ENDPOINT_ID,
        NUMBER_OF_CLIENT_CLUSTERS_SUPPORTED_MTR,
        pMeterClusterClientDefinition
};

/*SE Related*/
ZCL_DeviceInfo *pAppListOfDeviceClientInfo_MTR[] =
{
    &Meter_DeviceClientinfo
};

App_AttributeStorage  App_AttributeStorageTable_MTR[] =
{
   { METER_ENDPOINT_ID, ZCL_IDENTIFY_CLUSTER,
     sizeof(ZCL_IdentifyClusterAttributes),
     0xFFFF, &Meter_IdentifyAttributeDefaultValue },

   { METER_ENDPOINT_ID, ZCL_BASIC_CLUSTER,
     sizeof(ZCL_BasicClusterAttributes),
     0xFFFF, &Meter_BasicAttributeDefaultValue    },

	#ifdef I_SUPPORT_GROUP_ADDRESSING
	   { METER_ENDPOINT_ID, ZCL_GROUPS_CLUSTER,
	     sizeof(ZCL_GroupsClusterAttributes),
	     0xFFFF, &Meter_GroupsAttributeDefaultValue },
	#endif /*I_SUPPORT_GROUP_ADDRESSING*/
	
	#if (I_SUPPORT_SCENES == 1)
	    { METER_ENDPOINT_ID, ZCL_SCENES_CLUSTER,
	      sizeof(ZCL_ScenesClusterAttributes),
	      0xFFFF, &Meter_ScenesAttributeDefaultValue  },
	#endif /*I_SUPPORT_SCENES*/

   { METER_ENDPOINT_ID, ZCL_METERING_CLUSTER,
     sizeof(ZCL_MeteringClusterAttributes),
     0xFFFF, &Meter_MeteringAttributeDefaultValue   },

   { METER_ENDPOINT_ID, ZCL_COMMISSIONING_CLUSTER,
     sizeof(Commissioning_Cluster_Attributes),
     0xFFFF, &Meter_CommissioningAttributeDefaultValues    },

   { METER_ENDPOINT_ID, ZCL_POWER_CONFIGURATION_CLUSTER,
     sizeof(ZCL_PowerConfigurationClusterAttributes),
     0xFFFF, &Meter_PowerConfigurationAttributeDefaultValue },

   { METER_ENDPOINT_ID, ZCL_ALARM_CLUSTER,
     sizeof(ZCL_AlarmClusterAttributes),
     0xFFFF, &Meter_AlarmAttributeDefaultValue }

#if I_SUPPORT_CBKE == 1
    ,
   { METER_ENDPOINT_ID, ZCL_KEY_ESTABLISHMENT_CLUSTER,
     sizeof(_ZCL_KeyEstClusterAttributes),
     0xFFFF, &Meter_KeyEstAttributeDefaultValue }
#endif /*#if I_SUPPORT_CBKE */

#if I_SUPPORT_CBKE == 1
    ,
   { METER_ENDPOINT_ID, ZCL_KEY_ESTABLISHMENT_CLUSTER,
     sizeof(_ZCL_KeyEstClusterAttributes),
     0xFFFF, &Meter_KeyEstAttributeDefaultValue }
#endif /*#if I_SUPPORT_CBKE */
};

/*This variable holds the number of price events updated by the Utility*/
extern BYTE numberOfPriceEvents;
/*This variable holds the number of Tiers supported by the device*/
//BYTE numberofTiersSupported;

/*This gives the number of entries in the NVM storage table for
Application.This is used by SE_Profile.c during initialization*/
BYTE NoOfEntriesInAttrStorageTable =  (sizeof( App_AttributeStorageTable_MTR ) /
                                    sizeof( App_AttributeStorage ));

/*This table is used to Store the Price events. This table currently
intialized with the default values. This table entries may change
when user wants to change some of the tier information.
This table entry should be set by the Utility and initialized with
the Utility specified value. */
//PriceEvents ListOfPriceEvents[MAX_PRICE_EVENTS] =
//{
//    /*EventID, UTCStartTime, DurationInMinutes, StartTime, Tier, RateLabel, Price, PriceTrailingDigitAndPriceTier*/
//    { 0x00000001, 0x11CC4000, 0x00F0, 0,     1, {0x06, 'T', 'i', 'e', 'r', ' ', '1'}, 0x00000001, 0x01 },
//    { 0x00000001, 0x11CC7840, 0x00F0, 14400, 2, {0x06, 'T', 'i', 'e', 'r', ' ', '2'}, 0x00000002, 0x02 },
//    { 0x00000001, 0x11CCB080, 0x00F0, 28800, 3, {0x06, 'T', 'i', 'e', 'r', ' ', '3'}, 0x00000003, 0x03 },
//    { 0x00000001, 0x11CCE8C0, 0x00F0, 43200, 4, {0x06, 'T', 'i', 'e', 'r', ' ', '4'}, 0x00000004, 0x04 },
//    { 0x00000001, 0x11CD2100, 0x00F0, 57600, 5, {0x06, 'T', 'i', 'e', 'r', ' ', '5'}, 0x00000005, 0x05 },
//    { 0x00000001, 0x11CD5940, 0x00F0, 72000, 6, {0x06, 'T', 'i', 'e', 'r', ' ', '6'}, 0x00000006, 0x06 },
//};
//ZCL_Price_PublishPriceInfo PublishPriceInfo;

App_ProfileMeteringIntervals testMeterProfiles[] =
{
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }},
    { 0x00000000, {0x00, 0x00, 0x00 }}
};


extern Meter_Status mirrorStatus;

/*Initialized the display table to 0xFF - invalid value*/
extern displayMessages displayMessagesTable;

#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    static BYTE noOfTimesProfileIntervalElapses = 0x00;
#endif

/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
static void AddConsumptionToMeterReadingByteArray
(
    BYTE *data,
    BYTE len,
    BYTE consumption
);
#endif
static BYTE SE_Meter_Update_MeterReadings( void );

/******************************************************************************
* Function:        static void ValidatePriceEvents ( void )
*
* PreCondition:    None
*
* Input:           None
*
* Output:          None
*
* Return :         None
*
* Side Effects:    None
*
* Overview:        This function is called when Price Events needs to verified
*                  for Gaps. This function reagrranges the Price Events by leaving
*                  gaps, wherever necessary, as notified by Publish Price Command.
*
* Note:            None
*****************************************************************************/
static void ValidatePriceEvents ( void );

/***************************************************************************************
 * Function:        static void UpdateProfileIntervalValues(App_ProfileMeteringIntervals *profileMeteringIntervals)
 *
 * PreCondition:    None
 *
 * Input:           profileMeteringIntervals - it holds ProfileMeteringIntervalValue & corresponding
 *                                             ProfileMeteringIntervalValueStartTime
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function updates ProfileMeteringIntervalArray(testMeterProfiles).It
 *                  is used to send GetProfileResponseCommand.
 *
 * Note:            None
 *
 *
 ***************************************************************************************/
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
static void UpdateProfileIntervalValues
(
    App_ProfileMeteringIntervals *profileMeteringIntervals
);
#endif
/******************************************************************************
* Function:         static void ProcessPublishPriceEvent ( BYTE *pAsdu )
*
* PreCondition:     None
*
* Input:            pAsdu - Specifies the starting location of payload
*                          of the Publish Price Command
*
* Output:           None
*
* Return :          None
*
* Side Effects:     None
*
* Overview:         This function is called when Publish Price Event needs to
*                   be handled by the application. This function processes the
*                   Publish Price Command and updates the global structure
*                   PublishPriceInfo. This structure is used for updating the
*                   Price events table entry.
*
* Note:             pAsdu points to Start of the Publish Price Command Payload
*                   It does not contain the ZCL Header
*****************************************************************************/
static void ProcessPublishPriceEvent ( BYTE *pAsdu );

/*****************************************************************************
  Private Functions
 *****************************************************************************/
/* None */

/*****************************************************************************
  Public Functions
 *****************************************************************************/
/******************************************************************************
 * Function:        void SE_HandleEvents( ZCL_Event *pZCLEvent,
 *                                     APP_DATA_indication *p_dataInd );
 *
 * PreCondition:    None
 *
 * Input:           pZCLEvent  -  This points ZCL Event.
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function handles the Events send by the ZCL.
 *
 * Note:            None
 *****************************************************************************/
void SE_HandleEvents_MTR( ZCL_Event *pZCLEvent, APP_DATA_indication *p_dataInd )
{
    switch( pZCLEvent->eventId )
    {
        case ZCL_Identify_IdentifyEvent :
                appIdentifyTimers.IdentifyTime = 0x01;
                appIdentifyTimers.IdentifyStartTime = TickGet();
                LIGHT3_ON();
        break;

        case ZCL_Price_PublishPriceEvent:
            ProcessPublishPriceEvent ( &(p_dataInd->asdu[ZCL_HEADER_LENGTH_WO_MANUF_CODE]) );
            break;

        case ZCL_MeteringGetProfileEvent:
        {
            BYTE i;
            BYTE j;
            BYTE testMeterProfileIndex = 0x00;
            /*It indicates the no.of profile interval values can send in get profile response command.*/
            BYTE numOfAvailableIntervals = 0x00;
            ZCL_MeteringClusterAttributes meteringClusterAttributes;
            BYTE asdu[50];
            BYTE asduLength;
            /*It holds NVM index for Metering Cluster*/
            WORD storageIndex;
            ZCL_MeteringGetProfileRspCmdPayload *GetProfileRspCmdPayload;
            ZCL_MeteringGetProfileReqCmdPayload *GetProfileReqCmdPayloadReceived;

            GetProfileReqCmdPayloadReceived = (ZCL_MeteringGetProfileReqCmdPayload *)&p_dataInd->asdu[3];
            /*It points to the ZCL payload of GetProfileResponse*/
            GetProfileRspCmdPayload = (ZCL_MeteringGetProfileRspCmdPayload *)&asdu[3];

            /*If GetProfileRequest command is received with endTime as 0x00000000 then we have to consider
              endTime is currentTime.*/
            if( GetProfileReqCmdPayloadReceived->endTime == 0x00000000 )
            {
                GetProfileReqCmdPayloadReceived->endTime = ZCL_Callback_GetCurrentTimeInSeconds();
            }

            /*Get Profile Response command asdu length will be varied depending upon the status
            values.For any status value ,Get Profile Response command have fixed lenght i.e
            GET_PROFILE_RESP_FIXED_FIELDS_LEN */
            asduLength = GET_PROFILE_RESP_FIXED_FIELDS_LEN;

                        /* ZCL Header */
            /* Frame control field. */
            *asdu  = ( ZCL_ServerToClient | ZCL_ClusterSpecificCommand );
            /*Update Transaction sequence number*/
            *( asdu + ZCL_FRAME_TSN_POSITION_MANUFACTURE_BIT_DISABLED ) =
                          appNextSeqNum++;
            /*Update commandId with GetProfileResponse*/
            *( asdu + ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) =
                        ZCL_MeteringGetProfileResponse;

            /*This is initialzed with zero.It will be UPDATED when GetProfileResponse as
              Success*/
            GetProfileRspCmdPayload->numberOfPeriodsDelivered = 0x00;
            /*GetProfileResponse EndTime is assigned with Received EndTime for GetProfileResponse as
              not Success.If Success,according the meterProfileInterval array it is updated*/
            GetProfileRspCmdPayload->endTime = GetProfileReqCmdPayloadReceived->endTime;

            /* Currently ProfileIntervalPeriod is assigned with 2.5 minutes */
            GetProfileRspCmdPayload->profileIntervalPeriod = ZCL_MeteringTwoAndHalfMinutes;

            /* It is used to get NVM index for Metering cluster */
            if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                ZCL_Callback_GetClusterIndex
                (
                    METER_ENDPOINT_ID,
                    ZCL_METERING_CLUSTER,
                    &storageIndex
                )
            )
            {
                /* Read the ZCL Meter Cluster Server Attributes from NVM */
                ZCL_Callback_GetAttributeValue
                (
                    storageIndex,
                    (BYTE *)&meteringClusterAttributes,
                    sizeof(ZCL_MeteringClusterAttributes)
                );
            }

            /* Undefined interval channel requested */
            if( (GetProfileReqCmdPayloadReceived->intervalChannel !=
                    ZCL_MeteringConsumptionDelivered ) &&
                (GetProfileReqCmdPayloadReceived->intervalChannel !=
                    ZCL_MeteringConsumptionReceived ) )
            {
                GetProfileRspCmdPayload->status =
                    ZCL_MeteringUndefinedIntervalChannelRequested;
            }
            #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
            /* More periods requested than can be returned */
            else if
            (
                GetProfileReqCmdPayloadReceived->numberOfPeriods >
                meteringClusterAttributes.maxNumberOfPeriodsDelivered
            )
            {
                 GetProfileRspCmdPayload->status =
                            ZCL_MeteringMorePeriodsRequestedThanCanBeReturned;
            }
            #endif
            else
            {
                /*This loop is used for received endTime below captured Intrvals is existed
                  in MeterProfileInterval array.If existed, corresponding array index is stored into
                  testMeterProfileIndex and endTime is stored corresponding UTC time*/
                for( i = 0x00; i<METER_NUM_PROFILES_SUPPORTED; i++ )
                {
                   if( ( GetProfileReqCmdPayloadReceived->endTime >= testMeterProfiles[i].utcTime )&&
                        ( 0x00000000 != testMeterProfiles[i].utcTime ) )
                    {
                        testMeterProfileIndex = i;
                        GetProfileRspCmdPayload->endTime = testMeterProfiles[testMeterProfileIndex].utcTime;
                        GetProfileRspCmdPayload->status = ZCL_MeteringIntervalSuccess;
                        break;
                    }
                }
                /*If received endTime is not existed in MeterProfileInterval array,means corresponding
                  meterProfileInterval value is not captured in array*/
                if( i == METER_NUM_PROFILES_SUPPORTED )
                {
                    /*GetProfileResponse command No.of periods delivered should be 0x00.*/
                    numOfAvailableIntervals = 0x00;
                    /*Update status*/
                    GetProfileRspCmdPayload->status = ZCL_MeteringNoIntervalsAvailableForTheRequestedTime;


                }
                /* Number of Periods Delivered */
                if( ZCL_MeteringIntervalSuccess == GetProfileRspCmdPayload->status )
                {
                    for( i = testMeterProfileIndex,j=0x00; i < METER_NUM_PROFILES_SUPPORTED; i++,j++ )
                    {
                        /*Copying the ProfileInterval values into response frame from testMeterProfileIndex */
                        if( testMeterProfiles[i].utcTime )
                        {
                            memcpy
                            (
                                    (BYTE *)&(GetProfileRspCmdPayload->intervals[j].metringInterval[0]),
                                    (BYTE *)&(testMeterProfiles[i].metringInterval[0]),
                                    0x03
                            );
                            numOfAvailableIntervals++;
                        }
                    }
                    /*Received GetProfileRequest Command,no.of periods delivered /received is less
                      than numOfAvailableIntervals in  testMeterProfiles array.
                      Then we need send in response frame with requested no.of Intervals.otherwise
                      we need send in response frame with numOfAvailableIntervals */
                    if ( GetProfileReqCmdPayloadReceived->numberOfPeriods < numOfAvailableIntervals )
                    {
                        GetProfileRspCmdPayload->numberOfPeriodsDelivered = GetProfileReqCmdPayloadReceived->numberOfPeriods;
                    }
                    else
                    {
                        GetProfileRspCmdPayload->numberOfPeriodsDelivered = numOfAvailableIntervals;
                    }
                    /* Update asdu length */
                    asduLength += ( GetProfileRspCmdPayload->numberOfPeriodsDelivered * 0x03 );

                }
            }

            App_SendData
            (
                APS_ADDRESS_16_BIT,
                ( BYTE *)&(p_dataInd->SrcAddress.ShortAddr.v[0]),
                p_dataInd->SrcEndpoint,
                asduLength,
                ZCL_METERING_CLUSTER,
                asdu
            );
        }

        break;

        case ZCL_MeteringRequestMirrorResponseEvent:
        {
            WORD_VAL endPoint;

            endPoint.v[0] = *( p_dataInd->asdu +
                REQ_MIRROR_RESPONSE_BASE_INDEX);
            endPoint.v[1] = *( p_dataInd->asdu +
                REQ_MIRROR_RESPONSE_BASE_INDEX + 1);
            if ( endPoint.Val == 0xFFFF)
            {
                mirrorStatus.ESP_HasCapability  = FALSE;
                mirrorStatus.ESP_EndPoint       = 0xFF;
                mirrorStatus.deviceMirroring    = FALSE;
            }
            else
            {
                mirrorStatus.ESP_HasCapability  = TRUE;
                mirrorStatus.ESP_EndPoint       = endPoint.Val;
                mirrorStatus.deviceMirroring    = TRUE;
            }
        }
        break;
        case ZCL_MeteringMirrorRemovedEvent:
        {
            mirrorStatus.deviceMirroring    = FALSE;
        }
        break;

        case ZCL_CommissioningRestartDeviceEvent:
        {
            Commissioning_Cluster_Attributes commAttribute;
            WORD storageIndex;
            BYTE status;

            /* Store the payload received in the frame, as restarting
               the device should be performed in the background after
               specified delay(sec) and jitter(milliseconds). */
            RestartDeviceCmdPayload.Options = pZCLEvent->pEventData[0];
            RestartDeviceCmdPayload.Delay = pZCLEvent->pEventData[1];
            RestartDeviceCmdPayload.Jitter = pZCLEvent->pEventData[2];

            /* Get the storage index (NVM), where commissioning
               cluster attribute set is stored */
            if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                    ZCL_Callback_GetClusterIndex
                    (
                        METER_ENDPOINT_ID,
                        ZCL_COMMISSIONING_CLUSTER,
                        &storageIndex
                    )
               )
               {
                    /* Get the commissioning cluster attribute set */
                    ZCL_Callback_GetAttributeValue
                    (
                        storageIndex,
                        (BYTE *)&commAttribute,
                        sizeof( Commissioning_Cluster_Attributes )
                    );
               }

            /* validate the commissioning cluster attribute set, for consistency */
            status = SE_ValidateSAS(&commAttribute);

            if( status == ZCL_Success )
            {
                /* Commsissioning attribute Set is in consistence state,
                   so set the flag to perform restart operation in background */
                commissioningRestartInProgress = TRUE;

                CommRestartTimerInProgress = TRUE;
                CommRestartTime = TickGet();
            }

            /* Send restart device response to the commissioning Device.
               The status can be SUCCESS or INCONSISTENT_STARTUP_STATE */
            SE_SendCommissioningClusterResponse
            (
                p_dataInd,
                ZCL_CommissioningRestartDeviceResponse,
                status
            );

        }
        break;

        case ZCL_CommissioningSaveStartupParametersEvent:
        {
            WORD storageIndex;
            STARTUP_ATTRIBUTE_SET commAttribute;
            BYTE *ptr;
            BYTE status = ZCL_InsufficientSpace;

            /* Get the storage index (NVM), where commissioning
               cluster attribute set is stored */
            if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                    ZCL_Callback_GetClusterIndex
                    (
                        METER_ENDPOINT_ID,
                        ZCL_COMMISSIONING_CLUSTER,
                        &storageIndex
                    )
               )
               {
                   /* STARTUP_ATTRIBUTE_SET structure in ZDO will have validity key
                      followed by startup parameters. So we need to get the
                      commissioning cluster attribute values after validity key  */
                   ptr = (BYTE *)&commAttribute;
                   ZCL_Callback_GetAttributeValue
                    (
                        storageIndex,
                        ptr + 2,
                        sizeof( Commissioning_Cluster_Attributes )
                    );
                    /* Update the validity key to SAS_TABLE_VALID */
                    commAttribute.validitykey = SAS_TABLE_VALID;
                    /* Save STARTUP_ATTRIBUTE_SET into NVM in the ZDO onto
                       index specified in the save startup parameters request. */
                    status = SaveSAS(&commAttribute, 0x00, (pZCLEvent->pEventData[1] - 1));
               }

               /* Send Save startup parameters response to the commissioning Device.
                  The status can be SUCCESS or INSUFFICIENT_SPACE */
               SE_SendCommissioningClusterResponse
               (
                    p_dataInd,
                    ZCL_CommissioningSaveStartupParametersResponse,
                    status
               );
        }
        break;

        case ZCL_CommissioningRestoreStartupParametersEvent:
        {
            WORD storageIndex;
            STARTUP_ATTRIBUTE_SET commAttribute;
            BYTE *ptr;
            BYTE status;

            /* Restore the startup parameters set stored in NVM in the ZDO.
               The status retuned will be INVALID_FIELD if nothing is stored
               on the index specified in the command frame otherwise SUCCESS */
            status = RestoreSAS(&commAttribute, 0x00, (pZCLEvent->pEventData[1] - 1));

            if( status == ZCL_Success )
            {
                /* Get the storage index (NVM), where commissioning
                   cluster attribute set is stored */
                if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                        ZCL_Callback_GetClusterIndex
                        (
                            METER_ENDPOINT_ID,
                            ZCL_COMMISSIONING_CLUSTER,
                            &storageIndex
                        )
                   )
                {
                    ptr = (BYTE *)&commAttribute;
                    /* Put the Restored attribute set onto commissioning
                       cluster attribute set leaving validity key.
                       Validity key is not part of commissioning attribute set. */
                    ZCL_Callback_PutAttributeValue
                    (
                        storageIndex,
                        ptr + 2,
                        sizeof( Commissioning_Cluster_Attributes )
                    );
                }
            }
            /* Send Restore startup parameters response to the commissioning Device.
               The status can be SUCCESS or INVALID_FIELD */
            SE_SendCommissioningClusterResponse
            (
                p_dataInd,
                ZCL_CommissioningRestoreStartupParametersResponse,
                status
            );

        }
        break;

        case ZCL_CommissioningResetStartupParametersEvent:
        {
            WORD storageIndex;

            /* Check if RESET_CURRENT bit is set in the option field of the
               reset startup parametes command received */
            if( pZCLEvent->pEventData[0] & Reset_Current )
            {
                if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                    ZCL_Callback_GetClusterIndex
                    (
                        METER_ENDPOINT_ID,
                        ZCL_COMMISSIONING_CLUSTER,
                        &storageIndex
                    )
                )
                {
                    /* Put the default values onto commissioning cluster attribute set */
                    PutAttributeStorage
                    (
                        storageIndex,
                        (BYTE *)&Meter_CommissioningAttributeDefaultValues,
                        sizeof(Commissioning_Cluster_Attributes )
                    );
                }
            }
            /* Check if RESET_ALL or ERASE_INDEX bit is set in the option field of the
               reset startup parametes command received */
            if( (pZCLEvent->pEventData[0] & Reset_All) || (pZCLEvent->pEventData[0] & Erase_Index))
            {
                /* If the RESET ALL bit is set in the option, all attribute set will be resetted to default values
                   If the ERASE INDEX bit is set in the option, the validity key will be erased */
                ResetSAS(pZCLEvent->pEventData[0], (pZCLEvent->pEventData[1] - 1));
            }
            /* Send Reset startup parameters response to the commissioning Device.
               The status will be SUCCESS */
            SE_SendCommissioningClusterResponse
            (
                p_dataInd,
                ZCL_CommissioningResetStartupParametersResponse,
                SUCCESS
            );
        }
        break;

        case ZCL_CommissioningRestartDeviceResponseEvent:
            /* Application Should handle Reception of restart_device_Response */
        break;

        case ZCL_CommissioningSaveStartupParametersResponseEvent:
            /* Application Should handle Reception of SaveStartupParameters_Response */
        break;

        case ZCL_CommissioningRestoreStartupParametersResponseEvent:
            /* Application Should handle Reception of RestoreStartupParameters_Response */
        break;

        case ZCL_CommissioningResetStartupParametersResponseEvent:
            /* Application Should handle Reception of ResetStartupParameters_Response */
        break;
         case ZCL_Alarm_ResetAlarmEvent :
        {
                WORD clusterId;
                memcpy
                (
                   (BYTE*)&clusterId,
                    (BYTE*)&(pZCLEvent->pEventData[1]),
                    ZCL_CLUSTER_ID_LENGTH
                );
                SE_UpdateAlarmTable
                (
                   clusterId,
                    pZCLEvent->pEventData[0],
                    INVALID_TIMESTAMP,
                    ZCL_Alarm_ResetAlarmEvent
                );
        }
        break;

        case ZCL_Alarm_ResetAllAlarmsEvent :

                SE_UpdateAlarmTable
                (
                    INVALID_CLUSTERID,
                    0x00,
                    INVALID_TIMESTAMP,
                    ZCL_Alarm_ResetAllAlarmsEvent
                );
        break;
        case ZCL_Alarm_ResetAlarmLogEvent :

                SE_UpdateAlarmTable
                (
                    INVALID_CLUSTERID,
                    0x00,
                    INVALID_TIMESTAMP,
                    ZCL_Alarm_ResetAlarmLogEvent
                );
        break;
        case ZCL_Alarm_GetAlarmEvent :
        {
            /*It indicates the Get Alarm response payload length*/
            BYTE asduLength;
            /*This array holds Get Alarm response Command */
            BYTE asdu[11];
            /*Creates Get Alarm Response*/
            asduLength = SE_CreateGetAlarmResponse
                        (
                            p_dataInd->DstEndpoint,
                            p_dataInd->asdu[1],
                            asdu
                        );
            /*App_SendData - Allocates memory and queues the data for AIL*/
            App_SendData
            (
                APS_ADDRESS_16_BIT,
                (BYTE*)&(p_dataInd->SrcAddress.ShortAddr.Val),
                p_dataInd->SrcEndpoint,
                asduLength,
                p_dataInd->ClusterId.Val,
                asdu
            );
        }
        break;
        case ZCL_Alarm_AlarmEvent :
        //Implementation Specific
        break;


        #if I_SUPPORT_CBKE == 1
        case ZCL_KeyEstInitiateKeyEstReqEvent:
        //fall through
        case ZCL_KeyEstEphemeralDataReqEvent:
        //fall through
        case ZCL_KeyEstConfirmKeyDataReqEvent:
        //fall through
        case ZCL_KeyEstTerminateKeyEstEvent:
        //fall through
        case ZCL_KeyEstInitiateKeyEstRespEvent:
        //fall through
        case ZCL_KeyEstEphemeralDataRespEvent:
        //fall through
        case ZCL_KeyEstConfirmKeyDataRespEvent:
            HandleKeyEstablishmentCommands( pZCLEvent->eventId, p_dataInd );
            break;

        #endif /*#if I_SUPPORT_CBKE*/


        default :
        break;
    }
}

/******************************************************************************
 * Function:        void App_HandlePendingData ( void );
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function handles Back ground tasks of Application
 * Note:
 *****************************************************************************/
void App_HandlePendingData_MTR ( void )
{
    TICK currentTime =  TickGet();
    /*First priority is given to Identify Cluster*/
    /*For Handling identify mode */
   
    if ( appIdentifyTimers.IdentifyTime )
    {
        /*check if one second elapses,then we need to decement identifyTime by '1'.
          This is handled by using HandleIdentifyTimeout function*/
        if (TickGetDiff( currentTime, appIdentifyTimers.IdentifyStartTime ) > ONE_SECOND )
        {
            appIdentifyTimers.IdentifyStartTime = currentTime;
            SE_HandleIdentifyTimeout( METER_ENDPOINT_ID );
        }
    }
    
    
#ifdef DONTSEND  
    if(   (TickGetDiff( currentTime, startRFDSendTime ) > ONE_SECOND * 10)  && ZigBeeStatus.flags.bits.bNetworkJoined )
    {
        startRFDSendTime = TickGet();

        asduData[0] = 0x00;  /* coordinator's address */
        asduData[1] = 0x00;
        
        /* Time interval */
        asduData[2] = 0x02;
        asduData[3] = 0x00;
        
        AppSendData
        (
            ZCL_ClientToServer | ZCL_ClusterSpecificCommand,
            ZCL_IDENTIFY_CLUSTER,
            ZCL_IdentifyCmd,
            0x02,
            asduData
        );
    }
#endif  

#if (I_SUPPORT_SCENES == 1)
    /*Check for the scenes transition timer */
    if ( scenesRecallCmdRecord.sceneTxnTime )
    {
		if (TickGetDiff( currentTime, scenesRecallCmdRecord.sceneOneSecStartTime ) > ONE_SECOND )
		{
			scenesRecallCmdRecord.sceneTxnTimeout--;

			if ( !( scenesRecallCmdRecord.sceneTxnTimeout ) )
			{
				scenesRecallCmdRecord.sceneTxnTime = FALSE;	
			}
			/* Update the one second timer with the current time */
			scenesRecallCmdRecord.sceneOneSecStartTime = currentTime;

			scenesRecallCmdRecord.txnCount++;

			/* Invoke the recall scene command */
			Handle_StoreAndRecallSceneCmd 
			( 
				scenesRecallCmdRecord.endPoint, 
				scenesRecallCmdRecord.sceneTableIndex, 
				RecallScene
 			);

		}
    }

	/* Check if the current scene is valid */
    if ( sceneValidInfo.sceneValid )
    {
		/* Invoke the recall scene command with direction as check scene validity */
		Handle_StoreAndRecallSceneCmd 
		( 
			sceneValidInfo.endPoint, 
			sceneValidInfo.sceneTableIndex, 
			CheckSceneValidity 
		);
    }
#endif /*I_SUPPORT_SCENES*/
    
    /*It is for Handling MainsVoltageDwellTripPointTime*/
    if ( appPowerConfigMainsDwellTrip.DwellTripPoint == 1 )
    {
        if (TickGetDiff( currentTime, appPowerConfigMainsDwellTrip.DwellTripPointStartTime ) >
                                            ( mainsVoltageDwellTripPointValue * ONE_SECOND ) )
        {
            appPowerConfigMainsDwellTrip.DwellTripPoint = 0x00;
            MainsVoltageDwellTripPointHandled = 0x01;
            #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
            ZCL_Callback_CheckForAlarmCondition
            (
                METER_ENDPOINT_ID,
                ZCL_MAINS_VOLTAGE,
                &zcl_PowerConfigurationClusterServerInfo
            );
            #endif
        }
    }
    /*For Handling meter simulation */
    if ( meterStatus.flags.bits.bSimulateMeterReading == 1 )
    {
      /*check if profile Interval period is expired, then updates metering cluster attributes*/
      if( TickGetDiff( currentTime,meterStatus.simulateMeterStartTime ) > ( ONE_SECOND * PROFILE_INTERVAL_PERIOD ) )
      {
            meterStatus.simulateMeterStartTime = currentTime;
            /*This function updates the metering cluster attributes*/
            SE_Meter_Update_MeterReadings();
      }
    }
    #if I_SUPPORT_REPORTING == 1
    /*if only the device has joined the network continue further*/
    if( ZigBeeStatus.flags.bits.bNetworkJoined )
    {
        GetReportingBgTasks(&ReportingBgTasks);
        /*handle report attribute command*/
        if( ReportingBgTasks.flags.bits.bValidReportEntryConfigured )
        {
            TICK currentTimeinticks =  TickGet();
            /*check if minimum time interval expired, reset the minimum start time*/
            if( ( ReportingBgTasks.consolidatedMinTime ) && ( ReportingBgTasks.consolidatedMinTime != 0xFFFF ))
            {
                if (TickGetDiff( currentTimeinticks, ReportingBgTasks.minStartTime )
                > ( ReportingBgTasks.consolidatedMinTime * ONE_SECOND ) )
                {
                    ReportingBgTasks.minReportExpired = TRUE;
                    ReportingBgTasks.minStartTime = TickGet();
                }
            }
            /*check if maximum time interval expired, reset the maximum start time*/
            if( ( ReportingBgTasks.consolidatedMaxTime ) && ( 0xFFFF != ReportingBgTasks.consolidatedMaxTime ) )
            {
                if (TickGetDiff( currentTimeinticks, ReportingBgTasks.maxStartTime)
                > ( ReportingBgTasks.consolidatedMaxTime * ONE_SECOND ) )
                {
                    ReportingBgTasks.maxReportExpired = TRUE;
                    ReportingBgTasks.maxStartTime = TickGet();
                }
            }
            /*1. if max timer expired or
              2. min timer expired or
              3. min time is zero, max time is zero
              4. only min time is zero, the below function is called to check if report attribute
                command need to be sent.
            */
            if( ReportingBgTasks.maxReportExpired || ReportingBgTasks.minReportExpired ||
            ( (!( ReportingBgTasks.consolidatedMinTime)) && ( !(ReportingBgTasks.consolidatedMaxTime )) )
            || ( ReportingBgTasks.consolidatedMinTime == 0x0000 ) )
            {
                SE_CheckToSendReportAttributeCommand();
            }
            /*If report attribute command should be sent is TRUE, then allocate memory and
            send the report attribute command*/
            if( ReportingBgTasks.flags.bits.bSendreportAttributeCommand )
            {
                SE_SendReportAttributeCommand();
            }
        }
        /*if report attribute command should be rececived at periodical intervals,
        then check if the report attribute commands are received*/
        if( ReportingBgTasks.flags.bits.bAwaitingReportAttributeCommand )
        {

            TICK currentTimeinticks =  TickGet();
            /*check if timeout has occurred*/
            if (TickGetDiff( currentTimeinticks, ReportingBgTasks.timeOutStartTime)
                > ( ReportingBgTasks.timeOut * ONE_SECOND ) &&  ( 0x0000 != ReportingBgTasks.timeOut ) )
            {
                ReportingBgTasks.timeOutExpired = TRUE;
                ReportingBgTasks.timeOutStartTime = TickGet();
            }
            /*if timeout expired, then check and update if report attribute command is received.*/
            if( TRUE == ReportingBgTasks.timeOutExpired )
            {
                /*Application should write its code here to handle the timeout condition */
                ReportingBgTasks.timeOutExpired = FALSE;
            }
        }
        /*write the background tasks back into the NVM*/
        PutReportingBgTasks(&ReportingBgTasks);
    }
    #endif   /*I_SUPPORT_REPORTING */
}

/******************************************************************************
 * Function:        void App_UpdateDataConfTable ( APP_DATA_confirm *dataConf );
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 *****************************************************************************/
void App_UpdateDataConfTable_MTR( APP_DATA_confirm *dataConf )
{

}

/******************************************************************************
 * Function:        void APP_HandleInterPANDataConfirm ( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 *****************************************************************************/
#if I_SUPPORT_STUB_APS == 1
void APP_HandleInterPANDataConfirm_MTR ( void )
{
    /**/
    currentPrimitive_MTR = NO_PRIMITIVE;
}
#endif

/******************************************************************************
 * Function:        BYTE APP_HandleInterPANDataIndication ( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         Returns TRUE if test tool needs to notified about the reception
 *                  of the packet. Otherwise, returns FALSE.
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when a Inter-PAN message is received
 *                  from the lower layer. This function is used to check what kind of
 *                  cluster message is received via Inter-PAN network. Currently,
 *                  only Price cluster command are recieved. So, condition exists
 *                  only with respect to Price cluster.
 *
 * Note:            None
 *****************************************************************************/
#if I_SUPPORT_STUB_APS == 1
BYTE APP_HandleInterPANDataIndication_MTR( void )
{
    APP_DATA_request* pAppDataReq;
    ZCL_ActionHandler pActionHandler;
    BYTE notifyTestTool = TRUE;
    BYTE status = ZCL_Success;

    currentPrimitive_MTR = NO_PRIMITIVE;

    /*If Default Response command is received, then just indicate to higher layer.
    No further processing is required. Default Response command is General Command.
    CommandIds may be same for General and Cluster Specific Commands. So, before
    checking for the commandId, commandType (Generic or Cluster Specific)
    shall be verified.*/
    if( ( !( params.INTRP_DATA_indication.ASDU[0] & ZCL_CLUSTER_SPECIFIC_COMMAND ) ) &&
        ( params.INTRP_DATA_indication.ASDU[2] == ZCL_Default_Response_Cmd ) )
    {
        return notifyTestTool;
    }

    /* Allocation of Memory is for creating Response Frame.*/
    pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);

    pActionHandler.event.pEventData = SRAMalloc(60);

    if( ( pAppDataReq != NULL ) &&
        ( pActionHandler.event.pEventData != NULL) )
    {
        if ( params.INTRP_DATA_indication.ClusterId.Val == ZCL_PRICE_CLUSTER )
        {

            ZCL_HandlePriceClusterCommand
            (
                METER_ENDPOINT_ID,
                params.INTRP_DATA_indication.ASDULength,
                params.INTRP_DATA_indication.ASDU,
                pAppDataReq->asdu,
                &pActionHandler,
                &zcl_PriceClusterServerInfo
            );

            if ( pActionHandler.action == Send_Response )
            {
                /*Send Default Response with Status as UnsupportedCommand*/
                nfree (pAppDataReq);
                status = ZCL_UnSupportedClusterCommand;
            }
            else
            {
                /*Free the allocated request. Sending Publish Price command
                separate memory is allocated. So, this allocated memory is
                never used.*/
                nfree (pAppDataReq);

                switch( pActionHandler.event.eventId )
                {
                    case ZCL_Price_PublishPriceEvent:
                        ProcessPublishPriceEvent ( pActionHandler.event.pEventData );
                        break;

                    default:
                        /*LCD can receive onlly Publish Price command.
                        If any other command, then just drop it.*/
                        status = ZCL_UnSupportedClusterCommand;
                        break;
                }
            }
        }
        else
        {
            /*Cluster Not Supported*/
            status = ZCL_Failure;
        }

        nfree( pActionHandler.event.pEventData );

        if ( status != ZCL_Success )
        {
            BYTE *pData;

            /*If the command is received and the device does not process that command,
            then Default Response should be sent. Default Response should be sent only
            if the command was received as Unicast. If received command was broadcast, then we
            should not send Default Response*/
            if ( ( params.INTRP_DATA_indication.DstAddrMode == APS_ADDRESS_16_BIT ) &&
                 ( params.INTRP_DATA_indication.DstAddress.ShortAddr.Val == BROADCAST_ADDRESS ) )
            {
                return FALSE;
            }

            /*For any packet received via Inter-PAN, the SrcAddress will always be
            Extended Address. So, the destAddreMode will be set to Extended Addressing Mode*/
            interPANDataRequest.destAddrMode = APS_ADDRESS_64_BIT;
            memcpy
            (
                ( BYTE *)&(interPANDataRequest.destAddr[0]),
                (BYTE *)&params.INTRP_DATA_indication.SrcAddress.v[0],
                MAX_ADDR_LENGTH // Inter-PAN communication happens using Extended Address
            );
            interPANDataRequest.destInterPANId = params.INTRP_DATA_indication.SrcPANId.Val;

            interPANDataRequest.profileId = params.INTRP_DATA_indication.ProfileId.Val;
            interPANDataRequest.clusterID = params.INTRP_DATA_indication.ClusterId.Val;

            /*Allocate memory for Publish Price Command Payload + the ZCL Header (3 Bytes)*/
            pData =  (BYTE *)SRAMalloc(126);
            if( pData == NULL )
            {
                /*Memory Allocation failed. So, nothing to do about it*/
                return FALSE;
            }

            interPANDataRequest.pAsdu = pData;

            /*Note: asduLength is reused here. Earlier this field was used for copying the
            destination address.*/
            interPANDataRequest.asduLength = ZCL_CreateDefaultResponseCommand
                                             (
                                                 params.INTRP_DATA_indication.ASDU,
                                                 interPANDataRequest.pAsdu,
                                                 status
                                             );

            /*ASDUHandle is a unique number. This should be given by the application. Since,
            TransactionSeqNum is also unique and changes when every packet gets transmitted,
            the same value is used as the ASDUHandle*/
            interPANDataRequest.asduHandle = params.INTRP_DATA_indication.ASDU[1];

            App_SendDataOverInterPAN();

            notifyTestTool = FALSE;
        }
    }
    return notifyTestTool;
}
#endif


/***************************************************************************************
 * Function:        static void AddConsumptionToMeterReadingByteArray
 *                  (
 *                       BYTE *pdata,
 *                       BYTE len,
 *                       BYTE consumption
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           pdata - it holds the meterReading of "len" input parameter bytes.
 *                  len - size of meterReading
 *                  consumption - It indicates the consumption of Energy of
 *                                one profile Interval period.
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview  :      This function adds the consumption value to the meterReading array(pdata).
 *
 * Note:            None
 *
 ***************************************************************************************/
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
static void AddConsumptionToMeterReadingByteArray
(
    BYTE *pdata,
    BYTE len,
    BYTE consumption
)
{
    WORD carryOver = 0;
    BYTE i;
    carryOver = (WORD)( pdata[0]+ consumption);
    pdata[0] = (BYTE)carryOver;
    for( i = 0x01 ; i < len; i++ )
    {
        carryOver = (WORD)( ( pdata[i] ) + (carryOver>>8));
        pdata[i] = (BYTE)carryOver;
    }
 }
#endif
/***************************************************************************************
 * Function:        static void SE_Meter_Update_MeterReadings( void );
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called for every profileIntervalPeriodElapses.
 *                  It will update the metering cluster attributes such as
 *                  currentSummationDelivered,currentTierSummation,
 *                  CurrentPartialProfileIntervalValue,CurrentPartialProfileIntervalStartTime
 *                  and profileInterval meterReadings.
 * Note:            None
 ***************************************************************************************/
static BYTE SE_Meter_Update_MeterReadings( void )
{
    #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    DWORD_VAL   currentTimeInSeconds;
    //elapsedTimeInfo timeInfo;
    BYTE i;
    WORD attributeId;
    WORD attributeStorageBaseLocation;
    BYTE meterReading[6];
    BYTE consumption = 0x00;
    BYTE currentPartialProfileMeterReading[3];
    DWORD currentPartialProfileStartTime;

    App_ProfileMeteringIntervals profileMeteringIntervals;
    /*Calculate the current time and extract the corresponding Tier information.*/
    currentTimeInSeconds.Val = ZCL_Callback_GetCurrentTimeInSeconds();
    /*Getting CurrentTime value in YY MM DD HR MS SS format.That will be store
     timeInfo.*/
    //ZCL_Utility_Time_ConvertSecondsToYear ( currentTimeInSeconds.Val, &timeInfo );
    /* timeInfo.hour we can get no.of Seconds elapsed from 12AM.
      By using this we can extract Tier information */
    //currentTimeInSeconds.Val = (DWORD)(timeInfo.hour);
    //currentTimeInSeconds.Val = currentTimeInSeconds.Val * NUMBER_OF_SECONDS_IN_AN_HOUR;

    /*This for loop is used to get the tier information from current time */
    for (i=0x00; i<numberOfPriceEvents; i++)
    {
        /*if ( ( currentTimeInSeconds.Val >= ListOfPriceEvents[i].startTime ) &&
             ( currentTimeInSeconds.Val < ( ListOfPriceEvents[i].startTime +
             (DWORD)( ListOfPriceEvents[i].durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE ) ) ) )
        {
            break;
        }*/
        if ( currentTimeInSeconds.Val >= ListOfPriceEvents[i].UTCStartTime )
        {
            if ( ( ListOfPriceEvents[i].durationInMinutes == DURATION_UNTIL_CHANGED ) ||
                 ( currentTimeInSeconds.Val < ( ListOfPriceEvents[i].UTCStartTime +
                   (DWORD)( ListOfPriceEvents[i].durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE ) ) ) )
            {
                break;
            }
        }
    }
    /*Switch case updates the attributeId with corresponding TierAttributeId*/
    switch( ListOfPriceEvents[i].tierInfo )
    {
        case 1:
            attributeId = ZCL_METERING_CURRENT_TIER1_SUMMATION_DELIVERED;
        break;
        case 2:
            attributeId = ZCL_METERING_CURRENT_TIER2_SUMMATION_DELIVERED;
        break;
        case 3:
            attributeId = ZCL_METERING_CURRENT_TIER3_SUMMATION_DELIVERED;
        break;
        case 4:
            attributeId = ZCL_METERING_CURRENT_TIER4_SUMMATION_DELIVERED;
        break;
        case 5:
            attributeId = ZCL_METERING_CURRENT_TIER5_SUMMATION_DELIVERED;
        break;
        case 6:
            attributeId = ZCL_METERING_CURRENT_TIER6_SUMMATION_DELIVERED;
        break;
        default :
            return ZCL_Failure;
    }

    /* Generate random number between 0 to 15. This rand() function is used as given by
    <stdlib.h>.This random number indicates the consumption of energy for one profile
    Interval period */
    //consumption = rand()%15;
    consumption = TMR2%255;

    /*This condition is used to get NVM index for Metering cluster */
    if ( EP_CLUSTERID_NOT_FOUND_IN_NVM != ZCL_Callback_GetClusterIndex
                                            (
                                                METER_ENDPOINT_ID,
                                                ZCL_METERING_CLUSTER,
                                                &attributeStorageBaseLocation
                                            ) )
   {
        BYTE    attributeStorageOffset;
        BYTE    attributeDataTypeLength;

        /*This function is used to get the index of the NVM where the Metering cluster
        attributes are stored*/
        attributeDataTypeLength = GetAttributeDetails
                                    (
                                        zcl_MeteringClusterServerInfo.numberOfAttributes,
                                        attributeId,
                                        (ZCL_AttributeInfo *)(zcl_MeteringClusterServerInfo.pListofAttibuteInfo),
                                        &attributeStorageOffset
                                    );

        /*Here attributeId is one of the currenttiersummation attribute.
        This is updated from above switch case Read the Attribute Value from NVM.
        Read value is updated in meterReading array */
        ZCL_Callback_GetAttributeValue
        (
            attributeStorageBaseLocation + attributeStorageOffset,
            meterReading,
            attributeDataTypeLength
        );

        /*Adding the current profile interval Consumption value to the currentTierSummationValue*/
        AddConsumptionToMeterReadingByteArray(meterReading,0x06,consumption);

        /*Updated currentTierSummationValue is storing into NVM */
        ZCL_Callback_PutAttributeValue
        (
            attributeStorageBaseLocation + attributeStorageOffset,
            meterReading,
            attributeDataTypeLength
        );

        /*This function is used to get the index of the NVM where the Metering cluster
        attributes are stored*/
        attributeDataTypeLength = GetAttributeDetails
                                    (
                                        zcl_MeteringClusterServerInfo.numberOfAttributes,
                                        ZCL_METERING_CURRENT_SUMMATION_DELIVERED,
                                        (ZCL_AttributeInfo *)(zcl_MeteringClusterServerInfo.pListofAttibuteInfo),
                                        &attributeStorageOffset
                                    );

        /*Reading the CurrentSummation Deliverd attribute value from NVM */
        ZCL_Callback_GetAttributeValue
        (
            attributeStorageBaseLocation + attributeStorageOffset,
            meterReading,
            attributeDataTypeLength
        );

        /*Adding the current profile interval Consumption value to the currentSummationDeliverdValue*/
        AddConsumptionToMeterReadingByteArray( meterReading,0x06,consumption);

        /*Updated currentrSummationDeliverdValue is storing into NVM */
        ZCL_Callback_PutAttributeValue
        (
            attributeStorageBaseLocation + attributeStorageOffset,
            meterReading,
            attributeDataTypeLength
        );

         /*This function is used to get the index of the NVM where the Metering cluster
        attributes are stored*/
        attributeDataTypeLength = GetAttributeDetails
                                  (
                                    zcl_MeteringClusterServerInfo.numberOfAttributes,
                                    ZCL_METERING_CURRENT_PARTIAL_PROF_INT_VALUE_DELIVERED,
                                     (ZCL_AttributeInfo *)(zcl_MeteringClusterServerInfo.pListofAttibuteInfo),
                                    &attributeStorageOffset
                                  );

         /*Reading the CurrentPartialProfileIntervalDeliverd value from NVM */
        ZCL_Callback_GetAttributeValue
        (
            attributeStorageBaseLocation + attributeStorageOffset,
            currentPartialProfileMeterReading,
            attributeDataTypeLength
        );

        /*Before Updating CurrentPartialProfileIntervalDeliverd value,we need to copy the current value
          into profileIntervalArray.This array is used to send GetProfileResponse Command*/
        memcpy
        (
            (BYTE*)&(profileMeteringIntervals.metringInterval),
             currentPartialProfileMeterReading,
             0x03
        );
        /*Consumption is generated by using rand() function.Assuming,Generated value as
          one byte value.So lower byte is intialized with consumption */
        currentPartialProfileMeterReading[0] = consumption;
        currentPartialProfileMeterReading[1] = 0x00;
        currentPartialProfileMeterReading[2] = 0x00;
        /*Updated CurrentPartialProfileIntervalDeliverd attribute value is storing into NVM */
        ZCL_Callback_PutAttributeValue
        (
            attributeStorageBaseLocation + attributeStorageOffset,
            currentPartialProfileMeterReading,
            attributeDataTypeLength
        );

          /*This function is used to get the index of the NVM where the Metering cluster
        attributes are stored*/
        attributeDataTypeLength = GetAttributeDetails
                                  (
                                    zcl_MeteringClusterServerInfo.numberOfAttributes,
                                    ZCL_METERING_CURRENT_PARTIAL_PROF_INT_START_TIME_DELIVERED,
                                     (ZCL_AttributeInfo *)(zcl_MeteringClusterServerInfo.pListofAttibuteInfo),
                                    &attributeStorageOffset
                                  );
         /*Read the Current Partial profile Interval value start Time*/
        ZCL_Callback_GetAttributeValue
        (
            attributeStorageBaseLocation + attributeStorageOffset,
            (BYTE*)&( currentPartialProfileStartTime ),
            attributeDataTypeLength
        );
        profileMeteringIntervals.utcTime = currentPartialProfileStartTime;
        currentPartialProfileStartTime = ZCL_Callback_GetCurrentTimeInSeconds();
        /*Updated CurrentPartialProfileIntervalStartTimeDeliverd is storing into NVM */
        ZCL_Callback_PutAttributeValue
        (
            attributeStorageBaseLocation + attributeStorageOffset,
            (BYTE*)&(currentPartialProfileStartTime),
            attributeDataTypeLength
        );
        /*This function updates meter profile Interval values using reverse chronological order*/
        if( noOfTimesProfileIntervalElapses++ )
        {
            /*This function updates meter profile Interval values using reverse chronological order*/
            UpdateProfileIntervalValues(&profileMeteringIntervals);
            if( noOfTimesProfileIntervalElapses == 0xFF)
            {
              noOfTimesProfileIntervalElapses = 0x01;
            }
        }

   }
   #endif
   return ZCL_Success;
}

/***************************************************************************************
 * Function:        static void UpdateProfileIntervalValues(App_ProfileMeteringIntervals *profileMeteringIntervals)
 *
 * PreCondition:    None
 *
 * Input:           profileMeteringIntervals - it holds ProfileMeteringIntervalValue & corresponding
 *                                             ProfileMeteringIntervalValueStartTime
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function updates ProfileMeteringIntervalArray(testMeterProfiles).It
 *                  is used to send GetProfileResponseCommand.
 *
 * Note:            None
 *
 *
 ***************************************************************************************/
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
static void  UpdateProfileIntervalValues(App_ProfileMeteringIntervals *profileMeteringIntervals)
{
    BYTE i;
    /*Always received profileMeteringIntervals should be stored at index'0'.
      Before storing corresponding Index values needs be shifted to the other index.Like,
      0->1,1->2,2->3,3->4........
      Last Index value will be removed from array*/
    /*testMeterProfiles array values are arranged according reverse chronological order */
    for ( i = METER_NUM_PROFILES_SUPPORTED-1; i > 0; i -- )
    {
        memcpy
        (
            (BYTE*)&(testMeterProfiles[i].utcTime),
            (BYTE*)&(testMeterProfiles[i-1].utcTime),
            SIZE_OF_UTC_TIME
        );
        memcpy
        (
            (BYTE*)&(testMeterProfiles[i].metringInterval[0]),
            (BYTE*)&(testMeterProfiles[i-1].metringInterval[0]),
            SIZE_OF_PROFILE_INTERVAL_VALUE
        );
    }
    memcpy
    (
        (BYTE*)&(testMeterProfiles[0].utcTime),
        (BYTE*)&(profileMeteringIntervals->utcTime),
        SIZE_OF_UTC_TIME
    );
    memcpy
    (
        (BYTE*)&(testMeterProfiles[0].metringInterval[0]),
        (BYTE*)&(profileMeteringIntervals->metringInterval[0]),
        SIZE_OF_PROFILE_INTERVAL_VALUE
    );
}
#endif
/******************************************************************************
* Function:         void ProcessPublishPriceEvent ( BYTE *pAsdu )
*
* PreCondition:     None
*
* Input:            pAsdu - Specifies the starting location of payload
*                          of the Publish Price Command
*
* Output:           None
*
* Return :          None
*
* Side Effects:     None
*
* Overview:         This function is called when Publish Price Event needs to
*                   be handled by the application. This function processes the
*                   Publish Price Command and updates the global structure
*                   PublishPriceInfo. This structure is used for updating the
*                   Price events table entry.
*
* Note:             pAsdu points to Start of the Publish Price Command Payload
*                   It does not contain the ZCL Header
*****************************************************************************/
void ProcessPublishPriceEvent ( BYTE *pAsdu )
{
    BYTE rateLabelSize;

    /*Copy the Provider ID*/
    memcpy
    (
        (BYTE *)&PublishPriceInfo.ProviderId,
        pAsdu,
        0x04 /*Provider Id is of 4 bytes*/
    );

    /*Update the Rate Label Length*/
    rateLabelSize = pAsdu[4];
    memcpy
    (
        PublishPriceInfo.RateLabel,
        &( pAsdu[4] ),
        (rateLabelSize + 1) /*Rate label is a array of 12 octets. First byte specifies
                              the length and remaining 11 bytes are the label
                              1 is added to the rateLabelSize is because of the first
                              byte.*/
    );

    /*Copy the Remaining fields*/
    memcpy
    (
        (BYTE *)&PublishPriceInfo.IssuerEventId,
        /*Length of ProviderId (4 Bytes) + Lenght Of RateLable (First Byte (1 Byte) + Actual Bytes (rateLabelSize Bytes))*/
        &( pAsdu[ 4 + rateLabelSize + 1 ] ),
        PUBLISH_PRICE_CMD_LENGTH_FROM_ISSUER_EVENT_ID
    );

    /*Update the Price Events Table Entry*/
    App_UpdatePriceEvents_MTR();
}

/******************************************************************************
* Function:        void App_UpdatePriceEvents ( void )
*
* PreCondition:    None
*
* Input:           None
*
* Output:          None
*
* Return :         None
*
* Side Effects:    None
*
* Overview:        This function is called whenever the Price Event entry needs
*                  to be modified. This function loops through the Price Events
*                  and modifies the corresponding entry.
*
* Note:            None
*****************************************************************************/
void App_UpdatePriceEvents_MTR ( void )
{
    BYTE tier;
    BYTE rateLabel;
    elapsedTimeInfo elapsedTime;
    DWORD priceEventStartTime;

    /*Extract the Tier value. We are assuming that Tiers should be in
    a sequencial order (1,2,3,4,5,6). There cannot be a gap (1,3,8,10,2,4).
    This is because, Tier value is used to check whether a new entry needs
    to be created or the existing entry needs to be modified*/
    tier = PublishPriceInfo.PriceTrailingDigitAndPriceTier & 0x0F;
    if ( ( tier <= 0x00 ) || ( tier > numberofTiersSupported ) )
    {
        return;
    }

    /*Update the Event Id.
    Tier -1 is used because, the table index starts from zero but the Tier value
    starts from 1. To update the Tier 1 values, we should use the 0th index.*/
    ListOfPriceEvents[tier - 1].issuerEventId = PublishPriceInfo.IssuerEventId;

    /*Update the UTCStartTime*/
    if ( PublishPriceInfo.StartTime )
    {
        /*If the StartTime is non zero, means specific time frame is given for this
        tier. So, we should use the StartTime as specified in the received frame*/
        ListOfPriceEvents[tier - 1].UTCStartTime = PublishPriceInfo.StartTime;
    }
    else
    {
        /*If the StartTime is zero, means the tier starts from now. So, we should
        use the current time as the StartTime.*/
        ListOfPriceEvents[tier - 1].UTCStartTime = ZCL_Callback_GetCurrentTimeInSeconds();
    }

    /*Update the DurationInMinutes*/
    ListOfPriceEvents[tier - 1].durationInMinutes = PublishPriceInfo.DurationInMinutes;

    /*Update the StartTime and End Time*/
    ZCL_Utility_Time_ConvertSecondsToYear( ListOfPriceEvents[tier - 1].UTCStartTime, &elapsedTime );
    priceEventStartTime = (DWORD)( ( ((DWORD)elapsedTime.hour) * NUMBER_OF_SECONDS_IN_AN_HOUR ) +
                                 ( ((DWORD)elapsedTime.minute) * NUMBER_OF_SECONDS_IN_A_MINUTE ) + ( elapsedTime.second ) );

    ListOfPriceEvents[tier - 1].startTime = priceEventStartTime;
    //ListOfPriceEvents[tier - 1].endTime = (DWORD)( priceEventStartTime + ( PublishPriceInfo.DurationInMinutes * 60 ) );

    /*This is required if new tier value is being added.*/
    ListOfPriceEvents[tier - 1].tierInfo = tier;

    /*Update the Rate Label*/
    rateLabel = PublishPriceInfo.RateLabel[0];
    memcpy
    (
        (BYTE*)&( ListOfPriceEvents[tier - 1].rateLabel[0] ),
        (BYTE*)&( PublishPriceInfo.RateLabel[0] ),
        rateLabel + 1
    );

    /*Update the Price*/
    ListOfPriceEvents[tier - 1].price = PublishPriceInfo.Price;

    /*Update the PriceTrailingDigit*/
    ListOfPriceEvents[tier - 1].priceTrailingDigitAndPriceTier = PublishPriceInfo.PriceTrailingDigitAndPriceTier;

    numberofTiersSupported = ( PublishPriceInfo.NumOfPriceTiersAndRegisterTier >> GET_HIGHER_NIBBLE );
    numberOfPriceEvents = numberofTiersSupported;

    /*Check whether two or more events are merged. If merged, then we have to remove the tier information.*/
    ValidatePriceEvents();
}

/******************************************************************************
* Function:        static void ValidatePriceEvents ( void )
*
* PreCondition:    None
*
* Input:           None
*
* Output:          None
*
* Return :         None
*
* Side Effects:    None
*
* Overview:        This function is called when Price Events needs to verified
*                  for Gaps. This function reagrranges the Price Events by leaving
*                  gaps, wherever necessary, as notified by Publish Price Command.
*
* Note:            None
*****************************************************************************/
static void ValidatePriceEvents ( void )
{
    BYTE i, j;

    i = 0x00;
    while ( i != numberOfPriceEvents )
    {
        /*Check whether valid entry exists in the Price Tier Table*/
        if ( ListOfPriceEvents[i].tierInfo != 0xFF )
        {
            /*Compare with the next entry*/
            for (j=i+1; j<numberOfPriceEvents; j++)
            {
                /*Check whether the next entry is valid*/
                if ( ListOfPriceEvents[j].tierInfo != 0xFF )
                {
                    /*Check whether this entry was merged with the previous one.*/
                    if ( ( ListOfPriceEvents[i].UTCStartTime < ListOfPriceEvents[j].UTCStartTime ) &&
                         ( ListOfPriceEvents[j].UTCStartTime < ( ListOfPriceEvents[i].UTCStartTime +
                                         ( (DWORD)ListOfPriceEvents[i].durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE ) ) ) )
                    {
                        /*Reset the Entry*/
                        memset
                        (
                            &ListOfPriceEvents[j],
                            0xFF,
                            sizeof(PriceEvents)
                        );
                        ListOfPriceEvents[j].rateLabel[0] = 0x0B;
                    }
                    else
                    {
                        /*Else this entry is not merged with the previous one. So, break.*/
                        break;
                    }
                }
            }
        }
        i++;
    }
}

/******************************************************************************
 * Function:        App_Message_Tick
 *
 * PreCondition:
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:
 *
 * Return Value:
 *
 * Overview:
 *
 * Note:
 *****************************************************************************/
void App_Message_Tick ( void )
{
    /*check this , also check if user has pressed any button. If pressed the
    send. needs correction*/
    if( displayMessagesTable.confirmationRequired )
    {
        BYTE asduLength;
        BYTE asdu[40];

        /*reset the message confirmation required*/
        displayMessagesTable.confirmationRequired = 0x00;

        asduLength =    ZCL_Message_CreateMessageConfirmation
                        (
                            asdu,
                            displayMessagesTable.sequenceNumber,
                            &( displayMessagesTable.displayMessage.messageId)
                        );
        App_SendData
        (
            APS_ADDRESS_16_BIT,
            (BYTE *)&displayMessagesTable.destaddress,
            displayMessagesTable.destEndPoint,
            asduLength,
            ZCL_MESSAGING_CLUSTER,
            asdu
        );
    }
}

/***************************************************************************************
 * Function:        void App_PriceInit()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the Price Events
 *
 * Note:            None
 ***************************************************************************************/
void App_PriceInit_MTR()
{
    BYTE i;
    elapsedTimeInfo defaultTime;
    DWORD currentUTCTime;

    numberOfPriceEvents = MAX_PRICE_EVENTS;
    numberofTiersSupported = MAX_TIERS_SUPPORTED;

    defaultTime.year = DEFAULT_YEAR;
    defaultTime.month = DEFAULT_MONTH;
    defaultTime.day = DEFAULT_DAY;
    defaultTime.hour = DEFAULT_HOUR;
    defaultTime.minute = DEFAULT_MINUTE;
    defaultTime.second = DEFAULT_SECOND;

    currentUTCTime = ZCL_Utility_Time_ConvertYearToSeconds(&defaultTime);

    for( i = 0x00; i < numberOfPriceEvents; i++)
    {
        ListOfPriceEvents[i].UTCStartTime = currentUTCTime;
        currentUTCTime = currentUTCTime + (DWORD)( ListOfPriceEvents[i].durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE );
    }
}
/***************************************************************************************
 * Function:        void App_Meter_ProfileIntervalTableInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the ProfileIntervalTable to default values.
 *
 * Note:            None
 ***************************************************************************************/
void App_Meter_ProfileIntervalTableInit(void)
{
    BYTE i;
    for( i = 0x00; i < sizeof(testMeterProfiles)/sizeof(App_ProfileMeteringIntervals); i++)
    {
        memset
        (
            (BYTE*)&(testMeterProfiles[i]),
            0x00,
            sizeof(App_ProfileMeteringIntervals)
        );
    }
}

