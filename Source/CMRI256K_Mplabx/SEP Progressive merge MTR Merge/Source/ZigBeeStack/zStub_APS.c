/*********************************************************************
 *
 *                  ZigBee Stub APS Layer
 *
 *********************************************************************
 * FileName:        zStub_APS.c.c
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 ********************************************************************/
#include "ZigBeeTasks.h"
#include "sralloc.h"
#include "zNVM.h"
#include "generic.h"
#include "Log.h"
#include "zMAC.h"
#include "zNWK.h"
#include "zAPS.h"
#include "zZDO.h"
#include "zAPL.h"
#include "zigbee.def"
#include "zigbee.h"

#include "console.h"
#include "zSecurity.h"
#include "zStub_APS.h"

/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/
// Frame Type
#define INTRP_FRAME_DATA                          0x03

// Delivery Mode
#define INTRP_DELIVERY_DIRECT                     0x00
#define INTRP_DELIVERY_BROADCAST                  0x08
#define INTRP_DELIVERY_GROUP                      0xC0

// Destination Address Modes
#define INTRP_GROUP_ADDR_MODE                     0x01
#define INTRP_16BIT_ADDR_MODE                     0x02 /*Unicast or Broadcast*/
#define INTRP_64BIT_ADDR_MODE                     0x03
#define PROTOCOL_VERSION                          0x08
#define BROADCAST_SHORT_ADDRESS                   0xFFFF
#define INVALID_SHORT_ADDRESS1                    0xFFF7
/*****************************************************************************
   Customizable Macros
 *****************************************************************************/
#define INTRP_END_POINT                           0x01

/*****************************************************************************
   Data Structures
 *****************************************************************************/
/* None */


/*****************************************************************************
   Variable Definitions
 *****************************************************************************/
BYTE storeMsduHandle;
BYTE interPanDataTransmitted;
BYTE storeAsduHandle;

#if defined(I_SUPPORT_GROUP_ADDRESSING)
    extern GROUP_ADDRESS_RECORD currentGroupAddressRecord;
#endif

/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
#if defined(I_SUPPORT_GROUP_ADDRESSING)
    extern BYTE GetEndPointsFromGroup(INPUT SHORT_ADDR GroupAddr);
#endif


/*****************************************************************************
  Private Functions
 *****************************************************************************/
static BYTE STUBAPSGet( void );

/*****************************************************************************
  Public Functions
 *****************************************************************************/
/*********************************************************************
 * Function:        BOOL StubAPSHasBackgroundTasks( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - Stub APS layer has background tasks to run
 *                  FALSE - Stub APS layer does not have background tasks
 *
 * Side Effects:    None
 *
 * Overview:        Determines if the Stub APS layer has background tasks
 *                  that need to be run.
 *
 * Note:            None
 ********************************************************************/

BOOL StubAPSHasBackgroundTasks( void )
{
    /* There will not be any background tasks for STUB APS.
       So always return FALSE */
    return FALSE;
}

/*********************************************************************
 * Function:        void StubAPSInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    Stub APS layer data structures are initialized.
 *
 * Overview:        This routine initializes all Stub APS layer data
 *                  structures.
 *
 * Note:            This routine is intended to be called as part of
 *                  a network or power-up initialization.
 ********************************************************************/

void StubAPSInit( void )
{
    /* No global data for initialization */
}


/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE StubAPSTasks(ZIGBEE_PRIMITIVE inputPrimitive)
 *
 * PreCondition:    None
 *
 * Input:           inputPrimitive - the next primitive to run
 *
 * Output:          None

 * Return:          The next primitive to run.
 *
 * Side Effects:    None
 *
 * Overview:        This routine executes the indicated STUB APS primitive.
 *                  If no primitive is specified, then background
 *                  tasks are executed.
 *
 * Note:            This routine may be called while the TX path is blocked.
 *                  Therefore, we must check the Tx path before doing any
 *                  processing that may generate a message.
 *                  It is the responsibility of this task to ensure that
 *                  only one output primitive is generated by any path.
 *                  If multiple output primitives are generated, they
 *                  must be generated one at a time by background processing.
 ********************************************************************/
ZIGBEE_PRIMITIVE StubAPSTasks(ZIGBEE_PRIMITIVE inputPrimitive)
{
    ZIGBEE_PRIMITIVE nextPrimitive = NO_PRIMITIVE;

    switch(inputPrimitive)
    {
        case NO_PRIMITIVE:
            /* Any background tasks should be handled here */
            break;

        case INTRP_DATA_request:
        {
            LONG_ADDR macExtendedAddress;
            BYTE i;

            ZigBeeBlockTx();

            /* Application DATA */
            for (i = 0; i < params.INTRP_DATA_req.ASDULength; i++)
            {
                TxBuffer[TxData++] = params.INTRP_DATA_req.ASDU[i];
            }
            params.MCPS_DATA_request.msduLength += params.INTRP_DATA_req.ASDULength;
            nfree(params.INTRP_DATA_req.ASDU);

            /* APS Header */
            /* Update the profile ID in APS Header */
            TxBuffer[TxHeader--] = params.INTRP_DATA_req.ProfileId.v[1];
            TxBuffer[TxHeader--] = params.INTRP_DATA_req.ProfileId.v[0];

            /* Update the Cluster ID in APS Header */
            TxBuffer[TxHeader--] = params.INTRP_DATA_req.ClusterId.v[1];
            TxBuffer[TxHeader--] = params.INTRP_DATA_req.ClusterId.v[0];

            /* msduLength for profile ID and Cluster ID */
            params.MCPS_DATA_request.msduLength = 4;

            /* Default TxOption - No Security and No Ack */
            params.MCPS_DATA_request.TxOptions.Val = 0;

            /* Check destination address mode as group addressing */
            if ( params.INTRP_DATA_req.DstAddrMode == INTRP_GROUP_ADDR_MODE)
            {

                /* Increment msduLength for group address and APS frame control
                   which is 3 bytes */
                params.MCPS_DATA_request.msduLength += 0x03;
                /* Add the group address */
                TxBuffer[TxHeader--] = params.INTRP_DATA_req.DstAddress.v[1];
                TxBuffer[TxHeader--] = params.INTRP_DATA_req.DstAddress.v[0];

                /* APS Frame Control Field
                   Frame Type (bit 0 and bit 1) -- Reserved (0b11)
                   Delivery mode (bit 2 and bit 3)--Group   (0b11) */
                TxBuffer[TxHeader--] = INTRP_FRAME_DATA | INTRP_DELIVERY_GROUP;

                /* Group data will be broadcasted */
                params.MCPS_DATA_request.DstAddrMode = INTRP_16BIT_ADDR_MODE;
                params.MCPS_DATA_request.DstAddr.ShortAddr.Val = BROADCAST_SHORT_ADDRESS;
            }
            else if ( params.INTRP_DATA_req.DstAddrMode == INTRP_16BIT_ADDR_MODE )
            {
                /* Increment msduLength for APS frame control */
                params.MCPS_DATA_request.msduLength += 1;

                if ( params.INTRP_DATA_req.DstAddress.ShortAddr.Val >= INVALID_SHORT_ADDRESS1 )
                {
                     /* APS Frame Control Field
                        Frame Type (bit 0 and bit 1) -- Reserved (0b11)
                        Delivery mode (bit 2 and bit 3)--Broadcast(0b10) */
                     TxBuffer[TxHeader--] = INTRP_FRAME_DATA | INTRP_DELIVERY_BROADCAST;

                     /* Destination address will be broadcast address */
                     params.MCPS_DATA_request.DstAddr.ShortAddr.Val = BROADCAST_SHORT_ADDRESS;
                }
                else
                {
                    /* APS Frame Control Field
                        Frame Type (bit 0 and bit 1) -- Reserved (0b11)
                        Delivery mode (bit 2 and bit 3)-- Unicast (0b00) */
                    TxBuffer[TxHeader--] = INTRP_FRAME_DATA | INTRP_DELIVERY_DIRECT;

                    /* For unicast data MAC ack is always requested */
                    params.MCPS_DATA_request.TxOptions.bits.acknowledged_transmission = 1;
                }
            }

            else if ( params.INTRP_DATA_req.DstAddrMode == INTRP_64BIT_ADDR_MODE )
            {
                /* APS Frame Control Field
                   Frame Type (bit 0 and bit 1) -- Reserved (0b11)
                   Delivery mode (bit 2 and bit 3)--  Unicast(0b00) */
                TxBuffer[TxHeader--] = INTRP_FRAME_DATA | INTRP_DELIVERY_DIRECT;

                /* For unicast data MAC ack is always requested */
                params.MCPS_DATA_request.TxOptions.bits.acknowledged_transmission = 1;
            }

            if ( params.INTRP_DATA_req.DstPANId.Val == BROADCAST_SHORT_ADDRESS )
            {
                /* For Broadcast PAN ack should be disabled */
                params.MCPS_DATA_request.TxOptions.bits.acknowledged_transmission = 0;
            }
            /* NWK Header */
            /* Reserved values */
            TxBuffer[TxHeader--] = 0x00;
            /* Protocol version (Bit 2- 5) 0x02(0b010)
               frame type 0x03(0b11)*/
            TxBuffer[TxHeader--] = INTRP_FRAME_DATA | PROTOCOL_VERSION;

            /* Increment msdulength with network layer header */
            params.MCPS_DATA_request.msduLength += 2;

            /* Update the source PANID from MAC PIB */
            params.MCPS_DATA_request.SrcPANId.Val = macPIB.macPANId.Val;

             /* Source address will be always extended address */
            GetMACAddress(&macExtendedAddress);

            params.MCPS_DATA_request.SrcAddr.v[0] = macExtendedAddress.v[0];
            params.MCPS_DATA_request.SrcAddr.v[1] = macExtendedAddress.v[1];
            params.MCPS_DATA_request.SrcAddr.v[2] = macExtendedAddress.v[2];
            params.MCPS_DATA_request.SrcAddr.v[3] = macExtendedAddress.v[3];
            params.MCPS_DATA_request.SrcAddr.v[4] = macExtendedAddress.v[4];
            params.MCPS_DATA_request.SrcAddr.v[5] = macExtendedAddress.v[5];
            params.MCPS_DATA_request.SrcAddr.v[6] = macExtendedAddress.v[6];
            params.MCPS_DATA_request.SrcAddr.v[7] = macExtendedAddress.v[7];

            /* Update the frame type as data frame */
            params.MCPS_DATA_request.frameType = FRAME_DATA;

            /* Store MSDU handle for handling inter PAN data confirmation */
            storeMsduHandle = MLME_GET_macDSN();

            /* Store ASDU for updating inter PAN data confirmation */
            storeAsduHandle = params.INTRP_DATA_req.ASDUHandle;

            /* For handling confirmation this variable will be set */
            interPanDataTransmitted = 1;

            nextPrimitive = MCPS_DATA_request;
        }
        break;

        default:
            break;

    }
    return nextPrimitive;
}




/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE handleInterPanDataInd()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Return:          The next primitive to run.
 *
 * Side Effects:    None
 *
 * Overview:        This routine will handle reception of inter PAN
 *                  data indication and creates and sends inter PAN data
 *                  indication to the application.
 *
 ********************************************************************/
ZIGBEE_PRIMITIVE handleInterPanDataInd()
{
    BYTE apsFrameControl;

    #if defined(I_SUPPORT_GROUP_ADDRESSING)
        SHORT_ADDR GroupAddr;
    #endif

    apsFrameControl = STUBAPSGet();

    if ( ( apsFrameControl & INTRP_FRAME_DATA ) == INTRP_FRAME_DATA )
    {
        if ( ( apsFrameControl & INTRP_DELIVERY_GROUP ) == INTRP_DELIVERY_GROUP )
        {
            #if defined(I_SUPPORT_GROUP_ADDRESSING)
                GroupAddr.v[0] = STUBAPSGet();
                GroupAddr.v[1] = STUBAPSGet();

                /*Check whether the Group Address is exists or not.*/
                if ( GetEndPointsFromGroup(GroupAddr) != MAX_GROUP )
                {
                    BYTE i;
                    /*Do Validation of End Point*/
                    for( i = 0; i < MAX_GROUP_END_POINT; i++ )
                    {
                        if ( currentGroupAddressRecord.EndPoint[i] == INTRP_END_POINT )
                        {
                            /*End Point found*/
                            break;
                        }
                    }
                    if ( i != MAX_GROUP_END_POINT )
                    {
                        params.INTRP_DATA_indication.DstAddrMode = INTRP_GROUP_ADDR_MODE;
                        params.INTRP_DATA_indication.DstAddress.ShortAddr.Val = GroupAddr.Val;
                    }
                    else
                    {
                        APSDiscardRx();
                        return NO_PRIMITIVE;
                    }
                }
                else
                {
                    /* Group address is not existing in the Group table */
                    APSDiscardRx();
                    return NO_PRIMITIVE;
                }
            #else
                APSDiscardRx();
                return NO_PRIMITIVE;
            #endif
        }

        /*SrcAddrMode, SrcPANId, SrcAddress, DstAddrMode, DstPANId, DstAddress,
        LinkQuality are aligned already with MCPS_DATA_indication. */

        /*We need to update only the ClusterId and ProfileId*/
        params.INTRP_DATA_indication.ClusterId.v[0] = STUBAPSGet();
        params.INTRP_DATA_indication.ClusterId.v[1] = STUBAPSGet();

        params.INTRP_DATA_indication.ProfileId.v[0] = STUBAPSGet();
        params.INTRP_DATA_indication.ProfileId.v[1] = STUBAPSGet();

        params.INTRP_DATA_indication.ASDU = params.MCPS_DATA_indication.msdu;
    }
    else
    {
        APSDiscardRx();
        return NO_PRIMITIVE;
    }
    return INTRP_DATA_indication;
}


/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE handleInterPanDataConf()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          The next primitive to run.
 *
 * Side Effects:    None
 *
 * Overview:        This routine will handle reception of inter PAN
 *                  data confirm and creates and sends inter PAN data
 *                  confirmation to the application.
 *
 ********************************************************************/
ZIGBEE_PRIMITIVE handleInterPanDataConf()
{
    /* This variable will be set on inter pan data request and
       should be cleared when data confirmation for the sent
       inter PAN frame is received */
    interPanDataTransmitted = 0;

    /* Stored asdu handle when the request is issued needs to
       be given with inter pan data confirmation */
    params.INTRP_DATA_conf.ASDUHandle = storeAsduHandle;

    /* return the next primitive to be executed */
    return INTRP_DATA_confirm;
}


/*********************************************************************
 * Function:        static BYTE STUBAPSGet( void )
 *
 * PreCondition:    None
 *
 * Input:           none
 *
 * Output:          One byte from the msdu if the length is greater
 *                  than 0; otherwise, 0.
 *
 * Side Effects:    The msdu pointer is incremented to point to the
 *                  next byte, and msduLength is decremented.
 *
 * Overview:        This function returns the next byte from the
 *                  msdu.
 *
 * Note:            None
 ********************************************************************/
static BYTE STUBAPSGet( void )
{
    if (params.MCPS_DATA_indication.msduLength == 0)
    {
        /* No data existing */
        return 0;
    }
    else
    {
        /* Decrement the msdu length */
        params.MCPS_DATA_indication.msduLength--;

        /* Return one msdu byte */
        return *params.MCPS_DATA_indication.msdu++;
    }
}
