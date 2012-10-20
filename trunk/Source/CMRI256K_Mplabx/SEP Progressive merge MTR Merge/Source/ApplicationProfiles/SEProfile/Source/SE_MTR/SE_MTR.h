/*********************************************************************
 *
 *                  Simple Metering device Header File
 *
 *********************************************************************
 * FileName        : SE_MTR.h
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
#ifndef _SE_MTR_H
#define _SE_MTR_H

/*****************************************************************************
  Includes
 *****************************************************************************/
 /* None */
#include "ZCL_Message.h"
#include "ZCL_Price.h"
#include "SE_ESP.h"
 /*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
#define METER_ENDPOINT_ID               0x01

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

#define NUMBER_OF_SERVER_CLUSTERS_SUPPORTED_MTR    0x07 + CBKE_CLUSTER_SUPPORT + GROUPS_CLUSTER_SUPPORT + SCENES_CLUSTER_SUPPORT
#define NUMBER_OF_CLIENT_CLUSTERS_SUPPORTED_MTR    0x08 + CBKE_CLUSTER_SUPPORT + GROUPS_CLUSTER_SUPPORT + SCENES_CLUSTER_SUPPORT

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
#define METERING_CLUSTER_SERVER_ATTRIBUTE_STORAGE               sizeof(ZCL_MeteringClusterAttributes)
#define COMMISSIONING_CLUSTER_SERVER_ATTRIBUTE_STORAGE          sizeof(Commissioning_Cluster_Attributes)
#define POWER_CONFIGURATION_CLUSTER_SERVER_ATTRIBUTE_STORAGE    sizeof(ZCL_PowerConfigurationClusterAttributes)
#define ALARM_CLUSTER_SERVER_ATTRIBUTE_STORAGE                  sizeof(ZCL_AlarmClusterAttributes)

#define TOTAL_SERVER_ATTRIBUTE_STORAGE_MTR                  IDENTIFY_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        BASIC_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        GROUPS_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        SCENES_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        METERING_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        COMMISSIONING_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        POWER_CONFIGURATION_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        ALARM_CLUSTER_SERVER_ATTRIBUTE_STORAGE +\
                                                        CBKE_CLUSTER_SERVER_ATTRIBUTE_STORAGE 

#define TOTAL_CLIENT_ATTRIBUTE_STORAGE                  CBKE_CLUSTER_CLIENT_ATTRIBUTE_STORAGE
                                                        
#define TOTAL_ATTRIBTUE_STORAGE_MTR                         TOTAL_SERVER_ATTRIBUTE_STORAGE_MTR + TOTAL_CLIENT_ATTRIBUTE_STORAGE

/*Max Tiers supported for this device*/
#define MAX_TIERS_SUPPORTED             0x06
/*Max number of events (Price Tier Information) that the device can store*/
#define MAX_PRICE_EVENTS                0x06

//#define DEFAULT_YEAR            0x09
//#define DEFAULT_MONTH           0x06
//#define DEFAULT_DAY             0x12
#define DEFAULT_HOUR            0x00
#define DEFAULT_MINUTE          0x00
#define DEFAULT_SECOND          0x00
#define METER_DEVICE_ID         {0x01, 0x05}
#define METER_ID_DEVICE         0x0501
 /*****************************************************************************
  Data Structures
 *****************************************************************************/
/*Status Fields used for METER. This will be used as a Background task.*/
typedef struct _METER_STATUS
{
    union _METER_STATUS_flags
    {
        BYTE    Val;
        struct _METER_STATUS_bits
        {
            // Background Task Flags
            BYTE    bSimulateMeterReading        : 1;
        } bits;
    } flags;
    TICK simulateMeterStartTime;

} METER_STATUS;

/*This structure holds the Tier Information*/
//typedef struct PriceEvents
//{
//    DWORD   issuerEventId;
//    DWORD   UTCStartTime;
//    WORD    durationInMinutes;
//    DWORD   startTime;
//    BYTE    tierInfo;
//    BYTE    rateLabel[12];
//    DWORD   price;
//    BYTE    priceTrailingDigitAndPriceTier;
//}PriceEvents;



/*Structure maintained by Meter device to display the message
confirmationRequired - indicates if the message confirmation is required
destaddress - destination address to whom confirmation need to be sent
destEndPoint - destination endpoint to whom the confirmation is sent.
ZCL_Message_DisplayMessageCmdPayload - this is the payload for display message
*/
//typedef struct displayMessages
//{
//    BYTE confirmationRequired;
//    WORD destaddress;
//    BYTE destEndPoint;
//    BYTE sequenceNumber;
//    BYTE txOptions;
//    ZCL_Message_DisplayMessageCmdPayload displayMessage;
//}displayMessages;

/*****************************************************************************
 Variable definitions
*****************************************************************************/
extern BYTE numberOfPriceEvents;
extern BYTE numberofTiersSupported;

#if I_SUPPORT_REPORTING == 1
extern Reporting_Background_Tasks ReportingBgTasks;
#endif /*I_SUPPORT_REPORTING*/

/*METER_STATUS is used for meterSimulation.*/
extern METER_STATUS meterStatus;
extern displayMessages displayMessagesTable;
extern ZCL_Price_PublishPriceInfo PublishPriceInfo;
extern PriceEvents ListOfPriceEvents[];

 /*****************************************************************************
  Function Prototypes
 *****************************************************************************/

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
void App_HandlePendingData_MTR ( void );

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
void App_UpdateDataConfTable_MTR( APP_DATA_confirm *dataConf );

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
void APP_HandleInterPANDataConfirm_MTR ( void );
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
BYTE APP_HandleInterPANDataIndication_MTR( void );
#endif

/*********************************************************************
 * Function:        void TH_SendDataRequest(APP_DATA_request *pAppDataReq)
 *
 * PreCondition:    None
 *
 * Input:           pAppDataReq - Asdu and transmission parameters in
 *                                 the form of APP_DATA_request.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
void TH_SendDataRequest(APP_DATA_request *pAppDataReq);

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
void App_UpdatePriceEvents_MTR ( void );

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
void App_Message_Tick ( void );

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
void App_PriceInit_MTR();

/***************************************************************************************
 * Function:        void App_MessageTableInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the Message Table
 *
 * Note:            None
 ***************************************************************************************/
void App_MessageTableInit( void );

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
void App_Meter_ProfileIntervalTableInit(void);

#endif /*_SE_MTR_H*/
