/*********************************************************************
 *
 *                  ZCL Metering Header File
 *
 *********************************************************************
 * FileName        : ZCL_Metering.h
 * Dependencies    :
 * Processor       : PIC18 / PIC24 / dsPIC33
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

 *****************************************************************************/
#ifndef _ZCL_METERING_H_
#define _ZCL_METERING_H_

/*****************************************************************************
                                Includes
 *****************************************************************************/
 /* None */

/*****************************************************************************
                          Constants and Enumerations
 *****************************************************************************/
#define ZCL_METERING_CLUSTER                                           0x0702


/* Reading Information Attribute Set */
#define ZCL_METERING_CURRENT_SUMMATION_DELIVERED                       0x0000

#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )

    #define ZCL_METERING_CURRENT_SUMMATION_RECEIVED                    0x0001
    #define ZCL_METERING_CURRENT_MAX_DEMAND_DELIVERED                  0x0002
    #define ZCL_METERING_CURRENT_MAX_DEMAND_RECEIVED                   0x0003
    #define ZCL_METERING_DFT_SUMMATION                                 0x0004
    #define ZCL_METERING_DAILY_FREEZE_TIME                             0x0005
    #define ZCL_METERING_POWER_FACTOR                                  0x0006
    #define ZCL_METERING_READING_SNAP_SHOT_TIME                        0x0007
    #define ZCL_METERING_CURRENT_MAX_DEMAND_DELIVERED_TIME             0x0008
    #define ZCL_METERING_CUREENT_MAX_DEMAND_RECEIVED_TIME              0x0009

#endif

/* TOU Information Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_METERING_CURRENT_TIER1_SUMMATION_DELIVERED             0x0100
    #define ZCL_METERING_CURRENT_TIER1_SUMMATION_RECEIVED              0x0101
    #define ZCL_METERING_CURRENT_TIER2_SUMMATION_DELIVERED             0x0102
    #define ZCL_METERING_CURRENT_TIER2_SUMMATION_RECEIVED              0x0103
    #define ZCL_METERING_CURRENT_TIER3_SUMMATION_DELIVERED             0x0104
    #define ZCL_METERING_CURRENT_TIER3_SUMMATION_RECEIVED              0x0105
    #define ZCL_METERING_CURRENT_TIER4_SUMMATION_DELIVERED             0x0106
    #define ZCL_METERING_CURRENT_TIER4_SUMMATION_RECEIVED              0x0107
    #define ZCL_METERING_CURRENT_TIER5_SUMMATION_DELIVERED             0x0108
    #define ZCL_METERING_CURRENT_TIER5_SUMMATION_RECEIVED              0x0109
    #define ZCL_METERING_CURRENT_TIER6_SUMMATION_DELIVERED             0x010a
    #define ZCL_METERING_CURRENT_TIER6_SUMMATION_RECEIVED              0x010b
#endif

/* Meter Status Attribute */
#define ZCL_METERING_STATUS                                            0x0200

/* Formatting Attribute Set */
#define ZCL_METERING_UNIT_OF_MEASURE                                   0x0300

#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_METERING_MULTIPLIER                                    0x0301
    #define ZCL_METERING_DIVISOR                                       0x0302
#endif
#define ZCL_METERING_SUMMATION_FORMATTING                              0x0303

#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_METERING_DEMAND_FORMATTING                             0x0304
    #define ZCL_METERING_HISTORICAL_CONSUMPTION_FORMATTING             0x0305
#endif
#define ZCL_METERING_DEVICE_TYPE                                       0x0306

/* ESP Historical Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_METERING_INSTANTANEOUS_DEMAND                          0x0400
    #define ZCL_METERING_CURRENT_DAY_CONSUMPTION_DELIVERED             0x0401
    #define ZCL_METERING_CURRENT_DAY_CONSUMPTION_RECEIVED              0x0402
    #define ZCL_METERING_PREVIOUS_DAY_CONSUMPTION_DELIVERED            0x0403
    #define ZCL_METERING_PREVIOUS_DAY_CONSUMPTION_RECEIVED             0x0404
    #define ZCL_METERING_CURRENT_PARTIAL_PROF_INT_START_TIME_DELIVERED 0x0405
    #define ZCL_METERING_CURRENT_PARTIAL_PROF_INT_START_TIME_RECEIVED  0x0406
    #define ZCL_METERING_CURRENT_PARTIAL_PROF_INT_VALUE_DELIVERED      0x0407
    #define ZCL_METERING_CURRENT_PARTIAL_PROF_INT_VALUE_RECEIVED       0x0408
#endif

/* Load Profile Configuration Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_METERING_MAX_NUMBER_OF_PERIODS_DELIVERED               0x0500
#endif

/* Supply Limit Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_METERING_CURRENT_DEMAND_DELIVERED                      0x0600
    #define ZCL_METERING_DEMAND_LIMIT                                  0x0601
    #define ZCL_METERING_DEMAND_INTEGRATION_PERIOD                     0x0602
    #define ZCL_METERING_NUMBER_OF_DEMAND_SUBINTERVALS                 0x0603
#endif


enum Meter_Generated_Commands
{
    ZCL_MeteringGetProfileResponse                                      = 0x00,
    ZCL_MeteringRequestMirror                                           = 0x01,
    ZCL_MeteringRemoveMirror                                            = 0x02
};
enum Meter_Reception_Commands
{
    ZCL_MeteringGetProfile                                              = 0x00,
    ZCL_MeteringRequestMirrorResponse                                   = 0x01,
    ZCL_MeteringMirrorRemoved                                           = 0x02
};

enum Meter_Cluster_Events
{
    ZCL_MeteringGetProfileEvent                                         = 0x50,
    ZCL_MeteringRequestMirrorResponseEvent                              = 0x51,
    ZCL_MeteringMirrorRemovedEvent                                      = 0x52,
    ZCL_MeteringGetProfileResponseEvent                                 = 0x53,
    ZCL_MeteringRequestMirrorEvent                                      = 0x54,
    ZCL_MeteringRemoveMirrorEvent                                       = 0x55
};

enum Get_Profile_Response_Status
{
    ZCL_MeteringIntervalSuccess                                         = 0x00,
    ZCL_MeteringUndefinedIntervalChannelRequested                       = 0x01,
    ZCL_MeteringInetervalChannelNotSupported                            = 0x02,
    ZCL_MeteringInvalidEndTime                                          = 0x03,
    ZCL_MeteringMorePeriodsRequestedThanCanBeReturned                   = 0x04,
    ZCL_MeteringNoIntervalsAvailableForTheRequestedTime                 = 0x05
};

enum Interval_Channel_Values
{
    ZCL_MeteringConsumptionDelivered                                    = 0x00,
    ZCL_MeteringConsumptionReceived                                     = 0x01
};

enum ProfileIntervalPeriod
{
    ZCL_MeteringDaily                                                   = 0x00,
    ZCL_MeteringSixtyMinutes                                            = 0x01,
    ZCL_MeteringThirtyMinutes                                           = 0x02,
    ZCL_MeteringFifteenMinutes                                          = 0x03,
    ZCL_MeteringTenMinutes                                              = 0x04,
    ZCL_MeteringSevenAndHalfMinutes                                     = 0x05,
    ZCL_MeteringFiveMinutes                                             = 0x06,
    ZCL_MeteringTwoAndHalfMinutes                                       = 0x07
};

/*****************************************************************************
                          Customizable Macros
 *****************************************************************************/
 /* None */

/*****************************************************************************
                            Data Structures
 *****************************************************************************/
 /* Structure for holding Metering cluster attributes */
typedef struct __attribute__((packed,aligned(1))) _ZCL_MeteringClusterAttributes
{
    /* Reading Information Attribute Set */
    BYTE  currentSummationDelivered[6];

#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    BYTE  currentSummationReceived[6];
    BYTE  currentMaxDemandDelivered[6];
    BYTE  currentMaxDemandReceived[6];
    BYTE  DFTSummation[6];
    WORD  dailyFreezeTime;
    SBYTE powerFactor;
    DWORD readingSnapShotTime;
    DWORD currentMaxDemandDeliveredTime;
    DWORD currentMaxDemandReceivedTime;
#endif

    /* TOU Information Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    BYTE currentTier1SummationDelivered[6];
    BYTE currentTier1SummationReceived[6];
    BYTE currentTier2SummationDelivered[6];
    BYTE currentTier2SummationReceived[6];
    BYTE currentTier3SummationDelivered[6];
    BYTE currentTier3SummationReceived[6];
    BYTE currentTier4SummationDelivered[6];
    BYTE currentTier4SummationReceived[6];
    BYTE currentTier5SummationDelivered[6];
    BYTE currentTier5SummationReceived[6];
    BYTE currentTier6SummationDelivered[6];
    BYTE currentTier6SummationReceived[6];
#endif

    /* Meter Status Attribute Set */
    BYTE meterStatus;

    /* Formatting Attribute Set */
    BYTE  unitofMeasure;
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    BYTE multiplier[3];
    BYTE divisor[3];
#endif
    BYTE  summationFormatting;
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    BYTE  demandFormatting;
    BYTE  historicalConsumptionFormatting;
#endif
    BYTE  meteringDeviceType;

    /* ESP Historical Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    BYTE instantaneousDemand[3];
    BYTE currentDayConsumptionDelivered[3];
    BYTE currentDayConsumptionReceived[3];
    BYTE previousDayConsumptionDelivered[3];
    BYTE previousDayConsumptionReceived[3];
    DWORD currentPartialProfileIntervalStartTimeDelivered;
    DWORD currentPartialProfileIntervalStartTimeReceived;
    BYTE currentPartialProfileIntervalValueDelivered[3];
    BYTE currentPartialProfileIntervalValueReceived[3];
#endif

    /* Load Profile Configuration Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    BYTE maxNumberOfPeriodsDelivered;
#endif

    /* Supply Limit Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    BYTE currentDemandDelivered[3];
    BYTE demandLimit[3];
    BYTE demandIntegrationPeriod;
    BYTE numberOfDemandSubintervals;
#endif

} ZCL_MeteringClusterAttributes;


typedef struct _ZCL_MeteringIntervals
{
    BYTE metringInterval[3];
} ZCL_MeteringIntervals;

typedef struct __attribute__((packed,aligned(1))) _ZCL_MeteringGetProfileRspCmdPayload
{
    DWORD endTime;
    BYTE status;
    BYTE profileIntervalPeriod;
    BYTE numberOfPeriodsDelivered;
    ZCL_MeteringIntervals intervals[1];
} ZCL_MeteringGetProfileRspCmdPayload;

typedef struct __attribute__((packed,aligned(1))) _ZCL_MeteringGetProfileReqCmdPayload
{
    BYTE intervalChannel;
    DWORD endTime;
    BYTE numberOfPeriods;
} ZCL_MeteringGetProfileReqCmdPayload;

typedef struct _ZCL_MeteringRequestMirrorResponseCmdPayload
{
    WORD endPointId;
} ZCL_MeteringRequestMirrorResponseCmdPayload;
typedef struct _ZCL_MeteringMirrorRemovedCmdPayload
{
    WORD endPointId;
} ZCL_MeteringMirrorRemovedCmdPayload;

/*****************************************************************************
                          Variable definitions
 *****************************************************************************/
extern ROM ZCL_ClusterDefinition zcl_MeteringClusterServerDefinition;
extern ROM ZCL_ClusterDefinition zcl_MeteringClusterClientDefinition;

extern ROM ZCL_ClusterInfo zcl_MeteringClusterServerInfo;
extern ROM ZCL_ClusterInfo zcl_MeteringClusterClientInfo;

/*****************************************************************************
                          Function Prototypes
 *****************************************************************************/

 /******************************************************************************
 * Function:        BYTE ZCL_HandleMeteringClusterCommand
 *                  (
 *                      BYTE endPointId,
 *                      BYTE asduLength,
 *                      BYTE* pReceivedAsdu,
 *                      BYTE* pResponseAsduStartLocation,
 *                      ZCL_ActionHandler *pActionHandler,
 *                      ROM ZCL_ClusterInfo *pZCLClusterInfo
 *                  )
 *
 * PreCondition:    None
 *
 * Input:           endPointId -    EndPoint on which the Basic Cluster command needs
 *                                  to be processed
 *                  asduLength -    Asdu Length of the recevied frame
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and Payload.
 *                  pZCLClusterInfo - Pointer pointing to the Price Cluster info
 *
 * Output:          pResponseAsduStartLocation - Pointer pointing to the start
 *                                               location of asdu for which the
 *                                               response frame needs to be created
 *                  pActionHandler -   This is used to notify the application that a
 *                                     response frame is generated or not.
 *                                     It also holds the information of the type event
 *                                     to be handled by the application and has the
 *                                     payload of the recevied data.
 *
 * Return :         Returns the length of the response frame created
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when a Metering Cluster command is received.
 *                  This function process the Metering Cluster command and notifies the
 *                  application about the action to be taken.
 *
 * Note:            None
 *
 *****************************************************************************/
BYTE ZCL_HandleMeteringClusterCommand
(
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler *pActionHandler,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);

#endif /* _ZCL_METERING_H_ */
