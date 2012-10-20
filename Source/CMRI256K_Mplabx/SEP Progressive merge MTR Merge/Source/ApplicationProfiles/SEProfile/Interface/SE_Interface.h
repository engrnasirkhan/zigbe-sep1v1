/*********************************************************************
 *
 *                  Application Interface File
 *
 *********************************************************************
 * FileName        : SE_Interface.h
 * Dependencies    :
 * Processor       : PIC18 / PIC24 / dsPIC33
 * Complier        : MCC18 v3.00 or higher
 *                   MCC30 v2.05 or higher
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
#ifndef _SE_INTERFACE_H
#define _SE_INTERFACE_H

/****************************************************************************
   Includes
 *****************************************************************************/
#include "zigbee.def"
#include "ZCL_Utility.h"
#include "zAIL.h"

/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/
#define App_GetMaxPayloadLength()           ZCL_Utility_GetMaxPayloadLength()
#define SIZE_OF_UTC_TIME                    0x04
#define INVALID_END_POINT                   0xFF
//#if I_AM_ESP == 1
    
//#endif /*I_AM_ESP*/
#if I_AM_IPD == 1
    #define SOURCE_ENDPOINT                 IPD_ENDPOINT_ID
#endif /*I_AM_IPD*/
//#if I_AM_MTR == 1
//    #define SOURCE_ENDPOINT                 METER_ENDPOINT_ID
//#endif /*I_AM_MTR*/
#if I_AM_LCD== 1
    #define SOURCE_ENDPOINT                 LCD_ENDPOINT_ID
#endif /*I_AM_LCD*/
#if I_AM_PCT == 1
    #define SOURCE_ENDPOINT                 PCT_ENDPOINT_ID
#endif /*I_AM_PCT*/
#if I_AM_RED == 1
    #define SOURCE_ENDPOINT                 RED_ENDPOINT_ID
#endif /*I_AM_RED*/
#if I_AM_SAP == 1
    #define SOURCE_ENDPOINT                 SAP_ENDPOINT_ID
#endif /*I_AM_SAP*/

#define NO_SECURITY_WITHOUT_APS_ACK         0x00
#define NETWORK_SECURITY_WITHOUT_APS_ACK    0x02
#define HIGH_SECURITY_WITHOUT_APS_ACK       0x03
#define NO_SECURITY_WITH_APS_ACK            0x04
#define NETWORK_SECURITY_WITH_APS_ACK       0x06
#define HIGH_SECURITY_WITH_APS_ACK          0x07


/*****************************************************************************
   Customizable Macros
 *****************************************************************************/
#define MAX_NUMBER_OF_ENDPOINTS_SUPPORTED   NUM_USER_ENDPOINTS
#define SE_PROFILE_ID                       0x0109

/*****************************************************************************
                            Data Structures
 *****************************************************************************/


typedef struct _App_IdentifyTimer
{
    BYTE IdentifyTime :1;
    TICK IdentifyStartTime;
} App_IdentifyTimer;
typedef struct _App_PowerConfigMainsVoltageTimer
{
    BYTE DwellTripPoint :1;
    TICK DwellTripPointStartTime;
} App_PowerConfigMainsVoltageTimer;

typedef struct _Meter_Status
{
    BOOL ESP_HasCapability;
    BYTE ESP_EndPoint;
    BOOL deviceMirroring;
} Meter_Status;


typedef union __attribute__((packed,aligned(1))) dataTypeLength
{
    BYTE    byteVal;
    WORD    wordVal;
    DWORD   dwordVal;
    BYTE    arrayVal[6];
}dataTypeLength;

/*structure - maintaining server device for sending report
attribute command */
typedef struct __attribute__((packed,aligned(1))) ReportConfiguration
{
    BYTE destinationAddressMode;
    ADDR DstAddress;
    BYTE destinationEndpoint;
    BYTE srcEndPoint;
    WORD ClusterId;
    BYTE direction;
    WORD attributeId;
    BYTE attributeDataType;
    WORD minimumReportingInterval;
    WORD maximumReportingInterval;
    WORD storageIndex;
    BYTE SendReportFlag;
    WORD timeout;
    BYTE receivedReportAttribute;
    dataTypeLength previousReportedAttributeData;
    /*considering the maximum size as 6 bytes*/
    dataTypeLength reportableChange;
}ReportConfiguration;


#if I_SUPPORT_REPORTING == 1
/*Status Fields used for Meter. This will be used as a Background task.*/
typedef struct __attribute__((packed,aligned(1))) _Reporting_Background_Tasks
{
    union _Reporting_Status_flags
    {
        BYTE    Val;
        struct __attribute__((packed,aligned(1))) __Reporting_Background_Tasks_bits
        {
            BYTE    bValidReportEntryConfigured     : 1;
            BYTE    bSendreportAttributeCommand     : 1;
            BYTE    bAwaitingReportAttributeConfirm : 1;
            BYTE    bAwaitingReportAttributeCommand : 1;
        } bits;
    } flags;
    /*value is in seconds*/
    WORD consolidatedMinTime;
    WORD consolidatedMaxTime;
    TICK minStartTime;
    BYTE minReportExpired;
    TICK maxStartTime;
    BYTE maxReportExpired;
    WORD timeOut;
    TICK timeOutStartTime;
    BYTE timeOutExpired;
}Reporting_Background_Tasks;
#endif /*I_SUPPORT_REPORTING*/

/*This structure holds the information of the device to which the
Inter-PAN data needs to be transmitted.*/
typedef struct __attribute__((packed,aligned(1))) _InterPANDataRequest
{
    BYTE    destAddrMode;
    BYTE    destAddr[8];
    WORD    destInterPANId;
    WORD    profileId;
    WORD    clusterID;
    BYTE    asduHandle;
    BYTE    asduLength;
    BYTE    *pAsdu;
}InterPANDataRequest;

/*****************************************************************************
   Variable definitions
 *****************************************************************************/
extern ZCL_DeviceInfo *pAppListOfDeviceServerInfo_ESP[];
extern ZCL_DeviceInfo *pAppListOfDeviceServerInfo_MTR[];
extern ZCL_DeviceInfo *pAppListOfDeviceClientInfo_ESP[];
extern ZCL_DeviceInfo *pAppListOfDeviceClientInfo_MTR[];
/* This Timer is used for handling IdentifyTime attribute. */
extern App_IdentifyTimer appIdentifyTimers;
extern InterPANDataRequest interPANDataRequest;
extern App_PowerConfigMainsVoltageTimer appPowerConfigMainsDwellTrip;
/*****************************************************************************
  Function Prototypes
 *****************************************************************************/


/******************************************************************************
  Function:
         BYTE App_Set_SystemTime(
            BYTE year,BYTE month,BYTE day,BYTE hour,BYTE minute,BYTE second
        );

  Description:
    This function sets the time given from the Application into the RTC. It
    also determines the seconds elapsed from January 2000 00 hours(UTC
    Time) and writes the value into the Time attribute of a Time
    cluster.This function will be used by ESP.
  Conditions:
    None
  Parameters:
    year -    Current year
    month -   current month
    day -     current day
    hour -    current hour
    minute -  current minute
    second -  current second
  Return:
    None
  Example:
    <code>

    </code>
  Remarks:
    None.
  *******************************************************************************************/
BYTE App_Set_SystemTime
(
     BYTE year,
     BYTE month,
     BYTE day,
     BYTE hour,
     BYTE minute,
     BYTE second
 );

#if I_SUPPORT_REPORTING == 1

/******************************************************************************
  Function:
         BYTE App_CheckIfReportReceived( void  )

  Description:
    This function checks if the report attribute is received.
  PreCondition:
    None
  Parameters:
    None
  Return:
    TRUE - if all the report attributes are received
    FALSE - even if one report attribute is not received
  Example:
    <code>

    </code>
  Remarks:
    None.
  *******************************************************************************************/
BYTE App_CheckIfReportReceived( void  );
/******************************************************************************
  Function:
         void App_ResetReportAttributeReceived( void )

  Description:
    This function resets the flag that is maintained for received reports
  PreCondition:
    None
  Parameters:
    None
  Return:
    None
  Example:
    <code>

    </code>
  Remarks:
    None.
*******************************************************************************************/
void App_ResetReportAttributeReceived( void );

#endif /*I_SUPPORT_REPORTING*/


/*******************************************************************************
  Function:
       void App_SendData(
          BYTE addressMode,BYTE* pDestAddress,BYTE destEndpoint,BYTE asduLength,
          WORD clusterId,BYTE* pAsdu
            )
    
  Description:
    This function is called when application wants to transmit the data.
    This function allocates memory for APP_DATA_Request, fill the payload
    values using the input parameters and queue it up for zAIL.
  Conditions:
    None
  Input:
    addressMode -   gives the address mode of the destination address
    pDestAddress -  gives the pointer to destination address.The length of
                    this field is based on the address mode.
    destEndpoint -  destination endpoint to whom the frame is sent
    asduLength -    length of the data including zcl header
    clusterId -     cluster id whose command is generated
    pAsdu -         pointing to the data. The length of this field is based on
                    the asduLength.
  Return:
    None
  Example:
    <code>
    
    </code>
  Remarks:
    None.                                                                       
  *******************************************************************************/
void App_SendData ( BYTE addressMode, BYTE* pDestAddress, BYTE destEndpoint,
    BYTE asduLength, WORD clusterId, BYTE* pAsdu );

/**********************************************************************
  Function:
        void App_ResetAttributeData( void );
    
  Description:
    This function is used to Reset all attributes of all the clusters
    supported on endpoint to its default values which is stored in ROM.
  Conditions:
    None
  Return:
    None
  Example:
    <code>
    
    </code>
  Remarks:
    None.                                                              
  **********************************************************************/
 void App_ResetAttributeData( void );

/**************************************************************************
  Function:
        BYTE App_SendPublishPriceCommandOnInterPANNetwork( void )
    
  Description:
    This function is called when a Get Current Price or Get Scheduled Price
    commands is recieved through InterPAN. This function is used to send
    the Publish Price command on InterPAN network
  Conditions:
    None
  Return:
    TRUE - If Publish Price cmd is transmitted. FALSE - If Publish Price
    cmd could not be transmitted
  Example:
    <code>
    
    </code>
  Remarks:
    None.                                                                  
  **************************************************************************/
BYTE App_SendPublishPriceCommandOnInterPANNetwork( void );

/***************************************************************************************
  Function:
      void App_SendDataOverInterPAN( void );

  Description:
  This function is called when data needs to be transmitted over the Inter-PAN network. This function updates the
  inter-PAN structure and sends the data.

  Precondition:
           None
  Parameters:
           None
   Returns:
        None
  Example:
  <code>
   </code>

  Remarks:
        None.
 ***************************************************************************************/
void App_SendDataOverInterPAN( void );
#endif
