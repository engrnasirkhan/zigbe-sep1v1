/*********************************************************************
 *
 *                 SE ESP
 *
 *********************************************************************
 * FileName:        SE_ESP.c
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

/*Generic Files*/
#include <string.h>
#include "generic.h"
#include "sralloc.h"
#include "HAL.h"
#include "zNVM.h"

/*ZCL Related Files*/
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
#if I_SUPPORT_CBKE == 1
    #include "ZCL_Key_Establishment.h"
#endif

#if defined(I_SUPPORT_GROUP_ADDRESSING)
	#include "ZCL_Groups.h"
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)
	#include "ZCL_Scenes.h"
#endif /*I_SUPPORT_SCENES*/

/* SE Related Files */
#include "SE_Interface.h"
#include "SE_Profile.h"
#include "SE_ESP.h"
#include "SE_Display.h"

#if I_SUPPORT_CBKE == 1
    #include "eccapi.h"
    #include "SE_CBKE.h"
#endif /*I_SUPPORT_CBKE*/

/* Libraries */
#if defined(__C30__)
    #include "TLS2130.h"
#else
    #include "lcd PIC32.h"
#endif


/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/
#define GET_HIGHER_NIBBLE                               0x04

/*****************************************************************************
   Customizable Macros
 *****************************************************************************/
#define GET_PROFILE_RESP_FIXED_FIELDS_LEN               0x0A

#define MIRROR_RSP_END_POINT_INDEX                      0x03

/*****************************************************************************
   Data Structures
 *****************************************************************************/
/* None */

/******************************************************************************
        Variable Definations
******************************************************************************/
extern ZIGBEE_PRIMITIVE currentPrimitive_ESP;
extern ZIGBEE_PRIMITIVE currentPrimitive_MTR;
extern ZIGBEE_PRIMITIVE    *currentPrimitive_PTR;

extern ALARM_AlarmTable AlarmEntries[];


#if I_SUPPORT_CBKE == 1
/*Supports CBKE key establishment suite*/
ROM _ZCL_KeyEstClusterAttributes ESP_KeyEstAttributeDefaultValue = { 0x0001 };
#endif /*I_SUPPORT_CBKE*/

ROM ZCL_IdentifyClusterAttributes ESP_IdentifyAttributeDefaultValue = { 0x0000 };

#ifdef I_SUPPORT_GROUP_ADDRESSING
/* Supports the group names */
ROM ZCL_GroupsClusterAttributes ESP_GroupsAttributeDefaultValue = { 0x80 };
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

ROM ZCL_TimeClusterAttribute ESP_TimeAttributeDefaultValue =
{
    /*Time Attribute*/
    0x0000 ,
    /*Master, Master for Zone and DST*/
    0x05
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    ,
    /*Time zone*/
    0x00000000,
    /*DST start*/
    0x00000000,
    /*DST end*/
    0x00000000,
    /*DST shift*/
    0x00000000,
    /*standard time*/
    0x00000000,
    /*local time*/
    0x00000000
#endif /*I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES*/
};

ROM ZCL_BasicClusterAttributes ESP_BasicAttributeDefaultValue =
{
    /* ZCLVersion */
    0x04,
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1

    /* ApplicationVersion */
    0x05,

    /* StackVersion */
    0x06,

    /* HWVersion */
    0x07,

    /* ManufacturerName */
   {0x09,'M','i','c','r','o','c','h','i','p'},

    /* ModelIdentifier */
    {0x06,'M','o','d','e','l','1'},

    /* DateCode */
    {0x06,'1','2','0','3','0','9'},
#endif /* I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES */

    /* PowerSource */
    0x00
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    ,
    /* LocationDescription */
    {0x0a,'R','a','l','e','i','g','h',' ','N','C'},

    /* PhysicalEnvironment */
    0x01,

    /* DeviceEnabled */
    0x01,

    /* AlarmMask */
    0x00
#endif /*I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES*/
};

/*Default value for the Price Cluster Attributes*/
ROM ZCL_PriceClusterAttribute ESP_PriceAttributeDefaultValue =
{
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    /* Tier1 */
    {0x06,'T','i','e','r',' ','1'},

    /* Tier2 */
    {0x06,'T','i','e','r',' ','2'},

    /* Tier3 */
    {0x06,'T','i','e','r',' ','3'},

    /* Tier4 */
    {0x06,'T','i','e','r',' ','4'},

    /* Tier5 */
    {0x06,'T','i','e','r',' ','5'},

    /* Tier6 */
    {0x06,'T','i','e','r',' ','6'},
#endif /* I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES */
};

#if (I_SUPPORT_SCENES == 1)
/*Default value for scenes cluster*/
ROM ZCL_ScenesClusterAttributes ESP_ScenesAttributeDefaultValue = 
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

/*Default value for the Power Configuration Cluster Attributes*/
ROM ZCL_PowerConfigurationClusterAttributes ESP_PowerConfigurationAttributeDefaultValue =
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
ROM ZCL_AlarmClusterAttributes ESP_AlarmAttributeDefaultValue =
{
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    /*Alarm Count*/
     0x0000
#endif
};

ROM ZCL_MeteringClusterAttributes ESP_MeteringAttributeDefaultValue =
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
    0x00,
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    0x00,
    0x00,
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
    {0x00, 0x00, 0x00}
#endif
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    ,
    /* Load Profile Configuration Attribute Set */
    0x18
#endif
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    ,
    /* Supply Limit Attribute Set */
    {0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00},
    0x00,
    0x00
#endif
};

/*It consists of ESP cluster Defination*/
ROM ZCL_ClusterDefinition* pESPClusterServerDefinition[] =
{
    &zcl_IdentifyClusterServerDefinition ,
    &zcl_BasicClusterServerDefinition
    #ifdef I_SUPPORT_GROUP_ADDRESSING	
    ,
	    &zcl_GroupsClusterServerDefinition
	#endif /*I_SUPPORT_GROUP_ADDRESSING*/
	
	#if (I_SUPPORT_SCENES == 1)	
	,
		&zcl_ScenesClusterServerDefinition
	#endif /*I_SUPPORT_SCENES*/
	,
	&zcl_TimeClusterServerDefinition,
    &zcl_PriceClusterServerDefinition,
    &zcl_DRLCClusterServerDefinition,
    &zcl_MessageClusterServerDefinition,
    &zcl_MeteringClusterServerDefinition,
    &zcl_PowerConfigurationClusterServerDefinition,
    &zcl_AlarmClusterServerDefinition
    #if I_SUPPORT_CBKE == 1
    ,
        &zcl_KeyEstClusterServerDefinition
    #endif /*I_SUPPORT_CBKE*/
};

/*It consists of ESP cluster Defination*/
ROM ZCL_ClusterDefinition* pESPClusterClientDefinition[] =
{
    &zcl_IdentifyClusterClientDefinition,
    &zcl_BasicClusterClientDefinition
    #ifdef I_SUPPORT_GROUP_ADDRESSING
    ,
	    &zcl_GroupsClusterClientDefinition
	#endif /*I_SUPPORT_GROUP_ADDRESSING*/
	
	#if (I_SUPPORT_SCENES == 1)
	,
		&zcl_ScenesClusterClientDefinition
	#endif /*I_SUPPORT_SCENES*/
	,
	&zcl_PowerConfigurationClusterClientDefinition,
    &zcl_AlarmClusterClientDefinition,
    &zcl_PriceClusterClientDefinition,
    &zcl_MeteringClusterClientDefinition,
    &zcl_CommissioningClusterClientDefinition
    #if I_SUPPORT_CBKE == 1
    ,
        &zcl_KeyEstClusterClientDefinition
    #endif /*I_SUPPORT_CBKE*/
};

/*ROM holds ESP supported client and server clusters on which endpoint.*/
ZCL_DeviceInfo ESP_DeviceServerinfo =
{
        ESP_ENDPOINT_ID,
        NUMBER_OF_SERVER_CLUSTERS_SUPPORTED_ESP,
        pESPClusterServerDefinition
};

/*SE Related*/
ZCL_DeviceInfo *pAppListOfDeviceServerInfo_ESP[] =
{
    &ESP_DeviceServerinfo
};

/*ROM holds ESP supported client and server clusters on which endpoint.*/
ZCL_DeviceInfo ESP_DeviceClientinfo =
{
        ESP_ENDPOINT_ID,
        NUMBER_OF_CLIENT_CLUSTERS_SUPPORTED_ESP,
        pESPClusterClientDefinition
};

/*SE Related*/
ZCL_DeviceInfo *pAppListOfDeviceClientInfo_ESP[] =
{
    &ESP_DeviceClientinfo
};

App_AttributeStorage  App_AttributeStorageTable_ESP[] =
{
   { ESP_ENDPOINT_ID, ZCL_IDENTIFY_CLUSTER,
     sizeof(ZCL_IdentifyClusterAttributes),
     0xFFFF, &ESP_IdentifyAttributeDefaultValue },

   { ESP_ENDPOINT_ID, ZCL_BASIC_CLUSTER,
     sizeof(ZCL_BasicClusterAttributes),
     0xFFFF, &ESP_BasicAttributeDefaultValue    },

	#ifdef I_SUPPORT_GROUP_ADDRESSING
	   { ESP_ENDPOINT_ID, ZCL_GROUPS_CLUSTER,
	     sizeof(ZCL_GroupsClusterAttributes),
	     0xFFFF, &ESP_GroupsAttributeDefaultValue },
	#endif /*I_SUPPORT_GROUP_ADDRESSING*/

    #if (I_SUPPORT_SCENES == 1)
	    { ESP_ENDPOINT_ID, ZCL_SCENES_CLUSTER,
	      sizeof(ZCL_ScenesClusterAttributes),
	      0xFFFF, &ESP_ScenesAttributeDefaultValue  },
	#endif /*I_SUPPORT_SCENES*/
	
   { ESP_ENDPOINT_ID, ZCL_TIME_CLUSTER,
     sizeof(ZCL_TimeClusterAttribute),
     0xFFFF, &ESP_TimeAttributeDefaultValue     },

   { ESP_ENDPOINT_ID, ZCL_PRICE_CLUSTER,
     sizeof(ZCL_PriceClusterAttribute),
     0xFFFF, &ESP_PriceAttributeDefaultValue    },

   { ESP_ENDPOINT_ID, ZCL_METERING_CLUSTER,
     sizeof(ZCL_MeteringClusterAttributes),
     0xFFFF, &ESP_MeteringAttributeDefaultValue },

   { ESP_ENDPOINT_ID, ZCL_POWER_CONFIGURATION_CLUSTER,
     sizeof(ZCL_PowerConfigurationClusterAttributes),
     0xFFFF, &ESP_PowerConfigurationAttributeDefaultValue },

   { ESP_ENDPOINT_ID, ZCL_ALARM_CLUSTER,
     sizeof(ZCL_AlarmClusterAttributes),
     0xFFFF, &ESP_AlarmAttributeDefaultValue }
    /*server key establishment cluster*/
    
#if I_SUPPORT_CBKE == 1
    ,
   { ESP_ENDPOINT_ID, ZCL_KEY_ESTABLISHMENT_CLUSTER,
     sizeof(_ZCL_KeyEstClusterAttributes),
     0xFFFF, &ESP_KeyEstAttributeDefaultValue }
#endif /*#if I_SUPPORT_CBKE */
    /*client cluster storage for key establishment cluster*/
#if I_SUPPORT_CBKE == 1
    ,
   { ESP_ENDPOINT_ID, ZCL_KEY_ESTABLISHMENT_CLUSTER,
     sizeof(_ZCL_KeyEstClusterAttributes),
     0xFFFF, &ESP_KeyEstAttributeDefaultValue }
#endif /*#if I_SUPPORT_CBKE */
};

/*Default value used for sending the Publish Price Command.
Any of these fields can be changed based on the need basis,
before sending the Publish Price Command. This is used for
Testing purpose.*/
ZCL_Price_PublishPriceInfo PublishPriceCommandPayloadDefaultValue =
{
    0x11223344, /* ProviderId - Unsigned 32 bit Integer*/
    { 0x0B, 'M', 'i', 'c', 'r', 'o', 'c', 'h', 'i', 'p', 'M', 'T' }, /* RateLabel[12] - Octet String */
    0x00000000, /*IssuerEventId*/
    0x00000001, /*CurrentTime*/
    0x00, /*UnitOfMeasure*/
    0x0348, /*Currency*/
    0x01, /*PriceTrailingDigitAndPriceTier*/
    0x01, /*NumOfPriceTiersAndRegisterTier*/
    0x0000000F, /*StartTime*/
    0x0001, /*DurationInMinutes*/
    0x00000001, /*Price*/
    0xFF, /*PriceRatio*/
    0xFFFFFFFF, /*GenerationPrice*/
    0xFF, /*GenerationPriceRatio*/
    0xFFFFFFFF, /*AlternateCostDelivered*/
    0xFF, /*AlternateCostUnit*/
    0xFF /*AlternateCostTrailingDigit*/
};

/*Default value for PublishPriceCmdTxDetails Structure*/
PublishPriceCmdTxDetails PublishPriceRequestFrameValue =
{
    0x02,
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    0xFFFF,
    0x00,
    0x01,
    0x00,
    0x00,
    0x00,
    0x00000000,
    &PublishPriceCommandPayloadDefaultValue
};

/*This table is used to Store the Price events. This table currently
intialized with the default values. This table entries may change
when user wants to change some of the tier information.
This table entry should be set by the Utility and initialized with
the Utility specified value. */
PriceEvents ListOfPriceEvents[MAX_PRICE_EVENTS] =
{
    /*EventID, UTCStartTime, DurationInMinutes, StartTime, Tier, RateLabel, Price, PriceTrailingDigitAndPriceTier*/
    { 0x00000001, 0x11CC4000, 0x00F0, 0,     1, {0x06, 'T', 'i', 'e', 'r', ' ', '1'}, 0x00000001, 0x01 },
    { 0x00000001, 0x11CC7840, 0x00F0, 14400, 2, {0x06, 'T', 'i', 'e', 'r', ' ', '2'}, 0x00000002, 0x02 },
    { 0x00000001, 0x11CCB080, 0x00F0, 28800, 3, {0x06, 'T', 'i', 'e', 'r', ' ', '3'}, 0x00000003, 0x03 },
    { 0x00000001, 0x11CCE8C0, 0x00F0, 43200, 4, {0x06, 'T', 'i', 'e', 'r', ' ', '4'}, 0x00000004, 0x04 },
    { 0x00000001, 0x11CD2100, 0x00F0, 57600, 5, {0x06, 'T', 'i', 'e', 'r', ' ', '5'}, 0x00000005, 0x05 },
    { 0x00000001, 0x11CD5940, 0x00F0, 72000, 6, {0x06, 'T', 'i', 'e', 'r', ' ', '6'}, 0x00000006, 0x06 },
};
ZCL_Price_PublishPriceInfo PublishPriceInfo;

ZCL_Message_DisplayMessageCmdPayload ZCL_Message_DisplayMessageTable =
{
    /*Message Id*/
    0x11111110,
    /*Message Confirmation required, normal tramsmission only*/
    MESSAGE_CONFIRMATION_REQUIRED,
    0x00000000,
    0x00F0,
    { 0x0E,'M','I','C','R','O','C','H','I','P','-','M','C','H','P'}
};


displayMessages displayMessagesTable =
{
    0x0000,
    0x01,
    &ZCL_Message_DisplayMessageTable,
    0x00
};

Meter_Status mirrorStatus = {FALSE, INVALID_END_POINT, FALSE};

/*Default value used for sending the Load Control Event Command.
Any of these fields can be changed based on the need basis,
before sending the Load Control Event Command. This is used for
Testing purpose.*/

DRLC_LoadControlEvent LoadControlEventCommandPayloadDefaultValue[MAX_LOAD_CONTROL_EVENTS] =
{
    {
        0x00000001, /* issuerEventID */
        0x0080,     /* deviceClass   */
        0x03,       /* utilityEnrolementGroup */
        0x00000000, /* startTime */
        0x0001,     /* durationInMinutes */
        0x07,       /* criticalityLevel */
        0x30,       /* coolingTempOffset */
        0x20,       /* heatingTempOffset */
        0x8000,     /* coolingTempSetPoint */
        0x8000,     /* heatingTempSetPoint */
        0x80,       /* avgLoadAdjustmentPercentage */
        0xFF,       /* dutyCycle */
        0x00        /* eventControl */
    },
    {
        0x00000002, /* issuerEventID */
        0x0080,     /* deviceClass   */
        0x03,       /* utilityEnrolementGroup */
        0x00000000, /* startTime */
        0x0001,     /* durationInMinutes */
        0x07,       /* criticalityLevel */
        0x30,       /* coolingTempOffset */
        0x20,       /* heatingTempOffset */
        0x8000,     /* coolingTempSetPoint */
        0x8000,     /* heatingTempSetPoint */
        0x80,       /* avgLoadAdjustmentPercentage */
        0xFF,       /* dutyCycle */
        0x00        /* eventControl */
    },
    {
        0x00000003, /* issuerEventID */
        0x0080,     /* deviceClass   */
        0x03,       /* utilityEnrolementGroup */
        0x00000000, /* startTime */
        0x0001,     /* durationInMinutes */
        0x07,       /* criticalityLevel */
        0x30,       /* coolingTempOffset */
        0x20,       /* heatingTempOffset */
        0x8000,     /* coolingTempSetPoint */
        0x8000,     /* heatingTempSetPoint */
        0x80,       /* avgLoadAdjustmentPercentage */
        0xFF,       /* dutyCycle */
        0x00        /* eventControl */
    }
};

/*Default value for Load Control Event Command Structure*/
LoadControlEventCmdTxDetails LCEReqFrameVal =
{
    0x0000,
    0x01,
    0x00,
    0x00,
    0x00,
    LoadControlEventCommandPayloadDefaultValue
};

ZCL_MeteringIntervals MeteringIntervalsTable[] =
{
    {{ 0x01, 0x00, 0x00 }},
    {{ 0x02, 0x00, 0x00 }},
    {{ 0x03, 0x00, 0x00 }},
    {{ 0x04, 0x00, 0x00 }},
    {{ 0x05, 0x00, 0x00 }},
    {{ 0x06, 0x00, 0x00 }},
    {{ 0x07, 0x00, 0x00 }},
    {{ 0x08, 0x00, 0x00 }},
    {{ 0x09, 0x00, 0x00 }},
    {{ 0x0A, 0x00, 0x00 }}
};

/*This gives the number of entries in the NVM storage table for
Application.This is used by SE_Profile.c during initialization*/
BYTE NoOfEntriesInAttrStorageTable_ESP =  (sizeof( App_AttributeStorageTable_ESP ) /
                                    sizeof( App_AttributeStorage ));

Meter_FormattingAttributes meterFormatAttributes;

/*This variable holds the number of price events updated by the Utility*/
BYTE numberOfPriceEvents;
/*This variable holds the number of Tiers supported by the device*/
BYTE numberofTiersSupported;

ESP_STATUS espStatus;
/*The SE Profile utilizes a higher level of security on the network but not all clusters
  need to utilize Application Link keys.*/
/*The following table identifies the security keys utilized by each cluster*/
MeterAttributes_DisplayState meterAttributes_DisplayStateTable[METER_ATTRIBUTE_DISPLAY_TABLE_SIZE] =
{
    { ZCL_METERING_CURRENT_SUMMATION_DELIVERED, 0x11 },

    #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
        { ZCL_METERING_CURRENT_SUMMATION_RECEIVED,  0x12 },

        { ZCL_METERING_CURRENT_MAX_DEMAND_DELIVERED,0x13 },
        { ZCL_METERING_CURRENT_MAX_DEMAND_RECEIVED, 0x14 },

        { ZCL_METERING_DFT_SUMMATION,0x15 },

        { ZCL_METERING_CURRENT_TIER1_SUMMATION_DELIVERED, 0x16 },
        { ZCL_METERING_CURRENT_TIER1_SUMMATION_RECEIVED, 0x17 },

        { ZCL_METERING_CURRENT_TIER2_SUMMATION_DELIVERED, 0x18 },
        { ZCL_METERING_CURRENT_TIER2_SUMMATION_RECEIVED, 0x19 },

        { ZCL_METERING_CURRENT_TIER3_SUMMATION_DELIVERED, 0x1A },
        { ZCL_METERING_CURRENT_TIER3_SUMMATION_RECEIVED, 0x1B },

        { ZCL_METERING_CURRENT_TIER4_SUMMATION_DELIVERED, 0x1C},
        { ZCL_METERING_CURRENT_TIER4_SUMMATION_RECEIVED, 0x1D },

        { ZCL_METERING_CURRENT_TIER5_SUMMATION_DELIVERED, 0x1E },
        { ZCL_METERING_CURRENT_TIER5_SUMMATION_RECEIVED, 0x1F },

        { ZCL_METERING_CURRENT_TIER6_SUMMATION_DELIVERED, 0x20 },
        { ZCL_METERING_CURRENT_TIER6_SUMMATION_RECEIVED, 0x21 },

        { ZCL_METERING_INSTANTANEOUS_DEMAND, 0x22 },

        { ZCL_METERING_CURRENT_DAY_CONSUMPTION_DELIVERED, 0x23 },
        { ZCL_METERING_CURRENT_DAY_CONSUMPTION_RECEIVED, 0x24 },

        { ZCL_METERING_PREVIOUS_DAY_CONSUMPTION_DELIVERED, 0x25 },
        { ZCL_METERING_PREVIOUS_DAY_CONSUMPTION_RECEIVED, 0x26 },

        { ZCL_METERING_CURRENT_PARTIAL_PROF_INT_VALUE_DELIVERED, 0x27 },
        { ZCL_METERING_CURRENT_PARTIAL_PROF_INT_VALUE_RECEIVED, 0x28},

        { ZCL_METERING_CURRENT_DEMAND_DELIVERED, 0x29 }
        #endif
};
/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
static BYTE ProcessPriceClusterClientEvents
(
    BYTE eventId,
    BYTE interPANMode,
    BYTE *peventData
);
static void UpdateAndSendPublishPriceCmdPaylaod
(
    BYTE tierIndex,
    BYTE interPANMode,
    BYTE eventId
);

/******************************************************************************
 * Function:        static void CreateLoadControlEventCommand( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         TRUE - If LoadControlEvent cmd is transmitted.
 *                  FALSE - If LoadControlEvent cmd could not be transmitted
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to send the LoadControlEvent
 *                  command.
 *
 * Note:            None
 *****************************************************************************/
static void CreateLoadControlEventCommand
(
   void
);

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


/******************************************************************************
 * Function:        static BYTE SendPublishPriceCommand( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         TRUE - If Publish Price cmd is transmitted.
 *                  FALSE - If Publish Price cmd could not be transmitted
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when a GetCurrentPrice or
 *                  GetScheduledPrice commands. This function is used to
 *                  send the Publish Price command
 *
 * Note:            None
 *****************************************************************************/
static BYTE SendPublishPriceCommand( void );

/*****************************************************************************
  Private Functions
 *****************************************************************************/

/******************************************************************************
 * Function:        static BYTE ProcessPriceClusterClientEvents
 *                  (
 *                      BYTE eventId,
 *                      BYTE interPANMode,
 *                      BYTE *peventData
 *                  )
 *
 * PreCondition:    None
 *
 * Input:           eventId - Specifies the Event type of the Price Cluster
 *                            that application needs to handle
 *                  interPANMode - Specifies whether the Publish Price command
 *                                 needs to be sent to SE Network or InterPAN
 *                                 Network
 *                  pEventData - Holds the information of the Price Command that
 *                              has been received either from InterPAN Network
 *                              or from SE Network.
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when a GetCurrentPrice or
 *                  GetScheduledPrice events needs to be handled.
 *                  This function is used to update the Publish Price command
 *                  necessary parameters before sending out this command.
 *
 * Note:            None
 *****************************************************************************/
static BYTE ProcessPriceClusterClientEvents
(
    BYTE eventId,
    BYTE interPANMode,
    BYTE *peventData
)
{
    BYTE    i;

    if ( eventId == ZCL_Price_GetCurrentPriceEvent )
    {
        DWORD_VAL   currentTimeInSeconds;
        //elapsedTimeInfo timeInfo;

        /*Calculate the current time and extract the corresponding Tier information.*/
        currentTimeInSeconds.Val = ZCL_Callback_GetCurrentTimeInSeconds();
        /*ZCL_Utility_Time_ConvertSecondsToYear ( currentTimeInSeconds.Val, &timeInfo );
        currentTimeInSeconds.Val = ( ( (DWORD)(timeInfo.hour) * NUMBER_OF_SECONDS_IN_AN_HOUR ) +
                                     ( (DWORD)(timeInfo.minute) * NUMBER_OF_SECONDS_IN_A_MINUTE ) +
                                     ( (DWORD)(timeInfo.second) ) );*/

        /*Check which is the current tier information. This information can be
        extracted by comparing the current time with the list of tier information
        available in the ListOfPriceEvents table.*/
        for( i = 0x00; i < numberOfPriceEvents; i++)
        {
            /*if ( ( currentTimeInSeconds.Val >= ListOfPriceEvents[i].startTime ) &&
                 ( currentTimeInSeconds.Val < ( ListOfPriceEvents[i].startTime +
                 (DWORD)( ListOfPriceEvents[i].durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE ) ) ) )*/
            if ( currentTimeInSeconds.Val >= ListOfPriceEvents[i].UTCStartTime )
            {
                if ( ( ListOfPriceEvents[i].durationInMinutes == DURATION_UNTIL_CHANGED ) ||
                     ( currentTimeInSeconds.Val < ( ListOfPriceEvents[i].UTCStartTime +
                       (DWORD)( ListOfPriceEvents[i].durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE ) ) ) )
                {
                    /*This condition is required to check whether the UTCTime has been
                    updated by the utility. By default, all the price entries will be zero.
                    When Utility sets the price entry, the StartTime of the price event
                    will be updated with the UTCStartTime. Then the startTime and endTime
                    will be calculated based on per day seconds which will be used until
                    next price event overwriting the current price. If the UTCTime is zero,
                    this means that Utility has not provided any price information. So, we
                    should not send out the Publish Price Command*/
                    if ( ListOfPriceEvents[i].UTCStartTime != INVALID_UTC_START_TIME )
                    {
                        /*Update and Send the Publish Price Command Payload.*/
                        UpdateAndSendPublishPriceCmdPaylaod
                        (
                            i,
                            interPANMode,
                            eventId
                        );
                        return TRUE;
                    }
                }
            }
        }
    }
    else if ( eventId == ZCL_Price_GetScheduledPricesEvent )
    {
        DWORD   startTime;
        DWORD   currentTime;
        BYTE    numOfEvents;

        /*Copy the Event Data. Event Data holds the information of the
        Get Scheduled Price Payload (StartTime and the NumberOfEvents ).*/
        memcpy
        (
            (BYTE *)&startTime,
            peventData,
            0x04 /*StartTime is a DWORD - 32 bit unsigned integer*/
        );
        numOfEvents = peventData[4];

        /*This condition is required, because, if the number of events is
        Zero, then all the events shall be transmitted.*/
        if ( numOfEvents == 0x00 )
        {
            /*Check whether Utility has supplied any of the Price Events*/
            for (i=0x00; i<numberOfPriceEvents; i++)
            {
                /*If UTCStartTime is zero means that the utility has not
                supplied any pricing events. So, we should not send out
                any Publish Price command*/
                if ( ListOfPriceEvents[i].UTCStartTime != INVALID_UTC_START_TIME )
                {
                    /*This is required to pass the next condition */
                    i = 0x00;
                    /*Update the currentEventIndex to starting index to the
                    price event structure.*/
                    PublishPriceRequestFrameValue.currentEventIndex = 0x00;
                    PublishPriceRequestFrameValue.maxEventIndex = numberofTiersSupported;

                    //PublishPriceRequestFrameValue.startTime = ListOfPriceEvents[i].UTCStartTime;
                    break;
                }
            }
        }
        else
        {
            /*If numOfEvents is non Zero, then StartTime should be considered.
            Publish Price command should transmitted when the StartTime
            requested is stored in the table entry.*/
            for( i = 0x00; i < numberOfPriceEvents; i++ )
            {
                /*If UTCStartTime is zero means that the utility has not
                supplied any pricing events. So, we should not send out
                any Publish Price command*/
                if ( ListOfPriceEvents[i].UTCStartTime != INVALID_UTC_START_TIME )
                {
                    if ( startTime == CURRENT_TIME )
                    {
                        currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
                        if ( ListOfPriceEvents[i].UTCStartTime >= currentTime )
                        {
                            /*Update the currentEventIndex to the index of the
                            event structure from where the Publish Price command
                            should be transmitted.*/
                            PublishPriceRequestFrameValue.currentEventIndex = i;
                            /*Updated the numOfEvents. This is required because, if the
                            requesting does not have capacity to store more information,
                            then only one Publish Price command should be transmitted.
                            We should not transmit the Publish Price command more than
                            whatever the device has requested.*/
                            PublishPriceRequestFrameValue.maxEventIndex = numOfEvents;

                            //PublishPriceRequestFrameValue.startTime = ListOfPriceEvents[i].UTCStartTime;
                            break;
                        }
                    }
                    else
                    {
                        /*This condition is required, because, Publish Price
                        command should be transmitted based on the StartTime*/
                        /*if ( ( startTime >= ListOfPriceEvents[i].UTCStartTime ) &&
                             ( startTime < ( ListOfPriceEvents[i].UTCStartTime +
                             (DWORD)( ListOfPriceEvents[i].durationInMinutes * NUMBER_OF_SECONDS_IN_A_MINUTE ) ) ) )*/
                        if ( ListOfPriceEvents[i].UTCStartTime >= startTime )
                        {
                            /*Update the currentEventIndex to the index of the
                            event structure from where the Publish Price command
                            should be transmitted.*/
                            PublishPriceRequestFrameValue.currentEventIndex = i;
                            /*Updated the numOfEvents. This is required because, if the
                            requesting does not have capacity to store more information,
                            then only one Publish Price command should be transmitted.
                            We should not transmit the Publish Price command more than
                            whatever the device has requested.*/
                            PublishPriceRequestFrameValue.maxEventIndex = numOfEvents;

                            //PublishPriceRequestFrameValue.startTime = ListOfPriceEvents[i].UTCStartTime;
                            break;
                        }
                    }
                }
            }
        }

        if ( i < numberOfPriceEvents )
        {
            /*Update and Send the Publish Price Command Payload.*/
            UpdateAndSendPublishPriceCmdPaylaod
            (
                PublishPriceRequestFrameValue.currentEventIndex,
                interPANMode,
                eventId
            );

            /*Set the Background Bit*/
            espStatus.flags.bits.bPendingPublishPriceCmd = 0x01;
            espStatus.flags.bits.bAwaitingDataConfirm = 0x01;

            PublishPriceRequestFrameValue.currentEventIndex++;
            PublishPriceRequestFrameValue.maxEventIndex--;

            return TRUE;
        }
    }
    return FALSE;
}

/******************************************************************************
 * Function:        static void UpdateAndSendPublishPriceCmdPaylaod
 *                  (
 *                      BYTE tierIndex,
 *                      BYTE interPANMode,
 *                      BYTE eventId
 *                  )
 *
 * PreCondition:    None
 *
 * Input:           tierIndex - Specifies the Price Tier index of the table
 *                              ListOfPriceEvents that needs to be extracted
 *                              transmitted in the Publish Price command
 *                  interPANMode - Specifies whether the Publish Price command
 *                                 needs to be sent to SE Network or InterPAN
 *                                 Network
 *                  eventId - Specifies the Event type of the Price Cluster
 *                              that application needs to handle
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when a Publish Price command needs
 *                  to be updated for transmission. This function extracts the
 *                  pricing and tier information from the table ListOfPriceEvents,
 *                  and update this information onto the default structure of
 *                  Publish Price Command which is used for creating the
 *                  Publish Price command frame.
 *
 * Note:            None
 *****************************************************************************/
static void UpdateAndSendPublishPriceCmdPaylaod
(
    BYTE tierIndex,
    BYTE interPANMode,
    BYTE eventId
)
{
    BYTE lenOfRateLabel;

    PublishPriceCommandPayloadDefaultValue.IssuerEventId =
        ListOfPriceEvents[tierIndex].issuerEventId;

    /*Extract the corresponding Rate Label*/
    lenOfRateLabel = ListOfPriceEvents[tierIndex].rateLabel[0];
    memcpy
    (
        &PublishPriceCommandPayloadDefaultValue.RateLabel[0],
        &ListOfPriceEvents[tierIndex].rateLabel[0],
        lenOfRateLabel + 1
    );

    /*Number of Price Tier is specified using Higher Nibble.*/
    PublishPriceCommandPayloadDefaultValue.NumOfPriceTiersAndRegisterTier =
        ( numberofTiersSupported << GET_HIGHER_NIBBLE ) | ( ListOfPriceEvents[tierIndex].tierInfo );

    /*Price Trailing Digit is specified using Higher Nibble. Currently, it is assumed that the
    Price Trailing Digit is Zero and remain same for all the tiers. */
    PublishPriceCommandPayloadDefaultValue.PriceTrailingDigitAndPriceTier =
        ListOfPriceEvents[tierIndex].priceTrailingDigitAndPriceTier;

    /*Copy the Price information*/
    PublishPriceCommandPayloadDefaultValue.Price = ListOfPriceEvents[tierIndex].price;

    /*Update the StartTime*/
    PublishPriceCommandPayloadDefaultValue.StartTime = ListOfPriceEvents[tierIndex].UTCStartTime;

    /*Update the Duration in minutes. The Price and Tier table entry is maintained in terms
    of seconds for one day. This needs to be converted to minutes, as Publish Price command
    expects in minutes format. This convertion can be done by dividing the Seconds by 60*/
    PublishPriceCommandPayloadDefaultValue.DurationInMinutes =
        ListOfPriceEvents[tierIndex].durationInMinutes;

    if ( interPANMode != 0x01 )
    {
        SendPublishPriceCommand();
    }
    else
    {
        App_SendPublishPriceCommandOnInterPANNetwork();

        /*We need to reset the InterPAN Flag only if the received command is
        Get Current Price. We should not reset this flag when we receive the
        Get Scheduled Price command. This is because, there may be more
        scheduled price events that needs to be transmitted. In this case,
        the InterPAN flag will be resetted in the background, after transmitting
        all the scheduled price events.*/
        if ( eventId == ZCL_Price_GetCurrentPriceEvent )
        {
            /*Reset the InterPAN Flag*/
            PublishPriceRequestFrameValue.InterPANMode = 0x00;
        }
    }
}



/******************************************************************************
 * Function:        static BYTE SendPublishPriceCommand( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         TRUE - If Publish Price cmd is transmitted.
 *                  FALSE - If Publish Price cmd could not be transmitted
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when a GetCurrentPrice or
 *                  GetScheduledPrice commands. This function is used to
 *                  send the Publish Price command
 *
 * Note:            None
 *****************************************************************************/
static BYTE SendPublishPriceCommand( void )
{
    BYTE asdu[60];
    BYTE asduLength;

    /*CurrentTime field should always contain the time at which the Publish
    Price command is being transmitted*/
    PublishPriceCommandPayloadDefaultValue.CurrentTime = ZCL_Callback_GetCurrentTimeInSeconds();

    asduLength =    ZCL_Price_CreatePublishPriceCmd
                    (
                        PublishPriceRequestFrameValue.transactionSeqNum,
                        asdu,
                        &PublishPriceCommandPayloadDefaultValue
                    );
    App_SendData
    (
        APS_ADDRESS_16_BIT,
        (BYTE *)&PublishPriceRequestFrameValue.destAddr,
        PublishPriceRequestFrameValue.destEndPoint,
        asduLength,
        ZCL_PRICE_CLUSTER,
        asdu
    );

    return TRUE;

}

/******************************************************************************
 * Function:        BYTE App_SendPublishPriceCommandOnInterPANNetwork( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         TRUE - If Publish Price cmd is transmitted.
 *                  FALSE - If Publish Price cmd could not be transmitted
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when a GetCurrentPrice or
 *                  GetScheduledPrice commands is recieved through InterPAN.
 *                  This function is used to send the Publish Price command
 *                  on InterPAN network
 *
 * Note:            None
 *****************************************************************************/
BYTE App_SendPublishPriceCommandOnInterPANNetwork( void )
{
    BYTE *pData;

    interPANDataRequest.destAddrMode = PublishPriceRequestFrameValue.destAddrMode;
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
        ( BYTE *)&PublishPriceRequestFrameValue.destAddr,
        interPANDataRequest.asduLength
    );

    interPANDataRequest.destInterPANId = PublishPriceRequestFrameValue.destInterPANId;

    interPANDataRequest.profileId = SE_PROFILE_ID;
    interPANDataRequest.clusterID = ZCL_PRICE_CLUSTER;

    /*Allocate memory for Publish Price Command Payload + the ZCL Header (3 Bytes)*/
    pData =  (BYTE *)SRAMalloc(sizeof(ZCL_Price_PublishPriceInfo) + 0x03);
    if( pData == NULL )
    {
        return FALSE;
    }

    interPANDataRequest.pAsdu = pData;

    /*CurrentTime field should always contain the time at which the Publish
    Price command is being transmitted*/
    PublishPriceCommandPayloadDefaultValue.CurrentTime = ZCL_Callback_GetCurrentTimeInSeconds();

    /*Note: asduLength is reused here. Earlier this field was used for copying the
    destination address.*/
    interPANDataRequest.asduLength = ZCL_Price_CreatePublishPriceCmd
                                     (
                                         PublishPriceRequestFrameValue.transactionSeqNum,
                                         interPANDataRequest.pAsdu,
                                         &PublishPriceCommandPayloadDefaultValue
                                     );

    /*ASDUHandle is a unique number. This should be given by the application. Since,
    TransactionSeqNum is also unique and changes when every packet gets transmitted,
    the same value is used as the ASDUHandle*/
    interPANDataRequest.asduHandle = PublishPriceRequestFrameValue.transactionSeqNum;

    App_SendDataOverInterPAN();

    return TRUE;
}


/******************************************************************************
 * Function:        static void CreateLoadControlEventCommand( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         TRUE - If LoadControlEvent cmd is transmitted.
 *                  FALSE - If LoadControlEvent cmd could not be transmitted
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to send the LoadControlEvent
 *                  command.
 *
 * Note:            None
 *****************************************************************************/
static void CreateLoadControlEventCommand
(
   void
)
{
    BYTE asdu[40];
    BYTE asduLength;

    /*0x03 is ZCL header Length .Length of the Load Control Event Command Payload is fixed. */
    asduLength = ( 0x03 + sizeof( DRLC_LoadControlEvent ) );

    ZCL_DRLC_CreateLoadControlEventCmd
    (
        LCEReqFrameVal.transactionSeqNum,
        asduLength,
        asdu,
        &LCEReqFrameVal.LCECmdPayload[LCEReqFrameVal.currentEventIndex]
    );

    App_SendData
    (
        APS_ADDRESS_16_BIT,
        ( BYTE *)&LCEReqFrameVal.destAddr,
        LCEReqFrameVal.destEndPoint,
        asduLength,
        ZCL_DRLC_CLUSTER,
        asdu
    );
}

/*****************************************************************************
  Public Functions
 *****************************************************************************/

/******************************************************************************
 * Function:        void SE_HandleEvents( ZCL_Event *pZCLEvent, APP_DATA_indication* p_dataInd );
 *
 * PreCondition:    None
 *
 * Input:           pZCLEvent  -  This points ZCL Event.
 *                  p_dataInd  -  received data
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
void SE_HandleEvents_ESP( ZCL_Event *pZCLEvent, APP_DATA_indication* p_dataInd )
{
    switch( pZCLEvent->eventId )
    {
        case ZCL_Identify_IdentifyEvent :
            /*Every one second elapses we need to decrement identifyTimeoutVale.So
              we need Enable Identifymode as a backgroundTask.*/
            appIdentifyTimers.IdentifyTime = 0x01;
            appIdentifyTimers.IdentifyStartTime = TickGet();
            LIGHT3_ON();
            break;

        /*Since we are taking backup of the Device information, it does not matter what frame
        has been received. That is why the Fall through case statement has been added*/
        case ZCL_Price_GetCurrentPriceEvent: // Fall through
        case ZCL_Price_GetScheduledPricesEvent:
            {
                BYTE status;

                /*Take backup of the required fields used for creating the Publish Price Command.*/
                PublishPriceRequestFrameValue.InterPANMode = 0x00;
                /*For any packet received via SE Network, the SrcAddress will always be
                Short Address. So, the destAddreMode will be set to Short Addressing Mode*/
                PublishPriceRequestFrameValue.destAddrMode = APS_ADDRESS_16_BIT;
                /*Copy the SrcAddress information. Publish Price command should be sent this
                particular device*/
                PublishPriceRequestFrameValue.destAddr[0] = p_dataInd->SrcAddress.ShortAddr.v[0];
                PublishPriceRequestFrameValue.destAddr[1] = p_dataInd->SrcAddress.ShortAddr.v[1];
                /*Copy the SrcEndPoint information. Publish Price command should be sent to this
                End Point*/
                PublishPriceRequestFrameValue.destEndPoint = p_dataInd->SrcEndpoint;
                /*Transaction Sequence Number should be same as that the recevied frame*/
                PublishPriceRequestFrameValue.transactionSeqNum = p_dataInd->asdu[1];

                status = ProcessPriceClusterClientEvents
                        (
                            pZCLEvent->eventId,
                            PublishPriceRequestFrameValue.InterPANMode,
                            pZCLEvent->pEventData
                        );

                if ( status == FALSE )
                {
                    /*FALSE is beacuse Respective cluster specific security
                    shall be applied to the Default Response command*/
                    SE_SendDefaultResponse(ZCL_NotFound, FALSE, p_dataInd);
                }
            }
            break;

        case ZCL_Price_PublishPriceEvent:
            /*No need to handle Publish Price Command because Publish Price Command will never
            be received by ESP over the SE Network. This is because in SE Network only one ESP
            exists. So, Publish Price Command can only be received by ESP over the Inter-PAN.*/
            break;


        case ZCL_Message_GetLastMessageEvent:
        {
            BYTE asdu[40];
            BYTE asduLength;

            /*Create Display message and send it to the requested
            device*/
            if( ZCL_Message_DisplayMessageTable.startTime != INVALID_START_TIME )
            {
                asduLength =
                ZCL_CreateDisplayMessageCommand
                (
                    asdu,
                    &( ZCL_Message_DisplayMessageTable) ,
                    ( p_dataInd->asdu[1])
                );
                /*create and send data*/
                App_SendData
                (
                    APS_ADDRESS_16_BIT,
                    (BYTE*)&p_dataInd->SrcAddress.ShortAddr.Val,
                    p_dataInd->SrcEndpoint,
                    asduLength,
                    ZCL_MESSAGING_CLUSTER,
                    asdu
                );
            }
            else
            {
                /*FALSE is beacuse Respective cluster specific security
                shall be applied to the Default Response command*/
                SE_SendDefaultResponse(ZCL_NotFound, FALSE, p_dataInd);
            }
        }
        break;

        case ZCL_Message_MessageConfirmationEvent:
            /*No action done. Real application can maintain
            in the table of messages if confirmation is received
            or not*/
            break;
        case ZCL_DRLC_GetScheduledEvents:
            {
                BYTE i;
                BYTE    numOfEvents;
                /*We need to send LoadControl event command to the Requested device.so we need to
                 store the Requsted device details in LoadControlEventRequestFrame*/
                LCEReqFrameVal.destAddr = p_dataInd->SrcAddress.ShortAddr.Val;
                LCEReqFrameVal.destEndPoint = p_dataInd->SrcEndpoint;
                LCEReqFrameVal.transactionSeqNum = p_dataInd->asdu[1];

                memcpy
                (
                    (BYTE *)&espStatus.GetScheduledEventStartTime,
                    pZCLEvent->pEventData,
                    0x04 /*StartTime is a DWORD - 32 bit unsigned integer*/
                );
                if( espStatus.GetScheduledEventStartTime == 0x00000000 )
                {
                    espStatus.GetScheduledEventStartTime =
                        ZCL_Callback_GetCurrentTimeInSeconds();
                }
                numOfEvents = pZCLEvent->pEventData[4];

                //LCEReqFrameVal.currentEventIndex = 0x00;
                if ( numOfEvents == 0x00 )
                {
                    LCEReqFrameVal.maxEventIndex = MAX_LOAD_CONTROL_EVENTS;
                }
                else
                {
                    LCEReqFrameVal.maxEventIndex = numOfEvents;
                }

                for ( i = 0; i < MAX_LOAD_CONTROL_EVENTS; i++ )
                {
                    if ( LCEReqFrameVal.LCECmdPayload[i].startTime >
                         espStatus.GetScheduledEventStartTime)
                    {
                        LCEReqFrameVal.currentEventIndex = i;
                        break;
                    }
                }
                /*if
                (
                    ( LCEReqFrameVal.LCECmdPayload[LCEReqFrameVal.currentEventIndex].startTime >
                      espStatus.GetScheduledEventStartTime )
                )*/
                if ( i < MAX_LOAD_CONTROL_EVENTS )
                {
                    CreateLoadControlEventCommand();
                    espStatus.flags.bits.bAwaitingDataConfirm = 0x01;
                    LCEReqFrameVal.maxEventIndex--;

                    espStatus.flags.bits.bPendingLoadControlEventCmd = 0x01;
                    espStatus.loadControlEventTime = TickGet();
                    LCEReqFrameVal.currentEventIndex++;
                }
                else
                {
                    /*FALSE is beacuse Respective cluster specific security
                    shall be applied to the Default Response command*/
                    SE_SendDefaultResponse(ZCL_NotFound, FALSE, p_dataInd);
                }
                /*espStatus.flags.bits.bPendingLoadControlEventCmd = 0x01;
                espStatus.loadControlEventTime = TickGet();
                LCEReqFrameVal.currentEventIndex++;*/
            }
            break;

        case ZCL_DRLC_ReportEventStatusEvent:
        {


        }
        break;

        case ZCL_MeteringGetProfileEvent:
        {
            WORD storageIndex;
            DWORD current_Time;
            ZCL_MeteringClusterAttributes meteringClusterAttributes;
            BYTE asdu[50];
            BYTE asduLength;

            ZCL_MeteringGetProfileRspCmdPayload *GetProfileRspCmdPayload =
                (ZCL_MeteringGetProfileRspCmdPayload *)&asdu[3];
            ZCL_MeteringGetProfileReqCmdPayload *GetProfileReqCmdPayloadReceived =
                (ZCL_MeteringGetProfileReqCmdPayload *)&p_dataInd->asdu[3];

            asduLength = GET_PROFILE_RESP_FIXED_FIELDS_LEN;

                        /* ZCL Header */
            /* Frame control field. */
            *( asdu ) = ( ZCL_SERVER_TO_CLIENT_DIRECTION |ZCL_ClusterSpecificCommand);

            *( asdu + ZCL_FRAME_TSN_POSITION_MANUFACTURE_BIT_DISABLED ) = appNextSeqNum;
            appNextSeqNum++;

            *( asdu + ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) =
                        ZCL_MeteringGetProfileResponse;

                        /* ZCL DATA */

            /* End Time, updated as, received from get profile request.
               Needs to be modified in reference implementation. */
            GetProfileRspCmdPayload->endTime =
                GetProfileReqCmdPayloadReceived->endTime;

            /* For reading Meters MAX no of intervals from NVM we
               need get the NVM Index Value */
            if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                    ZCL_Callback_GetClusterIndex
                    (
                        ESP_ENDPOINT_ID,
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
            /* Get the current time */
            current_Time = ZCL_Callback_GetCurrentTimeInSeconds();

            /* Status */
            /* Undefined interval channel requested */
            if( (GetProfileReqCmdPayloadReceived->intervalChannel !=
                    ZCL_MeteringConsumptionDelivered ) &&
                (GetProfileReqCmdPayloadReceived->intervalChannel !=
                    ZCL_MeteringConsumptionReceived ) )
            {
                GetProfileRspCmdPayload->status =
                    ZCL_MeteringUndefinedIntervalChannelRequested;
            }

            /* Requested time is more than Current time,
              Needs to be updated for reference implementation. */
            else if( GetProfileReqCmdPayloadReceived->endTime > current_Time )
            {
                 GetProfileRspCmdPayload->status =
                            ZCL_MeteringInvalidEndTime;
            }
            #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                /* More periods requested than can be returned */
                else if( GetProfileReqCmdPayloadReceived->numberOfPeriods >
                    meteringClusterAttributes.maxNumberOfPeriodsDelivered )
                {
                     GetProfileRspCmdPayload->status =
                                ZCL_MeteringMorePeriodsRequestedThanCanBeReturned;
                }
            #endif
            /* No attributes available for requested time */
            else if( FALSE )
            {
                 GetProfileRspCmdPayload->status =
                            ZCL_MeteringNoIntervalsAvailableForTheRequestedTime;
            }
            else
            {
                 GetProfileRspCmdPayload->status =
                            ZCL_MeteringIntervalSuccess;
            }

            /* ProfileIntervalPeriod */
            GetProfileRspCmdPayload->profileIntervalPeriod = ZCL_MeteringSixtyMinutes;

            /* Number of Periods Delivered */
            if ( ZCL_MeteringIntervalSuccess == GetProfileRspCmdPayload->status)
            {
                GetProfileRspCmdPayload->numberOfPeriodsDelivered =
                    GetProfileReqCmdPayloadReceived->numberOfPeriods;
            }
            else
            {
                /* Not success, so make number of intervals to 0 */
                 GetProfileRspCmdPayload->numberOfPeriodsDelivered = 0x00;
            }

            memcpy (
                    (BYTE *)GetProfileRspCmdPayload->intervals,
                    (BYTE *) MeteringIntervalsTable,
                    GetProfileRspCmdPayload->numberOfPeriodsDelivered *
                            sizeof(ZCL_MeteringIntervals)
                    );

            asduLength += (GetProfileRspCmdPayload->numberOfPeriodsDelivered *
                            sizeof(ZCL_MeteringIntervals) );

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

        case ZCL_MeteringGetProfileResponseEvent:
        {
            /* Application should handle reception of
               Get profile response command */
        }
        break;


        case ZCL_MeteringRequestMirrorEvent:
        {
            WORD storageIndex;
            ZCL_BasicClusterAttributes basicAttributes;
            BYTE asdu[10];

            /* ZCL Header */
            /* Frame Control */
            *( asdu) = ZCL_ClusterSpecificCommand;
            *( asdu + ZCL_FRAME_TSN_POSITION_MANUFACTURE_BIT_DISABLED) = appNextSeqNum;
            appNextSeqNum++;

            *( asdu + ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) =
                ZCL_MeteringRequestMirrorResponse;

            /* For reading Basic cluster physical environment from NVM we
                need get the NVM Index Value */
            if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                    ZCL_Callback_GetClusterIndex
                    (
                        ESP_ENDPOINT_ID,
                        ZCL_BASIC_CLUSTER,
                        &storageIndex
                    )
               )
            {
                /* Read the ZCL Meter Cluster Server Attributes from NVM */
                ZCL_Callback_GetAttributeValue
                (
                    storageIndex,
                    (BYTE *)&basicAttributes,
                    sizeof(ZCL_BasicClusterAttributes)
                );
            }
            #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                mirrorStatus.ESP_HasCapability = basicAttributes.physicalEnvironment;
            #endif

            if ( mirrorStatus.ESP_HasCapability )
            {
                *( asdu + MIRROR_RSP_END_POINT_INDEX ) = ESP_ENDPOINT_ID;

                *( asdu + MIRROR_RSP_END_POINT_INDEX + 1 ) = 0x00;

                mirrorStatus.ESP_EndPoint = ESP_ENDPOINT_ID;

                mirrorStatus.deviceMirroring = TRUE;
            }
            else
            {
                *( asdu + MIRROR_RSP_END_POINT_INDEX ) = INVALID_END_POINT;

                *( asdu + MIRROR_RSP_END_POINT_INDEX + 1 ) = INVALID_END_POINT;

                mirrorStatus.deviceMirroring = FALSE;
            }
            App_SendData
            (
                APS_ADDRESS_16_BIT,
                ( BYTE *)&(p_dataInd->SrcAddress.ShortAddr.v[0]),
                p_dataInd->SrcEndpoint,
                ( sizeof( ZCL_MeteringRequestMirrorResponseCmdPayload ) +
                    ZCL_HEADER_LENGTH_WO_MANUF_CODE ),
                ZCL_METERING_CLUSTER,
                asdu
            );
        }
        break;
        case ZCL_MeteringRemoveMirrorEvent:
        {
            BYTE asdu[10];

            /* ZCL Header */
            /* Frame Control */
            *( asdu ) = ZCL_ClusterSpecificCommand;
            *( asdu + ZCL_FRAME_TSN_POSITION_MANUFACTURE_BIT_DISABLED) = appNextSeqNum;
            appNextSeqNum++;

            *( asdu +
            ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) =
                ZCL_MeteringMirrorRemoved;

            /* ZCL Payload */
            if ( mirrorStatus.deviceMirroring )
            {
                *( asdu + MIRROR_RSP_END_POINT_INDEX ) =
                    mirrorStatus.ESP_EndPoint;

                *( asdu +
                    MIRROR_RSP_END_POINT_INDEX + 1 ) = 0x00;

                mirrorStatus.deviceMirroring = FALSE;
            }
            else
            {
                *( asdu + MIRROR_RSP_END_POINT_INDEX ) = INVALID_END_POINT;
                *( asdu + MIRROR_RSP_END_POINT_INDEX + 1 ) = INVALID_END_POINT;
            }

            mirrorStatus.ESP_EndPoint = INVALID_END_POINT;
            App_SendData
            (
                APS_ADDRESS_16_BIT,
                ( BYTE *)&(p_dataInd->SrcAddress.ShortAddr.v[0]),
                p_dataInd->SrcEndpoint,
                ( sizeof( ZCL_MeteringMirrorRemovedCmdPayload ) +
                    ZCL_HEADER_LENGTH_WO_MANUF_CODE ),
                ZCL_METERING_CLUSTER,
                asdu
            );
        }
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

        case ZCL_Alarm_ResetAlarmEvent :
        {
                WORD clusterId;
                memcpy
                (
                   (BYTE*)&clusterId,
                    (BYTE*)&(pZCLEvent->pEventData[1]),
                    ZCL_CLUSTER_ID_LENGTH
                );
                /*This function resets(makes Inactive) the alarmEntry,if exists*/
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

                /*This function resets(makes Inactive) the all the alarmEntries,if exists*/
                SE_UpdateAlarmTable
                (
                    INVALID_CLUSTERID,
                    0x00,           /*Alarm code is ignored for Reset All Alarm Event */
                    INVALID_TIMESTAMP,
                    ZCL_Alarm_ResetAllAlarmsEvent
                );
        break;
        case ZCL_Alarm_ResetAlarmLogEvent :
                /*This function Clears the alarmTable*/
                SE_UpdateAlarmTable
                (
                    INVALID_CLUSTERID,
                    0x00,           /*Alarm code is ignored for Reset Alarm Log Event */
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
 * Overview:        None
 *
 * Note:            None
 *****************************************************************************/
void App_HandlePendingData_ESP ( void )
{
    TICK currentTime =  TickGet();

    /*First priority is given to Identify Cluster*/
    if ( appIdentifyTimers.IdentifyTime )
    {
        if (TickGetDiff( currentTime, appIdentifyTimers.IdentifyStartTime ) > ONE_SECOND )
        {
            appIdentifyTimers.IdentifyStartTime = currentTime;
            SE_HandleIdentifyTimeout( ESP_ENDPOINT_ID );
        }
    }
    
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
    #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
            ZCL_Callback_CheckForAlarmCondition
            (
                ESP_ENDPOINT_ID,
                ZCL_MAINS_VOLTAGE,
                &zcl_PowerConfigurationClusterServerInfo
            );
    #endif
        }
    }

    /*If waiting for Data Confirm, then just return*/
    if ( espStatus.flags.bits.bAwaitingDataConfirm )
    {
        return;
    }

    if ( espStatus.flags.bits.bPendingPublishPriceCmd )
    {
        if ( ( PublishPriceRequestFrameValue.currentEventIndex < numberOfPriceEvents ) &&
             ( PublishPriceRequestFrameValue.maxEventIndex != 0x00 ) )
        {
            //BYTE lenOfRateLabel;
            BYTE i;

            for (i=PublishPriceRequestFrameValue.currentEventIndex; i<numberOfPriceEvents; i++)
            {
                /*If UTCStartTime is zero means that the utility has not
                supplied any pricing events. So, we should not send out
                any Publish Price command*/
                if ( ListOfPriceEvents[i].UTCStartTime != INVALID_UTC_START_TIME )
                {
                    //if ( ListOfPriceEvents[i].UTCStartTime >= PublishPriceRequestFrameValue.startTime )
                    {
                        /*Get the Next Price Event based on the Start Time*/
                        PublishPriceRequestFrameValue.currentEventIndex = i;
                        break;
                    }
                }
            }

            if ( i < numberOfPriceEvents )
            {
                /*Update and Send the Publish Price Command Payload.*/
                UpdateAndSendPublishPriceCmdPaylaod
                (
                    PublishPriceRequestFrameValue.currentEventIndex,
                    PublishPriceRequestFrameValue.InterPANMode,
                    ZCL_Price_GetScheduledPricesEvent
                );
                espStatus.flags.bits.bAwaitingDataConfirm = 0x01;
            }

            PublishPriceRequestFrameValue.currentEventIndex++;
            PublishPriceRequestFrameValue.maxEventIndex--;
        }
        else
        {
            espStatus.flags.bits.bPendingPublishPriceCmd = 0x00;
            espStatus.flags.bits.bAwaitingDataConfirm = 0x00;
            /*Reset the InterPAN Flag*/
            PublishPriceRequestFrameValue.InterPANMode = 0x00;
            currentPrimitive_ESP = NO_PRIMITIVE;
        }
    }
    if ( espStatus.flags.bits.bPendingLoadControlEventCmd )
    {

       if (TickGetDiff( currentTime, espStatus.loadControlEventTime ) > ONE_SECOND )
       {

            espStatus.loadControlEventTime = TickGet();
            if ( ( LCEReqFrameVal.currentEventIndex < MAX_LOAD_CONTROL_EVENTS ) &&
                ( LCEReqFrameVal.maxEventIndex != 0x00 ) )
            {
                if
                (
                    ( LCEReqFrameVal.LCECmdPayload[LCEReqFrameVal.currentEventIndex].startTime >
                        espStatus.GetScheduledEventStartTime )
                )
                {
                    CreateLoadControlEventCommand();
                    espStatus.flags.bits.bAwaitingDataConfirm = 0x01;
                    LCEReqFrameVal.maxEventIndex--;
                }

                LCEReqFrameVal.currentEventIndex++;
            }
            else
            {
                espStatus.flags.bits.bPendingLoadControlEventCmd = 0x00;
                espStatus.flags.bits.bAwaitingDataConfirm = 0x00;
            }
       }
    }

    if( espStatus.flags.bits.bPendingDisplayMessageCmd )
    {
        /*get the current time*/
        DWORD currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
       /*store the previous difference in minutes*/
        static WORD prevdifferenceInMinutes = 0x00;
        /*store the current difference in minutes*/
        WORD currentDifferenceInMinutes = 0x00;
        /*if current time is greater than the display command time, then
        the start time has occurred and duration in minutes need to be
        manipulated*/
        if( currentTime > espStatus.displayMessageCmdTime )
        {
            /*if duration in miuntes is zero, then set the start time to invalid
            value and reset the background task*/
            if ( !( ZCL_Message_DisplayMessageTable.durationInMinutes ) )
            {
                espStatus.flags.bits.bPendingDisplayMessageCmd = 0x00;
                ZCL_Message_DisplayMessageTable.startTime = INVALID_START_TIME;
                prevdifferenceInMinutes = 0x0000;
            }
            /*if previous time is not equal to current time, then take
            the difference in minutes and calculate the duration in minutes.*/

            currentDifferenceInMinutes =
                ( ( currentTime - espStatus.displayMessageCmdTime ) / 60 );

            if( currentDifferenceInMinutes != prevdifferenceInMinutes )
            {
                if( ZCL_Message_DisplayMessageTable.durationInMinutes != INVALID_DURATION )
                {
                    ZCL_Message_DisplayMessageTable.durationInMinutes -=
                        ( currentDifferenceInMinutes - prevdifferenceInMinutes );
                }
                /*store the previous difference in minutes*/
                prevdifferenceInMinutes = currentDifferenceInMinutes;
            }
        }
    }
    #if I_SUPPORT_REPORTING == 1
        /*if only the network is formed continue further*/
        if( ZigBeeStatus.flags.bits.bNetworkFormed )
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
                    > ( ReportingBgTasks.consolidatedMaxTime * ONE_SECOND ) )
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

                    if ( FALSE == ( App_CheckIfReportReceived() ) )
                    {
                        /*Application should write its code here to handle the timeout condition */
                        //ConsolePutROMString( (ROM char *)"\r\nReport Attribute not received\r\n\r\n" );
                    }
                    /*Application should write its code here to handle the timeout condition */
                    ReportingBgTasks.timeOutExpired = FALSE;
                    /*Reset the flag maintained for Report attribute command to FALSE*/
                    App_ResetReportAttributeReceived();

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
void App_UpdateDataConfTable_ESP( APP_DATA_confirm *dataConf )
{
    espStatus.flags.bits.bAwaitingDataConfirm = 0x00;
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
 * Overview:        This function is called when Inter-PAN message is transmitted
 *                  and is notified by the lower layer about the status of the
 *                  transmission. This function reset the background flag so that
 *                  the next transmission can be attempted.
 *
 * Note:            None
 *****************************************************************************/
#if I_SUPPORT_STUB_APS == 1
void APP_HandleInterPANDataConfirm_ESP ( void )
{
    /**/
    espStatus.flags.bits.bAwaitingDataConfirm = 0x00;
    currentPrimitive_ESP = NO_PRIMITIVE;
}
#endif

/******************************************************************************
 * Function:        void putPhysicalEnvironment(BYTE capacity)
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
void putPhysicalEnvironment(BYTE capacity)
{
    #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
    WORD storageIndex;
    ZCL_BasicClusterAttributes basicAttributes;

    /* For reading Basic cluster physical environment from NVM we
       need get the NVM Index Value */
    if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
            ZCL_Callback_GetClusterIndex
            (
                ESP_ENDPOINT_ID,
                ZCL_BASIC_CLUSTER,
                &storageIndex
            )
       )
    {
        ZCL_Callback_GetAttributeValue
        (
            storageIndex,
            (BYTE *)&basicAttributes,
            sizeof(ZCL_BasicClusterAttributes)
        );

        basicAttributes.physicalEnvironment = capacity;
        /* Write the ZCL Meter Cluster Server Attributes to NVM */
        ZCL_Callback_PutAttributeValue
        (
            storageIndex,
            (BYTE *)&basicAttributes,
            sizeof(ZCL_BasicClusterAttributes)
        );
    }
    #endif
}


/******************************************************************************
 * Function:        void setDRLCTable( BYTE index, BYTE *LCEPayload )
 *
 * PreCondition:    None
 *
 * Input:           index - Table index where DRLC command payload needs
 *                  to be saved.
 *                  LCEPayload- DRLC payload to be saved
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        Saves the LCE payload onto Table provided on ESP.
 *
 * Note:            None
 *****************************************************************************/
void setDRLCTable( BYTE index, BYTE *LCEPayload )
{
    if( index < MAX_LOAD_CONTROL_EVENTS)
    {
        memcpy
        (
            (BYTE *)&LCEReqFrameVal.LCECmdPayload[index],
            LCEPayload,
            sizeof(DRLC_LoadControlEvent)
        );
    }
}

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
BYTE APP_HandleInterPANDataIndication_ESP( void )
{
    ZCL_ActionHandler pActionHandler;
    BYTE notifyTestTool = TRUE;
    BYTE status = ZCL_Success;
    BYTE dummyAsdu;

    currentPrimitive_ESP = NO_PRIMITIVE;

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

    pActionHandler.event.pEventData = SRAMalloc(60);

    if( pActionHandler.event.pEventData != NULL )
    {
        if ( params.INTRP_DATA_indication.ClusterId.Val == ZCL_PRICE_CLUSTER )
        {
            ZCL_HandlePriceClusterCommand
            (
                ESP_ENDPOINT_ID,
                params.INTRP_DATA_indication.ASDULength,
                params.INTRP_DATA_indication.ASDU,
                &dummyAsdu,
                &pActionHandler,
                &zcl_PriceClusterServerInfo
            );

            if ( pActionHandler.action == Send_Response )
            {
                /*Send Default Response with Status as UnsupportedCommand*/
                /*For time being, free up the memory*/
                status = ZCL_UnSupportedClusterCommand;
            }
            else
            {
                switch( pActionHandler.event.eventId )
                {
                    /*Since we are taking backup of the Device information, it does not matter what frame
                    has been received. That is why the Fall through case statement has been added*/
                    case ZCL_Price_GetCurrentPriceEvent: // Fall through
                    case ZCL_Price_GetScheduledPricesEvent:
                    {
                        /*Take backup of the required fields used for creating the Publish
                        Price Command.*/
                        PublishPriceRequestFrameValue.InterPANMode = 0x01;
                        /*For any packet received via Inter-PAN, the SrcAddress will always be
                        Extended Address. So, the destAddreMode will be set to Extended Addressing Mode*/
                        PublishPriceRequestFrameValue.destAddrMode = APS_ADDRESS_64_BIT;
                        /*Copy the SrcAddress which is of 8 Bytes (Extended Address)*/
                        memcpy
                        (
                            PublishPriceRequestFrameValue.destAddr,
                            (BYTE *)&params.INTRP_DATA_indication.SrcAddress.v[0],
                            MAX_ADDR_LENGTH // Inter-PAN communication happens using Extended Address
                        );
                        /*Store the PANId information. The Publish Price command will be transmitted
                        on to the network as specified by this SrcPANId field.*/
                        PublishPriceRequestFrameValue.destInterPANId = params.INTRP_DATA_indication.SrcPANId.Val;
                        /*Transaction Sequence Number should be same as that the recevied frame*/
                        PublishPriceRequestFrameValue.transactionSeqNum = params.INTRP_DATA_indication.ASDU[1];

                        if ( FALSE ==   ProcessPriceClusterClientEvents
                                        (
                                            pActionHandler.event.eventId,
                                            PublishPriceRequestFrameValue.InterPANMode,
                                            pActionHandler.event.pEventData
                                        ))
                        {
                            status = ZCL_NotFound;
                        }

                        /*Inter-PAN data indication should not be sent to
                        the test tool because Publish Price command is being
                        sent*/
                        notifyTestTool = FALSE;
                    }
                    break;

                    case ZCL_Price_PublishPriceEvent:
                        ProcessPublishPriceEvent ( pActionHandler.event.pEventData );
                        break;
                }
            }
        }
        else if ( params.INTRP_DATA_indication.ClusterId.Val == ZCL_MESSAGING_CLUSTER )
        {
            ZCL_HandleMessageClusterCommand
            (
                ESP_ENDPOINT_ID,
                params.INTRP_DATA_indication.ASDULength,
                params.INTRP_DATA_indication.ASDU,
                &dummyAsdu,
                &pActionHandler,
                &zcl_MessageClusterServerInfo
            );

            if ( pActionHandler.action == Send_Response )
            {
                /*Send Default Response with Status as UnsupportedCommand*/
                status = ZCL_UnSupportedClusterCommand;
            }
            else
            {
                switch( pActionHandler.event.eventId )
                {
                    case ZCL_Message_GetLastMessageEvent:
                        {
                            /*Create Display message and send it to the requested
                            device*/
                            if( ZCL_Message_DisplayMessageTable.startTime != INVALID_START_TIME )
                            {
                                BYTE *pData;

                                /*Allocate memory for Publish Price Command Payload + the ZCL Header (3 Bytes)*/
                                pData =  (BYTE *)SRAMalloc(126);
                                if( pData == NULL )
                                {
                                    /*Memory Allocation failed. So, nothing to do about it*/
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

                                interPANDataRequest.pAsdu = pData;

                                /*Note: asduLength is reused here. Earlier this field was used for copying the
                                destination address.*/
                                interPANDataRequest.asduLength = ZCL_CreateDisplayMessageCommand
                                                                 (
                                                                    interPANDataRequest.pAsdu,
                                                                    &( ZCL_Message_DisplayMessageTable) ,
                                                                    ( params.INTRP_DATA_indication.ASDU[1] )
                                                                 );

                                /*ASDUHandle is a unique number. This should be given by the application. Since,
                                TransactionSeqNum is also unique and changes when every packet gets transmitted,
                                the same value is used as the ASDUHandle*/
                                interPANDataRequest.asduHandle = params.INTRP_DATA_indication.ASDU[1];

                                App_SendDataOverInterPAN();

                                notifyTestTool = FALSE;
                            }
                            else
                            {
                                status = ZCL_NotFound;
                            }
                        }
                        break;

                    default:
                        status = ZCL_UnSupportedClusterCommand;
                        break;
                }
            }
        }
        else
        {
            status = ZCL_Failure;
        }

        /*Free the allocated pEventData*/
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
static void ProcessPublishPriceEvent ( BYTE *pAsdu )
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
    App_UpdatePriceEvents_ESP();
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
void App_UpdatePriceEvents_ESP ( void )
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
void App_PriceInit()
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
 * Function:        void App_MessageTableInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the MessageTable
 *
 * Note:            None
 ***************************************************************************************/
void App_MessageTableInit( void )
{
    ZCL_Message_DisplayMessageTable.startTime = INVALID_START_TIME;
    ZCL_Message_DisplayMessageTable.durationInMinutes = 0x0000;
}

