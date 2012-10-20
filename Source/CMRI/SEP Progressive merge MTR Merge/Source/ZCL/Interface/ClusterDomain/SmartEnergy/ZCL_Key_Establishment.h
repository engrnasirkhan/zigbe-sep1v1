/*********************************************************************
 *
 *                  ZCL KeyEstablishment Header File
 *
 *********************************************************************
 * FileName        : ZCL_Key_Establishment.h
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
#ifndef _ZCL_KEY_ESTABLISHMENT_H
#define _ZCL_KEY_ESTABLISHMENT_H

#if I_SUPPORT_CBKE == 1
/*****************************************************************************
                                Includes
 *****************************************************************************/
 /* None */

/*****************************************************************************
                          Constants and Enumerations
 *****************************************************************************/
/*Key establishment cluster*/
#define ZCL_KEY_ESTABLISHMENT_CLUSTER           0x0800
/*kesy establishment suite*/
#define ZCL_KEY_ESTABLISHMENT_SUITE             0x0000


/*enumerations for key establishment server commands*/
typedef enum keyEstServerCommands
{
    ZCL_KeyEstInitiateKeyEstReq                 = 0x00,
    ZCL_KeyEstEphemeralDataReq                  = 0x01,
    ZCL_KeyEstConfirmKeyDataReq                 = 0x02,
    ZCL_KeyEstTerminateKeyEst                   = 0x03
}keyEstServerCommands;

/*enumerations for key establishment client commands*/
typedef enum keyEstClientCommands
{
    ZCL_KeyEstInitiateKeyEstResp                = 0x00,
    ZCL_KeyEstEphemeralDataResp                 = 0x01,
    ZCL_KeyEstConfirmKeyDataResp                = 0x02
}keyEstClientCommands;


/*enumerations for key establishment events*/
typedef enum keyEstEvents
{
    ZCL_KeyEstInitiateKeyEstReqEvent            = 0x90,
    ZCL_KeyEstEphemeralDataReqEvent             = 0x91,
    ZCL_KeyEstConfirmKeyDataReqEvent            = 0x92,
    ZCL_KeyEstTerminateKeyEstEvent              = 0x93,
    ZCL_KeyEstInitiateKeyEstRespEvent           = 0x94,
    ZCL_KeyEstEphemeralDataRespEvent            = 0x95,
    ZCL_KeyEstConfirmKeyDataRespEvent           = 0x96
}keyEstEvents;

typedef enum keyEstTerminateStatusValues
{
    ZCL_KeyEstTerminateReserved                 = 0x00,
    ZCL_KeyEstTerminateUnknownIssuer            = 0x01,
    ZCL_KeyEstTerminateBad_Key_Confirm          = 0x02,
    ZCL_KeyEstTerminateBadMessage               = 0x03,
    ZCL_KeyEstTerminateNoResources              = 0x04,
    ZCL_KeyEstTerminateUnSupportedSuite         = 0x05,
}keyEstTerminateStatusValues;
/*****************************************************************************
                          Customizable Macros
 *****************************************************************************/
 /* None */

 /*****************************************************************************
                          Variable definitions
 *****************************************************************************/
extern ROM ZCL_ClusterDefinition zcl_KeyEstClusterServerDefinition;
extern ROM ZCL_ClusterDefinition zcl_KeyEstClusterClientDefinition;

extern ROM ZCL_ClusterInfo zcl_KeyEstClusterServerInfo;
extern ROM ZCL_ClusterInfo zcl_KeyEstClusterClientInfo;

/*****************************************************************************
                            Data Structures
 *****************************************************************************/
/*Stucture for Key Establishment Cluster's Attributes*/
typedef struct _ZCL_KeyEstClusterAttributes
{
    WORD keyEstSuite;
}_ZCL_KeyEstClusterAttributes;


/*structure for Initiate Key establishment request command*/
typedef struct __attribute__((packed,aligned(1))) ZCL_KeyEstInitiateKeyReqCommand
{
    WORD keyEstSuite;
    BYTE ephemeralDataGenerateTime;
    BYTE confirmKeyGenerateTime;
    BYTE certificate[48]; /*48 bytes of certificate.It is fixed size*/
}ZCL_KeyEstInitiateKeyReqCommand;


/*structure for Initiate Key establishment response command*/
typedef struct __attribute__((packed,aligned(1))) ZCL_KeyEstInitiateKeyRespCommand
{
    WORD keyEstSuite;
    BYTE ephemeralDataGenerateTime;
    BYTE confirmKeyGenerateTime;
    BYTE certificate[48]; /*48 bytes of certificate.It is fixed size*/
}ZCL_KeyEstInitiateKeyRespCommand;



/*structure for Ephemeral Data request command*/
typedef struct __attribute__((packed,aligned(1))) ZCL_KeyEstEphemeralDataReqCommand
{
    BYTE ephemeralDataNumber[22]; /*22 bytes of ephemeral number.It is fixed size*/
}ZCL_KeyEstEphemeralDataReqCommand;


/*structure for Ephemeral Data response command*/
typedef struct __attribute__((packed,aligned(1))) ZCL_KeyEstEphemeralDataRespCommand
{
    BYTE ephemeralDataNumber[22]; /*22 bytes of ephemeral number.It is fixed size*/
}ZCL_KeyEstEphemeralDataRespCommand;


/*structure for Confirm Data request command*/
typedef struct __attribute__((packed,aligned(1))) ZCL_KeyEstConfirmDataReqCommand
{
    BYTE message[16]; /*16 bytes of Message Authentication code -  MAC U or MAC V*/
}ZCL_KeyEstConfirmDataReqCommand;

/*structure for Confirm Data response command*/
typedef struct __attribute__((packed,aligned(1))) ZCL_KeyEstConfirmDataRespCommand
{
    BYTE message[16]; /*16 bytes of Message Authentication code -  MAC U or MAC V*/
}ZCL_KeyEstConfirmDataRespCommand;


/*structure for Terminate Key Establishment command*/
typedef struct __attribute__((packed,aligned(1))) ZCL_KeyEstTerminateKeyEstCommand
{
    BYTE status;
    BYTE waitTime;
    WORD keyEstSuite;
}ZCL_KeyEstTerminateKeyEstCommand;

/*****************************************************************************
                          Function Prototypes
 *****************************************************************************/

/******************************************************************************
 * Function:        BYTE ZCL_HandleKeyEstablishmentClusterCommand
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
 * Input:           endPointId -    EndPoint on which the Key establishment
 *                          Cluster command needs to be processed
 *                  asduLength -    Asdu Length of the recevied frame
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and Payload.
 *                  pZCLClusterInfo - Pointer pointing to the key establishment
 *                              Cluster info
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
 * Overview:        This function is called when a Key establishment
 *                  Cluster command is received.This function process the
 *                  key establishment Cluster command and notifies the
 *                  application about the action to be taken.
 *
 * Note:            None
 *
 *****************************************************************************/
BYTE ZCL_HandleKeyEstablishmentClusterCommand
(
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler *pActionHandler,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);

#endif /* #if I_SUPPORT_CBKE */

#endif /*_ZCL_KEY_ESTABLISHMENT_H*/
