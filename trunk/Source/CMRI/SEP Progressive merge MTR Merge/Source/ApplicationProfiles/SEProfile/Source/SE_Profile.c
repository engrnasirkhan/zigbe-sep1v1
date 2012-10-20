/*********************************************************************
 *
 *                  SE Profile
 *
 *********************************************************************
 * FileName:        SE_Profile.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v1.05 or higher
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *****************************************************************************/

 /****************************************************************************
                                Includes
 *****************************************************************************/
/* Configuration files */
#include "zigbee.def"

/*Generic Files*/
#include "generic.h"
#include "sralloc.h"
#include "zigbee.h"
#include "SymbolTime.h"
#include "zNVM.h"
#include "HAL.h"
#include "MSDCL_Commissioning.h"

/*Stack Related Files*/
#include "ZigbeeTasks.h"
#include "zPHY.h"
#include "zMAC.h"
#include "zNWK.h"
#include "rtcc.h"
#include "zAPS.h"

/*ZCL Related Files*/
#include "ZCL_Interface.h"
#include "ZCL_Foundation.h"
#include "ZCL_Basic.h"
#include "ZCL_Identify.h"
#include "ZCL_Power_Configuration.h"
#include "ZCL_Alarm.h"
#include "ZCL_Time.h"
#include "ZCL_Commissioning.h"
#include "ZCL_Price.h"
#include "ZCL_DRLC.h"
#include "ZCL_Metering.h"
#include "ZCL_Message.h"
#if( I_AM_PCT == 1 )
    #include "ZCL_Temp_Measurement.h"
    #include "ZCL_Thermostat.h"
#endif /*I_AM_PCT*/
#if I_SUPPORT_CBKE == 1
    #include "ZCL_Key_Establishment.h"
#endif /*#if I_SUPPORT_CBKE*/

/*SE Related Files*/
#include "SE_Interface.h"
#include "SE_Profile.h"
#if I_SUPPORT_CBKE == 1
    #include "eccapi.h"
    #include "SE_CBKE.h"
#endif

//#if I_AM_ESP == 1		//MSEB
    #include "SE_ESP.h"
//#endif /*I_AM_ESP*/

#if I_AM_IPD == 1
    #include "SE_IPD.h"
#endif /*I_AM_IPD*/

#if ( I_AM_PCT == 1 )
    #include "SE_PCT.h"
#endif /*I_AM_PCT*/


#if ( I_AM_LCD == 1 )
    #include "SE_LCD.h"
#endif /*I_AM_LCD*/


//#if ( I_AM_MTR == 1 )		//MSEB
    #include "SE_MTR.h"
//#endif /*I_AM_MTR*/

#if ( I_AM_RED == 1 )
    #include "SE_RED.h"
#endif /*I_AM_RED*/

#if ( I_AM_SAP == 1 )
    #include "SE_SAP.h"
#endif /*I_AM_SAP*/

#include "SE_Display.h"

#if defined(I_SUPPORT_GROUP_ADDRESSING)
	#include "ZCL_Groups.h"
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)
	#include "ZCL_Scenes.h"
#endif /*I_SUPPORT_SCENES*/


/*Libraries*/
#if defined(__C30__)
    #include "TLS2130.h"
#else
    #include "lcd PIC32.h"
#endif

/*****************************************************************************
                        Constants and Enumerations
 *****************************************************************************/
/* Represents the no of supported end points */
#define NUMBER_OF_SUPPORTED_END_POINTS	0x01
	#if defined(I_SUPPORT_GROUP_ADDRESSING)
	
	/* Represents the size of the cluster id */	
	#define SIZE_OF_CLUSTER_ID				0x02

	/* Represents the size of status field */	
	#define SIZE_OF_STATUS					0x01
	
	/* Indicates names are supported */
	#define IS_NAMES_SUPPORTED				0x80
	
	/* Represents the value of the free entry */
	#define NO_GROUP_ID						0xFFFF
	
	/* Represents the value of the no cluster id */
	#define NO_CLUSTER_ID					0xFFFF
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)
	/* Represents the invalid scene id */
	#define INVALID_SCENE_ID				0xFF
	
	/* Represents the size of the transition time */
	#define LENGTH_OF_TNS_TIME				0x02
	
	/* Represents that scene extension are supported */
	#define SCENES_EXTENSION_SUPPORTED		0x04
	
	/* Represents the size of the scene id */
	#define SIZE_OF_SCENE_ID				0x01
	
	/* Defines the size of the scene table */
	#define MAX_SCENES						MAX_GROUP
	
	
	/* This macro is used to chek whether attribute is analog or digital */
	#define CheckIfAttrIsAnalog(attr)	( ( ( attr >= (0x20) && attr <= (0x2f) ) || \
											( attr >= (0x38) && attr <= (0x3f) ) || \
								( attr >= (0xe0) && attr <= (0xe7) ) ) ? 0x01:0x00 )


	/* This macro is used to chek whether attribute is signed or not */
	#define IsSignedInteger(attr)   ( ( attr >= (0x28) && attr <= (0x2f) ) ? 0x01:0x00 )
	
	/* This macro defines the key for the scenes table */
	#define SCENE_TABLE_KEY			0x1111
#endif /*I_SUPPORT_SCENES*/
/*****************************************************************************
                            Customizable Macros
 *****************************************************************************/
/* None */

/*****************************************************************************
   Data Structures
 *****************************************************************************/
typedef struct _Cluster_specific_security
{
    WORD clusterID;
    BYTE security;
} Cluster_specific_security;


typedef struct _Initiator_Device_Info
{
    BYTE initiatorAddressMode;
    ADDR initiatorLongAddress;
    SHORT_ADDR initiatorShortAddress;
    BYTE initiatorEndPoint;
} Initiator_Device_Info;


/*****************************************************************************
                                Variable Definitions
 *****************************************************************************/
 
#if (APP_USE_EXTERNAL_NVM != 0x01)
BYTE attributeDataStorageInRAM_ESP[TOTAL_ATTRIBTUE_STORAGE_ESP];
BYTE attributeDataStorageInRAM_MTR[TOTAL_ATTRIBTUE_STORAGE_MTR];
#endif

#if I_SUPPORT_CBKE == 1
/*this variable holds the key establishment endpoint.The CBKE procedure is
performed using this endpoint*/
BYTE keyEstablishmentEndpoint = SOURCE_ENDPOINT;
#endif /*#if I_SUPPORT_CBKE == 1*/
extern ZIGBEE_PRIMITIVE    currentPrimitive_MTR;
extern ZIGBEE_PRIMITIVE    currentPrimitive_ESP;

//#if ( I_AM_ESP == 0 )		//MSEB
    extern BYTE startMode;
    extern BYTE AllowJoin;
//#endif /*I_AM_ESP == 0*/

//#if (I_AM_ESP == 1)		//MSEB
    extern Meter_FormattingAttributes meterFormatAttributes;
    extern MeterAttributes_DisplayState meterAttributes_DisplayStateTable[];
//#endif  /*(I_AM_ESP == 1)*/

//#if (I_AM_ESP == 1)		//MSEB
    DWORD displayQuotient;
    BYTE remainderArray[10];
//#endif  /*(I_AM_ESP == 1)*/


#if I_SUPPORT_REPORTING == 1
    Reporting_Background_Tasks ReportingBgTasks;
#endif /*I_SUPPORT_REPORTING*/

    Initiator_Device_Info InitiatorDeviceInfo;


//#if( I_AM_MTR == 1 )		//MSEB
    /*METER_STATUS is used for meterSimulation.*/
    METER_STATUS meterStatus = { {0x00}, { 0x00000000 } };
//#endif

//#if ( I_AM_ESP == 0 )		//MSEB
    BOOL commissioningRestartInProgress = FALSE;
    ZCL_CommissioningRestartDeviceCmdPayload RestartDeviceCmdPayload;
    BYTE CommRestartTimerInProgress = FALSE;
    TICK CommRestartTime;
//#endif /*I_AM_ESP == 0*/

#if( I_AM_IPD == 1 )
extern BYTE TiermeterReadingValue[6];
#endif /* I_AM_IPD == 1 */

Cluster_specific_security clusterSecurityTable[] =
{
    { ZCL_BASIC_CLUSTER, NETWORK_SECURITY },
    { ZCL_IDENTIFY_CLUSTER, NETWORK_SECURITY },
    #ifdef I_SUPPORT_GROUP_ADDRESSING
        { ZCL_GROUPS_CLUSTER, NETWORK_SECURITY },
    #endif
    #if (I_SUPPORT_SCENES == 1)
        { ZCL_SCENES_CLUSTER, NETWORK_SECURITY },
    #endif
    { ZCL_ALARM_CLUSTER, NETWORK_SECURITY },
    { ZCL_TIME_CLUSTER, HIGH_SECURITY },
    { ZCL_PRICE_CLUSTER, HIGH_SECURITY },
    { ZCL_DRLC_CLUSTER, HIGH_SECURITY },
    { ZCL_METERING_CLUSTER, HIGH_SECURITY },
    { ZCL_MESSAGING_CLUSTER, HIGH_SECURITY },
    { ZCL_COMMISSIONING_CLUSTER, HIGH_SECURITY },
    { ZCL_POWER_CONFIGURATION_CLUSTER, NETWORK_SECURITY },
    { ZCL_ALARM_CLUSTER, NETWORK_SECURITY }
    #if( I_AM_PCT == 1 )
        ,
        { ZCL_TEMP_MEASUREMENT_CLUSTER, NETWORK_SECURITY },
        { ZCL_THERMOSTAT_CLUSTER, NETWORK_SECURITY}
    #endif
    #if I_SUPPORT_CBKE == 1
        ,
        { ZCL_KEY_ESTABLISHMENT_CLUSTER, NETWORK_SECURITY }
    #endif /*#if I_SUPPORT_CBKE*/
};

/*It holds the differnt clusters alarm condition entries*/
AlarmConditionsTable alarmConditionTableEntries[] =
{
    #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    {
      ZCL_POWER_CONFIGURATION_CLUSTER,
      ZCL_MAINS_VOLTAGE,
      ZCL_MAINS_VOLTAGE_MIN_THRESHOLD,
      ZCL_MAINS_ALARM_MASK,
      MainsVoltageTooLow,
      0x01,
      MinThreshold
    },
    {
      ZCL_POWER_CONFIGURATION_CLUSTER,
      ZCL_MAINS_VOLTAGE,
      ZCL_MAINS_VOLTAGE_MAX_THRESHOLD,
      ZCL_MAINS_ALARM_MASK,
      MainsVoltageTooHigh,
      0x02,
      MaxThreshold
    },
    {
      ZCL_POWER_CONFIGURATION_CLUSTER,
      ZCL_BATTERY_VOLTAGE,
      ZCL_BATTERY_VOLTAGE_MINIMUM_THRESHOLD,
      ZCL_BATTERY_ALARM_MASK,
      BatteryVoltageTooLow,
      0x01,
      MinThreshold
    }
    #endif
};

/*Alarm Table holds the alarmEntries.This table is updated when alarm conditions occurs.*/
ALARM_AlarmTable AlarmEntries[] =
{
    { Alarm_Clear,  0xFF,  INVALID_CLUSTERID,  INVALID_TIMESTAMP},
    { Alarm_Clear,  0xFF,  INVALID_CLUSTERID,  INVALID_TIMESTAMP},
    { Alarm_Clear,  0xFF,  INVALID_CLUSTERID,  INVALID_TIMESTAMP}
};

/*If the Alarm table is full, the entry with the earliest time stamp field should
  be replaced by the new Alarm entry comes.So,this variable used to indicate
  earliest time stamp entry index */
WORD mainsVoltageDwellTripPointValue;
BYTE MainsVoltageDwellTripPointHandled = 0x00;

/* This Timer is used for handling IdentifyTime attribute. */
App_IdentifyTimer appIdentifyTimers = { 0x00, { 0x00000000 } };
/* This Timer is used for handling MainsVoltageDwellTripPoint attribute. */
App_PowerConfigMainsVoltageTimer appPowerConfigMainsDwellTrip = {0x00,{0x0000}};

#if I_SUPPORT_REPORTING == 1

    #if (APP_USE_EXTERNAL_NVM == 1)
        WORD ReportingBackgroundIndex;
        WORD ReportCfgTableIndex;
    #endif // (APP_USE_EXTERNAL_NVM == 1)
    
    ReportConfiguration reportConfigurationTable[REPORTING_TABLE_SIZE] =
    {
        { 0xFF }
    };
#endif /*I_SUPPORT_REPORTING*/

#if defined(I_SUPPORT_GROUP_ADDRESSING)
    #if (APP_USE_EXTERNAL_NVM == 1)
		/* Variable to hold the starting location of the application group table in NVM */
    	WORD AppGroupsNameTable;
	#else
		/* Variable to hold the starting location of the application group table in RAM */
    	App_GroupNameRecord AppGroupsNameTable[MAX_GROUP];
	#endif /*APP_USE_EXTERNAL_NVM == 1*/
#endif/*I_SUPPORT_GROUP_ADDRESSING*/

#if (I_SUPPORT_SCENES == 1)

	/* Initialize the scene recall command record with transition count as one */
	Scenes_RecallCmdRecord scenesRecallCmdRecord = {0x01};

	/* Initialize the scene valid info record with null */
	Scene_ValidInfo sceneValidInfo = {0x00};

    #if (APP_USE_EXTERNAL_NVM == 1)

		/* Variable to hold the starting location of the application scenes table in NVM */
    	WORD App_ScenesTable;
	#else
		/* Variable to hold the starting location of the application scenes table in RAM */
    	App_ScenesRecord App_ScenesTable[MAX_SCENES];
	#endif /*APP_USE_EXTERNAL_NVM == 1*/
#endif/*I_SUPPORT_SCENES*/

static BYTE nextIndexToBeUpdated = 0x00;
static WORD identifyTimeoutVal = 0x0000;
/*****************************************************************************
                                Function Prototypes
 *****************************************************************************/

/******************************************************************************
* Function:        void Time_Synchronization( BYTE endpoint, BYTE* pSeconds );
*
* PreCondition:    This function is called when time synchronization happens
*
* Input:           endpoint - endpoint for which time synchronization is done
*
* Output:          pSeconds - seconds elapsed from January 2000 00 hours
*
* Side Effects:    None
*
* Return Value:    Success( 1 ) or failure( 0 )
*
* Overview:        This function sets the Time attribute.Also synchronizes the
*                  time by setting the values in RTC
*
* Note:
*****************************************************************************/
//#if ( I_AM_ESP == 0 )		//MSEB
 void Time_Synchronization( BYTE endpoint, BYTE* pSeconds );
//#endif /*I_AM_ESP*/

/******************************************************************************
* Function:         void FormatMeterReadingValue
*                   (
*                       BYTE *pmeterReadingValue,
*                       BYTE meterAttributeSize
*                   );
*
* PreCondition:     None
*
* Input:            pmeterReadingValue - Meter reading value to be formatted
*                   meterAttributeSize - Size of meter attribute.
*
* Output:           None
*
* Return :          None
*
* Side Effects:     None
*
* Overview:         Formats the meter reading values depending on
*                   formatting attributes.
*
* Note:             None
*****************************************************************************/
//#if (I_AM_ESP == 1)		//MSEB
static void FormatMeterReadingValue(BYTE *pmeterReadingValue,BYTE meterAttributeSize);
//#endif  /*(I_AM_ESP == 1)*/

/******************************************************************************
 * Function:        static void ResetAlarmEntries(void);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        Resets all the attributes of all the clusters, on all
 *                  the endpoints supported on application.
 *
 * Note:            None
 *****************************************************************************/
static void ResetAlarmEntries( void );

/******************************************************************************
* Function:        static void GenerateAlarm ( WORD clusterId,BYTE alarmCode);
*
* PreCondition:    None
*
* Input:           clusterId - This needs be added in alarm command
*                  alarmCode - This needs be added in alarm command
*
*
* Output:          None
*
* Return :         None
*
* Side Effects:    None
*
* Overview:        This function is used to generate an alarmCommand
*
* Note:            None
*****************************************************************************/
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
static void GenerateAlarm(WORD clusterId,BYTE alarmCode);
#endif

/******************************************************************************
 * Function:       static BYTE VerifyReportableChange( BYTE index )
 *
 * PreCondition:    None
 *
 * Input:           index - index to the report configuration table
 *
 * Output:          None
 * Return :
 *                  TRUE - if there is a change in the attribute data
 *                  FALSE - if there is no change in the attribute data
 * Side Effects:    None
 *
 * Overview:        This function checks if there is any change in the value
 *                  positive or negative. If there is a change, the function
 *                  returns TRUE else it returns FALSE
 *****************************************************************************/
#if I_SUPPORT_REPORTING == 1
static BYTE VerifyReportableChange( BYTE index );
#endif /*I_SUPPORT_REPORTING*/

/******************************************************************************
 * Function:       static void InitializeReportingTable( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function initializes the reporting table with 0xff
 *                  invalid values.
 *****************************************************************************/
#if I_SUPPORT_REPORTING == 1
static void InitializeReportingTable( void );
#endif  /*I_SUPPORT_REPORTING*/


#if (I_SUPPORT_SCENES == 1)
/******************************************************************************
 *	Function:   	void VerifySceneTableEntry
 *					(
 *    					BYTE groupId,
 * 						BYTE sceneId,
 *    					BYTE* psceneTableIndex
 *					)
 *
 * PreCondition:    None
 *
 * Input:           groupId  - The groupId for which the scene entry is to be checked.
 *					groupId - The scene id for which the scene entry is to be checked.
 *
 * Output:          psceneTableIndex - This point to the index of the scene table entry 
 *                                     whch is matched for group id and scene id.
 * Return :         None
 * Side Effects:    None
 *
 * Overview:        This function checks whether scene table entry exists for a requested 
 *                  group id and scene id.
 *****************************************************************************/
void VerifySceneTableEntry 
( 
	WORD groupId, 
	BYTE sceneId, 
	BYTE* psceneTableIndex 
);


/*********************************************************************
 * Function:        BYTE Get_EmptySceneTableEntry ( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         Return the index of the free scene table entry.
 *
 * Side Effects:    None
 *
 * Overview:        This function gets the free entry (if any ) from scene table.
 *
 * Note:            None
 ********************************************************************/
BYTE Get_EmptySceneTableEntry ( void );



/******************************************************************************
 * Function:        void GetClusterDefinition
 *                  (
 *                      WORD clusterId, 
 *                      ZCL_DeviceInfo* pDeviceInfo,
 *                      ROM ZCL_ClusterDefinition** pZCLClusterDefinition
 *                  );
 *
 * PreCondition:    None
 *
 * Input:
 *                  clusterId - cluster id for which cluster defination has to be get
 *                  pDeviceInfo -  This is the endpoint for which the command is
 *                                received.
 *                  clusterId  -  This holds the device info of a particular endpoint
 *
 * Output:          pZCLClusterDefinition - This holds the cluster definition of
 *                                          a particular cluster endpoint.
 *
 *
 * Return :         None.
 *
 * Side Effects:    None
 *
 * Overview:        This function loops through all the clusters supported by the particular 
 *				    endpoint and comapres the requested cluster id with every cluster. If match 
 *					is found then it updates cluster defination pointer for that cluster id. Otherwise 
* 					it update the cluster defination with the null pointer.
 * Note:            None
 *****************************************************************************/
void GetClusterDefinition 
( 
	WORD clusterId, 
	ZCL_DeviceInfo* pDeviceInfo, 
	ROM ZCL_ClusterDefinition** pClusterDefinition
);


/******************************************************************************
 * Function:        void Update_sceneExtnWithCurrentAttrb
 *                  (
 *                      BYTE endPointId, 
 *                      App_ScenesRecord* pAppSceneRecord
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           endPointId - The endpoint for which scene entry has to be stored.  

 * Output:          pAppSceneRecord - This holds the location of the scene table entry in which 
 *                                    extension fields has to be copied.
 *
 *
 * Return :         None.
 *
 * Side Effects:    None
 *
 * Overview:        This function  gets called when store scene command is received for the scene id and 
 *					group id for which earlier scene was not added with add scene command. This function 
 *					loops through all the clusters supported on this endpoint and update the extension fields 
 *					for the clusters which supports extension fields.
 * Note:            None
 *****************************************************************************/

void Update_sceneExtnWithCurrentAttrb 
( 
	BYTE endPointId,
	App_ScenesRecord* pAppSceneRecord 
);



/******************************************************************************
 * Function:        void Update_LastConfiguredByAttr
 *                  (
 *                      BYTE endPointId
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           endPointId - The endpoint for which last configured by attribute has to be updated.  

 * Output:          None 
 *
 * Return :         None.
 *
 * Side Effects:    None
 *
 * Overview:        This function  gets called when add scene or store scene command is received. 
 *					This will update the last configured by attribute with the extended address of 
 *					the device hwhich has initiate the command.
 * Note:            None
 *****************************************************************************/
void Update_LastConfiguredByAttr 
( 
	BYTE endPointId 
);



/******************************************************************************
 *	Function:   	BYTE Get_SceneTableEntry
 *					(
 *    					WORD groupId,
 *    					BYTE index
 *					)
 *
 * PreCondition:    None
 *
 * Input:           groupId  - The groupId for which the scene entry is to be checked.
 *					index - The scene table index for group id is to be checked.
 *
 * Output:          None
 * Return :         Returns the scene id. 
 * Side Effects:    None
 *
 * Overview:        This function checks whether group id matched with the requested scene table 
 *					entry. If group id is matched, then it returns the scene id of this entry, 
 *					otherwise returns the invlid scene id.
 *****************************************************************************/
BYTE Get_SceneTableEntry 
( 
	WORD groupId, 
	BYTE index 
);


/******************************************************************************
 *	Function:   	BYTE Update_AndGetSceneCount
 *					(
 *    					BYTE endPointId,
 *						BYTE direction
 *					)
 *
 * PreCondition:    None
 *
 * Input:           endPointId - The endpoint for which scene count has to be updated     
 *		    		direction - The direction field tells whether scene count has to be 
 *							    updated( incremented or decremented or reset to zero ) or 
 *								scene count has to be get.
 *
 * Output:          None
 * Return :         It returns the value of the scene count variable.
 * Side Effects:    None
 *
 * Overview:        This function  performs the action according to the direction field. If 
 *					the direction field is get scene count, then it will simply returns the 
 *					scene count attribute value. If the direction field is reset scene count 
 *					then it will reset the scene count to zero. If the direction field is 
 *					incement\decrement scene count then it will increment\decrement the scene 
 *					count variable value 
 *****************************************************************************/	
BYTE Update_AndGetSceneCount 
( 
	BYTE endPointId,
	BYTE direction
);

/******************************************************************************
 *	Function:   	void Scenes_UpdateSceneValidFields
 *					(
 *    					BYTE sceneTableIndex,
 *						BYTE endPointId,
 						BYTE isSceneValid
 *					)
 *
 * PreCondition:    None
 *
 * Input:           sceneTableIndex - The index of the scene table entry for which scene 
 *									  valid fields to be updated.      
 *		    		endPointId - The end point for which the scene valid fields to be updated.
 *					isSceneValid - This variable willl tell whether scene valid attribute value 
 *                                  should be true or false.
 *
 * Output:          None
 * Return :         None. 
 * Side Effects:    None
 *
 * Overview:        This function  gets called when store scene or recall scene command is received. 
 *					This function uapdte the attributes related to scenes valid and sets the scene valid flag.
 *****************************************************************************/
void Scenes_UpdateSceneValidFields 
( 
	BYTE sceneTableIndex,
 	BYTE endPointId,
 	BYTE isSceneValid
);

/******************************************************************************
 *	Function:   	void App_DeleteScenesTableEntry
 *					(
 *						BYTE endPointId
 *    					WORD groupId
 *					)
 *
 * PreCondition:    None
 *
 * Input:           groupId  - The groupId for which the scene entries has to be deleted.
 *					endPointId - The endpoint for which scene entry has to be removed.
 *
 * Output:          None
 * Return :         None. 
 * Side Effects:    None
 *
 * Overview:        This function loop through the whole scene table and deletes all those 
 *                  entries which are matched the requested group id
 *****************************************************************************/
void App_DeleteScenesTableEntry 
( 
	BYTE endPointId,
	WORD groupId
);

/******************************************************************************
 * Function:        void Init_ScenesTable ( void )
 *
 * PreCondition:    None
 *
 * Input:			None 
 *
 * Output:          None
 *
 * Return :         None.
 *
 * Side Effects:    None
 *
 * Overview:        This function initialize the scenes table and reset all the global scene
 *                  variables to their default values.
 * Note:            None
 *****************************************************************************/
void Init_ScenesTable ( void );

#endif /*I_SUPPORT_SCENES*/

#if defined(I_SUPPORT_GROUP_ADDRESSING)
/******************************************************************************
 *	Function:   	void App_AddGroup
 *					(
 *    					SHORT_ADDR groupAddr,
 *						BYTE endPointId,
 *						BYTE* pReceivedCmdPayload 
 *					)
 *
 * PreCondition:    None
 *
 * Input:      		groupAddr - The group id for which group enty has to be added.     
 *					endPointId - The endpoint for which group entry has to be added.      
 *		    		pReceivedCmdPayload - Pointer to the payload location of the 
 *										  received command.
 *
 * Output:          None
 * Return :         None. 
 * Side Effects:    None
 *
 * Overview:        This function  will add the group entry in the applications group table
 *****************************************************************************/
void App_AddGroup 
( 
	SHORT_ADDR groupAddr,
    BYTE endPointId,
	BYTE* pReceivedCmdPayload 
);
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

/******************************************************************************
 *	Function:   	BYTE App_ReadWriteSingleAttribute
 *					(
 *    					BYTE endPointId,
 *						WORD attributeId,
 *						ROM ZCL_ClusterInfo* pZclClusterInfo,
 * 						BYTE* pData,
 *    					BYTE direction
 *					)
 *
 * PreCondition:    None
 *
 * Input:           endPointId  - The end point for which the attribute value has to be 
 *								  read or written.
 *					attributeId  - attribute id, whose value has to be read or written.
 *					pZCLClusterInfo - Pointer pointing to the Cluster info.
 *					direction - Direction which tells ehether attribute value ha s to be read or written.
 *
 * Output:          psceneTableIndex - This point to the index of the scene table entry 
 *                                     whch is matched for group id and scene id.
 * Return :         It returns the length of the attribue id which is successfully read or written.
 * Side Effects:    None
 *
 * Overview:        This function reads or writes the attribute value from (or to) pdata 
 *					depending upon the direction field.
 *****************************************************************************/
BYTE App_ReadWriteSingleAttribute
(
	BYTE endPointId,
	WORD attributeId,
	ROM ZCL_ClusterInfo* pZclClusterInfo,
	BYTE* pData,
	BYTE direction		
);


/***************************************************************************************
 * Function:        void DisplayGroupsClusterCommands(BYTE *pReceivedAsdu)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the Groups Cluster commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if defined(I_SUPPORT_GROUP_ADDRESSING)
void DisplayGroupsClusterCommands
(
    BYTE *pReceivedAsdu
);
#endif

/***************************************************************************************
 * Function:        void DisplayScenesClusterCommands(BYTE *pReceivedAsdu)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the Scenes Cluster commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if (I_SUPPORT_SCENES == 1)
void DisplayScenesClusterCommands
(
    BYTE *pReceivedAsdu
);
#endif

/*****************************************************************************
                                Private Functions
 *****************************************************************************/
/******************************************************************************
 * Function:       static BYTE VerifyReportableChange( BYTE index )
 *
 * PreCondition:    None
 *
 * Input:           index - index to the report configuration table
 *
 * Output:          None
 * Return :
 *                  TRUE - if there is a change in the attribute data
 *                  FALSE - if there is no change in the attribute data
 * Side Effects:    None
 *
 * Overview:        This function checks if there is any change in the value
 *                  positive or negative. If there is a change, the function
 *                  returns TRUE else it returns FALSE
 *****************************************************************************/
#if I_SUPPORT_REPORTING == 1
static BYTE VerifyReportableChange( BYTE index )
{
    BYTE i;
    BYTE status = FALSE;
    WORD carry = 0x0000;
    dataTypeLength currentAttributeValue;
    dataTypeLength tempAttributeValue ;
    BYTE attributeDataLength ;
    GetReportCfgTable(reportConfigurationTable);
    attributeDataLength =
    ZCL_GetDataLengthForAttributeDataType
    (
        reportConfigurationTable[index].attributeDataType,
        0x00
    ) ;
    ZCL_Callback_GetAttributeValue
    (
        reportConfigurationTable[index].storageIndex,
        currentAttributeValue.arrayVal,
        attributeDataLength
    );
   /*Always the attributes greater than 4 bytes are Analog data types*/
    if( ( attributeDataLength > 0x04 ) || ( attributeDataLength == 0x03 ) )
    {

        /*-------------------------*/
        /*checking for current value greater than previous value
        1. Add the reportable change to previuos attribute data and keep the same in
        TempAttribute value
        2. Temp attribute value is compared with current attribute value*/
        for( i = 0x00 ; i < attributeDataLength; i++ )
        {
            carry = (WORD)( ( reportConfigurationTable[index].previousReportedAttributeData.arrayVal[i] ) +
            ( reportConfigurationTable[index].reportableChange.arrayVal[i] ) );
            /*add the lower byte*/
            tempAttributeValue.arrayVal[i] = (BYTE)carry;
            if( i < ( attributeDataLength - 1 ) )
            {
                /*add the carry*/
                tempAttributeValue.arrayVal[i + 1] += ( carry >> 8 );
            }
        }
        /*check if current value is greater than temp attribute value*/
        for( i = attributeDataLength ; i > 0x00; i-- )
        {
           if( currentAttributeValue.arrayVal[i-1] >  tempAttributeValue.arrayVal[i-1] )
           {
               status =  TRUE;
               break;
           }
        }
        /*check for equal value*/
        if( ( i == 0x00 ) && ( status !=  TRUE ) )
        {
            /*check if temp attribute value is equal to current attribute value*/
            for( i = 0x00 ; i < attributeDataLength; i++ )
            {
               if( tempAttributeValue.arrayVal[i] !=
                ( currentAttributeValue.arrayVal[i] ) )
               {
                    break;
               }
            }
        }
        /*-Temp attribute value and current attribute value is equal, then only i = attributeLength */
        if( i == attributeDataLength )
        {
            status = TRUE;
        }
        /*check for less than value*/
        if( FALSE == status )
        {

            /*checking for current value greater than previous value
            1. Add the reportable change to current attribute data and keep the same in
            TempAttribute value
            2. Temp attribute value is compared with previous attribute value*/
            for( i = 0x00 ; i < attributeDataLength; i++ )
            {
                carry = (WORD)( ( currentAttributeValue.arrayVal[i] ) +
                ( reportConfigurationTable[index].reportableChange.arrayVal[i] ) );
                /*add the values*/
                tempAttributeValue.arrayVal[i] = (BYTE)carry;
                if( i < ( attributeDataLength - 1 ) )
                {
                    /*add the carry*/
                    tempAttributeValue.arrayVal[i + 1] += ( carry >> 8 );
                }
            }
            /*compare temp attribute data with previous attribute data*/
            for( i = attributeDataLength ; i > 0x00; i-- )
            {
               if(  ( reportConfigurationTable[index].previousReportedAttributeData.arrayVal[i-1] )  >
                 tempAttributeValue.arrayVal[i-1] )
               {
                   status =  TRUE;
                   break;
               }
            }
            /*compare temp attribute data with previous attribute data for equality*/
            if( ( i == 0x00 ) && ( status !=  TRUE ) )
            {
                for( i = 0x00 ; i < attributeDataLength; i++ )
                {
                   if( tempAttributeValue.arrayVal[i] !=
                    ( reportConfigurationTable[index].previousReportedAttributeData.arrayVal[i] ) )
                   {
                        break;
                   }
                }
                /*if equal update the status as TRUE*/
                if( i == attributeDataLength )
                {
                    status = TRUE;
                }
            }
        }
    }
    else
    {
        /*check if the received data type is analog, if so consider the reportable change
        else any unequal condition will generate report attribute command*/
        if( ZCL_CheckAttributeAnalogDataType(reportConfigurationTable[index].attributeDataType))
        {
            switch( attributeDataLength )
            {
                case 1:
                /*compare byte value*/
                if( ( currentAttributeValue.byteVal >= ( ( reportConfigurationTable[index].previousReportedAttributeData.byteVal ) +
                ( reportConfigurationTable[index].reportableChange.byteVal ) ) ) ||
                ( ( reportConfigurationTable[index].previousReportedAttributeData.byteVal ) >=
                ( currentAttributeValue.byteVal + ( reportConfigurationTable[index].reportableChange.byteVal ) ) )
                 )
                {
                    reportConfigurationTable[index].previousReportedAttributeData.byteVal = currentAttributeValue.byteVal;
                    PutReportCfgTable(reportConfigurationTable);
                    return TRUE;
                }
                break;

                case 2:
                /*compare word value*/
                if( ( currentAttributeValue.wordVal >= ( ( reportConfigurationTable[index].previousReportedAttributeData.wordVal ) +
                ( reportConfigurationTable[index].reportableChange.wordVal ) ) ) ||
                ( ( reportConfigurationTable[index].previousReportedAttributeData.wordVal ) >=
                ( currentAttributeValue.wordVal + ( reportConfigurationTable[index].reportableChange.wordVal ) ) )
                 )
                {
                    reportConfigurationTable[index].previousReportedAttributeData.wordVal = currentAttributeValue.wordVal;
                    PutReportCfgTable(reportConfigurationTable);
                    return TRUE;
                }
                break;

                case 4:
                /*compare dword value*/
                if( ( currentAttributeValue.dwordVal >= ( ( reportConfigurationTable[index].previousReportedAttributeData.dwordVal ) +
                ( reportConfigurationTable[index].reportableChange.dwordVal ) ) ) ||
                ( ( reportConfigurationTable[index].previousReportedAttributeData.dwordVal ) >=
                ( currentAttributeValue.dwordVal + ( reportConfigurationTable[index].reportableChange.dwordVal ) ) )
                 )
                {
                    reportConfigurationTable[index].previousReportedAttributeData.dwordVal = currentAttributeValue.dwordVal;
                    PutReportCfgTable(reportConfigurationTable);
                    return TRUE;
                }
                break;
            }
        }
        else
        {
            /*For the discrete data type, if the values are not equal, we have to report attribute command*/
            if( currentAttributeValue.byteVal != reportConfigurationTable[index].previousReportedAttributeData.byteVal )
            {
                reportConfigurationTable[index].previousReportedAttributeData.byteVal =
                currentAttributeValue.byteVal;
                PutReportCfgTable(reportConfigurationTable);
                return TRUE;
            }
        }

    }
    /*for any case, if the status is TRUE update the previous reported value*/
    if( TRUE == status )
    {
        for( i = 0x00 ; i < attributeDataLength; i++ )
        {
           reportConfigurationTable[index].previousReportedAttributeData.arrayVal[i] =
                        currentAttributeValue.arrayVal[i];
        }
    }
    PutReportCfgTable(reportConfigurationTable);
    return status;
}
#endif /*I_SUPPORT_REPORTING*/

/******************************************************************************
 * Function:       static void InitializeReportingTable( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function initializes the reporting table with 0xff
 *                  invalid values.
 *****************************************************************************/
#if I_SUPPORT_REPORTING == 1
static void InitializeReportingTable( void )
{
    BYTE i;
    GetReportCfgTable(reportConfigurationTable);
    for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
    {
        /*set the default destination address mode to two bytes.*/
        reportConfigurationTable[i].destinationAddressMode = APS_ADDRESS_16_BIT;
        /*initialise the reporting table configuration, the destination address
        field is initialised with Trust Center address namely 0x00*/
        reportConfigurationTable[i].DstAddress.ShortAddr.Val = 0x0000;
        /*Default the endpoint supported on all devices is 0x01
        NOTE: The destination address fields could be overwritten using the interface
        provided by the Application*/
        reportConfigurationTable[i].destinationEndpoint = 0x01;
        /*initialise the reporting table configuration, 10 bytes are subtracted
        considering the destination address fields.*/
        memset
        (
            (BYTE*)&(reportConfigurationTable[i].srcEndPoint),
            0xFF,
            sizeof(ReportConfiguration) - 0x0a
        );
    }
    PutReportCfgTable(reportConfigurationTable);
}
#endif /*I_SUPPORT_REPORTING*/

/******************************************************************************
* Function:        void SE_SendDefaultResponse
*                  (
*                      BYTE status,
*                      BYTE useNWKSecurity,
*                      APP_DATA_indication* p_dataInd
*                  )
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
* Overview:        Sends default response.
*
* Note:            None
*****************************************************************************/
void SE_SendDefaultResponse
(
    BYTE status,
    BYTE useNWKSecurity,
    APP_DATA_indication* p_dataInd
)
{
    APP_DATA_request* pAppDataReq;
    BYTE* pLocationForResponse;

    /* Allocation of Memory is for creating Response Frame.*/
    pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);
    if( pAppDataReq != NULL )
    {
        pLocationForResponse = pAppDataReq->asdu;
        pAppDataReq->asduLength =   ZCL_CreateDefaultResponseCommand
                                    (
                                        p_dataInd->asdu,
                                        pLocationForResponse,
                                        status
                                    );

        /*Update DestAddressMode with Indication srcAddress Mode*/
        pAppDataReq->DstAddrMode = p_dataInd->SrcAddrMode;
        /*Checking for addressing is shortAddress or GroupAddress*/
        if( ( pAppDataReq->DstAddrMode == APS_ADDRESS_GROUP ) ||
                ( pAppDataReq->DstAddrMode == APS_ADDRESS_16_BIT ))
        {
                /*UpdateDestAddress*/
                 memcpy
                 (
                    ( BYTE *)&(pAppDataReq->DstAddress.ShortAddr.v[0]),
                    ( BYTE *)&(p_dataInd->SrcAddress.ShortAddr.v[0]),
                     SHORT_ADDRESS_LENGTH
                 );

        }
        /*Checking for addressing is IEEEAddress */
        if( pAppDataReq->DstAddrMode == APS_ADDRESS_64_BIT )
        {
                /*UpdateDestAddress*/
                memcpy
                (
                    ( BYTE *)&(pAppDataReq->DstAddress.LongAddr.v[0]),
                    ( BYTE *)&(p_dataInd->SrcAddress.LongAddr.v[0]),
                    MAX_ADDR_LENGTH
                );
        }
        /*UpdateDestEndPoint*/
        pAppDataReq->DstEndpoint = p_dataInd->SrcEndpoint;
        /*UpdateProfileId*/
        pAppDataReq->ProfileId  = p_dataInd->ProfileId;
        /*Update ClusterId*/
        pAppDataReq->ClusterId = p_dataInd->ClusterId;
        /*UpdateSrcEndPoint*/
        pAppDataReq->SrcEndpoint = p_dataInd->DstEndpoint;
        /*useNWKSecurity is TRUE, Security applied for the received frame is incorrect.
        So, in this case Default Response should be sent with only NETWORK SECURITY*/
        if (useNWKSecurity == TRUE)
        {
            pAppDataReq->TxOptions.Val = NETWORK_SECURITY;
        }
        else
        {
            pAppDataReq->TxOptions.Val = SE_GetClusterSpecificSecurity( p_dataInd->ClusterId.Val );
        }
        pAppDataReq->RadiusCounter = DEFAULT_RADIUS;
        /*Enqueue for AIL layer to perform data transmission*/
        if ( AILEnqueue( (BYTE *)pAppDataReq, TransmitQueueID ) == AILQueueFailure )
        {
            nfree(pAppDataReq);
        }
    }
    else
    {
        /*Memory Allocation Failed. So, nothing to do about it*/
        return;
    }
}

/*****************************************************************************
                                Public Functions
 *****************************************************************************/

 /***************************************************************************************
 * Function:        BYTE SE_GetClusterSpecificSecurity(WORD clusterID)
 *
 * PreCondition:    None
 *
 * Input:           clusterID - Cluster Id for which security level needs to be known
 *
 * Output:          Security level for data communication.
 *
 * Side Effects:    None
 *
 * Overview:        Returns the security level for Cluster ID passed
 *
 * Note:            None
 *
 ***************************************************************************************/
BYTE SE_GetClusterSpecificSecurity(WORD clusterID)
{
    BYTE i;
    /*This loop is used for getting the security of given clusterId from
      "clusterSecurityTable".If the given clusterId is not existed in the
      "clusterSecurityTable",it reurns NETWORK_SECURITY.*/
    for (i = 0; i < (sizeof(clusterSecurityTable)/sizeof( Cluster_specific_security)); i++ )
    {
        if( clusterSecurityTable[i].clusterID == clusterID )
        {
            return clusterSecurityTable[i].security;
        }
    }
    return NETWORK_SECURITY;
}

/******************************************************************************
 * Function:        BYTE ZCL_Callback_IsDeviceInIdentifyMode( void );
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 * Return :         It returns '0x01' if the device is in identifymode else it
 *                  returns '0x00'.
 * Side Effects:    None
 *
 * Overview:        This function is used for finding whether the device is
 *                  in Identifcation mode or not.This functiion will be
 *                  implented by Application.
 * Note:            None
 *****************************************************************************/

BYTE ZCL_Callback_IsDeviceInIdentifyMode( void )
{
    return ( ( appIdentifyTimers.IdentifyTime )? 0x01 : 0x00 );
}

/******************************************************************************
* Function:        void ZCL_Callback_CheckForAlarmCondition
*                  (
*                       BYTE endpointID,
*                       WORD attributeId,
*                       ROM ZCL_ClusterInfo *pZCLClusterInfo
*                  );
*
* PreCondition:    None
*
* Input:           endpointID - It points to source end point
*                  pZCLClusterInfo - It points to ClusterInfo
*                  attributeId - its attribute data needs to be checked with
*                  threshold value.
*
* Output:          None
*
* Return :         None
*
* Side Effects:    None
*
* Overview:        This function checks,the given attributeID data with its threshold
*                  value.
*                  If it's data exceeds the threshold value and corresponding alarm mask
*                  is enabled then we need to generate an alarm and corresponding
*                  alarm entry will be added in its alarm table.
* Note:            None
*****************************************************************************/
void ZCL_Callback_CheckForAlarmCondition
(
    BYTE endpointId,
    WORD attributeId,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
)
{
    #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)

    BYTE i;
    /*This holds attributeData of a received attributeId */
    WORD receivedAttributeDataValue;
    /*This holds attributeData of a Threshold attributeId */
    WORD thresholdAttributeDataValue;
    /*This holds the NVM index of a Given Cluster.(Cluster is a member of ClusterInfo Structure)*/
    WORD attributeStorageBaseLocation;

    BYTE attributeStorageOffset;
    /*This holds the attributeDataTypeLength of a attributeId*/
    BYTE attributeDataTypeLength;
    BYTE maskValue;



    /*This loop is used for checking wheteher alarmConditions is happened or not*/
    for ( i=0x00; i<(sizeof(alarmConditionTableEntries)/sizeof(AlarmConditionsTable)); i++)
    {
        /*It checks the given attributeId and ClusterID with its AlarmConditionTableEntries.*/
        if( ( alarmConditionTableEntries[i].clusterId == pZCLClusterInfo->clusterId )&&
            ( alarmConditionTableEntries[i].attributeId == attributeId ))
        {
            /*It is used to get NVM index of a given cluster*/
            if ( EP_CLUSTERID_NOT_FOUND_IN_NVM != ZCL_Callback_GetClusterIndex
                                                    (
                                                        endpointId,
                                                        pZCLClusterInfo->clusterId,
                                                        &attributeStorageBaseLocation
                                                    ) )
            {
                /*For getting the attribute value of a given attributeId*/
                attributeDataTypeLength =   GetAttributeDetails
                                            (
                                                pZCLClusterInfo->numberOfAttributes,
                                                alarmConditionTableEntries[i].attributeId,
                                                (ZCL_AttributeInfo *)(pZCLClusterInfo->pListofAttibuteInfo),
                                                &attributeStorageOffset
                                            );

                /*This functions reads the attributedata from the given NVM index*/
                ZCL_Callback_GetAttributeValue
                (
                    attributeStorageBaseLocation + attributeStorageOffset,
                    (BYTE*)&receivedAttributeDataValue,
                    attributeDataTypeLength
                );
                /*For getting the attribute value of a given thresholdAttributeId*/
                attributeDataTypeLength =   GetAttributeDetails
                                            (
                                                pZCLClusterInfo->numberOfAttributes,
                                                alarmConditionTableEntries[i].thresholdAttributeId,
                                                (ZCL_AttributeInfo *)(pZCLClusterInfo->pListofAttibuteInfo),
                                                &attributeStorageOffset
                                            );
                /*This functions reads the attributedata from the given NVM index*/
                ZCL_Callback_GetAttributeValue
                (
                    attributeStorageBaseLocation + attributeStorageOffset,
                    (BYTE*)&thresholdAttributeDataValue,
                    attributeDataTypeLength
                );
                /*It checks the received attribute data with its threshold value*/
                if( (( receivedAttributeDataValue < thresholdAttributeDataValue )&&
                     ( alarmConditionTableEntries[i].compareToMaxOrMinThreshold == MinThreshold ))||
                    (( receivedAttributeDataValue > thresholdAttributeDataValue )&&
                     ( alarmConditionTableEntries[i].compareToMaxOrMinThreshold == MaxThreshold )))
                {
                   /*For getting the alarm mask value of a given alarmMaskAttributeId*/
                    attributeDataTypeLength =   GetAttributeDetails
                                                (
                                                    pZCLClusterInfo->numberOfAttributes,
                                                    alarmConditionTableEntries[i].alarmMaskAttributeId,
                                                    (ZCL_AttributeInfo *)(pZCLClusterInfo->pListofAttibuteInfo),
                                                    &attributeStorageOffset
                                                );
                    ZCL_Callback_GetAttributeValue
                    (
                        attributeStorageBaseLocation + attributeStorageOffset,
                        &maskValue,
                        attributeDataTypeLength
                    );

                    /*If read maskValue is matches with alarmConditionTableEntry alarmMask value,then
                      we need to add the alarm entry in its alarm table and also we need to generate an alarm*/
                    if( ( maskValue & alarmConditionTableEntries[i].alarmMaskData ) == alarmConditionTableEntries[i].alarmMaskData )
                    {
                        #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
                        if(( ZCL_POWER_CONFIGURATION_CLUSTER == pZCLClusterInfo->clusterId )&&
                           ( ZCL_MAINS_VOLTAGE == attributeId  ))
                        {

                            if( MainsVoltageDwellTripPointHandled == 0x00 )
                            {
                                attributeDataTypeLength =   GetAttributeDetails
                                                            (
                                                                pZCLClusterInfo->numberOfAttributes,
                                                                ZCL_MAINS_VOLTAGE_DWELL_TRIP_POINT,
                                                                (ZCL_AttributeInfo *)(pZCLClusterInfo->pListofAttibuteInfo),
                                                                &attributeStorageOffset
                                                            );
                                ZCL_Callback_GetAttributeValue
                                (
                                    attributeStorageBaseLocation + attributeStorageOffset,
                                    (BYTE*)&mainsVoltageDwellTripPointValue,
                                    attributeDataTypeLength
                                );

                                appPowerConfigMainsDwellTrip.DwellTripPoint = 0x01;
                                appPowerConfigMainsDwellTrip.DwellTripPointStartTime = TickGet();
                                return;
                            }
                            MainsVoltageDwellTripPointHandled = 0x00;

                        }
                        #endif
                        /*It used for adding alarm entry in its alarm table*/
                        SE_UpdateAlarmTable
                        (
                            alarmConditionTableEntries[i].clusterId,
                            alarmConditionTableEntries[i].alarmCode,
                            ZCL_Callback_GetCurrentTimeInSeconds(),
                            ZCL_Alarm_AlarmEvent

                        );
                        /*It is used to generate an anlarm*/
                        GenerateAlarm(alarmConditionTableEntries[i].clusterId,
                                          alarmConditionTableEntries[i].alarmCode);

                    }
                    return;
                }
            }
        }
    }
    #endif
}

/******************************************************************************
* Function:        void SE_UpdateAlarmTable ( WORD clusterId,BYTE alarmCode,
*                                              DWORD timeStamp,BYTE event );
*
* PreCondition:    None
*
* Input:           clusterId - this cluster id is added/Updated into alarmTable
*                  attributeId - this attribute id is added/Updated into alarmTable
*                  timeStamp - this timeStamp is added/Updated into alarmTable
*                  event - It specifies updation of alarmTable(Adding,deleting,Inactive)
*
*
* Output:          None
*
* Return :         None
*
* Side Effects:    None
*
* Overview:       This function updates the alarmTable according the "event".
*                 a)If event is ZCL_Alarm_AlarmEvent,it updates(adds) alarm
*                   entry with given values.
*                 b)If event is ZCL_Alarm_ResetAlarmEvent,it resets the alarm
*                   entry.It means it make alarmEntry status as Inactive.
*                 c)If event is ZCL_Alarm_ResetAllAlarmsEvent,it resets the alarm
*                   entriers.It means it make alarmEntries status as Inactive.
*                 d)If event is ZCL_Alarm_ResetAlarmLogEvent,it clears the alarm
*                   table and alarmCount to Zero.
*
*
* Note:            None
*****************************************************************************/
void SE_UpdateAlarmTable
(
    WORD clusterId,
    BYTE alarmCode,
    DWORD timeStamp,
    BYTE event
)
{
    WORD alarmCount = 0x0000;
    WORD storageIndex;
    BYTE i;
    /*It is for getting the alarmCount value from NVM.*/
    if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=   ZCL_Callback_GetClusterIndex
                                            (
                                                SOURCE_ENDPOINT,
                                                ZCL_ALARM_CLUSTER,
                                                &storageIndex
                                            ) )
    {

        ZCL_Callback_GetAttributeValue
        (
            storageIndex,
            (BYTE*)&alarmCount,
            0x02
        );

    }
    /*This is for checking duplicate alarmEntry.If exists it updates with input values and returns
      from the function*/
    for ( i = 0x00;i<(sizeof(AlarmEntries)/sizeof(ALARM_AlarmTable));i++)
    {
        if( ( AlarmEntries[i].clusterId == clusterId )&&( AlarmEntries[i].alarmCode == alarmCode )&&
            ( event == ZCL_Alarm_AlarmEvent ) )
        {
            AlarmEntries[i].timeStamp = timeStamp;
            AlarmEntries[i].status = Alarm_Active;
            return;
        }
    }
    /*This is for  adding alarmEntry in alarmTable */
    for ( i = 0x00;i<(sizeof(AlarmEntries)/sizeof(ALARM_AlarmTable));i++)
    {
        /*For adding alarmEntry.It increments alarm count by one*/
        if( ( AlarmEntries[i].status == Alarm_Clear )&&( event == ZCL_Alarm_AlarmEvent ) )
        {
            AlarmEntries[i].clusterId = clusterId;
            AlarmEntries[i].alarmCode = alarmCode;
            AlarmEntries[i].timeStamp = timeStamp;
            AlarmEntries[i].status = Alarm_Active;
            alarmCount++;
            break;
        }
        /*If alarmCount reaches max value,then we need to replace earliest time stamp value with
          the given input values*/
        if( ( alarmCount == ( sizeof(AlarmEntries)/sizeof(ALARM_AlarmTable) ) )&&( ZCL_Alarm_AlarmEvent == event) )
        {
            AlarmEntries[nextIndexToBeUpdated].clusterId = clusterId;
            AlarmEntries[nextIndexToBeUpdated].alarmCode = alarmCode;
            AlarmEntries[nextIndexToBeUpdated].timeStamp = timeStamp;
            AlarmEntries[nextIndexToBeUpdated].status = Alarm_Active;
            nextIndexToBeUpdated++;
            if( nextIndexToBeUpdated == ( sizeof(AlarmEntries)/sizeof(ALARM_AlarmTable)) )
            {
                nextIndexToBeUpdated = 0x00;
            }
            return;
        }
        /*It makes the alarmEntry Status with Inactive,if entry exists in alarmTable*/
        if( ( event == ZCL_Alarm_ResetAlarmEvent ) &&(AlarmEntries[i].clusterId == clusterId )&&
            ( AlarmEntries[i].alarmCode == alarmCode ))
        {
            AlarmEntries[i].status = Alarm_Inactive;
            return;
        }
        /*It makes the all alarmEntries Status with Inactive*/
         if( event == ZCL_Alarm_ResetAllAlarmsEvent )
        {
            AlarmEntries[i].status = Alarm_Inactive;
        }
        /*It Clears the all alarm entries*/
        if( event == ZCL_Alarm_ResetAlarmLogEvent )
        {
            AlarmEntries[i].clusterId = 0xFFFF;
            AlarmEntries[i].alarmCode = 0xFF;
            AlarmEntries[i].timeStamp = 0xFFFFFFFF;
            AlarmEntries[i].status = Alarm_Clear;
            alarmCount = 0x0000;
        }

    }
    /*It stores the Updated alarm count value in NVM*/
    ZCL_Callback_PutAttributeValue
    (
        storageIndex,
        (BYTE*)&alarmCount,
        0x02
    );

}

/******************************************************************************
* Function:        static void GenerateAlarm ( WORD clusterId,BYTE alarmCode);
*
* PreCondition:    None
*
* Input:           clusterId - This needs be added in alarm command
*                  alarmCode - This needs be added in alarm command
*
*
* Output:          None
*
* Return :         None
*
* Side Effects:    None
*
* Overview:        This function is used to generate an alarmCommand
*
* Note:            None
*****************************************************************************/
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
static void GenerateAlarm( WORD clusterId, BYTE alarmCode )
{
    /*This pointer holds the Default Response*/
    APP_DATA_request* pApplicationDataReq;

    /* Memory Allocated is for creating Response Frame.This memory
    is freed by the lower layer*/
    pApplicationDataReq =  (APP_DATA_request *)SRAMalloc(126);
    if( pApplicationDataReq != NULL )
    {
        /*update the destination address mode with Indirect Address mode.*/
        /*Alarm command should be sent to its bounded device.So Address mode is Indirect
          address mode. */
        pApplicationDataReq->DstAddrMode = APS_ADDRESS_NOT_PRESENT;
        /*Update ProfileId*/
        pApplicationDataReq->ProfileId.Val  = SE_PROFILE_ID;
        /*Update clusterId*/
        pApplicationDataReq->ClusterId.Val = ZCL_ALARM_CLUSTER;
        /*Update Src Endpoint*/
         pApplicationDataReq->SrcEndpoint = SOURCE_ENDPOINT;
        /*Get the Appropriate Security for the cluster*/
        pApplicationDataReq->TxOptions.Val = SE_GetClusterSpecificSecurity(pApplicationDataReq->ClusterId.Val);
        pApplicationDataReq->RadiusCounter = DEFAULT_RADIUS;
        pApplicationDataReq->asduLength = 0x06;
        /*ZCL Header*/
        /*Frame Control*/
        pApplicationDataReq->asdu[0] = ZCL_ClusterSpecificCommand | ZCL_ServerToClient |
                                       ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
        /*Transaction Sequence Number*/
        pApplicationDataReq->asdu[1] = appNextSeqNum++;
        /*CommandID*/
        pApplicationDataReq->asdu[2] = ZCL_AlarmCmd;
        /*ZCL Payload*/
        /*Alarm Code*/
        pApplicationDataReq->asdu[3] = alarmCode;
        /*ClusterID*/
        memcpy
        (
            (BYTE*)&pApplicationDataReq->asdu[4],
            (BYTE*)&clusterId,
            ZCL_CLUSTER_ID_LENGTH
        );

        /*Enqueue for AIL layer to perform data transmission*/
        if ( AILEnqueue( (BYTE *)pApplicationDataReq, TransmitQueueID ) == AILQueueFailure )
        {
        nfree(pApplicationDataReq);
        }

    }
}
#endif
/******************************************************************************
* Function:        BYTE SE_CreateGetAlarmResponse( BYTE endPoint,BYTE transactionSeqNum,BYTE *pResponseAsdu );
*
* PreCondition:    None
*
* Input:          endPoint-
*                 transactionSeqNum -
*                 pResponseAsdu
*
* Output:          None
*
* Return :
*
* Side Effects:    None
*
* Overview:    This function is used to Create GetAlarmResponse.
* Note:            None
*****************************************************************************/
BYTE SE_CreateGetAlarmResponse( BYTE endPoint,BYTE transactionSeqNum,BYTE *pResponseAsdu )
{
    WORD storageIndex;
    WORD alarmCount;

    /*ZCL Header*/
    /*Cluster specific command, Manufacturer specific bit is zero,
    direction is Server to Client */
    *pResponseAsdu++ = ZCL_ClusterSpecificCommand | ZCL_ServerToClient |
                       ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
    *pResponseAsdu++ = transactionSeqNum;
    *pResponseAsdu++ = ZCL_GetAlarmResponseCmd;
    /*Getting for Alarm Count attribute value.
      If alarm count is zero,then we need to send GetAlarmResponse with status
      as NOT_FOUND,otherwise we need extract the earlist time stamp index
      of alarm from alarm table.*/
    if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=   ZCL_Callback_GetClusterIndex
                                            (
                                                endPoint,
                                                ZCL_ALARM_CLUSTER,
                                                &storageIndex
                                                ) )
    {

        ZCL_Callback_GetAttributeValue
        (
            storageIndex,
            (BYTE*)&alarmCount,
            0x02
        );

    }
    /*If alarmCount is Zero then we need to GetAlarmResponse with status as
                      NOT FOUND*/
    if( alarmCount == 0x0000 )
    {
        /*Updating ZCL Payload*/
        /*Updating status */
        *pResponseAsdu = ZCL_NotFound;
        return ZCL_HEADER_LENGTH_WO_MANUF_CODE + 0x01;
    }
    else
    {
        DWORD timeStamp = 0xFFFFFFFF;
        BYTE index = 0xFF;
        BYTE i;
        /*This loop for getting Earliest time stamp value from alarmTable*/
        for ( i = 0x00;i<(sizeof(AlarmEntries)/sizeof(ALARM_AlarmTable));i++)
        {
            if( timeStamp < AlarmEntries[i].timeStamp )
            {
                timeStamp =  AlarmEntries[index].timeStamp;
            }
            else
            {
                index = i;
                timeStamp = AlarmEntries[index].timeStamp;
            }
        }
        /*Updating ZCL Payload*/
        /*Updating status */
        *pResponseAsdu++ = ZCL_Success;
        /*Updating the alarm code,ClusterId and Time stamp*/
         memcpy
         (
            (pResponseAsdu),
            (BYTE*)&(AlarmEntries[index].alarmCode),
            sizeof(ALARM_AlarmTable) - 0x01
         );
         /*After Creating GetAlarmResponse Corresponding alarm entry
           needs be removed(reseted with its intial values)*/
         memset
         (
            (BYTE*)&(AlarmEntries[index]),
            0xFF,
            sizeof(ALARM_AlarmTable)
         );
         /*Updating the Alarm count */
        alarmCount-=0x01;
        /*Alarm Count is storing into NVM.*/
        ZCL_Callback_PutAttributeValue
        (
            storageIndex,
            (BYTE*)&alarmCount,
            ZCL_ATTRIBUTE_ID_LENGTH
        );
        return ZCL_HEADER_LENGTH_WO_MANUF_CODE + sizeof(ALARM_AlarmTable);
    }

}
/******************************************************************************
 * Function:        static void ResetAlarmEntries(void);
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        Resets all the attributes of all the clusters, on all
 *                  the endpoints supported on application.
 *
 * Note:            None
 *****************************************************************************/
 static void ResetAlarmEntries( void )
 {
    BYTE index;
    for(index=0x00;index<sizeof(AlarmEntries)/sizeof(ALARM_AlarmTable);index++)
    {
        memset
         (
            (BYTE*)&(AlarmEntries[index]),
            0xFF,
            sizeof(ALARM_AlarmTable)
         );
    }
 }
/******************************************************************************
 * Function:        void App_Reset(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        Resets all the attributes of all the clusters, on all
 *                  the endpoints supported on application.
 *
 * Note:            None
 *****************************************************************************/
void App_Reset(void)
{
    App_ResetAttributeData();
#if I_SUPPORT_REPORTING == 1
    InitializeReportingTable();
    App_ResetVariables();
#endif /*I_SUPPORT_REPORTING*/
#if I_SUPPORT_CBKE == 1
    //keyEstablishmentEndpoint = SOURCE_ENDPOINT;
    App_CBKE_Reset();
    /*reset the link key table*/
    ResetTCLinkKeyTable();
#endif /* I_SUPPORT_CBKE */
#if (I_AM_LCD == 1)
    SE_LCDReset();
#endif
#if (I_AM_PCT == 1)
    SE_PCTReset();
#endif
    ResetAlarmEntries();

#if (I_SUPPORT_SCENES == 1)
	/* clear the scene table */
	Init_ScenesTable();
#endif

}

/******************************************************************************
 * Function:        void App_ResetAttributeData( void )
 *
 * PreCondition:    None
 *
 * Input:           endPoint - End Point on which all attributes of all
 *                             cluster supported needs to be restarted.
 *                  noOfClusters - No of clusters supported on that
 *                             particular endpoint
 *
 * Output:          None
 *
 * Return:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to Reset all attributes of all the
 *                  clusters supported on enpoint provided to its default values
 *                  which is stored in ROM.
 *
 * Note:            None
 *****************************************************************************/
void App_ResetAttributeData( void )
{
    BYTE i;
    /*  Note: Reset to Factory Default command is meant for all end point.
        It is not for one end point given by request. */
    for( i = 0; i < NoOfEntriesInAttrStorageTable; i++ )
    {

        ZCL_Callback_PutAttributeValue( App_AttributeStorageTable[i].index,
                (BYTE *)(App_AttributeStorageTable[i].defaultValueLocation),
                  App_AttributeStorageTable[i].length );

    }
}

/******************************************************************************
* Function:        void ZCL_Callback_AddEntryIntoReportingTable
*                   (
*                   BYTE srcEndPoint,
*                   BYTE direction,
*                   WORD attributeId,
*                   WORD minReportingInterval,
*                   WORD maxReportingInterval,
*                   WORD timeOut,
*                   BYTE* ReportableChange,
*                   ROM ZCL_ClusterInfo *pZCLClusterInfo
*                   )
 *
 * PreCondition:    None
 *
 * Input:           srcEndPoint - endpoint on which the attribute need to be
 *                  configured
 *                  direction - indicates if the attribute is reported or
 *                  receiving reports
 *                  attributeId - attribute id that is to be reported
 *                  minReportingInterval - minimum report interval
 *                  at which the attribute to be reported
 *                  maxReportingInterval - maximum reporting interval
 *                  at which the attribute is to be reported
 *                  timeOut - timeout value for the reports to be received
 *                  ReportableChange - reportable change value for the
 *                  attribute
 *                  pZCLClusterInfo - points to the cluster info
 * Output:          None
 * Return :
 * Side Effects:    None
 *
 * Overview:        This function adds an entry into the reporting table.
 *                  if there is an existing entry, it checks for the request if
 *                  it has to be deleted. If so, it deletes else it updates the
 *                  same entry. If the values dont exist in the table, new
 *                  entry is added.
 *
 * Note:            None
 *****************************************************************************/
void ZCL_Callback_AddEntryIntoReportingTable
(
    BYTE srcEndPoint,
    BYTE direction,
    WORD attributeId,
    WORD minReportingInterval,
    WORD maxReportingInterval,
    WORD timeOut,
    BYTE* ReportableChange,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
)
{
    #if I_SUPPORT_REPORTING == 1
	BYTE i = 0x00;
    WORD storageIndex = 0xFFFF;
    GetReportCfgTable(reportConfigurationTable);
    /*check if the entry already exists*/
    for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
    {
        BYTE entryExists = 0x00;
        
        if (reportConfigurationTable[i].destinationAddressMode == APS_ADDRESS_16_BIT)
        {
            if ( !memcmp ( &(reportConfigurationTable[i].DstAddress.ShortAddr.v[0]),
                        &(InitiatorDeviceInfo.initiatorShortAddress.v[0]),
                        SHORT_ADDRESS_LENGTH ) )
            {
                entryExists = 0x01;
            }
        }
        else
        {
            if ( !memcmp ( &(reportConfigurationTable[i].DstAddress.LongAddr.v[0]),
                        &(InitiatorDeviceInfo.initiatorLongAddress.LongAddr.v[0]),
                        MAX_ADDR_LENGTH ) )
            {
                entryExists = 0x01;
            }
        }
        
        /*check if the cluster id and attribute id is already existing.if so,
        update the same entry with the details received in the command*/
        if( ( reportConfigurationTable[i].ClusterId == pZCLClusterInfo->clusterId ) &&
            ( reportConfigurationTable[i].attributeId == attributeId ) &&
            ( entryExists ) )
        {
            /*if the direction is server to client, only then continue further*/
            if( direction != ReportingDirectionClientToServer )
            {
                /*if the maximum reporting interval is 0xffff, make the entry as invalid*/
                if( 0xFFFF == maxReportingInterval )
                {
                    /*reset the entry is invalid, means stop reporting, reset
                    only the fields except destination fields,
                    10 bytes is considering the destination address mode, destination endpoint
                    and destination address*/
                    memset
                    (
                        &reportConfigurationTable[i].srcEndPoint,
                        0xFF,
                        ( sizeof(ReportConfiguration) - 0x0a )
                    );
                    PutReportCfgTable(reportConfigurationTable);
                    /*reset the attribute that is configured to report*/
                    return;
                }
                break;
            }
        }
    }
    /*if entry doesnt exist, then check for empty entry*/
    if( i == REPORTING_TABLE_SIZE )
    {
        for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
        {
            /*if the source endpoint is 0xff, then the entry is empty*/
            if( reportConfigurationTable[i].srcEndPoint == 0xFF )
            {
                break;
            }
        }
    }
    if( i != REPORTING_TABLE_SIZE )
    {
        /*Update the Initiator Device Information. This is required to send the
        Report Attribute command to the initiator device from which the configuration command
        is received.*/
        reportConfigurationTable[i].destinationAddressMode = InitiatorDeviceInfo.initiatorAddressMode;
        
        if( InitiatorDeviceInfo.initiatorAddressMode == APS_ADDRESS_16_BIT )
        {
            memcpy
            (
                ( BYTE *)&(reportConfigurationTable[i].DstAddress.ShortAddr.v[0]),
                ( BYTE *)&(InitiatorDeviceInfo.initiatorShortAddress.v[0]),
                SHORT_ADDRESS_LENGTH
            );
        }
        else
        {
            memcpy
            (
                ( BYTE *)&(reportConfigurationTable[i].DstAddress.LongAddr.v[0]),
                ( BYTE *)&(InitiatorDeviceInfo.initiatorLongAddress.LongAddr.v[0]),
                MAX_ADDR_LENGTH
            );
        }
        reportConfigurationTable[i].destinationEndpoint = InitiatorDeviceInfo.initiatorEndPoint;
        
        /*copy the fields into the table without checking the direction
        since non applicable fields are initialized with invalid values by ZCL.*/
        reportConfigurationTable[i].srcEndPoint = srcEndPoint;
        reportConfigurationTable[i].direction =  direction;
        reportConfigurationTable[i].ClusterId = pZCLClusterInfo->clusterId;
        reportConfigurationTable[i].attributeId = attributeId;
        GetReportingBgTasks(&ReportingBgTasks);
        /*if the direction is server to client, update the minimum intervals etc*/
        if( direction != ReportingDirectionClientToServer )
        {

            /*copy the minimum and maximum intervals*/
            reportConfigurationTable[i].minimumReportingInterval = minReportingInterval;
            reportConfigurationTable[i].maximumReportingInterval = maxReportingInterval;
            /*first byte of the pointer gives the length of the reportable change
            value.*/
            memcpy
            (
                reportConfigurationTable[i].reportableChange.arrayVal,
                ReportableChange + 1,
                ( *ReportableChange )
            );
            /*get the cluster storage index*/
            ZCL_Callback_GetClusterIndex( srcEndPoint, pZCLClusterInfo->clusterId, &storageIndex );
            /*get the attribute index for the given cluster index*/
            reportConfigurationTable[i].storageIndex =
            SE_GetAttributeIndex( attributeId, pZCLClusterInfo, storageIndex,
            &( reportConfigurationTable[i].attributeDataType ) );
            /*get the attribute value for the index*/
            ZCL_Callback_GetAttributeValue( storageIndex,
            reportConfigurationTable[i].previousReportedAttributeData.arrayVal,( *ReportableChange ));


            /*for the first time assign the minm and maxm reporting interval
            without consolidation*/
            if( !ReportingBgTasks.flags.bits.bValidReportEntryConfigured )
            {
                ReportingBgTasks.consolidatedMinTime = minReportingInterval;
                ReportingBgTasks.consolidatedMaxTime = maxReportingInterval;
                ReportingBgTasks.flags.bits.bValidReportEntryConfigured = TRUE;
                ReportingBgTasks.minStartTime = TickGet();
                ReportingBgTasks.maxStartTime = TickGet();
            }
            else
            {
                /*consolidate the minimum and maximum reporting interval*/
                if( minReportingInterval < ReportingBgTasks.consolidatedMinTime )
                {
                    ReportingBgTasks.consolidatedMinTime = minReportingInterval;
                }
                if( maxReportingInterval < ReportingBgTasks.consolidatedMaxTime )
                {
                    ReportingBgTasks.consolidatedMaxTime = maxReportingInterval;
                }
            }

        }
        else
        {
            /*update the timeout field*/
            reportConfigurationTable[i].timeout = timeOut;
            reportConfigurationTable[i].receivedReportAttribute = FALSE;
            /*update the timeout values in background tasks*/
            ReportingBgTasks.timeOut = timeOut;
            ReportingBgTasks.flags.bits.bAwaitingReportAttributeCommand = TRUE;
            ReportingBgTasks.timeOutStartTime = TickGet();
        }
        /*put teh background taks into NVM*/
        PutReportingBgTasks(&ReportingBgTasks);
    }
    PutReportCfgTable(reportConfigurationTable);
	#endif /*I_SUPPORT_REPORTING*/
}

/******************************************************************************
 * Function:       void SE_CheckToSendReportAttributeCommand( void );
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 * Return :
 * Side Effects:    None
 *
 * Overview:        This function checks all the expired timers to see if
 *                  report attribute need to be sent
 * Note:            None
 *****************************************************************************/
#if I_SUPPORT_REPORTING == 1
void SE_CheckToSendReportAttributeCommand
(
    void
)
{
    BYTE i = 0x00;
    /*Get the report configuration table from NVM*/
    GetReportCfgTable(reportConfigurationTable);
    for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
    {
        /*only if the direction is server to client, then set the report flag
            and send the report attribute command*/
        if( reportConfigurationTable[i].srcEndPoint != 0xFF )
        {
            /*if the direction is server to client*/
            if( ReportingDirectionServerToClient == reportConfigurationTable[i].direction )
            {
                /*following condition report attribute will be generated
                1. minimum interval expired or minimum time is zero and value change in attribute data and
                    maximum time interval is not equal to 0xfffff
                2. maximum interval expired
                3. minimum interval and maximum interval is zero and there is change in value of attribute data
                */
                if ((  ( ReportingBgTasks.minReportExpired || ( 0x0000 == ReportingBgTasks.consolidatedMinTime ))
                && VerifyReportableChange( i ) && ( ReportingBgTasks.consolidatedMaxTime != 0xFFFF ) )
                || ( ReportingBgTasks.maxReportExpired ) ||
                ( (!( ReportingBgTasks.consolidatedMinTime)) && ( !(ReportingBgTasks.consolidatedMaxTime ))
                && VerifyReportableChange( i ) ) )
                {
                    /*send report attribute command*/
                    ReportingBgTasks.flags.bits.bSendreportAttributeCommand = TRUE;
                    reportConfigurationTable[i].SendReportFlag = TRUE;
                    /*update the NVM for report configuration table*/
                    PutReportCfgTable(reportConfigurationTable);
                }
            }
        }
    }
}
#endif /*I_SUPPORT_REPORTING*/


/******************************************************************************
 * Function:       void SE_SendReportAttributeCommand( void );
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 * Return :
 * Side Effects:    None
 *
 * Overview:        This function allocates memory and consolidates the
 *                  attributes into one single report attribute command
 *                  and reports.
 * Note:            None
 *****************************************************************************/
#if I_SUPPORT_REPORTING == 1
void SE_SendReportAttributeCommand( void )
{
    BYTE i = 0x00;
    BYTE dataLength = 0x00;
    BYTE attributeLength = 0x00;
    BYTE memoryAllocated = FALSE;
    APP_DATA_request* pAppDataReq = NULL;
    GetReportCfgTable(reportConfigurationTable);
    for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
    {
        BYTE length;
        
        if (reportConfigurationTable[i].destinationAddressMode == APS_ADDRESS_16_BIT)
        {
            length = SHORT_ADDRESS_LENGTH;
        }
        else
        {
            length = MAX_ADDR_LENGTH;
        }
        
        if( ( TRUE == reportConfigurationTable[i].SendReportFlag ) &&
        ( !( ReportingBgTasks.flags.bits.bAwaitingReportAttributeConfirm )) )
        {
            /*Only once the memory should be allocated for Report attribute command.
            And all attribute records within a cluster should be appended here.*/
            if( memoryAllocated != TRUE )
            {
                pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);
                if(pAppDataReq != NULL)
                {

                    memoryAllocated = TRUE;
                    pAppDataReq->DstAddrMode = reportConfigurationTable[i].destinationAddressMode;
                    memcpy
                    (
                        ( BYTE *)&(pAppDataReq->DstAddress.LongAddr.v[0]),
                        ( BYTE *)&(reportConfigurationTable[i].DstAddress.LongAddr.v[0]),
                        length
                    );
                    //pAppDataReq->DstAddress.ShortAddr = reportConfigurationTable[i].DstAddress.ShortAddr;
                    pAppDataReq->DstEndpoint = reportConfigurationTable[i].destinationEndpoint;
                    pAppDataReq->ProfileId.Val = SE_PROFILE_ID;
                    pAppDataReq->ClusterId.Val = reportConfigurationTable[i].ClusterId;
                    pAppDataReq->SrcEndpoint = reportConfigurationTable[i].srcEndPoint;
                    pAppDataReq->TxOptions.Val = SE_GetClusterSpecificSecurity(pAppDataReq->ClusterId.Val);
                    pAppDataReq->TxOptions.bits.acknowledged = 0;
                    pAppDataReq->RadiusCounter =  DEFAULT_RADIUS;
                    pAppDataReq->asdu[0] = ZCL_ServerToClient | ZCL_GenericCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
                    pAppDataReq->asdu[1] = appNextSeqNum++;
                    /*update the command id as Report attribute command*/
                    pAppDataReq->asdu[2] = ZCL_Report_Attributes_Cmd;
                    /*update the data length for the zcl header size*/
                    dataLength = 0x03;
                }
            }
            /*if the report attribute command is created for same cluster, then
            append the attribute records*/
            if( ( reportConfigurationTable[i].ClusterId == pAppDataReq->ClusterId.Val ) &&
                ( reportConfigurationTable[i].destinationAddressMode == pAppDataReq->DstAddrMode ) &&
                ( !memcmp ( &(reportConfigurationTable[i].DstAddress.LongAddr.v[0]),
                            &(pAppDataReq->DstAddress.LongAddr.v[0]),
                            length ) ) )
            {
                /*get the attribute length*/
                attributeLength = ZCL_GetDataLengthForAttributeDataType
                                      ( reportConfigurationTable[i].attributeDataType, 0x00 );
                /* check for maximum payload length*/
                if( ( dataLength + attributeLength + ZCL_ATTRIBUTE_ID_LENGTH + 1 )
                    <= App_GetMaxPayloadLength() )
                {
                    /*copy the attribute id*/
                    memcpy
                    (
                        &( pAppDataReq->asdu[dataLength]),
                        (BYTE*)&(reportConfigurationTable[i].attributeId),
                        ZCL_ATTRIBUTE_ID_LENGTH
                    );
                    /*increment the data length for attribute id*/
                    dataLength += ZCL_ATTRIBUTE_ID_LENGTH;
                    /*copy the attribute data type*/
                    pAppDataReq->asdu[dataLength++] = reportConfigurationTable[i].attributeDataType;
                    /*copy the attribute data and increment the length
                    for attribute data*/
                    ZCL_Callback_GetAttributeValue(reportConfigurationTable[i].storageIndex,
                    &(pAppDataReq->asdu[dataLength]), attributeLength);
                    dataLength += ( attributeLength );
                    /*reset the send report flag*/
                    reportConfigurationTable[i].SendReportFlag = FALSE;
                }
                else
                {
                    break;
                }
            }
        }

    }
    /*if memory is allocated, then enqueue*/
    if( memoryAllocated != FALSE )
    {
        /*This variable indicates whether all report attribute commands are sent. 
        This will be used to reset the background tasks.*/
        BYTE allReportsSent = TRUE;
        
        pAppDataReq->asduLength = dataLength;
        if ( AILEnqueue( (BYTE *)pAppDataReq, TransmitQueueID ) == AILQueueFailure )
        {
            nfree(pAppDataReq);
        }
        /*Set the report attribute confirmation bit to TRUE*/
        ReportingBgTasks.flags.bits.bAwaitingReportAttributeConfirm = TRUE;
        /*Reset the min reoprt and max report as per their previous values*/
        if( ReportingBgTasks.minReportExpired )
        {
            ReportingBgTasks.minReportExpired = FALSE;
        }
        if( ReportingBgTasks.maxReportExpired )
        {
            ReportingBgTasks.maxReportExpired = FALSE;
        }
        /*This is required to check whether the Report Attribute command is sent
        for all the entries in the Reporting Table.*/
        for( i = 0x00; i < REPORTING_TABLE_SIZE; i++ )
        {
            if( reportConfigurationTable[i].SendReportFlag == TRUE )
            {
                allReportsSent = FALSE;
                break;    
            }
        }
        /*All the report attribute commands are sent, reset the background tasks.*/
        if( allReportsSent == TRUE )
        {
            /*set the background bit to false indicating report attribute command is sent*/
            /*This bit will be set again when the timers expiry or reporatable change occurs*/
            ReportingBgTasks.flags.bits.bSendreportAttributeCommand = FALSE;
        }
    }
    /*write the configuration table into the NVM*/
    PutReportCfgTable(reportConfigurationTable);
}
#endif          /*I_SUPPORT_REPORTING*/

/******************************************************************************
 * Function:        void SE_UpdateReportReceived( APP_DATA_indication* p_dataInd )
 *
 * PreCondition:    None
 *
 * Input:           p_dataInd - data indication
 *
 *
 * Output:
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function checks if the received frame is report attribute
 *                  command and updates the received report attribute flag accordingly
 *****************************************************************************/
#if I_SUPPORT_REPORTING == 1
void SE_UpdateReportReceived( APP_DATA_indication* p_dataInd )
{
    BYTE i;
    WORD attributeId;
    BYTE dataLength = 0x03;
    if( ( ( p_dataInd->asdu[0] & ZCL_ClusterSpecificCommand ) == ZCL_GenericCommand  ) &&
        ( ZCL_Report_Attributes_Cmd == p_dataInd->asdu[2] ) )
    {
        GetReportCfgTable(reportConfigurationTable);
        while( dataLength != p_dataInd->asduLength )
        {
            for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
            {
                if( ( 0xFFFF != reportConfigurationTable[i].timeout ) &&
                    ( reportConfigurationTable[i].ClusterId == p_dataInd->ClusterId.Val ) )
                {
                    /*if cluster id is received, then the attributes would have reported
                    is assumption since the attributes will be consolidated
                    Application has to loop through the Report attribute command and check
                    for which attribute report attribute is received, based on that
                    set the bit */
                    memcpy
                    (
                        (BYTE*)&attributeId,
                        &( p_dataInd->asdu[dataLength] ),
                        0x02
                    );

                    if( attributeId == reportConfigurationTable[i].attributeId )
                    {
                        reportConfigurationTable[i].receivedReportAttribute = TRUE;
                    }
                }
            }
            dataLength += 0x02;
            dataLength += ZCL_GetDataLengthForAttributeDataType
                                    ( p_dataInd->asdu[dataLength], 0x00);
            dataLength++;
        }
        PutReportCfgTable(reportConfigurationTable);
    }
}

#endif /*I_SUPPORT_REPORTING*/

/******************************************************************************
 * Function:        BYTE SE_GetAttributeIndex( void );
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 * Return :
 * Side Effects:    None
 *
 * Overview:
 * Note:            None
 *****************************************************************************/
WORD SE_GetAttributeIndex( WORD attributeId,
    ROM ZCL_ClusterInfo *pZCLClusterInfo, WORD ClusterStorageIndex, BYTE *pAttributeDataType )
{
    BYTE i;
    BYTE dataLength = 0x00;
    for( i = 0x00; pZCLClusterInfo->numberOfAttributes; i++ )
    {
        if( attributeId == pZCLClusterInfo->pListofAttibuteInfo[i].attributeId )
        {
            *pAttributeDataType = pZCLClusterInfo->pListofAttibuteInfo[i].attributeDataType;
            /*return the storage index of the attribute*/
            return ( ClusterStorageIndex + dataLength );
        }
        dataLength += ZCL_GetDataLengthForAttributeDataType
        ( pZCLClusterInfo->pListofAttibuteInfo[i].attributeDataType, 0x00 );
    }
    return 0xFFFF;
}

/******************************************************************************
 * Function:       void ZCL_Callback_ValidateAndCreateReadReportRsp( BYTE direction,
 *                  WORD clusterId, WORD attributeId,
 *                  BYTE *pResponseAsdu, BYTE* pResponseDataLength ,
 *                  ROM ZCL_ClusterInfo *pZCLClusterInfo)
 *
 * PreCondition:    None
 *
 * Input:           direction - gives server to client or client to server
 *                  clusterId - cluster for which the read reporting
 *                  command is received
 *                  attributeId - attribute for which the read reporting
 *                  command is received
 *                  pZCLClusterInfo - points to the cluster info
 *
 *
 * Output:          pResponseAsdu - location where read reporting configuration
 *                  response is created
 *                  pResponseDataLength - holds the zcl header length. It gets
 *                  updated with read reporting configuration response payload
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function checks for all the entries in the reporting
 *                  table and creates the read reporting configuration response
 *****************************************************************************/
void ZCL_Callback_ValidateAndCreateReadReportRsp( BYTE direction, WORD clusterId, WORD attributeId,
  BYTE *pResponseAsdu, BYTE* pResponseDataLength ,ROM ZCL_ClusterInfo *pZCLClusterInfo)
{
    #if I_SUPPORT_REPORTING == 1
	BYTE status = ZCL_Success;
    BYTE i = 0x00;
    BYTE j;
    BYTE responseLength = *pResponseDataLength;

    GetReportCfgTable(reportConfigurationTable);

    for( j = 0x00; j < pZCLClusterInfo->numberOfAttributes; j++ )
    {
        /*check if the cluster id is supported*/
        if( pZCLClusterInfo->pListofAttibuteInfo[j].attributeId == attributeId )
        {
            /*check if the attribute is reportable*/
            if( !( pZCLClusterInfo->pListofAttibuteInfo[j].attributeProperties.bits.reportable ) )
            {
                status = ZCL_UnReportableAttribute;
            }
            break;
       }

    }
    if( status == ZCL_Success )
    {
        /*if the cluster, attribute, direction is seen in the table,
        then create the read reporting configuration response command*/
        for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
        {
            if( ( clusterId == reportConfigurationTable[i].ClusterId ) &&
            ( attributeId == reportConfigurationTable[i].attributeId  ) &&
            ( direction == reportConfigurationTable[i].direction ) )
            {
                break;
            }

        }
        /*if there is no entry for the received request update the status
        to unsupported attribute*/
        if( i == REPORTING_TABLE_SIZE )
        {
            status = ZCL_UnsupportedAttribute;
        }
    }
    /*update the status for Read Reporting configuration response and increment the
    response data length*/
    pResponseAsdu[responseLength] = status;
    responseLength++;
    /*update the direction for Read Reporting configuration response and increment the
    response data length*/
    pResponseAsdu[responseLength] = direction;
    responseLength++;
    /*copy the attribute id and increment the response data length*/
    memcpy
    (
        &(pResponseAsdu[responseLength]),
        (BYTE*)&attributeId,
        ZCL_ATTRIBUTE_ID_LENGTH
    );
    responseLength += ZCL_ATTRIBUTE_ID_LENGTH;

    if( status == ZCL_Success )
    {
        /*if the direction is server to client then fill entries accordingly*/
        if( direction == ReportingDirectionServerToClient )
        {
            /*get the attribute data length*/
            BYTE attributeDataLength = ZCL_GetDataLengthForAttributeDataType
                                    ( reportConfigurationTable[i].attributeDataType, 0x00 );
            /*update the attribute data type in the response pointer*/
            pResponseAsdu[responseLength++] = reportConfigurationTable[i].attributeDataType;
            /*copy the minimum interval*/
            memcpy
            (
                &pResponseAsdu[responseLength],
                (BYTE*)&(reportConfigurationTable[i].minimumReportingInterval),
                0x02
            );
            /*increment the response length for minimum interval*/
            responseLength += 0x02;
            /*copy the maximum interval*/
            memcpy
            (
                &pResponseAsdu[responseLength],
                (BYTE*)&(reportConfigurationTable[i].maximumReportingInterval),
                0x02
            );
            /*increment the length for Maximum reporting interval*/
            responseLength += 0x02;
            /*check  if the attribute is of analog, if so copy the reportable change*/
            if( ZCL_CheckAttributeAnalogDataType( reportConfigurationTable[i].attributeDataType ))
            {
                memcpy
                (
                    &pResponseAsdu[responseLength],
                    (BYTE*)&(reportConfigurationTable[i].reportableChange.arrayVal),
                    attributeDataLength
                );
                /*increment the length for reportable change. The length of reportable
                change is as per the attribute data*/
                responseLength += attributeDataLength;
            }
        }
        else
        {
            /*direction is client to server, copy the timeout*/
            memcpy
            (
                &pResponseAsdu[responseLength],
                (BYTE*)&(reportConfigurationTable[i].timeout),
                0x02
            );
            /*increase the response length for 2 bytes - size of timeout*/
            responseLength += 0x02;
        }
    }
    /*update the response data length in the output pointer*/
    *pResponseDataLength = responseLength;
    /*TBD why this is required*/
    PutReportCfgTable(reportConfigurationTable);
	#endif /*I_SUPPORT_REPORTING*/
}

/******************************************************************************
 * Function:       void App_WriteSingleAttribute(BYTE endpointId,WORD clusterId,
 *                                               WORD attributeId,BYTE attributeDataType,
 *                                               BYTE *pAttributeData);
 * PreCondition:    None
 *
 * Input:           endpointId - this is the endpoint whose attribute is to be written
 *                  clusterId - this is the clusterId whose attribute is to be written
 *                  attributeId - this is attributeID for which attribute data to be written
 *                  attributeDataType - it indicates the datatype of attributeId
 *                  pAttributeData - points to the attributeData,this data is
 *                                   written on NVM.
 *
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to write an attributeData into NVM.
 *****************************************************************************/
void App_WriteSingleAttribute
(
    BYTE endpointId,
    WORD clusterId,
    WORD attributeId,
    BYTE attributeDataType,
    BYTE *pAttributeData
)
{
    /*It holds the NVM storage Index for Given endpoint of a ClusterID */
    WORD storageIndex;
    BYTE directionOfFrame;
    ROM ZCL_ClusterDefinition *pZCLClusterDefinition;
    /*This function updates "storageIndex" with the NVM Location of a given
      endpoint of a ClusterID */
    ZCL_Callback_GetClusterIndex
    (
        endpointId,
        clusterId,
        &storageIndex
    );
    /*Only DRLC cluster has client attributes and all other cluster has server attributes.So directionOfFrame
      for DRLC cluster is Server to client and for all other clusters it is Client to server */
    directionOfFrame = ( clusterId == ZCL_DRLC_CLUSTER )?ZCL_ServerToClient:ZCL_ClientToServer;

    /*It validates the given Endpoint and ClusterID */
    if( ZCL_Success == ZCL_ValidateIncomingFrame( directionOfFrame,endpointId, clusterId, &pZCLClusterDefinition ) )
    {
       /*This function updates the "storageIndex" with the NVM location of a given attributeId*/
        storageIndex = SE_GetAttributeIndex( attributeId,
            pZCLClusterDefinition->pClusterInfo,storageIndex,&attributeDataType );
        /*This function writes the Received attributeData(pAttributeData) at storageIndex of NVM */
        ZCL_Callback_PutAttributeValue( storageIndex, pAttributeData, ZCL_GetDataLengthForAttributeDataType( attributeDataType, 0x00) );
        /*This function checks whether any attribute is excceding its ThresholdValue.If excceds,it
          Generates an alarm and adds corrsponding entry in its alarm table.*/
        ZCL_Callback_CheckForAlarmCondition
        (
           endpointId,
           attributeId,
           pZCLClusterDefinition->pClusterInfo
        );
    }

}

/******************************************************************************
 * Function:        void App_ResetVariables(void)
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
 * Overview:        This function resets the application global variables.
 *
 *****************************************************************************/
void App_ResetVariables(void)
{
#if ( I_SUPPORT_REPORTING == 1 )
    GetReportingBgTasks(&ReportingBgTasks);
    memset
    (
        &ReportingBgTasks,
        FALSE,
        sizeof(Reporting_Background_Tasks)
    );
    PutReportingBgTasks(&ReportingBgTasks);
#endif /*I_SUPPORT_REPORTING*/

    MainsVoltageDwellTripPointHandled = 0x00;
}


/******************************************************************************
 * Function:        void App_SendData
 *                  (
 *                      BYTE addressMode,
 *                      BYTE* pDestAddress,
 *                      BYTE destEndpoint,
 *                      BYTE asduLength,
 *                      WORD clusterId,
 *                      BYTE* pAsdu
 *                  )
 *
 * PreCondition:    None
 *
 * Input:           addressMode -
 *                  pDestAddress -
 *                  destEndpoint -
 *                  asduLength -
 *                  clusterId -
 *                  pAsdu -
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function Puts the data into AIL queue for
 *                  transmission.
 *
 * Note:
 *****************************************************************************/
void App_SendData
(
    BYTE addressMode,
    BYTE* pDestAddress,
    BYTE destEndpoint,
    BYTE asduLength,
    WORD clusterId,
    BYTE* pAsdu
)
{
    APP_DATA_request* pAppDataReq;
    pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);
    if( pAppDataReq != NULL )
    {
        pAppDataReq->DstAddrMode = addressMode;
        if( ( addressMode == APS_ADDRESS_16_BIT ) || ( addressMode == APS_ADDRESS_GROUP ) )
        {
            memcpy
            (
                pAppDataReq->DstAddress.v,
                pDestAddress,
                SHORT_ADDRESS_LENGTH
            );
        }
        if ( addressMode == APS_ADDRESS_64_BIT )
        {
            memcpy
            (
                pAppDataReq->DstAddress.v,
                pDestAddress,
                MAX_ADDR_LENGTH
            );
        }
        pAppDataReq->DstEndpoint = destEndpoint;
        pAppDataReq->ProfileId.Val = SE_PROFILE_ID;
        pAppDataReq->ClusterId.Val = clusterId;
#if I_SUPPORT_CBKE == 1
        /*This is added because if muliplte endpoints are supported on the device, still
        key establishment is performed using only one endpoint.But the established
        link key is applicable for the node.*/
        pAppDataReq->SrcEndpoint = SOURCE_ENDPOINT;
#else
        pAppDataReq->SrcEndpoint = SOURCE_ENDPOINT;
#endif /*#if I_SUPPORT_CBKE == 1*/
        pAppDataReq->TxOptions.Val = SE_GetClusterSpecificSecurity( clusterId );
        if
        (
            ( ZCL_PRICE_CLUSTER == clusterId ) ||
            ( ZCL_DRLC_CLUSTER == clusterId )
            #if I_SUPPORT_CBKE == 1
                || ( ZCL_KEY_ESTABLISHMENT_CLUSTER == clusterId )
            #endif
        )
        {
            pAppDataReq->TxOptions.Val |= NETWORK_SECURITY_WITH_APS_ACK;
        }
        pAppDataReq->RadiusCounter = 0x0a;
        pAppDataReq->asduLength = asduLength;
        memcpy
        (
            pAppDataReq->asdu,
            pAsdu,
            asduLength
        );
        
        if ( AILEnqueue( (BYTE *)pAppDataReq, TransmitQueueID ) == AILQueueFailure )
        {
            nfree(pAppDataReq);
        }
    }
}

/******************************************************************************
 * Function:        void App_SetCurrentTime
 *                  (
 *                      elapsedTimeInfo elapsedTime
 *                  );
 *
 *
 * PreCondition:
 *
 * Input:           elapsedTime - gives the time in the year, month etc format
 *
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function sets the RTC with the values as given by the
 *                  input parameter.
 * Note:
 *****************************************************************************/
void App_SetCurrentTime( elapsedTimeInfo *elapsedTime )
{
    DWORD timeInSeconds ;
    elapsedTimeInfo rtcElapsedTime;
    memcpy
    (
        (BYTE*)&rtcElapsedTime,
        elapsedTime,
        sizeof(elapsedTimeInfo)
    );
    /*Set the current time in the RTC*/
    mRTCCSetSec(ConvertDecimalToHex(rtcElapsedTime.second));
    mRTCCSetMin(ConvertDecimalToHex(rtcElapsedTime.minute));
    mRTCCSetHour(ConvertDecimalToHex(rtcElapsedTime.hour));
    mRTCCSetDay(ConvertDecimalToHex(rtcElapsedTime.day));
    mRTCCSetMonth(ConvertDecimalToHex(rtcElapsedTime.month));
    mRTCCSetYear(ConvertDecimalToHex(rtcElapsedTime.year));
    mRTCCSet();
    timeInSeconds = ZCL_Utility_Time_ConvertYearToSeconds( elapsedTime );
    /*Write the time in the Time attribute too*/
    //ZCL_Time_SetTimeAttribute( 0x01, (BYTE*)&timeInSeconds );
}

/******************************************************************************
 * Function:        DWORD ZCL_Callback_GetCurrentTimeInSeconds(void);
 *
 * PreCondition:
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    DWORD seconds - seconds elapsed
 *
 * Overview:        This function reads the RTC. Fethces the time elapsed
 *                  in the year month format and converts it into seconds
 *                  This value is returned
 *
 * Note:
 *****************************************************************************/
DWORD ZCL_Callback_GetCurrentTimeInSeconds
(
    void
)
{
    elapsedTimeInfo elapsedTime;
    /*After reading from RTC, convert from hex to decimal*/
    elapsedTime.year   = ConvertHexToDecimal( mRTCCGetYear());
    elapsedTime.month  = ConvertHexToDecimal(mRTCCGetMonth());
    elapsedTime.day    = ConvertHexToDecimal(mRTCCGetDay());
    elapsedTime.hour   = ConvertHexToDecimal(mRTCCGetHour());
    elapsedTime.minute = ConvertHexToDecimal(mRTCCGetMin());
    elapsedTime.second = ConvertHexToDecimal(mRTCCGetSec());
    /*convert it into seconds and return the value*/
    return( ZCL_Utility_Time_ConvertYearToSeconds( &elapsedTime ) );

}

/***************************************************************************************
 * Function:        void App_TimeInit()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the Default Time in RTC and Time Attribute in Time Cluster
 *
 * Note:            None
 ***************************************************************************************/
void App_TimeInit()
{
    elapsedTimeInfo defaultTime;

    defaultTime.year = DEFAULT_YEAR;
    defaultTime.month = DEFAULT_MONTH;
    defaultTime.day = DEFAULT_DAY;
    defaultTime.hour = DEFAULT_HOUR;
    defaultTime.minute = DEFAULT_MINUTE;
    defaultTime.second = DEFAULT_SECOND;

    App_SetCurrentTime(&defaultTime);
}

/******************************************************************************
 * Function:        BYTE App_InitAttributeStorage
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
 * Return :         Attribute storage allocation done successfully or not.
 *
 * Side Effects:    None
 *
 * Overview:        This storage is used for storing attribute values.It
 *                  allocates the memory in NVM.
 *
 * Note:            None
 *****************************************************************************/
BYTE App_InitAttributeStorage( void )
{
    BYTE i      = 0x00;
    BYTE result = 0x00;    
    #if (APP_USE_EXTERNAL_NVM == 1)
		#if (I_SUPPORT_SCENES == 1)
			/* Variable to hold the key fro the scenes table */
			WORD scenesTableKay;
			
			WORD key;
		#endif
		
        SPIUnselectEEPROM();
        CLRWDT();
        for( ; i < NoOfEntriesInAttrStorageTable; i++ )
        {
            result |= NVMalloc( App_AttributeStorageTable[i].length, &(App_AttributeStorageTable[i].index) );
        }
        #if (I_SUPPORT_REPORTING == 1)
            result |= NVMalloc( sizeof(Reporting_Background_Tasks), &ReportingBackgroundIndex );
            result |= NVMalloc( (sizeof(ReportConfiguration) * REPORTING_TABLE_SIZE), &ReportCfgTableIndex );
        #endif
        CLRWDT();

	    #if defined(I_SUPPORT_GROUP_ADDRESSING)
			/* allocate the memory for the AppGroupsNameTable in NVM */
	        result |= NVMalloc( sizeof(App_GroupNameRecord) * MAX_GROUP, &AppGroupsNameTable);
	    #endif/*I_SUPPORT_GROUP_ADDRESSING*/

	    #if (I_SUPPORT_SCENES == 1)

			/* Get the address of the key for Scenes table from NVM */
			result |= NVMalloc( 2, &scenesTableKay);

			/* allocate the memory for the Scenes Table in NVM */
	        result |= NVMalloc( sizeof(App_ScenesRecord) * MAX_SCENES, &App_ScenesTable);        
	        
	        NVMRead( (BYTE *)&key, scenesTableKay, 2);

			/* Check if the key has a valid value. If not init the scene table with all FF's and int other variables also */
			if ( key != SCENE_TABLE_KEY )
			{
				key = SCENE_TABLE_KEY;
				
				/* Set the key to proper value */
				NVMWrite( scenesTableKay, (BYTE *)&key, 2);

				Init_ScenesTable();
			}

			//Init_ScenesTable();
	    #endif/*I_SUPPORT_SCENES*/

    #else
        WORD currentIndex = 0x0000;
        
        for( ; i < NoOfEntriesInAttrStorageTable; i++ )
        {
            App_AttributeStorageTable[i].index = currentIndex;
            memcpy
            (
                (NOW_I_AM_A_ROUTER()?&attributeDataStorageInRAM_MTR[currentIndex]:&attributeDataStorageInRAM_ESP[currentIndex]),
                (BYTE *) App_AttributeStorageTable[i].defaultValueLocation,
                App_AttributeStorageTable[i].length
            );
            currentIndex = currentIndex + App_AttributeStorageTable[i].length;
        }

	    #if defined(I_SUPPORT_GROUP_ADDRESSING)
			/* Set the group table with all FF's */
			memset( &AppGroupsNameTable, 0xFF, sizeof(AppGroupsNameTable) );
	    #endif /*I_SUPPORT_GROUP_ADDRESSING*/

	    #if (I_SUPPORT_SCENES == 1)
			Init_ScenesTable();
	    #endif /*I_SUPPORT_SCENES*/

    #endif //(APP_USE_EXTERNAL_NVM == 1)
            
    return result;
}

/******************************************************************************
 * Function:        void ZCL_Callback_GetAttributeValue
 *                  (
 *                      WORD index,
 *                      BYTE *pDest,
 *                      BYTE noOfBytes
 *                  );
 *
 *
 * PreCondition:    None
 *
 *
 *  Input:          index      - From this index we need to retrive the attribute
 *                               value from NVM.
 *                  noOfBytes  - It gives the length of the data to be copied from
 *                               NVM
 *
 * Output:          pDest      - This pointer Points to the Retrived attribute data
 *                               value.
 *
 *
 * Return :
 *
 * Side Effects:    None
 *
 * Overview:        This function is used for getting the attribute value from
 *                  given NVM index.
 *
 * Note:            None
 *****************************************************************************/
void ZCL_Callback_GetAttributeValue(WORD index, BYTE *pDest, BYTE noOfBytes)
{
    #if (APP_USE_EXTERNAL_NVM == 1)
        NVMRead( pDest, index, noOfBytes );
    #else
        memcpy
        (
            pDest,
            (NOW_I_AM_A_ROUTER()?&attributeDataStorageInRAM_MTR[index]:&attributeDataStorageInRAM_ESP[index]),
            noOfBytes
        );
    #endif //(APP_USE_EXTERNAL_NVM == 1)
}

/******************************************************************************
 * Function:        void ZCL_Callback_PutAttributeValue
 *                  (
 *                      WORD index,
 *                      BYTE *pSrc,
 *                      BYTE noOfBytes
 *                  );
 *
 *
 * PreCondition:    None
 *
 *
 *  Input:          pSrc      - This pointer holds the attribute data which is
 *                              needs to be copied into NVM.
 *                  noOfBytes - It gives the length of the data to be copied into
 *                               NVM
 *
 *
 * Output:          index      - This gives the NVM location to where the attribute data
 *                               should be copied into NVM.
 *
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used for writing the attribute value into
 *                  the given NVM index.
 *
 * Note:            None
 *****************************************************************************/
void ZCL_Callback_PutAttributeValue(WORD index, BYTE *pSrc, BYTE noOfBytes)
{
    #if (APP_USE_EXTERNAL_NVM == 1)
        NVMWrite( index, pSrc, noOfBytes );
    #else
        memcpy
        (
            (NOW_I_AM_A_ROUTER()?&attributeDataStorageInRAM_MTR[index]:&attributeDataStorageInRAM_ESP[index]),
            pSrc,
            noOfBytes
        );
    #endif //(APP_USE_EXTERNAL_NVM == 1)
}

/******************************************************************************
 * Function:        BYTE ZCL_Callback_GetClusterIndex
 *                  (
 *                      BYTE endpointId,
 *                      WORD clusterId ,
 *                      WORD* pStorageIndex
 *                  );
 *
 *
 * PreCondition:    None
 *
 *  Input:          endpointId - For given endpoint we need get NVM Index for
 *                               storing ATTRIBUTE VALUE.
 *                  clusterId  - For given clusterId we need get NVM Index for
 *                               storing ATTRIBUTE VALUE
 *
 *
 * Output:          pStorageIndex - It will be updated with storage Index value
 *                                  for a given endpointId and a given cluster.
 *
 *
 * Return :         It returns TRUE or 0xFF
 *
 * Side Effects:    None
 *
 * Overview:        This used for getting NVM index for the given EndpointID,
 *                  clusterID.
 * Note:            None
 *****************************************************************************/
BYTE ZCL_Callback_GetClusterIndex(BYTE endpointId,WORD clusterId ,WORD* pStorageIndex )
{
    BYTE i = 0x00;
     for( ; i < NoOfEntriesInAttrStorageTable; i++ )
     {
         if( ( endpointId == App_AttributeStorageTable[i].endpointId )&&
                   ( clusterId == App_AttributeStorageTable[i].clusterId ) )
         {
            *pStorageIndex = App_AttributeStorageTable[i].index;
            return TRUE;
         }
     }
     return 0xFF;

}

/***************************************************************************************
 * Function:        BYTE App_HandleReceivedData(APP_DATA_indication* p_dataInd)
 *
 * PreCondition:    None
 *
 * Input:           p_dataInd - It holds the received ZCL command frame.
 *
 * Output:          None
 *
 * Return:
 * Side Effects:    None
 *
 * Overview:        This function is invoked by the Application when a ZCL frame is received.
 *
 * Note:            None
 *
 ***************************************************************************************/
BYTE App_HandleReceivedData( APP_DATA_indication* p_dataInd )
{
    /*status is assigned with success assuming most of the time, the parsing is
    successful.The status is updated after parsing the frame.*/
    BYTE status = ZCL_Success;
    /*This variable indicates the direction of ZCL command i.e "Client to Server" or
    "Server to Client"*/
    BYTE directionOfFrame;
    ROM ZCL_ClusterDefinition *pZCLClusterDefinition;
    /*This pointer indicates the location where the response command frame needs
    to be created*/
    BYTE* pLocationForResponse;
    ZCL_ActionHandler pActionHandler;
    /*initialise the security mapping to NO_SECURITY. This is updated based on the
    security received*/
    BYTE receivedSecurity = NO_SECURITY;
    BYTE expectedSecurity;
    
    expectedSecurity = SE_GetClusterSpecificSecurity(p_dataInd->ClusterId.Val);

    if( APS_SECURED_NWK_KEY == p_dataInd->SecurityStatus )
    {
        receivedSecurity = NETWORK_SECURITY;
    }
    else if( APS_SECURED_LINK_KEY == p_dataInd->SecurityStatus )
    {
        receivedSecurity = HIGH_SECURITY;
    }
    /*If Default Response command is received, then just indicate to higher layer.
    No further processing is required. Default Response command is General Command.
    CommandIds may be same for General and Cluster Specific Commands. So, before
    checking for the commandId, commandType (Generic or Cluster Specific)
    shall be verified.*/
    if( ( !( p_dataInd->asdu[0] & ZCL_CLUSTER_SPECIFIC_COMMAND ) ) &&
        ( p_dataInd->asdu[2] == ZCL_Default_Response_Cmd ) )
    {
        App_UpdateParamsForTestTool( p_dataInd );
        App_HandleDataIndication();
        return ZCL_Failure;
    }

    /* This condition checks whether the received ZCL command is using Valid security or not.
       According to the SE specification,if received ZCL command is not using valid
       security,then we need send "Default Response as ZCL_FAILURE".  */
    //if( SE_GetClusterSpecificSecurity(p_dataInd->ClusterId.Val) != receivedSecurity )
    if ( ( receivedSecurity == NO_SECURITY ) ||
         ( ( expectedSecurity == HIGH_SECURITY ) && ( receivedSecurity != HIGH_SECURITY ) ) )
    {
        /*TRUE is because security check has been failed. Only in this case,
        Default Response command should be sent with NETWORK SECURITY. In all
        other cases, respective cluster specific security shall be applied to
        the Default Response command*/
        SE_SendDefaultResponse(ZCL_Failure, TRUE, p_dataInd);
        return ZCL_Failure;
    }
    /*if the security mapping is successful, then the get the direction of the
    received frame to parse further*/
    directionOfFrame = ZCL_GetDirectionOfCommand( p_dataInd->asdu );
    /*This function is used to validate the received ZCL frame.*/
   /*Each device maintains "ZCL_DeviceInfo".*/
   /*If the received cluster & received endpoint is not supported
    by the device, the output parameter pZCLClusterDefinition is assigned with
    NULL_PTR and returns status as ZCL_FAILURE.
    If the received cluster is supported as server cluster by the given endpoint,
    the output parameter pZCLClusterDefinition is assigned with the cluster
    definition of the received cluster and returns ZCL_Success.*/
    status =    ZCL_ValidateIncomingFrame
                (
                    directionOfFrame,
                    p_dataInd->DstEndpoint,
                    p_dataInd->ClusterId.Val,
                    &pZCLClusterDefinition
                );

    /*If Received ZCL Frame validation is failed then we need to send "Default Response
      as "ZCL_FAILURE", otherwise we need to process the ZCL command*/
    if ( status == ZCL_Success )
    {
       APP_DATA_request* pAppDataReq;

       /* Memory Allocated is for creating Response Frame.This memory will
        be freed by the lower layer*/
       pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);
       /* This holds the event id and the event data for the received command.
        The fields are updated by the ZCL module.This memory is freed
        at the end of this function before returning*/
       pActionHandler.event.pEventData = SRAMalloc(80);
        /*check if the Allocated buffers are successfully*/
       if( ( pAppDataReq != NULL ) &&
           ( pActionHandler.event.pEventData != NULL ) )
       {

            if( NOW_I_AM_A_CORDINATOR() ){	//#if ( I_AM_ESP == 1 )		//MSEB
                /*Check if the read attribute response command is for time cluster
                and if the time cluster is the client for the endpoint*/
                if
                (
                    p_dataInd->ClusterId.Val == ZCL_TIME_CLUSTER &&
                    p_dataInd->asdu[2] == ZCL_Read_Attribute_Cmd
                )
                {
                    BYTE i;
                    WORD AttrID;
                    WORD storageIndex;
                    ZCL_TimeClusterAttribute    timerClusterAttributes;

                    for (i = ZCL_HEADER_LENGTH_WO_MANUF_CODE; i < p_dataInd->asduLength; i+= ZCL_ATTRIBUTE_ID_LENGTH)
                    {
                        AttrID = (p_dataInd->asdu[i + 1] << 8);
                        AttrID += p_dataInd->asdu[i];
                        if (ZCL_TIME_ATTRIBUTE == AttrID)
                        {
                            DWORD currentTime;

                            currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
                            ZCL_Time_SetTimeAttribute( SOURCE_ENDPOINT, (BYTE *)&currentTime);
                            
                            /* Get the current time */
                            //current_Time = ZCL_Callback_GetCurrentTimeInSeconds();

                            /* update the time cluster attributes here as well, localtime especially  */
                            /* For reading Meters MAX no of intervals from NVM we
                             * need get the NVM Index Value */
                            if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                                    ZCL_Callback_GetClusterIndex
                                    (
                                        ESP_ENDPOINT_ID,
                                        ZCL_TIME_CLUSTER,
                                        &storageIndex
                                    )
                               )
                            {
                                /* Read the ZCL Time Cluster Server Attributes from NVM */
                                ZCL_Callback_GetAttributeValue
                                (
                                    storageIndex,
                                    (BYTE *)&timerClusterAttributes,
                                    sizeof(ZCL_TimeClusterAttribute)
                                );
                                
                                /* update the time attributes */
                                timerClusterAttributes.standardTime     = timerClusterAttributes.timeAttribute + timerClusterAttributes.timeZone;
                                timerClusterAttributes.localTime        = timerClusterAttributes.standardTime; 
                                
                                ZCL_Callback_PutAttributeValue
                                (
                                    storageIndex,
                                    (BYTE *)&timerClusterAttributes,
                                    sizeof(ZCL_TimeClusterAttribute)
                                );
                            }
                            
                            break;
                        }
                    }
                }
            }//#endif  /*I_AM_ESP == 1*/

            if
            (
                p_dataInd->ClusterId.Val == ZCL_IDENTIFY_CLUSTER &&
                p_dataInd->asdu[2] == ZCL_Read_Attribute_Cmd &&
                ( ( p_dataInd->asdu[0] & ZCL_FRAME_TYPE_MASK ) == ZCL_GenericCommand )
            )
            {
                WORD storageIndex;
                /* For storing Identify Time into NVM we need get the NVM
                Index Value */
                if ( EP_CLUSTERID_NOT_FOUND_IN_NVM !=
                    ZCL_Callback_GetClusterIndex
                    (
                        SOURCE_ENDPOINT,
                        ZCL_IDENTIFY_CLUSTER,
                        &storageIndex
                    ) )
                {

                    /* This function will store the identifyTime value into NVM */
                    ZCL_Callback_PutAttributeValue
                    (
                        storageIndex,
                        (BYTE*)&identifyTimeoutVal,
                        SIZE_OF_IDENTIFY_TIME_ATTRIBUTE
                    );

                }
            }
            if
            (
                p_dataInd->ClusterId.Val == ZCL_IDENTIFY_CLUSTER &&
                p_dataInd->asdu[2] == ZCL_Write_Attributes_Cmd &&
                ( ( p_dataInd->asdu[0] & ZCL_FRAME_TYPE_MASK ) == ZCL_GenericCommand )
            )
            {
                memcpy
                (
                    (BYTE*)&identifyTimeoutVal,
                    &p_dataInd->asdu[6],
                    SIZE_OF_IDENTIFY_TIME_ATTRIBUTE
                );
            }
            /*assign the default event id for error handling*/
            pActionHandler.event.eventId = 0x00;
            pActionHandler.action = No_Response;
            /*assign the pointer with the start location of asdu*/
            pLocationForResponse = pAppDataReq->asdu;            

            /*--------------------------------------------------------------------------------*/
            /*This condition check is required to extract the Extended Address 
            of the device from with the data has been originated. This is useful 
            in updating/configuring some of the attributes for various clusters. 
            For example: Configure reporting requires the extended address of 
            source device originating the command. Scenes Cluster requires the 
            extended address of the source device originating the command for updating 
            the last configured device addresss attribute.*/
            if( p_dataInd->SrcAddrMode == APS_ADDRESS_16_BIT )
            {
                LONG_ADDR longAddr;
                
                if( IsThisAddressKnown(p_dataInd->SrcAddress.ShortAddr, longAddr.v) )
                {
                    BYTE i;
                    
                    for (i = 0; i < 8; i++)
                    {
                        InitiatorDeviceInfo.initiatorLongAddress.LongAddr.v[7-i] = longAddr.v[i];
                    }
                }
                InitiatorDeviceInfo.initiatorAddressMode = APS_ADDRESS_16_BIT;
                memcpy
                (
                    ( BYTE *)&(InitiatorDeviceInfo.initiatorShortAddress.v[0]),
                    ( BYTE *)&(p_dataInd->SrcAddress.ShortAddr.v[0]),
                    SHORT_ADDRESS_LENGTH
                );
            }
            else
            {
                InitiatorDeviceInfo.initiatorAddressMode = APS_ADDRESS_64_BIT;
                memcpy
                (
                    ( BYTE *)&(InitiatorDeviceInfo.initiatorLongAddress.LongAddr.v[0]),
                    ( BYTE *)&(p_dataInd->SrcAddress.LongAddr.v[0]),
                    MAX_ADDR_LENGTH
                );
            }
            
            InitiatorDeviceInfo.initiatorEndPoint = p_dataInd->SrcEndpoint;
            /*--------------------------------------------------------------------------------*/
            
            /*This function processes the "Client to Server" ZCL Commands.This function
            returns the response asdu length and updates the output parameters,
            pLocationForResponse and  pActionHandler */
            pAppDataReq->asduLength =
                            ZCL_ProcessReceivedFrame
                            (
                                p_dataInd->DstEndpoint,
                                p_dataInd->asduLength,
                                p_dataInd->asdu,
                                pLocationForResponse,
                                &pActionHandler,
                                pZCLClusterDefinition
                            );
            /* ZCL_ProcessReceivedFrame function updates the outparmeter,pActionHandler.action.*/
            /* pActionHandler.action is Send_Response, then we need send ZCL response */
            if( pActionHandler.action == Send_Response )
            {
                /*Default Response command frame needs to be transmitted only when the received
                frame is a unicast frame and does not have any specific response to the received frame.
                If the received frame was a broadcast or groupcast frame, then we should not send
                the Default Response command.
                For few of the Scenes Cluster commands which are received NOT as unicast command, we should not 
                send out the response.*/
                if ( ( ( *( pLocationForResponse + ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) == ZCL_Default_Response_Cmd ) 
                       #if (I_SUPPORT_SCENES == 1)
                       ||
                       ( *( pLocationForResponse + ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) == ZCL_Scenes_RemoveSceneResponseCmd ) ||
                       ( *( pLocationForResponse + ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) == ZCL_Scenes_RemoveAllScenesResponseCmd ) ||
                       ( *( pLocationForResponse + ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) == ZCL_Scenes_StoreSceneResponseCmd ) 
                       #endif
                     ) &&
                     ( ( p_dataInd->DstAddrMode == APS_ADDRESS_GROUP ) ||
                       ( ( p_dataInd->DstAddrMode == APS_ADDRESS_16_BIT ) &&
                         ( ( p_dataInd->DstAddress.ShortAddr.Val == BROADCAST_ADDRESS ) ||
                           ( p_dataInd->DstAddress.ShortAddr.Val == BROADCAST_RX_ON_ADDRESS ) || 
                           ( p_dataInd->DstAddress.ShortAddr.Val == BROADCAST_ALL_ROUTERS_ADDRESS ) ) ) ) )
                {
                    nfree(pAppDataReq);
                }
                else
                {
                    /*update the destination address mode*/
                    pAppDataReq->DstAddrMode = InitiatorDeviceInfo.initiatorAddressMode;
                    
                    /*check if the addressing is short address*/
                    if( pAppDataReq->DstAddrMode == APS_ADDRESS_16_BIT )
                    {
                        /*Update the destination address*/
                         memcpy
                         (
                            ( BYTE *)&(pAppDataReq->DstAddress.ShortAddr.v[0]),
                            ( BYTE *)&(InitiatorDeviceInfo.initiatorShortAddress.v[0]),
                             SHORT_ADDRESS_LENGTH
                         );
    
                    }
                    /*check if the addressing is IEEE address */
                    if( pAppDataReq->DstAddrMode == APS_ADDRESS_64_BIT )
                    {
                        /*Update the destination address*/
                        memcpy
                        (
                            ( BYTE *)&(pAppDataReq->DstAddress.LongAddr.v[0]),
                            ( BYTE *)&(InitiatorDeviceInfo.initiatorLongAddress.LongAddr.v[0]),
                            MAX_ADDR_LENGTH
                        );
                    }
                    /*Update Dest Endpoint*/
                    pAppDataReq->DstEndpoint = p_dataInd->SrcEndpoint;
                    /*Update ProfileId*/
                    pAppDataReq->ProfileId  = p_dataInd->ProfileId;
                    /*Update clusterId*/
                    pAppDataReq->ClusterId = p_dataInd->ClusterId;
                    /*Update Src Endpoint*/
                    pAppDataReq->SrcEndpoint = p_dataInd->DstEndpoint;
                    /*Get the Appropriate Security for the cluster*/
                    pAppDataReq->TxOptions.Val = SE_GetClusterSpecificSecurity(p_dataInd->ClusterId.Val);
                     pAppDataReq->RadiusCounter = DEFAULT_RADIUS;
                    /*Enqueue for AIL layer to perform data transmission*/
                     if ( AILEnqueue( (BYTE *)pAppDataReq, TransmitQueueID ) == AILQueueFailure )
                     {
                         nfree(pAppDataReq);
                     }
                } 
            }
            /*if the action is received response, give it to Application*/
            else if( pActionHandler.action == Received_Response )
            {
                if
                (
                    ( ZCL_DRLC_CLUSTER == p_dataInd->ClusterId.Val) &&
                    ( ZCL_ReportEventStatusCmd == p_dataInd->asdu[2] ) &&
                    ( ZCL_ClientToServer == directionOfFrame )
                )
                {
                    #if I_SUPPORT_CBKE == 1
                        if
                        (
                            CBKE_ApplyOrVerifyDSASignature
                            (
                                FALSE,
                                ( p_dataInd->asduLength - 3 ),
                                pActionHandler.event.pEventData
                            )
                        )
                        {
                            nfree(pAppDataReq);
                            nfree(pActionHandler.event.pEventData);
                            return ZCL_Failure;
                        }

                    #endif
                    /* Remove the signature type and signature */
                    p_dataInd->asduLength -= ( SIGNATURE_SIZE + 1 );
                }
                App_UpdateParamsForTestTool( p_dataInd );

                if( ( ZCL_Read_Attribute_Response_Cmd == p_dataInd->asdu[2] ) &&
                    ( ZCL_ServerToClient == directionOfFrame ) &&
                    ( p_dataInd->asdu[5] == ZCL_Success ) )
                {
                    if( NOW_I_AM_NOT_A_CORDINATOR() ){ //#if ( I_AM_ESP == 0)		//MSEB

                        /*Check if the read attribute response command is for time cluster
                        and if the time cluster is the client for the endpoint*/
                        if ( p_dataInd->ClusterId.Val == ZCL_TIME_CLUSTER )
                        {
                            ZCL_CheckTimeAttribute( p_dataInd->DstEndpoint, p_dataInd->asdu,
                                p_dataInd->asduLength );
                            
                           // if( pSeconds != NULL_PTR )
                           // {
                                /*synchronize the time, update the time attribute
                                and set the RTC to tick with the available seconds*/

                                //Time_Synchronization( p_dataInd->DstEndpoint,
                                //       pSeconds );
                               // #if( I_AM_MTR == 1 )
                                    /* For Simulating meterReadings,we need to enable backgroundTask.*/
                                 //   meterStatus.flags.bits.bSimulateMeterReading = 1;
                                //    meterStatus.simulateMeterStartTime = TickGet();
                                //#endif      /* I_AM_MTR */
                           // }
                        }
                    }//#endif  /* I_AM_ESP == 0 */

                    if( NOW_I_AM_A_CORDINATOR() ){ //#if ( I_AM_ESP == 1 )		//MSEB
                        /*Check if the read attribute response command is for Meter cluster*/
                        if ( ZCL_METERING_CLUSTER == p_dataInd->ClusterId.Val )
                        {
                            WORD attributeId;
                            BYTE meterReadingValue[6];
                            memcpy
                            (
                                (BYTE*)&attributeId,
                                (BYTE*)&(p_dataInd->asdu[3]),
                                0x02
                            );
                            if( (
                                  ( ( attributeId == ZCL_METERING_CURRENT_SUMMATION_DELIVERED ) ||
                                    ( attributeId > ZCL_METERING_CURRENT_SUMMATION_DELIVERED )
                                  )
                                  #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                                  && ( attributeId <= ZCL_METERING_DFT_SUMMATION )
                                  #endif
                                )
                                #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                                ||
                                (
                                  ( ( attributeId == ZCL_METERING_CURRENT_TIER1_SUMMATION_DELIVERED ) ||
                                    ( attributeId > ZCL_METERING_CURRENT_TIER1_SUMMATION_DELIVERED )
                                  ) &&
                                  ( attributeId <= ZCL_METERING_CURRENT_TIER6_SUMMATION_RECEIVED )
                                )
                                #endif
                             )
                            {

                                memcpy
                                (
                                    meterReadingValue,
                                    (BYTE*)&(p_dataInd->asdu[7]),
                                    0x06
                                );
                                FormatMeterReadingValue(meterReadingValue,0x06);
                                displayState = GetDisplayStateForMeterAttributes(attributeId);
                           }
                           #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                           if( ( ( attributeId >= ZCL_METERING_INSTANTANEOUS_DEMAND )&&
                                  ( attributeId <= ZCL_METERING_PREVIOUS_DAY_CONSUMPTION_RECEIVED ))||
                                  ( attributeId == ZCL_METERING_CURRENT_PARTIAL_PROF_INT_VALUE_DELIVERED )||
                                  ( attributeId == ZCL_METERING_CURRENT_PARTIAL_PROF_INT_VALUE_RECEIVED ))
                           {
                                memcpy
                                (
                                    meterReadingValue,
                                    (BYTE*)&(p_dataInd->asdu[7]),
                                    0x03
                                );
                                FormatMeterReadingValue(meterReadingValue,0x03);
                                displayState = GetDisplayStateForMeterAttributes(attributeId);
                           }
                           #endif
                           if( attributeId == ZCL_METERING_UNIT_OF_MEASURE )
                           {
                                meterFormatAttributes.unitOfMeasure =  p_dataInd->asdu[7];
                           }
                           #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                           if( attributeId == ZCL_METERING_MULTIPLIER )
                           {
                                meterFormatAttributes.multiplier = 0x00000000;
                                memcpy
                                (
                                    (BYTE*)&(meterFormatAttributes.multiplier),
                                    (BYTE*)&(p_dataInd->asdu[7]),
                                    0x03
                                );
                           }
                           #endif
                           #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                           if( attributeId == ZCL_METERING_DIVISOR )
                           {
                                meterFormatAttributes.divisor = 0x00000000;
                                memcpy
                                (
                                    (BYTE*)&(meterFormatAttributes.divisor),
                                    (BYTE*)&(p_dataInd->asdu[7]),
                                    0x03
                                );
                           }
                           #endif
                           if( attributeId == ZCL_METERING_SUMMATION_FORMATTING )
                           {
                                meterFormatAttributes.summationFormatting =  p_dataInd->asdu[7];
                           }
                           #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                           if( attributeId == ZCL_METERING_DEMAND_FORMATTING )
                           {
                                meterFormatAttributes.demandFormatting =  p_dataInd->asdu[7];
                           }
                           #endif
                           #if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01)
                           if( attributeId == ZCL_METERING_HISTORICAL_CONSUMPTION_FORMATTING )
                           {
                                meterFormatAttributes.historicalFormatting =  p_dataInd->asdu[7];
                           }
                           #endif
                        }

                    }//#endif /*( I_AM_ESP == 1 )*/

                    #if ( I_AM_IPD == 1 )
                    #if ( I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
                        /*Check if the read attribute response command is for Meter cluster*/
                        if ( ZCL_METERING_CLUSTER == p_dataInd->ClusterId.Val )
                        {
                            WORD attributeId;

                            memcpy
                            (
                                (BYTE*)&attributeId,
                                (BYTE*)&(p_dataInd->asdu[3]),
                                0x02
                            );
                            if(

                                   ( attributeId == ZCL_METERING_CURRENT_TIER1_SUMMATION_DELIVERED ) ||
                                   ( attributeId == ZCL_METERING_CURRENT_TIER2_SUMMATION_DELIVERED ) ||
                                   ( attributeId == ZCL_METERING_CURRENT_TIER3_SUMMATION_DELIVERED ) ||
                                   ( attributeId == ZCL_METERING_CURRENT_TIER4_SUMMATION_DELIVERED ) ||
                                   ( attributeId == ZCL_METERING_CURRENT_TIER5_SUMMATION_DELIVERED ) ||
                                   ( attributeId == ZCL_METERING_CURRENT_TIER6_SUMMATION_DELIVERED )  )
                            {

                                memcpy
                                (
                                    TiermeterReadingValue,
                                    (BYTE*)&(p_dataInd->asdu[7]),
                                    0x06
                                );

                                displayState =(BYTE)( attributeId - 0x0100 + receivedTier1MeterReading);

                           }

                        }
                    #endif
                    #endif /*( I_AM_IPD == 1 )*/
                }
                App_HandleDataIndication();
                nfree(pAppDataReq);
            }
            else if ( pActionHandler.action == No_Response )
            {
                nfree(pAppDataReq);
            }
            //SE_HandleEvents( &( pActionHandler.event ), p_dataInd );
            (NOW_I_AM_A_ROUTER()?SE_HandleEvents_MTR( &( pActionHandler.event ), p_dataInd ):SE_HandleEvents_ESP( &( pActionHandler.event ), p_dataInd ));
            nfree(pActionHandler.event.pEventData);
        }
    }
    else
    {
        /*Since this is not the Security failure case, respective cluster specific
        security shall be applied to the Default Response command*/
        SE_SendDefaultResponse(ZCL_Failure, FALSE, p_dataInd);
    }
    return status;
}

/******************************************************************************
 * Function:        BYTE SE_ValidateSAS(Commissioning_Cluster_Attributes *commAttribute)
 *
 * PreCondition:    None
 *
 * Input:           commAttribute- Commissioning cluster attributes
 *
 * Output:          None
 *
 * Return :         status of validation
 *
 * Side Effects:    None
 *
 * Overview:        validates the commissioning cluster attributes for consistency.
 *                  This function will be called before restarting device,
 *                  using ZCL commissioning cluster attributes.
 *
 * Note:            None
 *****************************************************************************/
//#if (I_AM_ESP == 0)		//MSEB
BYTE SE_ValidateSAS(Commissioning_Cluster_Attributes *commAttribute)
{
    BYTE i;
    BYTE status = Inconsistent_Startup_State;

    /* Verifying Short address */
    if( commAttribute->shortAddr > 0xFFF7 )
    {
        /* short address cannot be reserved values.*/
        return status;
    }

    /* Verifying Extended PANID */
    for( i = 0; i < MAX_ADDR_LENGTH; i++ )
    {
        /* Check any of the bytes of the extended PANID is not 0xFF */
        if( commAttribute->ExtendedPANId[i] != 0xFF)
        {
            break;
        }
    }
    /* Extended PANID can't be 0xFFFFFFFFFFFFFFFF */
    if (i == MAX_ADDR_LENGTH)
    {
        return status;
    }

    if
    (
        /* Channels can be between 0x800 (channel 11) and 0x04000000(channel 26) */
        (commAttribute->ChannelMask & 0xF80007FF ) ||
        /* protocol version can only be nwkcProtocolVersion */
        (commAttribute->ProtocolVersion != nwkcProtocolVersion) ||
        /* stack profile can only be ZIGBEE_2007_STACK_PROFILE or ZIGBEE_PRO_STACK_PROFILE*/
        ((commAttribute->StackProfile != ZIGBEE_2007_STACK_PROFILE) && (commAttribute->StackProfile != ZIGBEE_PRO_STACK_PROFILE)) ||
        /* Startup control cannot be more than 0x03 */
        (commAttribute->StartupControl > 0x03)
    )
    {
        return status;
    }

    if (NOW_I_AM_NOT_A_CORDINATOR()){ //#ifndef I_AM_COORDINATOR
        /* If the device is not coordinator, it cannot form the network */
        if( FORM_NETWORK == commAttribute->StartupControl )
        {
	        //Commented to enable commissioning the MTR into ESP with device reset
            //return status;
        }
        /* For rejoining the network extended PANID attribute Id (0x0001)
           should have valid extended panid */
        if
        (
            ( commAttribute->StartupControl == REJOIN_NETWORK ) ||
            ( commAttribute->StartupControl == SILENT_START_NETWORK )
        )
        {
            for( i = 0; i < MAX_ADDR_LENGTH; i++ )
            {
                /* Check any Byte in the extended panid is having non zero value. */
                if( commAttribute->ExtendedPANId[i] != 0x00)
                {
                    break;
                }
            }
        }

        if (MAX_ADDR_LENGTH == i)
        {
            /* If StartupControl is REJOIN_NETWORK, Extended PANID cannot be 0x0000000000000000. */
            return status;
        }

        if( commAttribute->StartupControl == ASSOC_JOIN_NETWORK )
        {
            for( i = 0; i < MAX_ADDR_LENGTH; i++ )
            {
                /* Check any byte of extended panid has non zero value */
                if( commAttribute->ExtendedPANId[i] != 0x00)
                {
                    /* If StartupControl is ASSOC_JOIN_NETWORK, Extended PANID should be 0x0000000000000000. */
                    return status;
                }
            }
        }
    }else{//#else /* I_AM_COORDINATOR */
        /* Coordinator can only form the network */
        if
        (
            ( FORM_NETWORK != commAttribute->StartupControl ) ||
            ( SILENT_START_NETWORK != commAttribute->StartupControl )
        )
        {
            return status;
        }
    }//#endif


    /* Verifying Trust center address */
    for( i = 0; i < MAX_ADDR_LENGTH; i++ )
    {
        if( commAttribute->TrustCenterAddress[i] != 0xFF)
        {
            break;
        }
    }
    /* Trust center's extended address can't be 0xFFFFFFFFFFFFFFFF */
    if( i == MAX_ADDR_LENGTH)
    {
         /* Trust center's extended address in inconsitence state */
        return status;
    }

    /* Trust center's extended address can't be 0x0000000000000000 */
    for( i = 0; i < MAX_ADDR_LENGTH; i++ )
    {
        if( commAttribute->TrustCenterAddress[i] != 0x00)
        {
            break;
        }
    }

    if( i == MAX_ADDR_LENGTH)
    {
        /* Trust center's extended address in inconsitence state */
        return status;
    }


    if( /* UseInsecured join can be true or false */
        ( ( commAttribute->UseInsecureJoin != TRUE) && (commAttribute->UseInsecureJoin != FALSE )) ||
        /* Network key type can only be 0x02 or 0x05 */
        ( ( commAttribute->NetworkKeyType != 0x02 ) && ( commAttribute->NetworkKeyType != 0x05 ) ) ||
        /* Network manager address should be a valid short address */
        ( commAttribute->NetworkManagerAddress > 0xFFF7 )
      )
    {
        /* Commissioning attribute set is in a inconsistence state */
        return status;
    }
    return ZCL_Success;
}
//#endif /*(I_AM_ESP == 0)*/

/******************************************************************************
 * Function:        void SE_HandleIdentifyTimeout( BYTE endpointId );
 *
 * PreCondition:    None
 *
 * Input:           None.
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function handles the Identification Procedure i.e Flashing
 *                  (Toggling)of LED and decrementing the IdentifyTime value
 *                  for every one second.
 *
 * Note:            None
 *****************************************************************************/
void SE_HandleIdentifyTimeout( BYTE endpointId )
{
    /*If IdentifyTime attribute value reaches zero, then we need to
     disable Identification Procedure.This is done by disabling background
     Task(appIdentifyTimers.IdentifyTime = 0x00)
     else we need to decrement IdentifyTime attribute value by one,we
     need to store the same value into NVM.*/
    if ( identifyTimeoutVal-- )
    {
        LIGHT3_TOGGLE();
    }
    else
    {
        appIdentifyTimers.IdentifyTime = 0x00;
        LIGHT3_OFF();
    }
}

/******************************************************************************
 * Function:        void SE_SendCommissioningClusterResponse
 *                  (
 *                      APP_DATA_indication *p_dataInd,
 *                      BYTE responseType,
 *                      BYTE status
 *                  )
 *
 * PreCondition:    None
 *
 * Input:           p_dataInd - commissioning cluster request received
 *                              command payload
 *                  responseType - Command for which commissioning cluster
 *                              response needs to be sent
 *                  status - status of comm cluster command processing
 *
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        Sends the commissioning cluster response depending on
 *                  request received and status of processing the request.
 *
 * Note:            None
 *****************************************************************************/
//#if (I_AM_ESP == 0)		//MSEB
void SE_SendCommissioningClusterResponse
(
    APP_DATA_indication *p_dataInd,
    BYTE responseType,
    BYTE status
)
{
    BYTE asdu[10];

    /* ZCL Header */
    /* Frame Control */
    *( asdu) = ZCL_ClusterSpecificCommand | ZCL_ServerToClient;
    *( asdu + ZCL_FRAME_TSN_POSITION_MANUFACTURE_BIT_DISABLED ) = appNextSeqNum;
    appNextSeqNum++;

    *( asdu + ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) =
        responseType;
    *( asdu + COMM_RESP_STATUS_INDEX ) = status;

    App_SendData
    (
        APS_ADDRESS_16_BIT,
        &(p_dataInd->SrcAddress.ShortAddr.v[0]),
        p_dataInd->SrcEndpoint,
        COMMISSIONING_CLUSTER_RESPONSE_ASDU_LENGTH,
        ZCL_COMMISSIONING_CLUSTER,
        asdu
    );
}
//#endif /*(I_AM_ESP == 0)*/

/***************************************************************************************
 * Function:        BYTE App_ValidateProfileId ( WORD profileID, BYTE endPoint )
 *
 * PreCondition:    None
 *
 * Input:           profileID - Profile Id specified in the received frame that needs to
 *                              be matched
 *                  endPoint -  End Point as specified in the received frame that needs
 *                              to be matched
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to validate whether the specified end point
 *                  supports the profile as specified in the received frame. If yes,
 *                  then it returns TRUE. Otherwise it returns FALSE.
 *
 * Note:            None
 *
 ***************************************************************************************/
BYTE App_ValidateProfileId ( WORD profileID, BYTE endPoint )
{
    NODE_SIMPLE_DESCRIPTOR  simpleDescriptor;
    BYTE index;

    /*Get Each Simple Descriptor. Zeroth index is reserved for ZDO End Point and should
    not be checked for the Profile ID for the Application Data.
    Application related Simple Descriptor will start from next index onwards.*/
    for (index=1; index<NUM_DEFINED_ENDPOINTS; index++)  /* Do not include EP0 */
    {
        ProfileGetSimpleDesc( &simpleDescriptor, index );
        if ( simpleDescriptor.Endpoint == endPoint )
        {
            if ( simpleDescriptor.AppProfId.Val == profileID )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/******************************************************************************
 * Function:        void App_StartDevice(void)
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
 * Overview:        This function optionally(if startUpMode = 0x00) adds
 *                  Commissioning cluster attribute values into stack SAS
 *                  and issues start device request.
 *
 * Note:            None
 *****************************************************************************/
//#if (I_AM_ESP == 0)		//MSEB
void App_StartDevice(void)
{
    BYTE startUpMode;
    commissioningRestartInProgress = FALSE;
    STARTUP_ATTRIBUTE_SET commAttribute;
    BYTE *ptr;
    WORD storageIndex;

    /* Issue Restart device to stack which will clear all stack information */
    RestartDevice(0x00, 0x00, 0x00 );
    


    /* Get the storage index for commissioning cluster attribute set */
    if
    (
        EP_CLUSTERID_NOT_FOUND_IN_NVM !=
        ZCL_Callback_GetClusterIndex
        (
            SOURCE_ENDPOINT,
            ZCL_COMMISSIONING_CLUSTER,
            &storageIndex
        )
    )
    {
       /* get the commissioning cluster attribute set onto
          STARTUP_ATTRIBUTE_SET(stack structure with validity key),
          leaving first 2 bytes for validity key */
        ptr = (BYTE *)&commAttribute;
        ZCL_Callback_GetAttributeValue
        (
            storageIndex,
            ptr + 2,
            sizeof( Commissioning_Cluster_Attributes )
        );
    }
    
  	//Update MSDCL Commission Cluster
  	MSDCL_Commission.ValidCleanStartUp = MSDCL_COMMISSION_DATA_VALID;
    MSDCL_Commission.StartupStatus = commAttribute.spas.StartupControl;
    MSDCL_Commission.ChannelMask.Val = commAttribute.spas.ChannelMask.Val;
    memcpy( &MSDCL_Commission.ExtendedPANId[0], &commAttribute.spas.ExtendedPANId.v[0], sizeof(MSDCL_Commission.ExtendedPANId) );
    MSDCL_Commission.LinkKey[0]= commAttribute.spas.PreconfiguredLinkKey[0];
    MSDCL_Commission.LinkKey[1]= commAttribute.spas.PreconfiguredLinkKey[1];
    MSDCL_Commission.LinkKey[2]= commAttribute.spas.PreconfiguredLinkKey[2];
    MSDCL_Commission.LinkKey[3]= commAttribute.spas.PreconfiguredLinkKey[3];
    MSDCL_Commission.LinkKey[4]= commAttribute.spas.PreconfiguredLinkKey[4];
    MSDCL_Commission.LinkKey[5]= commAttribute.spas.PreconfiguredLinkKey[5];
    MSDCL_Commission.LinkKey[6]= commAttribute.spas.PreconfiguredLinkKey[6];
    MSDCL_Commission.LinkKey[7]= commAttribute.spas.PreconfiguredLinkKey[7];
    MSDCL_Commission.LinkKey[8]= commAttribute.spas.PreconfiguredLinkKey[8];
    MSDCL_Commission.LinkKey[9]= commAttribute.spas.PreconfiguredLinkKey[9];
    MSDCL_Commission.LinkKey[10]= commAttribute.spas.PreconfiguredLinkKey[10];
    MSDCL_Commission.LinkKey[11]= commAttribute.spas.PreconfiguredLinkKey[11];
    MSDCL_Commission.LinkKey[12]= commAttribute.spas.PreconfiguredLinkKey[12];
    MSDCL_Commission.LinkKey[13]= commAttribute.spas.PreconfiguredLinkKey[13];
    MSDCL_Commission.LinkKey[14]= commAttribute.spas.PreconfiguredLinkKey[14];
    MSDCL_Commission.LinkKey[15]= commAttribute.spas.PreconfiguredLinkKey[15];

    if( !commAttribute.spas.StartupControl )
    {
        DWORD mask = 0x800;
        BYTE i;

        App_Reset();
        /* Update the nwkUpdateId of the device */
        memcpy
        (
            &currentNeighborTableInfo.nwkExtendedPANID.v[0],
            &commAttribute.spas.ExtendedPANId.v[0],
            MAX_ADDR_LENGTH
        );
        PutNeighborTableInfo();

        macPIB.macPANId.Val = commAttribute.spas.PANId.Val;
        macPIB.macShortAddress.Val = commAttribute.spas.shortAddr.Val;
        /* Verify the channel from 11 to 26 */
        for(i = 11; i <= 26; i++)
        {
            if( commAttribute.spas.ChannelMask.Val & mask )
            {
                phyPIB.phyCurrentChannel = i;
                break;
            }
            mask <<= 1;
        }
    #if I_SUPPORT_LINK_KEY == 1
        if(!I_AM_TRUST_CENTER){//#ifndef I_AM_TRUST_CENTER
            TCLinkKeyInfo.trustCenterShortAddr.Val = 0x0000;
            for(i=0; i < 8; i++)
            {
                TCLinkKeyInfo.trustCenterLongAddr.v[i] = commAttribute.spas.TrustCenterAddress.v[i];
            }
        }//#endif /* I_AM_TRUST_CENTER */

        for(i=0; i < KEY_LENGTH; i++)
        {
            TCLinkKeyInfo.link_key.v[i] = commAttribute.spas.PreconfiguredLinkKey[i];
        }
        TCLinkKeyInfo.frameCounter.Val = 0x00;
    #endif

        StorePersistancePIB();
        /* Issue the start device request onto stack */
        startMode = 0;
        AllowJoin = TRUE;
		if( NOW_I_AM_A_ROUTER() )
			currentPrimitive_MTR = APP_START_DEVICE_request;
		else if( NOW_I_AM_A_CORDINATOR() )
			currentPrimitive_ESP = APP_START_DEVICE_request;

    }
    else
    {

        /* Get the startup Mode from the option field of the restart
           device request command received */
        startUpMode = ( RestartDeviceCmdPayload.Options & 0x07 );

        if( startUpMode == 0x00 )
        {

            /* Update the validity key */
            commAttribute.validitykey = SAS_TABLE_VALID;
            /* Put the Commissioning cluster attribute values into stack SAS */
            updateCurrentSAS(&commAttribute);

            /* Update the parameters which needs to be set for stack operation */
            updateCommissioningParameters();

        }
        else if( startUpMode > 0x01 )
        {
            return;
        }

        App_Reset();
        /* Issue the start device request onto stack */
        startMode = 0;
        AllowJoin = TRUE;
		if( NOW_I_AM_A_ROUTER() )
			currentPrimitive_MTR = APP_START_DEVICE_request;
		else if( NOW_I_AM_A_CORDINATOR() )
			currentPrimitive_ESP = APP_START_DEVICE_request;

    }
    NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
    Reset();
}
//#endif /*(I_AM_ESP == 0)*/

///******************************************************************************
// * Function:        void App_TimeTick ( void )
// *
// * PreCondition:    None
// *
// * Input:           None
// *
// * Output:          None
// *
// * Return :         None
// *
// * Side Effects:    None
// *
// * Overview:        This function runs in an infinite loop. It reads the RTC and
// *                  updates the Time attribute of Time cluster
// *
// * Note:            None
// *****************************************************************************/
//void App_TimeTick ( void )
//{
//    /*This holds the previous elapsed seconds*/
//    static DWORD previousElapsedSeconds = 0x00;
//    DWORD currentTime = ZCL_Callback_GetCurrentTimeInSeconds();
//    /*if the current elapsed seconds is greater than previous seconds, then
//    update the time attribute with the current elapsed seconds*/
//    if( currentTime != previousElapsedSeconds )
//    {
//        /*set the previous elapsed seconds with the current seconds*/
//        previousElapsedSeconds = currentTime;
//        /*Reads the RTC time and converts it to seconds*/
//        ZCL_Time_SetTimeAttribute( 0x01 ,
//            (BYTE*)&previousElapsedSeconds );
//    }
//}

/******************************************************************************
 * Function:        void Time_Synchronization( BYTE endpoint, BYTE* pSeconds );
 *
 * PreCondition:    This function is called when time synchronization happens
 *
 * Input:           endpoint - endpoint for which time synchronization is done
 *
 * Output:          pSeconds - seconds elapsed from January 2000 00 hours
 *
 * Side Effects:    None
 *
 * Return Value:    Success( 1 ) or failure( 0 )
 *
 * Overview:        This function sets the Time attribute.Also synchronizes the
 *                  time by setting the values in RTC
 *
 * Note:
 *****************************************************************************/
//#if (I_AM_ESP == 0)		//MSEB
void Time_Synchronization( BYTE endpoint, BYTE* pSeconds )
{
    DWORD seconds;
    /*This variable holds the time in the form of year, month, day,
    hour, minute and second*/
    elapsedTimeInfo elapsedTime;
    /*TBD Set the time attribute, this should use a common API provided by ZCL*/
    //ZCL_Time_SetTimeAttribute( endpoint, pSeconds );
    /*convert the seconds to year format and update the RTC to tick with
    this seconds.*/
    memcpy( (BYTE*)&seconds, pSeconds ,  0x04 );

    ZCL_Utility_Time_ConvertSecondsToYear( seconds, &elapsedTime );
    App_SetCurrentTime(&elapsedTime);

}
//#endif /*I_AM_ESP == 0*/

/******************************************************************************
 * Function:        void SE_SendNLMELeaveRequest(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        On calling this function Device will leave the Network.
 *
 * Note:            None
 *****************************************************************************/
void SE_SendNLMELeaveRequest(void)
{
    BYTE i;

    for( i = 0; i < 8; i++)
    {
        params.NLME_LEAVE_request.DeviceAddress.v[i] = 0x00;
    }
    params.NLME_LEAVE_request.Rejoin = FALSE;
    params.NLME_LEAVE_request.RemoveChildren = FALSE;
    params.NLME_LEAVE_request.ReuseAddress = FALSE;
    params.NLME_LEAVE_request.Silent = FALSE;
		if( NOW_I_AM_A_ROUTER() )
			currentPrimitive_MTR = NLME_LEAVE_request;
		else if( NOW_I_AM_A_CORDINATOR() )
			currentPrimitive_ESP = NLME_LEAVE_request;

    //currentPrimitive = NLME_LEAVE_request;
}

/******************************************************************************
* Function:         void FormatMeterReadingValue
*                   (
*                       BYTE *pmeterReadingValue,
*                       BYTE meterAttributeSize
*                   );
*
* PreCondition:     None
*
* Input:            pmeterReadingValue - Meter reading value to be formatted
*                   meterAttributeSize - Size of meter attribute.
*
* Output:           None
*
* Return :          None
*
* Side Effects:     None
*
* Overview:         Formats the meter reading values depending on
*                   formatting attributes.
*
* Note:             None
*****************************************************************************/
//#if (I_AM_ESP == 1)		//MSEB
static void FormatMeterReadingValue
(
    BYTE *pmeterReadingValue,
    BYTE meterAttributeSize
)
{
    unsigned long long Summation;
    DWORD displayRemainder;
    BYTE i;
    DWORD j;
    Summation = 0x0000000000000000;

    memcpy
    (
        (BYTE*)&Summation,
        pmeterReadingValue,
        meterAttributeSize
    );
    if( ( meterFormatAttributes.multiplier != 0x00000000 )&&( meterFormatAttributes.divisor != 0x00000000 ))
    {
        displayQuotient = ( ( Summation ) * ( meterFormatAttributes.multiplier ) )/ ( meterFormatAttributes.divisor );
        displayRemainder = ( ( Summation ) * ( meterFormatAttributes.multiplier ) )%( meterFormatAttributes.divisor );
        j = meterFormatAttributes.divisor;
    }
    else
    {

        j=1;
        for(i = 0x00;i<(meterFormatAttributes.summationFormatting & 0x07 );i++)
        {
                j=j*10;
        }
        displayQuotient = Summation/j;
        displayRemainder = Summation%j;
    }
    for(i=0x00;i<10;i++)
    {
        remainderArray[i] = (displayRemainder*10)/j;
        displayRemainder = (displayRemainder*10)-(j*remainderArray[i]) ;

    }
}

/******************************************************************************
* Function:        void DisplayForMeterFormattingAttributes
*                  (DWORD displayQuotient, BYTE* pRemainderArray,BYTE displayState);
*
* PreCondition:    None
*
* Input:          None
*
* Output:          None
*
* Return :
*
* Side Effects:    None
*
* Overview:        None
* Note:            None
*****************************************************************************/
void DisplayForMeterFormattingAttributes
(
    DWORD displayQuotient,
    BYTE* pRemainderArray,
    BYTE display
)
{
    BYTE val;
    BYTE i;
    BYTE pos = 8;
    BYTE numberOfDigitsBeforeDecimalPoint;
    BYTE numberOfDigitsAfterDecimalPoint;

    lcdClear();
    lcdPutCur (0, 0);
    switch(display)
    {
        case 0x11:
            lcdPutStr("CurSumDel");
            break;
        case 0x12 :
            lcdPutStr("CurSumRec");
            break;
        case 0x13 :
            lcdPutStr("CurMaxDemDel");
            break;
        case 0x14 :
            lcdPutStr("CurMaxDemRec");
            break;
        case 0x15 :
            lcdPutStr("DFTSum");
            break;
        case 0x16 :
            lcdPutStr("CurT1SumDel");
            break;
        case 0x17 :
            lcdPutStr("CurT1SumRec");
            break;
        case 0x18 :
            lcdPutStr("CurT2SumDel");
            break;
        case 0x19 :
            lcdPutStr("CurT2SumRec");
            break;
        case 0x1A :
            lcdPutStr("CurT3SumDel");
            break;
        case 0x1b:
            lcdPutStr("CurT3SumRec");
            break;
        case 0x1c :
            lcdPutStr("CurT4umDel");
            break;
        case 0x1d :
            lcdPutStr("CurT4SumRec");
            break;
        case 0x1e :
            lcdPutStr("CurT5SumDel");
            break;
        case 0x1f :
            lcdPutStr("CurT5SumRec");
            break;
        case 0x20 :
            lcdPutStr("CurT6SumDel");
            break;
        case 0x21 :
            lcdPutStr("CurT6SumRec");
            break;
        case 0x22 :
            lcdPutStr("InstDemand");
            break;
        case 0x23 :
            lcdPutStr("CurDayConsDel");
            break;
        case 0x24 :
            lcdPutStr("CurDayConsRec");
            break;
        case 0x25 :
            lcdPutStr("PreDayConsDel");
            break;
        case 0x26 :
            lcdPutStr("PreDayConsRec");
            break;
        case 0x27 :
            lcdPutStr("CurParProDel");
            break;
        case 0x28 :
            lcdPutStr("CurParProRec");
            break;
        case 0x29 :
            lcdPutStr("CurDemDel");
            break;

        default :
        break;

    }
    numberOfDigitsAfterDecimalPoint = (meterFormatAttributes.summationFormatting & 0x07 );
    if(numberOfDigitsAfterDecimalPoint)
    {
        lcdPutCur (1, pos);
        lcdPutChar('.');
        pos = 0x09;
        for( i = 0x00; i < numberOfDigitsAfterDecimalPoint; i++ )
        {
            lcdPutCur (1, pos+i);
            lcdPutChar(pRemainderArray[i] + 0x30);
        }
    }
    numberOfDigitsBeforeDecimalPoint = (meterFormatAttributes.summationFormatting & 0x78 );
    numberOfDigitsBeforeDecimalPoint = numberOfDigitsBeforeDecimalPoint>>3;
    for( i = 0; i < numberOfDigitsBeforeDecimalPoint; i++ )
    {
        if( meterFormatAttributes.summationFormatting & 0x80 )
        {
            if(!displayQuotient)
            {
                break;
            }
        }
        val = displayQuotient % 10;
        displayQuotient /= 10;

        pos--;
        lcdPutCur (1, pos);
        lcdPutChar(val + 0x30);
    }


}

/******************************************************************************
* Function:        BYTE GetDisplayStateForMeterAttributes(WORD attributeId);
*
* PreCondition:    None
*
* Input:          None
*
* Output:          None
*
* Return :
*
* Side Effects:    None
*
* Overview:        None
* Note:            None
*****************************************************************************/
BYTE GetDisplayStateForMeterAttributes( WORD attributeId )
{
   BYTE i;

   for( i=0x00;i<METER_ATTRIBUTE_DISPLAY_TABLE_SIZE;i++)
   {
       if( meterAttributes_DisplayStateTable[i].attributeId == attributeId )
       {
            return meterAttributes_DisplayStateTable[i].displayState;
       }
   }
   return 0xFF;
}
//#endif /*(I_AM_ESP == 1)*/

/******************************************************************************
* Function:        void HandleLeaveConfirmAndIndication
*                   (
*                       BYTE* pDeviceAddress
*                   )
*
* PreCondition:
*
* Input:           pDeviceAddress - the address of the device that left the network
*
* Output:          None
*
* Side Effects:    None
*
* Return Value:    None
*
* Overview:        This function updates the link key table based on the device address
*
* Note:
*****************************************************************************/
#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1		//MSEB
void HandleLeaveConfirmAndIndication
(
    BYTE* pDeviceAddress
)
{
    BYTE i;
    BYTE value = 0x00;
    /*loop the device address to check if it is NULL*/
    for( i = 0x00; i < MAX_ADDR_LENGTH; i++ )
    {
        value |= pDeviceAddress[i];
    }
    /*if the device address is not NULL, then remote device has left*/
    if( value )
    {
        RemoveEntryTCLinkKeyTable( pDeviceAddress );
    }
    /*the device address is NULL*/
    else
    {
        /*if the device address is NULL, then reset the TC link key table*/
        ResetTCLinkKeyTable();
    }
}
#endif /*#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1*/
/************************************************************************************
  Function:
         void ZCL_Callback_Put_IdentifyTime( WORD identifyTime )

    Description:
        This function is used for Updatining the Identify Time attribute value.
    Precondition:
        None
    Parameters:
        identifyTime - It holds the identifyTime value

  Return:
    None.
  Example:
    <code>
    </code>
  Remarks:
    None.
  ************************************************************************************/
void ZCL_Callback_Put_IdentifyTime( WORD identifyTime )
{
    identifyTimeoutVal = identifyTime;
}

/************************************************************************************
  Function:
         WORD ZCL_Callback_Get_IdentifyTime(void)

    Description:
        This function is used for getting the Identify Time attribute value.
    Precondition:
        None
    Parameters:
        None

  Return:
    It returns the Identify Time attribute Value.
  Example:
    <code>
    </code>
  Remarks:
    None.
  ************************************************************************************/
WORD ZCL_Callback_Get_IdentifyTime( void )
{
    return identifyTimeoutVal;
}
/******************************************************************************
 * Function:        void App_SendDataOverInterPAN( void )
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
 * Overview:        This function is called when data needs to be
 *                  transmitted over the Inter-PAN network. This function
 *                  updates the inter-PAN structure and sends the data.
 *
 * Note:            None
 *****************************************************************************/
void App_SendDataOverInterPAN( void )
{
    /*SrcAddressMode should always be Extended Addressing for Inter-PAN data*/
    params.INTRP_DATA_req.SrcAddrMode = APS_ADDRESS_64_BIT;

    /*We always copy the address as Eight bytes. DstAddressMode will specify
    whether ShortAddress or ExtendedAddress should be used for data transmission */
    params.INTRP_DATA_req.DstAddrMode = interPANDataRequest.destAddrMode;
    memcpy
    (
        ( BYTE *)&(params.INTRP_DATA_req.DstAddress.v[0]),
        ( BYTE *)&(interPANDataRequest.destAddr[0]),
        MAX_ADDR_LENGTH
    );
    params.INTRP_DATA_req.DstPANId.Val = interPANDataRequest.destInterPANId;
    params.INTRP_DATA_req.ProfileId.Val = interPANDataRequest.profileId;
    params.INTRP_DATA_req.ClusterId.Val = interPANDataRequest.clusterID;
    params.INTRP_DATA_req.ASDULength = interPANDataRequest.asduLength;
    params.INTRP_DATA_req.ASDU = interPANDataRequest.pAsdu;
    params.INTRP_DATA_req.ASDUHandle = interPANDataRequest.asduHandle;

    /*Update the currentPrimitive to INTRP_DATA_request. This is handled by
    Stub layer which will transmit the Inter-PAN message.*/
		if( NOW_I_AM_A_ROUTER() )
			currentPrimitive_MTR = INTRP_DATA_request;
		else if( NOW_I_AM_A_CORDINATOR() )
			currentPrimitive_ESP = INTRP_DATA_request;
    //currentPrimitive = INTRP_DATA_request;
}

//#if (I_AM_MTR == 1) || (I_AM_IPD == 1) || (I_AM_SAP == 1) || (I_AM_RED == 1) || (I_AM_PCT == 1) || (I_AM_LCD == 1)		//MSEB
/******************************************************************************
 * Function:        void Handle_Fragmented_MessageCluster(BYTE *data)
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
 * Overview:        This function Display message is received From fragmentation.
 *
 * Note:            None
 *****************************************************************************/
void Handle_Fragmented_MessageCluster(BYTE *data)
{
    if( data[2] == ZCL_Message_DisplayMessageCmd )
    {
        BYTE msgLength;

        ZCL_Message_DisplayMessageCmdPayload *pDisplayMessge =
       (ZCL_Message_DisplayMessageCmdPayload*)&data[3];

        /*When Display message is received, store the destination address
        and destination endpoint to send Message confirmation*/
        displayMessagesTable.destaddress = params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val;
        displayMessagesTable.destEndPoint = params.APSDE_DATA_indication.SrcEndpoint;

        displayMessagesTable.txOptions = params.APSDE_DATA_indication.SecurityStatus;

        /*store the sequence number*/
        displayMessagesTable.sequenceNumber = data[1];

        /*Check if Message confirmation required and update the table*/
        if( pDisplayMessge->messageControl & MESSAGE_CONFIRMATION_REQUIRED)
        {
            displayMessagesTable.confirmationRequired = TRUE;
        }
        memcpy
        (
            (BYTE*)&( displayMessagesTable.displayMessage ),
            /*Actual message start from here*/
            &( data[ZCL_HEADER_LENGTH_WO_MANUF_CODE] ),
            11
        );
        if( !( displayMessagesTable.displayMessage.startTime ) )
        {
            displayMessagesTable.displayMessage.startTime =
                ZCL_Callback_GetCurrentTimeInSeconds();
        }
        msgLength = data[ZCL_HEADER_LENGTH_WO_MANUF_CODE + 11];

        memcpy
        (
            (BYTE*)( displayMessagesTable.displayMessage.aDisplayMessage ),
            /*Actual message start from here*/
            &( data[ZCL_HEADER_LENGTH_WO_MANUF_CODE + 11] ),
            msgLength + 1

        );
        #if ( I_AM_IPD == 1 )
            displayState = receivedMessage;
        #endif /* ( I_AM_IPD == 1 ) */
    }
}
//#endif

/******************************************************************************
 * Function:        BYTE App_CheckIfReportReceived( void  )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 *
 * Output:
 * Return :         TRUE - if all the report attributes are received
 *                  FALSE - even if one report attribute is not received
 *
 * Side Effects:    None
 *
 * Overview:        This function checks if the report attribute is received.
 *****************************************************************************/
#if I_SUPPORT_REPORTING == 1
BYTE App_CheckIfReportReceived( void  )
{
    BYTE i;
    BYTE status = TRUE;
    GetReportCfgTable(reportConfigurationTable);
    for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
    {
      /*check if report attribute command is received and timeout value is not
      an invalid value*/
      if( ( TRUE != reportConfigurationTable[i].receivedReportAttribute ) &&
      ( 0xFFFF != reportConfigurationTable[i].timeout ) )
       {
            /*if even for one attribute is not received, return TRUE*/
            status = FALSE;
       }
    }
    return status;
}


/******************************************************************************
 * Function:        void App_ResetReportAttributeReceived( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 *
 * Output:
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function resets the flag that is maintained for
 *                  received reports.
 *****************************************************************************/
void App_ResetReportAttributeReceived( void )
{
    BYTE i;
    /*Get the report configuration table from NVM*/
    GetReportCfgTable(reportConfigurationTable);
    for( i = 0x00 ; i < REPORTING_TABLE_SIZE; i++ )
    {
      /*check if report attribute command was received, if so reset the
      received attribute*/
      if( ( TRUE == reportConfigurationTable[i].receivedReportAttribute ) &&
      ( 0xFFFF != reportConfigurationTable[i].timeout ) )
       {
            /*if even for one attribute is not received, return TRUE*/
            reportConfigurationTable[i].receivedReportAttribute = FALSE;
       }
    }
    PutReportCfgTable(reportConfigurationTable);
}
#endif


/******************************************************************************
 * Function:        void CBKE_Callback_Handle_Status(BYTE status)
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
void CBKE_Callback_Handle_Status(BYTE status)
{
    if( NOW_I_AM_A_CORDINATOR() ){ //#if (I_AM_ESP == 0)		//MSEB
        App_Handle_CBKE_Status(status);
    }//#endif
}
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
BYTE CBKE_Callback_CheckForTCLinkKeyTableEntry(BYTE *DeviceIEEEAddress)
{
    BYTE    index;
    TC_LINK_KEY_TABLE currentTCLinkKey;
    WORD pCurrentEntry;
    
    pCurrentEntry = TCLinkKeyTable;
    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry+=(WORD)sizeof(TC_LINK_KEY_TABLE), index++)
    #else
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry++, index++)
    #endif
    {
        // Read the record into RAM.
        GetTCLinkKey(&currentTCLinkKey, pCurrentEntry );
        if ( currentTCLinkKey.InUse == TRUE )
        {
            if( !memcmp( currentTCLinkKey.DeviceAddress.v, DeviceIEEEAddress, 8 ) )
            {
                return TRUE;
            }
        }
        else
        {
            return TRUE;
        }
    }
    return FALSE;
}        
#endif

/***************************************************************************************
 * Function:        void DisplayGroupsClusterCommands(BYTE *pReceivedAsdu)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the Groups Cluster commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if defined(I_SUPPORT_GROUP_ADDRESSING)
void DisplayGroupsClusterCommands
(
    BYTE *pReceivedAsdu
)
{
    if ( !( *pReceivedAsdu & ZCL_SERVER_TO_CLIENT_DIRECTION ) )
    {
        switch (*( pReceivedAsdu +
                  ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ))
        {
            case ZCL_Groups_AddGroupCmd:
            {
                BYTE* pReceivedCmdPayload = ( pReceivedAsdu + ZCL_GROUPS_CMD_PAYLOAD_POSITION );
                
                displayState = receivedAddGroupCmd;
                memcpy
                (
                    (BYTE *) &displayGroups.groupID.v[0],
                    pReceivedCmdPayload,
                    2
                );
                memcpy
                (
                    (BYTE *) &displayGroups.groupName[0],
                    pReceivedCmdPayload + 2,
                    *(pReceivedCmdPayload + 2) + 1
                );
            }
                break;
            case ZCL_Groups_RemoveGroupCmd:
            {
                BYTE* pReceivedCmdPayload = ( pReceivedAsdu + ZCL_GROUPS_CMD_PAYLOAD_POSITION );
                
                displayState = receivedRemoveGroupCmd;
                memcpy
                (
                    (BYTE *) &displayGroups.groupID.v[0],
                    pReceivedCmdPayload,
                    2
                );
            }
                break;
        }
    }
    else
    {
        switch (*( pReceivedAsdu +
                  ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ))
        {
            case ZCL_Groups_ViewGroupResponseCmd:
            {
                BYTE* pReceivedCmdPayload = ( pReceivedAsdu + ZCL_GROUPS_CMD_PAYLOAD_POSITION );
                
                displayState = receivedViewGroupRspCmd;
                
                displayGroups.status = *pReceivedCmdPayload;
                pReceivedCmdPayload = ( pReceivedAsdu + ZCL_GROUPS_CMD_PAYLOAD_POSITION + 1);
                memcpy
                (
                    (BYTE *) &displayGroups.groupID.v[0],
                    pReceivedCmdPayload,
                    2
                );
                memcpy
                (
                    (BYTE *) &displayGroups.groupName[0],
                    pReceivedCmdPayload + 2,
                    *(pReceivedCmdPayload + 2) + 1
                );
            }
                break;
        }
    }
}
#endif

/***************************************************************************************
 * Function:        void DisplayScenesClusterCommands(BYTE *pReceivedAsdu)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Display the Scenes Cluster commands
 *
 * Note:            None
 *
 ***************************************************************************************/
#if (I_SUPPORT_SCENES == 1)
void DisplayScenesClusterCommands
(
    BYTE *pReceivedAsdu
)
{
    if ( !( *pReceivedAsdu & ZCL_SERVER_TO_CLIENT_DIRECTION ) )
    {
        switch (*( pReceivedAsdu +
                  ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ))
        {
            case ZCL_Scenes_AddSceneCmd:
            {
                //BYTE* pReceivedCmdPayload = ( pReceivedAsdu + ZCL_SCENES_CMD_PAYLOAD_POSITION );
                
                displayState = receivedAddSceneCmd;
                
            }
                break;
            case ZCL_Scenes_RemoveSceneCmd:
            {
                //BYTE* pReceivedCmdPayload = ( pReceivedAsdu + ZCL_SCENES_CMD_PAYLOAD_POSITION );
                
                displayState = receivedRemoveSceneCmd;
            }
                break;
        }
    }
    else
    {
        switch (*( pReceivedAsdu +
                  ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ))
        {
            case ZCL_Scenes_ViewSceneResponseCmd:
            {
                //BYTE* pReceivedCmdPayload = ( pReceivedAsdu + ZCL_SCENES_CMD_PAYLOAD_POSITION );
                
                displayState = receivedViewSceneRspCmd;
                
            }
                break;
        }
    }
}
#endif

/******************************************************************************
 *	Function:   	BYTE ZCL_Callback_HandleGroupCmds
 *					(
 *    					BYTE endPointId,
 *    					BYTE* pReceivedAsdu,
 *   					BYTE* pResponsePayloadStartLocation
 *					)
 *
 * PreCondition:    None
 *
 * Input:           endPointId  - EndPoint on which the Groups Cluster command needs
 *                                 to be processed.
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and Payload.
 *                  pZCLClusterInfo - Pointer pointing to the Groups Cluster info
 *
 * Output:          pResponseAsduStartLocation - Pointer pointing to the start
 *                                               location of asdu payload for which the
 *                                               response payload needs to be created
 * Return :         Returns the length of the payload of response frame created
 * Side Effects:    None
 *
 * Overview:        This function is called when a Groups Cluster command
 *                  is received.
 *                  This function process the Groups Cluster commands and 
 *                  builds the rseponse payload if required .
 *****************************************************************************/

#if defined(I_SUPPORT_GROUP_ADDRESSING)

BYTE ZCL_Callback_HandleGroupCmds
(
    BYTE endPointId,
    BYTE* pReceivedAsdu,
    BYTE* pResponsePayloadStartLocation
)
{
	BYTE responsePayloadLangth = 0x00;
	BYTE status;
	BYTE groupIndex;
	App_GroupNameRecord appGroupNameRecord;
	SHORT_ADDR groupAddr;

	/* Loop variable */
	BYTE i;

	BYTE* pReceivedCmdPayload = ( pReceivedAsdu + ZCL_GROUPS_CMD_PAYLOAD_POSITION );

	/* Set the group table variable with all FF's */
	memset
	( 
		&appGroupNameRecord, 
		0xFF, 
		sizeof(App_GroupNameRecord)
	);

	/* Copy the group id received in the group command */
	groupAddr.byte.LSB = *( pReceivedCmdPayload );
	groupAddr.byte.MSB = *( pReceivedCmdPayload + 1 );

	/* Check if the group entry is exist for the endpoint which has received the group command */
	status = VerifyGroupAddrAndEndPoint	
			(
				groupAddr,
				endPointId, 
				&groupIndex
			);

    if ( *pReceivedAsdu & ZCL_SERVER_TO_CLIENT_DIRECTION )
    {
        DisplayGroupsClusterCommands
        (
            pReceivedAsdu
        );
    }
    else
    {
        switch ( *( pReceivedAsdu +
                  ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ))
        {
            case ZCL_Groups_AddGroupCmd:
            {
				if ( GROUP_ENTRY_FOUND == status )
				{
					status = ZCL_DuplicateExists;
				}
				else
				{
					status = AddGroup 
	           				(
								groupAddr,
								endPointId	
							);	
	
					/* If group is added successfully, update the entry in the AppGroupsNameTable */
					if ( GROUP_SUCCESS == status )
					{
						App_AddGroup 
						(
							groupAddr,
						 	endPointId,
							pReceivedCmdPayload
					 	);			
					 	
					 	DisplayGroupsClusterCommands
					 	(
					 	    pReceivedAsdu
					 	);
				    }
					else
					{
						status = ZCL_InsufficientSpace;
					}
				}

				/* Create the response payload */
				/* Copy the status */
				*pResponsePayloadStartLocation = status;
				
				/* Copy the group id */
				memcpy
				(
					pResponsePayloadStartLocation + 1,
					(BYTE*)( pReceivedCmdPayload ),
					SIZE_OF_GROUP_ID
				);

				/* Update the total response payload length */
				responsePayloadLangth = SIZE_OF_STATUS + SIZE_OF_GROUP_ID;
            }
            break;

            case ZCL_Groups_ViewGroupCmd:
            {
				/* Check if the group entry exists or not */
				if ( GROUP_ENTRY_NOT_FOUND == status  )
				{
					/* Put the status as Not Found */
					status = ZCL_NotFound;					

					/* Copy the null string */
					*(pResponsePayloadStartLocation + SIZE_OF_STATUS + SIZE_OF_GROUP_ID ) = 0x00;

					/* Update the total response payload length */
					responsePayloadLangth = SIZE_OF_STATUS + SIZE_OF_GROUP_ID + 1;
				}
				else
				{
					/* Put the status as success */
					status = ZCL_Success;
 
					/* Get the AppGroupsNameTable record for the particular group id */
					GetAppGroupNameAddress 
					(
						&appGroupNameRecord,
						AppGroupsNameTable + (WORD)(groupIndex) * (WORD)sizeof(App_GroupNameRecord)	
					);
					
					/*This condition is required when APP_USE_EXTERNAL_NVM is disbled and
					Group Table entries are maintained in NVM by the Stack. When Power failure 
					happens, the Group Name for each of the Group Address stored by the Application in 
					RAM will be erased, but the Group Address will not be erased, as it is stored in NVM 
					by the Stack. When View Group command is issued to during this case, we will put the 
					null string because we have lost the Group Names.*/
					if ( appGroupNameRecord.groupName[0] == 0xFF )
					{
						appGroupNameRecord.groupName[0] = 0;
					}
					
					/* Copy the Group Name. Total bytes to be copied are string 
				       lengh plus one byte which hold the lengh of the string */
					memcpy
					(
						(pResponsePayloadStartLocation + SIZE_OF_STATUS + SIZE_OF_GROUP_ID),
						appGroupNameRecord.groupName,
						appGroupNameRecord.groupName[0]+1  
					);

					/* Update the total response payload length */
					responsePayloadLangth = SIZE_OF_STATUS + SIZE_OF_GROUP_ID + appGroupNameRecord.groupName[0]+1;
				}

				/* Copy the status */
				*pResponsePayloadStartLocation = status;
				
				/* Copy the group id */
				memcpy
				(
					pResponsePayloadStartLocation + SIZE_OF_STATUS,
					(BYTE*)( pReceivedCmdPayload ),
					SIZE_OF_GROUP_ID
				);
            }
            break;

            case ZCL_Groups_GetGroupMembershipCmd:
            {	
				/* Initialize the remaining capacity and response group count to null */
	            BYTE remaingCapacity = 0x00;
				BYTE respGroupCount = 0x00;
			
				/* Get the group count from the received command */
				BYTE groupCount = *( pReceivedCmdPayload );

				/* Initialize the pointer to the starting location of the group list in the 
                   received command */
				BYTE* pgroupList = ( pReceivedCmdPayload + 1 );

				/* Initialize the pointer to the starting location where the group list has to be 
	               copied the response payload */
				BYTE* prespGroupList = ( pResponsePayloadStartLocation + 2 );

				/* Check if the group count is null. If so, Put all the groups present in 
				   the group table in the response payload */
				if (0x00 == groupCount )
				{
					for ( i = 0; i < MAX_GROUP ; i++ )	
					{
						groupAddr.Val = GetGroupByIndex ( i );

						/* Check if this is a free entry or not. If this is a free entry, update the 
						   remaining capacity variable */
						if ( NO_GROUP_ID ==  groupAddr.Val )
						{
							remaingCapacity++;
						}
						else
						{
							/* Copy the group id */
							memcpy
							(
								prespGroupList,
								(BYTE*)&(groupAddr.Val),
								SIZE_OF_GROUP_ID
							);
							/* Update the group count which ahs to be send in the response payload */
							respGroupCount++;

							/* Update response payload lenght */
							responsePayloadLangth += SIZE_OF_GROUP_ID;

							/* Increment the pointer to response group list */
							prespGroupList += SIZE_OF_GROUP_ID;
						}						
					}
				}
				/* Loop through all group entries and update the response payload with group id's 
				   which are matched with the requested group list */
				else
				{
					for ( i = 0; i < groupCount ; i++ )
					{
						/* Copy the group id received in the list */
						groupAddr.byte.LSB = *pgroupList;
						groupAddr.byte.MSB = *(pgroupList + 1);

						pgroupList += SIZE_OF_GROUP_ID;
							
						/* Check if the requested group entry is found */	
						if( GROUP_ENTRY_FOUND  == VerifyGroupAddrAndEndPoint	
												  (
													groupAddr,
													endPointId, 
													&groupIndex
												  ))
						{
							/* Update the response payload with the found group id */
							*prespGroupList = groupAddr.byte.LSB;
							*(prespGroupList + 1) = groupAddr.byte.MSB;
							
							/* Update the group count, payload length */
							respGroupCount++;

							prespGroupList += SIZE_OF_GROUP_ID;

							responsePayloadLangth += SIZE_OF_GROUP_ID;							
						}				
					}

					/* Calculate the remaining capacity in the group table */
					for ( i = 0; i < MAX_GROUP ; i++ )
					{
						if ( NO_GROUP_ID ==  GetGroupByIndex ( i ) )	
						{										
							remaingCapacity++;							
						}
					}
				}
				
				/* Send the response only if something is matched with the group list 
				   i.e. respGroupCount has some value*/
				if ( ( respGroupCount ) || ( (0x00) == *( pReceivedCmdPayload ) ) )
				{
					/* Copy the remaining capacity */
					*pResponsePayloadStartLocation++ = remaingCapacity;
					
					/* Copy the group list count */
					*pResponsePayloadStartLocation = respGroupCount;
	
	
					/* Update the total response payload length */
					responsePayloadLangth += sizeof ( respGroupCount ) + sizeof ( remaingCapacity );
				}
			}           
            break;

            case ZCL_Groups_RemoveGroupCmd:
            {	
				/* Check if the group entry exists or not */
				if ( GROUP_ENTRY_NOT_FOUND == status  )
				{
					status = ZCL_NotFound;
				}
				else
				{
					status = RemoveGroup 
	           				(
								groupAddr.Val,
								endPointId	
							);	
							
				    DisplayGroupsClusterCommands
				 	(
				 	    pReceivedAsdu
				 	);

				#if (I_SUPPORT_SCENES == 1)
					/* Delete the scene table entries for this group id, if any */
					App_DeleteScenesTableEntry( endPointId, groupAddr.Val);
				#endif /*I_SUPPORT_SCENES*/

					/* delete the entry in the AppGroupsNameTable */					
					PutAppGroupNameAddress
					(	
						AppGroupsNameTable + (WORD)(groupIndex) * (WORD)sizeof(App_GroupNameRecord), 
						&appGroupNameRecord
					);
	
				}
				/* Create the response payload */
				/* Copy the status */
				*pResponsePayloadStartLocation = status;
				
				/* Copy the group id in the response payload */
				memcpy
				(
					pResponsePayloadStartLocation + 1,
					(BYTE*)( pReceivedCmdPayload ),
					SIZE_OF_GROUP_ID
				);
	
				/* Update the total response payload length */
				responsePayloadLangth = SIZE_OF_STATUS + SIZE_OF_GROUP_ID;

            }
            break;

            case ZCL_Groups_RemoveAllGroupsCmd:
            {
	            
	            #if (I_SUPPORT_SCENES == 1)
					for(i = 0; i < MAX_GROUP; i++)
				    {
						/* Get the group adress from the aps group table. If it is 0xFFFF, 
						   then delete the entry in the AppGroupsNameTable also */
						groupAddr.Val = GetGroupByIndex ( i );				
							
								/* Delete the scene table entries for this group id, if any */
								App_DeleteScenesTableEntry( endPointId, groupAddr.Val );					
				    }
				#endif /*I_SUPPORT_SCENES*/
				
				/* Remove all the groups from the group table for this endpoint */
				RemoveGroup
				(
					NO_GROUP_ID,
					endPointId
				);
			
			    for(i = 0; i < MAX_GROUP; i++)
			    {
					/* Get the group adress from the aps group table. If it is 0xFFFF, 
					   then delete the entry in the AppGroupsNameTable also */
					groupAddr.Val = GetGroupByIndex ( i );

					if ( NO_GROUP_ID == groupAddr.Val)
					{
						/* Delete the AppGroupsNameTable entry from the table ( fill it with all FF's)*/								
						PutAppGroupNameAddress
						(	
							AppGroupsNameTable + (WORD)(i) * (WORD)sizeof(App_GroupNameRecord), 
							&appGroupNameRecord
						);
					}
			    }

				/* Copy the status */
				*pResponsePayloadStartLocation = ZCL_Success;				
            }
            break;

            case ZCL_Groups_AddGroupIfIdentifyingCmd:
            {
				/* Check if the device is deintifying itself */
				if ( appIdentifyTimers.IdentifyTime )
				{
					if ( GROUP_ENTRY_FOUND == status )
					{
						status = ZCL_DuplicateExists;
					}
					else
					{					
						status = AddGroup 
		           				(
									groupAddr,
									endPointId	
								);	
	
						/* If group is added successfully, update the entry in the AppGroupsNameTable */
						if ( GROUP_SUCCESS == status )
						{						
							App_AddGroup 
							(
								groupAddr, 
								endPointId, 
								pReceivedCmdPayload
							);
						}	
					}	
				}
				/* Device is not identifying itself */
				else
				{
					status = ZCL_Failure;
				}

				/* Copy the status */
				*pResponsePayloadStartLocation = status;
            }
            break;

            default :
                break;
        }
    }    
	return responsePayloadLangth;
}

/******************************************************************************
 *	Function:   	void App_AddGroup
 *					(
 *    					SHORT_ADDR groupAddr,
 *						BYTE endPointId,
 *						BYTE* pReceivedCmdPayload 
 *					)
 *
 * PreCondition:    None
 *
 * Input:      		groupAddr - The group id for which group enty has to be added.     
 *					endPointId - The endpoint for which group entry ha to be added.      
 *		    		pReceivedCmdPayload - Pointer to the payload location of the 
 *										  received command.
 *
 * Output:          None
 * Return :         None. 
 * Side Effects:    None
 *
 * Overview:        This function  will add the group entry in the applications group table
 *****************************************************************************/
void App_AddGroup 
( 
	SHORT_ADDR groupAddr,
    BYTE endPointId,
	BYTE* pReceivedCmdPayload 
)
{
	BYTE groupIndex;
	App_GroupNameRecord appGroupNameRecord;
	BYTE groupNameSupported;

	/* Get the index of the apsGroupAddressTable */
	groupIndex = GetEndPointsFromGroup ( groupAddr );

	/* Get the name support attribute value. If name is supported then copy the group name, 
	   Otherwise copy the null string */
	App_ReadWriteSingleAttribute
	 (
		endPointId,
		ZCL_GROUPS_NAME_SUPPORT_ATTRIBUTE,
		&zcl_GroupsClusterServerInfo,
		&groupNameSupported,
		ReadAttribute		
	 );

	/* Check if group names are supported. If so, copy the group name */
	if ( IS_NAMES_SUPPORTED & groupNameSupported )
	{
		/* Copy the received group name */
		memcpy
                 (
                     appGroupNameRecord.groupName,
                     ( pReceivedCmdPayload + SIZE_OF_GROUP_ID ),
                     MAX_SIZE_OF_GROUP_NAME
                 );
	}
	else
	{
		/* Put the null string */
		appGroupNameRecord.groupName[0] = 0x00;
	}

	/* Save the information in the AppGroupsNameTable*/	
	PutAppGroupNameAddress
	(	
		AppGroupsNameTable + (WORD)(groupIndex) * (WORD)sizeof(App_GroupNameRecord), 
		&appGroupNameRecord
	);
}
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

/******************************************************************************
 *	Function:   	BYTE App_ReadWriteSingleAttribute
 *					(
 *    					BYTE endPointId,
 *						WORD attributeId,
 *						ROM ZCL_ClusterInfo* pZclClusterInfo,
 * 						BYTE* pData,
 *    					BYTE direction
 *					)
 *
 * PreCondition:    None
 *
 * Input:           endPointId  - The end point for which the attribute value has to be 
 *								  read or written.
 *					attributeId  - attribute id, whose value has to be read or written.
 *					pZCLClusterInfo - Pointer pointing to the Cluster info.
 *					direction - Direction which tells ehether attribute value ha s to be read or written.
 *
 * Output:          psceneTableIndex - This point to the index of the scene table entry 
 *                                     whch is matched for group id and scene id.
 * Return :         It returns the length of the attribue id which is successfully read or written.
 * Side Effects:    None
 *
 * Overview:        This function reads or writes the attribute value from (or to) pdata 
 *					depending upon the direction field.
 *****************************************************************************/

BYTE App_ReadWriteSingleAttribute
(
	BYTE endPointId,
	WORD attributeId,
	ROM ZCL_ClusterInfo* pZclClusterInfo,
	BYTE* pData,
	BYTE direction		
)
{
	BYTE length;
	BYTE attributeStorageOffset;
	WORD storageBaseLocation;

	/* Get the cluster storage index from the the app attribute table */
    ZCL_Callback_GetClusterIndex
    (
        endPointId,
        pZclClusterInfo->clusterId,
        &storageBaseLocation
    );

	/* Get the length and offset of the attribute from the starting location */
	length = GetAttributeDetails
			 (
			   pZclClusterInfo->numberOfAttributes,
			   attributeId,
			   (ZCL_AttributeInfo *)(pZclClusterInfo->pListofAttibuteInfo),
			   &attributeStorageOffset
			 );

	/* If the direction is read, get the value of attribute */
	if ( ReadAttribute == direction )
	{
		/* Get the value of attribute */
	    ZCL_Callback_GetAttributeValue
	    (
	        storageBaseLocation + attributeStorageOffset,
	        pData,
	        length
	    );
	}
	/* If the direction is Write, put the value of attribute */
	else
	{
		/* Put the value of the attribute in the attribute table */
		ZCL_Callback_PutAttributeValue
		(
	        storageBaseLocation + attributeStorageOffset,
	        pData,
	        length
		);	
	
	}
	return length;
}


/******************************************************************************
 *	Function:   	BYTE ZCL_Callback_HandleSceneCmds
 *					(
 *    					BYTE endPointId,
 *    					BYTE* pReceivedAsdu,
 *   					BYTE* pResponsePayloadStartLocation
 *					)
 *
 * PreCondition:    None
 *
 * Input:           endPointId  - EndPoint on which the Scenes Cluster command needs
 *                                 to be processed.
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and Payload.
 *                  pZCLClusterInfo - Pointer pointing to the Scenes Cluster info
 *
 * Output:          pResponseAsduStartLocation - Pointer pointing to the start
 *                                               location of asdu payload for which the
 *                                               response payload needs to be created
 * Return :         Returns the length of the payload of response frame created
 * Side Effects:    None
 *
 * Overview:        This function is called when a Scenes Cluster command
 *                  is received with client to srever direction.
 *                  This function process the Scenes Cluster commands and 
 *                  builds the rseponse payload if required .
 *****************************************************************************/

#if (I_SUPPORT_SCENES == 1)

BYTE ZCL_Callback_HandleSceneCmds
(
    BYTE endPointId,
	BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponsePayloadStartLocation
)
{
	BYTE status;
	BYTE groupIndex;
	BYTE sceneTableIndex;
	App_ScenesRecord appSceneRecord;
	SHORT_ADDR groupAddr;
	BYTE responsePayloadLangth = 0x00;

	/* Get the pointer to the actual payload of the command received */
	BYTE* pReceivedCmdPayload = ( pReceivedAsdu + ZCL_SCENES_CMD_PAYLOAD_POSITION );

	/* Get the scene id */
	BYTE sceneId = *(pReceivedCmdPayload + 2 );

	/* Intialize the scene count with zero */
	BYTE sceneCount = 0x00;

	/*Loop variable*/
	BYTE i = 0x00;

	/* Initialize the create resp variable to true by default.If creation of response is not required 
	   for any command, then set it to false for that command */
	BYTE createResp = TRUE;

	/* Copy the group id received in the group command */
	groupAddr.byte.LSB = *( pReceivedCmdPayload );
	groupAddr.byte.MSB = *( pReceivedCmdPayload + 1 );

	/* Set the scene table variable with all FF's */
	memset
	( 
		&appSceneRecord, 
		0xFF, 
		sizeof(App_ScenesRecord) 
	);

	/* Check if the received group address is null. If so, set the status as success, 
       otherwise check in the group table */
	if ( 0x0000 == groupAddr.Val )
	{
		status = ZCL_Success;
	}
	/* Check if the group entry is exist for the endpoint which has received the scene command */
	else
	{
		status = VerifyGroupAddrAndEndPoint	
				(
					groupAddr,
					endPointId, 
					&groupIndex
				);

		/* If no group entry exists for the end point, set the status as invalid field */
		if ( GROUP_ENTRY_NOT_FOUND == status )
		{
			status = ZCL_InvalidField;
		}
	}

	/* Check if any scene entry exist for the received group id and scene id. 
	   If so, get the index in the output parameter */
	VerifySceneTableEntry 
	( 
		groupAddr.Val,
		sceneId,
		&sceneTableIndex 
	);
	
	if ( *pReceivedAsdu & ZCL_SERVER_TO_CLIENT_DIRECTION )
    {
        DisplayScenesClusterCommands
        (
            pReceivedAsdu
        );
    }
    else
    {

    	switch ( *( pReceivedAsdu +
                  ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED ) )
    	{
    		case ZCL_Scenes_AddSceneCmd:
    		{
    			/* check if the status is not invalid field */
    			if ( ZCL_InvalidField != status )
    			{
    				/* Check the scene table index. If entry exists, overwrite it */
    				if ( MAX_SCENES != sceneTableIndex )
    				{
    					/* Remove all the information in this entry 
    					  (i.e. fill the scene table entry with all FF's )*/					
    					PutSceneTableEntry
    					(	
    						App_ScenesTable + (WORD)(sceneTableIndex) * (WORD)sizeof(App_ScenesRecord), 
    						&appSceneRecord
    					);
    
    					/* Make the scene count to one. So that we will not increment the scene count 
    					   attribute for this add request */
    					sceneCount = 0x01;	
    				}
    				/* Search empty scene entry from the scenes table, if any */
    				else 
    				{
    					sceneTableIndex = Get_EmptySceneTableEntry ( );				
    					
    				}
    				
    				/* Check if valid scene entry is there. If not, update the status as insufficient space */
    				if ( MAX_SCENES == sceneTableIndex )
    				{
    					status = ZCL_InsufficientSpace;
    				}
    				else
    				{	
    					BYTE SceneNameSupported = 0x00;
    					BYTE length = 0x00;
    				
    					/* Set the status as success */
    					status = ZCL_Success;
    
    					/* Update the scene table variable with the information received from add scene command */
    					/* Copy the group id */
    					memcpy
    					(
    						(BYTE*)&(appSceneRecord.groupId),
    						( pReceivedCmdPayload ),
    						SIZE_OF_GROUP_ID
    					);		
    					/* Increment the received command payload pointer also */	
    					pReceivedCmdPayload += SIZE_OF_GROUP_ID;
    
    					/* copy the scene id */
    					appSceneRecord.sceneId = *( pReceivedCmdPayload );
    					pReceivedCmdPayload++;
    					
    					/* Copy the scene transition time */
    					memcpy
    					(
    						(BYTE*)&(appSceneRecord.sceneTransitionTime),
    						( pReceivedCmdPayload ),
    						2
    					);
    					pReceivedCmdPayload += LENGTH_OF_TNS_TIME;
    
    					/* Get the name support attribute value. If name is supported then copy the scene name, 
    					   Otherwise copy the null string */
    					App_ReadWriteSingleAttribute
    					 (
    						endPointId,
    						ZCL_SCENES_NAME_SUPPORT_ATTRIBUTE,
    						&zcl_ScenesClusterServerInfo,
    						&SceneNameSupported,
    						ReadAttribute		
    					 );
    
    					if ( IS_NAMES_SUPPORTED & SceneNameSupported )
    					{
    						/* Copy the Group Name. Total bytes to be copied are string 
    				           lengh plus one byte which hold the lengh of the string */
    						memcpy
    						(
    							(appSceneRecord.sceneName),
    							( pReceivedCmdPayload ),
    							( *pReceivedCmdPayload + 1 )
    						);
    					}
    					else
    					{
    						/* Copy the null string if scene names are not supported */
    						appSceneRecord.sceneName[0] = 0x00;
    					}
    
    					/* Decrement the asdu length also */
    					asduLength = asduLength -
    								  ( SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID + 
    									LENGTH_OF_TNS_TIME + ( *pReceivedCmdPayload + 1 ) );
    
    					/* Increment the received command payload by scene name lengh plus byte
    					   to hald the scene name length */
    					pReceivedCmdPayload += ( *pReceivedCmdPayload + 1 );
    
    					/* Copy the extension field sets */
    					while ( asduLength > 0x00 )
    					{
    						/* Copy the cluster id */	
    						memcpy
    						(
    							(BYTE*)&(appSceneRecord.extFieldRecord[i].clusterId),
    							( pReceivedCmdPayload ),
    							SIZE_OF_CLUSTER_ID
    						);
    						pReceivedCmdPayload += 2;
    						
    						/* Get the extn field length */
    						length = *pReceivedCmdPayload;
    	
    						/* Copy the extn field length */					
    						appSceneRecord.extFieldRecord[i].attributesLength = length;
    						pReceivedCmdPayload++;
    						
    						/* Copy the attribute data */
    						{
    							memcpy
    							(
    								( appSceneRecord.extFieldRecord[i].attributeData ),
    								( pReceivedCmdPayload ),
    								length
    							);							
    						}
    						/* Increment the received command payload pointer also */
    						pReceivedCmdPayload += length;	
    						
    						i++;
    
    						/* Decrement the received asdu length */
    						asduLength = asduLength - ( SIZE_OF_CLUSTER_ID + length + 1 );
    					}	
    					
    					/* Save the information in the AppSceneTable */	
    					PutSceneTableEntry
    					(	
    						App_ScenesTable + (WORD)(sceneTableIndex) * (WORD)sizeof(App_ScenesRecord), 
    						&appSceneRecord
    					);	
    					
    					DisplayScenesClusterCommands
                        (
                            pReceivedAsdu
                        );
    
    					/* Update the scene count only if it is null. If not null then scene entry is 
    					   overwritten so there is no need to update the scene count */	
    					if ( !sceneCount )
    					{
    						Update_AndGetSceneCount( endPointId, Increment_SceneCount );
    					}

						/* Update the last configured by attribute also */
						Update_LastConfiguredByAttr ( endPointId );
    				}
    			}
    			
    			/* update the total response payload length */
    			responsePayloadLangth = SIZE_OF_STATUS + SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID;
    		}
    		break;
    
    		case ZCL_Scenes_ViewSceneCmd:
    		{
    			/* check if the status is not invalid field */
    			if ( ZCL_InvalidField != status )
    			{
    				/* Check if requested scene entry exists in the scene table. Entry will be 
    				   existed if scene index is not MAX_SCENES */
    				if ( MAX_SCENES == sceneTableIndex )
    				{
    					status = ZCL_NotFound;
    				}
    				else
    				{
    					BYTE* pLocal;
    
    					/* Put the status as success */
    					status = ZCL_Success;
     
    					/* Get the scene table entry from the scane table */
    					GetSceneTableEntry 
    					(
    						&appSceneRecord,
    						App_ScenesTable + (WORD)(sceneTableIndex) * (WORD)sizeof(App_ScenesRecord)	
    					);
    				
    					/* Make the local pointer to point to the start of the transition time field 
    					   in the response payload */
    					pLocal = ( pResponsePayloadStartLocation + SIZE_OF_STATUS + 
    							   SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID );
    
    					/* Copy the scene transition time */
    					memcpy
    					(
    						pLocal,
    						(BYTE*)&(appSceneRecord.sceneTransitionTime),
    						LENGTH_OF_TNS_TIME
    					);
    			
    					pLocal += LENGTH_OF_TNS_TIME;
    					
    					/* Copy the Group Name. Total bytes to be copied are string 
    			           lengh plus one byte which hold the lengh of the string */
    					memcpy
    					(
    						pLocal,
    						(appSceneRecord.sceneName),
    						( appSceneRecord.sceneName[0] + 1 )
    					);
    
    					/* Increment the pointer by scene name lengh plus byte to hald the scene name length */
    					pLocal += ( appSceneRecord.sceneName[0] + 1 );
    
    					/* Copy the extension field sets */
    					for ( i=0; i<3; i++ )
    					{
    						/* Check if any extension filed recor exist in this entry.If exists, copy it 
    						   in the response payload, otherwise come out of the loop */
    						if ( NO_CLUSTER_ID != appSceneRecord.extFieldRecord[i].clusterId )
    						{
    							/* Copy the cluster id */	
    							memcpy
    							(
    								pLocal,
    								(BYTE*)&(appSceneRecord.extFieldRecord[i].clusterId),
    								SIZE_OF_CLUSTER_ID
    							);						
    						
    							pLocal += SIZE_OF_CLUSTER_ID;
    
    							/* Copy the attributes length */			
    							*pLocal = appSceneRecord.extFieldRecord[i].attributesLength;
    							pLocal++;
    
    							/* Copy the attrubute data */
    							memcpy
    							(
    								pLocal,
    								(appSceneRecord.extFieldRecord[i].attributeData),
    								appSceneRecord.extFieldRecord[i].attributesLength
    							);	
    							
    							/* Update the pointer by the attribute length */
    							pLocal += appSceneRecord.extFieldRecord[i].attributesLength;
    
    						}
    					}
    
    					/* Get the response payload length of transition time, scene name and 
    					   extension fields */
    					responsePayloadLangth = pLocal - 
    											( pResponsePayloadStartLocation + SIZE_OF_STATUS + 
    											  SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID );
    				}
    			}	
    
    			/* Update the total response payload length */
    			responsePayloadLangth += SIZE_OF_STATUS + SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID;
    		}
    		break;
    
    		case ZCL_Scenes_RemoveSceneCmd:
    		{
    			/* check if the status is not invalid field */
    			if ( ZCL_InvalidField != status )
    			{    			
    				/* Check if requested scene entry exists in the scene table. Entry will be 
    				   existed if scene index is not MAX_SCENES */
    				if ( MAX_SCENES == sceneTableIndex )
    				{
    					status = ZCL_NotFound;
    				}
    				else
    				{
	    				/* Check if the scene is valid for the entry whch has to be removed. 
	    				   If so, make the scene invalid */
	    				if (sceneValidInfo.sceneValid && sceneValidInfo.sceneTableIndex == sceneTableIndex )
	    				{
		    					Scenes_UpdateSceneValidFields 
								( 
									sceneTableIndex,
								 	endPointId, 
								 	FALSE
								);
	    				}

    					/* Put the status as success */
    					status = ZCL_Success;
    
    					/* Decrement the scene count */
    					Update_AndGetSceneCount ( endPointId, Decrement_SceneCount );
    
    					/* delete the entry in the scenes table */					
    					PutSceneTableEntry
    					(	
    						App_ScenesTable + (WORD)(sceneTableIndex) * (WORD)sizeof(App_ScenesRecord), 
    						&appSceneRecord
    					);	
    					
    					DisplayScenesClusterCommands
                        (
                            pReceivedAsdu
                        );
    				}
    			}
    
    			/* Update the total response payload length */
    			responsePayloadLangth += SIZE_OF_STATUS + SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID;			
    		}
    		break;
    
    		case ZCL_Scenes_RemoveAllScenesCmd:
    		{    		
    			/* check if the status is not invalid field */
    			if ( ZCL_InvalidField != status )
    			{
	    			/* Make the status as invalid field. If any scene entry will found for this group id, 
	    			   then change the status to success*/
	    			status = ZCL_InvalidField;
	    			
    				/* Remove all the scene entries associated with this group */	
    			    for(i = 0; i < MAX_SCENES; i++)
    			    {
	    			    /* Check if any scene exists for the group id. Remove all the scenes associated with this group */
	    			    sceneId = Get_SceneTableEntry ( groupAddr.Val, i );	
	    			    
	    			    if ( INVALID_SCENE_ID != sceneId )
	    			    {
		    				/* Check if the scene is valid for the entry whch has to be removed. 
		    				   If so, make the scene invalid */
		    				if (sceneValidInfo.sceneValid && sceneValidInfo.sceneTableIndex == i )
		    				{
			    					Scenes_UpdateSceneValidFields 
									( 
										i,
									 	endPointId, 
									 	FALSE
									);
		    				}
		    			    /* Remove the scene table entry */
	    					PutSceneTableEntry
	    					(	
	    						App_ScenesTable + (WORD)(i) * (WORD)sizeof(App_ScenesRecord), 
	    						&appSceneRecord
	    					);
	    					
	    					/* Decrement the scene count */	
			    			Update_AndGetSceneCount 
		    				( 
		    					endPointId,
		    					Decrement_SceneCount
		    				 );
		    				 
		    				/*Set the status as success*/
		    				status = ZCL_Success;
    					}
    			    }    

    			}
    			/* Update the total response payload length */
    			responsePayloadLangth += SIZE_OF_STATUS + SIZE_OF_GROUP_ID;			
    		}
    		break;
    
    		case ZCL_Scenes_StoreSceneCmd:
    		{
    			/* check if the status is not invalid field */
    			if ( ZCL_InvalidField != status )
    			{
    				/* Set the status as success */
    				status = ZCL_Success;
    
    				/* Check if scene index has valid value */
    				if ( MAX_SCENES != sceneTableIndex )
    				{
    
    					/* Entry exists. Now modify it */
    					appSceneRecord = Handle_StoreAndRecallSceneCmd
    								  	( 
    										endPointId, 
    										sceneTableIndex,
    										StoreScene 
    									 );
    				}
    				else
    				{
    					/* Get free entry from the scene table, if any */
    					sceneTableIndex = Get_EmptySceneTableEntry();
    					
    					/* Check if scene index has valid value */
    					if ( MAX_SCENES == sceneTableIndex )
    					{
    						status = ZCL_InsufficientSpace;
    					}
    					else
    					{
    						/* Update the scene table variable with the information received 
    						   from add scene command */
    						/* Copy the group id */
    						memcpy
    						(
    							(BYTE*)&(appSceneRecord.groupId),
    							( pReceivedCmdPayload ),
    							SIZE_OF_GROUP_ID
    						);			
    						pReceivedCmdPayload += SIZE_OF_GROUP_ID;
    	
    						/* copy the scene id */
    						appSceneRecord.sceneId = *( pReceivedCmdPayload );
    
    						/* Copy the scenes transition time to null */
    						appSceneRecord.sceneTransitionTime = 0x0000;
    						
    						/* Copy the null string */
    						appSceneRecord.sceneName[0] = 0x00;

							/* Update the scenes extension fields */
							Update_sceneExtnWithCurrentAttrb 
							( 
								endPointId,
								&appSceneRecord								  
							);
    						
    						/* Increment the scene count also */
    						Update_AndGetSceneCount( endPointId, Increment_SceneCount );
    					}
    				}
    				/* Store the scene entry back in the scenes table if scene index has a valid value */
    				if ( MAX_SCENES != sceneTableIndex )
    				{					
    					PutSceneTableEntry
    					(	
    						App_ScenesTable + (WORD)(sceneTableIndex) * (WORD)sizeof(App_ScenesRecord), 
    						&appSceneRecord
    					);
    					
    					/* Update the scene valid fields i.e. set the attributes 
        				   ( scene valid, current scene and current group) and scene valid flag */
        				Scenes_UpdateSceneValidFields 
        			    ( 
        					sceneTableIndex,
        				 	endPointId,
        				 	TRUE
        				);

						/* Update the last configured by attribute also */
						Update_LastConfiguredByAttr ( endPointId );
    				}
    			}
    
    			/* Update the total response payload length */
    			responsePayloadLangth += SIZE_OF_STATUS + SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID;			
    		}
    		break;
    
    		case ZCL_Scenes_RecallSceneCmd:
    		{
    			/* Check if the status is not invalid field */
    			if ( ZCL_InvalidField != status )
    			{
    				/* Verify the scene table entry for the received scene id and group id. 
    				   If entry exists, overwrite extension fields with the current values */
    				if ( MAX_SCENES != sceneTableIndex )
    				{
        				status = ZCL_Success;
        				
    					scenesRecallCmdRecord.txnCount = 0x01;
						
						/* Make the scenes transition time flag to true.*/
						scenesRecallCmdRecord.sceneTxnTime = TRUE;

						/* Copy the endpoint id and scene table index in the recall command record. 
						   So when we call this function at timer expiry, it will get the info from 
						   the command record variable */
						scenesRecallCmdRecord.endPoint = endPointId;
						
						scenesRecallCmdRecord.sceneTableIndex = sceneTableIndex;
						
    					/* Store the scene entry in the scene table variable */
    					appSceneRecord = Handle_StoreAndRecallSceneCmd 
    									( 
    										endPointId,
    										sceneTableIndex,
    									 	RecallScene
    									);
    				
    					if (!(appSceneRecord.sceneTransitionTime))
    					{
        					/* Transition Time is set to Zero. So, we dont need to do background
        					check. All the attributes are already set to specified value in the received 
        					frame*/
						    scenesRecallCmdRecord.sceneTxnTime = FALSE;
    					}
    					else
    					{
        					/*Transition Time is valid. We need update the attributes gradually based on the 
        					transition time. So, update the timers.*/
        					
        					/* Get the time for one second timer */
    						scenesRecallCmdRecord.sceneOneSecStartTime = TickGet();
    	
    						/* Get the scenes transition time */
    						scenesRecallCmdRecord.sceneTxnTimeout = (appSceneRecord.sceneTransitionTime);
    					}
    					
    					/* Update the scene valid fields i.e. set the attributes 
    					   ( scene valid, current scene and current group) and scene valid flag */
    					Scenes_UpdateSceneValidFields ( sceneTableIndex, endPointId,TRUE );
    				}
    				else
    				{
        				status = ZCL_NotFound;
    				}
    			}
    			/* Copy the status */
		        *pResponsePayloadStartLocation = status;
    			
    			/* No response is required for this command */
    			createResp = FALSE;
    		}
    		break;
    
    		case ZCL_Scenes_GetSceneMembershipCmd:
    		{
    			BYTE remaingCapacity = 0x00;
    			BYTE respSceneCount  = 0x00;
    			/* Initialize the pointer to the starting location where the scene list has to 
    			   be copied the response payload */
    			BYTE* prespSceneList = ( pResponsePayloadStartLocation + SIZE_OF_STATUS + 
    									 SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID + sizeof ( respSceneCount ));
    
    			/* Get the scene count */
    			sceneCount = Update_AndGetSceneCount (endPointId, Get_SceneCount);
    
    			/* Calculate the remaining capacity of the scenes table */
    			remaingCapacity = MAX_SCENES - sceneCount;
    
    			/* check if the status is not invalid field */
    			if ( ZCL_InvalidField != status )
    			{
    				/* Set the status as success */
    				status = ZCL_Success;
    
    				/* Loop through the scenes table and find out the scene entries for 
    				   the requested group id */
    				for ( i = 0; i < MAX_SCENES; i++ )
    				{
    					/* Get the scene id from the scene table */
    					sceneId = Get_SceneTableEntry ( groupAddr.Val, i );
    					
    					/* Check if this is a valid scene id. If so, update the scene count 
    					   and scene list */
    					if ( INVALID_SCENE_ID != sceneId )
    					{
    						respSceneCount++;
    						*prespSceneList++ = sceneId;
    					}
    				}
    				
    				/* Put the scene count in the response payload */
    				*( pResponsePayloadStartLocation + SIZE_OF_STATUS + 
    				   SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID ) = respSceneCount;
    				
    				/* Update the response payload length by scene count plus a byte whch holds 
    				   the scene count */
    				responsePayloadLangth += ( 1 + respSceneCount );
    			}
    
    			/* Update the total response payload length */
    			responsePayloadLangth += SIZE_OF_STATUS + SIZE_OF_GROUP_ID + SIZE_OF_SCENE_ID;
    
    			/* Copy the status */
    			*pResponsePayloadStartLocation = status;
    
    			/* Copy the remaining capacity */
    			*( pResponsePayloadStartLocation + SIZE_OF_STATUS )  = remaingCapacity;
    			
    			/* Copy the group id in the response payload */
    			memcpy
    			(
    				pResponsePayloadStartLocation + 2,
    				(BYTE*)&(groupAddr.Val),
    				SIZE_OF_GROUP_ID
    			);
    
    			/* Response is already created for this command. So no need to create it again */
    			createResp = FALSE;
    		}
    		break;
    
    		default:
    			createResp = FALSE;
    			break;
    	}
    }	

	/* Check if the response is to be created */
	if ( createResp )
	{
		/* Copy the status */
		*pResponsePayloadStartLocation = status;
		
		/* Copy the group id in the response payload */
		memcpy
		(
			pResponsePayloadStartLocation + SIZE_OF_STATUS,
			(BYTE*)&(groupAddr.Val),
			SIZE_OF_GROUP_ID
		);
		
		/* Copy the scene id in the response payload */
		*( pResponsePayloadStartLocation +  SIZE_OF_STATUS + SIZE_OF_GROUP_ID ) = sceneId;
	}

	return responsePayloadLangth;
}


/******************************************************************************
 *	Function:   	void VerifySceneTableEntry
 *					(
 *    					BYTE groupId,
 * 						BYTE sceneId,
 *    					BYTE* psceneTableIndex
 *					)
 *
 * PreCondition:    None
 *
 * Input:           groupId  - The groupId for which the scene entry is to be checked.
 *					groupId - The scene id for which the scene entry is to be checked.
 *
 * Output:          psceneTableIndex - This point to the index of the scene table entry 
 *                                     whch is matched for group id and scene id.
 * Return :         None
 * Side Effects:    None
 *
 * Overview:        This function checks whether scene table entry exists for a requested 
 *                  group id and scene id.
 *****************************************************************************/

void VerifySceneTableEntry 
( 
	WORD groupId, 
	BYTE sceneId, 
	BYTE* psceneTableIndex 
)
{
	BYTE index;

	App_ScenesRecord appSceneRecord;
	for ( index = 0; index < MAX_SCENES; index++ )
	{
		/* Get the scene entry from the scenes table */
		GetSceneTableEntry 
		(
			&appSceneRecord,
			App_ScenesTable + (WORD)(index) * (WORD)sizeof(App_ScenesRecord)	
		);
		/* If the requested group id and scene id found, then update the scene table 
		   index to this scne table entry */
		if ( ( appSceneRecord.groupId == groupId ) && ( appSceneRecord.sceneId == sceneId ) )
		{
			*psceneTableIndex = index;
			return;
		}
	}
	/* Scene id and group id are not found, update scnee table with max scenes */
	*psceneTableIndex = MAX_SCENES;
}

/*********************************************************************
 * Function:        BYTE Get_EmptySceneTableEntry ( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return :         Return the index of the free scene table entry.
 *
 * Side Effects:    None
 *
 * Overview:        This function gets the free entry (if any ) from scene table.
 *
 * Note:            None
 ********************************************************************/

BYTE Get_EmptySceneTableEntry ( void )
{
	BYTE index;

	App_ScenesRecord appSceneRecord;

	for ( index = 0; index < MAX_SCENES; index++ )
	{
		/* Get the scene entry from the scenes table */
		GetSceneTableEntry 
		(
			&appSceneRecord,
			App_ScenesTable + (WORD)(index) * (WORD)sizeof(App_ScenesRecord)	
		);
		
		/* If entry is free, then returns the index of the free entry */
		if ( ( NO_GROUP_ID == appSceneRecord.groupId ) )
		{			
			return index;
		}
	}
	/* Free entry is not found, returns max scenes */
	return MAX_SCENES;
}

/******************************************************************************
 * Function:        void GetClusterDefinition
 *                  (
 *                      WORD clusterId, 
 *                      ZCL_DeviceInfo* pDeviceInfo,
 *                      ROM ZCL_ClusterDefinition** pZCLClusterDefinition
 *                  );
 *
 * PreCondition:    None
 *
 * Input:
 *                  clusterId - cluster id for which cluster defination has to be get
 *                  pDeviceInfo -  This is the endpoint for which the command is
 *                                received.
 *                  clusterId  -  This holds the device info of a particular endpoint
 *
 * Output:          pZCLClusterDefinition - This holds the cluster definition of
 *                                          a particular cluster endpoint.
 *
 *
 * Return :         None.
 *
 * Side Effects:    None
 *
 * Overview:        This function loops through all the clusters supported by the particular 
 *				    endpoint and comapres the requested cluster id with every cluster. If match 
 *					is found then it updates cluster defination pointer for that cluster id. Otherwise 
* 					it update the cluster defination with the null pointer.
 * Note:            None
 *****************************************************************************/

void GetClusterDefinition 
( 
	WORD clusterId, 
	ZCL_DeviceInfo* pDeviceInfo, 
	ROM ZCL_ClusterDefinition** pClusterDefinition
)
{
	BYTE i;

	/* Loop through all the clusters */
	for( i = 0x00; i < (pDeviceInfo->numberOfClustersSupported); i++ )
	{
		/* If the requested cluster id is found, then returns the pointer to the cluster 
			defination for that cluster id */
	    if( clusterId == pDeviceInfo->pListOfClusterDefinition[i]->pClusterInfo->clusterId )
	    {
	        (*pClusterDefinition) = ( pDeviceInfo->pListOfClusterDefinition[i]);
			break;	        
	    }
	}
	
	/* If requested cluter id is not found, then returns the null pointer */
	if ( i == ( pDeviceInfo->numberOfClustersSupported ) )
	{
		(*pClusterDefinition) = (ZCL_ClusterDefinition*)NULL_PTR;
	}
}

/******************************************************************************
 * Function:        void Init_ScenesTable ( void )
 *
 * PreCondition:    None
 *
 * Input:			None 
 *
 * Output:          None
 *
 * Return :         None.
 *
 * Side Effects:    None
 *
 * Overview:        This function initialize the scenes table and reset all the global scene
 *                  variables to their default values.
 * Note:            None
 *****************************************************************************/
void Init_ScenesTable ( void )
{
	App_ScenesRecord appSceneRecord;

	BYTE i;	

	/*Set the scene table variable with all FF's */
	memset
	( 
		&appSceneRecord,
		0xFF, 
		sizeof(App_ScenesRecord)
 	);

	for ( i = 0; i < MAX_SCENES; i++ )
	{
		PutSceneTableEntry
		(	
			App_ScenesTable + (WORD)(i) * (WORD)sizeof(App_ScenesRecord), 
			&appSceneRecord
		);
	}
//	#if (APP_USE_EXTERNAL_NVM == 0)
	 //   memset( &App_ScenesTable, 0xFF, sizeof(App_ScenesTable) );
//	#endif
	
	/* Initialize the scene recall command record with transition count as one */
	scenesRecallCmdRecord.txnCount = 0x01;

	/* Initialize the scene valid info record with null */
	memset( &sceneValidInfo, 0x00, sizeof(Scene_ValidInfo) );


}

/******************************************************************************
 *	Function:   	BYTE Get_SceneTableEntry
 *					(
 *    					WORD groupId,
 *    					BYTE index
 *					)
 *
 * PreCondition:    None
 *
 * Input:           groupId  - The groupId for which the scene entry is to be checked.
 *					index - The scene table index for group id is to be checked.
 *
 * Output:          None
 * Return :         Returns the scene id. 
 * Side Effects:    None
 *
 * Overview:        This function checks whether group id matched with the requested scene table 
 *					entry. If group id is matched, then it returns the scene id of this entry, 
 *					otherwise returns the invlid scene id.
 *****************************************************************************/

BYTE Get_SceneTableEntry 
( 
	WORD groupId, 
	BYTE index 
)
{
	App_ScenesRecord appSceneRecord;

	/* Get the scene entry from the scenes table */
	GetSceneTableEntry 
	(
		&appSceneRecord,
		App_ScenesTable + (WORD)(index) * (WORD)sizeof(App_ScenesRecord)	
	);
	
	/* Returns the scene id, if group id is matched */
	if ( ( groupId == appSceneRecord.groupId ) )
	{			
		return appSceneRecord.sceneId;
	}

	/* Retrun FF if group id is not matched */
	return 0xFF;
}


/******************************************************************************
 *	Function:   	BYTE Update_AndGetSceneCount
 *					(
 *    					BYTE endPointId,
 *						BYTE direction
 *					)
 *
 * PreCondition:    None
 *
 * Input:           endPointId - The endpoint for which scene count has to be updated     
 *		    		direction - The direction field tells whether scene count has to be 
 *							    updated( incremented or decremented or reset to zero ) or 
 *								scene count has to be get.
 *
 * Output:          None
 * Return :         It returns the value of the scene count variable.
 * Side Effects:    None
 *
 * Overview:        This function  performs the action according to the direction field. If 
 *					the direction field is get scene count, then it will simply returns the 
 *					scene count attribute value. If the direction field is reset scene count 
 *					then it will reset the scene count to zero. If the direction field is 
 *					incement\decrement scene count then it will increment\decrement the scene 
 *					count variable value 
 *****************************************************************************/	
BYTE Update_AndGetSceneCount 
( 
	BYTE endPointId,
	BYTE direction
)
{
	BYTE sceneCount = 0x00;

	/* Check if direction is not reset count.If direction is reset count,then put 
	   scene count as zero in the attribute table */
	if ( Reset_SceneCount != direction)
	{
		/* Get the value of the scene count attribute */
		App_ReadWriteSingleAttribute
		(
			endPointId,
			ZCL_SCENES_SCENE_COUNT,
			&(zcl_ScenesClusterServerInfo),
			&sceneCount,
			ReadAttribute		
		);
	
		/* Check If direction is get scenes count. If so, returns the scene count */
		if ( Get_SceneCount == direction )
		{
			return sceneCount;
		}
		/* If direction is increment scenes count, increment the scne count value */
		if ( Increment_SceneCount == direction )
		{
			sceneCount++;
		}
		/* If direction is decrement scenes count, decrement the scne count value */
		else
		{
			sceneCount--;
		}
	}
	/* Put the value of the scene count attribute */
	App_ReadWriteSingleAttribute
	(
		endPointId,
		ZCL_SCENES_SCENE_COUNT,
		&(zcl_ScenesClusterServerInfo),
		&sceneCount,
		WriteAttribute		
	);
	
	/* Returns the scene count */
	return sceneCount;
	
}

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
)
{
	ROM ZCL_ClusterDefinition* pZCLClusterDefinition;
	ZCL_DeviceInfo *pDeviceInfo = NULL;
	BYTE attrLength = 0x00;
	BYTE isAnalog = 0x00;
	BYTE extFieldLength = 0x00;
	WORD attributeId;
	DWORD attrData = 0x00000000;
	App_ScenesRecord appSceneRecord;

	/* Loop variables */
	BYTE i;
	BYTE j;

	/* Get the scene entry from the scenes table */
	GetSceneTableEntry 
	(
	
		&appSceneRecord,
		App_ScenesTable + (WORD)(sceneTableIndex) * (WORD)sizeof(App_ScenesRecord)	
	);
	
	/* Get the device server info for the received endpoint*/
	for ( i = 0; i < NUMBER_OF_SUPPORTED_END_POINTS; i++ )
	{
		if(NOW_I_AM_A_ROUTER())
			pDeviceInfo = pAppListOfDeviceServerInfo_MTR[ i ];
		else if (NOW_I_AM_A_CORDINATOR())
			pDeviceInfo = pAppListOfDeviceServerInfo_ESP[ i ];
		
	
		/* Come out of loop if end point matched with the requested endpoint*/
		if ( pDeviceInfo->endpointId == endPointId )
		{
			break;
		}
	}
	
	/* Loop through all cluster which has extension fields */
	for ( i = 0; i < 3; i++ )
	{
		/* check if it is a valid cluster id or not */
		if ( NO_CLUSTER_ID == appSceneRecord.extFieldRecord[i].clusterId )
		{
			break;
		}					
		
		/* Get the cluster defination stored in ROM */
		GetClusterDefinition 
		( 
			appSceneRecord.extFieldRecord[i].clusterId,
			pDeviceInfo, 
			&pZCLClusterDefinition
		);

        /*If the ClusterId is not supported by the device, then no operation should 
        be performed for this cluster. But we should continue checking to next 
        extension fields, if any*/
        if ( NULL_PTR != pZCLClusterDefinition )
        {
    		/* Make the ext fiel to zero before looping through the attribute table */
    		extFieldLength = 0x00;
    	
    		/* Loop through all the attributes in the attribute table for this cluster id */ 
    		for ( j = 0; ( ( j < pZCLClusterDefinition->pClusterInfo->numberOfAttributes) && 
    			 (extFieldLength < (appSceneRecord.extFieldRecord[i].attributesLength ) ) ); j++ )						
    		{
    			attributeId = pZCLClusterDefinition->pClusterInfo->pListofAttibuteInfo[j].attributeId;
    			
    			/* Check if the attribute has scenes extension */
    			if ( pZCLClusterDefinition->pClusterInfo->pListofAttibuteInfo[j].attributeProperties.bits.scenesExtn )
    			{
    				/* Get the attribute type ( analog or digital )*/
    				isAnalog = 
    					CheckIfAttrIsAnalog( pZCLClusterDefinition->pClusterInfo->pListofAttibuteInfo[j].attributeDataType );
    
    				/* Check if direction is recall scene command */
    				if ( RecallScene == direction )
    				{
    
    					/* Read the length of the attribute id */
    					attrLength =  ZCL_GetDataLengthForAttributeDataType
    	                           	  (
    		                            	pZCLClusterDefinition->pClusterInfo->pListofAttibuteInfo[j].attributeDataType,
    		                            	0xFF 
    	                              );
    
    					/* We are making assumption that any attribute which supports scnenes extension, 
    					   the maximum value will be 4 byte only */
    					attrData = 0x00000000;
    					
    					/* Get the attribute data */
    					memcpy
    					(
    						( BYTE*)&attrData,
    						&( appSceneRecord.extFieldRecord[i].attributeData[extFieldLength] ),
    						attrLength
    					);
    
    					/* Check if attribute is analog or discrete. If discrete, put the given vlaue immidiately, 
    						otherwise store the total value in transition time */
    					if ( isAnalog )
    					{
    						/* Check if timer is expired. If so, store the value immidiately. 
    						   Otherwise store in parts */
    						if ( (scenesRecallCmdRecord.sceneTxnTime) && (appSceneRecord.sceneTransitionTime))
    						{
    							/* check if the variable is signed integer and its value is negative. If so, store only 
    								the negative offset in transition time */
    							if ( (IsSignedInteger( pZCLClusterDefinition->pClusterInfo->pListofAttibuteInfo[j].attributeDataType ) )&&
    								( attrData & (WORD)1 << ( (attrLength * 8 )-1) ) )
    							{
    								/* Calculate the offset data to be stored */	
    								attrData = 
    									( ( attrData ^ ( WORD )1 << ( (attrLength * 8 )-1) ) * scenesRecallCmdRecord.txnCount ) / 
                                          ( appSceneRecord.sceneTransitionTime );
    
    								/* Put the MSB as 1 */
    								attrData |= ( WORD )1 << ( (attrLength * 8 )-1) ;
    								
    							}
    							else
    							{
    								/* Calculate the offset data to be stored */	
    								attrData = 
    									( attrData * scenesRecallCmdRecord.txnCount ) / ( appSceneRecord.sceneTransitionTime );
    							}
    						}
    					}
    
    					/* Write the digital attributes only first time ( when count is zero) 
    					   but analog attributes will be written whenever this function will be 
    					   called with recall scene direction*/
    					if ( ( scenesRecallCmdRecord.txnCount == 0x01 ) || (isAnalog ))
    					{
    						App_ReadWriteSingleAttribute
    						(
    							endPointId,
    							attributeId,
    							(pZCLClusterDefinition->pClusterInfo),
    							(BYTE*) &attrData,
    							WriteAttribute		
    						);
    					}
    
    					/* Update the ext field length also */
    					extFieldLength += attrLength;
    				}
    				else 
    				{
    					/* Read the attribute in the local variable */
    					attrLength= (App_ReadWriteSingleAttribute
    								(
    									endPointId,
    									attributeId,
    									(pZCLClusterDefinition->pClusterInfo),
    									(BYTE*) &attrData,
    									ReadAttribute		
    								));
    
    					/* check if the command is store scene. If so, write the attribute value in the scenes table */
    					if ( StoreScene == direction )
    					{
    						memcpy
    						(
    							&( appSceneRecord.extFieldRecord[i].attributeData[extFieldLength] ),
    							( BYTE*)&attrData,							
    							attrLength
    						);	
    						extFieldLength += attrLength;
    					}
    					/* Check if any attribute value is changed or not after the successful store 
    					   or recall scene command */
    					else if ( CheckSceneValidity == direction )
    					{
        					/* Check if the attribute is analog and the transition timer is not expired yet. 
    						   If attribute is analog, dont comare the attribute,because attribute will updated 
    						   fully when the transition timer will expire*/
    						if ( !( isAnalog && scenesRecallCmdRecord.sceneTxnTime ) )
    						{
    							/* Compare the attribute value. If it is changed, then make the scene valid attribute
     							   and flag to false */	
    							if ( memcmp( 
    											&( appSceneRecord.extFieldRecord[i].attributeData[extFieldLength] ), 
    											( BYTE*)&attrData,
    											attrLength
    									   ) )
    							{
    								/* Make the scene valid flag to false */								
    								Scenes_UpdateSceneValidFields 
    								( 
    									sceneTableIndex,
    								 	endPointId, 
    								 	FALSE
    								);
    							}
    						}
    						extFieldLength += attrLength;
    					}
    				}							
    			}
    		}
		}
	}	

	return 	appSceneRecord;
}

/******************************************************************************
 *	Function:   	void Scenes_UpdateSceneValidFields
 *					(
 *    					BYTE sceneTableIndex,
 *						BYTE endPointId,
 						BYTE isSceneValid
 *					)
 *
 * PreCondition:    None
 *
 * Input:           sceneTableIndex - The index of the scene table entry for which scene 
 *									  valid fields to be updated.      
 *		    		endPointId - The end point for which the scene valid fields to be updated.
 *					isSceneValid - This variable willl tell whether scene valid attribute value 
 *                                  should be true or false.
 *
 * Output:          None
 * Return :         None. 
 * Side Effects:    None
 *
 * Overview:        This function  gets called when store scene or recall scene command is received. 
 *					This function uapdte the attributes related to scenes valid and sets the scene valid flag.
 *****************************************************************************/
void Scenes_UpdateSceneValidFields 
( 
	BYTE sceneTableIndex,
 	BYTE endPointId, 
 	BYTE isSceneValid
)
{

	App_ScenesRecord appSceneRecord;

	/* Get the scene entry from the scenes table */
	GetSceneTableEntry 
	(
	
		&appSceneRecord,
		App_ScenesTable + (WORD)(sceneTableIndex) * (WORD)sizeof(App_ScenesRecord)	
	);	

	/* Fill the scene vaild info structure fields */
	sceneValidInfo.sceneValid = isSceneValid;

	sceneValidInfo.endPoint = endPointId;

	sceneValidInfo.sceneTableIndex = sceneTableIndex;

	/* Update the current scene and current group only if scne is valid */
	if ( isSceneValid)
	{
		
	/* Update the current group id attribute */
		App_ReadWriteSingleAttribute
		(
			endPointId,
			ZCL_SCENES_CURRENT_GROUP,
			&(zcl_ScenesClusterServerInfo),
			(BYTE*)&(appSceneRecord.groupId),
			WriteAttribute		
		);
	
		/* Update the current scene id attribute */
		App_ReadWriteSingleAttribute
		(
			endPointId,
			ZCL_SCENES_CURRENT_SCENE,
			&(zcl_ScenesClusterServerInfo),
			(BYTE*)&(appSceneRecord.sceneId),
			WriteAttribute		
		);
	}
	
	/* Make the scene valid attribute to true */
	App_ReadWriteSingleAttribute
	(
		endPointId,
		ZCL_SCENES_SCENE_VALID,
		&(zcl_ScenesClusterServerInfo),
		&(sceneValidInfo.sceneValid),
		WriteAttribute		
	);
}

/******************************************************************************
 *	Function:   	void App_DeleteScenesTableEntry
 *					(
 *						BYTE endPointId
 *    					WORD groupId
 *					)
 *
 * PreCondition:    None
 *
 * Input:           groupId  - The groupId for which the scene entries has to be deleted.
 *					endPointId - The endpoint for which scene entry has to be removed.
 *
 * Output:          None
 * Return :         None. 
 * Side Effects:    None
 *
 * Overview:        This function loop through the whole scene table and deletes all those 
 *                  entries which are matched the requested group id
 *****************************************************************************/
void App_DeleteScenesTableEntry 
( 
	BYTE endPointId,
	WORD groupId
)
{
	BYTE i;
	BYTE sceneId;
	App_ScenesRecord appSceneRecord;

	/* Set the scene table variable with all FF's */
	memset
	( 
		&appSceneRecord, 
		0xFF,
		sizeof(App_ScenesRecord) 
	);

	/* Delete all the entries in the scenes table for this group id, if any */
	for ( i = 0 ; i < MAX_SCENES; i++ )
	{
		sceneId = Get_SceneTableEntry 
				  ( 
					  groupId,
					  i
				  );
		
		if ( 0xFF != sceneId )
		{
			
			/* Check if the scene is valid for the entry whch has to be removed. 
	    				   If so, make the scene invalid */
 			if ( (sceneValidInfo.sceneValid) && (sceneValidInfo.sceneTableIndex == i) )
 			{
 				Scenes_UpdateSceneValidFields 
				( 
					i,
				 	endPointId, 
				 	FALSE
				);
 			}
			/* update the scene table */					
			PutSceneTableEntry
			(	
				App_ScenesTable + (WORD)(i) * (WORD)sizeof(App_ScenesRecord), 
				&appSceneRecord
			);
			
			/* Decrement the scenes count also */
			Update_AndGetSceneCount( endPointId, Decrement_SceneCount );
		}
	}
}



/******************************************************************************
 * Function:        void Update_sceneExtnWithCurrentAttrb
 *                  (
 *                      BYTE endPointId, 
 *                      App_ScenesRecord* pAppSceneRecord
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           endPointId - The endpoint for which scene entry has to be stored.  

 * Output:          pAppSceneRecord - This holds the location of the scene table entry in which 
 *                                    extension fields has to be copied.
 *
 *
 * Return :         None.
 *
 * Side Effects:    None
 *
 * Overview:        This function  gets called when store scene command is received for the scene id and 
 *					group id for which earlier scene was not added with add scene command. This function 
 *					loops through all the clusters supported on this endpoint and update the extension fields 
 *					for the clusters which supports extension fields.
 * Note:            None
 *****************************************************************************/

void Update_sceneExtnWithCurrentAttrb 
( 
	BYTE endPointId, 
	App_ScenesRecord* pAppSceneRecord  
)
{

	ROM ZCL_ClusterDefinition* pZCLClusterDefinition;
	BYTE extFieldLength = 0x00;
	BYTE attrLength = 0x00;
	BYTE extFieldRecordIndex = 0x00;
	ZCL_DeviceInfo *pDeviceInfo = NULL;
	WORD attributeId;
	DWORD attrData = 0x00;
	BYTE i;
	BYTE j;
	BYTE updateExtFieldIndex;

	/* Get the device server info for the received endpoint*/
	for ( i = 0; i < NUMBER_OF_SUPPORTED_END_POINTS; i++ )
	{
		if(NOW_I_AM_A_ROUTER())
			pDeviceInfo = pAppListOfDeviceServerInfo_MTR[ i ];
		else if (NOW_I_AM_A_CORDINATOR())
			pDeviceInfo = pAppListOfDeviceServerInfo_ESP[ i ];
	
		/* Come out of loop if end point matched with the requested endpoint*/
		if ( pDeviceInfo->endpointId == endPointId )
		{
			break;
		}
	}
	
	/* Loop through all cluster which has extension fields */
	for ( i = 0; i < pDeviceInfo->numberOfClustersSupported; i++ )
	{
		
		/* Check if extension field index is less than three. Bacause currently we 
           are supporting extension fields for max three clusters in scene table*/
		if ( extFieldRecordIndex >= MAX_EXT_FIELD_RECORDS )
		{
			break;
		} 					
		
		/* Get the cluster defination stored in ROM */
			GetClusterDefinition 
			( 
				pDeviceInfo->pListOfClusterDefinition[i]->pClusterInfo->clusterId,
				pDeviceInfo, 
				&pZCLClusterDefinition
			);

		/* Make the ext field to zero before looping through the attribute table */
		extFieldLength = 0x00;
		
		updateExtFieldIndex = FALSE;
	
		/* Loop through all the attributes in the attribute table for this cluster id */ 
		for ( j = 0; ( ( j < pZCLClusterDefinition->pClusterInfo->numberOfAttributes) && 
			 (extFieldLength < MAX_SCENES_ATTR_LENGTH ) ); j++ )						
		{
			attributeId = pZCLClusterDefinition->pClusterInfo->pListofAttibuteInfo[j].attributeId;
			
			/* Check if the attribute has scenes extension */
			if ( pZCLClusterDefinition->pClusterInfo->pListofAttibuteInfo[j].attributeProperties.bits.scenesExtn )
			{
				updateExtFieldIndex = TRUE;
				
				/* Read the attribute in the local variable */
				attrLength= (App_ReadWriteSingleAttribute
							(
								endPointId,
								attributeId,
								(pZCLClusterDefinition->pClusterInfo),
								(BYTE*) &attrData,
								ReadAttribute		
							));				

				/* Copy the current attribute value in the extension field */
				memcpy
				(
					&( pAppSceneRecord->extFieldRecord[extFieldRecordIndex].attributeData[extFieldLength] ),
					( BYTE*)&attrData,							
					attrLength
				);

				extFieldLength += attrLength;
				
				/* Update the lengh of extension fields in scenes table */
				pAppSceneRecord->extFieldRecord[extFieldRecordIndex].attributesLength = extFieldLength;
			}
		}
		
		/* Update ext field record only if any attribute with extension field found */
		if ( updateExtFieldIndex )
		{
			
			/* Copy the cluster id in the extension field */	
			 pAppSceneRecord->extFieldRecord[extFieldRecordIndex].clusterId = 
											pDeviceInfo->pListOfClusterDefinition[i]->pClusterInfo->clusterId;		

			
			/* Increment the extension field index */
			extFieldRecordIndex++;
		}
	}
}

void Update_LastConfiguredByAttr 
( 
	BYTE endPointId
)
{

	/* Update the last configured by attribute */
	App_ReadWriteSingleAttribute
	(
		endPointId,
		ZCL_SCENES_LAST_CONFIGURED_BY,
		&(zcl_ScenesClusterServerInfo),
		(InitiatorDeviceInfo.initiatorLongAddress.LongAddr.v),
		WriteAttribute		
	);
	
}

#endif /*I_SUPPORT_SCENES*/
