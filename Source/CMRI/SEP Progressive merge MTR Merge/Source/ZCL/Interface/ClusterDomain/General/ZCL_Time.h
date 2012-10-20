/******************************************************************************
 *
 *                  ZCL Time
 *
 ******************************************************************************
 * FileName:        ZCL_Time.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v1.05 or higher
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
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                    26/03/09

 *****************************************************************************/

#ifndef _ZCL_TIME_H_
#define _ZCL_TIME_H_

/*****************************************************************************
  Includes
 *****************************************************************************/
/*None*/

/*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
/*Time cluster Identifier as defined by ZCL specification*/
#define ZCL_TIME_CLUSTER                            0x000a

/*Time Cluster Attributes*/
#define ZCL_TIME_ATTRIBUTE                          0x0000
#define ZCL_TIME_STATUS                             0x0001
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    #define ZCL_TIME_ZONE                           0x0002
    #define ZCL_TIME_DST_START                      0x0003
    #define ZCL_TIME_DST_END                        0x0004
    #define ZCL_TIME_DST_SHIFT                      0x0005
    #define ZCL_TIME_STANDARD_TIME                  0x0006
    #define ZCL_TIME_LOCAL_TIME                     0x0007
#endif /*I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES*/


/*Length for Time attribute data is 4 bytes*/
#define ZCL_TIME_LENGTH_OF_TIME_ATTRIBUTE_DATA      0x04

/*****************************************************************************
  Customizable Macros
 *****************************************************************************/
/* None */

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
 extern ROM ZCL_ClusterDefinition zcl_TimeClusterServerDefinition;
 extern ROM ZCL_ClusterDefinition zcl_TimeClusterClientDefinition;

 extern ROM ZCL_ClusterInfo zcl_TimeClusterServerInfo;
 extern ROM ZCL_ClusterInfo zcl_TimeClusterClientInfo;

/*****************************************************************************
  Data Structures
 *****************************************************************************/
/*Stucture for Time Cluster's Attributes*/
typedef struct __attribute__((packed,aligned(1))) _ZCL_TimeClusterAttribute
{
    DWORD timeAttribute;
    BYTE timeStatus;
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    SDWORD timeZone;    /*Signed 32 bit*/
    DWORD dstStart;
    DWORD dstEnd;
    SDWORD dstShift;    /*Signed 32 bit*/
    DWORD standardTime;
    DWORD localTime;
#endif /*I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES*/
} ZCL_TimeClusterAttribute;

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/
/******************************************************************************
 * Function:        ZCL_Time_SetTimeAttribute
 *
 * PreCondition:    None
 *
 * Input:           endPointId - endpoint id for which the command is received
 *                  pTime - time in seconds
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    void
 *
 * Overview:        This sets the time attribute of time cluster. The input
 *                  pointer holds the value in seconds. The endpoint is used to
 *                  fetch the storage index of NVM. This is then passed to
 *                  write the time attribute into NVM.
 *
 * Note:            This function doesnt set the value into the RTC. The
 *                  Application should call appropriate function to write time
 *                  into RTC.
 *****************************************************************************/
void ZCL_Time_SetTimeAttribute( BYTE endpointId, BYTE *pTime );


/******************************************************************************
 * Function:        ZCL_CheckTimeAttribute
 *
 * PreCondition:    None
 *
 * Input:           pReceivedAsdu - gives the poniter to the asdu of the
 *                  received packet.This is the read attribute response
 *                  asdu.
 *                  asduLength - gives the length of the read attribute response
 *                  payload.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    Returns pointer
 *
 * Overview:        This function parses the read attribute response and
 *                  checks if time attribute is received in the response
 *                  command. If time attribute is present and status is
 *                  successfull, then return the pointer from where the
 *                  time value starts. if time attribute is not present
 *                  then retutns NULL pointer.
 *
 * Note:
 *****************************************************************************/
BYTE* ZCL_CheckTimeAttribute( BYTE endPointId, BYTE* pReceivedAsdu, BYTE asduLength );

/******************************************************************************
 * Function:        BYTE ZCL_HandleTimeClusterCommand
 *                  (
 *                      BYTE endPointId,
 *                      BYTE asduLength,
 *                      BYTE* pReceivedAsdu,
 *                      BYTE* pResponseAsduStartLocation,
 *                      ZCL_ActionHandler* pActionHandler,
 *                      ROM ZCL_ClusterInfo* pZCLClusterInfo
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           endPointId - EndPoint on which the command is received
 *                  asduLength - length of the asdu
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and Payload.
 *                  pZCLClusterInfo - Pointer pointing to the Time Cluster info
 *
 * Output:          pResponseAsduStartLocation - Pointer pointing to the start
 *                                               location of asdu for which the
 *                                               response frame needs to be created
 *                  pActionHandler -   This is used to notify the application that a
 *                                     response frame is generated or not.
 *                                     It also holds the information of the type event
 *                                     to be handled by the application and has the
 *                                     payload of the recevied data
 *
 * Return :         Returns zero data since there is no commands in Time cluster
 *
 * Side Effects:    None
 *
 * Overview:        This function is called by the ZCL module if a cluster specific
 *                  command with Time cluster is received. There are no commands in
 *                  Time cluster. For error handling, this function is implemented.
 *                  This creates default response command with status Unsupported
 *                  cluster command.
 *
 * Note:            None
 *****************************************************************************/
BYTE ZCL_HandleTimeClusterCommand
(
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler *pActionHandler,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);
#endif /* _ZCL_TIME_H_ */
