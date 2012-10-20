/*********************************************************************
 *
 *                  ZCL Interface Header File
 *
 *********************************************************************
 * FileName        : ZCL_Interface.h
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

#ifndef _ZCL_INTERFACE_H
#define _ZCL_INTERFACE_H

/*****************************************************************************
  Includes
 *****************************************************************************/
#include "generic.h"
#include "SymbolTime.h"
#include "zAIL.h"

/*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
#define NULL_PTR                                (void*)(0)
#define EP_CLUSTERID_NOT_FOUND_IN_NVM           0xFF
#define ZCL_HEADER_LENGTH_WO_MANUF_CODE         0x03
#define ZCL_HEADER_LENGTH_WITH_MANUF_CODE       0x05
#define ZCL_DIRECTION_BIT_MASK                  0x08
/*Manufacturer specific bit in Frame Type */
#define ZCL_FRAME_CONTROL_MANUFACTURE_SPECIFIC  0x04
#define ZCL_FRAME_TYPE_MASK                     0x03

/*Enumerations for ZCL status*/
typedef enum ZCL_Status
{
   ZCL_Success                                  = 0x00,
   ZCL_Failure                                  = 0x01,
   ZCL_MalformedCommand                         = 0x80,
   ZCL_UnSupportedClusterCommand                = 0x81,
   ZCL_UnSupportedGeneralCommand                = 0x82,
   ZCL_UnSupportedManufacturerClusterCommand    = 0x83,
   ZCL_UnSupportedManufacturerGeneralCommand    = 0x84,
   ZCL_InvalidField                             = 0x85,
   ZCL_UnsupportedAttribute                     = 0x86,
   ZCL_InvalidValue                             = 0x87,
   ZCL_ReadOnly                                 = 0x88,
   ZCL_InsufficientSpace                        = 0x89,
   ZCL_DuplicateExists                          = 0x8A,
   ZCL_NotFound                                 = 0x8B,
   ZCL_UnReportableAttribute                    = 0x8C,
   ZCL_InvalidDataType                          = 0x8D,
   Inconsistent_Startup_State                   = 0x90,
   ZCL_HardwareFailure                          = 0xC0,
   ZCL_SoftwareFailure                          = 0xC1,
   ZCL_CalibrationError                         = 0xC2,
}ZCL_Status;

/*Enumeration for Generic Commands specified by ZCL Foundation*/
typedef enum ZCL_Generic_Commands
{
   ZCL_Read_Attribute_Cmd                       = 0x00,
   ZCL_Read_Attribute_Response_Cmd              = 0x01,
   ZCL_Write_Attributes_Cmd                     = 0x02,
   ZCL_Write_Attributes_Undivided_Cmd           = 0x03,
   ZCL_Write_Attributes_Response_Cmd            = 0x04,
   ZCL_Write_Attributes_No_Response_Cmd         = 0x05,
   ZCL_Configure_Reporting_Cmd                  = 0x06,
   ZCL_Configure_Reporting_Response_Cmd         = 0x07,
   ZCL_Read_Reporting_Configuration_Cmd         = 0x08,
   ZCL_Read_Reporting_Configuration_Response_Cmd= 0x09,
   ZCL_Report_Attributes_Cmd                    = 0x0A,
   ZCL_Default_Response_Cmd                     = 0x0B,
   ZCL_Discover_Attributes_Cmd                  = 0x0C,
   ZCL_Discover_Attributes_Response_Cmd         = 0x0D
}ZCL_Generic_Commands;

/*enumerations for attribute data types*/
typedef enum  ZCL_Attribute_Data_Type
{
   No_Data                                      = 0x00,
   Eight_Bit_Data                               = 0x08,
   Sixteen_Bit_Data                             = 0x09,
   Twenty_Four_Bit_Data                         = 0x0A,
   Thirty_Two_Bit_Data                          = 0x0B,
   Boolean_Data                                 = 0x10,
   Eight_Bit_Bitmap_Data                        = 0x18,
   Sixteen_Bit_Bitmap_Data                      = 0x19,
   Twenty_Four_Bit_Bitmap_Data                  = 0x1A,
   Thirty_Two_Bit_Bitmap_Data                   = 0x1B,
   Unsigned_Eight_Bit_Integer_Data              = 0x20,
   Unsigned_Sixteen_Bit_Integer_Data            = 0x21,
   Unsigned_Twenty_Four_Bit_Integer_Data        = 0x22,
   Unsigned_Thirty_Two_Bit_Integer_Data         = 0x23,
   Unsigned_Fourty_Eight_Bit_Integer_Data       = 0x25,
   Signed_Eight_Bit_Integer_Data                = 0x28,
   Signed_Sixteen_Bit_Integer_Data              = 0x29,
   Signed_Twenty_Four_Bit_Integer_Data          = 0x2A,
   Signed_Thirty_Two_Bit_Integer_Data           = 0x2B,
   Eight_Bit_Enumeration_Data                   = 0x30,
   Sixteen_Bit_Enumeration_Data                 = 0x31,
   Semi_Precision_Data                          = 0x38,
   Single_Precision_Data                        = 0x39,
   Double_Presicion_Data                        = 0x3A,
   Octet_String_Data                            = 0x41,
   Character_String_Data                        = 0x42,
   Time_Of_Day_Data                             = 0xE0,
   Date_Data                                    = 0xE1,
   UTC_Time                                     = 0xE2,
   Cluster_Id_Data                              = 0xE8,
   Attribute_Id_Data                            = 0xE9,
   BACNet_OID_Data                              = 0xEA,
   IEEE_Address_Data                            = 0xF0,
   One_Twenty_Eight_Bit_Sec_Key                 = 0xF1,
   Unknown_Data                                 = 0xFF

}ZCL_Attribute_Data_Type;

typedef enum ZCL_Actions
{
    No_Response                                 = 0x00,
    Send_Response                               = 0x01,
    Received_Response                           = 0x02
}ZCL_Actions;

typedef enum Attribute_Properties
{
    Readable                                    = 0x01,
    Writeable                                   = 0x02,
    Reportable                                  = 0x04,
    ScenesExtSupported                          = 0x08
}Attribute_Properties;

typedef enum Direction
{
    ZCL_ClientToServer                          = 0x00,
    ZCL_ServerToClient                          = 0x08
}Direction;

typedef enum ManufacturerSpecific
{
    ZCL_ManufacturerCodeFieldAbsent             = 0x00,
    ZCL_ManufacturerCodeFieldPresent            = 0x04
}ManufacturerSpecific;
typedef enum CommandType
{
    ZCL_GenericCommand                          = 0x00,
    ZCL_ClusterSpecificCommand                  = 0x01,
}CommandType;

/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
    /* None */


/*Parameterised Macros*/
#define ZCL_GetDirectionOfCommand( param ) \
                                        ( param[0] & ZCL_DIRECTION_BIT_MASK )

/*Check if the ZCL command is manufacture specific or not
0 - if the command is manufacture specific
1 - if the command is not manufacture specific*/
#define ZCL_CheckIfManufactureSpecific( param ) \
    ( !( param[0] & ZCL_FRAME_CONTROL_MANUFACTURE_SPECIFIC ) )

/*Check if the received ZCL command is manufacture specific or not, based
on the result, return the ZCL header length*/
#define ZCL_GetZCLHeaderLength( param ) \
        ( ( param[0] & ZCL_FRAME_CONTROL_MANUFACTURE_SPECIFIC ) ?  \
        ZCL_HEADER_LENGTH_WITH_MANUF_CODE : ZCL_HEADER_LENGTH_WO_MANUF_CODE )

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
     /* None */

/*****************************************************************************
  Data Structures
 *****************************************************************************/

/*-----------------------------------------------------------------------------
 Structure Name : ZCL_AttributeInfo
 Fields:
 attributeId        -   This is 16-bit identifier defining an attribute.
 attributeDataType  -   This is an 8-bit enumeration that defines the length
                        of the attribute data.There will be a table maintained
                        in the ZCL that links the attribute data type
                        enumerations with the actual length in terms of bytes.
 attributeProperties -  This defines the properties of the attribute. It will
                        say if the attribute id is readable, writeable and
                        Reportable.
 It denotes the each attribute supported by the cluster Ids are stored in
 the NVM in the above format.
-----------------------------------------------------------------------------*/
typedef struct __attribute__((packed,aligned(1))) _ZCL_AttributeInfo
{
   WORD attributeId;
   BYTE attributeDataType;
   union _Attribute_Properties
   {
       BYTE    val;
       struct _attribute_properties
       {
           BYTE    readable  : 1;
           BYTE    writeable : 1;
           BYTE    reportable: 1;
           BYTE    scenesExtn: 1;
       } bits;
   }attributeProperties;

}ZCL_AttributeInfo;

/*************************************************************
 Overview: This Structre defines ClusterInfo.It Consists of 
           Cluster and List of Supported attributes. 
**************************************************************/
typedef struct __attribute__((packed,aligned(1))) _ZCL_ClusterInfo
{
     WORD   clusterId; // It is a 16-bit integer which defines the cluster.
     BYTE   numberOfAttributes;/*This is 8-bit identifier gives the count of 
                                 attributes within a cluster.*/
     ROM    ZCL_AttributeInfo *pListofAttibuteInfo;/*This pointer points to the
                                                   list of attribute info list.
                                                   Each cluster defines a set of 
                                                   Attributes with unique Id.*/
}ZCL_ClusterInfo;

/*************************************************************
 Overview: This Structure Defines an Event.
**************************************************************/
typedef struct __attribute__((packed,aligned(1))) _ZCL_Event
{
   BYTE     eventId; /*This is a 8-bit integer and it is unique for each command either 
                 generic or cluster specific. This indicates the command received
                 by the Application. The event ids for each cluster are available
                 in respective header files.*/
   BYTE     *pEventData; /*This is the event data. This is closely associated with the eventId.
                The event data is specific to each command and takes the payload of
                the command. This is allocated by the Application and will be freed 
                by the Application*/
} ZCL_Event;

/*************************************************************
 Overview: This Structure is used to define the Action.
**************************************************************/
typedef struct __attribute__((packed,aligned(1))) _ZCL_ActionHandler
{
    BYTE        action; /*This is 8-bit integer. It indicates application what kind of action need to be taken.*/
    ZCL_Event   event; /*This holds the event id and the event data for the received command.
                         The memory for this pointer is allocated by the Application. 
                         The fields are updated by the ZCL module.*/
}ZCL_ActionHandler;

/*-----------------------------------------------------------------------------
  Structure Name : ZCL_AttributeLengthLookupTable
  Fields:
  attributeDataType    -  This is an 8-bit enumeration that defines the length
                          of the attribute data.
  dataLength           - This gives the actual length of the corresponding
                         attribute datatype.
  This table is maintained for storing attribute data types and correspoinding
  data lengths into ROM.
   -----------------------------------------------------------------------------*/
typedef struct _ZCL_AttributeLengthLookupTable
{
  BYTE attributeDataType;
  BYTE dataLength;
}ZCL_AttributeLengthLookupTable;


/*-----------------------------------------------------------------------------
Structure Name : ClusterSpecificFuncptr
 Fields :
 endPointId         - Some of the nodes supports multiple endpoints.For this
                      we need to mention explicitly for calling corresponding
                      cluster specific handlers of a given endpoint.
 pReceivedAsdu        - Points to the received ZCL header and payload.
 pResponseAsduStartLocation - This Pointer points to the Response Frame.It is
                              updated using cluster specific function.
 pActionHandler     - This is the pointer where ZCL updates the action to be
                      taken by the Application. Also sends the event and the
                      corresponding event data.
 pZCL_ClusterInfo   - This pointer holds the cluster information.

 The function pointer is used for handling cluster specific commands.
 ----------------------------------------------------------------------------*/
typedef BYTE (*ClusterSpecificFuncptr)(
                         BYTE endPointId,
                         BYTE asduLength,
                         BYTE* pReceivedAsdu,
                         BYTE* pResponseAsduStartLocation,
                         ZCL_ActionHandler *pActionHandler,
                         ROM ZCL_ClusterInfo* pZCL_ClusterInfo );

/*************************************************************
 Overview: This Structure Defines Cluster Definition.It holds
           clusterInfo and Corresponding function handler.
**************************************************************/
typedef struct __attribute__((packed,aligned(1))) _ZCL_ClusterDefinition
{
     ROM ZCL_ClusterInfo* pClusterInfo;//This pointer points to cluster information.
     ClusterSpecificFuncptr  function; //Function pointer for cluster specific function.
}ZCL_ClusterDefinition;


/*-----------------------------------------------------------------------------
Structure Name : ZCL_DeviceInfo
 Fields :
 endpointId - This is 8-bit endpoint identifier which defined by the device.
 numberOfClustersSupported - This denotes no.of cluster supproting on this
                             endpoint by the device.
 pListOfClusterDefinition    -  This is the pointer to the Listof cluster
                                definitions.
    This structure is used to define a device and supporting client and server
     clusters on this endpoint.
-----------------------------------------------------------------------------*/
typedef struct __attribute__((packed,aligned(1))) _ZCL_DeviceInfo
{
   BYTE endpointId;
   BYTE numberOfClustersSupported;
   ROM ZCL_ClusterDefinition** pListOfClusterDefinition;
}ZCL_DeviceInfo;

/*-----------------------------------------------------------------------------
 Structure Name : AppZCLHeaderInfo
 Fields :
 DirectionAndCommandType - This indicates if the ZCL frame is generic or
                              cluster specific command.
                              MSB indicates the direction of the command.
                              If it is '1' Command is sent from server to client
                              If it is '0' command is sent from client to server.
                              LSB inidcates the command type
                              0 - indicates generic command
                              1 - indicates cluster specific command
 ManufactureSpecific        - This indicates if the ZCL frame is manufacture
                              specific or not.
                                0x01 – manufacture specific command
                                0x00 – Standard ZCL command
 ManufactureCode            - This holds the actual manufacture code if the
                              manufacture specific byte is TRUE, else it holds
                              invalid manufacture code.
 DefaultResponse        - This indicates if default response need to be
                              sent for a cluster specific command or not.
 CommandId                  - This indicates the ZCL command that needs to be
                              sent over the air
 aZCLPayload                - This is the payload for each command. This is specific
                               to the command(Generic or cluster specific)
    Application wishes to initiate a ZCL command frame, it should fill the
    following structure
-----------------------------------------------------------------------------*/
typedef struct /*__attribute__((packed,aligned(1)))*/ _AppZCLHeaderInfo
{
   BYTE DirectionAndCommandType;
   BYTE ManufactureSpecific;
   WORD ManufactureCode;
   BYTE DefaultResponse;
   BYTE CommandId;
   BYTE aZCLPayload[1];
} AppZCLHeaderInfo;

/*****************************************************************************
   Variable definitions
 *****************************************************************************/
/* None */

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/


 /*********************************************************************************************
  Function:
      BYTE ZCL_GetDataLengthForAttributeDataType(BYTE attributeDataType,BYTE stringDataLength)

  Description:
    This function is used to get the attribute data length corresponding to the attributeDataType.
  Precondition:
    None
  Parameters:
    attributeDataType -  Specifies the Attribute Data Type for which data length needs to be found
    stringDataLength -   This is used only for Octet and Character String data type. This field holds the first byte of the
                   payload that specifies the total length of the attribute value.
  Return:
    It returns attribute data length corresponding to the attributeDataType.
  Example:
    <code>

    </code>
  Remarks:
    None.
  *********************************************************************************************/
BYTE ZCL_GetDataLengthForAttributeDataType(BYTE attributeDataType,BYTE stringDataLength);

/********************************************************************************
  Function:
        BYTE ZCL_ProcessReceivedFrame(
              BYTE endPointId,BYTE  asduLength,BYTE* pReceivedAsdu,
              BYTE* pResponseAsduStartLocation,ZCL_ActionHandler* pActionHandler,
              ROM ZCL_ClusterDefinition *pZCLClusterDefinition
              )
    
  Description:
    This function is invoked by the Application on receipt of any ZCL
    command. ZCL parses the Header of the received command and determines
    what command is received. Based on generic command or cluster specific
    command, ZCL processes it. If the command is generic, ZCL calls
    respective handlers to create a response command. If the command is
    cluster specific command, the call back function in the Cluster info is
    invoked to handle the same
  Conditions:
    None
  Input:
    endPointId -                           This is the endpoint for which
                                           the data frame should be
                                           processed.
    pApsPayload -                          This points to the actual data
                                           received in the APSDE\-Data
                                           Indication. This consists of ZCL
                                           header and Payload.
    pZCLClusterDefinition -                Points to the cluster Definition
                                           information.
    pResponsePayload (Output parameter) -  This is updated by ZCL while
                                           creating the Response for a
                                           request command. This will be
                                           pointing to the asdu field of
                                           Data Request command.
    pActionHandler (Output Parameter) -    This pointer updates if response
                                           has to be sent and what kind of
                                           event and payload is received.
  Return:
    It returns the length of response frame of ZCL Header and Payload.
  Example:
    <code>
    
    </code>
  Remarks:
    None.                                                                        
  ********************************************************************************/
BYTE ZCL_ProcessReceivedFrame
(
    BYTE endPointId,
    BYTE  asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler* pActionHandler,
    ROM ZCL_ClusterDefinition *pZCLClusterDefinition
);

/*********************************************************************************************
  Function:
      BYTE ZCL_CreateHeader( BYTE* pReceivedAsdu,BYTE asdulength,BYTE seqnumber)

  Description:
    The Application fills AppZCLHeaderInfo and invokes this function to create ZCL header
    and Payload. The parameter pApsPayload is allocated by the Application. ZCL Modules
    uses Transaction sequence number and restructures the ZCL header. The asdulength
    passed as parameter gives the total length of header and the payload as given by Application.
    It returns the actual length taken by the ZCL header and ZCL payload. The Application should
    fill the address parameters and send the frame out.
  Precondition:
    None
  Parameters:
    pApspayload - It contains the AppZCLHeaderInformation.It is received by the application.
    asdulength  - This gives the length of the asdu as received in the data indication.
    seqnumber   -  This is the TSN maintained for ZCL packets.
    pApspayload-   This is updated with the actual ZCL Header(It is also used as Output Parameter)

  Return:
    It returns the actual length of ZCL header and ZCL payload.
  Example:
    <code>

    </code>
  Remarks:
    None.
 *********************************************************************************************/
BYTE ZCL_CreateHeader
(
    BYTE* pReceivedAsdu,
    BYTE asdulength,
    BYTE seqnumber
 );


/*********************************************************************************************
  Function:
      BYTE ZCL_ValidateIncomingFrame (
           BYTE direction,BYTE endpointId,WORD clusterId,ROM ZCL_ClusterDefinition **pZCLClusterDefinition);

  Description:
    This function is used to validate the received frame.The cluster id passed as input parameter is checked
    within the device information as exposed by the Application. If the cluster is supported, the Output
    parameter is assigned with the address of the cluster definition. If the cluster is not supported within the
    device info, it is assigned with NULL indicating the cluster is not supported.
  Precondition:
    None
  Parameters:
    direction  - This indicates the direction of the frame
    clusterId  - This is the clusterid for which the data frame is received.
    endpointId -  This is the endpoint for which the data frame is received.
    pZCLClusterDefinition - This is an output parameter holding the cluster specific information if
                            cluster is supported else it will be NULL.(Output Parameter)
  Return:
    It returns the SUCCESS or FAILURE.
  Example:
    <code>

    </code>
  Remarks:
    None.
 *********************************************************************************************/
 BYTE ZCL_ValidateIncomingFrame
 (
    BYTE direction,
    BYTE endpointId,
    WORD clusterId,
    ROM ZCL_ClusterDefinition **pZCLClusterDefinition
 );

/*********************************************************************************************
  Function:
      void ZCL_Callback_AddEntryIntoReportingTable(
        BYTE srcEndPoint,BYTE direction,WORD attributeId,WORD minReportingInterval,
        WORD maxReportingInterval,WORD timeOut,BYTE* ReportableChange,
        ROM ZCL_ClusterInfo *pZCLClusterInfo);

  Description:
    This function adds an entry into the reporting table.If there is an existing entry, it
    checks for the request if it has to be deleted.If so, it deletes else it updates the
    same entry. If the values dont exist in the table, new entry is added.
  Precondition:
    None
  Parameters:
    srcEndPoint - endpoint on which the attribute need to be configured
    direction   - indicates if the attribute is reported or receiving reports
    attributeId - attribute id that is to be reported
    minReportingInterval - minimum report interval at which the attribute to be reported
    maxReportingInterval - maximum reporting interval at which the attribute is to be reported
    timeOut - timeout value for the reports to be received
    ReportableChange - reportable change value for the attribute
    pZCLClusterInfo - points to the cluster info
  Return:
    None
  Example:
    <code>

    </code>
  Remarks:
    None.
 *********************************************************************************************/
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
);

/*********************************************************************************************
  Function:
       void ZCL_Callback_ValidateAndCreateReadReportRsp(
            BYTE direction,WORD clusterId,WORD attributeId,BYTE *pResponseAsdu,
            BYTE* pResponseDataLength, ROM ZCL_ClusterInfo *pZCLClusterInfo
            )

  Description:
    This function checks for all the entries in the reporting table and creates the read
    reporting configuration response
  Precondition:
    None
  Parameters:
    direction - gives server to client or client to server
    clusterId - cluster for which the read reporting command is received
    attributeId - attribute for which the read reporting command is received
    pZCLClusterInfo - points to the cluster info

    pResponseAsdu - location where read reporting configuration response is created(Output Parameter)
    pResponseDataLength - holds the zcl header length. It gets updated with read reporting
                          configuration response payload(Output Parameter)
   Return:
    None
  Example:
    <code>

    </code>
  Remarks:
    None.
 *********************************************************************************************/
void ZCL_Callback_ValidateAndCreateReadReportRsp
(
    BYTE direction,
    WORD clusterId,
    WORD attributeId,
    BYTE *pResponseAsdu,
    BYTE* pResponseDataLength,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);

/***************************************************************************************
  Function:
  DWORD ZCL_Callback_GetCurrentTimeInSeconds( void)

  Description:
   This function is used to get the current time in UTC format (Time elapsed since 01 January 2000 00 Hours).
   The Current Time in Seconds (UTC Format) will be returned.

  Precondition:
    None
  Parameters:
    None

Returns:
    seconds         - gives the seconds elapsed since 01 January 2000 00 hours.

  Example:
  <code>
  </code>

  Remarks:
    None.
 ***************************************************************************************/
DWORD ZCL_Callback_GetCurrentTimeInSeconds( void);

/************************************************************************************
  Function:
         void ZCL_Callback_GetAttributeValue(WORD index, BYTE *pDest, BYTE noOfBytes)

    Description:
    This function is used to get the attribute value from given NVM index.
    Precondition:
        None
    Parameters:
    index     -    From this index we need to retrieve the attribute value from NVM.
    noOfBytes  -  It gives the length of the data to be copied from NVM
    pDest (Output Parameter)     -  Specifies the attribute data to be copied to this location
  Return:
    None
  Example:
    <code>

    </code>
  Remarks:
    None.
  ************************************************************************************/
void ZCL_Callback_GetAttributeValue(WORD index, BYTE *pDest, BYTE noOfBytes);

/**************************************************************************************
  Function:
          void ZCL_Callback_PutAttributeValue(WORD index,BYTE *pSrc,BYTE noOfBytes    )

    Description:
    This function is used for writing the attribute value into the given  NVM index.
  Precondition:
    None
  Parameters:
    index  -  This gives the NVM location to where the attribute data  should be copied into NVM(0utput Parameter)
    pSrc  -   This pointer holds the attribute data which needs to be copied into NVM.
    noOfBytes -  It gives the length of the data to be copied into NVM
  Return:
    None
  Example:
    <code>
	
    </code>
  Remarks:
    None.
  **************************************************************************************/
void ZCL_Callback_PutAttributeValue(WORD index,BYTE *pSrc,BYTE noOfBytes);

/***********************************************************************************************
  Function:
           BYTE ZCL_Callback_GetClusterIndex(BYTE endpointId,WORD clusterId,WORD* pStorageIndex)
  Description:
    This function takes endpointId and clusterId as input parameter. This
    \function is called when the application wants to get/set the value of
    an Attribute supported by the given Cluster for the specified End
    Point. This function is used only to get the NVM index at which the
    Attributes for the specied cluster are located. This index should be
    passed to ZCL_Callback_GetAttributeValue/ZCL_Callback_PutAttributeValue
    \function to get/set the acutal value of the attribtue accordingly.
    This function updates the output parameter with the NVM index of the
    Cluster where the Attributes are stored. This function returns 0xFF (if
    NVM index not obtained). Otherwise it returns TRUE.
  Conditions:
    None
  Input:
    endpointId -                        Specifies the End Point for which
                                        the NVM index needs to be obtained.
    clusterId -                         Specifies the Cluster ID for which
                                        the NVM index needs to be obtained.
    pStorageIndex (Output parameter) -  It will be updated with storage
                                        Index value for a given End Point
                                        and a given Cluster
  Return:
    It returns either TRUE(clusterindex obtained from NVM) or 0xFF(Not
    obtained from NVM)
  Example:
    <code>
        
    </code>
  Remarks:
    None.                                                                                       
  ***********************************************************************************************/
BYTE ZCL_Callback_GetClusterIndex(BYTE endpointId,WORD clusterId,WORD* pStorageIndex);
#endif


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
void ZCL_Callback_Put_IdentifyTime( WORD identifyTime );

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
WORD ZCL_Callback_Get_IdentifyTime( void );


/************************************************************************************
 * Function:        void ZCL_Callback_HandleTimeAttributes(attributeId,Length,dataIndex)
 *
 * PreCondition:    None
 *
 * Input:           attributeId - attribute id of the particular time attribute
 *                  Length - Length of attribute data
 *                  dataIndex - pointer to the data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handles the different attributes of the time cluster
 *
 * Note:            None
 ***********************************************************************************/

void ZCL_Callback_HandleTimeAttributes( BYTE endPointId, WORD attributeId, BYTE Length,BYTE* dataIndex);
