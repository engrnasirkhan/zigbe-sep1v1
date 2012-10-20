/*********************************************************************
 *
 *                  ZCL Commissioning Header File
 *
 *********************************************************************
 * FileName        : ZCL_Commissioning.h
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

 ********************************************************************/
#ifndef _ZCL_COMMISSIONING_H
#define _ZCL_COMMISSIONING_H

/*****************************************************************************
                                Includes
 *****************************************************************************/
 /* None */

/*****************************************************************************
                          Constants and Enumerations
 *****************************************************************************/
#define ZCL_COMMISSIONING_CLUSTER                           0x0015

/* Commssioning Cluster Attribute Set information */
#define ZCL_COMMISSIONING_SHORT_ADDRESS                     0x0000
#define ZCL_COMMISSIONING_EXTENDED_PANID                    0x0001
#define ZCL_COMMISSIONING_PANID                             0x0002
#define ZCL_COMMISSIONING_CHANNEL_MASK                      0x0003
#define ZCL_COMMISSIONING_PROTOCOL_VERSION                  0x0004
#define ZCL_COMMISSIONING_STACK_PROFILE                     0x0005
#define ZCL_COMMISSIONING_STARTUP_CONTROL                   0x0006

#define ZCL_COMMISSIONING_TRUST_CENTER_ADDRESS              0x0010
#define ZCL_COMMISSIONING_TRUST_CENTER_MASTER_KEY           0x0011
#define ZCL_COMMISSIONING_NETWORK_KEY                       0x0012
#define ZCL_COMMISSIONING_USE_INSECURE_JOIN                 0x0013
#define ZCL_COMMISSIONING_PRE_CONFIGURED_LINK_KEY           0x0014
#define ZCL_COMMISSIONING_NETWORK_KEY_SEQ_NUM               0x0015
#define ZCL_COMMISSIONING_NETWORK_KEY_TYPE                  0x0016
#define ZCL_COMMISSIONING_NETWORK_MANAGER_ADDRESS           0x0017

#define ZCL_COMMISSIONING_SCAN_ATTEMPTS                     0x0020
#define ZCL_COMMISSIONING_TIME_BETWEEN_SCANS                0x0021
#define ZCL_COMMISSIONING_REJOIN_INTERVAL                   0x0022
#define ZCL_COMMISSIONING_MAX_REJOIN_INTERVAL               0x0023

#define ZCL_COMMISSIONING_INDIRECT_POLL_RATE                0x0030
#define ZCL_COMMISSIONING_PARENT_RETRY_THRESHOLD            0x0031

#define ZCL_COMMISSIONING_CONCENTRATOR_FLAG                 0x0040
#define ZCL_COMMISSIONING_CONCENTRATOR_RADIUS               0x0041
#define ZCL_COMMISSIONING_CONCENTRATOR_DISCOVERY_TIME       0x0042

/*****************************************************************************
                                Data Types
*****************************************************************************/
#define DATA_TYPE_32_BITMAP									0x1B
#define DATA_TYPE_8_ENUMERATION								0x30
#define DATA_TYPE_IEEE_ADDRESS								0xF0
#define DATA_TYPE_128_SECURITY_KEY							0xF1
 
/*****************************************************************************
                                Startup Control Attributes
*****************************************************************************/
#define	STARTUP_CONTROL_PART_OF_NETWORK_NO_EXPLICIT_ACTION	0x00
#define	STARTUP_CONTROL_FORM_NEW_NETWORK					0x01
#define	STARTUP_CONTROL_JOIN_NEW_NETWORK					0x02
#define	STARTUP_CONTROL_START_FROM_SCRATCH_AS_ROUTER		0x03

enum Commissioning_Cluster_Request_Commands
{
    ZCL_CommissioningRestartDevice                          = 0x00,
    ZCL_CommissioningSaveStartupParameters                  = 0x01,
    ZCL_CommissioningRestoreStartupParameters               = 0x02,
    ZCL_CommissioningResetStartupParameters                 = 0x03
};
enum Commissioning_Cluster_Response_Commands
{
    ZCL_CommissioningRestartDeviceResponse                  = 0x00,
    ZCL_CommissioningSaveStartupParametersResponse          = 0x01,
    ZCL_CommissioningRestoreStartupParametersResponse       = 0x02,
    ZCL_CommissioningResetStartupParametersResponse         = 0x03,
};

enum Commissioning_Cluster_Events
{
    ZCL_CommissioningRestartDeviceEvent                     = 0x40,
    ZCL_CommissioningSaveStartupParametersEvent             = 0x41,
    ZCL_CommissioningRestoreStartupParametersEvent          = 0x42,
    ZCL_CommissioningResetStartupParametersEvent            = 0x43,
    ZCL_CommissioningRestartDeviceResponseEvent             = 0x44,
    ZCL_CommissioningSaveStartupParametersResponseEvent     = 0x45,
    ZCL_CommissioningRestoreStartupParametersResponseEvent  = 0x46,
    ZCL_CommissioningResetStartupParametersResponseEvent    = 0x47
};

enum Commissioning_reset_types
{
    Reset_Current                                           = 0x01,
    Reset_All                                               = 0x02,
    Erase_Index                                             = 0x04
};

#define COMM_RESP_STATUS_INDEX                              0x03

/*****************************************************************************
                          Customizable Macros
 *****************************************************************************/
 /* None */

/*****************************************************************************
                            Data Structures
 *****************************************************************************/
typedef struct __attribute__((packed,aligned(1))) _Commissioning_Cluster_Attributes
{
    WORD        shortAddr;
    BYTE        ExtendedPANId[8];
    WORD        PANId;
    DWORD       ChannelMask;
    BYTE        ProtocolVersion;
    BYTE        StackProfile;
    BYTE        StartupControl;

    BYTE        TrustCenterAddress[8];
    BYTE        TrustCenterMasterKey[16];
    BYTE        NetworkKey[16];
    BOOL        UseInsecureJoin;
    BYTE        PreconfiguredLinkKey[16];
    BYTE        NetworkKeySeqNum;
    BYTE        NetworkKeyType;
    WORD        NetworkManagerAddress;

    BYTE        ScanAttempts;
    WORD        TimeBetweenScans;
    WORD        RejoinInterval;
    WORD        MaxRejoinInterval;

    WORD        IndirectPollRate;
    BYTE        ParentRetryThreshold;

    BYTE        ConcentratorFlag;
    BYTE        ConcentratorRadius;
    BYTE        ConcentratorDiscoveryTime;

} Commissioning_Cluster_Attributes;

typedef struct __attribute__((packed,aligned(1))) _ZCL_CommissioningRestartDeviceCmdPayload
{
    BYTE Options;
    BYTE Delay;
    BYTE Jitter;
}ZCL_CommissioningRestartDeviceCmdPayload;

typedef struct __attribute__((packed,aligned(1))) _ZCL_CommissioningSaveStartupParamsCmdPayload
{
    BYTE Options;
    BYTE Index;
} ZCL_CommissioningSaveStartupParamsCmdPayload;

typedef struct __attribute__((packed,aligned(1))) _ZCL_CommissioningRestoreStartupParamsCmdPayload
{
    BYTE Options;
    BYTE Index;
} ZCL_CommissioningRestoreStartupParamsCmdPayload;

typedef struct __attribute__((packed,aligned(1))) _ZCL_CommissioningResetStartupParamsCmdPayload
{
    BYTE Options;
    BYTE Index;
} ZCL_CommissioningResetStartupParamsCmdPayload;

typedef struct __attribute__((packed,aligned(1))) _ZCL_CommissioningResponseCmdPayload
{
    BYTE Status;
} ZCL_CommissioningResponseCmdPayload;

/*****************************************************************************
                          Variable definitions
 *****************************************************************************/
extern ROM ZCL_ClusterDefinition zcl_CommissioningClusterServerDefinition;
extern ROM ZCL_ClusterDefinition zcl_CommissioningClusterClientDefinition;

extern ROM ZCL_ClusterInfo zcl_CommissioningClusterServerInfo;
extern ROM ZCL_ClusterInfo zcl_CommissioningClusterClientInfo;

/*****************************************************************************
                          Function Prototypes
 *****************************************************************************/

 /******************************************************************************
 * Function:        BYTE ZCL_HandleCommissioningClusterCommand
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
 * Input:           endPointId -    EndPoint on which the Commissioning Cluster command needs
 *                                  to be processed
 *                  asduLength -    Asdu Length of the recevied frame
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and Payload.
 *                  pZCLClusterInfo - Pointer pointing to the Commissioning Cluster info
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
 * Overview:        This function is called when a Commissioning Cluster command is received.
 *                  This function process the Commissioning Cluster command and notifies the
 *                  application about the action to be taken.
 *
 * Note:            None
 *
 *****************************************************************************/
BYTE ZCL_HandleCommissioningClusterCommand
(
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler *pActionHandler,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);

#endif /*_ZCL_COMMISSIONING_H*/
