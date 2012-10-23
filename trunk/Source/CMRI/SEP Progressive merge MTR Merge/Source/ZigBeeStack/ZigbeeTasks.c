/*********************************************************************
 *
 *                  ZigBee Tasks File
 *
 *********************************************************************
 * FileName:        ZigBeeTasks.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY             01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

#include "Compiler.h"
#include "generic.h"
#include "zigbee.def"
#include "ZigbeeTasks.h"
#include "zNVM.h"
#include "zPHY.h"
#include "zMAC.h"
#include "zNWK.h"
#include "zAPS.h"
#include "zZDO.h"
#include "zAIL.h"
#include "MSPI.h"
#include "sralloc.h"
#include "SymbolTime.h"
#include "console.h"
#if I_SUPPORT_STUB_APS == 1
#include "zStub_APS.h"
#endif

BYTE TxBuffer[TX_BUFFER_SIZE+1];
#if defined(__C30__) || defined(__C32__)
    volatile BYTE RxBuffer[RX_BUFFER_SIZE];
#else
    #pragma udata RX_BUFFER=RX_BUFFER_LOCATION
        volatile BYTE RxBuffer[RX_BUFFER_SIZE];
    #pragma udata
#endif

BYTE                    * CurrentRxPacket;
PARAMS                  params;
APP_NOTIFICATION        appNotify;
#if (RX_BUFFER_SIZE > 256)
    volatile unsigned int    RxWrite;
    unsigned int     RxRead;
#else
    volatile BYTE    RxWrite;
    BYTE             RxRead;
#endif

BYTE                    TxData;
BYTE                    TxHeader;
volatile ZIGBEE_STATUS  ZigBeeStatus;

ROM BYTE releaseVersionNumber[] = "Rel-Vesion.0.5.2";

ZIGBEE_PRIMITIVE AppTasks(ZIGBEE_PRIMITIVE inputPrimitive);

/***************************************************************************************
 * Function:        void ZigBeeInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the ZigBee stack.
 *
 * Note:            None
 ***************************************************************************************/

void ZigBeeInit(void)
{
    #if !defined(__C30__) && !defined(__C32__)
        SRAMInitHeap();
    #endif


    MACInit();
    NWKInit();
    APSInit();
    ZDOInit();

    TxHeader = 127;
    TxData = 0;
    RxWrite = 0;
    RxRead = 0;

    // Set up the interrupt to read in a data packet.
    // set to capture on falling edge
    #if !defined(__C30__) && !defined (__C32__)
        #if (RF_CHIP == UZ2400) || (RF_CHIP == MRF24J40)
            CCP2CON = 0b00000100;
        #endif
    #endif

    // Set up the interrupt to read in a data packet.
    #if (RF_CHIP==UZ2400) || (RF_CHIP == MRF24J40)
        RFIF = 0;
        RFIE = 1;
    #endif
    InitSymbolTimer();

    ZigBeeStatus.nextZigBeeState = NO_PRIMITIVE;
    // Set CurrentRxPacket to NULL so that the device can receive new packets.
    CurrentRxPacket = NULL;
    
    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
    ZigBeeStatus.flags.bits.bLostSyncWithParent = 0;
    }//#endif
    
    ZigBeeStatus.flags.bits.bActiveScanToRejoin = 0;
}


/***************************************************************************************
 * Function:        BOOL ZigBeeTasks( ZIGBEE_PRIMITIVE *command )
 *
 * PreCondition:    None
 *
 * Input:           *command: current Foreground primitive to be handled.
 *
 * Output:          Returns whether background processes are pending.
 *
 * Side Effects:    None
 *
 * Overview:        Handles the state machine of stack and application.
 *
 * Note:            None
 ***************************************************************************************/

BOOL ZigBeeTasks( ZIGBEE_PRIMITIVE *command )
{
    ZigBeeStatus.nextZigBeeState = *command;

    BOOL appEvent = 0;

    if(RF_INT_PIN == 0)
    {
        RFIF = 1;
    }

    do        /* need to determine/modify the exit conditions */
    {
        CLRWDT();
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = PHYTasks(ZigBeeStatus.nextZigBeeState);
        }
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = MACTasks(ZigBeeStatus.nextZigBeeState);
        }
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = NWKTasks(ZigBeeStatus.nextZigBeeState);
        }
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = APSTasks(ZigBeeStatus.nextZigBeeState);
        }
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = ZDOTasks(ZigBeeStatus.nextZigBeeState);
        }

        #if I_SUPPORT_STUB_APS == 1
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = StubAPSTasks(ZigBeeStatus.nextZigBeeState);
        }
        #endif

        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = AILTasks(ZigBeeStatus.nextZigBeeState);
        }


        switch(ZigBeeStatus.nextZigBeeState)
        {
            // Check for the primitives that are handled by the PHY.
            case PD_DATA_request:
            case PLME_CCA_request:
            case PLME_ED_request:
            case PLME_SET_request:
            case PLME_GET_request:
            case PLME_SET_TRX_STATE_request:
                ZigBeeStatus.nextZigBeeState = PHYTasks(ZigBeeStatus.nextZigBeeState);
                break;

            // Check for the primitives that are handled by the MAC.
            case PD_DATA_indication:
            case PD_DATA_confirm:
            case PLME_ED_confirm:
            case PLME_GET_confirm:
            case PLME_CCA_confirm:
            case PLME_SET_TRX_STATE_confirm:
            case PLME_SET_confirm:
            case MCPS_DATA_request:
            case MCPS_PURGE_request:
            case MLME_ASSOCIATE_request:
            case MLME_ASSOCIATE_response:
            case MLME_DISASSOCIATE_request:
            case MLME_GET_request:
            case MLME_GTS_request:
            case MLME_ORPHAN_response:
            case MLME_RESET_request:
            case MLME_RX_ENABLE_request:
            case MLME_SCAN_request:
            case MLME_SET_request:
            case MLME_START_request:
            case MLME_SYNC_request:
            case MLME_POLL_request:
                ZigBeeStatus.nextZigBeeState = MACTasks(ZigBeeStatus.nextZigBeeState);
                break;

            // Check for the primitives that are handled by the NWK.
            case MCPS_DATA_confirm:
            case MCPS_DATA_indication:
            case MCPS_PURGE_confirm:
            case MLME_ASSOCIATE_indication:
			{
				ZigBeeStatus.nextZigBeeState = NWKTasks( ZigBeeStatus.nextZigBeeState );
				if(ZigBeeStatus.nextZigBeeState == MLME_ASSOCIATE_request)
				{
					ZigBeeStatus.nextZigBeeState = MACTasks(ZigBeeStatus.nextZigBeeState);
				}
				break;
			}
            case MLME_ASSOCIATE_confirm:
            case MLME_DISASSOCIATE_indication:
            case MLME_DISASSOCIATE_confirm:
            case MLME_BEACON_NOTIFY_indication:
            case MLME_GET_confirm:
            case MLME_GTS_confirm:
            case MLME_GTS_indication:
            case MLME_ORPHAN_indication:
            case MLME_RESET_confirm:
            case MLME_RX_ENABLE_confirm:
            case MLME_SCAN_confirm:
            case MLME_COMM_STATUS_indication:
            case MLME_SET_confirm:
            case MLME_START_confirm:
            case MLME_SYNC_LOSS_indication:
            case MLME_POLL_confirm:
            case NLDE_DATA_request:
            case NLME_NETWORK_DISCOVERY_request:
            case NLME_NETWORK_FORMATION_request:
            case NLME_PERMIT_JOINING_request:
            case NLME_START_ROUTER_request:
            case NLME_JOIN_request:
            case NLME_DIRECT_JOIN_request:
            case NLME_LEAVE_request:
            case NLME_RESET_request:
            case NLME_SYNC_request:
            case NLME_GET_request:
            case NLME_SET_request:
            case NLME_ROUTE_DISCOVERY_request:
                ZigBeeStatus.nextZigBeeState = NWKTasks( ZigBeeStatus.nextZigBeeState );
                break;

            // Check for the primitives that are handled by the APS.
            case NLDE_DATA_confirm:
            case NLDE_DATA_indication:
            case APSDE_DATA_request:
            case APSME_BIND_request:
            case APSME_UNBIND_request:
            case APSME_ESTABLISH_KEY_request:
            case APSME_ESTABLISH_KEY_confirm:
            case APSME_ESTABLISH_KEY_response:
            case APSME_TRANSPORT_KEY_request:
            case APSME_UPDATE_DEVICE_request:
            case APSME_REMOVE_DEVICE_request:
            case APSME_REQUEST_KEY_request:
            case APSME_SWITCH_KEY_request:
            case APSME_ADD_GROUP_request:
            case APSME_REMOVE_GROUP_request:
            case APSME_REMOVE_ALL_GROUPS_request:
                ZigBeeStatus.nextZigBeeState = APSTasks( ZigBeeStatus.nextZigBeeState );
                break;

            /* Cases that are handled by the ZDO */
            case ZDO_DATA_indication:
            case ZDO_BIND_req:
            case ZDO_UNBIND_req:
            case ZDO_END_DEVICE_BIND_req:
            case APP_START_DEVICE_request:
            case APSME_TRANSPORT_KEY_indication:
            case APSME_ESTABLISH_KEY_indication:
            case APSME_UPDATE_DEVICE_indication:
            case APSME_REMOVE_DEVICE_indication:
            case APSME_REQUEST_KEY_indication:
            case APSME_SWITCH_KEY_indication:
            case NLME_NWK_STATUS_indication:

            case NLME_JOIN_indication:
            case NLME_NETWORK_FORMATION_confirm:
            case NLME_JOIN_confirm:
            case NLME_START_ROUTER_confirm:
            case NLME_NETWORK_DISCOVERY_confirm:
            case NLME_LEAVE_indication:
                ZigBeeStatus.nextZigBeeState = ZDOTasks( ZigBeeStatus.nextZigBeeState );
                break;
            #if I_SUPPORT_STUB_APS == 1
            case INTRP_DATA_request:
                ZigBeeStatus.nextZigBeeState = StubAPSTasks(ZigBeeStatus.nextZigBeeState);
                break;
            #endif

            // Check for the primitives that are handled by the AIL.
            case APSDE_DATA_confirm:
            case APSDE_DATA_indication:
                ZigBeeStatus.nextZigBeeState = AILTasks( ZigBeeStatus.nextZigBeeState );
                break;

            // Check for the primitives that are returned to the user.
            case NLME_PERMIT_JOINING_confirm:
            case NLME_DIRECT_JOIN_confirm:
            case NLME_LEAVE_confirm:
            case NLME_RESET_confirm:
            case NLME_SYNC_confirm:
            case NLME_SYNC_LOSS_indication:
            case NLME_GET_confirm:
            case NLME_SET_confirm:
            case APP_LEAVE_indication:
            case NLME_ROUTE_DISCOVERY_confirm:
            case APSME_BIND_confirm:
            case APSME_UNBIND_confirm:
            case APSME_ADD_GROUP_confirm:
            case APSME_REMOVE_GROUP_confirm:
            case APSME_REMOVE_ALL_GROUPS_confirm:
            case APP_START_DEVICE_confirm:
            case APP_NETWORK_DISCOVERY_confirm:
            case INTRP_DATA_confirm:
            case INTRP_DATA_indication:
#if I_SUPPORT_FREQUENCY_AGILITY == 1
    #if APP_RESOLVE_FREQUENCY_AGILITY == 1
            case APP_MGMT_NWK_UPDATE_NOTIFY:
                /* While Ed scan in progress do not process any request */
                if (!ZDOHasBackgroundTasks())
    #endif
#endif
                {
                    appEvent = TRUE;
                }
                break;

            default:
                break;
        }
    } while (ZigBeeStatus.nextZigBeeState != NO_PRIMITIVE && !appEvent);

    *command = ZigBeeStatus.nextZigBeeState;

    if (ZDOHasBackgroundTasks() || APSHasBackgroundTasks() || NWKHasBackgroundTasks() ||
        MACHasBackgroundTasks() || PHYHasBackgroundTasks())
    {
        ZigBeeStatus.flags.bits.bHasBackgroundTasks = 1;
        return TRUE;
    }
    else
    {
        ZigBeeStatus.flags.bits.bHasBackgroundTasks = 0;
        return FALSE;
    }
}
