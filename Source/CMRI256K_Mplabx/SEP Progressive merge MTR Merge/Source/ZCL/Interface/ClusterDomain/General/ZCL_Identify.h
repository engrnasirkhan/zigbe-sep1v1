/*********************************************************************
 *
 *                 ZCL Identify Cluster Header File
 *
 *********************************************************************
 * FileName:        ZCL_Identify.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v2.05 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright � 2004-2007 Microchip Technology Inc.  All rights reserved.
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
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY OF ANY
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
#ifndef _ZCL_IDENTIFY_H
#define _ZCL_IDENTIFY_H
/****************************************************************************
   Includes
 *****************************************************************************/
/* None */

/*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
/*Identify cluster Identifier as defined by ZCL specification*/
#define ZCL_IDENTIFY_CLUSTER                0x0003

/*Identify Cluster Attribute*/
#define ZCL_IDENTIFY_TIME_ATTRIBUTE         0x0000

/*This Macro defines the size of Identify Time Attribute*/
#define SIZE_OF_IDENTIFY_TIME_ATTRIBUTE     0x02

/*Events handled by the application*/
enum Identify_Cluster_Events
{
    ZCL_Identify_IdentifyEvent              = 0x20,
    ZCL_IdentifyQueryEvent                  = 0x21,
    ZCL_Identify_Query_Response_Event       = 0x22
};

/*Commands generated by Client Side of the IdentifyCluster*/
enum Identify_Generated_Commands
{
    ZCL_IdentifyCmd                         = 0x00,
    ZCL_IdentifyQueryCmd                    = 0x01
};
/*Commands Received by Client Side of the IdentifyCluster*/
enum Identify_Received_Commands
{
    ZCL_IdentifyQueryResponseCmd            = 0x00
};

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
/*Stucture for Identify Cluster's Attributes*/
typedef struct _ZCL_IdentifyClusterAttributes
{
    WORD identifyTime;
}ZCL_IdentifyClusterAttributes;

typedef struct _IdentifyCommand
{
    WORD identifyTime;
}IdentifyCommand;
/*****************************************************************************
   Variable definitions
 *****************************************************************************/
extern ROM ZCL_ClusterDefinition zcl_IdentifyClusterServerDefinition;
extern ROM ZCL_ClusterDefinition zcl_IdentifyClusterClientDefinition;

extern ROM ZCL_ClusterInfo zcl_IdentifyClusterServerInfo;
extern ROM ZCL_ClusterInfo zcl_IdentifyClusterClientInfo;

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/

/******************************************************************************
 * Function:        BYTE ZCL_HandleIdentifyClusterCommand
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
 * Input:           endPointId  - EndPoint on which the IdentifyCluster command
 *                                 needs to be processed.
 *                  asduLength - Asdu Length of the recevied frame.
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and Payload.
 *                  pZCLClusterInfo - Pointer pointing to the IdentifyClusterinfo
 *
 * Output:          pResponseAsduStartLocation - Pointer pointing to the start
 *                                               location of asdu for which the
 *                                               response frame needs to be
 *                                               created
*                  pActionHandler -  This is used to notify the application
*                                    that a response frame is generated or not.
*                                    It also holds the information of the type
*                                    event to be handled by the application
*                                    and  has the  payload of the recevied
*                                    data.
 * Return :         Returns the length of the response frame created
 * Side Effects:    None
 *
 * Overview:        This function is called when a Identify Cluster command
 *                  is received.
 *                  This function process the Identify Cluster commands and
 *                  notifies the application about the action to be taken.
 * Note:            None
 *****************************************************************************/
BYTE ZCL_HandleIdentifyClusterCommand
(
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler *pActionHandler,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);

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
BYTE ZCL_Callback_IsDeviceInIdentifyMode( void );

#endif      /*_ZCL_IDENTIFY_H*/
