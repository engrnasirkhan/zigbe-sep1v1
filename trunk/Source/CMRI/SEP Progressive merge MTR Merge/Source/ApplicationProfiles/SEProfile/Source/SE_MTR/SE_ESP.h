/*********************************************************************
 *
 *                          SE ESP Header File
 *
 *********************************************************************
 * FileName:        SE_ESP.h
 * Dependencies:
 * Processor:       PIC24FJ128GA010
 * Complier:        MPLab C30 C
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

#ifndef _SE_ESP_H_
#define _SE_ESP_H_

/****************************************************************************
 Includes
*****************************************************************************/
#include "ZCL_Price.h"
#include "ZCL_Message.h"
#include "ZCL_DRLC.h"
#include "ZCL_Commissioning.h"
/*****************************************************************************
 Constants and Enumerations
*****************************************************************************/
#define ESP_ENDPOINT_ID     0x01

#if I_SUPPORT_CBKE == 1
#define CBKE_CLUSTER_SUPPORT    0x01
#else
#define CBKE_CLUSTER_SUPPORT    0x00
#endif

#ifdef I_SUPPORT_GROUP_ADDRESSING
#define GROUPS_CLUSTER_SUPPORT   0x01
#else
#define GROUPS_CLUSTER_SUPPORT   0x00
#endif

#if (I_SUPPORT_SCENES == 1)
#define SCENES_CLUSTER_SUPPORT   0x01
#else
#define SCENES_CLUSTER_SUPPORT   0x00
#endif

#define NUMBER_OF_SERVER_CLUSTERS_SUPPORTED_ESP    0x09 + CBKE_CLUSTER_SUPPORT + GROUPS_CLUSTER_SUPPORT + SCENES_CLUSTER_SUPPORT
#define NUMBER_OF_CLIENT_CLUSTERS_SUPPORTED_ESP    0x07 + CBKE_CLUSTER_SUPPORT + GROUPS_CLUSTER_SUPPORT + SCENES_CLUSTER_SUPPORT

/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
#if I_SUPPORT_CBKE == 1
#define CBKE_CLUSTER_SERVER_ATTRIBUTE_STORAGE                   sizeof(_ZCL_KeyEstClusterAttributes)
#define CBKE_CLUSTER_CLIENT_ATTRIBUTE_STORAGE                   sizeof(_ZCL_KeyEstClusterAttributes)
#else
#define CBKE_CLUSTER_SERVER_ATTRIBUTE_STORAGE                   0x00
#define CBKE_CLUSTER_CLIENT_ATTRIBUTE_STORAGE                   0x00
#endif

#ifdef I_SUPPORT_GROUP_ADDRESSING
#define GROUPS_CLUSTER_SERVER_ATTRIBUTE_STORAGE                 sizeof(ZCL_GroupsClusterAttributes)
#else
#define GROUPS_CLUSTER_SERVER_ATTRIBUTE_STORAGE                 0x00
#endif

#if (I_SUPPORT_SCENES == 1)
#define SCENES_CLUSTER_SERVER_ATTRIBUTE_STORAGE                 sizeof(ZCL_ScenesClusterAttributes)
#else
#define SCENES_CLUSTER_SERVER_ATTRIBUTE_STORAGE                 0x00
#endif

#define IDENTIFY_CLUSTER_SERVER_ATTRIBUTE_STORAGE               sizeof(ZCL_IdentifyClusterAttributes)
#define BASIC_CLUSTER_SERVER_ATTRIBUTE_STORAGE                  sizeof(ZCL_BasicClusterAttributes)
#define TIME_CLUSTER_SERVER_ATTRIBUTE_STORAGE                   sizeof(ZCL_TimeClusterAttribute)
#define PRICE_CLUSTER_SERVER_ATTRIBUTE_STORAGE                  sizeof(ZCL_PriceClusterAttribute)
#define METERING_CLUSTER_SERVER_ATTRIBUTE_STORAGE               sizeof(ZCL_MeteringClusterAttributes)
#define POWER_CONFIGURATION_CLUSTER_SERVER_ATTRIBUTE_STORAGE    sizeof(ZCL_PowerConfigurationClusterAttributes)
#define ALARM_CLUSTER_SERVER_ATTRIBUTE_STORAGE                  sizeof(ZCL_AlarmClusterAttributes)


#define TOTAL_SERVER_ATTRIBUTE_STORAGE_ESP                  IDENTIFY_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        BASIC_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        GROUPS_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        SCENES_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        TIME_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        PRICE_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        METERING_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        POWER_CONFIGURATION_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        ALARM_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        CBKE_CLUSTER_SERVER_ATTRIBUTE_STORAGE

#define TOTAL_CLIENT_ATTRIBUTE_STORAGE                  CBKE_CLUSTER_CLIENT_ATTRIBUTE_STORAGE

#define TOTAL_ATTRIBTUE_STORAGE_ESP                         TOTAL_SERVER_ATTRIBUTE_STORAGE_ESP + TOTAL_CLIENT_ATTRIBUTE_STORAGE

/*Max Tiers supported for this device*/
#define MAX_TIERS_SUPPORTED             0x06
/*Max number of events (Price Tier Information) that the device can store*/
#define MAX_PRICE_EVENTS                0x06

#define MAX_LOAD_CONTROL_EVENTS         0x03

#define DEFAULT_YEAR            0x0A
#define DEFAULT_MONTH           0x0A
#define DEFAULT_DAY             0x0C
#define DEFAULT_HOUR            0x00
#define DEFAULT_MINUTE          0x00
#define DEFAULT_SECOND          0x00
#define INVALID_DURATION        0xFFFF
#define INVALID_START_TIME      0xFFFFFFFF
#define ESP_DEVICE_ID           {0x00, 0x05}
#define ESP_ID_DEVICE           0x0500

#define INVALID_UTC_START_TIME  0xFFFFFFFF

#define METER_ATTRIBUTE_DISPLAY_TABLE_SIZE	25
/*****************************************************************************
 Data Structures
*****************************************************************************/
/*Status Fields used for ESP. This will be used as a Background task.*/
typedef struct PACKED _ESP_STATUS
{
    union PACKED _ESP_STATUS_flags
    {
        BYTE    Val;
        struct PACKED _ESP_STATUS_bits
        {
            // Background Task Flags
            BYTE    bAwaitingDataConfirm        : 1;
            BYTE    bPendingPublishPriceCmd     : 1;
            BYTE    bPendingLoadControlEventCmd : 1;
            BYTE    bPendingDisplayMessageCmd   : 1;
            BYTE    bDisplayMessageMemAllocated : 1;
            // Status Flags
        } bits;
    } flags;
  TICK loadControlEventTime;
  DWORD displayMessageCmdTime;
  DWORD GetScheduledEventStartTime;
} ESP_STATUS;

/*This structure holds the Tier Information*/
typedef struct PACKED _PriceEvents
{
    DWORD   issuerEventId;
    DWORD   UTCStartTime;
    WORD    durationInMinutes;
    DWORD   startTime;
    BYTE    tierInfo;
    BYTE    rateLabel[12];
    DWORD   price;
    BYTE    priceTrailingDigitAndPriceTier;
}PriceEvents;

/*This structure holds the information of the device to which the
Publish Price command needs to be transmitted.*/
typedef struct PACKED _PublishPriceCmdTxDetails
{
    BYTE    destAddrMode;
    BYTE    destAddr[8];
    WORD    destInterPANId;
    BYTE    InterPANMode;
    BYTE    destEndPoint;
    BYTE    transactionSeqNum;
    BYTE    currentEventIndex;
    BYTE    maxEventIndex;
    DWORD   startTime;
    ZCL_Price_PublishPriceInfo *PublishPriceCommandPayload;
}PublishPriceCmdTxDetails;

typedef struct PACKED _displayMessages
{
    WORD destaddress;
    BYTE destEndPoint;
    ZCL_Message_DisplayMessageCmdPayload* pDisplayMessage;
    BYTE confirmationRequired;
    BYTE sequenceNumber;
    BYTE txOptions;
    ZCL_Message_DisplayMessageCmdPayload displayMessage;
}displayMessages;

/*This structure holds the information of the device to which the
Load Control Event command needs to be transmitted.*/
typedef struct PACKED _LoadControlEventCmdTxDetails
{
    WORD    destAddr;
    BYTE    destEndPoint;
    BYTE    transactionSeqNum;
    BYTE    currentEventIndex;
    BYTE    maxEventIndex;
    DRLC_LoadControlEvent *LCECmdPayload;
}LoadControlEventCmdTxDetails;


typedef struct PACKED _MeterAttributes_DisplayState
{
    WORD attributeId;
    BYTE displayState;
}MeterAttributes_DisplayState;

typedef struct PACKED _Meter_FormattingAttributes
{
    BYTE unitOfMeasure;
    DWORD multiplier;
    DWORD divisor;
    BYTE summationFormatting;
    BYTE demandFormatting;
    BYTE historicalFormatting;

}Meter_FormattingAttributes;

/*****************************************************************************
 Variable definitions
*****************************************************************************/
extern BYTE numberOfPriceEvents;
extern BYTE numberofTiersSupported;
extern ZCL_CommissioningRestartDeviceCmdPayload RestartDeviceCmdPayload;

extern ESP_STATUS espStatus;

#if I_SUPPORT_REPORTING == 1
    extern Reporting_Background_Tasks ReportingBgTasks;
#endif /*I_SUPPORT_REPORTING*/

extern PriceEvents ListOfPriceEvents[];
extern ZCL_Price_PublishPriceInfo PublishPriceInfo;
extern ZCL_Price_PublishPriceInfo PublishPriceCommandPayloadDefaultValue;
extern PublishPriceCmdTxDetails PublishPriceRequestFrameValue;

extern ZCL_Message_DisplayMessageCmdPayload ZCL_Message_DisplayMessageTable;
extern displayMessages displayMessagesTable;

extern DRLC_LoadControlEvent LoadControlEventCommandPayloadDefaultValue[];
extern LoadControlEventCmdTxDetails LCEReqFrameVal;
/*****************************************************************************
 Function Prototypes
*****************************************************************************/


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
void App_UpdatePriceEvents_ESP ( void );

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
void App_HandlePendingData_ESP ( void );

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
void App_UpdateDataConfTable_ESP( APP_DATA_confirm *dataConf );

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
void APP_HandleInterPANDataConfirm_ESP ( void );
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
BYTE APP_HandleInterPANDataIndication_ESP( void );
#endif


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
void App_PriceInit();

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
void App_MessageTableInit( void );

#endif /*_SE_ESP_H_*/
