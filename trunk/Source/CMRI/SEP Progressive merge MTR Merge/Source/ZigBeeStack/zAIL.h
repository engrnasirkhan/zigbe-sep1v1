/*********************************************************************
 *
 *                  Application Interface Layer
 *
 *********************************************************************
 * FileName:        zAIL.h
 * Dependencies:
 * Processor:       PIC18
 * Complier:        MCC18 v1.00.50 or higher
 *                  HITECH PICC-18 V8.10PL1 or higher
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
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      10/24/07    Original
 ********************************************************************/

 #ifndef _zAIL_H_
 #define _zAIL_H_

//******************************************************************************
// Header Files
//******************************************************************************

#include "generic.h"
#include "zigbee.h"
#include "ZigbeeTasks.h"

//******************************************************************************
// Constants
//******************************************************************************

#define TransmitQueueID     0x01
#define ReceiveQueueID      0x02

#define MessageTypeAppDataIndication                0x01
#define MessageTypeAppDataConfirm                   0x02

#define AIL_QUEUE_LENGTH                            0x0A
#define AIL_QUEUE_INDEX_DEFAULT_VALUE               0x00

#define GET_AIL_TRANSMIT_QUEUE_SPACE()              (AILTransmitQueueTailIndex > AILTransmitQueueHeadIndex) ? \
                                                    (AILTransmitQueueTailIndex - AILTransmitQueueHeadIndex != AIL_QUEUE_LENGTH) : \
                                                    (AILTransmitQueueHeadIndex - AILTransmitQueueTailIndex != 1)

#define GET_AIL_TRANSMIT_QUEUE_STATUS()             ((AILTransmitQueueTailIndex - AILTransmitQueueHeadIndex) != 0)

#define GET_AIL_RECEIVE_QUEUE_SPACE()               (AILReceiveQueueTailIndex > AILReceiveQueueHeadIndex) ? \
                                                    (AILReceiveQueueTailIndex - AILReceiveQueueHeadIndex != AIL_QUEUE_LENGTH) : \
                                                    (AILReceiveQueueHeadIndex - AILReceiveQueueTailIndex != 1)

#define GET_AIL_RECEIVE_QUEUE_STATUS()              ((AILReceiveQueueTailIndex - AILReceiveQueueHeadIndex) != 0)

enum
{
    AILQueueSuccess,
    AILQueueFailure
};

typedef struct __attribute__((packed,aligned(2))) _APP_DATA_request
{
    BYTE        DstAddrMode;
    ADDR        DstAddress;
    BYTE        DstEndpoint;
    WORD_VAL    ProfileId;
    WORD_VAL    ClusterId;
    BYTE        SrcEndpoint;
    union _APP_TX_OPTIONS
    {
        BYTE    Val;
        struct _APP_TX_OPTIONS_BITS
        {
            BYTE    securityEnabled : 1;
            BYTE    useNWKKey       : 1;
            BYTE    acknowledged    : 1;
        } bits;
    }           TxOptions;
    BYTE        RadiusCounter;
    BYTE        asduLength;
    BYTE        asdu[1];
} APP_DATA_request;

typedef struct __attribute__((packed,aligned(2))) _APP_DATA_confirm
{
    BYTE        MessageType;
    BYTE        Status;
    BYTE        DstAddrMode;
    ADDR        DstAddress;
    BYTE        DstEndpoint;
    BYTE        SrcEndpoint;
    TICK        TxTime;
} APP_DATA_confirm;
/**************************************************
  Overview : Structure that holds all the payload information from the most recently received packet,
 **************************************************/ 
typedef struct __attribute__((packed,aligned(2))) _APP_DATA_indication
{
    BYTE        MessageType; 
    BYTE        DstAddrMode; 
    ADDR        DstAddress;
    BYTE        DstEndpoint;
    BYTE        SrcAddrMode;
    ADDR        SrcAddress;// the packet Sender's address
    BYTE        SrcEndpoint;// the packet Sender's End Point
    WORD_VAL    ProfileId;// Profile to which it packet belongs
    WORD_VAL    ClusterId;
    BYTE        Status;
    BYTE        SecurityStatus;
    BYTE        WasBroadcast;
    BYTE        LinkQuality;
    TICK        RxTime;
    BYTE        asduLength; // Length of Asdu
    BYTE        asdu[1];
} APP_DATA_indication;

//******************************************************************************
// Function Prototypes
//******************************************************************************

/***********************************************************************************************************************
 * Function:        void AILInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ***********************************************************************************************************************/

void AILInit( void );

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE AILTasks(ZIGBEE_PRIMITIVE inputPrimitive)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/

ZIGBEE_PRIMITIVE AILTasks(ZIGBEE_PRIMITIVE inputPrimitive);

/*********************************************************************
 * Function:        BYTE AILEnqueue( BYTE *pPacketToBeQueued, BYTE QueueID )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/

BYTE AILEnqueue( BYTE *pPacketToBeQueued, BYTE QueueID );

/*********************************************************************
 * Function:        BYTE * AILDequeue( BYTE QueueID )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/

BYTE * AILDequeue( BYTE QueueID );

#endif
