/*********************************************************************
 *
 *                  Application Interface Layer
 *
 *********************************************************************
 * FileName:        zAIL.c
 * Dependencies:
 * Processor:       PIC18
 * Complier:        MCC18 v1.00.50 or higher
 *                  HITECH PICC-18 V8.10PL1 or higher
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
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      10/24/07    Original
 ********************************************************************/

//******************************************************************************
// Header Files
//******************************************************************************

#include "zAIL.h"
#include "ZigbeeTasks.h"
#include "sralloc.h"
#include "zAPL.h"

//******************************************************************************
// Constants
//******************************************************************************

//#define NULL                0
#define ZDODiscardRx()  APSDiscardRx()

//******************************************************************************
// Function Prototypes
//******************************************************************************

//******************************************************************************
// Variable Definitions
//******************************************************************************

BYTE *AILTransmitQueue[AIL_QUEUE_LENGTH];
BYTE *AILReceiveQueue[AIL_QUEUE_LENGTH];

BYTE AILTransmitQueueHeadIndex, AILTransmitQueueTailIndex;
BYTE AILReceiveQueueHeadIndex, AILReceiveQueueTailIndex;

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

void AILInit( void )
{
    AILTransmitQueueHeadIndex = AIL_QUEUE_INDEX_DEFAULT_VALUE;
    AILTransmitQueueTailIndex = AIL_QUEUE_INDEX_DEFAULT_VALUE;
    AILReceiveQueueHeadIndex = AIL_QUEUE_INDEX_DEFAULT_VALUE;
    AILReceiveQueueTailIndex = AIL_QUEUE_INDEX_DEFAULT_VALUE;
}

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE AILTasks(ZIGBEE_PRIMITIVE inputPrimitive)
 *
 * PreCondition:    None
 *
 * Input:           inputPrimitive - the next primitive to run
 *
 * Output:          The next primitive to run.
 *
 * Side Effects:    Numerous
 *
 * Overview:        This routine executes the indicated primitive.
 *                  If no primitive is specified, then background
 *                  tasks are executed.
 *
 * Note:            If this routine is called with NO_PRIMITIVE, it is
 *                  assumed that the TX and RX paths are not blocked,
 *                  and the background tasks may initiate a transmission.
 *                  It is the responsibility of this task to ensure that
 *                  only one output primitive is generated by any path.
 *                  If multiple output primitives are generated, they
 *                  must be generated one at a time by background processing.
 ********************************************************************/

ZIGBEE_PRIMITIVE AILTasks(ZIGBEE_PRIMITIVE inputPrimitive)
{
    if (inputPrimitive == NO_PRIMITIVE)
    {
        APP_DATA_request    *dataReq;
        BYTE                length;

        dataReq = (APP_DATA_request *) AILDequeue( TransmitQueueID );
        if ( dataReq != NULL )
        {
            params.APSDE_DATA_request.DstAddrMode               = dataReq->DstAddrMode;
            params.APSDE_DATA_request.DstAddress.ShortAddr.Val  = dataReq->DstAddress.ShortAddr.Val;
            params.APSDE_DATA_request.DstEndpoint               = dataReq->DstEndpoint;
            params.APSDE_DATA_request.ProfileId.Val             = dataReq->ProfileId.Val;
            params.APSDE_DATA_request.ClusterId.Val             = dataReq->ClusterId.Val;
            params.APSDE_DATA_request.SrcEndpoint               = dataReq->SrcEndpoint;
            params.APSDE_DATA_request.TxOptions.Val             = dataReq->TxOptions.Val;
            params.APSDE_DATA_request.RadiusCounter             = dataReq->RadiusCounter;
            params.APSDE_DATA_request.asduLength                = dataReq->asduLength;
            params.APSDE_DATA_request.DiscoverRoute             = 1;
            if (dataReq->asduLength > 0)
            {
                for( length = 0; length < dataReq->asduLength; length++ )
                {
                    TxBuffer[TxData++] = *( dataReq->asdu + length );
                }
            }

            nfree(dataReq);
            return APSDE_DATA_request;
        }
        return NO_PRIMITIVE;
    }
    else
    {
        switch ( inputPrimitive )
        {
            case APSDE_DATA_indication:
            {
                APP_DATA_indication     *dataInd;
                BYTE                    length;

                dataInd = (APP_DATA_indication *)SRAMalloc( 126 );
                if ( dataInd != NULL )
                {
                    dataInd->MessageType                = MessageTypeAppDataIndication;
                    dataInd->DstAddrMode                = params.APSDE_DATA_indication.DstAddrMode;
                    dataInd->DstAddress.ShortAddr.Val   = params.APSDE_DATA_indication.DstAddress.ShortAddr.Val;
                    dataInd->DstEndpoint                = params.APSDE_DATA_indication.DstEndpoint;
                    dataInd->SrcAddrMode                = params.APSDE_DATA_indication.SrcAddrMode;
                    dataInd->SrcAddress.ShortAddr.Val   = params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val;
                    dataInd->SrcEndpoint                = params.APSDE_DATA_indication.SrcEndpoint;
                    dataInd->ProfileId.Val              = params.APSDE_DATA_indication.ProfileId.Val;
                    dataInd->ClusterId.Val              = params.APSDE_DATA_indication.ClusterId.Val;
                    dataInd->Status                     = params.APSDE_DATA_indication.Status;
                    dataInd->SecurityStatus             = params.APSDE_DATA_indication.SecurityStatus;
                    dataInd->WasBroadcast               = params.APSDE_DATA_indication.WasBroadcast;
                    dataInd->LinkQuality                = params.APSDE_DATA_indication.LinkQuality;
                    dataInd->RxTime.Val                 = params.APSDE_DATA_indication.RxTime.Val;
                    dataInd->asduLength                 = params.APSDE_DATA_indication.asduLength;

                    if (params.APSDE_DATA_indication.asduLength > 0)
                    {
                        for( length = 0; length < params.APSDE_DATA_indication.asduLength; length++ )
                        {
                            *(dataInd->asdu + length) = *( params.APSDE_DATA_indication.asdu + length );
                        }
                    }

                    if (params.APSDE_DATA_indication.DstAddrMode != 0x01)
                    {
                        APLDiscardRx();
                    }

                    if (params.APSDE_DATA_indication.SrcEndpoint == 0x00)
                    {
                        ZDODiscardRx();
                    }

                    if ( AILEnqueue( (BYTE *)dataInd, ReceiveQueueID ) == AILQueueFailure )
                    {
                        nfree(dataInd);
                    }
                    return NO_PRIMITIVE;
                }
            }
            break;

            case APSDE_DATA_confirm:
            {
                APP_DATA_confirm    *dataConf;

                dataConf = (APP_DATA_confirm *)SRAMalloc(sizeof( APP_DATA_confirm ) );
                if ( dataConf != NULL )
                {
                    dataConf->MessageType               = MessageTypeAppDataConfirm;
                    dataConf->Status                    = params.APSDE_DATA_confirm.Status;
                    dataConf->DstAddrMode               = params.APSDE_DATA_confirm.DstAddrMode;
                    dataConf->DstAddress.ShortAddr.Val  = params.APSDE_DATA_confirm.DstAddress.ShortAddr.Val;
                    dataConf->DstEndpoint               = params.APSDE_DATA_confirm.DstEndpoint;
                    dataConf->SrcEndpoint               = params.APSDE_DATA_confirm.SrcEndpoint;
                    dataConf->TxTime.Val                = params.APSDE_DATA_confirm.TxTime.Val;

                    if ( AILEnqueue( (BYTE *)dataConf, ReceiveQueueID ) == AILQueueFailure )
                    {
                        nfree(dataConf);
                    }

                    return NO_PRIMITIVE;
                }
            }
            break;

            default:
            break;
        }
        return NO_PRIMITIVE;
    }
}

/*****************************************************************************************************
 * Function:        BYTE AILEnqueue( BYTE *pPacketToBeQueued, BYTE QueueID );
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
 ****************************************************************************************************/
BYTE AILEnqueue( BYTE *pPacketToBeQueued, BYTE QueueID )
{
    if ( QueueID == TransmitQueueID )
    {
        if( GET_AIL_TRANSMIT_QUEUE_SPACE() )
        {
            AILTransmitQueue[AILTransmitQueueTailIndex] = pPacketToBeQueued;
            AILTransmitQueueTailIndex++;
            if( AILTransmitQueueTailIndex == AIL_QUEUE_LENGTH)
            {
                AILTransmitQueueTailIndex = AIL_QUEUE_INDEX_DEFAULT_VALUE;
            }
            return AILQueueSuccess;
        }
    }
    else if ( QueueID == ReceiveQueueID )
    {
        if( GET_AIL_RECEIVE_QUEUE_SPACE() )
        {
            AILReceiveQueue[AILReceiveQueueTailIndex] = pPacketToBeQueued;
            AILReceiveQueueTailIndex++;
            if( AILReceiveQueueTailIndex == AIL_QUEUE_LENGTH)
            {
                AILReceiveQueueTailIndex = AIL_QUEUE_INDEX_DEFAULT_VALUE;
            }
            return AILQueueSuccess;
        }
    }
    return AILQueueFailure;
}

/*********************************************************************
 * Function:        BYTE *AILDequeue( BYTE QueueID )
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

BYTE *AILDequeue( BYTE QueueID )
{
    BYTE *pMessage;

    if ( QueueID == TransmitQueueID )
    {
        if( GET_AIL_TRANSMIT_QUEUE_STATUS() )
        {
            pMessage = AILTransmitQueue[AILTransmitQueueHeadIndex];
            AILTransmitQueueHeadIndex++;
            if( AILTransmitQueueHeadIndex == AIL_QUEUE_LENGTH )
            {
                AILTransmitQueueHeadIndex = AIL_QUEUE_INDEX_DEFAULT_VALUE;
            }
            return pMessage;
        }
    }
    else if ( QueueID == ReceiveQueueID )
    {
        if( GET_AIL_RECEIVE_QUEUE_STATUS() )
        {
            pMessage = AILReceiveQueue[AILReceiveQueueHeadIndex];
            AILReceiveQueueHeadIndex++;
            if( AILReceiveQueueHeadIndex == AIL_QUEUE_LENGTH )
            {
                AILReceiveQueueHeadIndex = AIL_QUEUE_INDEX_DEFAULT_VALUE;
            }
            return pMessage;
        }
    }
    return NULL;
}
