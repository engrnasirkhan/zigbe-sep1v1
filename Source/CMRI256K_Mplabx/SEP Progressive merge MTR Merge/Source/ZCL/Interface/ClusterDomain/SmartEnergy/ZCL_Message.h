
/******************************************************************************
 *
 *                  ZCL Message
 *
 ******************************************************************************
 * FileName:        ZCL_Message.h
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
 *                  09/04/09

 *****************************************************************************/

#ifndef _ZCL_MESSAGE_H_
#define _ZCL_MESSAGE_H_

/*****************************************************************************
  Includes
 *****************************************************************************/
/* None */

/*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
#define ZCL_MESSAGING_CLUSTER                   0x0703

/*Message Control Fields*/
/*Type of Transmission in Message Control Field*/
#define NORMAL_TRANSMISSION_ONLY                0x00
#define NORMAL_AND_ANONYMOUS_INTERPAN           0x01
#define ANONYMOUS_INTERPAN_ONLY                 0x02
/*Level of Importance of the Message in Message Control Field*/
#define LOW_LEVEL_IMPORTANCE                    0x00
#define MEDIUM_LEVEL_IMPORTANCE                 0x04
#define HIGH_LEVEL_IMPORTANCE                   0x08
#define CRITICAL_LEVEL_IMPORTANCE               0x0C
/*Message Confirmation Bits in Message Control Field*/
#define MESSAGE_CONFIRMATION_NOT_REQUIRED       0x00
#define MESSAGE_CONFIRMATION_REQUIRED           0x80
/*End of Message Control Fields*/

/*Enumerations for Client to Server Message Commands*/
typedef enum ZCL_Message_ClientToServerCommands
{
    ZCL_Message_GetLastMessageCmd               = 0x00,
    ZCL_Message_MessageConfirmation             = 0x01
}ZCL_Message_ClientToServerCommands;

/*Enumerations for Server to Client commands*/
typedef enum ZCL_Message_ServerToClientCommands
{
    ZCL_Message_DisplayMessageCmd               = 0x00,
    ZCL_Message_CancelMessageCmd                = 0x01
}ZCL_Message_ServerToClientCommands;


/*enumerations for events for message cluster*/
typedef enum ZCL_Message_Events
{
    ZCL_Message_GetLastMessageEvent             = 0x80,
    ZCL_Message_MessageConfirmationEvent        = 0x81,
    ZCL_Message_DisplayMessageEvent             = 0x82,
    ZCL_Message_CancelMessageEvent              = 0x83
}ZCL_Message_Events;

/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
/* None */

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/
extern ROM ZCL_ClusterDefinition zcl_MessageClusterServerDefinition;
extern ROM ZCL_ClusterDefinition zcl_MessageClusterClientDefinition;

extern ROM ZCL_ClusterInfo zcl_MessageClusterServerInfo;
extern ROM ZCL_ClusterInfo zcl_MessageClusterClientInfo;

/*****************************************************************************
  Data Structures
 *****************************************************************************/
/* Structures for Message Cluster commands*/
typedef struct PACKED ZCL_Message_MessageConfirmationCmdPayload
{
    DWORD messageId;
    BYTE  messageControl;
}ZCL_Message_MessageConfirmationCmdPayload;

typedef struct PACKED ZCL_Message_CancelMessageCmdPayload
{
    DWORD messageId;
    BYTE  cancelControl;
}ZCL_Message_CancelMessageCmdPayload;

typedef struct __attribute__((packed,aligned(1))) ZCL_Message_DisplayMessageCmdPayload
{
    DWORD messageId;
    BYTE  messageControl;
    DWORD startTime;
    WORD durationInMinutes;
    //BYTE *pDisplayMessage;
    BYTE aDisplayMessage[200];
}ZCL_Message_DisplayMessageCmdPayload;


/*****************************************************************************
  Function Prototypes
 *****************************************************************************/
/******************************************************************************
 * Function:        BYTE ZCL_HandleMessageClusterCommand
 *                  (
 *                      BYTE endPointId,
 *                      BYTE asduLength,
 *                      BYTE* pReceivedAsdu,
 *                      BYTE* pResponseAsduStartLocation,
 *                      ZCL_ActionHandler* pActionHandler,
 *                      ROM ZCL_ClusterInfo* pZCLClusterInfo
 *                  );
 *
 * PreCondition:    pResponseAsduStartLocation - should be pointing to
 *                  valid address since validation is not done in this function.
 *                  Event Data member in the pointer pActionHandler should be
 *                  allocated statically or dynamically since validation of this
 *                  field is not taken care in this function.
 *                  If invalid pointers arrive since validation is not
 *                  performed, the code might freeze
 *
 * Input:           endPointId - EndPoint on which the Message cluster
 *                                  command is received
 *                  asduLength - gives length of the payload
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and
 *                                  Message command payload.
 *                  pZCLClusterInfo - Pointer pointing to the Message Cluster info
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
 * Return :         Returns length of the data if response needs to be created
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when Message cluster specific client
 *                  or server command is received. This parses the command
 *                  updates the action accordingly. Based on the action and the
 *                  received command, the length is updated and returned from
 *                  function.
 *
 *
 * Note:            None
 *****************************************************************************/
BYTE ZCL_HandleMessageClusterCommand
(
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler *pActionHandler,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);


/******************************************************************************
 * Function:        BYTE ZCL_Message_CreateMessageConfirmation
 *                  (
 *                      BYTE *pResponseAsdu,
 *                      BYTE transactionSeqNum,
 *                      ZCL_Message_DisplayMessageCmdPayload displayMessage
 *                  )
 *
 * PreCondition:    pResponseAsdu - should be pointing to
 *                  valid address since validation is not done in this function.
 *                  If invalid pointers arrive since validation is not
 *                  performed, the code might freeze.
 *
 * Input:           transactionSeqNum - Transaction sequence number to be
 *                                      added into the ZCL header while
 *                                      creating Message confirmation command
 *                  pMessageId   - This gives the Message id for which
 *                                  message confirmation is sent.
 *
 * Output:          pResponseAsdu - Pointer pointing to the start
 *                                  location of asdu for which the
 *                                  response frame namely message confirmation
 *                                  needs to be created.
 * Return :         Returns length of the Message confirmation
 *
 * Side Effects:    None
 *
 * Overview:        This is invoked by the Application when it wishes to create
 *                  Message confirmation command. It returns the length of the
 *                  message confirmation.
 *
 *
 * Note:            None
 *****************************************************************************/
BYTE ZCL_Message_CreateMessageConfirmation
(
    BYTE *pResponseAsdu,
    BYTE transactionSeqNum,
    DWORD* pMessageId
);


/******************************************************************************
 * Function:        BYTE ZCL_CreateDisplayMessageCommand
 *                   (
 *                       BYTE *pResponseAsdu,
 *                       ZCL_Message_DisplayMessageCmdPayload* pDisplayMessage ,
 *                       BYTE transactionSeqNum
 *                   )
 *
 * PreCondition:    pResponseAsdu - should be pointing to
 *                  valid address since validation is not done in this function.
 *                  If invalid pointers arrive since validation is not
 *                  performed, the code might freeze.
 *
 * Input:           transactionSeqNum - Transaction sequence number to be
 *                                      added into the ZCL header while
 *                                      creating Message confirmation command
 *                  pDisplayMessage   - This gives the payload of display message
 *                                      command.
 *
 * Output:          pResponseAsdu - Pointer pointing to the start
 *                                  location of asdu for which the
 *                                  response frame namely display message
 *                                  needs to be created.
 * Return :         Returns length of the display message command
 *
 * Side Effects:    None
 *
 * Overview:        This is invoked by the Application when it wishes to create
 *                  Display message command command. It returns the length of the
 *                  command payload.
 *
 *
 * Note:            None
 *****************************************************************************/
BYTE ZCL_CreateDisplayMessageCommand
(
    BYTE *pResponseAsdu,
    ZCL_Message_DisplayMessageCmdPayload* pDisplayMessage,
    BYTE transactionSeqNum
 );
#endif /* _ZCL_MESSAGE_H_ */
