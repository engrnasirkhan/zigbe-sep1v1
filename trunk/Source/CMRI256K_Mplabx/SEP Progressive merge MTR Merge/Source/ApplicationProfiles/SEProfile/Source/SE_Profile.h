/******************************************************************************
 *
 *                  SE Profile
 *
 ******************************************************************************
 * FileName:        SE_Profile.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v2.05 or higher
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
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      07/04/09

 *****************************************************************************/

#ifndef _SE_PROFILE_H_
#define _SE_PROFILE_H_

/*****************************************************************************
  Includes
 *****************************************************************************/
#include "ZCL_Utility.h"
#include "ZCL_Interface.h"
#include "ZCL_Commissioning.h"
#include "SE_Interface.h"
/*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/

#define MAX_ADDR_LENGTH                             0x08
#define SHORT_ADDRESS_LENGTH                        0x02

#define HIGH_SECURITY                               0x03
#define NETWORK_SECURITY                            0x02
#define NO_SECURITY                                 0x00
#define NETWORK_SECURITY_WITH_APS_ACK               0x06

#if (I_SUPPORT_REPORTING == 1)
    #define REPORTING_TABLE_SIZE                    0x05
#endif

#define COMMISSIONING_CLUSTER_RESPONSE_ASDU_LENGTH  0x04
#define SILENT_START_NETWORK                        0x00
#define FORM_NETWORK                                0x01
#define REJOIN_NETWORK                              0x02
#define ASSOC_JOIN_NETWORK                          0x03

#define INVALID_CLUSTERID                           0xFFFF
#define INVALID_TIMESTAMP                           0xFFFFFFFF

#define BROADCAST_ADDRESS                           0xFFFF
#define BROADCAST_RX_ON_ADDRESS                     0xFFFD
#define BROADCAST_ALL_ROUTERS_ADDRESS               0xFFFC

#define SE_PROF_ID                                  {0x09, 0x01}
#define MANU_FACT_ID_MSB                            0x10
#define MANU_FACT_ID_LSB                            0x5f

#if defined(I_SUPPORT_GROUP_ADDRESSING)
	/* Defines the maximum size of group name given by the specification */
	#define MAX_SIZE_OF_GROUP_NAME						0x10
	/* Defines the size of the group id */
	#define SIZE_OF_GROUP_ID						    0x02
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1) 
	/* Defines the maximum lenght of the attribute data for one extension field */
	#define MAX_SCENES_ATTR_LENGTH							0x14
	
	/* Defines the maximum size of group name given by the specification */
	#define MAX_SIZE_OF_SCENE_NAME							0x10

	/* Defines the maximum scenes extension fields for one scene entry */
	#define MAX_EXT_FIELD_RECORDS							0x03
	
#endif /*I_SUPPORT_SCENES*/

enum Threshold
 {
    MinThreshold = 0x00,
    MaxThreshold = 0x01
 };
 enum
 {
     Alarm_Inactive   = 0x00,
     Alarm_Active     = 0x01,
     Alarm_Clear      = 0xFF
 };

enum
 {
     ReadAttribute   		= 0x00,
     WriteAttribute     	= 0x01
 };

#if (I_SUPPORT_SCENES == 1)
enum
 {
     Get_SceneCount   			= 0x00,
     Increment_SceneCount     	= 0x01,
     Decrement_SceneCount      	= 0x02,
	 Reset_SceneCount			= 0x03
 };

enum
 {
     StoreScene   		= 0x00,
     RecallScene     	= 0x01,
     CheckSceneValidity = 0x02
 };

enum
 {
     discrete   			= 0x00,
     Analog     			= 0x01
 };
#endif /*I_SUPPORT_SCENES*/

/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
#define ConvertHexToDecimal( value )    value - \
                                        ( ( ( value & 0xF0 ) >> 4 ) * 0x06 )

#define ConvertDecimalToHex( value )    ( ( ( value / 10 ) * 16 ) + \
                                        ( value % 10 ) )



#if (I_SUPPORT_REPORTING == 1)
    #if (APP_USE_EXTERNAL_NVM == 1)
        #define GetReportingBgTasks(x)      NVMRead( (BYTE *)x, ReportingBackgroundIndex, sizeof(Reporting_Background_Tasks))
        #define PutReportingBgTasks(x)      NVMWrite( ReportingBackgroundIndex, (BYTE *)x, sizeof(Reporting_Background_Tasks))
    
        #define GetReportCfgTable(x)        NVMRead( (BYTE *)x, ReportCfgTableIndex, ( (sizeof(ReportConfiguration)) * REPORTING_TABLE_SIZE ))
        #define PutReportCfgTable(x)        NVMWrite( ReportCfgTableIndex, (BYTE *)x, (sizeof(ReportConfiguration) * REPORTING_TABLE_SIZE ))
    #else
        #define GetReportingBgTasks(x)      /**/ /*This is a dummy function used when ReportingTable is stored in RAM*/
        #define PutReportingBgTasks(x)      /**/ /*This is a dummy function used when ReportingTable is stored in RAM*/
    
        #define GetReportCfgTable(x)        /**/ /*This is a dummy function used when ReportingTable is stored in RAM*/
        #define PutReportCfgTable(x)        /**/ /*This is a dummy function used when ReportingTable is stored in RAM*/
    #endif //(APP_USE_EXTERNAL_NVM == 1)
#endif //(I_SUPPORT_REPORTING == 1)

#if defined(I_SUPPORT_GROUP_ADDRESSING)
	#if (APP_USE_EXTERNAL_NVM == 1)
		/* Stores the AppGroupsNameTable entry in the NVM */
		#define PutAppGroupNameAddress(x, y)           	NVMWrite(x, (BYTE *)y, sizeof(App_GroupNameRecord))
		/* Gets the AppGroupsNameTable entry from NVM */
		#define GetAppGroupNameAddress(x, y)           	NVMRead( (BYTE *)x, y, sizeof(App_GroupNameRecord))
	#else
		/* Stores the AppGroupsNameTable entry in the RAM */
		#define PutAppGroupNameAddress(x, y)  			memcpy ( (BYTE *)x, (BYTE *)y, sizeof(App_GroupNameRecord))
		/* Gets the AppGroupsNameTable entry from RAM */
		#define GetAppGroupNameAddress(x, y)           memcpy ( (BYTE *)x, (BYTE *)y, sizeof(App_GroupNameRecord))
	#endif /*APP_USE_EXTERNAL_NVM == 1*/
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)
	#if (APP_USE_EXTERNAL_NVM == 1)
		/* Stores the Scenes Table entry in the NVM */
		#define PutSceneTableEntry(x, y)           	NVMWrite(x, (BYTE *)y, sizeof(App_ScenesRecord))
		/* Gets the Scenes Table entry from NVM */
		#define GetSceneTableEntry(x, y)           	NVMRead( (BYTE *)x, y, sizeof(App_ScenesRecord))
	#else
		/* Stores the Scenes Table entry in the RAM */
		#define PutSceneTableEntry(x, y)  			memcpy ( (BYTE *)x, (BYTE *)y,sizeof(App_ScenesRecord))
		/* Gets the Scenes Table entry from RAM */
		#define GetSceneTableEntry(x, y)           	memcpy ( (BYTE *)x, (BYTE *)y,sizeof(App_ScenesRecord))
	#endif /*APP_USE_EXTERNAL_NVM == 1*/
#endif /*I_SUPPORT_SCENES*/

#define CBKE_SUCCESS                     0x00
#define CBKE_FAILURE                     0x01

/*****************************************************************************
  Data Structures
 *****************************************************************************/
typedef struct _App_AttributeStorage
{
    BYTE endpointId;
    WORD clusterId;
    BYTE length;
    WORD index;
    ROM void *defaultValueLocation;
} App_AttributeStorage;

#if defined(I_SUPPORT_GROUP_ADDRESSING)
	typedef struct __attribute__((packed,aligned(1))) _App_GroupNameRecord
	{
		BYTE groupName[MAX_SIZE_OF_GROUP_NAME];
	} App_GroupNameRecord;
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)
	typedef struct __attribute__((packed,aligned(1))) _ExtensionFieldSets
	{
		WORD clusterId;
		BYTE attributesLength;
		BYTE attributeData[MAX_SCENES_ATTR_LENGTH];	
	}ExtensionFieldSets;

	typedef struct __attribute__((packed,aligned(1))) _App_ScenesRecord
	{
		WORD groupId;
		BYTE sceneId;
		BYTE sceneName[MAX_SIZE_OF_SCENE_NAME];
		WORD sceneTransitionTime;
		ExtensionFieldSets extFieldRecord[MAX_EXT_FIELD_RECORDS];
	} App_ScenesRecord;

	typedef struct __attribute__((packed,aligned(1))) _Scenes_RecallCmdRecord
	{
		BYTE txnCount;
		BYTE endPoint;
		BYTE sceneTableIndex;
		BYTE sceneTxnTime;
		WORD sceneTxnTimeout;
		TICK sceneOneSecStartTime;
	} Scenes_RecallCmdRecord;

	typedef struct __attribute__((packed,aligned(1))) _Scene_ValidInfo
	{
		BYTE endPoint;
		BYTE sceneTableIndex;
		BYTE sceneValid;
	} Scene_ValidInfo;

#endif /*I_SUPPORT_SCENES*/

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
#if I_SUPPORT_CBKE == 1
extern BYTE keyEstablishmentEndpoint;
#endif /*#if I_SUPPORT_CBKE == 1*/
extern WORD mainsVoltageDwellTripPointValue;
extern BYTE MainsVoltageDwellTripPointHandled;
extern BYTE NoOfEntriesInAttrStorageTable;
extern App_AttributeStorage  App_AttributeStorageTable_MTR[];
extern App_AttributeStorage  App_AttributeStorageTable_ESP[];
extern App_AttributeStorage  *App_AttributeStorageTable;
extern BYTE *appNextSeqNum_PTR;
extern BYTE CommRestartTimerInProgress;
extern TICK CommRestartTime;
extern BOOL commissioningRestartInProgress;
extern ZCL_CommissioningRestartDeviceCmdPayload RestartDeviceCmdPayload;
#if I_SUPPORT_REPORTING == 1
extern Reporting_Background_Tasks ReportingBgTasks;
#endif /*I_SUPPORT_REPORTING*/

#if (I_SUPPORT_SCENES == 1)
	extern Scenes_RecallCmdRecord scenesRecallCmdRecord;
	extern Scene_ValidInfo sceneValidInfo;
#endif /*I_SUPPORT_SCENES*/
/*****************************************************************************
  Function Prototypes
 *****************************************************************************/

/*************************************************************************
  Function:
        BYTE SE_GetClusterSpecificSecurity(WORD clusterID)
    
  Description:
    This function is used to get the Security level used by the given
    Cluster ID.Generally ZCL Clusters use Network Layer Security and Smart
    Energy Profile Clusters will use Application Layer Security. So, it
    \returns either Network Layer Security or Application Layer Security
    according to the input parameter.
  Conditions:
    None
  Input:
    clusterID -  Cluster Id for which security level needs to be known
  Return:
    It returns Security level of the given clusterID.
  Example:
    None
  Remarks:
    None.                                                                 
  *************************************************************************/
BYTE SE_GetClusterSpecificSecurity(WORD clusterID);

/***************************************************************************************
  Function:
  void SE_SendDefaultResponse(   BYTE status, BYTE useNWKSecurity, APP_DATA_indication* p_dataInd);

  Description:
  The default response command is generated  when a device receives a unicast command, there is no other relevant response
  specified for the command, and either an error results or the Disable default response bit of its Frame control field is set to 0.
  Precondition:
           None
  Parameters:
    status        - This field indicates the status field of Default response command
    useNWKSecurity - it is a boolean variable.If it is true we need to send the Default Response Command  by using network layer
                security otherwise by using application layer security.
   p_dataInd     - It holds the APSDE_Data_Indication.By using this we can Send the default response command to the corresponding
                destination address.
  Returns:
        None

  Example:
  <code>
   </code>

  Remarks:
        None.
 ***************************************************************************************/
void SE_SendDefaultResponse
(
    BYTE status,
    BYTE useNWKSecurity,
    APP_DATA_indication* p_dataInd
);

/***************************************************************************************************
  Function:
      void SE_UpdateAlarmTable(WORD clusterId,BYTE alarmCode,DWORD timeStamp,BYTE event);

  Description:
    This function updates the alarmTable according the "event".
    a)If event is ZCL_Alarm_AlarmEvent,it updates(adds) alarm entry with given values.
    b)If event is ZCL_Alarm_ResetAlarmEvent,it resets the alarm entry.It means it make
      alarmEntry status as Inactive.
    c)If event is ZCL_Alarm_ResetAllAlarmsEvent,it resets the alarm entriers.It means it
      make alarmEntries status as Inactive.
    d)If event is ZCL_Alarm_ResetAlarmLogEvent,it clears the alarm table and alarmCount to Zero
  PreCondition:
        None
  Parameters:
    clusterId - this cluster id is added/Updated into alarmTable
    alarmCode - this alarm code is added/Updated into alarmTable
    timeStamp - this timeStamp is added/Updated into alarmTable
    event     - It specifies updation of alarmTable(Adding,deleting,Inactive)

  Return:
    None
  Example:
    <code>
    </code>
  Remarks:
    None.
***************************************************************************************************/
void SE_UpdateAlarmTable
(
    WORD clusterId,
    BYTE alarmCode,
    DWORD timeStamp,
    BYTE event
);

/***************************************************************************************************
  Function:
      BYTE SE_CreateGetAlarmResponse( BYTE endPoint,BYTE transactionSeqNum,BYTE *pResponseAsdu )

  Description:
    This function is used to Create GetAlarmResponse

  PreCondition:
        None
  Parameters:

    endPoint            -   this endpoint we need to send response
    transactionSeqNum   -   sequence number of the frame
    pResponseAsdu       -   asdu of the Get Alarm Response command.

  Return:
    None
  Example:
    <code>
    </code>
  Remarks:
    None.
***************************************************************************************************/
BYTE SE_CreateGetAlarmResponse( BYTE endPoint,BYTE transactionSeqNum,BYTE *pResponseAsdu );

/****************************************************************************************************
  Function:
       void App_WriteSingleAttribute(
          BYTE endpointId,WORD clusterId,WORD attributeId,BYTE attributeDataType,BYTE *pAttributeData
      )

  Description:
    This function is used to write an attributeData into NVM except if
    attribute data type is CharacterString or Octet Data String.
  Precondition:
    None
  Parameters:
    endpointId -        this is the endpoint whose attribute is to be  written
    clusterId -          this is the clusterId whose attribute is to be  written
    attributeId -      this is attributeID for which attribute data to be   written
    attributeDataType -  it indicates the datatype of attributeId
    pAttributeData -     points to the attributeData,this data is written on  NVM.

  Return:
        None
  Example:
    <code>

    </code>
  Remarks:
    None.
  ****************************************************************************************************/
void App_WriteSingleAttribute
(
    BYTE endpointId,
    WORD clusterId,
    WORD attributeId,
    BYTE attributeDataType,
    BYTE *pAttributeData
);

/****************************************************************************************************
  Function:
       void App_Reset(void)

  Description:
    Resets all the attributes of all the clusters, on all the endpoints supported on application.
  Precondition:
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
****************************************************************************************************/
  void App_Reset(void);

/****************************************************************************************************
  Function:
       void App_ResetVariables(void)

  Description:
    This function resets the application global variables.
  Precondition:
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
****************************************************************************************************/
void App_ResetVariables(void);

/****************************************************************************************************
  Function:
       void App_SetCurrentTime(elapsedTimeInfo* elapsedTime)

  Description:
    This function sets the RTC with the values as given by the input parameter.
  Precondition:
    None
  Parameters:
    elapsedTime - gives the time in the year, month,day,hour,min,sec format
  Return:
        None
  Example:
    <code>

    </code>
  Remarks:
    None.
****************************************************************************************************/
void App_SetCurrentTime
(
    elapsedTimeInfo* elapsedTime
);

/*************************************************************************
  Function:
          BYTE App_HandleReceivedData(APP_DATA_indication* p_dataInd);
    
  Description:
    This function is invoked by the Application when a ZCL frame is
    received. According to the ZCL frame (Generic or Cluster Specific)
    corresponding function handlers will be invoked.
  Conditions:
    None
  Input:
    p_dataInd -  It holds the received ZCL command frame.
  Return:
    if received ZCL frame is valid frame, it returns ZCL_SUCCESS otherwise
    ZCL_FAILURE
  Example:
    <code>
    
    </code>
  Remarks:
    None.                                                                 
  *************************************************************************/
BYTE App_HandleReceivedData(APP_DATA_indication* p_dataInd);

/****************************************************************************************************
  Function:
       void SE_CheckToSendReportAttributeCommand(void)

  Description:
    This function checks all the expired timers to see if report attribute need to be sent
  Precondition:
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
  ****************************************************************************************************/
#if I_SUPPORT_REPORTING == 1
void SE_CheckToSendReportAttributeCommand
(
    void
);
#endif /*I_SUPPORT_REPORTING == 1*/

/****************************************************************************************************
  Function:
       void SE_SendReportAttributeCommand( void )

  Description:
    This function allocates memory and consolidates the attributes into one single
    report attribute command and reports.
  Precondition:
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
  ****************************************************************************************************/
#if I_SUPPORT_REPORTING == 1
    void SE_SendReportAttributeCommand( void );
#endif /*I_SUPPORT_REPORTING*/

/****************************************************************************************************
  Function:
       void SE_UpdateReportReceived( APP_DATA_indication* p_dataInd )

  Description:
    This function checks if the received frame is report attribute command and updates the
    received report attribute flag accordingly.
  Precondition:
    None
  Parameters:
    p_dataInd - data indication consits of received report
  Return:
        None
  Example:
    <code>

    </code>
  Remarks:
    None.
  ****************************************************************************************************/
#if I_SUPPORT_REPORTING == 1
void SE_UpdateReportReceived( APP_DATA_indication* p_dataInd );
#endif /*I_SUPPORT_REPORTING*/

/**********************************************************************
  Function:
        WORD SE_GetAttributeIndex(
          WORD attributeId,ROM ZCL_ClusterInfo *pZCLClusterInfo,
          WORD ClusterStorageIndex,BYTE *pAttributeDataType
      )

  Description:
    This function is used to get NVM index of a given attributeId.
  Conditions:
    None
  Input:
    attributeId -          For this attributeId needs to get NVM Index.
    pZCLClusterInfo -      Points to ClusterInfo
    clusterStorageIndex -  Points to the cluster NVM Index.
    pAttributeDataType -   Points to the datatype of the given
                           attributeID.
  Return:
    Gives attribute NVM Index.
  Example:
    <code>

    </code>
  Remarks:
    None.
  **********************************************************************/
WORD SE_GetAttributeIndex
(
    WORD attributeId,
    ROM ZCL_ClusterInfo *pZCLClusterInfo,
    WORD ClusterStorageIndex,
    BYTE *pAttributeDataType
);


/****************************************************************************************************
  Function:
       void App_TimeInit()

  Description:
    Initializes the Default Time in RTC and Time Attribute in Time Cluster.
  Precondition:
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
  ****************************************************************************************************/
  void App_TimeInit();

/**************************************************************************
  Function:
         BYTE App_InitAttributeStorage( void )
    
  Description:
    This function is used for allocating the memory in NVM that is required
    for storing Attribute data for each clusters.
  Conditions:
    None
  Return:
    It returns whether the attribute storage allocation in NVM is done
    successfully or not.
  Example:
    <code>
    
    </code>
  Remarks:
    None.                                                                  
  **************************************************************************/
BYTE App_InitAttributeStorage( void );

/*********************************************************************
  Function:
       void App_UpdateParamsForTestTool(  APP_DATA_indication *dataInd )

  Description:
    This function parses the data indication from the lower layer
    and updates the global params structure.
  PreCondition:
    None
  Parameters:
   dataInd - data indication as received from the lower layer
  Return:
    It returns whether the attribute storage allocation in NVM is done successfully or not.
  Note:
   payload pointer is allocated memory here. This is freed by the Application( Test harness )
  Example:
    <code>

    </code>
  Remarks:
    None.
***********************************************************************/
void App_UpdateParamsForTestTool(  APP_DATA_indication *dataInd );

/*********************************************************************
  Function:
       void App_HandleDataIndication( void )

  Description:
    APSDE Data Indication is given to the test tool
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
***********************************************************************/
void App_HandleDataIndication( void );

/*********************************************************************************
  Function:
      void SE_HandleEvents( ZCL_Event *pZCLEvent, APP_DATA_indication* p_dataInd )
    
  Description:
    This function handles the events send by the ZCL to the Application.
    Application has to implement the corresponding code according to the
    event.
  Conditions:
    None
  Input:
    pZCLEvent -  This points to the ZCL Event.It consists of EventId and
                 EventData.
    p_dataInd -  It holds received data.
  Return:
    None
  Example:
    None
  Remarks:
    None.                                                                         
  *********************************************************************************/
 void SE_HandleEvents_MTR( ZCL_Event *pZCLEvent, APP_DATA_indication* p_dataInd );
 void SE_HandleEvents_ESP( ZCL_Event *pZCLEvent, APP_DATA_indication* p_dataInd );
 
 /*******************************************************************************
  Function:
    void SendZDPRequest( WORD_VAL DstAddr,WORD ClusterId,BYTE * Asdu,BYTE AsduLength )

  Description:
    Prepare the APP_DATA_Req and enqueue the same on to AIL
  PreCondition:
        None
  Parameters:
    DstAddr     -  ZDP request is sent to this address.
    ClusterId   -  ClusterId of the ZDP Request
    pAsdu       -  It consists of ZDP Request payload
    AsduLength  -  length of the ZDP Request
  Return:
        None
  Example:
    <code>

    </code>
  Remarks:
    None.
  *******************************************************************************/
void SendZDPRequest
(
     WORD_VAL DstAddr,
     WORD ClusterId,
     BYTE * pAsdu,
     BYTE AsduLength
 );
 
  void SendAPPLRequest
(
     WORD_VAL DstAddr,
     WORD ClusterId,
     BYTE * pAsdu,
     BYTE AsduLength
 );

/*******************************************************************************
  Function:
    BYTE SE_ValidateSAS(Commissioning_Cluster_Attributes *pcommAttribute)

  Description:
    validates the commissioning cluster attributes for consistency.
    This function will be called before restarting device,
    using ZCL commissioning cluster attributes.
  PreCondition:
        None
  Parameters:
    pcommAttribute- Commissioning cluster attributes.
  Return:
        None
  Example:
    <code>

    </code>
  Remarks:
    None.
  *******************************************************************************/
//#if (I_AM_ESP == 0)		//MSEB
BYTE SE_ValidateSAS(Commissioning_Cluster_Attributes *commAttribute);
//#endif /*(I_AM_ESP == 0)*/

/*******************************************************************************
  Function:
    void SE_HandleIdentifyTimeout( BYTE endpointId )

  Description:
    This function handles the Identification Procedure i.e Flashing(Toggling)of
    LED and decrementing the IdentifyTime value for every one second
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
  *******************************************************************************/
void SE_HandleIdentifyTimeout( BYTE endpointId );

/*******************************************************************************
  Function:
    void SE_SendCommissioningClusterResponse(
            APP_DATA_indication *p_dataInd,BYTE responseType, BYTE status
            )

  Description:
    Sends the commissioning cluster response depending on request received
    and status of processing the request.
  PreCondition:
        None
  Parameters:
      p_dataInd     - commissioning cluster request received command payload
      responseType  - Command for which commissioning cluster response needs to be sent
      status        - status of comm cluster command processing
  Return:
        None
  Example:
    <code>

    </code>
  Remarks:
    None.
*******************************************************************************/

//#if (I_AM_ESP == 0)		//MSEB
void SE_SendCommissioningClusterResponse
(
    APP_DATA_indication *p_dataInd,
    BYTE responseType,
    BYTE status
);
//#endif /*(I_AM_ESP == 0)*/


/*******************************************************************************
  Function:
    BYTE App_ValidateProfileId ( WORD profileID, BYTE endPoint )

  Description:
    This function is used to validate whether the specified end point supports
    the profile as specified in the received frame. If yes,then it returns TRUE.
    Otherwise it returns FALSE.
  PreCondition:
        None
  Parameters:
        profileID - Profile Id specified in the received frame that needs
                    to be matched.
        endPoint -  End Point as specified in the received frame that needs
                    to be matched
  Return:
        It returns TRUE when ProfileID is supported on the given endpoint else
        it returns FALSE.
  Example:
    <code>

    </code>
  Remarks:
    None.
*******************************************************************************/
BYTE App_ValidateProfileId ( WORD profileID, BYTE endPoint );

/****************************************************************************************************
  Function:
       void App_StartDevice(void)

  Description:
    This function optionally(if startUpMode = 0x00) adds Commissioning cluster
    attribute values into stack SAS and issues start device request.

  Precondition:
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
****************************************************************************************************/
//#if (I_AM_ESP == 0)		//MSEB
void App_StartDevice(void);
//#endif /*(I_AM_ESP == 0)*/

/****************************************************************************************************
  Function:
       void SE_SendNLMELeaveRequest(void)

  Description:
    On calling this function Device will leave the Network.
  Precondition:
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
****************************************************************************************************/
void SE_SendNLMELeaveRequest(void);

//#if ( I_AM_ESP == 1 )		//MSEB
/****************************************************************************************************
  Function:
       BYTE GetDisplayStateForMeterAttributes(WORD attributeId);

  Description:
    This function is used for getting the display state for the given attributeId.This is
    used for displyaing attribute value on LCD display On ESP device.

  Precondition:
    None
  Parameters:
    attributeId - for which we need to get display state.
  Return:
        It returns display state corresponding attributeID.
  Example:
    <code>

    </code>
  Remarks:
    None.
****************************************************************************************************/
BYTE GetDisplayStateForMeterAttributes( WORD attributeId );
//#endif

//#if ( I_AM_ESP == 1 )		//MSEB

/****************************************************************************************************
  Function:
       void DisplayForMeterFormattingAttributes(DWORD displayQuotient, BYTE* pRemainderArray,BYTE displayState)

  Description:
    This function is used to display meter attribute value using formatting attributes.
    Attribute value is stored in the form of Quotient and Remainder form.Attribute value
    is displayed in the following form
    <Quotient> . <Remainder>

  Precondition:
    None
  Parameters:
    displayQuotient - Quotient of Meter attribute value needs to be displayed
    pRemainderArray - Remainder of Meter attribute value needs to be displayed
    displayState    - it holds the display state.
  Return:
        It returns display state corresponding attributeID.
  Example:
    <code>

    </code>
  Remarks:
    None.
****************************************************************************************************/
void DisplayForMeterFormattingAttributes
(
    DWORD displayQuotient,
    BYTE* pRemainderArray,
    BYTE display
);
//#endif /*( I_AM_ESP == 1 )*/


/****************************************************************************************************
  Function:
       void HandleLeaveConfirmAndIndication( BYTE* pDeviceAddress)

  Description:
    This function updates the link key table based on the device address
  Precondition:
    None
  Parameters:
    pDeviceAddress - the address of the device that left the network
  Return:
        None
  Example:
    <code>

    </code>
  Remarks:
    None.
****************************************************************************************************/
#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1		//MSEB
void HandleLeaveConfirmAndIndication
(
    BYTE* pDeviceAddress
);
#endif /*#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1*/


/****************************************************************************************************
  Function:
       void AppInit()

  Description:
    Initializes the Application Specific Variable and Tables.
  Precondition:
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
****************************************************************************************************/
void AppInit();

/****************************************************************************************************
  Function:
       void Handle_Fragmented_MessageCluster(BYTE *data)

  Description:
    This function Display message is received From fragmentation.
  Precondition:
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
****************************************************************************************************/
void Handle_Fragmented_MessageCluster(BYTE *data);

/******************************************************************************
 * Function:        void CBKE_Callback_Handle_Status(BYTE status);
 *
 * PreCondition:    None
 *
 * Input:           status - Status of CBKE operation (CBKE_SUCCESS or CBKE_FAILURE)
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This is a callback function from CBKE. This function is called
 *                  by CBKE procedure upon Successful or Failure of establishing the 
 *                  Application Link Key.
 *
 *****************************************************************************/
#if (I_SUPPORT_CBKE == 0x01)
void CBKE_Callback_Handle_Status(BYTE status);
#endif

/******************************************************************************
 * Function:        BYTE CBKE_Callback_CheckForTCLinkKeyTableEntry(BYTE *DeviceIEEEAddress)
 *
 * PreCondition:    None
 *
 * Input:           DeviceIEEEAddress - IEEE address of the partner device of which 
 *                  entry needs to be added or verified in the table.
 *
 * Output:          None
 *
 * Return :         Returns TRUE if the space available for add an entry or entry 
 *                  is already existing otherwise FALSE.
 *
 * Side Effects:    None
 *
 * Overview:        This is a callback function from CBKE. This function is called
 *                  to verify the free entry in the TC link key table if new entry 
 *                  needs to be added or the entry is already existing. 
 *
 *****************************************************************************/
#if (I_SUPPORT_CBKE == 0x01)
BYTE CBKE_Callback_CheckForTCLinkKeyTableEntry(BYTE *DeviceIEEEAddress);
#endif

/*********************************************************************
 * Function:        void App_Handle_CBKE_Status(status)
 *
 * PreCondition:    None
 *
 * Input:           status - status of CBKE procedure
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Application handling, after the CBKE procedure.
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_CBKE == 0x01) 		//MSEB
void App_Handle_CBKE_Status(BYTE status);
#endif

#if (I_SUPPORT_SCENES == 1)
/******************************************************************************
 *	Function:   	App_ScenesRecord Handle_StoreAndRecallSceneCmd
 *					(
 *						BYTE endPointId,
 *    					BYTE sceneTableIndex,
 *						BYTE direction
 *					)
 *
 * PreCondition:    None
 *
 * Input:   	    endPointId - The end point id which has received the store scene or 
 *								 recall scene command or for which scene validity has to be checked.        
 *		    		sceneTableIndex - The index of the scene table entry for which scene has to be stored 
 *									  or recalled or scene validity has to be checked if current scene is 
 *									  valid scene.      
 *		    		direction  - The drection field will tell whether scene has to be stored or recalled 
 *								 or scene validity has to be checked.
 *
 * Output:          None 
 * Return :         It return the updated App_ScenesRecord type variable.
 * Side Effects:    None
 *
 * Overview:        This function  gets called when store scene or recall scene command is received or 
 *					when scene validity has to be checked. I the direction is store scene, then it will 
 *					update the extension fields with current attributes. If the direction is recall scene, 
 *					then it will set its attributes with the values of extension fields. If the direction is 
 *					check scene validity, then it will check whether current scene is valid, if so, then it 
 *					compares the attribute values in the extension fields with the current attribue values. 
 *					If any value differs, thenn it makes the scene valid flag and scene valid attribute to false.
 *****************************************************************************/
App_ScenesRecord Handle_StoreAndRecallSceneCmd 
( 
	BYTE endPointId, 
	BYTE sceneTableIndex, 
	BYTE direction  
);

#endif /*I_SUPPORT_SCENES*/

#endif /* _SE_PROFILE_H_ */
