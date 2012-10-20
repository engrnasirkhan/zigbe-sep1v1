/*********************************************************************
 *
 *                  ZCL Basic Header File
 *
 *********************************************************************
 * FileName        : ZCL_Basic.h
 * Dependencies    :
 * Processor       : PIC18 / PIC24 / PIC32
 * Complier        : MCC18 v3.00 or higher
 *                   MCC30 v2.05 or higher
 *                   MCC32 v1.05 or higher
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
#ifndef _ZCL_BASIC_H_
#define _ZCL_BASIC_H_

/*****************************************************************************
                                Includes
 *****************************************************************************/
 /* None */

/*****************************************************************************
                          Constants and Enumerations
 *****************************************************************************/
#define ZCL_BASIC_CLUSTER                                   0x0000

/* Basic Device Information Attribute Set */
#define ZCL_BASIC_VERSION_ATTRIBUTE                         0x0000
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_BASIC_APPLICATION_VERSION_ATTRIBUTE         0x0001
    #define ZCL_BASIC_STACK_VERSION_ATTRIBUTE               0x0002
    #define ZCL_BASIC_HW_VERSION_ATTRIBUTE                  0x0003
    #define ZCL_BASIC_MANUFACTURER_NAME_ATTRIBUTE           0x0004
    #define ZCL_BASIC_MODEL_IDENTIFIER_ATTRIBUTE            0x0005
    #define ZCL_BASIC_DATE_CODE_ATTRIBUTE                   0x0006
#endif
#define ZCL_BASIC_POWER_SOURCE_ATTRIBUTE                    0x0007

/* Basic Device Settings Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_BASIC_LOCATION_DESCRIPTION_ATTRIBUTE        0x0010
    #define ZCL_BASIC_PHYSICAL_ENVIRONMENT_ATTRIBUTE        0x0011
    #define ZCL_BASIC_DEVICE_ENABLED_ATTRIBUTE              0x0012
    #define ZCL_BASIC_ALARM_MASK_ATTRIBUTE                  0x0013
#endif

enum ZCL_BasicClusterCommands
{
    ZCL_BasicResetToFactoryDefaults     = 0x00
};

enum ZCL_BasicClusterEvents
{
  ZCL_BasicResetToFactoryDefaultsEvent  = 0x10
};

/*****************************************************************************
                          Customizable Macros
 *****************************************************************************/
 /* None */

 /*****************************************************************************
                          Variable definitions
 *****************************************************************************/
extern ROM ZCL_ClusterDefinition zcl_BasicClusterServerDefinition;
extern ROM ZCL_ClusterDefinition zcl_BasicClusterClientDefinition;

extern ROM ZCL_ClusterInfo zcl_BasicClusterServerInfo;
extern ROM ZCL_ClusterInfo zcl_BasicClusterClientInfo;

/*****************************************************************************
                            Data Structures
 *****************************************************************************/
 /* Structure for holding basic cluster attributes */
typedef struct _ZCL_BasicClusterAttributes
{
    BYTE zclVesion;

#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    BYTE appVersion;
    BYTE stackVersion;
    BYTE HWVesion;
    BYTE manufacturerName[16];
    BYTE modelIdentifier[16];
    BYTE dateCode[16];
#endif

    BYTE powerSource;

#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    BYTE locationDescription[16];
    BYTE physicalEnvironment;
    BOOL deviceEnabled;
    BYTE alarmMask;
#endif
} ZCL_BasicClusterAttributes;

/*****************************************************************************
                          Function Prototypes
 *****************************************************************************/

/******************************************************************************
 * Function:        BYTE ZCL_HandleBasicClusterCommand
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
 * Overview:        This function is called when a Basic Cluster command is received.
 *                  This function process the Basic Cluster command and notifies the
 *                  application about the action to be taken.
 *
 * Note:            None
 *
 *****************************************************************************/
BYTE ZCL_HandleBasicClusterCommand
(
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler *pActionHandler,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);
#endif /* _ZCL_BASIC_H_ */
