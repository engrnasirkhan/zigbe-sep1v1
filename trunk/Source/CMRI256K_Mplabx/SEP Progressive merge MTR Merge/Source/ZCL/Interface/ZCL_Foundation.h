/*********************************************************************
 *
 *                 ZCL Foundation Header File
 *
 *********************************************************************
 * FileName:        ZCL_Foundation.h
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
 * Author               Date            Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      23/03/09
 *****************************************************************************/
#ifndef _ZCL_FOUNDATION_H
#define _ZCL_FOUNDATION_H

/****************************************************************************
   Includes
 *****************************************************************************/
 #include <string.h>

 /*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
/*The following Macros values are defined by consider Manufacturer
  bit disabled  */
/*This Indicates Transaction sequence Number Position in ZCL Frame */
#define ZCL_FRAME_TSN_POSITION_MANUFACTURE_BIT_DISABLED                  0x01
/*This Indicates Command Id Position in ZCL Frame */
#define ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_DISABLED            0x02
#define ZCL_FRAME_COMMANDID_POSITION_MANUFACTURE_BIT_ENABLED            0x04

/*This Indicates Default Resp CommandId Position  */
#define ZCL_DEFAULT_RESP_CID_POSITION_MANUFACTURE_BIT_DISABLED           0x03
/*This Indicates Default Resp Status Position  */
#define ZCL_DEFAULT_RESP_STATUS_POSITION_MANUFACTURE_BIT_DISABLED        0x04



/*This Indicates the ZCL command is sent from server to client direction*/
#define ZCL_SERVER_TO_CLIENT_DIRECTION                                   0x08
/*This Indicates the ZCL command is sent from client to server direction*/
#define ZCL_CLIENT_TO_SERVER_DIRECTION                                   0x00
/*This Indicates the Default response not required for a ZCL Command*/
#define ZCL_DEFAULT_RESPONSE_NOT_REQUIRED                                0x10
/*This Indicates the ZCL cluster specific commandid value*/
#define ZCL_CLUSTER_SPECIFIC_COMMAND                                     0x01
/*Length of the attribute id*/
#define ZCL_ATTRIBUTE_ID_LENGTH                                          0x02
/*Length of the cluster id*/
#define ZCL_CLUSTER_ID_LENGTH                                            0x02
/*Length of the data is zero*/
#define ZERO_DATA                                                        0x00
/*Length of manufacturer code*/
#define LENGTH_OF_MANUFACTURER_CODE                                      0x02

/*minimum reporting interval*/
#define MIN_REPORTING_INTERVAL                                           0x0001

#define ZCL_CheckAttributeAnalogDataType( datatype )     ( ( ( datatype >= 0x20 ) && ( datatype <= 0x2F ) ) || \
            ( ( datatype >= 0x38 ) && ( datatype <= 0x3a ) ) || ( ( datatype >= 0xe0 ) && \
            ( datatype <= 0xe2 ) ) )
/*****************************************************************************/
 typedef enum genericCommandEvents
 {
     READ_ATTRIBUTE_COMMAND_EVENT                                       = 0x00,
     READ_ATTRIBUTE_RESPONSE_COMMAND_EVENT                              = 0x01,
     WRITE_ATTRIBUTE_COMMAND_EVENT                                      = 0x02,
     WRITE_ATTRIBUTE_UNDIVIDED_COMMAND_EVENT                            = 0x03,
     WRITE_ATTRIBUTE_RESPONSE_COMMAND_EVENT                             = 0x04,
     WRITE_ATTRIBUTE_NO_RESPONSE_COMMAND_EVENT                          = 0x05,
     CONFIGURE_REPORTING_COMMAND_EVENT                                  = 0x06,
     CONFGIURE_REPORTING_RESPONSE_COMMAND_EVENT                         = 0x07,
     READ_REPORTING_CONFIGURATION_COMMAND_EVENT                         = 0x08,
     READ_REPORTING_CONFIGURATION_RESPONSE_COMMAND_EVENT                = 0x09,
     REPORT_ATTRIBUTE_COMMAND_EVENT                                     = 0x0a,
     DEFAULT_RESPONSE_COMMAND_EVENT                                     = 0x0b,
     DISCOVER_ATTRIBUTE_COMMAND_EVENT                                   = 0x0c,
     DISCOVER_ATTRIBUTE_RESPONSE_COMMAND_EVENT                          = 0x0d
 }genericCommandEvents;

 /*enumerations for discovery response frame*/
 typedef enum discoveryComplete
 {
     ZCL_DiscoveryInComplete                                            = 0x00,
     ZCL_DiscoveryComplete                                              = 0x01
 } discoveryComplete;

 /*enumerations for direction fields in reporting */
 typedef enum reportingDirection
 {
    ReportingDirectionServerToClient                                    = 0x00,
    ReportingDirectionClientToServer                                    = 0x01
 }reportingDirection;
/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
  /* None */

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
   /* None */

/*****************************************************************************
   Data Structures
 *****************************************************************************/
 /*Structure Name:s ZCL_ReadAttributeCommand
 Field:
 aAttributeId - this is a place holder for list of attribute ids of
            type WORD.
 Read attribute command payload consist of lists of attribute ids.*/
 typedef struct ZCL_ReadAttributeCommand
 {
     WORD aAttributeId[1];
 }ZCL_ReadAttributeCommand;


/*Structure Name:s ZCL_ReadAttributeResponseCommand
Fields:
attributeId: This indicates the attribute id that is read
attributeDataType: attribute data type of the above attribute
aAttributeData: This is of variable length. This is dependent on the type of
            attribute read.
Note: Read attribute response command will have a list of such records. This is
only one read attribute record.*/
typedef struct ZCL_ReadAttributeResponseCommand
{
    WORD attributeId;
    BYTE attributeDataType;
    BYTE aAttributeData[1];
}ZCL_ReadAttributeResponseCommand;


/*Structure Name:s ZCL_WriteAttributeCommand
Fields:
attributeId: This indicates the attribute id that is to be written
attributeDataType: attribute data type of the above attribute
aAttributeData: This is of variable length. This is dependent on the type of
            attribute.
Note:
1. This is for one write attribute record. The above format should be repeated
    for multiple attributes
2. The same structure should be used for Write attribute undivided, Write attribute no
response command.Only the command id in the ZCL header will vary, rest all the fields
in the Header and the payload are the same. */
typedef struct ZCL_WriteAttributeCommand
{
    WORD attributeId;
    BYTE attributeDataType;
    BYTE aAttributeData[1];
}ZCL_WriteAttributeCommand;

/*Structure Name: ZCL_WriteAttributeResponseCommand
Fields:
    status - indicates the status of the write attribute.
                example: 0x00  success , 0x8d - invalid data type
    attributeId - indicates the attribute for which the above status
                is given. This field is not present if the status is success.

Note:
1. This is for one write attribute record. The above format should be repeated
    for multiple attributes
2. If all the attributes are written successfully, then only one byte of status
    is received in the write attribute response command
3. Write attribute response command is sent in response to write attribute command
    and write attribute undivided command.
4. There is no response for Write attribute no response command */
typedef struct ZCL_WriteAttributeResponseCommand
{
    BYTE status;
    WORD attributeId;
}ZCL_WriteAttributeResponseCommand;


/*Structure Name: ZCL_DiscoverAttributeRequest
Fields:
startAttributeId: This indicates the start attribute id for discovering
                the attributes
maximumCount: This gives the maximum count of attributes to be discovered.
*/
typedef struct ZCL_DiscoverAttributeRequest
{
    WORD startAttributeId;
    BYTE maximumCount;
}ZCL_DiscoverAttributeRequest;


/*Structure Name:s ZCL_DiscoverAttributeResponse
Fields:
discoveryComplete - This indicates if the discovery is complete or not.
                    0x00 - discovery complete
                    0x01 - discovery incomplete
attributeId - attribute id that is discovered
attributeDataType - attribute data type of the corresponding attribute
Note:
Attribute id and attribute data type is for a single discover attribute record.
This will be repeated for as many number of records discovered. The number of
records are based on the number of attributes supported within the cluster
and the maximum count given in the discover attribute request command
    */
typedef struct ZCL_DiscoverAttributeResponse
{
    BYTE discoveryComplete;
    WORD attributeId;
    BYTE attributeDataType;
}ZCL_DiscoverAttributeResponse;


/*Structure Name: SendReportConfiguration
Fields: This is used for configuraing a device to send reports
attributeDatatype - data type of the attribute
minimumReportingInterval - minimum time between two report attribute commands.
maximumReportingInterval - maximum time between two report attribute commands.
aReportableChange - reportable change - value change in the attribute data
    */
typedef struct SendReportConfiguration
{
    BYTE attributeDatatype;
    WORD minimumReportingInterval;
    WORD maximumReportingInterval;
    BYTE aReportableChange[1];
}SendReportConfiguration;

/*Structure Name: ReceiveReportConfiguration
Fields: This is used for configuraing a device to receive reports
timeout - timeout for receiving the report attribute
*/
typedef struct ReceiveReportConfiguration
{
    WORD timeOut;
}ReceiveReportConfiguration;


/*Structure Name:ZCL_ConfigureReportCommand
Fields:
direction - indicates the direction - send reports / receive reports
attributeId - attribute that is to be reported or receive reports
*/
typedef struct ZCL_ConfigureReportCommand
{
    BYTE direction;
    WORD attributeId;
    union
    {
        SendReportConfiguration sendReport;
        ReceiveReportConfiguration receiveReport;
    }ReportConfiguration;

}ZCL_ConfigureReportCommand;


/*Structure Name:ZCL_ConfigureReportingResponseCommand
This structure is used to parse configure reporting response command
Fields:
status - indicates success or any other status value
attributeId - this field is present only if the status is other than success
*/
typedef struct ZCL_ConfigureReportingResponseCommand
{
    BYTE status;
    WORD attributeId;
}ZCL_ConfigureReportingResponseCommand;

/*Structure Name:ZCL_ReadReportingCommand
This structure is used to construct the Read Report configuration command
Fields:
direction - indicates the direction - send reports / receive reports
attributeId - attribute that is to be reported or receive reports
*/
typedef struct ZCL_ReadReportingCommand
{
    BYTE direction;
    WORD attributeId;
}ZCL_ReadReportingCommand;


/*Structure Name:ZCL_ReadReportingResponseCommand
This structure is used to parse Read reporting configuration response command
Fields:
status - indicates success or any other status value
attributeId - this field is present only if the status is other than success
*/
typedef struct ZCL_ReadReportingResponseCommand
{
    BYTE status;
    WORD attributeId;
}ZCL_ReadReportingResponseCommand;

/*Structure Name:ZCL_ReportAttributeCommand
Fields:
attributeId: This indicates the attribute id that is reported
attributeDataType: attribute data type of the above attribute
aAttributeData: This is of variable length. This is dependent on the type of
            attribute.
Note:
This is for one report attribute record. The above format should be repeated
    for multiple attributes
*/
typedef struct ZCL_ReportAttributeCommand
{
    WORD attributeId;
    BYTE attributeDataType;
    BYTE attributeData[1];
}ZCL_ReportAttributeCommand;

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/

/******************************************************************************
 * Function:        BYTE ZCL_CreateDefaultResponseCommand
 *                  (
 *                      BYTE* pReceivedAsdu,
 *                      BYTE* pResponseAsduStartLocation,
 *                      BYTE status
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           pApsPayload - This point to the actual data received in the
 *                                APSDE-Data Indication.This consists of ZCL
 *                                header and Payload.
 *                  status      - This field indicates the status field of Default
 *                                response command
 *
 * Output:          pResponsePayload - This is the location where the Default
 *                                     response will be created.
 *
 *
 * Return :         It returns the length of Default response frame.
 *
 * Side Effects:    None
 *
 * Overview:        This function is called by ZCL or Application when default
 *                  response needs to be created.. It takes the command id and
 *                  the status parameter passed to create the payload for
 *                  default response command. The Default response command is
 *                  created in the output parameter pResponsePayload.
 *
 * Note:            None
 *****************************************************************************/
 BYTE ZCL_CreateDefaultResponseCommand
 (
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    BYTE status
 );


 /******************************************************************************
 *  Function:       BYTE GetAttributeDetails
 *                  (
 *                      BYTE numberOfAttributes,
 *                      WORD attributeId,
 *                      ZCL_AttributeInfo *pZCL_AttributeInfo,
 *                      BYTE* pAttributeOffset
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           numberOfAttributes - No.of attributes in ZCL_AttributeInfo
 *                  attributeId        - We need to get the attribute
 *                                       details for this attributeId.
 *                  pZCL_AttributeInfo - List of Attributes
 *
 * Output:          pAttributeOffset - It indicates the offset from the First
 *                                     attribute.
 *
 * Return:          It returns the size of the attribute.
 *
 * Side Effects:    None
 *
 * Overview: This function is used to get the attribute offset value from start
 *           of the attribute and also returns the size of attribute.This details
 *           are required for storing attribute values into NVM.
 *
 * Note:            None
 *****************************************************************************/
BYTE GetAttributeDetails
(
     BYTE numberOfAttributes,
     WORD attributeId,
     ZCL_AttributeInfo *pZCL_AttributeInfo,
     BYTE* pAttributeOffset
);

/******************************************************************************
 *  Function:       BYTE ZCL_CreateReadAttributeRespCmd
 *                  (
 *                      BYTE endPointId,
 *                      BYTE asduLength,
 *                      BYTE* pReceivedAsu,
 *                      BYTE* pResponseAsduStartLocation,
 *                      ROM ZCL_ClusterInfo *pZCLCClusterInfo
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           endPointId -
 *                  pReceivedAsu -
 *
 * Output:          pResponseAsduStartLocation -
 *                  pZCLClusterDefinition -
 * Return:
 *
 * Side Effects:    None
 *
 * Overview:
 *
 * Note:            None
 *****************************************************************************/
 BYTE ZCL_CreateReadAttributeRespCmd
 (
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
 );

 /******************************************************************************
 *  Function:       BYTE ZCL_CreateWriteAttributeRespCmd
 *                  (
 *                      BYTE endPointId,
 *                      BYTE asduLength,
 *                      BYTE* pReceivedAsu,
 *                      BYTE* pResponseAsduStartLocation,
 *                      ROM ZCL_ClusterInfo *pZCLCClusterInfo
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           endPointId -
 *                  pApsPayload -
 *
 * Output:          pResponsePayload -
 *                  pZCLClusterDefinition -
 * Return:
 *
 * Side Effects:    None
 *
 * Overview:
 *
 * Note:            None
 *****************************************************************************/
 BYTE ZCL_CreateWriteAttributeRespCmd
 (
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsu,
    BYTE* pResponseAsduStartLocation,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
 );

 /******************************************************************************
 * Function:        BYTE ZCL_CreateConfigureReportingResponseCommand
 *                  (
 *                  BYTE endPointId,
 *                  BYTE asduLength,
 *                  BYTE* pReceivedAsu,
 *                  BYTE* pResponseAsduStartLocation,
 *                  ROM ZCL_ClusterInfo *pZCLClusterInfo
 *                  )
 *
 * PreCondition:    None
 *
 * Input:           endPoint - End Point for which configure report command
 *                  is received.
 *                  asduLength - length of confgiure report command
 *                  pReceivedAsu - pointer to the received configure report
 *                  attribute command.
 *                  pZCLClusterInfo - Cluster info for the received cluster
 *
 *
 * Output:          pResponseAsduStartLocation - pointer to configure
 *                  attribute response command which will be created.
 *
 * Return:          returns the length of the configure report attribute response
 *                  comand
 *
 * Side Effects:    None
 *
 * Overview:        This function validates the configure report command and
 *                  creates the configure report response command.
 *
 * Note:            None
 *****************************************************************************/
 BYTE ZCL_CreateConfigureReportingResponseCommand
 (
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsu,
    BYTE* pResponseAsduStartLocation,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
 );

/******************************************************************************
 *  Function:       BYTE ZCL_CreateReadReportingConfigurationResponseCommand
 *                  (
 *                      BYTE endPointId,
 *                      BYTE asduLength,
 *                      BYTE* pReceivedAsu,
 *                      BYTE* pResponseAsduStartLocation,
 *                      ROM ZCL_ClusterInfo *pZCLCClusterInfo
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           endPoint - End Point for which configure report command
 *                  is received.
 *                  asduLength - length of confgiure report command
 *                  pReceivedAsu - pointer to the received configure report
 *                  attribute command.
 *                  pZCLClusterInfo - Cluster info for the received cluster
 * Return:          returns the length of the configure report attribute response
 *                  comand
 *
 * Side Effects:    None
 *
 * Overview:
 *
 * Note:            None
 *****************************************************************************/
 BYTE ZCL_CreateReadReportingConfigurationResponseCommand
 (
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsu,
    BYTE* pResponseAsduStartLocation,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
 );

 /******************************************************************************
* Function:         BYTE ZCL_CreateDiscoverAttributesResponseCommand
*                   (
*                       BYTE endPointId,
*                       BYTE asduLength,
*                       BYTE* pReceivedAsdu,
*                       BYTE* pResponseAsduStartLocation,
*                       ROM ZCL_ClusterInfo *pZCLClusterInfo
*                   );
*
* PreCondition:    pResponseAsduStartLocation should be allocated dynamically
*                   or statically since error handling is not done in this
*                   function.
* Input:
*                   endPointId : endpoint for which the discovery request
*                                frame is received
*                   asduLength : length of the discovery request frame
*
*                   pReceivedAsdu: pointer to discovery request frame
*                   pZCLClusterInfo: pointer to the cluster info of
*                       the received cluster.
* Output:           pResponseAsduStartLocation: points to the start location
*                                              of response asdu.
*                                              Discovery response will be
*                                              created in this location.
* Side Effects:    None
*
* Overview:        This function is called when attributes within a cluster
*                  is to be discovered. This function loops through the
*                  supported attributes within the cluster. It creates
*                  discovery attribute response based on the start attribute
*                  and maximum attribute received in the request frame.
*
* Note:            None
*****************************************************************************/
 BYTE ZCL_CreateDiscoverAttributesResponseCommand
 (
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsu,
    BYTE* pResponseAsduStartLocation,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
 );

#endif /*_ZCL_FOUNDATION_H*/

