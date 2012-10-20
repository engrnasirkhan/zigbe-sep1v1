/*********************************************************************
 *
 *                  ZigBee APS Layer
 *
 *********************************************************************
 * FileName:        zAPS.c
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

#include "ZigBeeTasks.h"
#include "sralloc.h"
#include "zAPS.h"
#include "zAPL.h"
#include "zNWK.h"
#include "zMAC.h"
#include "zNVM.h"
#include "generic.h"
#include "merge_info.h"
//#ifdef ZCP_DEBUG
   // #include "console.h"
//#else
//    #define ConsolePutROMString(x)
//    #undef printf
//    #define printf(x)
//    #define PrintChar(x)
//#endif

#ifdef I_SUPPORT_SECURITY
    #include "zSecurity.h"
    #include "zPHY_MRF24J40.h"
#endif
#include "msdlclusterhanddle.h"

// ******************************************************************************
// Configuration Definitions and Error Checks

#ifdef I_AM_RFD		//MSEB: Need to do something to maintain this condition
    #if !defined(RFD_POLL_RATE)
        #error Please define the internal stack message poll rate.
    #endif
#endif


// ******************************************************************************
// Constant Definitions

//-----------------------------------------------------------------------------
// Frame Control Bits
// Frame Type
#define APS_FRAME_DATA                          0x00
#define APS_FRAME_COMMAND                       0x01
#define APS_FRAME_ACKNOWLEDGE                   0x02

// Delivery Mode
#define APS_DELIVERY_DIRECT                     0x00
#define APS_DELIVERY_INDIRECT                   0x01
#define APS_DELIVERY_BROADCAST                  0x02
#define APS_DELIVERY_GROUP                      0x03
#define APS_INDIRECT_ADDRESS_MODE_FROM_COORD    0x00
#define APS_INDIRECT_ADDRESS_MODE_TO_COORD      0x01

// ACK Format
#define APS_ACK_FOR_DATA                        0x00
#define APS_ACK_FOR_COMMAND                     0x01

// Security
#define APS_SECURITY_OFF                        0x00
#define APS_SECURITY_ON                         0x01

// ACK Request
#define APS_ACK_NOT_REQUESTED                   0x00
#define APS_ACK_REQUESTED                       0x01

// Extended Header
#define APS_EXTENDED_HEADER_EXCLUDED            0x00
#define APS_EXTENDED_HEADER_INCLUDED            0x01
// Extended Header Frame Control Fields
#define APS_EXTENDED_HDR_NO_FRAGMENT            0x00
#define APS_EXTENDED_HDR_FIRST_FRAGMENT         0x01
#define APS_EXTENDED_HDR_PART_OF_FRAGMENT       0x02
#define MAX_ADDR_LENGTH                         0x08
//-----------------------------------------------------------------------------


// This value marks whether or not the binding table contains valid
// values left over from before a loss of power.
#define BINDING_TABLE_VALID        0xC35A

#define END_BINDING_RECORDS         0xff

// ******************************************************************************
// Data Structures

#define ONE_MILLISECOND                     ONE_SECOND/1000     //Intepretation for one millisecond

#if NUM_BUFFERED_INDIRECT_MESSAGES > 14
    #error Maximum buffered indirect messages is 14.
#endif
#define INVALID_INDIRECT_RELAY_INDEX      15

#if MAX_APL_FRAMES > 14
    #error Maximum APL messages is 14.
#endif
#define INVALID_APL_INDEX      15

#if apscMaxFrameRetries > 7
    #error apscMaxFrameRetries too large.
#endif

extern struct __MyCommandFlag MyCommandFlag;

typedef struct _INDIRECT_MESSAGE_INFO
{
    WORD_VAL            profileID;
    BYTE                *message;
    APS_FRAME_CONTROL   apsFrameControl;
    BYTE                messageLength;
    BYTE                currentBinding;
    BYTE                sourceEndpoint;     // Used only for messages from upper layers
    union
    {
        BYTE    Val;
        struct
        {
            BYTE    nTransmitAttempts   : 3;
            BYTE    bSendMessage        : 1;
            BYTE    bFromMe             : 1;
           // BYTE    bRouteRepair        : 1;
        } bits;
    } flags;
} INDIRECT_MESSAGE_INFO;



#define APS_BACKGROUND_TASKS 0x1FFF //0x1F


typedef struct _APS_DUPLICATE_TABLE
{
    TICK        StartTick;
    SHORT_ADDR  SrcAddress;
    BYTE        APSCounter;
    BYTE        blockNum;
}APS_DUPLICATE_TABLE;

#if (I_SUPPORT_FRAGMENTATION == 1)
    typedef struct _MISSED_BLOCKS_INFO
    {
        // Used be both Src and Dst devices taking part in the Fragmentation.
        // On Src Device -> Used to store the value of AckBitField in the APS ACK.
        //                  This is required to know whether a particular block/blocks needs to be retried.
        // On Dst Device -> Used for tracking which all blocks in the window has been recieved.
        //                  This is required to set the AckBitField in the APS ACK.
        BYTE    AckBitField;
        // This is used to know which block is being retried.
        BYTE    RetriedBlockNumber;
    } MISSED_BLOCKS_INFO;

    typedef struct _APS_FRAGMENTATION_INFO
    {
        // Used by both Src and Dst Devices taking part in Fragmentation.
        // On Src Device -> This field will have the Destination Address.
        // On Dst Device -> This field will have the Source Address from which Fragment packet has been received.
        ADDR                FragmentDstAddr;
        // These fields are required by both Src and Dst Devices taking part in Fragmentation.
        // On Src Device -> Used for transmitting the blocks.
        // On Dst Device -> Used for sending indication after receiving all the blocks.
        WORD_VAL            ProfileId;
        WORD_VAL            ClusterId;
        BYTE                SrcEndpoint;
        BYTE                DstEndpoint;
        BYTE                asduLength;
        BYTE                *asdu;
        BYTE                SecurityStatus;
        BYTE                LinkQuality;
        BYTE                APSCounter;
        union _FRAGMENT_TX_OPTIONS
        {
            BYTE    Val;
            struct _FRAGMENT_TX_OPTIONS_BITS
            {
                BYTE    securityEnabled : 1;
                BYTE    useNWKKey       : 1;
                BYTE    acknowledged    : 1;
            } bits;
        }           FragmentTxOptions;
        // Used by both Src and Dst Devices taking part in Fragmentation.
        // On Src Device -> Stores the Total Number blocks that needs to be transmitted.
        // On Dst Device -> Stores the Total Number blocks that needs to be received.
        BYTE                TotalNumberOfBlocks;
        // Holds the Total Data Length that needs to be fragmented.
        WORD                DataPayloadLength;
        // Points to actual application payload
        BYTE                *DataPayloadAddress;
        // Used on Dst Device for sending Data Indication
        WORD                PreviousDataPayloadLength;
        // Specifies the number of blocks that needs to be transmitted per window.
        // Used by both Src and Dst Devices taking part in Fragmentation.
        BYTE                WindowSize;
        // Used by both Src and Dst Devices taking part in Fragmentation.
        // On Src Device -> Stores the Next block that needs to be transmitted.
        // On Dst Device -> Stores the Next block that needs to be received.
        BYTE                BlockNumberToTxOrRx;
        // Used by both Src and Dst Devices taking part in Fragmentation.
        // On Src Device -> Stores the First block that needs to be transmitted.
        // On Dst Device -> Stores the First block that needs to be received.
        BYTE                FirstBlockInWindow;
        // Used by both Src and Dst Devices taking part in Fragmentation.
        // On Src Device -> Stores the Last block that needs to be transmitted.
        // On Dst Device -> Stores the Last block that needs to be received.
        BYTE                LastBlockInWindow;
        // Used by both Src and Dst Devices taking part in Fragmentation.
        // On Src Device -> Used to know whether the block has been retried.
        // On Dst Device -> Used to know whether the retried block has been received.
        BYTE                IsThisBlockRetried;
        // A delay between the transmission of each block.
        TICK                InterframeDelayTick;
        // A delay before movind on to next window
        TICK                APSAckDelayTick;
        // These field are used for cleaning up the buffer in case all the blocks were not received with in the specified time.
        TICK                FragmentationStartTime;
        TICK                TotalFragmentationTime;
        // This is used to check whether the block transmission should be continued or wait for APS ACK.
        BYTE                WaitingForAck;
        // This is used to check whether the APS ACK is received for block specified by this parameter.
        BYTE                ExpectingAckForTheBlockNumber;
        // A counter which will keep track of retries. Useful for cleanup when Fragment data transmission fails.
        BYTE                RetryCounter;
        // Used by both Src and Dst Devices taking part in Fragmentation.
        // On Src Device -> This will be set to TRUE.
        // On Dst Device -> This will be set to FALSE.
        BYTE                FragmentTxInitiator;
        // Used by the Dst Device. This will specify that all the blocks have been received, so that indication can be sent up.
        BYTE                AllFragmentsReceived;
        MISSED_BLOCKS_INFO  MissedBlockInfo;
    }APS_FRAGMENTATION_INFO;
#endif //(I_SUPPORT_FRAGMENTATION == 1)



// ******************************************************************************
// Variable Definitions

BYTE            APSCounter;

#if I_SUPPORT_LINK_KEY == 1
    TC_LINK_KEY_INFO TCLinkKeyInfo;
    APS_KEY_PAIR_DESCRIPTOR currentAppLinkKeyTable;
#endif

DWORD_VAL   apsChannelMask;

APS_STATUS      apsStatus;
APS_FRAMES      *apsConfirmationHandles[MAX_APS_FRAMES];

#if (I_SUPPORT_FRAGMENTATION == 1)
    APS_FRAGMENTATION_INFO  apsFragmentationInfo;
#endif //(I_SUPPORT_FRAGMENTATION == 1)

#if !defined(__C30__) && !defined(__C32__)
    #pragma udata DUP_TABLE=0xc00
#endif
        APS_DUPLICATE_TABLE apsDuplicateTable[MAX_DUPLICATE_TABLE];
#if !defined(__C30__) && !defined(__C32__)
    #pragma udata
#endif

#ifdef I_SUPPORT_SECURITY
    KEY_VAL     KeyVal;
    extern SECURITY_STATUS  securityStatus;
    TICK        AuthorizationTimeout;
    TICK        WaitForAuthentication;
//    #ifdef I_AM_RFD
        extern volatile PHY_PENDING_TASKS  PHYTasksPending;
        TICK    lastPollTime;
//    #endif
    LONG_ADDR       transportKeySrcAddr;
#endif
extern volatile TX_STAT TxStat;

#if (I_SUPPORT_FRAGMENTATION == 1)
    BYTE        fragmentWindowSize;         // Total number of blocks that can be transmitted in a window
    BYTE        fragmentInterframeDelay;    // Time delay between the blocks transmissions
    BYTE        fragmentDataSize;           // DataLength per block
    WORD        fragmentTotalDataLength;    // Total DataLength
    #if (I_SUPPORT_FRAGMENTATION_TESTING == 1)
        BYTE        TestErrorWindow;        // Window on which block should not transmitted. (Used for testing only)
        BYTE        TestErrorPacket;        // Bit field which specifies particular block/blocks that should not be transmitted.
                                            // 0x01 -> First block in the window should not be transmitted
                                            // 0x02 -> Second block in the window should not be transmitted
                                            // 0x03 -> First and Second ... and so on.
    #endif //(I_SUPPORT_FRAGMENTATION_TESTING == 1)
#endif //(I_SUPPORT_FRAGMENTATION == 1)

#if I_SUPPORT_FREQUENCY_AGILITY == 1
    extern WORD_VAL TotalTransmittedPackets;
    extern WORD_VAL TotalTransmitFailures;
#endif

// ******************************************************************************
// Function Prototypes
void FormatTxBuffer( void );
void initAPPLinkKeyTable();
#if MAX_APS_ADDRESSES > 0
    BOOL LookupAPSAddress( LONG_ADDR *longAddr );
    BOOL LookupAPSLongAddress(INPUT SHORT_ADDR *shortAddr);
    BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap);
#endif
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
    BYTE LookupSourceBindingInfo( SHORT_ADDR srcAddr, BYTE srcEP, WORD_VAL clusterID );
    void RemoveAllBindings( SHORT_ADDR shortAddr );
#endif

#if defined(I_SUPPORT_GROUP_ADDRESSING)
    GROUP_ADDRESS_RECORD currentGroupAddressRecord;
    BYTE    GetEmptyGroup(void);
#endif /*I_SUPPORT_GROUP_ADDRESSING*/

#ifdef I_SUPPORT_SECURITY
    BOOL    APSFillSecurityRequest(INPUT LONG_ADDR *DestAddr, INPUT BOOL bSecuredFrame,
WORD NwkDstAddress);
#endif
BOOL    DuplicatePacket(INPUT SHORT_ADDR SrcAddress, INPUT BYTE currentAPSCounter,INPUT BYTE currentBlockNum);
extern BOOL NWKThisIsMyLongAddress(LONG_ADDR *);
extern NEIGHBOR_KEY NWKLookupNodeByShortAddrVal( WORD shortAddrVal );

extern ZDO_STATUS zdoStatus;
BYTE    QuickPoll  = 0;
BYTE    SentBindRequest = 0;

#ifdef I_SUPPORT_SECURITY
    void TunnelTransportKey( LONG_ADDR *deviceAddress, SHORT_ADDR parentShortAddress);
    BOOL    firstKeyHasBeenSent = FALSE;
    BOOL    previousKeyNotSeq0;

    //#if defined(I_AM_COORDINATOR) || defined(I_AM_ROUTER)
        APS_ADDRESS_MAP updateDevAPSAddr;
    //#endif

    #if defined(USE_EXTERNAL_NVM)
        extern NETWORK_KEY_INFO plainSecurityKey[2];
        extern BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey);
        extern BOOL InitSecurityKey(void);
    #endif
//    #ifdef I_AM_TRUST_CENTER
        extern void GetHashKey(LONG_ADDR *longAddress1, LONG_ADDR *longAddress2, KEY_VAL *key);
//    #endif
#endif

//#if defined(MERGED_STACK) || defined (I_AM_COORDINATOR)
    APS_ADDRESS_MAP currentAPSAddress1;
//#endif

#if (I_SUPPORT_FRAGMENTATION == 1)
    static void SendFragmentDataConfirm (BYTE Status);
    static void RetryBlock (BYTE AllBlocksInWindow);
    static BYTE GetTotalNumberOfBlocks (void);
    static void UpdateDataPayloadLengthAndAddress ( void );
    static ZIGBEE_PRIMITIVE UpdateFragmentationInfoFields
                                (
                                    BYTE        fragmentType,
                                    BYTE        blockNumber,
                                    WORD_VAL    clusterID,
                                    WORD_VAL    profileID,
                                    BYTE        dstEndPoint,
                                    BYTE        srcEndPoint,
                                    BYTE        apsCounter,
                                    SHORT_ADDR  ackDstAddr
                                );
    static ZIGBEE_PRIMITIVE SendAPSACKForFragmentedData (BYTE blockNumber, BYTE ackBitField);
#endif //(I_SUPPORT_FRAGMENTATION == 1)

#if I_SUPPORT_LINK_KEY == 1
BYTE ApplyAPSSecurityToAPSACK( void );
#endif


/*********************************************************************
 * Function:        BOOL APSHasBackgroundTasks( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - APS layer has background tasks to run
 *                  FALSE - APS layer does not have background tasks
 *
 * Side Effects:    None
 *
 * Overview:        Determines if the APS layer has background tasks
 *                  that need to be run.
 *
 * Note:            None
 ********************************************************************/

BOOL APSHasBackgroundTasks( void )
{
    return ((apsStatus.flags.Val & APS_BACKGROUND_TASKS) != 0);
}


/*********************************************************************
 * Function:        void APSInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    APS layer data structures are initialized.
 *
 * Overview:        This routine initializes all APS layer data
 *                  structures.
 *
 * Note:            This routine is intended to be called as part of
 *                  a network or power-up initialization.  If called
 *                  after the network has been running, heap space
 *                  may be lost unless the heap is also reinitialized.
 ********************************************************************/

void APSInit( void )
{
    BYTE        i;
    #if (I_SUPPORT_FRAGMENTATION == 1)
        NODE_DESCRIPTOR     nodeDesc;
    #endif //(I_SUPPORT_FRAGMENTATION == 1)

    #if defined(I_SUPPORT_BINDINGS)
        WORD    key;
    #endif

    apsStatus.flags.Val = 0;

    // Initialize APS frame handles awaiting confirmation.
    for (i=0; i<MAX_APS_FRAMES; i++)
    {
        apsConfirmationHandles[i] = NULL;
    }

    // Initialize the buffered APL message pointers.
    for (i=0; i<MAX_APL_FRAMES; i++)
    {
        apsStatus.aplMessages[i] = NULL;
    }

    // Initialize the buffered Group pointers
    #ifdef I_SUPPORT_GROUP_ADDRESSING
        for( i = 0; i < MAX_GROUP_RECORD_BUFFER; i++)
        {
            apsStatus.apsGroupRecord[i] = NULL;
        }
    #endif

    // Initialize the duplicate table
    for(i = 0; i < MAX_DUPLICATE_TABLE; i++)
    {
        apsDuplicateTable[i].SrcAddress.Val = 0xFFFF;
    }

    #if defined(I_SUPPORT_BINDINGS)

        GetBindingValidityKey( &key );
        if (key != BINDING_TABLE_VALID)
        {
            key = BINDING_TABLE_VALID;
            PutBindingValidityKey( &key );
            ClearBindingTable();
        }
    #endif
    #if MAX_APS_ADDRESSES > 0
    {
        WORD ValidKey;
        GetAPSAddressValidityKey(&ValidKey);
        if (ValidKey != apsMAGICValid)
        {
            APSClearAPSAddressTable();
        }
    }
    #endif
    APSCounter = 0;
    #ifdef I_SUPPORT_SECURITY
        if (NOW_I_AM_NOT_A_CORDINATOR()){//#if defined(MERGED_STACK) || !defined (I_AM_COORDINATOR)
            securityStatus.flags.bits.bAuthorization = FALSE;
        }//#endif
    #endif

    apsChannelMask.Val = ALLOWED_CHANNELS;

    #if (I_SUPPORT_FRAGMENTATION == 1)
        ProfileGetNodeDesc(&nodeDesc);
        fragmentWindowSize = PROFILE_FRAGMENT_WINDOW_SIZE;
        fragmentInterframeDelay = PROFILE_FRAGMENT_INTERFRAME_DELAY;
        fragmentDataSize = nodeDesc.NodeMaxBufferSize;
        fragmentTotalDataLength = nodeDesc.NodeMaxOutgoingTransferSize.v[0];
        #if (I_SUPPORT_FRAGMENTATION_TESTING == 1)
            TestErrorWindow = 0x00;
            TestErrorPacket = 0xFF;
        #endif //(I_SUPPORT_FRAGMENTATION_TESTING == 1)

        apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val = 0xFFFF;

    #endif //(I_SUPPORT_FRAGMENTATION == 1)
}

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE APSTasks(ZIGBEE_PRIMITIVE inputPrimitive)
 *
 * PreCondition:    None
 *
 * Input:           inputPrimitive - the next primitive to run
 *
 * Output:          The next primitive to run.
 *
 * Side Effects:    Numerous
 *
 * Overview:        This routine executes the indicated ZigBee primitive.
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
extern NWK_STATUS nwkStatus;
ZIGBEE_PRIMITIVE APSTasks(ZIGBEE_PRIMITIVE inputPrimitive)
{
    BYTE    i;
    BYTE    j;
    BYTE    *ptr;
    #ifdef I_SUPPORT_SECURITY

    #if PROFILE_nwkSecureAllFrames
    BYTE    apsHeaderpos;

    #endif
    #endif
    if (inputPrimitive == NO_PRIMITIVE)
    {
        // Perform background tasks
        TICK currentTime = TickGet();

        // Handle responder authorization
        #if defined(I_SUPPORT_SECURITY)
        if(NOW_I_AM_NOT_A_CORDINATOR()){//#if defined(I_SUPPORT_SECURITY) && (defined(MERGED_STACK) || !defined (I_AM_COORDINATOR))
            if (securityStatus.flags.bits.bAuthorization)
            {
                //if( TickGetDiff( currentTime, AuthorizationTimeout) > AUTHORIZATION_TIMEOUT)
                if( TickGetDiff( currentTime, AuthorizationTimeout) > WaitForAuthentication.Val )
                {
                    //ConsolePutROMString((ROM char *)"Authorization timeout\r\n");
                    /* Added at NTS - reset this bAuthorization flag in the NLME_LEAVE primitive */
                      if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        PHYTasksPending.bits.PHY_AUTHORIZE = 0;
                    }//#endif
                    //GetMACAddress(&(params.NLME_LEAVE_request.DeviceAddress));
                    for(i=0; i < 8; i++)
                    {
                        /* force a self initiated leave */
                        params.NLME_LEAVE_request.DeviceAddress.v[i] = 0x00;
                    }
                    params.NLME_LEAVE_request.RemoveChildren = TRUE;
                    params.NLME_LEAVE_request.Rejoin         = FALSE;
                    params.NLME_LEAVE_request.Silent         = TRUE;
                    params.NLME_LEAVE_request.ReuseAddress   = FALSE;
                    return NLME_LEAVE_request;
                }
                if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                    if (TickGetDiff( currentTime, lastPollTime ) > ONE_SECOND * 0.5)
                    {
                        // Send a data request message so we can try to receive our Key
                        lastPollTime = currentTime;
                        params.NLME_SYNC_request.Track = FALSE;
                        return NLME_SYNC_request;
                    }
                }//#endif
            }
        }
        #endif//I_SUPPORT_SECURITY

        #if (I_SUPPORT_LINK_KEY == 1)
        
           // if (apsStatus.flags.bits.bSaveOutgoingFrameCounter == 1)
           // {
           //     apsStatus.flags.bits.bSaveOutgoingFrameCounter = 0;
           //     StorePersistancePIB(); //Store the OutgoingFrameCounter, LinkKey, and TrustCenter Information
           // }
        
        #endif
        // ---------------------------------------------------------------------
        // Handle pending data indication from an ACK request
        if (apsStatus.flags.bits.bDataIndicationPending)
        {
            // If we have't finished the last transmission, don't start processing the message.
            // Otherwise, we'll never see the MAC ACK for the APS ACK
            if (ZigBeeReady())
            {
                if ( apsStatus.flags.bits.bLoopBackData )
                {
                    //apsStatus.flags.bits.bLoopBackData = 0;
                    apsStatus.flags.bits.bFreeAllocatedAsdu = 1;
                }
                apsStatus.flags.bits.bDataIndicationPending = 0;
                if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                    ZigBeeStatus.flags.bits.bRequestingData = 0;
                }//#endif
                params.APSDE_DATA_indication.asduLength             = apsStatus.ackMessage.asduLength;
                params.APSDE_DATA_indication.asdu                   = apsStatus.ackMessage.asdu;
                params.APSDE_DATA_indication.ProfileId              = apsStatus.ackMessage.ProfileId;
                params.APSDE_DATA_indication.SrcAddrMode            = apsStatus.ackMessage.SrcAddrMode;
                params.APSDE_DATA_indication.WasBroadcast           = apsStatus.ackMessage.WasBroadcast;
                params.APSDE_DATA_indication.SrcAddress.ShortAddr   = apsStatus.ackMessage.SrcAddress;
                params.APSDE_DATA_indication.SrcEndpoint            = apsStatus.ackMessage.SrcEndpoint;
                params.APSDE_DATA_indication.DstEndpoint            = apsStatus.ackMessage.DstEndpoint;
                params.APSDE_DATA_indication.ClusterId.Val          = apsStatus.ackMessage.ClusterId.Val;
                params.APSDE_DATA_indication.SecurityStatus         = apsStatus.ackMessage.SecurityStatus;
                params.APSDE_DATA_indication.LinkQuality	        = apsStatus.ackMessage.LinkQuality;                
                params.APSDE_DATA_indication.DstAddrMode            = APS_ADDRESS_16_BIT;
                params.APSDE_DATA_indication.DstAddress.ShortAddr   =apsStatus.ackMessage.DstAddress;
                if (params.APSDE_DATA_indication.DstEndpoint == 0)
                {
                    return ZDO_DATA_indication;
                }
                else
                {
                    return APSDE_DATA_indication;
                }
            }
        }

        // --------------------------------------------------------------------
        // CAUTION: DO NOT CHANGE THE ORDER OF EXECUTION. THIS CODE SHOULD BE AFTER bDataIndicationPending CHECK
        /* Check whether asdu needs be freed, if allocated. This will happen in case of LoopBack data,
           i.e, device sending data to itself. */
        if ( apsStatus.flags.bits.bFreeAllocatedAsdu )
        {
            apsStatus.flags.bits.bFreeAllocatedAsdu = 0;
            nfree( apsStatus.ackMessage.asdu );
        }

        // --------------------------------------------------------------------
        // Handle Group addressing
        #ifdef I_SUPPORT_GROUP_ADDRESSING
            if( apsStatus.flags.bits.bGroupAddressing)
            {
                for(i = 0; i < MAX_GROUP_RECORD_BUFFER; i++)
                {
                    if( apsStatus.apsGroupRecord[i] != NULL )
                    {
                        if( apsStatus.apsGroupRecord[i]->EndPoints[apsStatus.apsGroupRecord[i]->EndPointIndex] == 0xFF )
                        {
                            // this is the end of the group address map
                            nfree(apsStatus.apsGroupRecord[i]->CurrentRxPacket);
                            nfree(apsStatus.apsGroupRecord[i]);
                        }
                        else
                        {
                            params.APSDE_DATA_indication.DstEndpoint        = apsStatus.apsGroupRecord[i]->EndPoints[apsStatus.apsGroupRecord[i]->EndPointIndex++];
                            params.APSDE_DATA_indication.SrcAddrMode        = apsStatus.apsGroupRecord[i]->SrcAddrMode;
                            params.APSDE_DATA_indication.SrcAddress         = apsStatus.apsGroupRecord[i]->SrcAddress;
                            params.APSDE_DATA_indication.SrcEndpoint        = apsStatus.apsGroupRecord[i]->SrcEndpoint;
                            params.APSDE_DATA_indication.ProfileId.Val      = apsStatus.apsGroupRecord[i]->ProfileId.Val;
                            params.APSDE_DATA_indication.ClusterId.Val      = apsStatus.apsGroupRecord[i]->ClusterId.Val;
                            params.APSDE_DATA_indication.asduLength         = apsStatus.apsGroupRecord[i]->asduLength;
                            params.APSDE_DATA_indication.asdu               = apsStatus.apsGroupRecord[i]->asdu;
                            params.APSDE_DATA_indication.WasBroadcast       = TRUE;
                            params.APSDE_DATA_indication.SecurityStatus     = apsStatus.apsGroupRecord[i]->SecurityStatus;
                            params.APSDE_DATA_indication.DstAddrMode        = APS_ADDRESS_GROUP;
                            // Set CurrentRxPacket to NULL so that the device can receive new packets.
                            CurrentRxPacket                                 = NULL;
                            return APSDE_DATA_indication;
                        }
                    }
                }

                apsStatus.flags.bits.bGroupAddressing = 0;
            }
        #endif

        // Handle duplicate table expiration
        if (apsStatus.flags.bits.bDuplicateTable)
        {
            BOOL isValid = FALSE;

            for(i = 0; i < MAX_DUPLICATE_TABLE; i++)
            {
                if( apsDuplicateTable[i].SrcAddress.Val != 0xFFFF )
                {
                    if( TickGetDiff(currentTime, apsDuplicateTable[i].StartTick) > ((ONE_SECOND) * ((DWORD)DUPLICATE_TABLE_EXPIRATION)))
                    {
                        apsDuplicateTable[i].SrcAddress.Val = 0xFFFF;
                    }
                    else
                    {
                        isValid = TRUE;
                    }
                }
            }
            if( isValid == FALSE )
            {
                apsStatus.flags.bits.bDuplicateTable = 0;
            }
        }

        // ---------------------------------------------------------------------
        // Handle frames awaiting sending (or resending) from upper layers
        if (apsStatus.flags.bits.bFramesAwaitingTransmission)
        {
            #if I_SUPPORT_LINK_KEY == 1
                KEY_INFO    linkKeyDetails;
                BYTE        *linkKey;
                KEY_VAL     linkKeyToBeUsed;
            #endif


            #ifndef PROFILE_nwkSecureAllFrames
                KEY_INFO ActiveNetworkKey;
            #endif
            //#ifdef I_AM_RFD
            if ( (NOW_I_AM_A_RFD() && ZigBeeReady() && ZigBeeStatus.flags.bits.bDataRequestComplete)
            //#else
             || (NOW_I_AM_NOT_A_RFD()&&ZigBeeReady())
            )//#endif
			{
                for (i=0; i<MAX_APL_FRAMES; i++)
                {
                    if (apsStatus.aplMessages[i] != NULL)
                    {
                        if (apsStatus.aplMessages[i]->flags.bits.bSendMessage)
                        {
                            BYTE    cIndex;

                            cIndex = apsStatus.aplMessages[i]->confirmationIndex;

                            // If we've run out of retries, destroy everything and send up a confirm.
                            if (apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts == 0)
                            {
                                // We have run out of transmit attempts.  Prepare the confirmation primitive.
                                params.APSDE_DATA_confirm.Status        = apsStatus.aplMessages[i]->status;
                                params.APSDE_DATA_confirm.DstAddrMode   = apsConfirmationHandles[cIndex]->DstAddrMode;
                                params.APSDE_DATA_confirm.DstAddress    = apsConfirmationHandles[cIndex]->DstAddress;
                                params.APSDE_DATA_confirm.SrcEndpoint   = apsConfirmationHandles[cIndex]->SrcEndpoint;
                                params.APSDE_DATA_confirm.DstEndpoint   = apsConfirmationHandles[cIndex]->DstEndpoint;
                                params.APSDE_DATA_confirm.TxTime        = apsConfirmationHandles[cIndex]->timeStamp;

                                // Clean up everything.
                                if (apsStatus.aplMessages[i]->message != NULL)
                                {
                                    nfree( apsStatus.aplMessages[i]->message );
                                }
                                nfree( apsStatus.aplMessages[i] );
                                nfree( apsConfirmationHandles[cIndex] );
                                return APSDE_DATA_confirm;
                            }

                            // We still have retries left.
                            // Load the primitive parameters.
                            params.NLDE_DATA_request.BroadcastRadius = apsStatus.aplMessages[i]->radiusCounter;
                            
                            params.NLDE_DATA_request.DiscoverRoute   = apsStatus.aplMessages[i]->flags.bits.nDiscoverRoute;
                            params.NLDE_DATA_request.NsduLength      = apsStatus.aplMessages[i]->messageLength;
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                if( apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP )
                                {
                                    #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                        PERSISTENCE_PIB currentPIB;

                                        GetPersistenceStorage((void *)&currentPIB);
                                        if ( currentPIB.nwkUseMulticast )
                                        {
                                            // Important
                                            apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode = APS_DELIVERY_BROADCAST;

                                            params.NLDE_DATA_request.DstAddrMode = 0x01; // Needed for NWK Layer Multicast
                                            params.NLDE_DATA_request.DstAddr = apsStatus.aplMessages[i]->shortDstAddress;
                                            params.NLDE_DATA_request.MulticastMode = apsStatus.aplMessages[i]->EnableNwkLayerMulticast;
                                        }
                                        else
                                    #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                        {
                                            params.NLDE_DATA_request.DstAddr.Val = 0xFFFD; // Group data should be broadcasted only to RxOn devices
                                        }
                                    apsStatus.aplMessages[i]->apsFrameControl.bits.acknowledgeRequest = 0;
                                }
                                else
                            #endif
                                {
                                    params.NLDE_DATA_request.DstAddr         = apsStatus.aplMessages[i]->shortDstAddress;
                                    params.NLDE_DATA_request.DstAddrMode = 0x02; // Needed for NWK Layer Multicast
                                }
                            params.NLDE_DATA_request.NsduHandle      = NLME_GET_nwkBCSN();

                            // Update the confirmation queue entry.  The entry already exists from when we received
                            // the original request.  We just need to fill in the nsduHandle and timeStamp.
                            apsConfirmationHandles[cIndex]->nsduHandle = params.NLDE_DATA_request.NsduHandle;
                            apsConfirmationHandles[cIndex]->timeStamp  = TickGet();
                            #ifdef I_SUPPORT_SECURITY

                                #if PROFILE_nwkSecureAllFrames
                                    apsHeaderpos = TxData;
                                    TxBuffer[apsHeaderpos] = apsStatus.aplMessages[i]->apsFrameControl.Val;
                                    #if I_SUPPORT_LINK_KEY == 1
                                        /*  check whether data has to be encrypted with link key */
                                        if( apsStatus.aplMessages[i]->apsFrameControl.bits.security )
                                        {
                                            apsHeaderpos = TxHeader - 7;
                                            TxBuffer[apsHeaderpos] = apsStatus.aplMessages[i]->apsFrameControl.Val;
                                        }
                                     #endif
                                    apsHeaderpos++;
                                    if (apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_INDIRECT &&
                                        apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_GROUP)
                                    {
                                        TxBuffer[apsHeaderpos++] = apsConfirmationHandles[cIndex]->DstEndpoint;
                                    }
                                    #ifdef I_SUPPORT_GROUP_ADDRESSING
                                        if( apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)
                                        {
                                            #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                                PERSISTENCE_PIB currentPIB;

                                                GetPersistenceStorage((void *)&currentPIB);
                                                if ( !currentPIB.nwkUseMulticast )
                                            #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                                {
                                                    TxBuffer[apsHeaderpos++] = apsStatus.aplMessages[i]->shortDstAddress.byte.LSB;
                                                    TxBuffer[apsHeaderpos++] = apsStatus.aplMessages[i]->shortDstAddress.byte.MSB;
                                                }
                                        }
                                    #endif
                                    TxBuffer[apsHeaderpos++] = apsStatus.aplMessages[i]->clusterID.byte.LB;
                                    TxBuffer[apsHeaderpos++] = apsStatus.aplMessages[i]->clusterID.byte.HB;
                                    TxBuffer[apsHeaderpos++] = apsStatus.aplMessages[i]->profileID.byte.LB;
                                    TxBuffer[apsHeaderpos++] = apsStatus.aplMessages[i]->profileID.byte.HB;
                                    TxBuffer[apsHeaderpos++] = apsConfirmationHandles[cIndex]->SrcEndpoint;
                                    TxBuffer[apsHeaderpos++] = apsStatus.aplMessages[i]->APSCounter;
                                    if( apsStatus.aplMessages[i]->apsFrameControl.bits.security )
                                    {
                                        /* We have to updated TxData */
                                        TxHeader = apsHeaderpos - 9;
                                    }
                                    else
                                    {
                                        TxData = apsHeaderpos;
                                    }
                                    for (j=0; j<params.NLDE_DATA_request.NsduLength; j++)
                                    {
                                        TxBuffer[TxData++] = apsStatus.aplMessages[i]->message[j];

                                    }
                                    params.NLDE_DATA_request.SecurityEnable =
                                            apsStatus.aplMessages[i]->flags.bits.nwkSecurity;

                                    #if I_SUPPORT_LINK_KEY == 1
                                        if( apsStatus.aplMessages[i]->apsFrameControl.bits.security )
                                        {
                                            #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                                #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                                                    TC_LINK_KEY_TABLE   tcLinkKeyTable;
                                                    LONG_ADDR           dstLongAddrReverse;
                                                #endif
                                            #endif

                                            //#ifdef I_AM_TRUST_CENTER
                                                LONG_ADDR dstLongAddress;
                                                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                                                    LONG_ADDR   dstLongAddrReverse;
                                                    LONG_ADDR   SrcAddr;
                                                    BYTE k;
                                                #endif
											if(I_AM_TRUST_CENTER){
                                                if(( IsThisAddressKnown ( params.NLDE_DATA_request.DstAddr, &dstLongAddress.v[0]))
                                                && (params.NLDE_DATA_request.DstAddr.Val < 0xFFFB ))
                                                {
                                                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                                                        GetMACAddress(&SrcAddr);
                                                        for(k=0; k < 8 ; k++)
                                                        {
                                                            dstLongAddrReverse.v[k] = dstLongAddress.v[7 - k];
                                                        }
                                                        GetHashKey(&dstLongAddrReverse, &SrcAddr, &linkKeyToBeUsed);
                                                    #endif

                                                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                                        BYTE count;
                                                        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1

                                                            for(count = 0; count < 8; count++)
                                                            {
                                                                dstLongAddrReverse.v[count] = dstLongAddress.v[7 - count];
                                                            }
                                                            if ( SearchForTCLinkKey(dstLongAddrReverse, &tcLinkKeyTable) )
                                                            {
                                                                for (count = 0; count < KEY_LENGTH; count++)
                                                                {
                                                                    linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                                                }
                                                            }
                                                            else
                                                            {
                                                                apsStatus.aplMessages[i]->status = APS_SECURITY_FAIL;
                                                                apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts = 0;
                                                                ZigBeeUnblockTx();
                                                                return NO_PRIMITIVE;
                                                            }
                                                        #else
                                                            for(count = 0; count < KEY_LENGTH; count++)
                                                            {
                                                                linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                                                            }
                                                        #endif
                                                    #endif

                                                    linkKey = &linkKeyToBeUsed.v[0];
                                                    linkKeyDetails.frameCounter = TCLinkKeyInfo.frameCounter;
                                                    TCLinkKeyInfo.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val +1;
                                                    apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
                                                }
                                                else
                                                {
                                                    linkKey = NULL;
                                                }

            								}else{//#else
                                                if( TCLinkKeyInfo.trustCenterShortAddr.Val
                                                 == params.NLDE_DATA_request.DstAddr.Val)
                                                {
                                                    BYTE count;
                                                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                                        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1

                                                            for(count = 0; count < 8; count++)
                                                            {
                                                                dstLongAddrReverse.v[count] = TCLinkKeyInfo.trustCenterLongAddr.v[count];
                                                            }
                                                            if ( SearchForTCLinkKey(dstLongAddrReverse, &tcLinkKeyTable) )
                                                            {
                                                                for (count = 0; count < KEY_LENGTH; count++)
                                                                {
                                                                    linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                                                }
                                                            }
                                                            else
                                                            {
                                                                apsStatus.aplMessages[i]->status = APS_SECURITY_FAIL;
                                                                apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts = 0;
                                                                ZigBeeUnblockTx();
                                                                return NO_PRIMITIVE;
                                                            }
                                                        #else
                                                            for(count = 0; count < KEY_LENGTH; count++)
                                                            {
                                                                linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                                                            }
                                                        #endif
                                                    #else
                                                        for(count = 0; count < KEY_LENGTH; count++)
                                                        {
                                                            linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                                                        }
                                                    #endif

                                                    linkKeyDetails.frameCounter = TCLinkKeyInfo.frameCounter;
                                                    linkKey = &linkKeyToBeUsed.v[0];
                                                    TCLinkKeyInfo.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val +1;
                                                    apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
                                                }
                                                else
                                                {
                                                    LONG_ADDR dstLongAddress;
                                                    if((IsThisAddressKnown ( params.NLDE_DATA_request.DstAddr, &dstLongAddress.v[0]))
                                                    && (params.NLDE_DATA_request.DstAddr.Val < 0xFFFB ))
                                                    {

                                                        APS_KEY_PAIR_DESCRIPTOR appLinkKeyDetails;
                                                        BYTE count;
                                                        for(count=0; count < 8; count++)
                                                        {
                                                            appLinkKeyDetails.PartnerAddress.v[count] = dstLongAddress.v[count];

                                                        }
                                                        /*0x01 means the Link Key is used for Encryption. In this case,
                                                        the IncomingFrameCounter should be incremented and stored onto NVM*/
                                                        if( SearchForLinkKey(&appLinkKeyDetails, 0x01) )
                                                        {
                                                            linkKey = &appLinkKeyDetails.Link_Key.v[0];
                                                            linkKeyDetails.frameCounter = appLinkKeyDetails.IncomingFrameCounter;
                                                        }
                                                        else
                                                        {
                                                            linkKey = NULL;
                                                        }
                                                    }
                                                    else
                                                    {
                                                       linkKey = NULL;
                                                    }

                                                }
                    					}//#endif     // #ifdef I_AM_TRUST_CENTER
                                            if(linkKey != NULL)
                                            {
                                                linkKeyDetails.key = linkKey;
                                                linkKeyDetails.keyId = ID_LinkKey;
                                                if (
                                                        !DataEncrypt
                                                        (
                                                            TxBuffer,
                                                            &TxData,
                                                            &(TxBuffer[TxHeader+1]),
                                                            (TX_HEADER_START-TxHeader),
                                                            &linkKeyDetails, FALSE
                                                         )
                                                     )

                                                {
                                                    ZigBeeUnblockTx();
                                                    return NO_PRIMITIVE;
                                                }
                                                else
                                                {
                                                    FormatTxBuffer();
                                                }

                                            //  TxData = TxData + 4;
                                            }
                                            else
                                            {
                                                apsStatus.aplMessages[i]->status = APS_SECURITY_FAIL;
                                                apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts = 0;
                                                ZigBeeUnblockTx();
                                                return NO_PRIMITIVE;
                                            }

                                        }
                                    #else
                                        /*Link Key security is requested but Link Key support is disabled.
                                        So, reject the frame and notify the application.*/
                                        if( apsStatus.aplMessages[i]->apsFrameControl.bits.security )
                                        {
                                            apsStatus.aplMessages[i]->status = APS_SECURITY_FAIL;
                                            apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts = 0;
                                            ZigBeeUnblockTx();
                                            return NO_PRIMITIVE;
                                        }
                                    #endif /*#if I_SUPPORT_LINK_KEY == 1*/
                                #else
                                    // Load the APS Payload.
                                    for (j=0; j<params.NLDE_DATA_request.NsduLength; j++)
                                    {
                                        TxBuffer[TxData++] = apsStatus.aplMessages[i]->message[j];
                                    }

                                    // Load the APS Header (backwards).
                                    TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->APSCounter;
                                    TxBuffer[TxHeader--] = apsConfirmationHandles[cIndex]->SrcEndpoint;
                                    TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->profileID.byte.MSB;
                                    TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->profileID.byte.LSB;
                                    TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->clusterID.byte.MSB;
                                    TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->clusterID.byte.LSB;
                                    #ifdef I_SUPPORT_GROUP_ADDRESSING
                                        if( apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)
                                        {
                                            TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->shortDstAddress.byte.MSB;
                                            TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->shortDstAddress.byte.LSB;
                                        }
                                    #endif
                                    if (apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_INDIRECT &&
                                        apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_GROUP)
                                    {
                                        TxBuffer[TxHeader--] = apsConfirmationHandles[cIndex]->DstEndpoint;
                                    }
                                    TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->apsFrameControl.Val;
                                    if( apsStatus.aplMessages[i]->apsFrameControl.bits.security )
                                    {
                                         if( GetActiveNetworkKey(&ActiveNetworkKey))
                                         {
                                            if( !DataEncrypt(TxBuffer, &TxData,
                                            &(TxBuffer[TxHeader+1), (TX_HEADER_START-TxHeader),
                                            &ActiveNetworkKey, FALSE) )

                                            {
                                                ZigBeeUnblockTx();
                                                return NO_PRIMITIVE;
                                            }
                                        }
                                    }
                                #endif

                            #else

                                params.NLDE_DATA_request.SecurityEnable = FALSE;

                                // Load the APS Payload.
                                for (j=0; j<params.NLDE_DATA_request.NsduLength; j++)
                                {
                                    TxBuffer[TxData++] = apsStatus.aplMessages[i]->message[j];
                                }

                                // Load the APS Header (backwards).
                                TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->APSCounter;
                                TxBuffer[TxHeader--] = apsConfirmationHandles[cIndex]->SrcEndpoint;
                                TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->profileID.byte.MSB;
                                TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->profileID.byte.LSB;
                                TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->clusterID.byte.MSB;
                                TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->clusterID.byte.LSB;
                                #ifdef I_SUPPORT_GROUP_ADDRESSING
                                    if (apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)
                                    {
                                        #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                            PERSISTENCE_PIB currentPIB;

                                            GetPersistenceStorage((void *)&currentPIB);
                                            if ( !currentPIB.nwkUseMulticast )
                                        #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                            {
                                                TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->shortDstAddress.byte.MSB;
                                                TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->shortDstAddress.byte.LSB;
                                            }
                                    }
                                #endif
                                if (apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_INDIRECT &&
                                    apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_GROUP)
                                {
                                    TxBuffer[TxHeader--] = apsConfirmationHandles[cIndex]->DstEndpoint;
                                }

                                TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->apsFrameControl.Val;
                            #endif

                            // Update the message info.
                            apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts--;
                            apsStatus.aplMessages[i]->flags.bits.bSendMessage   = 0;
                            if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                                apsStatus.aplMessages[i]->dataRequestTimer      = TickGet();
                            }//#endif

                            if (apsConfirmationHandles[cIndex]->flags.bits.bWaitingForAck)
                            {
                                apsStatus.flags.bits.bFramesPendingConfirm = 1;
                            }

                            /* For ZigBee 2006: Free space now so that we don't run out on
                             * when creating BTT at nwk layer on PIC18
                            */
                            if ( (apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts == 0) )
                            {
                                // Delete this entry to save space.
                                if (apsStatus.aplMessages[i]->message != NULL)
                                {
                                    nfree( apsStatus.aplMessages[i]->message );
                                }
                                nfree( apsStatus.aplMessages[i] );
                                nfree( apsConfirmationHandles[cIndex] );
                            }

                            ZigBeeBlockTx();
                            return NLDE_DATA_request;
                        }
//#ifdef I_AM_RFD
                        else if (NOW_I_AM_A_RFD()&& apsStatus.aplMessages[i]->apsFrameControl.bits.acknowledgeRequest)
                        {
                            if (TickGetDiff( currentTime, apsStatus.aplMessages[i]->dataRequestTimer ) > (ONE_SECOND)/5 )
                            {
                                // Send a data request message so we can try to receive our ACK
                                apsStatus.aplMessages[i]->dataRequestTimer = currentTime;
                                params.NLME_SYNC_request.Track = FALSE;
                                return NLME_SYNC_request;
                            }
                        }
//#endif
                    }
                }
            }

            for (i=0; (i<MAX_APL_FRAMES) && (apsStatus.aplMessages[i] == NULL); i++) {}
            if (i == MAX_APL_FRAMES)
            {
                //ConsolePutROMString( (ROM char *)"APS: No APL frames awaiting transmission\r\n" );
                apsStatus.flags.bits.bFramesAwaitingTransmission = 0;
            }
        }

        // ---------------------------------------------------------------------
        // Handle fragmentation transmissions
        #if (I_SUPPORT_FRAGMENTATION == 1)
            if ( ( apsStatus.flags.bits.bFragmentationInProgress ) &&
                 ( apsFragmentationInfo.FragmentTxInitiator ) )
            {
                BYTE i;
                BYTE *ptr;
                APS_FRAME_CONTROL   apsFragmentFrameControl;

                #if I_SUPPORT_LINK_KEY == 1
                    KEY_INFO linkKeyDetails;
                    BYTE *linkKey;
                #endif

                #ifndef PROFILE_nwkSecureAllFrames
                    KEY_INFO ActiveNetworkKey;
                #endif

                if ( ( !apsStatus.flags.bits.bWaitingForFragmentDataConfirm ) &&
                     ( !apsFragmentationInfo.WaitingForAck ) )
                {
					//#ifdef I_AM_RFD
					if ( (NOW_I_AM_A_RFD() && ZigBeeReady() && ZigBeeStatus.flags.bits.bDataRequestComplete)
					//#else
					 || (NOW_I_AM_NOT_A_RFD()&&ZigBeeReady())
					)//#endif
					
                    {
                        if (TickGetDiff( TickGet(), apsFragmentationInfo.InterframeDelayTick) > (fragmentInterframeDelay*ONE_MILLISECOND))
                        {
                            //TxHeader = TX_HEADER_START;
                            //TxData = TX_DATA_START;

                            //--------------------------------------------------------------------------
                            //THIS BLOCK IS REQUIRED ONLY FOR TESTING
                            #if (I_SUPPORT_FRAGMENTATION_TESTING == 1)
                            // Check whether testing is under progress for this window.
                                if ( TestErrorWindow == apsFragmentationInfo.WindowSize )
                                {
                                    // Check which of the block should not be transmitted.
                                    if ( TestErrorPacket & 0x01 )
                                    {
                                        TestErrorPacket = TestErrorPacket >> 0x01;
                                        apsStatus.flags.bits.bWaitingForFragmentDataConfirm = 0;
                                        nfree(apsFragmentationInfo.asdu);

                                        apsFragmentationInfo.InterframeDelayTick = TickGet();

                                        if ( ( apsFragmentationInfo.BlockNumberToTxOrRx == apsFragmentationInfo.LastBlockInWindow ) ||
                                            ( apsFragmentationInfo.IsThisBlockRetried ) )
                                        {
                                            apsFragmentationInfo.APSAckDelayTick = TickGet();
                                            apsFragmentationInfo.WaitingForAck = 1;
                                            if ( fragmentWindowSize > 1 )
                                            {

                                                // This part is for Ember GU Implementation.
                                                //-------------------------------------------------------------------
                                                apsFragmentationInfo.ExpectingAckForTheBlockNumber = apsFragmentationInfo.FirstBlockInWindow;
                                                //-------------------------------------------------------------------
                                            }
                                            else
                                            {
                                                apsFragmentationInfo.ExpectingAckForTheBlockNumber = apsFragmentationInfo.FirstBlockInWindow;
                                            }
                                        }
                                        else
                                        {
                                            // Update the field with Next Block Number.
                                            apsFragmentationInfo.BlockNumberToTxOrRx++;
                                            UpdateDataPayloadLengthAndAddress();
                                        }
                                        return NO_PRIMITIVE;
                                    }
                                    else
                                    {
                                        TestErrorPacket = TestErrorPacket >> 0x01;
                                    }
                                }
                            #endif //(I_SUPPORT_FRAGMENTATION_TESTING == 1)
                            //--------------------------------------------------------------------------

                            // Update the FrameControl fields for Fragment Data
                            apsFragmentFrameControl.bits.frameType          = APS_FRAME_DATA;
                            apsFragmentFrameControl.bits.deliveryMode       = APS_DELIVERY_DIRECT;
                            apsFragmentFrameControl.bits.ackFormat          = APS_ACK_FOR_DATA;
                            apsFragmentFrameControl.bits.security           = APS_SECURITY_OFF;
                            if (apsFragmentationInfo.FragmentTxOptions.bits.securityEnabled)
                            {
                                apsFragmentFrameControl.bits.security = APS_SECURITY_ON;
                            }
                            apsFragmentFrameControl.bits.acknowledgeRequest = APS_ACK_REQUESTED;
                            apsFragmentFrameControl.bits.extHeaderPresent   = APS_EXTENDED_HEADER_INCLUDED;

                            // Update NLDE_DATA_request params.
                            params.NLDE_DATA_request.NsduLength         = apsFragmentationInfo.asduLength;
                            params.NLDE_DATA_request.DstAddrMode        = APS_ADDRESS_16_BIT;
                            params.NLDE_DATA_request.DstAddr            = apsFragmentationInfo.FragmentDstAddr.ShortAddr;
                            params.NLDE_DATA_request.BroadcastRadius    = DEFAULT_RADIUS;
                            params.NLDE_DATA_request.DiscoverRoute      = ROUTE_DISCOVERY_ENABLE;
                            params.NLDE_DATA_request.NsduHandle         = NLME_GET_nwkBCSN();
                            params.NLDE_DATA_request.SecurityEnable     = FALSE;

                            #ifdef I_SUPPORT_SECURITY
                                #if PROFILE_nwkSecureAllFrames
                                    BYTE apsHeaderpos = TxData;
                                    TxBuffer[apsHeaderpos] = apsFragmentFrameControl.Val;
                                    #if I_SUPPORT_LINK_KEY == 1
                                        /*  check whether data has to be encrypted with link key */
                                        if( apsFragmentFrameControl.bits.security == APS_SECURITY_ON )
                                        {
                                            apsHeaderpos = TxHeader - 9;
                                            TxBuffer[apsHeaderpos] = apsFragmentFrameControl.Val;
                                        }
                                    #endif
                                    apsHeaderpos++;
                                    TxBuffer[apsHeaderpos++] = apsFragmentationInfo.DstEndpoint;
                                    TxBuffer[apsHeaderpos++] = apsFragmentationInfo.ClusterId.byte.LB;
                                    TxBuffer[apsHeaderpos++] = apsFragmentationInfo.ClusterId.byte.HB;
                                    TxBuffer[apsHeaderpos++] = apsFragmentationInfo.ProfileId.byte.LB;
                                    TxBuffer[apsHeaderpos++] = apsFragmentationInfo.ProfileId.byte.HB;
                                    TxBuffer[apsHeaderpos++] = apsFragmentationInfo.SrcEndpoint;
                                    TxBuffer[apsHeaderpos++] = apsFragmentationInfo.APSCounter;

                                    // Load APS Extended Header Fields
                                    // Check if the fragment packet is the first packet.
                                    if (apsFragmentationInfo.BlockNumberToTxOrRx == 0)
                                    {
                                        // If first packet, then BlockNumber field should contain the Total Number of blocks
                                        TxBuffer[apsHeaderpos++] = APS_EXTENDED_HDR_FIRST_FRAGMENT;
                                        TxBuffer[apsHeaderpos++] = apsFragmentationInfo.TotalNumberOfBlocks;
                                    }
                                    else
                                    {
                                        // If not the first packet, then BlockNumber field should contain the actual Block that is being transmitted.
                                        TxBuffer[apsHeaderpos++] = APS_EXTENDED_HDR_PART_OF_FRAGMENT;
                                        TxBuffer[apsHeaderpos++] = apsFragmentationInfo.BlockNumberToTxOrRx;
                                    }

                                    if( apsFragmentFrameControl.bits.security == APS_SECURITY_ON )
                                    {
                                        TxHeader = apsHeaderpos - 11;
                                    }
                                    else
                                    {
                                        TxData = apsHeaderpos;
                                    }

                                    // Load the APP Payload.
                                    ptr = apsFragmentationInfo.asdu;
                                    for (i=0; i<apsFragmentationInfo.asduLength; i++)
                                    {
                                        TxBuffer[TxData++] = *ptr++;
                                    }

                                    params.NLDE_DATA_request.SecurityEnable = apsFragmentationInfo.FragmentTxOptions.bits.useNWKKey;

                                    #if I_SUPPORT_LINK_KEY == 1
                                        if( apsFragmentFrameControl.bits.security == APS_SECURITY_ON )
                                        {
                                            KEY_VAL     linkKeyToBeUsed;

                                            #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                                #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                                                    TC_LINK_KEY_TABLE   tcLinkKeyTable;
                                                    LONG_ADDR           dstLongAddrReverse;
                                                #endif
                                            #endif

                                            //#ifdef I_AM_TRUST_CENTER
                                                LONG_ADDR   dstLongAddress;

                                                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                                                    LONG_ADDR dstLongAddrReverse;
                                                    LONG_ADDR   SrcAddr;
                                                    BYTE k;
                                                #endif
											if(I_AM_TRUST_CENTER){//
                                                if( ( IsThisAddressKnown ( params.NLDE_DATA_request.DstAddr, &dstLongAddress.v[0] ) ) &&
                                                    ( params.NLDE_DATA_request.DstAddr.Val < 0xFFFB ) )
                                                {
                                                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                                                        GetMACAddress(&SrcAddr);
                                                        for(k=0; k < 8 ; k++)
                                                        {
                                                            dstLongAddrReverse.v[k] = dstLongAddress.v[7 - k];
                                                        }
                                                        GetHashKey(&dstLongAddrReverse, &SrcAddr, &linkKeyToBeUsed);
                                                    #endif

                                                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                                        BYTE        count;
                                                        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                                                            for(count=0; count < 8 ; count++)
                                                            {
                                                                dstLongAddrReverse.v[count] = dstLongAddress.v[7 - count];
                                                            }
                                                            if ( SearchForTCLinkKey(dstLongAddrReverse, &tcLinkKeyTable) )
                                                            {
                                                                for (count = 0; count < KEY_LENGTH; count++)
                                                                {
                                                                    linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                                                }
                                                            }
                                                            else
                                                            {
                                                                ZigBeeUnblockTx();
                                                                nfree(apsFragmentationInfo.asdu);
                                                                SendFragmentDataConfirm(APS_SECURITY_FAIL);
                                                                return NO_PRIMITIVE;
                                                            }
                                                        #else
                                                            for(count = 0; count < KEY_LENGTH; count++)
                                                            {
                                                                linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                                                            }
                                                        #endif
                                                    #endif

                                                    linkKey = &linkKeyToBeUsed.v[0];
                                                    linkKeyDetails.frameCounter = TCLinkKeyInfo.frameCounter;
                                                    TCLinkKeyInfo.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val +1;
                                                    apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
                                                }
                                                else
                                                {
                                                    linkKey = NULL;
                                                }

           									}else{//#else
                                                if( TCLinkKeyInfo.trustCenterShortAddr.Val == params.NLDE_DATA_request.DstAddr.Val )
                                                {
                                                    BYTE        count;

                                                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                                        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1

                                                            for(count = 0; count < 8; count++)
                                                            {
                                                                dstLongAddrReverse.v[count] = TCLinkKeyInfo.trustCenterLongAddr.v[count];
                                                            }
                                                            if ( SearchForTCLinkKey(dstLongAddrReverse, &tcLinkKeyTable) )
                                                            {
                                                                for (count = 0; count < KEY_LENGTH; count++)
                                                                {
                                                                    linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                                                }
                                                            }
                                                            else
                                                            {
                                                                ZigBeeUnblockTx();
                                                                nfree(apsFragmentationInfo.asdu);
                                                                SendFragmentDataConfirm(APS_SECURITY_FAIL);
                                                                return NO_PRIMITIVE;
                                                            }
                                                        #else
                                                            for(count = 0; count < KEY_LENGTH; count++)
                                                            {
                                                                linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                                                            }
                                                        #endif
                                                    #else
                                                        for(count = 0; count < KEY_LENGTH; count++)
                                                        {
                                                            linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                                                        }
                                                    #endif
                                                    linkKeyDetails.frameCounter = TCLinkKeyInfo.frameCounter;
                                                    linkKey = &linkKeyToBeUsed.v[0];
                                                    TCLinkKeyInfo.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val +1;
                                                    apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
                                                }
                                                else
                                                {
                                                    LONG_ADDR dstLongAddress;
                                                    if( ( IsThisAddressKnown ( params.NLDE_DATA_request.DstAddr, &dstLongAddress.v[0] ) ) &&
                                                        ( params.NLDE_DATA_request.DstAddr.Val < 0xFFFB ) )
                                                    {
                                                        APS_KEY_PAIR_DESCRIPTOR appLinkKeyDetails;
                                                        BYTE count;
                                                        for(count=0; count < 8; count++)
                                                        {
                                                            appLinkKeyDetails.PartnerAddress.v[count] = dstLongAddress.v[count];
                                                        }
                                                        /*0x01 means the Link Key is used for Encryption. In this case,
                                                        the IncomingFrameCounter should be incremented and stored onto NVM*/
                                                        if( SearchForLinkKey(&appLinkKeyDetails, 0x01) )
                                                        {
                                                            linkKey = &appLinkKeyDetails.Link_Key.v[0];
                                                            linkKeyDetails.frameCounter = appLinkKeyDetails.IncomingFrameCounter;
                                                        }
                                                        else
                                                        {
                                                            linkKey = NULL;
                                                        }
                                                    }
                                                    else
                                                    {
                                                       linkKey = NULL;
                                                    }

                                                }
                    					}//#endif     // #ifdef I_AM_TRUST_CENTER
                                            if(linkKey != NULL)
                                            {
                                                linkKeyDetails.key = linkKey;
                                                linkKeyDetails.keyId = ID_LinkKey;
                                                if (
                                                        !DataEncrypt
                                                        (
                                                            TxBuffer,
                                                            &TxData,
                                                            &(TxBuffer[TxHeader+1]),
                                                            (TX_HEADER_START-TxHeader),
                                                            &linkKeyDetails, FALSE
                                                         )
                                                     )

                                                {
                                                    ZigBeeUnblockTx();
                                                    nfree(apsFragmentationInfo.asdu);
                                                    SendFragmentDataConfirm(APS_SECURITY_FAIL);
                                                    return NO_PRIMITIVE;
                                                }
                                                else
                                                {
                                                    FormatTxBuffer();
                                                }
                                            }
                                            else
                                            {
                                                // Link Key is NULL. So handle error here.
                                                // Currently we just return NO_PRIMITIVE.
                                                // But we need to free up the memory and some cleaning is required.
                                                ZigBeeUnblockTx();
                                                nfree(apsFragmentationInfo.asdu);
                                                SendFragmentDataConfirm(APS_SECURITY_FAIL);
                                                return NO_PRIMITIVE;
                                            }
                                        }
                                    #endif // #if I_SUPPORT_LINK_KEY == 1
                                #else // #if PROFILE_nwkSecureAllFrames
                                    params.NLDE_DATA_request.SecurityEnable = FALSE;

                                    if( apsFragmentFrameControl.bits.security )
                                    {
                                         if( GetActiveNetworkKey(&ActiveNetworkKey))
                                         {
                                            if( !DataEncrypt(TxBuffer, &TxData,
                                            &(TxBuffer[TxHeader+1), (TX_HEADER_START-TxHeader),
                                            &ActiveNetworkKey, FALSE) )
                                            {
                                                ZigBeeUnblockTx();
                                                return NO_PRIMITIVE;
                                            }
                                        }
                                    }
                                #endif // #if PROFILE_nwkSecureAllFrames
                            #else
                                params.NLDE_DATA_request.SecurityEnable = FALSE;

                                // Load the APS Header (backwards).
                                // Load APS Extended Header Fields
                                // Check if the fragment packet is the first packet.
                                if (apsFragmentationInfo.BlockNumberToTxOrRx == 0)
                                {
                                    // If first packet, then BlockNumber field should contain the Total Number of blocks
                                    TxBuffer[TxHeader--] = apsFragmentationInfo.TotalNumberOfBlocks;
                                    TxBuffer[TxHeader--] = APS_EXTENDED_HDR_FIRST_FRAGMENT;
                                }
                                else
                                {
                                    // If not the first packet, then BlockNumber field should contain the actual Block that is being transmitted.
                                    TxBuffer[TxHeader--] = apsFragmentationInfo.BlockNumberToTxOrRx;
                                    TxBuffer[TxHeader--] = APS_EXTENDED_HDR_PART_OF_FRAGMENT;
                                }
                                // Load other APS Header Fields
                                TxBuffer[TxHeader--] = apsFragmentationInfo.APSCounter;
                                TxBuffer[TxHeader--] = apsFragmentationInfo.SrcEndpoint;
                                TxBuffer[TxHeader--] = apsFragmentationInfo.ProfileId.byte.MSB;
                                TxBuffer[TxHeader--] = apsFragmentationInfo.ProfileId.byte.LSB;
                                TxBuffer[TxHeader--] = apsFragmentationInfo.ClusterId.byte.MSB;
                                TxBuffer[TxHeader--] = apsFragmentationInfo.ClusterId.byte.LSB;
                                TxBuffer[TxHeader--] = apsFragmentationInfo.DstEndpoint;
                                TxBuffer[TxHeader--] = apsFragmentFrameControl.Val;

                                // Load the APP Payload.
                                ptr = apsFragmentationInfo.asdu;
                                for (i=0; i<apsFragmentationInfo.asduLength; i++)
                                {
                                    TxBuffer[TxData++] = *ptr++;
                                }

                            #endif // #if I_SUPPORT_SECURITY

                            apsStatus.flags.bits.bWaitingForFragmentDataConfirm = 1;

                            ZigBeeBlockTx();
                            return NLDE_DATA_request;

                        }
                    }
                }
                else
                {
                    // Check whether the device is waiting for APS ACK
                    if ( apsFragmentationInfo.WaitingForAck )
                    {
						
                        // Check whether the APS ACK timer has elapsed
                        if ( TickGetDiff( TickGet(), apsFragmentationInfo.APSAckDelayTick) > apscAckWaitDuration )
                        {
                            // Check whether the Retry counter has exceed its limit. If exceeded, then stop further
                            // fragment data transmission and send the data confirmation.
                            if ( apsFragmentationInfo.RetryCounter == apscMaxFrameRetries + 1 )
                            {
								SendFragmentDataConfirm(APS_NO_ACK);
                            }
                            else
                            {
                                apsStatus.flags.bits.bWaitingForFragmentDataConfirm = 0;
                                apsFragmentationInfo.WaitingForAck = 0;
                                // Did not receive APS ACK for the first window. So need to retry all the blocks.
                                RetryBlock(TRUE); // All the blocks in the window needs to be retried.
                            }
                        }
                    }
                }
            }
        #endif //(I_SUPPORT_FRAGMENTATION == 1)

        // ---------------------------------------------------------------------
        // Send Fragment Data Indication to higher layer
        #if (I_SUPPORT_FRAGMENTATION == 1)
            if ( ( apsStatus.flags.bits.bFragmentationInProgress ) &&
                 ( apsStatus.flags.bits.bPendingFragmentDataIndication ) &&
                 ( apsFragmentationInfo.AllFragmentsReceived ) &&
                 ( !apsFragmentationInfo.FragmentTxInitiator ) )
            {
                // Reset the flags.
                apsStatus.flags.bits.bFragmentationInProgress = 0;
                apsStatus.flags.bits.bPendingFragmentDataIndication = 0;

                apsFragmentationInfo.FragmentationStartTime.Val = 0;

                apsFragmentationInfo.AllFragmentsReceived = 0;
                apsFragmentationInfo.IsThisBlockRetried = 0;

                #ifdef I_AM_END_DEVICE
                    QuickPoll = 0;
                #endif

                // Update Data Indication parameters
                params.APSDE_DATA_indication.ClusterId                  = apsFragmentationInfo.ClusterId;
                params.APSDE_DATA_indication.DstEndpoint                = apsFragmentationInfo.DstEndpoint;
                params.APSDE_DATA_indication.ProfileId                  = apsFragmentationInfo.ProfileId;
                params.APSDE_DATA_indication.DstAddrMode                = APS_ADDRESS_16_BIT;
                params.APSDE_DATA_indication.DstAddress.ShortAddr.Val   = macPIB.macShortAddress.Val;
                params.APSDE_DATA_indication.SrcAddrMode                = APS_ADDRESS_16_BIT;
                params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val   = apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val;
                params.APSDE_DATA_indication.SrcEndpoint                = apsFragmentationInfo.SrcEndpoint;
                params.APSDE_DATA_indication.Status                     = SUCCESS;
                params.APSDE_DATA_indication.SecurityStatus             = apsFragmentationInfo.SecurityStatus;
                params.APSDE_DATA_indication.LinkQuality                = apsFragmentationInfo.LinkQuality;
                params.APSDE_DATA_indication.WasBroadcast               = FALSE;

                // Reset the FragmentDstAddr, so that next fragmentation can be processed.
                apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val = 0xFFFF;

                // Send Data Indication
                HandleFragmentDataIndication
                (
                    apsFragmentationInfo.DataPayloadLength,
                    apsFragmentationInfo.DataPayloadAddress
                );
            }
        #endif //(I_SUPPORT_FRAGMENTATION == 1)

        // ---------------------------------------------------------------------
        // Clean up when all the blocks were not received.
        #if (I_SUPPORT_FRAGMENTATION == 1)
            if ( ( apsStatus.flags.bits.bFragmentationInProgress ) &&
                 ( !apsFragmentationInfo.FragmentTxInitiator ) &&
                 ( apsFragmentationInfo.FragmentationStartTime.Val != 0 ) )
            {
                TICK currentTime;
                currentTime = TickGet();

                if ( TickGetDiff( currentTime, apsFragmentationInfo.FragmentationStartTime ) > apsFragmentationInfo.TotalFragmentationTime.Val )
                {
                    // Clean up all the fragmentation related params.
                    apsStatus.flags.bits.bFragmentationInProgress = 0;
                    apsStatus.flags.bits.bPendingFragmentDataIndication = 0;

                    apsFragmentationInfo.FragmentationStartTime.Val = 0;

                    apsFragmentationInfo.AllFragmentsReceived = 0;
                    apsFragmentationInfo.IsThisBlockRetried = 0;

                    apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val = 0xFFFF;

                    #ifdef I_AM_END_DEVICE
                        QuickPoll = 0;
                    #endif

                    nfree(apsFragmentationInfo.DataPayloadAddress);
                }
            }
        #endif //(I_SUPPORT_FRAGMENTATION == 1)

        // ---------------------------------------------------------------------
        // Handle frames awaiting confirmation
        if (apsStatus.flags.bits.bFramesPendingConfirm)
        {
            // NOTE: Compiler SSR27744, TickGet() output must be assigned to a variable.

            for (i=0; i<MAX_APS_FRAMES; i++)
            {
                if (apsConfirmationHandles[i] != NULL)
                {
                    if ((apsConfirmationHandles[i]->nsduHandle != INVALID_NWK_HANDLE) &&
                        (TickGetDiff( currentTime, apsConfirmationHandles[i]->timeStamp ) > apscAckWaitDuration))
                    {
                        // The frame has timed out while waiting for an ACK.  See if we can try again.

                        // Get the index to either the indirect message buffer of APL message buffer
                        // (nIndirectRelayIndex is the same as nAPLFrameIndex).
                        j = apsConfirmationHandles[i]->flags.bits.nAPLFrameIndex;

                        #ifdef I_SUPPORT_BINDINGS
                        if (apsConfirmationHandles[i]->flags.bits.bWeAreOriginator)
                        #endif
                        {
                            // We are the originator of the frame, so look in aplMessages.

                            // Try to send the message again.
                            apsStatus.aplMessages[j]->flags.bits.bSendMessage = 1;
                            apsStatus.aplMessages[j]->status = APS_NO_ACK;
                        }

                        #ifdef I_SUPPORT_BINDINGS
                            if (!apsConfirmationHandles[i]->flags.bits.bWeAreOriginator)
                            {
                                nfree( apsConfirmationHandles[i] );
                            }
                        #endif
                    }
                }
            }

            for (i=0; (i<MAX_APS_FRAMES) && (apsConfirmationHandles[i]==NULL); i++) {}
            if (i == MAX_APS_FRAMES)
            {
                //ConsolePutROMString( (ROM char *)"APS: No APS frames awaiting confirmation\r\n" );
                apsStatus.flags.bits.bFramesPendingConfirm = 0;
                params.APSDE_DATA_confirm.Status = SUCCESS;
            }
        }
    }
    else
    {
        switch (inputPrimitive)
        {
            case NLDE_DATA_confirm:
                #if I_SUPPORT_FREQUENCY_AGILITY == 1
                    /* Update TotalTransmittedPackets and TotalTransmitFailures
                       depending on data transmission status received. */
                    TotalTransmittedPackets.Val++;
                    if ( params.NLDE_DATA_confirm.Status != SUCCESS )
                    {
                        TotalTransmitFailures.Val++;
                    }
                #endif

                #if (I_SUPPORT_FRAGMENTATION == 0x01)
                    if ( apsStatus.flags.bits.bFragmentationInProgress )
                    {
                        if ( apsFragmentationInfo.FragmentTxInitiator )
                        {
                            apsStatus.flags.bits.bWaitingForFragmentDataConfirm = 0;
                            nfree(apsFragmentationInfo.asdu);

                            apsFragmentationInfo.InterframeDelayTick = TickGet();

                            // If we have transmitted the last block, then we have to wait for the APA ACK.
                            // Otherwise, continue transmitting the next block.
                            if ( ( apsFragmentationInfo.BlockNumberToTxOrRx == apsFragmentationInfo.LastBlockInWindow ) ||
                                 ( apsFragmentationInfo.IsThisBlockRetried ) )
                            {
                                apsFragmentationInfo.APSAckDelayTick = TickGet();
                                apsFragmentationInfo.WaitingForAck = 1;
                                if ( fragmentWindowSize > 1 )
                                {

                                    // This part is for Ember GU Implementation.
                                    //-------------------------------------------------------------------
                                    apsFragmentationInfo.ExpectingAckForTheBlockNumber = apsFragmentationInfo.FirstBlockInWindow;
                                    //-------------------------------------------------------------------
                                }
                                else
                                {
                                    apsFragmentationInfo.ExpectingAckForTheBlockNumber = apsFragmentationInfo.FirstBlockInWindow;
                                }
                            }
                            else
                            {
                                apsFragmentationInfo.BlockNumberToTxOrRx++;
                                UpdateDataPayloadLengthAndAddress();
                            }
                        }
                        return NO_PRIMITIVE;
                    }
                #endif //(I_SUPPORT_FRAGMENTATION == 0x01)

                for (i=0; i<MAX_APS_FRAMES; i++)
                {
                    if ((apsConfirmationHandles[i] != NULL) &&
                        (apsConfirmationHandles[i]->nsduHandle == params.NLDE_DATA_confirm.NsduHandle))
                    {
                        if (!apsConfirmationHandles[i]->flags.bits.bWaitingForAck ||
                            (params.NLDE_DATA_confirm.Status != NWK_SUCCESS))
                        {
FinishConfirmation:
                            {
                                // Get the index into the message info buffer
                                j = apsConfirmationHandles[i]->flags.bits.nIndirectRelayIndex;

                                #ifdef I_SUPPORT_BINDINGS
                                if (apsConfirmationHandles[i]->flags.bits.bWeAreOriginator)
                                #endif
                                {
                                    if (params.NLDE_DATA_confirm.Status != NWK_SUCCESS)
                                    {
                                        // The transmission failed, so try again.
                                        apsStatus.aplMessages[j]->flags.bits.bSendMessage = 1;
                                        apsStatus.aplMessages[j]->status = params.NLDE_DATA_confirm.Status;
                                    }
                                    else
                                    {
                                        if ( apsConfirmationHandles[i]->DstAddrMode == APS_ADDRESS_GROUP )
                                        {
                                            apsStatus.flags.bits.bGroupAddressing = 1;
                                        }

                                        // We have a successful confirmation for a frame we transmitted for the upper layers.
                                        params.APSDE_DATA_confirm.DstAddrMode   = apsConfirmationHandles[i]->DstAddrMode;
                                        params.APSDE_DATA_confirm.DstAddress    = apsConfirmationHandles[i]->DstAddress;
                                        params.APSDE_DATA_confirm.SrcEndpoint   = apsConfirmationHandles[i]->SrcEndpoint;
                                        params.APSDE_DATA_confirm.DstEndpoint   = apsConfirmationHandles[i]->DstEndpoint;
                                        params.APSDE_DATA_confirm.TxTime        = apsConfirmationHandles[i]->timeStamp;

                                        // Clean up everything.
                                        if (apsStatus.aplMessages[j]->message != NULL)
                                        {
                                            nfree( apsStatus.aplMessages[j]->message );
                                        }
                                        nfree( apsStatus.aplMessages[j] );
                                        nfree( apsConfirmationHandles[i] );
                                        return APSDE_DATA_confirm;
                                    }
                                }
                            }
                        }
                    }
                }
                break;

            case NLDE_DATA_indication:
                {
                    APS_FRAME_CONTROL   apsFrameControl;
                    BYTE                currentAPSCounter = 0;

                    BYTE        FragmentType = 0;
                    BYTE        BlockNumber = 0;
                    BYTE        ACKBitField = 0;
                    SHORT_ADDR  locDestAddress;
                    #if (I_SUPPORT_LINK_KEY == 0x01)
                        BYTE   *apsHeader = params.NLDE_DATA_indication.Nsdu;
                        //BYTE keyType;
                    #endif
                    #define destinationEPL      apsStatus.ackMessage.DstEndpoint
                    #define clusterIDL          apsStatus.ackMessage.ClusterId
                    #define profileIDL          apsStatus.ackMessage.ProfileId
                    #define sourceEPL           apsStatus.ackMessage.SrcEndpoint
                    #define addressModeL        apsStatus.ackMessage.SrcAddrMode
                    #define ackSourceAddressL   apsStatus.ackMessage.SrcAddress
                    #define groupIDL            apsStatus.ackMessage.GroupId

                    // Start extracting the APS header
                    apsFrameControl.Val = APSGet();

                    BYTE apsHeaderSize = 1;


                    if ((apsFrameControl.bits.frameType == APS_FRAME_DATA) ||
                        (apsFrameControl.bits.frameType == APS_FRAME_ACKNOWLEDGE))
                    {
                        // Finish reading the APS header
                        
                        if (apsFrameControl.bits.deliveryMode != APS_DELIVERY_GROUP)
                        {
                            destinationEPL   = APSGet();
                            apsHeaderSize++;
                        }
                        if( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)
                        {
                            #ifndef I_SUPPORT_GROUP_ADDRESSING
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            #endif
                            params.APSDE_DATA_indication.DstAddress.ShortAddr.v[0] = groupIDL.v[0]   = APSGet();
                            params.APSDE_DATA_indication.DstAddress.ShortAddr.v[1] = groupIDL.v[1]   = APSGet();
                            apsHeaderSize += 2;
                        }
                        if (params.NLDE_DATA_indication.isMulticastFrame)
                        {
                            params.APSDE_DATA_indication.DstAddress.ShortAddr.v[0] = groupIDL.v[0]   = params.NLDE_DATA_indication.DstAddr.v[0];
                            params.APSDE_DATA_indication.DstAddress.ShortAddr.v[1] = groupIDL.v[1]   = params.NLDE_DATA_indication.DstAddr.v[1];
                        }

                        clusterIDL.v[0]      = APSGet();
                        clusterIDL.v[1]      = APSGet();
                        profileIDL.byte.LB  = APSGet();
                        profileIDL.byte.HB  = APSGet();
                        apsHeaderSize += 4;

                        {
                            sourceEPL        = APSGet();
                            apsHeaderSize++;                        
                        }
                        
                        /* get the destination address here to properly handle some mgmt messages */
                        if( clusterIDL.Val == MGMT_PERMIT_JOINING_req )
                        {
                            nwkStatus.mgmtPermitJoiningRequestDestination.Val =  params.NLDE_DATA_indication.DstAddr.Val;      
                        }
                        
                        /* preserve the destination address for reuse - PIC32 compiler requires this */
                        locDestAddress.Val = params.NLDE_DATA_indication.DstAddr.Val;

                        currentAPSCounter = APSGet(); // APS counter
                        apsHeaderSize++;
                        if ( apsFrameControl.bits.extHeaderPresent == APS_EXTENDED_HEADER_INCLUDED )
                        {
                            FragmentType = APSGet();
                            BlockNumber = APSGet();
                            apsHeaderSize += 2;
                            if( (apsFrameControl.bits.frameType == APS_FRAME_DATA) &&
                                DuplicatePacket(params.NLDE_DATA_indication.SrcAddress, currentAPSCounter, BlockNumber) )
                            {
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            }
                            if ( apsFrameControl.bits.frameType == APS_FRAME_ACKNOWLEDGE )
                            {
                                ACKBitField = APSGet();
                                apsHeaderSize++;
                            }
                        }
                        else
                        {
                            if( (apsFrameControl.bits.frameType == APS_FRAME_DATA) &&
                                DuplicatePacket(params.NLDE_DATA_indication.SrcAddress, currentAPSCounter, 0xff) )
                            {
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            }
                        }
                    }
                    else
                    {
                        currentAPSCounter = APSGet(); // APS counter
                        apsHeaderSize++;
                    }


                    if ( params.NLDE_DATA_indication.SecurityUse )
                    {
                        params.APSDE_DATA_indication.SecurityStatus = APS_SECURED_NWK_KEY;
                    }
                    else
                    {
                        params.APSDE_DATA_indication.SecurityStatus = APS_UNSECURED;
                    }

                    #ifdef I_SUPPORT_SECURITY
                        #if((PROFILE_nwkSecureAllFrames == 0) || (I_SUPPORT_LINK_KEY == 1 ))
                            if( apsFrameControl.bits.security )
                            {
                                LONG_ADDR   longAddress;
                                if( !IsThisAddressKnown(params.NLDE_DATA_indication.SrcAddress, longAddress.v ))
                                {
                                    #if (I_SUPPORT_LINK_KEY == 1 )
                                        //#ifndef  I_AM_TRUST_CENTER
                                            if( !I_AM_TRUST_CENTER && (params.NLDE_DATA_indication.SrcAddress.Val == TCLinkKeyInfo.trustCenterShortAddr.Val) )
                                            {
                                                BYTE index;

                                                for(index=0; index < 8; index++)
                                                {
                                                    longAddress.v[index] = TCLinkKeyInfo.trustCenterLongAddr.v[7-index];
                                                }
                                            }
                                            else if (!I_AM_TRUST_CENTER)
                                        // #endif /*  I_AM_TRUST_CENTER */
                                    #endif
                                        {
                                            APSDiscardRx();
                                            return NO_PRIMITIVE;
                                        }
                                }
                                if( !DataDecrypt(params.NLDE_DATA_indication.Nsdu,
                                    &(params.NLDE_DATA_indication.NsduLength),
                                    apsHeader, apsHeaderSize, ID_LinkKey, &longAddress) )
                                {
                                    APSDiscardRx();
                                    return NO_PRIMITIVE;
                                }

                                params.APSDE_DATA_indication.SecurityStatus = APS_SECURED_LINK_KEY;
                            }
                        #endif  //((PROFILE_nwkSecureAllFrames == 0) || (I_SUPPORT_LINK_KEY == 1 ))
                    #endif // I_SUPPORT_SECURITY

                    if (apsFrameControl.bits.frameType == APS_FRAME_DATA)
                    {
#ifdef I_SUPPORT_GROUP_ADDRESSING
                        if ( ( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP ) ||
                             ( params.NLDE_DATA_indication.isMulticastFrame == 0x01 ) )
                        {
                            /* ZigBee 2006: Don't send group broadcast request to self */
                            if(
                            (GetEndPointsFromGroup(groupIDL) == MAX_GROUP ) )
                            {
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            }
                        }
#endif

#ifdef I_SUPPORT_BINDINGS
                        if( (apsFrameControl.bits.deliveryMode == APS_DELIVERY_DIRECT) ||
                            (apsFrameControl.bits.deliveryMode == APS_DELIVERY_BROADCAST) ||
                            (apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)) //||
#endif
                        {
                            
                            {
                                addressModeL = APS_ADDRESS_16_BIT;
                            }
                            #if (I_SUPPORT_FRAGMENTATION == 1)
                                if ( apsFrameControl.bits.extHeaderPresent == APS_EXTENDED_HEADER_INCLUDED )
                                {
                                    // We have received fragmented packet. Update the Fragmentation Info fields.
                                    return UpdateFragmentationInfoFields
                                            (
                                                FragmentType,
                                                BlockNumber,
                                                clusterIDL,
                                                profileIDL,
                                                destinationEPL,
                                                sourceEPL,
                                                currentAPSCounter,
                                                params.NLDE_DATA_indication.SrcAddress
                                            );
                                }
                            #endif // (I_SUPPORT_FRAGMENTATION == 1)

                            // Check for and send APS ACK here.
                            if (apsFrameControl.bits.acknowledgeRequest == APS_ACK_REQUESTED)
                            {
                                // Since we also need to process this message, buffer the info so we can send up an
                                // APSDE_DATA_indication as soon as possible. Set the flag to send the data indication
                                // in the background
                                apsStatus.flags.bits.bDataIndicationPending = 1;

                                // Buffer the old message info
                                apsStatus.ackMessage.asduLength     = params.NLDE_DATA_indication.NsduLength;
                                apsStatus.ackMessage.asdu           = params.NLDE_DATA_indication.Nsdu;
                                apsStatus.ackMessage.WasBroadcast   = (apsFrameControl.bits.deliveryMode == APS_DELIVERY_BROADCAST);
                                apsStatus.ackMessage.SrcAddress     = params.NLDE_DATA_indication.SrcAddress;
                                apsStatus.ackMessage.SecurityStatus = params.APSDE_DATA_indication.SecurityStatus;
                                apsStatus.ackMessage.DstAddress     = params.APSDE_DATA_indication.DstAddress.ShortAddr;
                                apsStatus.ackMessage.LinkQuality	= params.NLDE_DATA_indication.LinkQuality;

                                /* ForZigBee 2006: The NLDE_DATA_request.DstAddr
                                 * for an acknowledge cannot be a long address like the source
                                 * address of the message to which we are responding - it MUST
                                 * be a short address, this is why its changed back to a short address
                                 * The net effect is that the ZDO_data_indication goes up as a long address
                                 * but the acknowledge is returned to the sender using its short address.
                                */
                                params.NLDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
                                /* params.NLDE_DATA_request.DstAddr = params.NLDE_DATA_indication.SrcAddress; */
                                
                                /* Use the reserved src address here to help with C32 compiler structure packing issue */
                                params.NLDE_DATA_request.DstAddr    = apsStatus.ackMessage.SrcAddress;
                                
                                //params.NLDE_DATA_request.DstAddr = params.NLDE_DATA_indication.SrcAddress;
                                params.NLDE_DATA_request.BroadcastRadius    = DEFAULT_RADIUS;
                                params.NLDE_DATA_request.DiscoverRoute      = ROUTE_DISCOVERY_ENABLE;

                                params.NLDE_DATA_request.NsduLength         = 0;
                                params.NLDE_DATA_request.NsduHandle         = NLME_GET_nwkBCSN();

                                apsFrameControl.bits.acknowledgeRequest = APS_ACK_NOT_REQUESTED;
                                apsFrameControl.bits.frameType          = APS_FRAME_ACKNOWLEDGE;
                                //apsFrameControl.bits.security           = 0; // don't need to secure the ack frame in APL, no payload
                                /*APS ACK should always be transmitted using Direct Delivery Mode*/
                                apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT;

                                if ( apsFrameControl.bits.security )
                                {
                                    #if I_SUPPORT_LINK_KEY == 1
                                        TxBuffer[TxHeader--] = currentAPSCounter;
                                        TxBuffer[TxHeader--] = destinationEPL;
                                        TxBuffer[TxHeader--] = profileIDL.byte.HB;
                                        TxBuffer[TxHeader--] = profileIDL.byte.LB;
                                        TxBuffer[TxHeader--] = clusterIDL.byte.HB;
                                        TxBuffer[TxHeader--] = clusterIDL.byte.LB;
                                        TxBuffer[TxHeader--] = sourceEPL;
                                        TxBuffer[TxHeader--] = apsFrameControl.Val;

                                        if ( ApplyAPSSecurityToAPSACK() )
                                        {
                                            #if defined(I_SUPPORT_SECURITY) && PROFILE_nwkSecureAllFrames
                                                /*APS ACK may also be encrypted using NWK Key, if
                                                SecureAllFrames is set to TRUE.*/
                                                params.NLDE_DATA_request.SecurityEnable = TRUE;
                                            #endif
                                        }
                                        else
                                        {
                                            APSDiscardRx();
                                            return NO_PRIMITIVE;
                                        }
                                    #else
                                        /*If Link Key is not supported and if we receive a data frame
                                        secured using Link Key, then it is an invalid frame. So, we
                                        have to drop the frame.*/
                                        apsStatus.flags.bits.bDataIndicationPending = 0; // This is a security failure. No need to send indication
                                        APSDiscardRx();
                                        return NO_PRIMITIVE;
                                    #endif
                                }
                                else
                                {
                                    /*The received data was not encrypted using Link Key.
                                    But, we may have to encrypt the APS ACK with NWK Key, if, security is supported.*/
                                    #if defined(I_SUPPORT_SECURITY) && PROFILE_nwkSecureAllFrames
                                        // params.NLDE_DATA_request.SecurityEnable = TRUE;
                                        //if (apsFrameControl.bits.deliveryMode == APS_DELIVERY_INDIRECT)
                                        //{
                                        //    apsFrameControl.bits.indirectAddressMode = APS_INDIRECT_ADDRESS_MODE_TO_COORD;
                                        //    TxBuffer[TxData++] = apsFrameControl.Val;
                                        //}
                                        //else
                                        params.NLDE_DATA_request.SecurityEnable = TRUE;
                                        {
                                            TxBuffer[TxData++] = apsFrameControl.Val & 0xF3; // 0b11110011 set delivery mode to the direct
                                            TxBuffer[TxData++] = sourceEPL;
                                        }
                                        TxBuffer[TxData++] = clusterIDL.byte.LB;
                                        TxBuffer[TxData++] = clusterIDL.byte.HB;
                                        TxBuffer[TxData++] = profileIDL.byte.LB;
                                        TxBuffer[TxData++] = profileIDL.byte.HB;
                                        TxBuffer[TxData++] = destinationEPL;
                                        TxBuffer[TxData++] = currentAPSCounter;
                                    #else
                                        /*If security is not supported at all, then we have to
                                        send APS ACK without encryption.*/
                                        params.NLDE_DATA_request.SecurityEnable = FALSE;
                                        // Load up the APS Header (backwards).  Note that the source and destination EP's get flipped.
                                        TxBuffer[TxHeader--] = currentAPSCounter;
                                        TxBuffer[TxHeader--] = destinationEPL;
                                        TxBuffer[TxHeader--] = profileIDL.byte.HB;
                                        TxBuffer[TxHeader--] = profileIDL.byte.LB;
                                        TxBuffer[TxHeader--] = clusterIDL.byte.HB;
                                        TxBuffer[TxHeader--] = clusterIDL.byte.LB;

                                    
                                        {
                                            apsFrameControl.bits.deliveryMode       = APS_DELIVERY_DIRECT;
                                            TxBuffer[TxHeader--] = sourceEPL;
                                        }

                                        apsFrameControl.bits.acknowledgeRequest = APS_ACK_NOT_REQUESTED;
                                        apsFrameControl.bits.frameType          = APS_FRAME_ACKNOWLEDGE;
                                        apsFrameControl.bits.security           = 0; // don't need to secure the ack frame in APL, no payload

                                        TxBuffer[TxHeader--] = apsFrameControl.Val;
                                    #endif
                                }

                                ZigBeeBlockTx();
                                return NLDE_DATA_request;
                            }

                            if( ( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP ) ||
                                ( destinationEPL == 0xFF ) ||
                                ( params.NLDE_DATA_indication.isMulticastFrame ) )
                            {
                                #ifdef I_SUPPORT_GROUP_ADDRESSING
                                    for(i = 0; i < MAX_GROUP_RECORD_BUFFER; i++)
                                    {
                                        if( apsStatus.apsGroupRecord[i] == NULL )
                                        {
                                            apsStatus.apsGroupRecord[i] = (APS_GROUP_RECORD *)SRAMalloc(sizeof(APS_GROUP_RECORD));
                                            if( apsStatus.apsGroupRecord[i] == NULL )
                                            {
                                                APSDiscardRx();
                                                return NO_PRIMITIVE;
                                            }
                                            if ( ( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP ) ||
                                                 ( params.NLDE_DATA_indication.isMulticastFrame ) )
                                            {
                                                params.NLDE_DATA_indication.isMulticastFrame = 0; // Clear to process next one.
                                                for(j = 0; j < MAX_GROUP_END_POINT; j++)
                                                {
                                                    apsStatus.apsGroupRecord[i]->EndPoints[j] = currentGroupAddressRecord.EndPoint[j];
                                                }
                                            }
                                            else
                                            {
                                                for(j = 0; j < NUM_USER_ENDPOINTS; j++)
                                                {
													if(NOW_I_AM_A_ROUTER())
														apsStatus.apsGroupRecord[i]->EndPoints[j] = Config_Simple_Descriptors_MTR[j+1].Endpoint;
													else if (NOW_I_AM_A_CORDINATOR())
                                                    	apsStatus.apsGroupRecord[i]->EndPoints[j] = Config_Simple_Descriptors_ESP[j+1].Endpoint;
                                                }
                                                apsStatus.apsGroupRecord[i]->EndPoints[j] = 0xFF;

                                            }
                                            apsStatus.apsGroupRecord[i]->EndPointIndex   = 0;
                                            apsStatus.apsGroupRecord[i]->SrcAddrMode     = addressModeL;
                                            apsStatus.apsGroupRecord[i]->SrcAddress      = params.APSDE_DATA_indication.SrcAddress;
                                            apsStatus.apsGroupRecord[i]->SrcEndpoint     = sourceEPL;
                                            apsStatus.apsGroupRecord[i]->ProfileId       = profileIDL;
                                            apsStatus.apsGroupRecord[i]->ClusterId       = clusterIDL;
                                            apsStatus.apsGroupRecord[i]->asduLength      = params.APSDE_DATA_indication.asduLength;
                                            apsStatus.apsGroupRecord[i]->asdu            = params.APSDE_DATA_indication.asdu;
                                            apsStatus.apsGroupRecord[i]->SecurityStatus  = params.APSDE_DATA_indication.SecurityStatus;
                                            apsStatus.apsGroupRecord[i]->CurrentRxPacket = CurrentRxPacket;
                                            // Set CurrentRxPacket to NULL so that the device can receive new packets.
                                            CurrentRxPacket                              = NULL;
                                            apsStatus.flags.bits.bGroupAddressing = 1;
                                            break;
                                        }
                                    }

                                    if( i == MAX_GROUP_RECORD_BUFFER )
                                    {
                                        //report error here
                                    }
                                #else
                                    APSDiscardRx();
                                #endif
                                return NO_PRIMITIVE;
                            }


                            // asduLength already in place
                            // *asdu already in place
                            // SrcAddress already in place
                            params.APSDE_DATA_indication.SrcAddrMode    = addressModeL;
                            params.APSDE_DATA_indication.WasBroadcast   = (apsFrameControl.bits.deliveryMode == APS_DELIVERY_BROADCAST);
                            params.APSDE_DATA_indication.DstEndpoint    = destinationEPL;
                            params.APSDE_DATA_indication.ClusterId      = clusterIDL;
                            params.APSDE_DATA_indication.ProfileId      = profileIDL;
                            params.APSDE_DATA_indication.SrcEndpoint    = sourceEPL;   // May be invalid.  SrcAddrMode will indicate.
                            params.APSDE_DATA_indication.RxTime         = TickGet();
                            params.APSDE_DATA_indication.DstAddrMode    = APS_ADDRESS_16_BIT;
                            /* The C32 compiler: preserve this address across layers to eliminate alignment issues  */
                            params.APSDE_DATA_indication.DstAddress.ShortAddr.Val = locDestAddress.Val;
                            
                            if (params.APSDE_DATA_indication.DstEndpoint == 0)
                            {
                                /* this is used to reset the polling rate to back to normal at the appl level */
                                #ifdef I_AM_END_DEVICE
                                    if( params.APSDE_DATA_indication.ClusterId.Val == END_DEVICE_BIND_rsp)
                                    {
                                        QuickPoll = 0;
                                    }
                                #endif

                                return ZDO_DATA_indication;
                            }
                            else
                            {
                                return APSDE_DATA_indication;
                            }
                        }

                    }
                    else if (apsFrameControl.bits.frameType == APS_FRAME_ACKNOWLEDGE)
                    {
                        #if (I_SUPPORT_FRAGMENTATION == 1)
                            if ( apsFrameControl.bits.extHeaderPresent == APS_EXTENDED_HEADER_INCLUDED )
                            {
                                if ( ( apsStatus.flags.bits.bFragmentationInProgress ) &&
                                     ( apsFragmentationInfo.WaitingForAck ) )
                                {
                                    BYTE    tempAckBitField = 0x01;

                                    APSDiscardRx();

                                    if ( BlockNumber != apsFragmentationInfo.ExpectingAckForTheBlockNumber )
                                    {
                                        return NO_PRIMITIVE;
                                    }

                                    apsFragmentationInfo.WaitingForAck = 0;
                                    if ( ( BlockNumber == apsFragmentationInfo.FirstBlockInWindow ) ||
                                         ( fragmentWindowSize == 1 ) )
                                    {
                                        // If the BlockNumber in the APS ACK has the value of First Block that has been
                                        // transmitted in the window, then we need to update AckBitField.
                                        apsFragmentationInfo.MissedBlockInfo.AckBitField = ACKBitField;
                                    }
                                    if ( ACKBitField == 0xFF )
                                    {
                                        apsFragmentationInfo.RetryCounter = 0;

                                        // We might have received the APS ACK for the retried block.
                                        if ( apsFragmentationInfo.IsThisBlockRetried )
                                        {
                                            // Update the bit corresponding to the block in the AckBitFied.
                                            // All other fields shall not be disturbed, because there may be some blocks that
                                            // needs to be reteied.
                                            apsFragmentationInfo.IsThisBlockRetried = 0;
                                            tempAckBitField = tempAckBitField << ( apsFragmentationInfo.MissedBlockInfo.RetriedBlockNumber - apsFragmentationInfo.FirstBlockInWindow );
                                            apsFragmentationInfo.MissedBlockInfo.AckBitField =
                                                apsFragmentationInfo.MissedBlockInfo.AckBitField | tempAckBitField;

                                            // Check whether any other blocks needs to be retried.
                                            RetryBlock(FALSE); // Not all blocks requires retransmission
                                        }
                                        // We have transmitted all the blocks in the window and acknowledged.
                                        if ( apsFragmentationInfo.MissedBlockInfo.AckBitField == 0xFF )
                                        {
                                            // Is this APS ACK received when we are at the last window.
                                            // This is condition check because, last block of the fragment packet that is transmitted
                                            // can be the last block in the window itself.
                                            if ( apsFragmentationInfo.LastBlockInWindow == apsFragmentationInfo.TotalNumberOfBlocks - 1 )
                                            {
                                                // In this case, we have made a successful fragment data transmission.
                                                // So send the confirmation to upper layer.
                                                SendFragmentDataConfirm(SUCCESS);
                                                return NO_PRIMITIVE;
                                            }

                                            // All the blocks in this window has been successfully transmitted and acknowledged.
                                            // Move on to next window, update the First Block and Last block in the window.
                                            apsFragmentationInfo.MissedBlockInfo.AckBitField = ACKBitField;
                                            apsFragmentationInfo.WindowSize++;
                                            apsFragmentationInfo.BlockNumberToTxOrRx = apsFragmentationInfo.LastBlockInWindow + 1;
                                            UpdateDataPayloadLengthAndAddress();
                                            apsFragmentationInfo.FirstBlockInWindow = apsFragmentationInfo.BlockNumberToTxOrRx;
                                            apsFragmentationInfo.LastBlockInWindow  =
                                                    ( fragmentWindowSize * apsFragmentationInfo.WindowSize ) - 1;

                                            // There may be cases where last block (last fragment packet) in the window is less the window size.
                                            // Example: Consider the following
                                            //      fragmentWindowSize = 5, TotalBlocks = 7, WindowSize = 2
                                            // When we are at the last window, the last block to be transmitted is 10. But TotalBlocks
                                            // to be transmitted is 7. So, in this case 7 itself should become the last block.
                                            if ( apsFragmentationInfo.LastBlockInWindow > ( apsFragmentationInfo.TotalNumberOfBlocks - 1 ) )
                                            {
                                                apsFragmentationInfo.LastBlockInWindow = apsFragmentationInfo.TotalNumberOfBlocks - 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // Some blocks have been missed. We need to do retry of each missed blocks.
                                        //RetryBlock(FALSE); // Not all blocks requires retransmission

                                        // We might have received the APS ACK for the retried block.
                                        if ( apsFragmentationInfo.IsThisBlockRetried )
                                        {
                                            // Update the bit corresponding to the block in the AckBitFied.
                                            // All other fields shall not be disturbed, because there may be some blocks that
                                            // needs to be reteied.
                                            apsFragmentationInfo.IsThisBlockRetried = 0;
                                            tempAckBitField = tempAckBitField << ( apsFragmentationInfo.MissedBlockInfo.RetriedBlockNumber - apsFragmentationInfo.FirstBlockInWindow );
                                            apsFragmentationInfo.MissedBlockInfo.AckBitField =
                                                apsFragmentationInfo.MissedBlockInfo.AckBitField | tempAckBitField;
                                        }
                                        // Check whether any other blocks needs to be retried.
                                        RetryBlock(FALSE); // Not all blocks requires retransmission
                                    }
                                    if (apsFragmentationInfo.RetryCounter == ( apscMaxFrameRetries + 1 ) )
                                    {
                                        // We have exceeded the retries.
                                        SendFragmentDataConfirm(APS_NO_ACK);
                                    }
                                    return NO_PRIMITIVE;
                                }
                            }
                        #endif //(I_SUPPORT_FRAGMENTATION == 1)

                        for (i=0; i<MAX_APS_FRAMES; i++)
                        {
                            if (apsConfirmationHandles[i] != NULL)
                            {
                                if (apsConfirmationHandles[i]->DstAddrMode == APS_ADDRESS_64_BIT)
                                {
                                    #if MAX_APS_ADDRESSES > 0
                                        if (!LookupAPSAddress( &apsConfirmationHandles[i]->DstAddress.LongAddr ))
                                        {
                                            ackSourceAddressL = currentAPSAddress.shortAddr;
                                        }
                                        else
                                        {
                                            continue;
                                        }
                                    #else
                                        continue;
                                    #endif
                                }
                                else
                                {
                                    ackSourceAddressL = apsConfirmationHandles[i]->DstAddress.ShortAddr;
                                }

                                if (ackSourceAddressL.Val == params.NLDE_DATA_indication.SrcAddress.Val)
                                {
                                    if( currentAPSCounter != apsConfirmationHandles[i]->APSCounter )
                                    {
                                        continue;
                                    }

                                    // If all the above tests pass, the frame has been ACK'd
                                    APSDiscardRx();

                                    params.APSDE_DATA_confirm.Status        = SUCCESS;

                                    // This is the same as when we get an NLDE_DATA_confirm
                                    goto FinishConfirmation;
                                }
                            }
                        }
                    }
                    // There are currently no APS-level commands

                #ifdef I_SUPPORT_SECURITY
                    else if( apsFrameControl.bits.frameType == APS_FRAME_COMMAND )
                    {
                        BYTE CommandIdentifier;

                        if( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP )
                        {
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                APSGet();
                                APSGet();
                            #else
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            #endif
                        }

                        if( DuplicatePacket(params.NLDE_DATA_indication.SrcAddress, currentAPSCounter, 0xff) )
                        {
                            APSDiscardRx();
                            return NO_PRIMITIVE;
                        }

                        CommandIdentifier = APSGet();

                        switch( CommandIdentifier )
                        {
                           #if I_SUPPORT_LINK_KEY == 1
                           //#ifndef I_AM_TRUST_CENTER
                            #ifndef I_AM_END_DEVICE
                                case APS_CMD_TUNNEL:
                                if(!I_AM_TRUST_CENTER){//
                                    LONG_ADDR childAddress;
                                    SHORT_ADDR childShortAddress;
                                    BYTE NTIndex;
                                    for(i = 0; i < 8; i++)
                                    {
                                        childAddress.v[i] = APSGet();
                                    }
                                    if(IsThisLongAddressKnown(&childAddress, &childShortAddress.v[0], 0x01)) // 0x01 means search in Neighbor and AddressMap table
                                    {
                                        if( INVALID_NEIGHBOR_KEY != ( NTIndex = (NWKLookupNodeByLongAddr(&childAddress))))
                                        {
                                            if( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD &&
                                                !currentNeighborRecord.bSecured )
                                            {
                                                currentNeighborRecord.bSecured = TRUE;
                                                #ifdef USE_EXTERNAL_NVM
                                                    PutNeighborRecord( neighborTable + (WORD)NTIndex * (WORD)sizeof(NEIGHBOR_RECORD), &currentNeighborRecord );
                                                #else
                                                    PutNeighborRecord( &(neighborTable[NTIndex]), &currentNeighborRecord );
                                                #endif
                                            }
                                        }
                                        while(params.NLDE_DATA_indication.NsduLength != 0)
                                        {
                                            TxBuffer[TxData++] = APSGet();
                                        }
                                        APSDiscardRx();
                                        params.NLDE_DATA_request.DstAddr.Val = childShortAddress.Val;
                                        params.NLDE_DATA_request.NsduHandle = NLME_GET_nwkBCSN();
                                        params.NLDE_DATA_request.BroadcastRadius    = DEFAULT_RADIUS;
                                        params.NLDE_DATA_request.DiscoverRoute      = ROUTE_DISCOVERY_ENABLE;
                                        params.NLDE_DATA_request.SecurityEnable = FALSE;
                                        params.NLDE_DATA_request.DstAddrMode = 0x02;    /* Destination Addressing Mode was missing */
                                        ZigBeeBlockTx();
                                        return NLDE_DATA_request;
                                    }
                                    else
                                    {
                                        APSDiscardRx();
                                        return NO_PRIMITIVE;
                                    }
                                }

                                break;
                               #endif // #ifndef I_AM_END_DEVICE:
                            //#endif //#ifndef I_AM_TRUST_CENTER
                            #endif //I_SUPPORT_LINK_KEY
                            //#ifndef I_AM_TRUST_CENTER
                                case APS_CMD_TRANSPORT_KEY:
                                if(!I_AM_TRUST_CENTER){
                                    BYTE KeyType = APSGet();
                                    BYTE SeqNum = 0x00;
                                    for(i = 0; i < 16; i++)
                                    {
                                        KeyVal.v[i] = APSGet();
                                    }

                                    switch( KeyType )
                                    {

                                        case NETWORK_KEY:
                                            SeqNum = APSGet(); // fall through
                                        case TC_LINK_KEY:
                                        {
                                            LONG_ADDR DstAddr;
                                            LONG_ADDR SrcAddr;
                                            for(i = 0; i < 8; i++)
                                            {
                                                DstAddr.v[i] = APSGet();
                                            }
                                            for(i = 0; i < 8; i++) {
                                                SrcAddr.v[i] = APSGet();

                                                /* added at NTS */
                                                transportKeySrcAddr.v[i] = SrcAddr.v[i];
                                            }


                                            /* For Zigbee 2006: if this was broadcast or I am the destination
                                             * then just handle the return indication here
                                            */
                                            if( NWKThisIsMyLongAddress(&DstAddr)   ||
                                               ( ( DstAddr.v[0] == 0x00) && ( DstAddr.v[1] == 0x00) &&
                                                 ( DstAddr.v[2] == 0x00) && ( DstAddr.v[3] == 0x00) &&
                                                 ( DstAddr.v[4] == 0x00) && ( DstAddr.v[5] == 0x00) &&
                                                 ( DstAddr.v[6] == 0x00) && ( DstAddr.v[7] == 0x00) ) )
                                            {
                                                params.APSME_TRANSPORT_KEY_indication.SrcShortAddress.Val = params.NLDE_DATA_indication.SrcAddress.Val;
                                                params.APSME_TRANSPORT_KEY_indication.KeyType = KeyType;
                                                for(i = 0; i < 8; i++)
                                                {
                                                    params.APSME_TRANSPORT_KEY_indication.SrcAddr.v[i] = SrcAddr.v[i];
                                                }
                                                params.APSME_TRANSPORT_KEY_indication.TransportKeyData.NetworkKey.KeySeqNumber = SeqNum;
                                                params.APSME_TRANSPORT_KEY_indication.Key = &KeyVal;
                                                APSDiscardRx();
                                                return APSME_TRANSPORT_KEY_indication;
                                            }
                                            #ifndef I_AM_END_DEVICE
                                                params.APSME_TRANSPORT_KEY_request.KeyType = KeyType;
                                                params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                                                params.APSME_TRANSPORT_KEY_request.DestinationAddress = DstAddr;
                                                switch(KeyType)
                                                {
                                                    case 0x01:
                                                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = 0;
                                                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = SeqNum;

                                                        break;
                                                    default:
                                                        break;

                                                }
                                                APSDiscardRx();
                                                /* ZigBee 2006: if device has key already always send new key or dummy key secure */
                                                #ifdef PRECONFIG_NWK_KEY
                                                    params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = TRUE;
                                                #else
                                                    params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = FALSE;
                                                #endif
                                                params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = FALSE;
                                                params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val = 0xFFFF;
                                                //return APSME_TRANSPORT_KEY_request;
                                                goto Send_Transport_Key;
                                            #endif // #ifndef I_AM_END_DEVICE
                                        }
                                        case APP_LINK_KEY:
                                        {
                                            LONG_ADDR partnerAddr;
                                            BYTE i;

                                            for(i = 0; i < 8; i++)
                                            {
                                                partnerAddr.v[i] = APSGet();
                                            }

                                            params.APSME_TRANSPORT_KEY_indication.Key = &KeyVal;
                                            for (i=0; i<8; i++)
                                            {
                                                params.APSME_TRANSPORT_KEY_indication.TransportKeyData.Application_LinkKey.PartnerAddress.v[i] = partnerAddr.v[i];
                                            }
                                            params.APSME_TRANSPORT_KEY_indication.TransportKeyData.Application_LinkKey.Initiator = APSGet();
                                            params.APSME_TRANSPORT_KEY_indication.KeyType = KeyType;
                                            APSDiscardRx();
                                            return APSME_TRANSPORT_KEY_indication;
                                        }
                                    }
                                    APSDiscardRx();
                                    return NO_PRIMITIVE;

                                }
                               //#endif //#ifndef I_AM_TRUST_CENTER

//#if defined(I_AM_TRUST_CENTER)
                            case APS_CMD_UPDATE_DEVICE:
                            if(I_AM_TRUST_CENTER){
                                LONG_ADDR   tempLongAddr;
                                BYTE        i;

                                if ( IsThisAddressKnown( params.NLDE_DATA_indication.SrcAddress,
                                        (BYTE *)&tempLongAddr ) )
                                {
                                    params.APSME_UPDATE_DEVICE_indication.ParentShortAddress =
                                        params.NLDE_DATA_indication.SrcAddress;
                                    params.APSME_UPDATE_DEVICE_indication.SrcAddress = tempLongAddr;

                                    for(i = 0; i < 8; i++)
                                    {
                                        params.APSME_UPDATE_DEVICE_indication.DeviceAddress.v[i] = APSGet();

                                        /* ZigBee 2006: prepare to store in APS table in case device joined in out of range */
                                        updateDevAPSAddr.longAddr.v[i] = params.APSME_UPDATE_DEVICE_indication.DeviceAddress.v[i];
                                    }
                                    params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.v[0] = APSGet();
                                    params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.v[1] = APSGet();
                                    /* ZigBee 2006: Just in case device is out of range and we didn't have its
                                    * address mappings, then store it now while we have a chance
                                    */
                                    updateDevAPSAddr.shortAddr.v[0] = params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.v[0];
                                    updateDevAPSAddr.shortAddr.v[1] = params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.v[1];

                                    params.APSME_UPDATE_DEVICE_indication.Status = APSGet();

                                    APSDiscardRx();
                                    return APSME_UPDATE_DEVICE_indication;
                                }
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            }
//#endif

//#ifndef I_AM_TRUST_CENTER
#ifndef I_AM_END_DEVICE
                            case APS_CMD_REMOVE_DEVICE:
                            if(!I_AM_TRUST_CENTER){
                                #if I_SUPPORT_LINK_KEY == 1
                                    if(params.NLDE_DATA_indication.SrcAddress.Val == TCLinkKeyInfo.trustCenterShortAddr.Val)
                                #endif
                                    {

                                        for(i = 0; i < 8; i++)
                                        {
                                             #if I_SUPPORT_LINK_KEY == 1
                                                params.APSME_REMOVE_DEVICE_indication.SrcAddress.v[i] =             TCLinkKeyInfo.trustCenterLongAddr.v[i];
                                           #endif   /*  I_SUPPORT_LINK_KEY == 1 */
                                            params.APSME_REMOVE_DEVICE_indication.ChildAddress.v[i] = APSGet();
                                        }
                                        APSDiscardRx();
                                        return APSME_REMOVE_DEVICE_indication;
                                     }
                                     #if I_SUPPORT_LINK_KEY == 1
                                         else
                                         {
                                             APSDiscardRx();
                                             return NO_PRIMITIVE;
                                         }
                                     #endif
                            }
//#endif    /* I_AM_TRUST_CENTER */
#endif   /* I_AM_END_DEVICE */

//#ifdef I_AM_TRUST_CENTER
                            case APS_CMD_REQUEST_KEY:
                            if(I_AM_TRUST_CENTER){
                                LONG_ADDR   tempLongAddr;
                                BYTE        i;
                                BYTE        keyType;

                                if ( IsThisAddressKnown( params.NLDE_DATA_indication.SrcAddress,
                                        (BYTE *)&tempLongAddr ) )
                                {
                                    params.APSME_REQUEST_KEY_indication.SrcShortAddress.Val =
                                        params.NLDE_DATA_indication.SrcAddress.Val;

                                    for (i=0; i<8; i++)
                                    {
                                        params.APSME_REQUEST_KEY_indication.SrcAddress.v[i] = tempLongAddr.v[7-i];
                                    }
                                    keyType = APSGet();
                                    if( APP_MASTER_KEY == keyType )
                                    {
                                        for(i = 0; i < 8; i++)
                                        {
                                            params.APSME_REQUEST_KEY_indication.PartnerAddress.v[i] = APSGet();
                                        }
                                    }
                                    params.APSME_REQUEST_KEY_indication.KeyType = keyType;
                                    APSDiscardRx();
                                    return APSME_REQUEST_KEY_indication;
                                }
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            }
//#endif
//#ifndef I_AM_TRUST_CENTER
                            case APS_CMD_SWITCH_KEY:
                            if(!I_AM_TRUST_CENTER){
                                LONG_ADDR   tempLongAddr;
                                BYTE        i;

                                if ( IsThisAddressKnown( params.NLDE_DATA_indication.SrcAddress,
                                        (BYTE *)&tempLongAddr ) )
                                {
                                    for (i=0; i<8; i++)
                                    {
                                        params.APSME_SWITCH_KEY_indication.SrcAddress.v[i] = tempLongAddr.v[i];
                                    }
                                    params.APSME_SWITCH_KEY_indication.KeySeqNumber = APSGet();
                                    APSDiscardRx();
                                    return APSME_SWITCH_KEY_indication;
                                }
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            }
//#endif

                            default:
                                break;
                        }
                    }
                #endif  // if I_SUPPORT SECURITY
                    APSDiscardRx();
                    return NO_PRIMITIVE;
                }
                break;
                #undef destinationEPL
                #undef clusterIDL
                #undef profileIDL
                #undef sourceEPL
                #undef addressModeL
                #undef ackSourceAddressL

            case APSDE_DATA_request:
                {
                    #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                        BYTE    EnableNwkLayerMulticast = 0x00;
                    #endif
                    #ifdef I_SUPPORT_BINDINGS
                        APS_FRAME_CONTROL   apsFrameControl;
                        BYTE                originalBinding;
                        BYTE                currentBinding = 0;
                    #endif
                    #ifdef I_SUPPORT_GROUP_ADDRESSING
                        BYTE                indexToBufferedGroupRecord = 0;
                    #endif // #ifdef I_SUPPORT_GROUP_ADDRESSING

                    /* Don't attempt to send any data if we are not to on the network */
                    //#ifdef I_AM_COORDINATOR
                    //if (!ZigBeeStatus.flags.bits.bNetworkFormed)
                    //#else
                    //if (!ZigBeeStatus.flags.bits.bNetworkJoined)
                    //#endif
                    if (((NOW_I_AM_A_CORDINATOR())&&!ZigBeeStatus.flags.bits.bNetworkFormed)
                    	||((NOW_I_AM_A_ROUTER())&&!ZigBeeStatus.flags.bits.bNetworkJoined) )
	                {
                        params.APSDE_DATA_confirm.DstAddrMode           = params.APSDE_DATA_request.DstAddrMode;
                        if(params.APSDE_DATA_request.DstAddrMode != APS_ADDRESS_NOT_PRESENT)
                        {
                            params.APSDE_DATA_confirm.DstAddress        = params.APSDE_DATA_request.DstAddress;  //macPIB.macShortAddress.byte.MSB;
                            params.APSDE_DATA_confirm.DstEndpoint       = params.APSDE_DATA_request.DstEndpoint;
                        }
                        params.APSDE_DATA_confirm.SrcEndpoint       = params.APSDE_DATA_request.SrcEndpoint;
                        params.APSDE_DATA_confirm.Status            = NWK_INVALID_REQUEST;
                        ZigBeeUnblockTx();
                        return APSDE_DATA_confirm;
                    }

                    #if (I_SUPPORT_FRAGMENTATION == 1)
                        if (apsStatus.flags.bits.bFragmentationInProgress)
                        {
                            // When Fragementation is under progress, we do not process any other request
                            // from the application. So we return with ERROR status to application, so that
                            // application can issue the data request again after fragmentation is complete.
                            // Currently, the Status value passed in the confirmation is not specified by
                            // the specification. So, we have defined our own Status code.
                            params.APSDE_DATA_confirm.DstAddrMode           = params.APSDE_DATA_request.DstAddrMode;
                            if(params.APSDE_DATA_request.DstAddrMode != APS_ADDRESS_NOT_PRESENT)
                            {
                                params.APSDE_DATA_confirm.DstAddress        = params.APSDE_DATA_request.DstAddress;  //macPIB.macShortAddress.byte.MSB;
                                params.APSDE_DATA_confirm.DstEndpoint       = params.APSDE_DATA_request.DstEndpoint;
                            }
                            params.APSDE_DATA_confirm.SrcEndpoint       = params.APSDE_DATA_request.SrcEndpoint;
                            params.APSDE_DATA_confirm.Status            = APS_FRAMGENTATION_UNDER_PROGRESS;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }
                    #endif // (I_SUPPORT_FRAGMENTATION == 1)
                    if ( ( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_16_BIT ) &&
                         ( params.APSDE_DATA_request.DstAddress.ShortAddr.Val == macPIB.macShortAddress.Val ) )
                    {
                        BYTE dataLength;

                        params.APSDE_DATA_confirm.DstAddrMode       = params.APSDE_DATA_request.DstAddrMode;
                        params.APSDE_DATA_confirm.DstAddress        = params.APSDE_DATA_request.DstAddress;
                        params.APSDE_DATA_confirm.DstEndpoint       = params.APSDE_DATA_request.DstEndpoint;
                        params.APSDE_DATA_confirm.SrcEndpoint       = params.APSDE_DATA_request.SrcEndpoint;

                        dataLength = TxData;
                        if (params.APSDE_DATA_request.DstEndpoint == 0x00)
                        {
                            dataLength = params.APSDE_DATA_request.asduLength;
                        }
                        apsStatus.ackMessage.asduLength     = dataLength;
                        ptr                                 = (BYTE *)SRAMalloc(dataLength);
                        if ( ptr == NULL )
                        {
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }
                        apsStatus.ackMessage.asdu = ptr;
                        TxData = TX_DATA_START;
                        if (params.APSDE_DATA_request.DstEndpoint == 0x00)
                        {
                            TxBuffer[TxData] = APSCounter++;
                            *ptr++ = TxBuffer[TxData++];
                        }
                        for (i=0;i<dataLength;i++)
                        {
                            *ptr++ = TxBuffer[TxData++];
                        }
                        apsStatus.ackMessage.ProfileId      = params.APSDE_DATA_request.ProfileId;
                        apsStatus.ackMessage.SrcAddrMode    = APS_ADDRESS_16_BIT;
                        apsStatus.ackMessage.SrcAddress     = macPIB.macShortAddress;
                        apsStatus.ackMessage.WasBroadcast   = 0x00;
                        apsStatus.ackMessage.SrcEndpoint    = params.APSDE_DATA_request.SrcEndpoint;
                        apsStatus.ackMessage.DstEndpoint    = params.APSDE_DATA_request.DstEndpoint;
                        apsStatus.ackMessage.ClusterId.Val  = params.APSDE_DATA_request.ClusterId.Val;

                        if ( params.APSDE_DATA_request.TxOptions.bits.securityEnabled )
                        {
                            apsStatus.ackMessage.SecurityStatus = APS_SECURED_LINK_KEY;
                        }
                        else if ( params.APSDE_DATA_request.TxOptions.bits.useNWKKey )
                        {
                            apsStatus.ackMessage.SecurityStatus = APS_SECURED_NWK_KEY;
                        }
                        else
                        {
                            apsStatus.ackMessage.SecurityStatus = APS_UNSECURED;
                        }

                        apsStatus.flags.bits.bDataIndicationPending = 1;
                        apsStatus.flags.bits.bLoopBackData          = 1;

                        #ifdef I_SUPPORT_BINDINGS
                            if( (params.APSDE_DATA_request.ClusterId.Val == BIND_req)  ||
                                (params.APSDE_DATA_request.ClusterId.Val == UNBIND_req)
                               )
                            {
                                SentBindRequest = 1;
                            }
                        #endif

                        params.APSDE_DATA_confirm.Status            = SUCCESS;
                        ZigBeeUnblockTx();
                        return APSDE_DATA_confirm;
                    }

                    /*ZigBee 2006:  If I am the coordinator and doing a self end device bind request, capture the
                     * request here, and make sure its tagged as fromself
                    */
                    if (NOW_I_AM_A_CORDINATOR()){//#ifdef I_AM_COORDINATOR
                        if( (params.APSDE_DATA_request.ClusterId.Val == END_DEVICE_BIND_req)
                        && (params.APSDE_DATA_request.ProfileId.Val == ZDP_PROFILE_ID  ) )
                        {
                            return ZDO_END_DEVICE_BIND_req;
                        }
                    }//#endif


                    #ifdef I_SUPPORT_GROUP_ADDRESSING
                        // addressing myself, because I will not be able to receive my own broadcast message
                        if( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP )
                        {

                            if( GetEndPointsFromGroup(params.APSDE_DATA_request.DstAddress.ShortAddr) != MAX_GROUP )
                            {
                                for(j = 0; j < MAX_GROUP_RECORD_BUFFER; j++)
                                {
                                    if( apsStatus.apsGroupRecord[j] == NULL )
                                    {
                                        BYTE k;
                                        #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                            PERSISTENCE_PIB currentPIB;

                                            GetPersistenceStorage((void *)&currentPIB);
                                        #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)

                                        apsStatus.apsGroupRecord[j] = (APS_GROUP_RECORD *)SRAMalloc(sizeof(APS_GROUP_RECORD));
                                        if( apsStatus.apsGroupRecord[j] == NULL )
                                        {
                                            //  report error here
                                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                                            ZigBeeUnblockTx();
                                            return APSDE_DATA_confirm;
                                        }
                                        // Take backup of the index
                                        indexToBufferedGroupRecord = j;

                                        for(k = 0; k < MAX_GROUP_END_POINT; k++)
                                        {
                                            apsStatus.apsGroupRecord[j]->EndPoints[k] = currentGroupAddressRecord.EndPoint[k];
                                        }
                                        apsStatus.apsGroupRecord[j]->EndPointIndex          = 0;
                                        apsStatus.apsGroupRecord[j]->SrcAddrMode            = APS_ADDRESS_16_BIT;
                                        apsStatus.apsGroupRecord[j]->SrcAddress.ShortAddr   = macPIB.macShortAddress;
                                        apsStatus.apsGroupRecord[j]->SrcEndpoint            = params.APSDE_DATA_request.SrcEndpoint;
                                        apsStatus.apsGroupRecord[j]->ProfileId.Val          = params.APSDE_DATA_request.ProfileId.Val;
                                        apsStatus.apsGroupRecord[j]->ClusterId.Val          = params.APSDE_DATA_request.ClusterId.Val;
                                        apsStatus.apsGroupRecord[j]->asduLength             = TxData;
                                        apsStatus.apsGroupRecord[j]->asdu                   = (BYTE *)SRAMalloc(TxData);
                                        if( apsStatus.apsGroupRecord[j]->asdu == NULL )
                                        {
                                            nfree (apsStatus.apsGroupRecord[j]);
                                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                                            ZigBeeUnblockTx();
                                            return APSDE_DATA_confirm;
                                        }
                                        if ( params.APSDE_DATA_request.TxOptions.bits.useNWKKey )
                                        {
                                            apsStatus.apsGroupRecord[j]->SecurityStatus         = APS_SECURED_NWK_KEY;
                                        }
                                        else
                                        {
                                            apsStatus.apsGroupRecord[j]->SecurityStatus         = APS_UNSECURED;
                                        }
                                        apsStatus.apsGroupRecord[j]->CurrentRxPacket        = apsStatus.apsGroupRecord[j]->asdu;
                                        // Set CurrentRxPacket to NULL so that the device can receive new packets.
                                        CurrentRxPacket                                     = NULL;
                                        //apsStatus.flags.bits.bGroupAddressing = 1;

                                        // Copy the message.
                                        for (k=0; k<TxData; k++)
                                        {
                                            apsStatus.apsGroupRecord[j]->asdu[k] = TxBuffer[k];
                                        }

                                        #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                            if (currentPIB.nwkUseMulticast)
                                            {
                                                EnableNwkLayerMulticast = 0x01; // Member Mode Multicast.
                                            }
                                        #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                // I am not the member of the group.
                                #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                    EnableNwkLayerMulticast = 0x00; // Non Member Mode Multicast
                                #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                            }
                            //#ifdef I_AM_ROUTER
                            /* if group addressing but no binding record exist, then return */


                            //#endif
                        }

                    #endif // #ifdef I_SUPPORT_GROUP_ADDRESSING


                    #ifndef I_SUPPORT_BINDINGS
                        // Bindings are not supported, so all messages are buffered in the aplMessages buffer
                        // and all messages have an apsConfirmationHandles entry.

                        // Validate what we can before allocating space.
                        if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT)
                        {
                            if (!APSFromLongToShort( &params.APSDE_DATA_request.DstAddress.LongAddr ))
                            {
                                // We do not have a short address for this long address, so return an error.
                                params.APSDE_DATA_confirm.Status = APS_NO_SHORT_ADDRESS;
                                ZigBeeUnblockTx();
                                return APSDE_DATA_confirm;
                            }
                        }

                        // Prepare a confirmation handle entry.
                        for (i=0; (i<MAX_APS_FRAMES) && (apsConfirmationHandles[i]!=NULL); i++) {}
                        if ((i == MAX_APS_FRAMES) ||
                            ((apsConfirmationHandles[i] = (APS_FRAMES *)SRAMalloc( sizeof(APS_FRAMES) )) == NULL))
                        {
                            // There was no room for another frame, so return an error.
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Prepare an APL message buffer
                        for (j=0; (j<MAX_APL_FRAMES) && (apsStatus.aplMessages[j]!=NULL); j++) {}
                        if ((j == MAX_APL_FRAMES) ||
                            ((apsStatus.aplMessages[j] = (APL_FRAME_INFO *)SRAMalloc( sizeof(APL_FRAME_INFO) )) == NULL))
                        {
                            // There was no room for another frame, so return an error.
                            nfree( apsConfirmationHandles[i] );
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Load the confirmation handle entry.  Set nsduHandle to INVALID, since we do the actual
                        // transmission from the background.
                        apsConfirmationHandles[i]->nsduHandle                   = INVALID_NWK_HANDLE;
                        apsConfirmationHandles[i]->DstAddrMode                  = params.APSDE_DATA_request.DstAddrMode;
                        apsConfirmationHandles[i]->SrcEndpoint                  = params.APSDE_DATA_request.SrcEndpoint;
                        #ifdef I_SUPPORT_GROUP_ADDRESSING
                            if( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP )
                                apsConfirmationHandles[i]->DstEndpoint          = 0xFE;
                            else
                        #endif
                        apsConfirmationHandles[i]->DstEndpoint                  = params.APSDE_DATA_request.DstEndpoint;
                        apsConfirmationHandles[i]->DstAddress                   = params.APSDE_DATA_request.DstAddress; // May change later...
                        apsConfirmationHandles[i]->flags.bits.nAPLFrameIndex    = j;
                        apsConfirmationHandles[i]->flags.bits.bWaitingForAck    = FALSE;    // May change later...
                        apsConfirmationHandles[i]->APSCounter                   = APSCounter;
                        apsConfirmationHandles[i]->timeStamp                    = TickGet();

                        // Start loading the APL message info.
                        if ((apsStatus.aplMessages[j]->message = SRAMalloc( TxData )) == NULL)
                        {
                            nfree( apsStatus.aplMessages[j] );
                            nfree( apsConfirmationHandles[i] );
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }
                        apsStatus.aplMessages[j]->flags.Val                     = 0;
                        apsStatus.aplMessages[j]->profileID                     = params.APSDE_DATA_request.ProfileId;
                        apsStatus.aplMessages[j]->radiusCounter                 = params.APSDE_DATA_request.RadiusCounter;
                        apsStatus.aplMessages[j]->clusterID.Val                 = params.APSDE_DATA_request.ClusterId.Val;
                        apsStatus.aplMessages[j]->confirmationIndex             = i;
                        apsStatus.aplMessages[j]->flags.bits.nDiscoverRoute     = params.APSDE_DATA_request.DiscoverRoute;
                        apsStatus.aplMessages[j]->shortDstAddress               = params.APSDE_DATA_request.DstAddress.ShortAddr; // May not be correct - fixed later.
                        apsStatus.aplMessages[j]->flags.bits.bSendMessage       = 1;
                        apsStatus.aplMessages[j]->messageLength                 = TxData;
                        apsStatus.aplMessages[j]->APSCounter                    = APSCounter++;
                        apsStatus.aplMessages[j]->flags.bits.nwkSecurity    =
                                params.APSDE_DATA_request.TxOptions.bits.useNWKKey;

                        // Start building the frame control.
                        apsStatus.aplMessages[j]->apsFrameControl.Val = APS_FRAME_DATA;   // APS_DELIVERY_DIRECT

                        apsStatus.aplMessages[j]->apsFrameControl.bits.security =
                        params.APSDE_DATA_request.TxOptions.bits.securityEnabled;


                        if (
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                (params.APSDE_DATA_request.DstAddrMode != APS_ADDRESS_GROUP) &&
                            #endif
                            params.APSDE_DATA_request.TxOptions.bits.acknowledged)
                        {
                            apsConfirmationHandles[i]->flags.bits.bWaitingForAck = TRUE;
                            apsStatus.aplMessages[j]->apsFrameControl.bits.acknowledgeRequest = APS_ACK_REQUESTED;
                            apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts  = apscMaxFrameRetries + 1;
                            //apsStatus.aplMessages[j]->flags.bits.bRouteRepair = 1;
                        }
                        else
                        {
                            apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts  = 1;
                        }

                        #if MAX_APS_ADDRESSES > 0
                            if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT)
                            {
                                // If we get to here, then currentAPSAddress is waiting for us.
                                apsStatus.aplMessages[j]->shortDstAddress = currentAPSAddress.shortAddr;
                            }
                            else
                        #endif

                        #ifdef I_SUPPORT_GROUP_ADDRESSING
                            if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                            {
                                apsStatus.aplMessages[j]->apsFrameControl.bits.deliveryMode = APS_DELIVERY_GROUP;
                            }
                            else
                        #endif
                        if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_16_BIT)
                        {
                            // NOTE According to the spec, broadcast at this level means the frame
                            // goes to all devices AND all endpoints.  That makes no sense - EP0 Cluster 0
                            // means something very different from Cluster 0 on any other endpoint.  We'll
                            // set it for all devices...
                            if (params.APSDE_DATA_request.DstAddress.ShortAddr.Val == 0xFFFF)
                            {
                                apsStatus.aplMessages[j]->apsFrameControl.bits.deliveryMode = APS_DELIVERY_BROADCAST;
                            }
                        }
                        // Buffer the message payload.
                        ptr = apsStatus.aplMessages[i]->message;
                        i = 0;
                        while (TxData--)
                        {
                            *ptr++ = TxBuffer[i++];
                        }

                        apsStatus.flags.bits.bFramesAwaitingTransmission = 1;
                        ZigBeeUnblockTx();
                        return NO_PRIMITIVE;

                    #else

                        // Bindings are supported, so we are either a coordinator or a router.  If we are sending
                        // a direct message or an indirect message to the coordinator, we need to create an
                        // aplMessages and an apsConfirmationHandles entry.  If we are trying to send indirect
                        // messages from the coordinator, we need to create an indirectMessages queue entry.

                        // Start building the frame control.
                        /* this will be used to speed up the poll rate during end_device bind */
                        #ifdef I_AM_END_DEVICE
                            if(params.APSDE_DATA_request.ClusterId.Val == END_DEVICE_BIND_req)
                            {
                                QuickPoll = 1;
                            }
                        #endif

                        #ifdef I_SUPPORT_BINDINGS
                            if( (params.APSDE_DATA_request.ClusterId.Val == BIND_req)  ||
                                (params.APSDE_DATA_request.ClusterId.Val == UNBIND_req)
                               )
                            {
                                SentBindRequest = 1;
                            }
                        #endif

                        apsFrameControl.Val = APS_FRAME_DATA;   // and APS_DELIVERY_DIRECT

                        if (params.APSDE_DATA_request.TxOptions.bits.acknowledged)
                        {
                            apsFrameControl.bits.acknowledgeRequest = APS_ACK_REQUESTED;
                        }

                        if (params.APSDE_DATA_request.TxOptions.bits.securityEnabled)
                        {
                            apsFrameControl.bits.security = APS_SECURITY_ON;

                        }

                        // For group, Indirect, and broadcast data transmissions APS ACK and APS SECURITY should be disabled
                        if ( ( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP ) ||
                             ( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_NOT_PRESENT ) ||
                             ( ( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_16_BIT ) &&
                               ( params.APSDE_DATA_request.DstAddress.ShortAddr.Val > 0xFFF7 ) ) )
                        {
                            apsFrameControl.bits.security = APS_SECURITY_OFF;

                            // APS ACK can be requested for Indirect Data Transmission.
                            if ( params.APSDE_DATA_request.DstAddrMode != APS_ADDRESS_NOT_PRESENT )
                            {
                                apsFrameControl.bits.acknowledgeRequest = APS_ACK_NOT_REQUESTED;
                            }
                        }

                        // Validate what we can before allocating space, and determine the addressing mode.
                        if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT)
                        {
                            if (!APSFromLongToShort( &params.APSDE_DATA_request.DstAddress.LongAddr ))
                            {
                                // We do not have a short address for this long address, so return an error.
                                params.APSDE_DATA_confirm.Status = APS_NO_SHORT_ADDRESS;
                                ZigBeeUnblockTx();
                                #ifdef I_AM_END_DEVICE
                                    QuickPoll = 0;
                                #endif
                                return APSDE_DATA_confirm;
                            }
                        }
                        #ifdef I_SUPPORT_GROUP_ADDRESSING
                            else if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                            {
                                #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                    PERSISTENCE_PIB currentPIB;

                                    GetPersistenceStorage((void *)&currentPIB);
                                    if ( currentPIB.nwkUseMulticast )
                                    {
                                        apsFrameControl.bits.deliveryMode = APS_DELIVERY_GROUP;
                                    }
                                    else
                                #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                    {
                                        apsFrameControl.bits.deliveryMode = APS_DELIVERY_GROUP;
                                    }
                            }
                        #endif
                        else if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_16_BIT)
                        {
                            // NOTE According to the spec, broadcast at this level means the frame
                            // goes to all devices AND all endpoints.  That makes no sense - EP0 Cluster 0
                            // means something very different from Cluster 0 on any other endpoint.  We'll
                            // set it for all devices...
                            if (params.APSDE_DATA_request.DstAddress.ShortAddr.Val > 0xFFF7)
                            {
                                apsFrameControl.bits.deliveryMode = APS_DELIVERY_BROADCAST;
                            }
                            else
                            {
                                apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT; //which is 0x00
                            }
                        }
                        else
                        {
                            // This is for Indirect data transmission.
                            if(LookupSourceBindingInfo(macPIB.macShortAddress, params.APSDE_DATA_request.SrcEndpoint, params.APSDE_DATA_request.ClusterId ) == END_BINDING_RECORDS)
                            {
                                // I don't have a binding record for it, so issue APSDE_DATA_confirm primitives with
                                // status of NO_BOND_DEVICE
                                params.APSDE_DATA_confirm.Status = APS_NO_BOUND_DEVICE;
                                /* ZigBee 2006: When toggling end_device_bind-request clusters, this is encountered
                                 * a lot, so proper cleanup is neccessary here
                                */
                                ZigBeeUnblockTx();
                                #ifdef I_AM_END_DEVICE
                                    QuickPoll = 0;
                                #endif
                                return APSDE_DATA_confirm;
                            }
                            currentBinding = currentBindingRecord.nextBindingRecord;
                            originalBinding = currentBinding;
                        }

                        // We are sending either an indirect message to the coordinator or a direct message to someone.
                        // Create both an aplMessages entry and an apsConfirmationHandles entry.
BufferAPLMessage:
                        // Prepare a confirmation handle entry.
                        for (i=0; (i<MAX_APS_FRAMES) && (apsConfirmationHandles[i]!=NULL); i++) {}
                        if ((i == MAX_APS_FRAMES) ||
                            ((apsConfirmationHandles[i] = (APS_FRAMES *)SRAMalloc( sizeof(APS_FRAMES) )) == NULL))
                        {
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                                {
                                    nfree (apsStatus.apsGroupRecord[indexToBufferedGroupRecord]->asdu);
                                    nfree (apsStatus.apsGroupRecord[indexToBufferedGroupRecord]);
                                }
                            #endif

                            // There was no room for another frame, so return an error.
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Prepare an APL message buffer
                        for (j=0; (i<MAX_APL_FRAMES) && (apsStatus.aplMessages[j]!=NULL); j++) {}
                        if ((j == MAX_APL_FRAMES) ||
                            ((apsStatus.aplMessages[j] = (APL_FRAME_INFO *)SRAMalloc( sizeof(APL_FRAME_INFO) )) == NULL))
                        {
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                                {
                                    nfree (apsStatus.apsGroupRecord[indexToBufferedGroupRecord]->asdu);
                                    nfree (apsStatus.apsGroupRecord[indexToBufferedGroupRecord]);
                                }
                            #endif

                            // There was no room for another frame, so return an error.
                            nfree( apsConfirmationHandles[i] );
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Start loading the APL message info.
                        if ((apsStatus.aplMessages[j]->message = SRAMalloc( TxData )) == NULL)
                        {
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                                {
                                    nfree (apsStatus.apsGroupRecord[indexToBufferedGroupRecord]->asdu);
                                    nfree (apsStatus.apsGroupRecord[indexToBufferedGroupRecord]);
                                }
                            #endif

                            nfree( apsStatus.aplMessages[j] );
                            nfree( apsConfirmationHandles[i] );
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Load the confirmation handle entry.  Set nsduHandle to INVALID, and do not load
                        // timeStamp, since we do the actual transmission from the background.
                        apsConfirmationHandles[i]->nsduHandle                   = INVALID_NWK_HANDLE;
                        apsConfirmationHandles[i]->SrcEndpoint                  = params.APSDE_DATA_request.SrcEndpoint;
                        apsConfirmationHandles[i]->APSCounter                   = APSCounter;
                        apsConfirmationHandles[i]->flags.bits.nAPLFrameIndex    = j;
                        apsConfirmationHandles[i]->flags.bits.bWaitingForAck    = 0;    // May change later...
                        apsConfirmationHandles[i]->flags.bits.bWeAreOriginator  = 1;

                        // For ZigBee 2006
                        apsStatus.aplMessages[j]->flags.Val                      = 0;
                        apsStatus.aplMessages[j]->profileID                      = params.APSDE_DATA_request.ProfileId;
                        apsStatus.aplMessages[j]->radiusCounter                  = params.APSDE_DATA_request.RadiusCounter;
                        apsStatus.aplMessages[j]->clusterID                      = params.APSDE_DATA_request.ClusterId;
                        apsStatus.aplMessages[j]->confirmationIndex              = i;
                        apsStatus.aplMessages[j]->shortDstAddress                = params.APSDE_DATA_request.DstAddress.ShortAddr; // May not be correct - fixed later.
                        apsStatus.aplMessages[j]->messageLength                  = TxData;
                        apsStatus.aplMessages[j]->flags.bits.nDiscoverRoute      = params.APSDE_DATA_request.DiscoverRoute;
                        apsStatus.aplMessages[j]->flags.bits.bSendMessage        = 1;
                        apsStatus.aplMessages[j]->APSCounter                     = APSCounter++;
                        apsStatus.aplMessages[j]->flags.bits.nwkSecurity    =
                                    params.APSDE_DATA_request.TxOptions.bits.useNWKKey;
                        #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                            apsStatus.aplMessages[j]->EnableNwkLayerMulticast = EnableNwkLayerMulticast;
                        #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)

                        if (params.APSDE_DATA_request.TxOptions.bits.acknowledged)
                        {
                            // For group, Indirect, and broadcast data transmissions APS ACK should be disabled
                            if ( ( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP ) ||
                                 ( ( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_16_BIT ) &&
                                   ( params.APSDE_DATA_request.DstAddress.ShortAddr.Val > 0xFFF7 ) ) )
                            {
                                apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts = 1;
                            }
                            else
                            {
                                apsConfirmationHandles[i]->flags.bits.bWaitingForAck   = 1;
                                apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts = apscMaxFrameRetries + 1;
                            }
                        }
                        else
                        {
                            apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts = 1;
                        }

                        #if MAX_APS_ADDRESSES > 0
                            if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT)
                            {
                                // If we get to here, then currentAPSAddress is waiting for us.
                                apsStatus.aplMessages[j]->shortDstAddress = currentAPSAddress.shortAddr;
                            }
                            else
                        #endif

                        #ifdef I_SUPPORT_GROUP_ADDRESSING
                            if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                            {
                                #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                    PERSISTENCE_PIB currentPIB;

                                    GetPersistenceStorage((void *)&currentPIB);
                                    if ( currentPIB.nwkUseMulticast )
                                    {
                                        apsFrameControl.bits.deliveryMode = APS_DELIVERY_GROUP;
                                        params.APSDE_DATA_request.DstEndpoint = 0xFF;
                                    }
                                    else
                                #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                    {
                                        apsFrameControl.bits.deliveryMode       = APS_DELIVERY_GROUP;
                                    }
                            }
                        #endif

                        if( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_NOT_PRESENT )
                        {
                            if( currentBinding != END_BINDING_RECORDS )
                            {
                                BINDING_RECORD thisBindingRecord;

                                #ifdef USE_EXTERNAL_NVM
                                    pCurrentBindingRecord = apsBindingTable + (WORD)(currentBinding) * (WORD)sizeof(BINDING_RECORD);
                                #else
                                    pCurrentBindingRecord = &apsBindingTable[currentBinding];
                                #endif
                                GetBindingRecord(&thisBindingRecord, pCurrentBindingRecord);

                                if( thisBindingRecord.endPoint == 0xFE )
                                {
                                    apsConfirmationHandles[i]->DstAddrMode              = APS_ADDRESS_GROUP;
                                    apsFrameControl.bits.deliveryMode                   = APS_DELIVERY_GROUP;
                                }
                                else
                                {
                                    apsConfirmationHandles[i]->DstAddrMode              = APS_ADDRESS_16_BIT;
                                    apsFrameControl.bits.deliveryMode                   = APS_DELIVERY_DIRECT;
                                }
                                apsConfirmationHandles[i]->DstEndpoint              = thisBindingRecord.endPoint;
                                apsConfirmationHandles[i]->DstAddress.ShortAddr     = thisBindingRecord.shortAddr;
                                /* For ZigBee 2006: The proper destination address is dest address
                                 * that is in the destination bind record
                                */
                                apsStatus.aplMessages[j]->shortDstAddress.Val       = thisBindingRecord.shortAddr.Val;

                                apsStatus.aplMessages[j]->apsFrameControl = apsFrameControl;
                                apsStatus.aplMessages[j]->apsFrameControl.bits.security =
                                    params.APSDE_DATA_request.TxOptions.bits.securityEnabled;
                                // Buffer the message payload.
                                ptr = apsStatus.aplMessages[i]->message;
                                i = 0;
                                while (TxData > i)
                                {
                                    *ptr++ = TxBuffer[i++];
                                }

                                currentBinding = thisBindingRecord.nextBindingRecord;
                                /* For ZigBee 2006: This check is necessary in order to terminate
                                 * the sending of requests at the end of the bind list
                                 */
                                if(currentBinding == END_BINDING_RECORDS)
                                {
                                    goto endofbindlist;
                                }
                                goto BufferAPLMessage;
                            }
                        }
                        else
                        {
                            apsConfirmationHandles[i]->DstAddrMode                  = params.APSDE_DATA_request.DstAddrMode;
                            apsConfirmationHandles[i]->DstEndpoint                  = params.APSDE_DATA_request.DstEndpoint;
                            apsConfirmationHandles[i]->DstAddress                   = params.APSDE_DATA_request.DstAddress;

                            apsStatus.aplMessages[j]->apsFrameControl = apsFrameControl;
                            apsStatus.aplMessages[j]->apsFrameControl.bits.security = params.APSDE_DATA_request.TxOptions.bits.securityEnabled;

                            // Buffer the message payload.
                            ptr = apsStatus.aplMessages[i]->message;
                            i = 0;
                            while (TxData > i)
                            {
                                *ptr++ = TxBuffer[i++];
                            }
                        }

endofbindlist:          apsStatus.flags.bits.bFramesAwaitingTransmission = 1;
                        ZigBeeUnblockTx();
                        return NO_PRIMITIVE;

                    #endif
                }
                break;

            case APSME_ADD_GROUP_request:
                {
                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        params.APSME_ADD_GROUP_confirm.Status = APS_INVALID_PARAMERTER; //GROUP_INVALID_PARAMETER;
                    }else{//#else
                        #ifndef I_SUPPORT_GROUP_ADDRESSING
                            params.APSME_ADD_GROUP_confirm.Status = APS_INVALID_PARAMERTER; //GROUP_INVALID_PARAMETER;
                        #else
                            params.APSME_ADD_GROUP_confirm.Status       = AddGroup(params.APSME_ADD_GROUP_request.GroupAddress, params.APSME_ADD_GROUP_request.Endpoint);
                        #endif
                    }//#endif
                    params.APSME_ADD_GROUP_confirm.GroupAddress = params.APSME_ADD_GROUP_request.GroupAddress;
                    params.APSME_ADD_GROUP_confirm.Endpoint     = params.APSME_ADD_GROUP_request.Endpoint;
                    return APSME_ADD_GROUP_confirm;
                }
                break;

            case APSME_REMOVE_GROUP_request:
                {
                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        params.APSME_REMOVE_GROUP_confirm.Status            = APS_INVALID_PARAMERTER; //GROUP_INVALID_PARAMETER;
                    }else{//#else
                        #ifndef I_SUPPORT_GROUP_ADDRESSING
                            params.APSME_REMOVE_GROUP_confirm.Status            = APS_INVALID_PARAMERTER; //GROUP_INVALID_PARAMETER;
                        #else
                            params.APSME_REMOVE_GROUP_confirm.Status            = RemoveGroup(params.APSME_REMOVE_GROUP_request.GroupAddress.Val, params.APSME_REMOVE_GROUP_request.Endpoint);
                        #endif
                    }//#endif
                    params.APSME_REMOVE_GROUP_confirm.GroupAddress.Val  =  params.APSME_REMOVE_GROUP_request.GroupAddress.Val;
                    params.APSME_REMOVE_GROUP_confirm.Endpoint          =  params.APSME_REMOVE_GROUP_request.Endpoint;
                    return APSME_REMOVE_GROUP_confirm;
                }
                break;

            case APSME_REMOVE_ALL_GROUPS_request:
                {
                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        params.APSME_REMOVE_ALL_GROUPS_confirm.Status = APS_INVALID_PARAMERTER; //GROUP_INVALID_PARAMETER;
                    }else{//#else
                        #ifndef I_SUPPORT_GROUP_ADDRESSING
                            params.APSME_REMOVE_ALL_GROUPS_confirm.Status = APS_INVALID_PARAMERTER; //GROUP_INVALID_PARAMETER;
                        #else
                            params.APSME_REMOVE_ALL_GROUPS_confirm.Status   = RemoveGroup(0xFFFF, params.APSME_REMOVE_ALL_GROUPS_request.Endpoint);
                        #endif
                    }//#endif
                    params.APSME_REMOVE_ALL_GROUPS_confirm.Endpoint = params.APSME_REMOVE_ALL_GROUPS_request.Endpoint;
                    return APSME_REMOVE_ALL_GROUPS_confirm;
                }
                break;

            case APSME_BIND_request:
                #ifndef I_SUPPORT_BINDINGS
                    params.APSME_BIND_confirm.Status = BIND_NOT_SUPPORTED;
                    return APSME_BIND_confirm;
                #else
                    // NOTE - The spec allows bindings to be created even if we are not
                    // associated.  However, it doesn't allow us to unbind...
                    {
                        SHORT_ADDR  srcShortAddress;
                        SHORT_ADDR  dstShortAddress;

                        if( APSFromLongToShort( &params.APSME_BIND_request.SrcAddr ) == FALSE )
                        {
                            params.APSME_BIND_confirm.Status = BIND_ILLEGAL_DEVICE;
                            return APSME_BIND_confirm;
                        }
                        else
                        {
                            srcShortAddress = currentAPSAddress.shortAddr; //currentNeighborRecord.shortAddr;
                        }

                        if( params.APSME_BIND_request.DstAddrMode != APS_ADDRESS_GROUP )
                        {
                            if( APSFromLongToShort( &params.APSME_BIND_request.DstAddr.LongAddr ) == FALSE )
                            {
                                params.APSME_BIND_confirm.Status = BIND_ILLEGAL_DEVICE;
                                return APSME_BIND_confirm;
                            }
                            else
                            {
                                dstShortAddress = currentAPSAddress.shortAddr; //currentNeighborRecord.shortAddr;
                            }
                        }
                        else
                        {
                            dstShortAddress = params.APSME_BIND_request.DstAddr.ShortAddr;
                            params.APSME_BIND_request.DstEndpoint = 0xFE; // indicate it is group binding
                        }

                        params.APSME_BIND_confirm.Status = APSAddBindingInfo( srcShortAddress,
                            params.APSME_BIND_request.SrcEndpoint, params.APSME_BIND_request.ClusterId,
                            dstShortAddress, params.APSME_BIND_request.DstEndpoint );
                        return APSME_BIND_confirm;
                    }
                #endif
                break;

            case APSME_UNBIND_request:
                #ifndef I_SUPPORT_BINDINGS
                    // NOTE - This is a deviation from the spec.  The spec does not specify
                    // what to do with this primitive on an end device, only the Bind Request.
                    params.APSME_UNBIND_confirm.Status = BIND_NOT_SUPPORTED;
                    return APSME_UNBIND_confirm;
                #else
                    // NOTE - The spec allows bindings to be created even if we are not
                    // associated.  However, it doesn't allow us to unbind...
                    //#ifdef I_AM_COORDINATOR
                    //if (!ZigBeeStatus.flags.bits.bNetworkFormed)
                    //#else
                    //if (!ZigBeeStatus.flags.bits.bNetworkJoined)
                    //#endif //TODO check validity of the dead code that follows in {} below
                    if (((NOW_I_AM_A_CORDINATOR())&&!ZigBeeStatus.flags.bits.bNetworkFormed)
                    	||((NOW_I_AM_NOT_A_CORDINATOR())&&!ZigBeeStatus.flags.bits.bNetworkJoined) )
                    params.APSME_UNBIND_confirm.Status = BIND_ILLEGAL_REQUEST;
                    return APSME_UNBIND_confirm;

                    {
                        SHORT_ADDR  srcShortAddress;
                        SHORT_ADDR  dstShortAddress;

                        if( APSFromLongToShort( &params.APSME_UNBIND_request.SrcAddr ) == FALSE )
                        {
                            params.APSME_UNBIND_confirm.Status = BIND_ILLEGAL_DEVICE;
                            return APSME_UNBIND_confirm;
                        }
                        else
                        {
                            srcShortAddress = currentAPSAddress.shortAddr; //currentNeighborRecord.shortAddr;
                        }

                        if( params.APSME_UNBIND_request.DstAddrMode != APS_ADDRESS_GROUP )
                        {
                            if( APSFromLongToShort( &params.APSME_UNBIND_request.DstAddr.LongAddr ) == FALSE )
                            {
                                params.APSME_UNBIND_confirm.Status = BIND_ILLEGAL_DEVICE;
                                return APSME_UNBIND_confirm;
                            }
                            else
                            {
                                dstShortAddress = currentAPSAddress.shortAddr; //currentNeighborRecord.shortAddr;
                            }
                        }
                        else
                        {
                            dstShortAddress = params.APSME_UNBIND_request.DstAddr.ShortAddr;
                            params.APSME_UNBIND_request.DstEndpoint = 0xFE; // indicate group binding
                        }

                        params.APSME_UNBIND_confirm.Status = APSRemoveBindingInfo( srcShortAddress,
                            params.APSME_UNBIND_request.SrcEndpoint, params.APSME_UNBIND_request.ClusterId,
                            dstShortAddress, params.APSME_UNBIND_request.DstEndpoint );
                        return APSME_UNBIND_confirm;
                    }
                #endif
                break;

        #ifdef I_SUPPORT_SECURITY

            #ifndef I_AM_END_DEVICE
            case APSME_TRANSPORT_KEY_request:

//#ifndef I_AM_TRUST_CENTER
Send_Transport_Key:
//#endif
            {

                BOOL nwkKeySecure = TRUE;
                BYTE useParent;
                SHORT_ADDR parentShortAddress;
                BYTE NTIndex;
                LONG_ADDR DstAddress;
                BYTE APSSecurity;
                #if ( I_SUPPORT_LINK_KEY == 1 )
                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1 //defined I_AM_TRUST_CENTER
                    BOOL extendedNonce = FALSE;
                #endif
                #endif

                BYTE NWKSecurity = params.APSME_TRANSPORT_KEY_request._UseNwkSecurity;
                APSSecurity =  (params.APSME_TRANSPORT_KEY_request._UseAPSSecurity== FALSE)?(FALSE):TRUE;
                useParent = params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent;
                parentShortAddress.Val = params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val;

                if( INVALID_NEIGHBOR_KEY != (NTIndex = NWKLookupNodeByLongAddr(&(params.APSME_TRANSPORT_KEY_request.DestinationAddress))) )
                {
                    if( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD &&
                        !currentNeighborRecord.bSecured )
                    {
                        nwkKeySecure = FALSE;
                        currentNeighborRecord.bSecured = TRUE;
                        #ifdef USE_EXTERNAL_NVM
                            PutNeighborRecord( neighborTable + (WORD)NTIndex * (WORD)sizeof(NEIGHBOR_RECORD), &currentNeighborRecord );
                        #else
                            PutNeighborRecord( &(neighborTable[NTIndex]), &currentNeighborRecord );
                        #endif
                    }
                }

                #if PROFILE_nwkSecureAllFrames
                    #if I_SUPPORT_LINK_KEY == 1
                        if( APSSecurity == FALSE )
                        {
                            TxData++;   // reserve space for frame control
                            TxBuffer[TxData++] = APSCounter++;
                        }

                     #else
                        TxData++;   // reserve space for frame control
                        TxBuffer[TxData++] = APSCounter++;
                    #endif
                #endif
                TxBuffer[TxData++] = APS_CMD_TRANSPORT_KEY;
                TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.KeyType;
                for(i = 0; i < 16; i++)
                {
                    TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.Key->v[i];
                    if(I_AM_TRUST_CENTER){//#ifdef I_AM_TRUST_CENTER
                        currentNetworkKeyInfo.NetKey.v[i] = params.APSME_TRANSPORT_KEY_request.Key->v[i];
                    }//#endif
                }

                /* For ZigBee 2006:  Update the Key Sequence Number to match key bytes
                 * since this is the way ZCP tells explicitily what key we are using the
                 * correct keys when we have not only key0 and Key1 but Key2 Key3 etc...
                 * the key number must now kept where as before it was on 0 or 1
                */
                KEY_INFO keyDetails;
                BYTE ActiveKeyIndex;
             if(I_AM_TRUST_CENTER){//#ifdef I_AM_TRUST_CENTER
                #if (I_SUPPORT_LINK_KEY == 0x01)
                #endif
                currentNetworkKeyInfo.SeqNumber.v[0] =
                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber;
                currentNetworkKeyInfo.SeqNumber.v[1] = nwkMAGICResSeq;
                GetNwkActiveKeyNumber(&ActiveKeyIndex);
                if ( ((firstKeyHasBeenSent && currentNetworkKeyInfo.SeqNumber.v[0] != 0x00)) ||
                       previousKeyNotSeq0 )   /* Skip first */
                {
                    previousKeyNotSeq0 = TRUE;
                    if(ActiveKeyIndex == 0x01)
                    {
                        /* put it in the not currently active slot */
                        #ifdef USE_EXTERNAL_NVM
                            SetSecurityKey(1, currentNetworkKeyInfo);
                        #else
                            PutNwkKeyInfo( &networkKeyInfo[1] , &currentNetworkKeyInfo );
                        #endif
                    }
                    else
                    {
                        /* put it in the not currrently active slot */
                        #ifdef USE_EXTERNAL_NVM
                            SetSecurityKey(0, currentNetworkKeyInfo);
                        #else
                            PutNwkKeyInfo( &networkKeyInfo[0] , &currentNetworkKeyInfo );
                        #endif
                    }
               }
			}//#endif


                switch( params.APSME_TRANSPORT_KEY_request.KeyType )
                {
                    case 0x01:  // Network key
                    {
                        LONG_ADDR   SrcAddr;
                        TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber;

                        /* For Zigbee 2006:  If not broadcast then use proper longAddress */
                        if(params.APSME_TRANSPORT_KEY_request._DstAddrMode != 0xff  &&
                           params.APSME_TRANSPORT_KEY_request._DstAddrMode != 0xfd )
                        {
                            for(i = 0; i < 8; i++)
                            {
                                TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.DestinationAddress.v[i];
                            }
                        }
                        else        /* if broadcasting then destAddr must be 0x000... */
                        {
                            apsStatus.flags.bits.bBroadcastingNetworkKey = 1;
                            for(i = 0; i < 8; i++)
                            {
                                TxBuffer[TxData++] = 0x00;
                            }
                            /* Since this is a broadcast, disable APS security */
                            APSSecurity = FALSE;
                        }

                        /* Added at NTS - preserve the trust center's address as source */
                        GetMACAddress(&SrcAddr);
                        for(i = 0; i < 8; i++)
                        {
                            if(I_AM_TRUST_CENTER){//#ifdef I_AM_TRUST_CENTER
                                TxBuffer[TxData++] = SrcAddr.v[i];
                            }else{//#else
                                TxBuffer[TxData++] = transportKeySrcAddr.v[i];
                            }//#endif
                        }
                        break;
                    }
                    #if ( I_SUPPORT_LINK_KEY == 0x01 )
                    //#ifdef I_AM_TRUST_CENTER
                        case TC_LINK_KEY:
                        if(I_AM_TRUST_CENTER){
                            LONG_ADDR   SrcAddr;
                            useParent = FALSE;
                             for(i = 0; i < MAX_ADDR_LENGTH; i++)
                            {
                                TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.DestinationAddress.v[i];
                            }
                            GetMACAddress(&SrcAddr);
                            for(i = 0; i < MAX_ADDR_LENGTH; i++)
                            {
                                TxBuffer[TxData++] = SrcAddr.v[i];
                            }
                            break;
                        }
                    //#endif // I_AM_TRUST_CENTER
                    #endif //( I_SUPPORT_LINK_KEY == 0x01 )
                    #if ( I_SUPPORT_LINK_KEY == 0x01 )
                        case APP_LINK_KEY:
                        case APP_MASTER_KEY:
                        {
                             useParent = FALSE;
                            for(i = 0; i < MAX_ADDR_LENGTH; i++)
                            {
                                TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.TransportKeyData.APPLICATION_LINKKEY.PartnerAddress.v[i];
                            }
                            TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.TransportKeyData.APPLICATION_LINKKEY.Initiator;
                            break;
                        }
                    #endif //( I_SUPPORT_LINK_KEY == 0x01 )
                }
                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                    if( useParent )
                    {
                        for(i = 0; i < MAX_ADDR_LENGTH; i++)
                        {
                            DstAddress.v[i] = params.APSME_TRANSPORT_KEY_request.ParentAddress.v[i];
                        }
                    }
                    else
                #endif    /* I_SUPPORT_PRECONFIGURED_TC_LINK_KEY */
                    {
                        for(i = 0; i < MAX_ADDR_LENGTH; i++)
                        {
                            DstAddress.v[i] = params.APSME_TRANSPORT_KEY_request.DestinationAddress.v[i];
                        }
                        #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                            if(useParent)
                            {
                                extendedNonce = TRUE;
                            }
                        #endif
                    }

                if( !APSFillSecurityRequest(&DstAddress, APSSecurity,
                params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val))
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #if I_SUPPORT_LINK_KEY == 1
                    if(I_AM_TRUST_CENTER){//#ifdef I_AM_TRUST_CENTER
                        if( APSSecurity == 1 )
                        {
                            KEY_VAL secretkey;
                            KEY_VAL linkKey;
                            BYTE inputToHMAC;
                            BYTE count;

                            #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                                LONG_ADDR   SrcAddr;
                                LONG_ADDR   destAddrReverse;
                            #endif

                            #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                //#if /*USE_COMMON_TC_LINK_KEY == 0 ||*/ I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                                    TC_LINK_KEY_TABLE   tcLinkKeyTable;
                               // #endif
                            #endif

                            if ( params.APSME_TRANSPORT_KEY_request.KeyType == 0x01 ) // Network Key
                            {
                                keyDetails.keyId = ID_KeyTransportKey;
                                inputToHMAC = 0;

                                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                    if (USE_COMMON_TC_LINK_KEY == 1){//#if USE_COMMON_TC_LINK_KEY == 1
                                        for(count = 0; count < KEY_LENGTH; count++)
                                        {
                                            linkKey.v[count] = TCLinkKeyInfo.link_key.v[count];
                                        }
                                    }else{//#else
                                        if ( SearchForTCLinkKey(DstAddress, &tcLinkKeyTable) )
                                        {
                                            for (count = 0; count < KEY_LENGTH; count++)
                                            {
                                                linkKey.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                            }
                                        }
                                        else
                                        {
                                            ZigBeeUnblockTx();
                                            return NO_PRIMITIVE;
                                        }
                                    }//#endif
                                #else
                                    GetMACAddress(&SrcAddr);
                                    if(useParent)
                                    {
                                        for(count = 0; count < 8; count++)
                                        {
                                            destAddrReverse.v[count] = DstAddress.v[7-count];
                                        }
                                        GetHashKey(&destAddrReverse, &SrcAddr, &linkKey);
                                    }
                                    else
                                    {
                                        GetHashKey(&DstAddress, &SrcAddr, &linkKey);
                                    }
                                #endif
                            }
                            else
                            {
                                keyDetails.keyId = ID_KeyLoadKey;
                                inputToHMAC = 2;

                                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                    if (USE_COMMON_TC_LINK_KEY == 1){ //#if USE_COMMON_TC_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 0 // TODOmultiple link key =0 for MSDL project
                                        for(count = 0; count < KEY_LENGTH; count++)
                                        {
                                            linkKey.v[count] = TCLinkKeyInfo.link_key.v[count];
                                        }
                                    }else{//#else
                                        if ( SearchForTCLinkKey(DstAddress, &tcLinkKeyTable) )
                                        {
                                            for (count = 0; count < KEY_LENGTH; count++)
                                            {
                                                linkKey.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                            }
                                        }
                                        else
                                        {
                                            ZigBeeUnblockTx();
                                            return NO_PRIMITIVE;
                                        }
                                    }//#endif
                                #else
                                    GetMACAddress(&SrcAddr);
                                    if(useParent)
                                    {
                                        for(count = 0; count < 8; count++)
                                        {
                                            destAddrReverse.v[count] = DstAddress.v[7-count];
                                        }
                                        GetHashKey(&destAddrReverse, &SrcAddr, &linkKey);
                                    }
                                    else
                                    {
                                        GetHashKey(&DstAddress, &SrcAddr, &linkKey);
                                    }
                                #endif
                            }

                            keyDetails.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val;
                            TCLinkKeyInfo.frameCounter.Val++;
                            apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
                            ApplyHMACAlgorithm(&linkKey, &inputToHMAC, &secretkey, 1);
                            keyDetails.key = (BYTE *)&secretkey;

                             if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                             (TX_HEADER_START-TxHeader), &keyDetails, extendedNonce) )
                             {
                                 ZigBeeUnblockTx();
                                 return NO_PRIMITIVE;
                             }
                             else
                             {
                                FormatTxBuffer();
                             }
                        }
                    }//#endif
                #endif
                if(I_AM_TRUST_CENTER){//#ifdef I_AM_TRUST_CENTER
                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                    if(useParent)
                    {
                        TunnelTransportKey( &DstAddress, parentShortAddress);
                    }
                    #endif //I_SUPPORT_PRECONFIGURED_TC_LINK_KEY
                }//#endif // I_AM_TRUST_CENTER
                #ifdef I_SUPPORT_RES_SECURITY
                    #ifdef PROFILE_nwkSecureAllFrames

                    #else
                        if( nwkKeySecure )
                        {
                             if( GetActiveNetworkKey(&keyDetails));
                            if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                            (TX_HEADER_START-TxHeader), &keyDetails, FALSE) )
                            {
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }
                        }
                    #endif
                #endif

                #ifdef I_SUPPORT_SECURITY_SPEC
                    params.NLDE_DATA_request.SecurityEnable =
                    (securityStatus.flags.bits.nwkSecureAllFrames && nwkKeySecure);
                #else
                    if( NWKSecurity == TRUE )
                    {

                        params.NLDE_DATA_request.SecurityEnable = TRUE;
                    }
                    else
                    {
                        params.NLDE_DATA_request.SecurityEnable = FALSE;
                    }
                #endif

                /* from now on update keySeguence number in ROM to match with switching
                 * keys that are sent
                */
                firstKeyHasBeenSent = TRUE;

                return NLDE_DATA_request;

            }
            #endif  // I_AM_END_DEVICE

            //#ifndef I_AM_TRUST_CENTER
            #ifndef I_AM_END_DEVICE
            case APSME_UPDATE_DEVICE_request:
            if(!I_AM_TRUST_CENTER){
                BOOL APSSecure = TRUE;
                #if I_SUPPORT_LINK_KEY == 1
                    KEY_INFO    linkKey;
                    BYTE        count;
                    KEY_VAL     linkKeyToBeUsed;
                #endif
                #ifndef PROFILE_nwkSecureAllFrames
                    KEY_INFO ActiveNetworkKey;
                #endif
                #if I_SUPPORT_LINK_KEY == 0
                    #if PROFILE_nwkSecureAllFrames
                        TxData++;
                        TxBuffer[TxData++] = APSCounter++;
                        APSSecure = FALSE;
                    #endif
                #else
                    APSSecure = TRUE;
                #endif
                TxBuffer[TxData++] = APS_CMD_UPDATE_DEVICE;
                for(i = 0; i < 8; i++)
                {
                    TxBuffer[TxData++] = params.APSME_UPDATE_DEVICE_request.DeviceAddress.v[i];
                    /* For ZigBee 2006 added at NTS - keep track of my child's key progress */
                    if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
                        updateDevAPSAddr.longAddr.v[i] = params.APSME_UPDATE_DEVICE_request.DeviceAddress.v[i];
                    }//#endif
                }
                TxBuffer[TxData++] = params.APSME_UPDATE_DEVICE_request.DeviceShortAddress.byte.LSB;
                TxBuffer[TxData++] = params.APSME_UPDATE_DEVICE_request.DeviceShortAddress.byte.MSB;

                /* Added at NTS - keep track of child that is been authenticated */
                if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
                    updateDevAPSAddr.shortAddr  = params.APSME_UPDATE_DEVICE_request.DeviceShortAddress;
                }//#endif

                TxBuffer[TxData++] = params.APSME_UPDATE_DEVICE_request.Status;

                if( !APSFillSecurityRequest(&(params.APSME_UPDATE_DEVICE_request.DestAddress), APSSecure,0x0000) )
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #if I_SUPPORT_LINK_KEY == 1
                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                            TC_LINK_KEY_TABLE   tcLinkKeyTable;
                            LONG_ADDR           dstLongAddrReverse;

                            for(count = 0; count < 8; count++)
                            {
                                dstLongAddrReverse.v[count] = TCLinkKeyInfo.trustCenterLongAddr.v[count];
                            }
                            if ( SearchForTCLinkKey(dstLongAddrReverse, &tcLinkKeyTable) )
                            {
                                for (count = 0; count < KEY_LENGTH; count++)
                                {
                                    linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                }
                            }
                            else
                            {
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }
                        #else
                            for(count = 0; count < KEY_LENGTH; count++)
                            {
                                linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                            }
                        #endif
                    #else
                        for(count = 0; count < KEY_LENGTH; count++)
                        {
                            linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                        }
                    #endif

                    linkKey.keyId = ID_LinkKey;
                    linkKey.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val;
                    TCLinkKeyInfo.frameCounter.Val++;
                    linkKey.key = &linkKeyToBeUsed.v[0];
                    if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                            (TX_HEADER_START-TxHeader), &linkKey, FALSE) )
                    {
                        ZigBeeUnblockTx();
                        return NO_PRIMITIVE;
                    }
                    else
                    {
                        FormatTxBuffer();
                    }
                #endif /* I_SUPPORT_LINK_KEY */
                #ifdef I_SUPPORT_RES_SECURITY
                    #if PROFILE_nwkSecureAllFrames
                    #else
                        if( GetActiveNetworkKey(&ActiveNetworkKey))
                        {
                            if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                            (TX_HEADER_START-TxHeader), &ActiveNetworkKey, FALSE) )
                            {
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }
                        }
                    #endif
                #endif
                params.NLDE_DATA_request.SecurityEnable = securityStatus.flags.bits.nwkSecureAllFrames;

                params.NLDE_DATA_request.DstAddr.Val = 0x0000;
                params.NLDE_DATA_request.DstAddrMode = 0x02;

                return NLDE_DATA_request;
            }
            break;
        #endif //#ifndef I_AM_END_DEVICE
        //#endif  /* I_AM_TRUST_CENTER */
        //#ifdef I_AM_TRUST_CENTER
            case APSME_REMOVE_DEVICE_request:
            if(I_AM_TRUST_CENTER){
                BYTE APSSecurity = FALSE;
                #if I_SUPPORT_LINK_KEY == 1
                    KEY_VAL KeyForEncryption;
                    KEY_INFO linkKey;
                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                        LONG_ADDR SrcAddr;
                    #endif
                #endif
                #ifndef PROFILE_nwkSecureAllFrames
                    KEY_INFO ActiveNetworkKey;
                #endif
                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                        LONG_ADDR parentAddress;
                #endif
                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                    #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                        TC_LINK_KEY_TABLE   tcLinkKeyTable;
                        BYTE                count;
                        LONG_ADDR           parentAddress;
                    #endif
                #endif

                #if PROFILE_nwkSecureAllFrames
                    #if I_SUPPORT_LINK_KEY == 0
                        TxData++;
                        TxBuffer[TxData++] = APSCounter++;
                    #else
                        APSSecurity = TRUE;
                    #endif
                #endif


                TxBuffer[TxData++] = APS_CMD_REMOVE_DEVICE;
                for(i = 0; i < 8; i++)
                {
                    TxBuffer[TxData++] = params.APSME_REMOVE_DEVICE_request.ChildAddress.v[i];
                }
                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                    for(i = 0; i < 8; i++)
                    {
                        parentAddress.v[i] = params.APSME_REMOVE_DEVICE_request.ParentAddress.v[i];
                    }
                #endif

                #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                    #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                        for(count = 0; count < 8; count++)
                        {
                            parentAddress.v[count] = params.APSME_REMOVE_DEVICE_request.ParentAddress.v[count];
                        }
                    #endif
                #endif

                if( !APSFillSecurityRequest(&(params.APSME_REMOVE_DEVICE_request.ParentAddress),
                APSSecurity,0xFFFF) )
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #if I_SUPPORT_LINK_KEY == 1
                    linkKey.keyId = ID_LinkKey;
                    linkKey.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val;
                    TCLinkKeyInfo.frameCounter.Val++;
                    apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                            if ( SearchForTCLinkKey(parentAddress, &tcLinkKeyTable) )
                            {
                                for (count = 0; count < KEY_LENGTH; count++)
                                {
                                    KeyForEncryption.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                }
                            }
                            else
                            {
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }
                        #else
                            for(i = 0; i < KEY_LENGTH; i++)
                            {
                                KeyForEncryption.v[i] = TCLinkKeyInfo.link_key.v[i];
                            }
                        #endif
                    #else
                        GetMACAddress(&SrcAddr);
                        GetHashKey(&parentAddress, &SrcAddr, &KeyForEncryption);
                    #endif
                    linkKey.key = &KeyForEncryption.v[0];
                    if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                            (TX_HEADER_START-TxHeader), &linkKey, FALSE) )
                    {
                        ZigBeeUnblockTx();
                        return NO_PRIMITIVE;
                    }
                    else
                    {
                        FormatTxBuffer();
                    }
                #endif /* I_SUPPORT_LINK_KEY */
                #ifdef I_SUPPORT_RES_SECURITY
                    #if PROFILE_nwkSecureAllFrames

                    #else
                        if( GetActiveNetworkKey(&ActiveNetworkKey))
                        {
                            if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                            (TX_HEADER_START-TxHeader), &ActiveNetworkKey, FALSE) )
                            {
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }
                        }
                    #endif
                #endif
                params.NLDE_DATA_request.SecurityEnable = securityStatus.flags.bits.nwkSecureAllFrames;

                return NLDE_DATA_request;
            }

        //#endif
            //#if !defined(I_AM_TRUST_CENTER)
            case APSME_REQUEST_KEY_request:
            if(!I_AM_TRUST_CENTER){
                #if I_SUPPORT_LINK_KEY == 1
                    KEY_INFO    linkKey;
                    BYTE        count;
                    KEY_VAL     linkKeyToBeUsed;
                #endif
                BOOL APSSecurity = FALSE;
                BYTE KeyType;
                KeyType = params.APSME_REQUEST_KEY_request.KeyType;
                if( APP_MASTER_KEY == KeyType )
                {
                    APSSecurity = TRUE;
                }
                #if PROFILE_nwkSecureAllFrames
                    if(APSSecurity == FALSE )
                    {
                        TxData++;
                        TxBuffer[TxData++] = APSCounter++;
                    }
                #endif


                TxBuffer[TxData++] = APS_CMD_REQUEST_KEY;
                TxBuffer[TxData++] = KeyType;

                if( APP_MASTER_KEY == KeyType )
                {
                    for(i = 0; i < 8; i++)
                    {
                        TxBuffer[TxData++] = params.APSME_REQUEST_KEY_request.PartnerAddress.v[i];
                    }
                }
                if( !APSFillSecurityRequest(&(params.APSME_REQUEST_KEY_request.DestAddress),
                APSSecurity,0x0000 ))
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #if I_SUPPORT_LINK_KEY ==1
                    if( APSSecurity == 1 )
                    {
                        #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                            #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                                TC_LINK_KEY_TABLE   tcLinkKeyTable;
                                LONG_ADDR           dstLongAddrReverse;

                                for(count = 0; count < 8; count++)
                                {
                                    dstLongAddrReverse.v[count] = TCLinkKeyInfo.trustCenterLongAddr.v[count];
                                }
                                if ( SearchForTCLinkKey(dstLongAddrReverse, &tcLinkKeyTable) )
                                {
                                    for (count = 0; count < KEY_LENGTH; count++)
                                    {
                                        linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                    }
                                }
                                else
                                {
                                    ZigBeeUnblockTx();
                                    return NO_PRIMITIVE;
                                }
                            #else
                                for(count = 0; count < KEY_LENGTH; count++)
                                {
                                    linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                                }
                            #endif
                        #else
                            for(count = 0; count < KEY_LENGTH; count++)
                            {
                                linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                            }
                        #endif

                        linkKey.keyId = ID_LinkKey;
                        linkKey.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val;
                        TCLinkKeyInfo.frameCounter.Val++;
                        apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
                        linkKey.key = &linkKeyToBeUsed.v[0];

                        if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                            (TX_HEADER_START-TxHeader), &linkKey, FALSE) )
                        {
                            ZigBeeUnblockTx();
                            return NO_PRIMITIVE;
                        }
                        else
                        {
                            FormatTxBuffer();
                        }
                    }
                #endif //I_SUPPORT_LINK_KEY
                #ifdef I_SUPPORT_RES_SECURITY
                    #if PROFILE_nwkSecureAllFrames

                    #else
                        if( GetActiveNetworkKey(&ActiveNetworkKey))
                        {
                            if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                            (TX_HEADER_START-TxHeader), &ActiveNetworkKey, FALSE) )
                            {
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }
                        }
                    #endif
                #endif
                params.NLDE_DATA_request.SecurityEnable =
                securityStatus.flags.bits.nwkSecureAllFrames;
                ZigBeeBlockTx();
                return NLDE_DATA_request;
            }
            //#endif

            //#if defined(I_AM_TRUST_CENTER)
            case APSME_SWITCH_KEY_request:
            if(I_AM_TRUST_CENTER){

                #ifndef PROFILE_nwkSecureAllFrames
                    KEY_INFO ActiveNetworkKey;
                #endif
                zdoStatus.KeySeq = params.APSME_SWITCH_KEY_request.KeySeqNumber;
                zdoStatus.SwitchKeyTick = TickGet();
                zdoStatus.flags.bits.bSwitchKey = 1;

                #if PROFILE_nwkSecureAllFrames
                    TxData++;
                #endif
                /* Zigbee 2006 needs this APSCounter */
                TxBuffer[TxData++] = APSCounter++;

                TxBuffer[TxData++] = APS_CMD_SWITCH_KEY;
                TxBuffer[TxData++] = params.APSME_SWITCH_KEY_request.KeySeqNumber;

                apsStatus.flags.bits.bBroadcastingSwitchKey = 1;

                if( !APSFillSecurityRequest(&(params.APSME_SWITCH_KEY_request.DestAddress),
                !securityStatus.flags.bits.nwkSecureAllFrames, 0xFFFF) )
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #ifdef I_SUPPORT_RES_SECURITY
                    #if PROFILE_nwkSecureAllFrames

                    #else
                        if( GetActiveNetworkKey(&ActiveNetworkKey))
                        {
                            if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]),
                            (TX_HEADER_START-TxHeader), &ActiveNetworkKey, FALSE) )
                            {
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }
                        }
                    #endif
                #endif
                params.NLDE_DATA_request.SecurityEnable = securityStatus.flags.bits.nwkSecureAllFrames;

                return NLDE_DATA_request;
            }

            //#endif //I_AM_TRUST_CENTER

        #endif   // I_SUPPORT_SECURITY

            default:
                break;
        }
    }

    return NO_PRIMITIVE;
}

/*********************************************************************
 * Function:       BOOL    APSFillSecurityRequest(INPUT LONG_ADDR *DestAddr, INPUT BOOL bSecuredFrame,
 *                 WORD NwkDstAddress)
 *
 * PreCondition:    None
 *
 * Input:           DestAddr - pointer to destination long address
 *                  bSecuredFrame - boolean to specify if secure the frame
 *                  NwkDstAddress - The destination short address. If unknown set it as 0xFFFF.
 *                  If NwkDstAddress is 0xFFFF, then this function gets the short address
 *                  by searching through address map and neighbor tables.
 *
 * Output:          TRUE - information successfully filled
 *                  FALSE - failed to fill the information to send data
 *
 * Side Effects:    None
 *
 * Overview:        Fill the parameters for NLDE_DATA_request for Security APS commands
 *
 * Note:            None
 ********************************************************************/
#ifdef I_SUPPORT_SECURITY
BOOL    APSFillSecurityRequest(INPUT LONG_ADDR *DestAddr, INPUT BOOL bSecuredFrame,
WORD NwkDstAddress)
{
    APS_FRAME_CONTROL   apsFrameControl;
    LONG_ADDR           broadcastAddrFF;
    BYTE i;

    if( !ZigBeeReady() )
    {
        return FALSE;
    }

    ZigBeeBlockTx();

    /* Zigbee 2006: Check if  long address is 0x0000... See what we are broadcasting
     *  it will be either transport_key_requests or switch_key_requests.
     */
    for(i = 0; i < 8; i++)
    {
            currentAPSAddress.longAddr.v[i] = 0x00;
            broadcastAddrFF.v[i] = 0xFF;
    }

    if( ( ( !memcmp( (void *)&currentAPSAddress.longAddr, (void *)&(DestAddr->v[0]), 8 ) ) ||
          ( !memcmp( (void *)&broadcastAddrFF.v[0], (void *)&(DestAddr->v[0]), 8 ) ) ) &&
        apsStatus.flags.bits.bBroadcastingNetworkKey == 1 )
    {
        if(params.APSME_TRANSPORT_KEY_request._DstAddrMode == 0xfd)
            currentAPSAddress.shortAddr.Val = 0xfffd;
        else
            currentAPSAddress.shortAddr.Val = 0xffff;

        params.NLDE_DATA_request.DstAddr.Val = currentAPSAddress.shortAddr.Val;
        apsStatus.flags.bits.bBroadcastingNetworkKey  = 0;
        goto finish_req;
    }
    else if( (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&(DestAddr->v[0]), 8 )) &&
    apsStatus.flags.bits.bBroadcastingSwitchKey == 1 )
    {
         currentAPSAddress.shortAddr.Val = 0xfffd;
         apsStatus.flags.bits.bBroadcastingSwitchKey  = 0;
         params.NLDE_DATA_request.DstAddr.Val = currentAPSAddress.shortAddr.Val;
         goto finish_req;
    }
    if(NwkDstAddress == 0xFFFF )
    {
        SHORT_ADDR shortAddr;
        //if( !APSFromLongToShort(DestAddr) )
        if ( !IsThisLongAddressKnown (DestAddr, &shortAddr.v[0], 0x01 )) // 0x01 means search in Neighbor and AddressMap table
        {
            /* This will send an incorrect msg to Coordinator to at least alert operator
             */
           return FALSE;
        }
        params.NLDE_DATA_request.DstAddr.Val = shortAddr.Val;
    }
    else
    {
        params.NLDE_DATA_request.DstAddr.Val = NwkDstAddress;
    }

    /* Zigbee 2006: if this was to a broadcast long address then make the short address 0xffff */
finish_req:

    params.NLDE_DATA_request.NsduHandle = NLME_GET_nwkBCSN();
    params.NLDE_DATA_request.BroadcastRadius    = DEFAULT_RADIUS;
    params.NLDE_DATA_request.DiscoverRoute      = ROUTE_DISCOVERY_ENABLE;
    params.NLDE_DATA_request.DstAddrMode = 0x02; //short address mode
    apsFrameControl.Val = APS_FRAME_COMMAND;
    apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT;
    apsFrameControl.bits.acknowledgeRequest = 0;
    if( bSecuredFrame )
    {
        apsFrameControl.bits.security = 1;
    }
    else
    {
        apsFrameControl.bits.security = 0;
    }

    #if PROFILE_nwkSecureAllFrames
        #if I_SUPPORT_LINK_KEY == 1
        if( bSecuredFrame )
        {
            TxBuffer[TxHeader--] = APSCounter++;
            TxBuffer[TxHeader--] = apsFrameControl.Val;

        }
        else
        {
            TxBuffer[TX_DATA_START] = apsFrameControl.Val;
        }

        #else
            TxBuffer[TX_DATA_START] = apsFrameControl.Val;
        #endif
    #else
        TxBuffer[TxHeader--] = APSCounter++;
        TxBuffer[TxHeader--] = apsFrameControl.Val;


    #endif

    return TRUE;
}
#endif



/*********************************************************************
 * Function:        BYTE APSAddBindingInfo(SHORT_ADDR srcAddr,
 *                                      BYTE srcEP,
 *                                      WORD_VAL clusterID,
 *                                      SHORT_ADDR destAddr,
 *                                      BYTE destEP)
 *
 *
 * PreCondition:    srcAddr and destAddr must be valid addresses of
 *                  devices on the network
 *
 * Input:           srcAddr     - source short address
 *                  srcEP       - source end point
 *                  clusterID   - cluster id
 *                  destAddr    - destination short address
 *                  destEP      - destination end point
 *
 * Output:          SUCCESS if an entry was created or already exists
 *                  TABLE_FULL if the binding table is full
 *                  FALSE if illegal binding attempted
 *
 * Side Effects:    None
 *
 * Overview:        Creates/updates a binding entry for given
 *                  set of data.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
BYTE APSAddBindingInfo( SHORT_ADDR srcAddr, BYTE srcEP, WORD_VAL clusterID,
                     SHORT_ADDR destAddr, BYTE destEP )
{
    BYTE            bindingDestIndex    = 0;
    BYTE            bindingSrcIndex     = 0;
    BYTE            bindingMapSourceByte;
    BYTE            bindingMapUsageByte;
    BOOL            Found           = FALSE;
    BYTE            oldBindingLink;
    BINDING_RECORD  tempBindingRecord;

    // See if a list for the source data already exists.
    while ((bindingSrcIndex < MAX_BINDINGS) && !Found)
    {
        GetBindingSourceMap( &bindingMapSourceByte, bindingSrcIndex );
        GetBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );

        if (BindingIsUsed( bindingMapUsageByte,  bindingSrcIndex ) &&
            BindingIsUsed( bindingMapSourceByte, bindingSrcIndex ))
        {
            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(bindingSrcIndex) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[bindingSrcIndex];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );

            if ((currentBindingRecord.shortAddr.Val == srcAddr.Val) &&
                (currentBindingRecord.endPoint == srcEP) &&
                (currentBindingRecord.clusterID.Val == clusterID.Val))
            {
                Found = TRUE;
                break;
            }
        }
        bindingSrcIndex ++;
    }

    // If there was no source data list, create one.
    if (!Found)
    {
        bindingSrcIndex = 0;
        GetBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );
        while ((bindingSrcIndex < MAX_BINDINGS) &&
               BindingIsUsed( bindingMapUsageByte,  bindingSrcIndex ))
        {
            bindingSrcIndex ++;
            GetBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );
        }
        if (bindingSrcIndex == MAX_BINDINGS)
        {
            return BIND_TABLE_FULL;
        }

        #ifdef USE_EXTERNAL_NVM
            pCurrentBindingRecord = apsBindingTable + (WORD)(bindingSrcIndex) * (WORD)sizeof(BINDING_RECORD);
        #else
            pCurrentBindingRecord = &apsBindingTable[bindingSrcIndex];
        #endif
        currentBindingRecord.shortAddr = srcAddr;
        currentBindingRecord.endPoint = srcEP;
        currentBindingRecord.clusterID.Val = clusterID.Val;
        currentBindingRecord.nextBindingRecord = END_BINDING_RECORDS;
    }
    // If we found the source data, make sure the destination link
    // doesn't already exist.  Leave currentBindingRecord as the source node.
    else
    {
        tempBindingRecord.nextBindingRecord = currentBindingRecord.nextBindingRecord;
        while (tempBindingRecord.nextBindingRecord != END_BINDING_RECORDS)
        {
            #ifdef USE_EXTERNAL_NVM
                GetBindingRecord(&tempBindingRecord, apsBindingTable + (WORD)tempBindingRecord.nextBindingRecord * (WORD)sizeof(BINDING_RECORD) );
            #else
                GetBindingRecord(&tempBindingRecord, &apsBindingTable[tempBindingRecord.nextBindingRecord] );
            #endif
            if ((tempBindingRecord.shortAddr.Val == destAddr.Val) &&
                 (tempBindingRecord.endPoint == destEP))
            {
                return SUCCESS;  // already exists
            }
        }
    }

    // Make sure there is room for a new destination node. Make sure we avoid the
    // node that we're trying to use for the source in case it's a new one!
    bindingDestIndex = 0;
    GetBindingUsageMap( &bindingMapUsageByte, bindingDestIndex );
    while (((bindingDestIndex < MAX_BINDINGS) &&
            BindingIsUsed( bindingMapUsageByte,  bindingDestIndex )) ||
           (bindingDestIndex == bindingSrcIndex))
    {
        bindingDestIndex ++;
        GetBindingUsageMap( &bindingMapUsageByte, bindingDestIndex );
    }
    if (bindingDestIndex == MAX_BINDINGS)
    {
        return BIND_TABLE_FULL;
    }

    // Update the source node to point to the new destination node.
    oldBindingLink = currentBindingRecord.nextBindingRecord;
    currentBindingRecord.nextBindingRecord = bindingDestIndex;
    PutBindingRecord( pCurrentBindingRecord, &currentBindingRecord );

    // Create the new binding record, inserting it at the head of the destinations.
    currentBindingRecord.shortAddr = destAddr;
    currentBindingRecord.endPoint = destEP;

    currentBindingRecord.nextBindingRecord = oldBindingLink;
    #ifdef USE_EXTERNAL_NVM
        pCurrentBindingRecord = apsBindingTable + (WORD)(bindingDestIndex) * (WORD)sizeof(BINDING_RECORD);
    #else
        pCurrentBindingRecord = &apsBindingTable[bindingDestIndex];
    #endif
    PutBindingRecord( pCurrentBindingRecord, &currentBindingRecord );

    // Mark the source node as used.  Is redundant if it already existed, but if there
    // was room for the source but not the destination, we don't want to take the
    // source node.
    GetBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );
    MarkBindingUsed( bindingMapUsageByte, bindingSrcIndex );
    PutBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );
    GetBindingSourceMap( &bindingMapSourceByte, bindingSrcIndex );
    MarkBindingUsed( bindingMapSourceByte, bindingSrcIndex );
    PutBindingSourceMap( &bindingMapSourceByte, bindingSrcIndex );

    // Mark the destination node as used, but not a source node
    GetBindingUsageMap( &bindingMapUsageByte, bindingDestIndex );
    MarkBindingUsed( bindingMapUsageByte, bindingDestIndex );
    PutBindingUsageMap( &bindingMapUsageByte, bindingDestIndex );
    GetBindingSourceMap( &bindingMapSourceByte, bindingDestIndex );
    MarkBindingUnused( bindingMapSourceByte, bindingDestIndex );
    PutBindingSourceMap( &bindingMapSourceByte, bindingDestIndex );

    return SUCCESS;
}
#endif


/*********************************************************************
 * Function:        void APSClearAPSAddressTable( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    currentAPSAddress is destroyed
 *
 * Overview:        This function sets the entire APS address map table
 *                  to all 0xFF.
 *
 * Note:            None
 ********************************************************************/
#if MAX_APS_ADDRESSES > 0
void APSClearAPSAddressTable( void )
{
    BYTE    i;
    WORD    ValidKey;

    ValidKey = apsMAGICValid;
    PutAPSAddressValidityKey(&ValidKey);

    for(i = 0; i < 8; i++)
    {
        currentAPSAddress.longAddr.v[i] = 0xFF;
    }
    currentAPSAddress.shortAddr.v[0] = 0xFF;
    currentAPSAddress.shortAddr.v[1] = 0xFF;

    for (i=0; i<MAX_APS_ADDRESSES; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress );
        #else
            PutAPSAddress( &apsAddressMap[i], &currentAPSAddress );
        #endif
    }
}
#endif

/*********************************************************************
 * Function:        BYTE APSGet( void )
 *
 * PreCondition:    Must be called from the NLDE_DATA_indication
 *                  primitive.
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

BYTE APSGet( void )
{
    if (params.NLDE_DATA_indication.NsduLength == 0)
    {
        return 0;
    }
    else
    {
        params.NLDE_DATA_indication.NsduLength--;
        return *params.NLDE_DATA_indication.Nsdu++;
    }
}

/*********************************************************************
 * Function:        BYTE APSRemoveBindingInfo(SHORT_ADDR srcAddr,
 *                                      BYTE srcEP,
 *                                      WORD_VAL clusterID,
 *                                      SHORT_ADDR destAddr,
 *                                      BYTE destEP)
 *
 *
 * PreCondition:    srcAddr and destAddr must be valid addresses of
 *                  devices on the network
 *
 * Input:           srcAddr     - source short address
 *                  srcEP       - source end point
 *                  clusterID   - cluster id
 *                  destAddr    - destination short address
 *                  destEP      - destination EP
 *
 * Output:          SUCCESS if the entry was removed
 *                  INVALID_BINDING if the binding did not exist
 *
 * Side Effects:    None
 *
 * Overview:        Removes a binding entry for given set of data.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
BYTE APSRemoveBindingInfo( SHORT_ADDR srcAddr, BYTE srcEP, WORD_VAL clusterID,
                    SHORT_ADDR destAddr, BYTE destEP )
{
    BYTE    bindingMapUsageByte;
    BYTE    currentKey;
    BYTE    nextKey;
    BYTE    previousKey;
    BYTE    sourceKey;

    if ((sourceKey = LookupSourceBindingInfo( srcAddr, srcEP, clusterID)) ==
            END_BINDING_RECORDS)
    {
        return BIND_INVALID_BINDING;
    }

    previousKey = sourceKey;
    #ifdef USE_EXTERNAL_NVM
        pCurrentBindingRecord = apsBindingTable + (WORD)(previousKey) * (WORD)sizeof(BINDING_RECORD);
    #else
        pCurrentBindingRecord = &apsBindingTable[previousKey];
    #endif
    GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
    while (currentBindingRecord.nextBindingRecord != END_BINDING_RECORDS)
    {
        currentKey = currentBindingRecord.nextBindingRecord;
        #ifdef USE_EXTERNAL_NVM
            pCurrentBindingRecord = apsBindingTable + (WORD)(currentKey) * (WORD)sizeof(BINDING_RECORD);
        #else
            pCurrentBindingRecord = &apsBindingTable[currentKey];
        #endif
        GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
        if ((currentBindingRecord.shortAddr.Val == destAddr.Val) &&
                (currentBindingRecord.endPoint == destEP))
        {
            nextKey = currentBindingRecord.nextBindingRecord;

            // Go back and get the previous record, and point it to the record
            // that the deleted record was pointing to.
            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(previousKey) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[previousKey];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
            currentBindingRecord.nextBindingRecord = nextKey;
            PutBindingRecord( pCurrentBindingRecord, &currentBindingRecord );

            // Update the usage map to delete the current node.
            GetBindingUsageMap( &bindingMapUsageByte, currentKey );
            MarkBindingUnused( bindingMapUsageByte, currentKey );
            PutBindingUsageMap( &bindingMapUsageByte, currentKey );

            // If we just deleted the only destination, delete the source as well.
            if ((sourceKey == previousKey) && (nextKey == END_BINDING_RECORDS))
            {
                GetBindingUsageMap( &bindingMapUsageByte, sourceKey );
                MarkBindingUnused( bindingMapUsageByte, sourceKey );
                PutBindingUsageMap( &bindingMapUsageByte, sourceKey );
            }

            return SUCCESS;
         }
         else
         {
             previousKey = currentKey;
         }
     }
     return BIND_INVALID_BINDING;
}
#endif

/*********************************************************************
 * Function:        BYTE LookupAPSAddress( LONG_ADDR *longAddr )
 *
 * PreCondition:    None
 *
 * Input:           longAddr - pointer to long address to match
 *
 * Output:          TRUE - a corresponding short address was found and
 *                      is held in currentAPSAddress.shortAddr
 *                  FALSE - a corresponding short address was not found
 *
 * Side Effects:    currentAPSAddress is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the APS address map for the short address
 *                  of a given long address.
 *
 * Note:            The end application is responsible for populating
 *                  this table.
 ********************************************************************/
#if MAX_APS_ADDRESSES > 0

BOOL LookupAPSAddress( LONG_ADDR *longAddr )
{
    BYTE    i;

    for (i=0; i<apscMaxAddrMapEntries; i++)
    {

        #ifdef USE_EXTERNAL_NVM
            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
        #else
            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
        #endif

        if (currentAPSAddress.shortAddr.Val != 0xFFFF)
        {
            if ( !memcmp((void*)longAddr, (void*)&currentAPSAddress.longAddr, (BYTE)(sizeof(LONG_ADDR))) )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
#endif

/*********************************************************************
 * Function:        BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap)
 *
 * PreCondition:    None
 *
 * Input:           AddressMap - pointer to the APS_ADDRESS_MAP to be saved
 *
 * Output:          TRUE - operation successful
 *                  FALSE - no more APS_ADDRESS_MAP slot
 *
 * Side Effects:    address map modified to include the new item
 *
 * Overview:        Searches the APS address map, find empty slot to save
 *                  the new match between short address and long address
 *
 * Note:            None
 ********************************************************************/
#if MAX_APS_ADDRESSES > 0

BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap)
{
    BYTE i;
    APS_ADDRESS_MAP tmpMap = *AddressMap;

    if( LookupAPSLongAddress(&(AddressMap->shortAddr)) )
    {
        return TRUE;
    }

    for( i = 0; i < apscMaxAddrMapEntries; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
        #else
            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
        #endif
        if (currentAPSAddress.shortAddr.Val == 0xFFFF)
        {
            #ifdef USE_EXTERNAL_NVM
                PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &tmpMap );
            #else
                PutAPSAddress( &apsAddressMap[i], &tmpMap );
            #endif
            return TRUE;
        }
    }

    /* APS Table is full so return an error indication */
    return FALSE;
}
#endif

/*********************************************************************
 * Function:        BYTE LookupAPSLongAddress( LONG_ADDR *longAddr )
 *
 * PreCondition:    None
 *
 * Input:           shortAddr - pointer to short address to match
 *
 * Output:          TRUE - a corresponding long address was found and
 *                      is held in currentAPSAddress.shortAddr
 *                  FALSE - a corresponding long address was not found
 *
 * Side Effects:    currentAPSAddress is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the APS address map for the long address
 *                  of a given short address.
 *
 * Note:            The end application is responsible for populating
 *                  this table.
 ********************************************************************/
#if MAX_APS_ADDRESSES > 0

BOOL LookupAPSLongAddress(INPUT SHORT_ADDR *shortAddr)
{
    BYTE i;

    for(i = 0; i < apscMaxAddrMapEntries; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
        #else
            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
        #endif
        if( currentAPSAddress.shortAddr.Val != 0xFFFF )
        {

            if( currentAPSAddress.shortAddr.Val == shortAddr->Val)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
#endif


/*********************************************************************
 * Function:        BYTE LookupSourceBindingInfo( SHORT_ADDR srcAddr,
 *                                          BYTE srcEP,
 *                                          WORD_VAL clusterID)
 *
 * PreCondition:    None
 *
 * Input:           srcAddr     - short address of source node
 *                  srcEP       - source end point
 *                  clusterID   - cluster id
 *
 * Output:          key to the source binding record if matching record found
 *                  END_BINDING_RECORDS otherwise
 *
 * Side Effects:    pCurrentBindingRecord and currentBindingRecord
 *                  are set to the source binding record
 *
 * Overview:        Searches binding table for matching source binding.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
BYTE LookupSourceBindingInfo( SHORT_ADDR srcAddr, BYTE srcEP, WORD_VAL clusterID )
{
    BYTE            bindingIndex    = 0;
    BYTE            bindingMapSourceByte;
    BYTE            bindingMapUsageByte;

    while (bindingIndex < MAX_BINDINGS)
    {
        GetBindingSourceMap( &bindingMapSourceByte, bindingIndex );
        GetBindingUsageMap( &bindingMapUsageByte, bindingIndex );

        if (BindingIsUsed( bindingMapUsageByte,  bindingIndex ) &&
            BindingIsUsed( bindingMapSourceByte, bindingIndex ))
        {

            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(bindingIndex) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[bindingIndex];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );

            // If we find the matching source, we're done
            if ((currentBindingRecord.shortAddr.Val == srcAddr.Val) &&
                (currentBindingRecord.endPoint == srcEP) &&
                (currentBindingRecord.clusterID.Val == clusterID.Val))
            {
                return bindingIndex;
            }
        }
        bindingIndex ++;
    }

    // We didn't find a match, so return an error condition
    return END_BINDING_RECORDS;
}
#endif

/*********************************************************************
 * Function:        void RemoveAllBindings(SHORT_ADDR shortAddr)
 *
 * PreCondition:    none
 *
 * Input:           shortAddr - Address of a node on the network
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Searches binding table and removes all entries
 *                  associated with the address shortAddr.  Nodes are
 *                  removed by clearing the usage flag in the usage
 *                  map and fixing up any destination links.  If the
 *                  node is a source node, then all associated
 *                  destination nodes are also removed.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS)
void RemoveAllBindings(SHORT_ADDR shortAddr)
{
    BYTE                bindingIndex    = 0;
    BYTE                bindingMapSourceByte;
    BYTE                bindingMapUsageByte;
    BINDING_RECORD      nextRecord;
    #ifdef USE_EXTERNAL_NVM
        WORD                pPreviousRecord;
    #else
        ROM BINDING_RECORD  *pPreviousRecord;
    #endif

    while (bindingIndex < MAX_BINDINGS)
    {
        // Get the maps for each check, in case we overwrote them removing other entries
        GetBindingSourceMap( &bindingMapSourceByte, bindingIndex );
        GetBindingUsageMap( &bindingMapUsageByte, bindingIndex );

        if (BindingIsUsed( bindingMapUsageByte,  bindingIndex ) &&
            BindingIsUsed( bindingMapSourceByte, bindingIndex ))
        {
            // Read the source node record into RAM.
            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(bindingIndex) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[bindingIndex];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );

            // See if the source node address matches the address we are trying to find
            if (currentBindingRecord.shortAddr.Val == shortAddr.Val)
            {
                // Remove this source node and all destination nodes
                MarkBindingUnused( bindingMapUsageByte, bindingIndex );
                PutBindingUsageMap( &bindingMapUsageByte, bindingIndex );
                MarkBindingUnused( bindingMapSourceByte, bindingIndex );
                PutBindingSourceMap( &bindingMapSourceByte, bindingIndex );
                while (currentBindingRecord.nextBindingRecord != END_BINDING_RECORDS)
                {
                    // Read the destination node record into RAM.
                    GetBindingUsageMap( &bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                    MarkBindingUnused( bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                    PutBindingUsageMap( &bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                    #ifdef USE_EXTERNAL_NVM
                        GetBindingRecord(&currentBindingRecord, apsBindingTable + (WORD)currentBindingRecord.nextBindingRecord * (WORD)sizeof(BINDING_RECORD) );
                    #else
                        GetBindingRecord(&currentBindingRecord, &apsBindingTable[currentBindingRecord.nextBindingRecord] );
                    #endif
                }
            }
            else
            {
                #ifdef USE_EXTERNAL_NVM
                    pPreviousRecord = apsBindingTable + (WORD)bindingIndex * (WORD)sizeof(BINDING_RECORD);
                #else
                    pPreviousRecord = &apsBindingTable[bindingIndex];
                #endif

                // See if any of the destination nodes in this list matches the address
                while (currentBindingRecord.nextBindingRecord != END_BINDING_RECORDS)
                {
                    // Read the destination node record into RAM.
                    #ifdef USE_EXTERNAL_NVM
                        pCurrentBindingRecord = apsBindingTable + (WORD)(currentBindingRecord.nextBindingRecord) * (WORD)sizeof(BINDING_RECORD);
                    #else
                        pCurrentBindingRecord = &apsBindingTable[currentBindingRecord.nextBindingRecord];
                    #endif
                    GetBindingRecord(&nextRecord, pCurrentBindingRecord );
                    if (nextRecord.shortAddr.Val == shortAddr.Val)
                    {
                        // Remove the destination node and patch up the list
                        GetBindingUsageMap( &bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                        MarkBindingUnused( bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                        PutBindingUsageMap( &bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                        currentBindingRecord.nextBindingRecord = nextRecord.nextBindingRecord;
                        PutBindingRecord( pPreviousRecord, &currentBindingRecord );
                    }
                    else
                    {
                        // Read the next destination node record into RAM.
                        pPreviousRecord = pCurrentBindingRecord;
                        #ifdef USE_EXTERNAL_NVM
                            pCurrentBindingRecord = apsBindingTable + (WORD)(currentBindingRecord.nextBindingRecord) * (WORD)sizeof(BINDING_RECORD);
                        #else
                            pCurrentBindingRecord = &apsBindingTable[currentBindingRecord.nextBindingRecord];
                        #endif
                        GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
                    }
                }

                // If we just deleted the only destination, delete the source as well.
                // Read the source node record into RAM.
                #ifdef USE_EXTERNAL_NVM
                    pCurrentBindingRecord = apsBindingTable + (WORD)(bindingIndex) * (WORD)sizeof(BINDING_RECORD);
                #else
                    pCurrentBindingRecord = &apsBindingTable[bindingIndex];
                #endif
                GetBindingRecord( &currentBindingRecord, pCurrentBindingRecord );
                if (currentBindingRecord.nextBindingRecord == END_BINDING_RECORDS)
                {
                    GetBindingUsageMap( &bindingMapUsageByte, bindingIndex );
                    MarkBindingUnused( bindingMapUsageByte, bindingIndex );
                    PutBindingUsageMap( &bindingMapUsageByte, bindingIndex );
                    GetBindingSourceMap( &bindingMapSourceByte, bindingIndex );
                    MarkBindingUnused( bindingMapSourceByte, bindingIndex );
                    PutBindingSourceMap( &bindingMapSourceByte, bindingIndex );
                }
            }
        }
        bindingIndex ++;
    }
}
#endif

/*********************************************************************
 * Function:        BOOL APSFromShortToLong(INPUT SHORT_ADDR *ShortAddr)
 *
 * PreCondition:    None
 *
 * Input:           ShortAddr - pointer to short address to match long address
 *
 * Output:          TRUE - a corresponding long address was found and
 *                      is held in currentAPSAddress.longAddr
 *                  FALSE - a corresponding long address was not found
 *
 * Side Effects:    currentAPSAddress is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the APS address map and neighbor table for the long address
 *                  of a given short address.
 *
 * Note:            The end application is responsible for populating
 *                  this table.
 ********************************************************************/


BOOL APSFromShortToLong(INPUT SHORT_ADDR *ShortAddr)
{
    #if MAX_APS_ADDRESSES > 0
        if( LookupAPSLongAddress(ShortAddr) )
        {
            return TRUE;
        }
    #endif

    if( INVALID_NEIGHBOR_KEY != NWKLookupNodeByShortAddrVal(ShortAddr->Val) )
    {
        if( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ||
            currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT )
        {
            currentAPSAddress.longAddr = currentNeighborRecord.longAddr;
            return TRUE;
        }
    }

    return FALSE;

}


/*********************************************************************
 * Function:        BOOL APSFromLongToShort(INPUT SHORT_ADDR *LongAddr)
 *
 * PreCondition:    None
 *
 * Input:           LongAddr - pointer to long address to match short address
 *
 * Output:          TRUE - a corresponding short address was found and
 *                      is held in currentAPSAddress.shortAddr
 *                  FALSE - a corresponding short address was not found
 *
 * Side Effects:    currentAPSAddress is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the APS address map and neighbor table for the short address
 *                  of a given long address.
 *
 * Note:            The end application is responsible for populating
 *                  this table.
 ********************************************************************/


BOOL APSFromLongToShort(INPUT LONG_ADDR *LongAddr)
{
    #if MAX_APS_ADDRESSES > 0
        if( LookupAPSAddress(LongAddr) )
        {
            return TRUE;
        }
    #endif

    if( INVALID_NEIGHBOR_KEY != NWKLookupNodeByLongAddr(LongAddr) )
    {
        currentAPSAddress.shortAddr = currentNeighborRecord.shortAddr;
        return TRUE;
    }

    return FALSE;
}



BOOL    DuplicatePacket(INPUT SHORT_ADDR SrcAddress, INPUT BYTE currentAPSCounter, INPUT BYTE currentBlockNum)
{
    BYTE i;
    BYTE updateIndex = MAX_DUPLICATE_TABLE;

    apsStatus.flags.bits.bDuplicateTable = 1;
    for(i = 0; i < MAX_DUPLICATE_TABLE; i++)
    {
        if( SrcAddress.Val == apsDuplicateTable[i].SrcAddress.Val &&
            currentAPSCounter == apsDuplicateTable[i].APSCounter &&
            currentBlockNum == apsDuplicateTable[i].blockNum )
        {
            apsDuplicateTable[i].StartTick = TickGet();
            if( updateIndex < MAX_DUPLICATE_TABLE )
            {
                apsDuplicateTable[updateIndex].SrcAddress.Val = 0xFFFF;
            }
            return TRUE;
        }

        if( updateIndex == MAX_DUPLICATE_TABLE && apsDuplicateTable[i].SrcAddress.Val == 0xFFFF )
        {
            apsDuplicateTable[i].SrcAddress.Val = params.NLDE_DATA_indication.SrcAddress.Val;
            apsDuplicateTable[i].APSCounter = currentAPSCounter;
            apsDuplicateTable[i].blockNum = currentBlockNum;
            apsDuplicateTable[i].StartTick = TickGet();
            updateIndex = i;
        }
    }

    return FALSE;
}

/*********************************************************************
 * Function:        BYTE    GetEndPointsFromGroup(INPUT SHORT_ADDR GroupAddr)
 *
 * PreCondition:    None
 *
 * Input:           GroupAddr - pointer to long address to match short address
 *
 * Output:          BYTE - a corresponding to the index in the group table
 *                      where the matching GroupAddr entry was found
 *                  MAX_GROUP - a corresponding index if no match was  found
 *
 * Side Effects:    currentGroupAddressRecord is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the Group table for the GroupAddr.Val
 *                  given as input.
 *
 * Note:            The end application is responsible for populating
 *                  the Group table.
 ********************************************************************/
#if defined(I_SUPPORT_GROUP_ADDRESSING)
BYTE    GetEndPointsFromGroup(INPUT SHORT_ADDR GroupAddr)
{
    BYTE i;

    for(i = 0; i < MAX_GROUP; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(i) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
        #else
            pCurrentGroupAddressRecord = &apsGroupAddressTable[i];
        #endif
        GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
        if( currentGroupAddressRecord.GroupAddress.Val == GroupAddr.Val )
        {
            return i;
        }
    }
    return MAX_GROUP;
}
#endif

#if defined(I_SUPPORT_GROUP_ADDRESSING)
/*********************************************************************
 * Function:        BYTE    GetEmptyGroup(void)
 *
 * PreCondition:    None
 *
 * Input:           None -
 *
 * Output:          BYTE - corresponding to the index in the group table
 *                      where the first empty slot is found i.e. 0xffff
 *                  MAX_GROUP - corresponding index if no match was  found
 *
 * Side Effects:    currentGroupAddressRecord is destroyed
 *
 *
 * Overview:        Searches the Group table for the first empty slot
 *
 * Note:            The end application is responsible for populating
 *                  the Group table.
 ********************************************************************/
BYTE    GetEmptyGroup(void)
{
    BYTE i;

    for(i = 0; i < MAX_GROUP; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(i) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
        #else
            pCurrentGroupAddressRecord = &apsGroupAddressTable[i];
        #endif
        GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
        if( currentGroupAddressRecord.GroupAddress.Val == 0xFFFF )
        {
            return i;
        }
    }
    return MAX_GROUP;
}

/*********************************************************************
  Function:        
    BYTE    AddGroup(INPUT WORD_VAL GroupAddress, INPUT BYTE EndPoint)
  
  Summary:
    Add a device to a group.
  
  Description:
    Allows a device to become member of a group.  The function
    searches the Group table for an empty slot and adds the
    GroupAddress + EndPoint to the table.
     
  Precondition:    
    The device must first support a Group Table in order
    to become a member of a group.  End Devices do not 
    support group table. 

  Parameters:
   <table>
    GroupAddress    WORD_VAL representing the GroupAddress to be added to the the group table
    Endpoint        BYTE the destination endpoint to be linked to the GroupAddress
   </table> 

  Returns:
    <table>       
     GROUP_SUCCESS      if entry was successfully added to GroupTable 
     GROUP_TABLE_FULL   if GroupTable was full
     GROUP_INVALID_PARAMETER    If invalid GroupAddress used i.e > 0xFFF7 or EndPoint == 0
    </table>
    
  Example:
    None
  
 Remarks:
   The currentGroupAddressRecord data structure is destroyed, 
   and the GroupTable is modified
 ********************************************************************/
BYTE    AddGroup(INPUT SHORT_ADDR GroupAddress, INPUT BYTE EndPoint)
{
    BYTE groupIndex;
    BYTE i;
    BYTE j;

    if( EndPoint == 0x00 ||
        GroupAddress.Val > 0xFFF7 )
    {
        return GROUP_INVALID_PARAMETER;
    }

    groupIndex = GetEndPointsFromGroup(GroupAddress);
    if( groupIndex == MAX_GROUP )
    {
        if( (groupIndex = GetEmptyGroup()) == MAX_GROUP )
        {
            return GROUP_TABLE_FULL;
        }
        currentGroupAddressRecord.GroupAddress.Val = GroupAddress.Val;
        currentGroupAddressRecord.EndPoint[0] = EndPoint;
        for(j = 1; j < MAX_GROUP_END_POINT; j++)
        {
            currentGroupAddressRecord.EndPoint[j] = 0xFF;
        }
    }
    else
    {
        for(i = 0; i < MAX_GROUP_END_POINT; i++)
        {
            if( currentGroupAddressRecord.EndPoint[i] == 0xFF )
            {
                break;
            }
            if( currentGroupAddressRecord.EndPoint[i] == EndPoint )
            {
                return GROUP_SUCCESS;
            }
        }
        if( i == MAX_GROUP_END_POINT )
        {
            return GROUP_TABLE_FULL;
        }
        currentGroupAddressRecord.EndPoint[i] = EndPoint;
    }

    #ifdef USE_EXTERNAL_NVM
        pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(groupIndex) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
    #else
        pCurrentGroupAddressRecord = &apsGroupAddressTable[groupIndex];
    #endif
    PutGroupAddress(pCurrentGroupAddressRecord, &currentGroupAddressRecord);

    return GROUP_SUCCESS;
}

/*********************************************************************
  Function:        
    BYTE    RemoveGroup(INPUT WORD GroupAddress, INPUT BYTE EndPoint)

  Summary:
    Searches the Group table for  GroupAddress + EndPoint and 
    sets them to 0xffff and 0xff respectively in the Group Table

 * PreCondition:    None
 *
 * Input:           GroupAddress - WORD_VAL representing the GroupAddress to be removed from the 
 *                                  the group table
 *                  Endpoint    - the destination endpoint to be unlinked from the GroupAddress in table
 *
 * Output:          BYTE  GROUP_SUCCESS     - if entry was successfully removed from GroupTable 
 *                  BYTE  GROUP_INVALID_PARAMETER   - If invalid GroupAddress used i.e > 0xFFF7 or EndPoint == 0
 *
 * Side Effects:    currentGroupAddressRecord is destroyed, GroupTable modified 
 *
 * Overview:        
 *
 * Note:            The end application is responsible for creating 
 *                  the Group table.
 ********************************************************************/    
BYTE    RemoveGroup(INPUT WORD GroupAddress, INPUT BYTE EndPoint)
{
    BYTE i;
    BYTE j;
    BYTE k;

    if( EndPoint == 0x00 ||
        ( GroupAddress > 0xFFF7 && GroupAddress != 0xFFFF ) )
    {
        return GROUP_INVALID_PARAMETER;
    }

    for(i = 0; i < MAX_GROUP; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(i) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
        #else
            pCurrentGroupAddressRecord = &apsGroupAddressTable[i];
        #endif
        GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
        if( GroupAddress == 0xFFFF ||
            GroupAddress == currentGroupAddressRecord.GroupAddress.Val )
        {
            for(j = 0; j < MAX_GROUP_END_POINT; j++)
            {
                if( currentGroupAddressRecord.EndPoint[j] == EndPoint )
                {
                    break;
                }
            }
            if( j != MAX_GROUP_END_POINT )
            {
                if( j == 0 && (currentGroupAddressRecord.EndPoint[1] == 0xFF))
                {
                    currentGroupAddressRecord.EndPoint[0] = 0xFF;
                    currentGroupAddressRecord.GroupAddress.Val = 0xFFFF;
                    PutGroupAddress(pCurrentGroupAddressRecord, &currentGroupAddressRecord);
                }
                else
                {
                    for(k = j; k < MAX_GROUP_END_POINT-1; k++)
                    {
                        currentGroupAddressRecord.EndPoint[k] = currentGroupAddressRecord.EndPoint[k+1];
                    }
                    currentGroupAddressRecord.EndPoint[MAX_GROUP_END_POINT-1] = 0xFF;
                    PutGroupAddress(pCurrentGroupAddressRecord, &currentGroupAddressRecord);
                }

                if( GroupAddress != 0xFFFF )
                {
                    break;
                }
            }
        }
    }
    return GROUP_SUCCESS;
}

/****************************************************************************
  Function:
    void RemoveAllGroups(void)

  Summary:
    Empties the Group Table of a device.  

  Description:
    Searches the Group table and marks all the slots as empty

  Precondition:
    None

  Parameters:
    None

  Returns:
    None.

  Example:
  

  Remarks:
    The sample application is responsible for populating
    the Group table.
 ********************************************************************/
void RemoveAllGroups(void)
{
    BYTE i;
    BYTE j;

    for(i = 0; i < MAX_GROUP; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(i) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
        #else
            pCurrentGroupAddressRecord = &apsGroupAddressTable[i];
        #endif
        GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
        currentGroupAddressRecord.GroupAddress.Val = 0xFFFF;
        for(j = 0; j < MAX_GROUP_END_POINT; j++)
        {
            currentGroupAddressRecord.EndPoint[j] = 0xFF;
        }
        PutGroupAddress(pCurrentGroupAddressRecord, &currentGroupAddressRecord);
    }
}
#endif

void APLDiscardRx( void )
{
    if ( apsStatus.flags.bits.bLoopBackData )
    {
        apsStatus.flags.bits.bLoopBackData = 0;
    }
    else
    {
        APSDiscardRx();
    }
}

/*********************************************************************
 * Function:        void UpdateFragmentParams ( BYTE WindowSize, BYTE InterframeDelay, BYTE DataLengthPerBlock, BYTE TotalFragmentDataLength)
  Summary:
    Initializes the parameters that controls how a fragmented
    packet will be transmitted.
        
  Description:
    This function is used for setting parameters which are
    used by fragmentation feature to control the how the 
    actual fragmentation is carried out.
 
  PreCondition:    
    None
 
  Parameters:
  <table>           
    WindowSize              Specifies the number of blocks that can be Txd/Rxd in one window
    InterframeDelay         Specifies the time delay between the block transmission 
    DataLengthPerBlock      Specifies the data length that can be fit in one block transmission
    TotalFragmentDataLength Specifies the total number bytes that needs to be transmitted using fragmentation
  </table>
 
  Returns:          
    None
    
  Remarks:            
    None
 ********************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
void UpdateFragmentParams ( BYTE WindowSize, BYTE InterframeDelay, BYTE DataLengthPerBlock, WORD TotalFragmentDataLength )
{
    fragmentWindowSize = WindowSize;
    fragmentInterframeDelay = InterframeDelay;
    fragmentDataSize = DataLengthPerBlock;
    fragmentTotalDataLength = TotalFragmentDataLength;
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE HandleFragmentDataRequest( BYTE dataLength, BYTE *data )
 *
  Summary:
    The function manages the transmision of a fragmented packet
        
  Description:
    This function updates the FRAGMENTATION_INFO structure and 
    starts transmitting the fragmented packet.
 
  PreCondition:    
    None
 
  Parameters:
  <table>           
    dataLength  Total number of bytes to be transmitted
    Data        pointer to a buffer containing the data to be transmitted
  </table>
 
  Returns:          
    NO_PRIMITIVE
    
  Remarks:            
    None
 ********************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
ZIGBEE_PRIMITIVE HandleFragmentDataRequest( WORD dataLength, BYTE *data )
{
    BYTE i;
    LONG_ADDR destLongAddr;
    SHORT_ADDR  destShortAddr;
    NODE_DESCRIPTOR     nodeDesc;

    ProfileGetNodeDesc(&nodeDesc);

    if ( apsStatus.flags.bits.bFragmentationInProgress )
    {
        // We are already processing one fragmentation. We cant process one more.
        SendFragmentDataConfirm(APS_FRAMGENTATION_UNDER_PROGRESS);
        return NO_PRIMITIVE;
    }

    // Backup the APS Header related fields
    apsFragmentationInfo.ProfileId.Val      = params.APSDE_DATA_request.ProfileId.Val;
    apsFragmentationInfo.SrcEndpoint        = params.APSDE_DATA_request.SrcEndpoint;
    apsFragmentationInfo.DstEndpoint        = params.APSDE_DATA_request.DstEndpoint;
    apsFragmentationInfo.ClusterId.Val      = params.APSDE_DATA_request.ClusterId.Val;
    apsFragmentationInfo.FragmentTxOptions.Val      = params.APSDE_DATA_request.TxOptions.Val;

    if ( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_16_BIT )
    {
        destShortAddr.Val = params.APSDE_DATA_request.DstAddress.ShortAddr.Val;
    }
    else if ( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT )
    {
        for (i=0; i<8; i++)
        {
            destLongAddr.v[i] = params.APSDE_DATA_request.DstAddress.v[i];
        }
        if ( !IsThisLongAddressKnown ( &destLongAddr, &destShortAddr.v[0], 0x01 ) ) // 0x01 means search in Neighbor and AddressMap table
        {
            // asdu cannot be fragmented.
            SendFragmentDataConfirm(APS_INVALID_PARAMERTER);
            return NO_PRIMITIVE;
        }
    }
    else
    {
        // asdu cannot be fragmented.
        SendFragmentDataConfirm(APS_INVALID_PARAMERTER);
        return NO_PRIMITIVE;
    }

    apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val  = destShortAddr.Val;

    if ( ( ( params.APSDE_DATA_request.TxOptions.bits.useNWKKey ) &&
           ( fragmentDataSize > nodeDesc.NodeMaxBufferSize ) ) ||
         ( ( params.APSDE_DATA_request.TxOptions.bits.securityEnabled ) &&
           ( fragmentDataSize > ( nodeDesc.NodeMaxBufferSize - 14 - 4 ) ) ) ) // 14 -> Auxiliary Header, 4 -> MIC for SecurityLevel 5
    {
        SendFragmentDataConfirm(FRAME_TOO_LONG);
        return NO_PRIMITIVE;
    }

    apsFragmentationInfo.FragmentTxInitiator            = 1;
    apsFragmentationInfo.DataPayloadLength              = dataLength;
    apsFragmentationInfo.DataPayloadAddress             = data;
    apsFragmentationInfo.TotalNumberOfBlocks            = GetTotalNumberOfBlocks();
    apsFragmentationInfo.WindowSize                     = 1;
    apsFragmentationInfo.BlockNumberToTxOrRx            = 0;
    apsFragmentationInfo.FirstBlockInWindow             = 0;
    apsFragmentationInfo.LastBlockInWindow              = ( fragmentWindowSize * apsFragmentationInfo.WindowSize ) - 1;
    UpdateDataPayloadLengthAndAddress();
    apsFragmentationInfo.APSCounter                     = APSCounter++;
    apsFragmentationInfo.IsThisBlockRetried             = 0;
    apsFragmentationInfo.RetryCounter                   = 0;
    apsFragmentationInfo.InterframeDelayTick            = TickGet();
    apsFragmentationInfo.WaitingForAck                  = 0;

    apsStatus.flags.bits.bFragmentationInProgress = 1;

    #ifdef I_AM_END_DEVICE
        QuickPoll = 1;
    #endif

    return NO_PRIMITIVE;
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*********************************************************************
 * Function:        void UpdateDataPayloadLengthAndAddress ( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          DataLength and Data will be updated for the next fragment packet that needs to be transmitted.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
void UpdateDataPayloadLengthAndAddress ( void )
{
    BYTE        count = 0;
    WORD        dataLength, currentDataLength, previousDataLength, dataLengthPerFrame;
    BYTE        *ptr1;
    BYTE        *ptr2;

    dataLengthPerFrame = fragmentDataSize;

    dataLength = apsFragmentationInfo.DataPayloadLength;
    // Should start from where we have left before.
    currentDataLength = ((apsFragmentationInfo.BlockNumberToTxOrRx) * dataLengthPerFrame);
    previousDataLength = currentDataLength;
    ptr1 = (apsFragmentationInfo.DataPayloadAddress + currentDataLength);

    ptr2 = (BYTE *)SRAMalloc(fragmentDataSize);
    if (ptr2 == NULL)
    {
        return;
    }
    apsFragmentationInfo.asdu = ptr2;

    while ( ( currentDataLength < dataLength ) &&
            ( ( currentDataLength - previousDataLength ) != dataLengthPerFrame ) )
    {
        *ptr2++ = *ptr1++;
        currentDataLength++;
        count++;
    }

    apsFragmentationInfo.asduLength = count;
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*********************************************************************
 * Function:        BYTE GetTotalNumberOfBlocks (void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Calculates the total number of blocks that needs to be fragmented and transmitted.
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
BYTE GetTotalNumberOfBlocks (void)
{
    BYTE        totalBlocks = 0;
    WORD        dataLength, currentDataLength, previousDataLength, dataLengthPerFrame;

    dataLength = apsFragmentationInfo.DataPayloadLength;
    dataLengthPerFrame = fragmentDataSize;
    currentDataLength = 1;
    previousDataLength = currentDataLength - 1;

    while ( currentDataLength <= dataLength )
    {
        if ( ( currentDataLength - previousDataLength ) == dataLengthPerFrame )
        {
            if ( currentDataLength != dataLength )
            {
                previousDataLength = currentDataLength;
            }
            totalBlocks++;
        }
        currentDataLength++;
    }
    if ( ( ( currentDataLength - 1 ) - previousDataLength ) != dataLengthPerFrame )
    {
        totalBlocks++;
    }
    return totalBlocks;
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*********************************************************************
 * Function:        void RetryBlock (BYTE AllBlocksInWindow)
 *
 * PreCondition:    None
 *
 * Input:           AllBlocksInWindow - if TRUE, all the blocks in the window should be retried
 *                                      FALSE, a particular block should be retried.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to check whether any/all/none of the needs to be retried.
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
void RetryBlock (BYTE AllBlocksInWindow)
{
    BYTE        i;
    BYTE        missedBlocks;

    if (AllBlocksInWindow)
    {
        apsFragmentationInfo.BlockNumberToTxOrRx = apsFragmentationInfo.FirstBlockInWindow;
        UpdateDataPayloadLengthAndAddress();
        apsFragmentationInfo.RetryCounter++;
    }
    else
    {
        missedBlocks = apsFragmentationInfo.MissedBlockInfo.AckBitField;

        for (i=0; i<8; i++)
        {
            if ( missedBlocks & 0x01 )
            {
                missedBlocks = missedBlocks >> 1;
            }
            else
            {
                apsFragmentationInfo.BlockNumberToTxOrRx = apsFragmentationInfo.FirstBlockInWindow + i;
                apsFragmentationInfo.IsThisBlockRetried = 1;
                apsFragmentationInfo.MissedBlockInfo.RetriedBlockNumber = apsFragmentationInfo.BlockNumberToTxOrRx;
                UpdateDataPayloadLengthAndAddress();
                apsFragmentationInfo.RetryCounter++;
                break;
            }
        }
    }

}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*********************************************************************
 * Function:        void SendFragmentDataConfirm (BYTE Status)
 *
 * PreCondition:    None
 *
 * Input:           None -
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to send the fragment data confirm to the application
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
void SendFragmentDataConfirm (BYTE Status)
{
    apsStatus.flags.bits.bFragmentationInProgress = 0;
    apsStatus.flags.bits.bWaitingForFragmentDataConfirm = 0;
    apsFragmentationInfo.WaitingForAck = 0;
    apsFragmentationInfo.FragmentTxInitiator = 0;
    apsFragmentationInfo.RetryCounter = 0;

    #ifdef I_AM_END_DEVICE
        QuickPoll = 0;
    #endif

    #if (I_SUPPORT_FRAGMENTATION_TESTING == 1)
        TestErrorWindow = 0x00;
        TestErrorPacket = 0xFF;
    #endif //(I_SUPPORT_FRAGMENTATION_TESTING == 1)

    params.APSDE_DATA_confirm.DstAddrMode       = APS_ADDRESS_16_BIT;
    params.APSDE_DATA_confirm.DstAddress        = apsFragmentationInfo.FragmentDstAddr;
    params.APSDE_DATA_confirm.DstEndpoint       = apsFragmentationInfo.DstEndpoint;
    params.APSDE_DATA_confirm.SrcEndpoint       = apsFragmentationInfo.SrcEndpoint;
    params.APSDE_DATA_confirm.Status            = Status;

    HandleFragmentDataConfirm();

    if ( ( Status == SUCCESS ) ||
         ( Status == APS_NO_ACK ) ||
         ( Status == APS_SECURITY_FAIL ) )
    {
    }
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE UpdateFragmentationInfoFields
 *                                   (
 *                                      BYTE        fragmentType,
 *                                      BYTE        blockNumber,
 *                                      WORD_VAL    clusterID,
 *                                      WORD_VAL    profileID,
 *                                      BYTE        dstEndPoint,
 *                                      BYTE        srcEndPoint,
 *                                      BYTE        apsCounter,
 *                                  )
 *
 * PreCondition:    None
 *
 * Input:           fragmentType    -> First or Part
 *                  blockNumber     -> Block Number contained in the recevied fragment data
 *                  clusterID       -> Cluster Id of the recevied fragment data
 *                  profileID       -> Profile Id of the recevied fragment data
 *                  dstEndPoint     -> Dst End Point of the recevied fragment data
 *                  srcEndPoint     -> Src End Point of the recevied fragment data
 *                  apsCounter      -> Aps Counter of the recevied fragment data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function updates FRAGMENTATION_INFO strucuture upon receiving the fragment data packet
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
ZIGBEE_PRIMITIVE UpdateFragmentationInfoFields
    (
        BYTE        fragmentType,
        BYTE        blockNumber,
        WORD_VAL    clusterID,
        WORD_VAL    profileID,
        BYTE        dstEndPoint,
        BYTE        srcEndPoint,
        BYTE        apsCounter,
        SHORT_ADDR  ackDstAddr
    )
{
    BYTE        i, AckBlockBits, currentBlockNumberAck, currentAckBitField;
    BYTE        *fragmentData, *ptr;

    if ( fragmentType & APS_EXTENDED_HDR_FIRST_FRAGMENT )
    {
        // In the above cases, drop the packet. Otherwise process it.
        if ( ( ( apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val != 0xFFFF ) &&
               ( params.NLDE_DATA_indication.SrcAddress.Val != apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val ) ) )
        {
            APSDiscardRx();
            return NO_PRIMITIVE;
        }

		/* If stack receives more than one of the first block of fragmented data - then reject it */
		if (apsStatus.flags.bits.bFragmentationInProgress)
		{
            APSDiscardRx();
			return NO_PRIMITIVE;
		}			
        apsStatus.flags.bits.bFragmentationInProgress = 1;

        #ifdef I_AM_END_DEVICE
            QuickPoll = 1;
        #endif

        // Update the FRAGMENTATION_INFO structure.
        apsFragmentationInfo.ProfileId.Val  = profileID.Val;
        apsFragmentationInfo.SrcEndpoint    = srcEndPoint;
        apsFragmentationInfo.DstEndpoint    = dstEndPoint;
        apsFragmentationInfo.ClusterId.Val  = clusterID.Val;

        if ( params.APSDE_DATA_indication.SecurityStatus == APS_SECURED_LINK_KEY )
        {
            apsFragmentationInfo.FragmentTxOptions.bits.securityEnabled = 1;
        }
        else if ( params.APSDE_DATA_indication.SecurityStatus == APS_SECURED_NWK_KEY )
        {
            apsFragmentationInfo.FragmentTxOptions.bits.useNWKKey = 1;
        }
        else
        {
            apsFragmentationInfo.FragmentTxOptions.Val = 0;
        }

        apsFragmentationInfo.AllFragmentsReceived       = 0;
        apsFragmentationInfo.FragmentDstAddr.ShortAddr  = params.NLDE_DATA_indication.SrcAddress;
        apsFragmentationInfo.TotalNumberOfBlocks        = blockNumber;
        apsFragmentationInfo.PreviousDataPayloadLength  = params.NLDE_DATA_indication.NsduLength;
        apsFragmentationInfo.DataPayloadLength          = params.NLDE_DATA_indication.NsduLength * blockNumber;
        apsFragmentationInfo.SecurityStatus             = params.NLDE_DATA_indication.SecurityUse;
        apsFragmentationInfo.LinkQuality                = params.NLDE_DATA_indication.LinkQuality;
        /* Get the pointer to the static buffer */
        fragmentData = GetFragmentDataBuffer();
        
        if ( fragmentData == NULL )
        {
            // Reset the flags.
            apsStatus.flags.bits.bFragmentationInProgress = 0;
            apsStatus.flags.bits.bPendingFragmentDataIndication = 0;

            apsFragmentationInfo.FragmentationStartTime.Val = 0;

            apsFragmentationInfo.AllFragmentsReceived = 0;
            apsFragmentationInfo.IsThisBlockRetried = 0;

            #ifdef I_AM_END_DEVICE
                QuickPoll = 0;
            #endif

            // Reset the FragmentDstAddr, so that next fragmentation can be processed.
            apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val = 0xFFFF;
            
            return NO_PRIMITIVE;
        }
        
        /* Assign payload address */
        apsFragmentationInfo.DataPayloadAddress = fragmentData;
        
      
        apsFragmentationInfo.WindowSize                     = 1;
        apsFragmentationInfo.BlockNumberToTxOrRx            = 0;
        if ( fragmentWindowSize > 1)
        {
            apsFragmentationInfo.BlockNumberToTxOrRx        = 1;
        }

        apsFragmentationInfo.FirstBlockInWindow             = 0;
        apsFragmentationInfo.LastBlockInWindow              =
                    ( fragmentWindowSize * apsFragmentationInfo.WindowSize ) - 1;

        // Update the block number to be received in the APS ACK frame.
        apsFragmentationInfo.ExpectingAckForTheBlockNumber = apsFragmentationInfo.FirstBlockInWindow;

        // Total number of blocks may be less than the first window itself.
        if ( apsFragmentationInfo.LastBlockInWindow > ( apsFragmentationInfo.TotalNumberOfBlocks - 1 ) )
        {
            // In this case, AckBitField should be set to 0 only for those blocks which will be received. All other bits
            // in the AckBitField should be set to 1.
            // Example: Consider the following.
            //      fragmentWindowSize = 5, TotalNumberOfBlocks = 2, WindowSize = 1
            //      In this case, AckBitField should be set to 0xFC, beacuse only two blocks needs to tranmitted.
            //      Each bit corresponds to each block in the sequencial order.
            apsFragmentationInfo.LastBlockInWindow = ( apsFragmentationInfo.TotalNumberOfBlocks - 1 );
            apsFragmentationInfo.MissedBlockInfo.AckBitField
                    = ( 0xFF << ( ( apsFragmentationInfo.LastBlockInWindow - apsFragmentationInfo.FirstBlockInWindow ) + 1 ) ) | 0x01;
        }
        else
        {
            // Otherwise, update AckBitField based on the window size.
            // OR operation is used because we have already received (current block) the first block. So, first bit should be set.
            apsFragmentationInfo.MissedBlockInfo.AckBitField = ( 0xFF << fragmentWindowSize ) | 0x01;
        }

        apsFragmentationInfo.APSCounter                     = apsCounter;

        ptr = params.NLDE_DATA_indication.Nsdu;
        for (i=0; i<params.NLDE_DATA_indication.NsduLength; i++)
        {
            *fragmentData++ = *ptr++;
        }

        APSDiscardRx();

        apsFragmentationInfo.IsThisBlockRetried             = 0;
        apsFragmentationInfo.RetryCounter                   = 0;
        apsFragmentationInfo.InterframeDelayTick            = TickGet();
        apsFragmentationInfo.FragmentationStartTime         = TickGet();
        apsFragmentationInfo.TotalFragmentationTime.Val     =
			( ( apsFragmentationInfo.TotalNumberOfBlocks ) * apscAckWaitDuration * ( apscMaxFrameRetries + 1 ) );

        apsFragmentationInfo.WaitingForAck                  = 0;

        // If only one block in the window.
        if (apsFragmentationInfo.MissedBlockInfo.AckBitField == 0xFF)
        {
            currentBlockNumberAck = apsFragmentationInfo.BlockNumberToTxOrRx;
            currentAckBitField = apsFragmentationInfo.MissedBlockInfo.AckBitField;

            apsFragmentationInfo.WindowSize++;
            apsFragmentationInfo.BlockNumberToTxOrRx++;
            apsFragmentationInfo.FirstBlockInWindow = apsFragmentationInfo.BlockNumberToTxOrRx;
            apsFragmentationInfo.LastBlockInWindow  = ( fragmentWindowSize * apsFragmentationInfo.WindowSize ) - 1;

            return SendAPSACKForFragmentedData (currentBlockNumberAck, currentAckBitField);
        }

        return NO_PRIMITIVE;
    }
    if ( fragmentType & APS_EXTENDED_HDR_PART_OF_FRAGMENT )
    {
        // If we received fragment packet without receiving the first block, then drop the frame.
        // If we received fragment packet from different device, then drop the frame.
        if ( ( !apsStatus.flags.bits.bFragmentationInProgress ) ||
             ( apsFragmentationInfo.FragmentDstAddr.ShortAddr.Val != params.NLDE_DATA_indication.SrcAddress.Val ) )
        {
            APSDiscardRx();
            return NO_PRIMITIVE;
        }

        fragmentData = apsFragmentationInfo.DataPayloadAddress;
        fragmentData = fragmentData + ( blockNumber * apsFragmentationInfo.PreviousDataPayloadLength );

        ptr = params.NLDE_DATA_indication.Nsdu;

        for (i=0; i<params.NLDE_DATA_indication.NsduLength; i++)
        {
            *fragmentData++ = *ptr++;
        }

        APSDiscardRx();

        // If received blockNumber is the first block in the window. (This can happen from Second Window onwards)
        if ( apsFragmentationInfo.FirstBlockInWindow == blockNumber )
        {
            // In case where fragmentWindowSize is 1, FirstBlock and LastBlock will be same for any window.
            if ( apsFragmentationInfo.LastBlockInWindow == blockNumber )
            {
                apsFragmentationInfo.MissedBlockInfo.AckBitField
                    = ( 0xFF << ( ( apsFragmentationInfo.LastBlockInWindow - apsFragmentationInfo.FirstBlockInWindow ) + 1 ) ) | 0x01;
            }
            else
            {
                // First block can be retried from Second Window onwards
                if ( !apsFragmentationInfo.IsThisBlockRetried )
                {
                    if ( ( apsFragmentationInfo.LastBlockInWindow == ( apsFragmentationInfo.TotalNumberOfBlocks - 1 ) ) &&
                         ( ( apsFragmentationInfo.LastBlockInWindow - apsFragmentationInfo.FirstBlockInWindow ) != fragmentWindowSize ) )
                    {
                        apsFragmentationInfo.MissedBlockInfo.AckBitField
                                = ( 0xFF << ( ( apsFragmentationInfo.LastBlockInWindow - apsFragmentationInfo.FirstBlockInWindow ) + 1 ) ) | 0x01;
                    }
                    else
                    {
                        apsFragmentationInfo.MissedBlockInfo.AckBitField = ( 0xFF << fragmentWindowSize ) | 0x01;
                    }
                }
            }
        }

        // If any of intermediate blocks have been recived.
        AckBlockBits = 0x01 << ( blockNumber - apsFragmentationInfo.FirstBlockInWindow );
        apsFragmentationInfo.MissedBlockInfo.AckBitField = apsFragmentationInfo.MissedBlockInfo.AckBitField | AckBlockBits;

        // Last block in the window have been received. Time to send the APS ACK
        if ( blockNumber == apsFragmentationInfo.LastBlockInWindow )
        {
            currentAckBitField = apsFragmentationInfo.MissedBlockInfo.AckBitField;

            if ( apsFragmentationInfo.MissedBlockInfo.AckBitField == 0xFF )
            {
                currentBlockNumberAck = apsFragmentationInfo.FirstBlockInWindow;

                // Check is this the last block. If yes, then send indication
                if ( blockNumber == ( apsFragmentationInfo.TotalNumberOfBlocks - 1 ) )
                {
                    apsStatus.flags.bits.bPendingFragmentDataIndication = 1;
                    apsFragmentationInfo.AllFragmentsReceived = 1;
                    if ( ( apsFragmentationInfo.DataPayloadLength / apsFragmentationInfo.TotalNumberOfBlocks )
                            != params.NLDE_DATA_indication.NsduLength )
                    {
                        apsFragmentationInfo.DataPayloadLength =
                            apsFragmentationInfo.DataPayloadLength - ( apsFragmentationInfo.PreviousDataPayloadLength - params.NLDE_DATA_indication.NsduLength );
                    }
                }
                // Otherwise, move on to next window.
                else
                {
                    apsFragmentationInfo.WindowSize++;
                    apsFragmentationInfo.BlockNumberToTxOrRx = apsFragmentationInfo.LastBlockInWindow + 1;
                    apsFragmentationInfo.FirstBlockInWindow = apsFragmentationInfo.BlockNumberToTxOrRx;
                    apsFragmentationInfo.LastBlockInWindow  = ( fragmentWindowSize * apsFragmentationInfo.WindowSize ) - 1;
                    if ( apsFragmentationInfo.LastBlockInWindow > ( apsFragmentationInfo.TotalNumberOfBlocks - 1 ) )
                    {
                        apsFragmentationInfo.LastBlockInWindow = ( apsFragmentationInfo.TotalNumberOfBlocks - 1 );
                        apsFragmentationInfo.MissedBlockInfo.AckBitField
                                = 0xFF << ( ( apsFragmentationInfo.LastBlockInWindow - apsFragmentationInfo.FirstBlockInWindow ) + 1 );
                    }
                    else
                    {
                        apsFragmentationInfo.MissedBlockInfo.AckBitField = ( 0xFF << fragmentWindowSize );
                    }
                }
                // Send Acknowledgment
            }
            else
            {
                // Some of the blocks have been missed.
                apsFragmentationInfo.IsThisBlockRetried = 1;

                // This part is for Ember GU Implementation. Other GUs as well?
                // If other GUs not implemented like this, then we have to put
                // currentBlockNumberAck = blockNumber; insted of the following line.
                //-------------------------------------------------------------------
                currentBlockNumberAck = apsFragmentationInfo.FirstBlockInWindow;
                //-------------------------------------------------------------------
                // Send Acknowledgment
            }
            return SendAPSACKForFragmentedData (currentBlockNumberAck, currentAckBitField);
        }
        // Intermediate block has been received.
        else
        {
            // Check whether the block has been retried.
            if ( apsFragmentationInfo.IsThisBlockRetried )
            {
                // This part is for other GU Implementation
                //-------------------------------------------------------------------
                //currentBlockNumberAck = blockNumber;
                //currentAckBitField = 0xFF;
                //-------------------------------------------------------------------

                // This part is for Ember GU Implementation
                //-------------------------------------------------------------------
                currentBlockNumberAck = apsFragmentationInfo.FirstBlockInWindow;
                currentAckBitField = apsFragmentationInfo.MissedBlockInfo.AckBitField;
                //-------------------------------------------------------------------

                // Check whether all the blocks have been received.
                if ( apsFragmentationInfo.MissedBlockInfo.AckBitField == 0xFF )
                {
                    apsFragmentationInfo.IsThisBlockRetried = 0;

                    // Send indication, if all the blocks in the last window have been received.
                    if  ( ( apsFragmentationInfo.LastBlockInWindow == ( apsFragmentationInfo.TotalNumberOfBlocks - 1 ) ) &&
                          ( apsFragmentationInfo.MissedBlockInfo.AckBitField  == 0xFF ) )
                    {
                        apsStatus.flags.bits.bPendingFragmentDataIndication = 1;
                        apsFragmentationInfo.AllFragmentsReceived = 1;
                        if ( ( apsFragmentationInfo.DataPayloadLength / apsFragmentationInfo.TotalNumberOfBlocks )
                                != params.NLDE_DATA_indication.NsduLength )
                        {
                            apsFragmentationInfo.DataPayloadLength =
                                apsFragmentationInfo.DataPayloadLength - ( apsFragmentationInfo.PreviousDataPayloadLength - params.NLDE_DATA_indication.NsduLength );
                        }
                    }
                    // Otherwise, move on to next window.
                    else
                    {
                        apsFragmentationInfo.WindowSize++;
                        apsFragmentationInfo.BlockNumberToTxOrRx = apsFragmentationInfo.LastBlockInWindow + 1;
                        apsFragmentationInfo.FirstBlockInWindow = apsFragmentationInfo.BlockNumberToTxOrRx;
                        apsFragmentationInfo.LastBlockInWindow  =
                                    ( fragmentWindowSize * apsFragmentationInfo.WindowSize ) - 1;
                        if ( apsFragmentationInfo.LastBlockInWindow > ( apsFragmentationInfo.TotalNumberOfBlocks - 1 ) )
                        {
                            apsFragmentationInfo.LastBlockInWindow = ( apsFragmentationInfo.TotalNumberOfBlocks - 1 );
                            apsFragmentationInfo.MissedBlockInfo.AckBitField
                                    = 0xFF << ( apsFragmentationInfo.LastBlockInWindow - apsFragmentationInfo.FirstBlockInWindow );
                        }
                    }

                    if ( apsStatus.flags.bits.bPendingFragmentDataIndication )
                    {
                        apsFragmentationInfo.AllFragmentsReceived = 1;
                    }
                    // Send Acknowledgment
                }
                else
                {
                    // Still some more blocks needs to be recieved
                    // Send Acknowledgment
                }
                return SendAPSACKForFragmentedData (currentBlockNumberAck, currentAckBitField);
            }
            else
            {
                // Update the next block to be received.
                apsFragmentationInfo.BlockNumberToTxOrRx++;
            }
        }
    }
    return NO_PRIMITIVE;
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE SendAPSACKForFragmentedData (BYTE blockNumber, BYTE ackBitField)
 *
 * PreCondition:    None
 *
 * Input:           blockNumber - BlockNumber to be put in the APS ACK Extended Header field
 *                  ackBitField - AckBitField to be put in the APS ACK Extended Header field
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function sends the APS ACK for the Fragmentation
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
ZIGBEE_PRIMITIVE SendAPSACKForFragmentedData (BYTE blockNumber, BYTE ackBitField)
{
    SHORT_ADDR          dstAddr;
    APS_FRAME_CONTROL   apsFragmentFrameControl;

    dstAddr = apsFragmentationInfo.FragmentDstAddr.ShortAddr;

    // Update NLDE_DATA_request params.
    params.NLDE_DATA_request.NsduLength         = 0;
    params.NLDE_DATA_request.DstAddrMode        = APS_ADDRESS_16_BIT;
    params.NLDE_DATA_request.DstAddr            = apsFragmentationInfo.FragmentDstAddr.ShortAddr;
    params.NLDE_DATA_request.BroadcastRadius    = DEFAULT_RADIUS;
    params.NLDE_DATA_request.DiscoverRoute      = ROUTE_DISCOVERY_ENABLE;
    params.NLDE_DATA_request.NsduHandle         = NLME_GET_nwkBCSN();
    params.NLDE_DATA_request.SecurityEnable     = FALSE;

    apsFragmentFrameControl.bits.frameType          = APS_FRAME_ACKNOWLEDGE;
    apsFragmentFrameControl.bits.deliveryMode       = APS_DELIVERY_DIRECT;
    apsFragmentFrameControl.bits.ackFormat          = APS_ACK_FOR_DATA;
    apsFragmentFrameControl.bits.acknowledgeRequest = APS_ACK_NOT_REQUESTED;
    apsFragmentFrameControl.bits.extHeaderPresent   = APS_EXTENDED_HEADER_INCLUDED;

    if ( apsFragmentationInfo.FragmentTxOptions.bits.securityEnabled )
    {
        apsFragmentFrameControl.bits.security           = APS_SECURITY_ON;

        #if I_SUPPORT_LINK_KEY == 1
            // Load the APS Header (backwards).
            // Load APS Extended Header Fields
            TxBuffer[TxHeader--] = ackBitField;
            TxBuffer[TxHeader--] = blockNumber;
            TxBuffer[TxHeader--] = APS_EXTENDED_HDR_PART_OF_FRAGMENT;

            // Load other APS Header Fields
            TxBuffer[TxHeader--] = apsFragmentationInfo.APSCounter;
            TxBuffer[TxHeader--] = apsFragmentationInfo.DstEndpoint;
            TxBuffer[TxHeader--] = apsFragmentationInfo.ProfileId.byte.HB;
            TxBuffer[TxHeader--] = apsFragmentationInfo.ProfileId.byte.LB;
            TxBuffer[TxHeader--] = apsFragmentationInfo.ClusterId.byte.HB;
            TxBuffer[TxHeader--] = apsFragmentationInfo.ClusterId.byte.LB;
            TxBuffer[TxHeader--] = apsFragmentationInfo.SrcEndpoint;
            TxBuffer[TxHeader--] = apsFragmentFrameControl.Val;

            if ( ApplyAPSSecurityToAPSACK() )
            {
                #if defined(I_SUPPORT_SECURITY) && PROFILE_nwkSecureAllFrames
                    /*APS ACK may also be encrypted using NWK Key, if
                    SecureAllFrames is set to TRUE.*/
                    params.NLDE_DATA_request.SecurityEnable = TRUE;
                #endif
            }
            else
            {
                ZigBeeUnblockTx();
                return NO_PRIMITIVE;
            }
        #else
            ZigBeeUnblockTx();
            return NO_PRIMITIVE;
        #endif
    }
    else
    {
        apsFragmentFrameControl.bits.security           = APS_SECURITY_OFF;

        #if defined(I_SUPPORT_SECURITY) && PROFILE_nwkSecureAllFrames
            params.NLDE_DATA_request.SecurityEnable = TRUE;

            TxBuffer[TxData++] = apsFragmentFrameControl.Val;
            TxBuffer[TxData++] = apsFragmentationInfo.SrcEndpoint;
            TxBuffer[TxData++] = apsFragmentationInfo.ClusterId.byte.LB;
            TxBuffer[TxData++] = apsFragmentationInfo.ClusterId.byte.HB;
            TxBuffer[TxData++] = apsFragmentationInfo.ProfileId.byte.LB;
            TxBuffer[TxData++] = apsFragmentationInfo.ProfileId.byte.HB;
            TxBuffer[TxData++] = apsFragmentationInfo.DstEndpoint;
            TxBuffer[TxData++] = apsFragmentationInfo.APSCounter;

            // Load APS Extended Header Fields
            TxBuffer[TxData++] = APS_EXTENDED_HDR_PART_OF_FRAGMENT;
            TxBuffer[TxData++] = blockNumber;
            TxBuffer[TxData++] = ackBitField;

        #else
            params.NLDE_DATA_request.SecurityEnable = FALSE;

            // Load the APS Header (backwards).
            // Load APS Extended Header Fields
            TxBuffer[TxHeader--] = ackBitField;
            TxBuffer[TxHeader--] = blockNumber;
            TxBuffer[TxHeader--] = APS_EXTENDED_HDR_PART_OF_FRAGMENT;

            // Load other APS Header Fields
            TxBuffer[TxHeader--] = apsFragmentationInfo.APSCounter;
            TxBuffer[TxHeader--] = apsFragmentationInfo.DstEndpoint;
            TxBuffer[TxHeader--] = apsFragmentationInfo.ProfileId.byte.HB;
            TxBuffer[TxHeader--] = apsFragmentationInfo.ProfileId.byte.LB;
            TxBuffer[TxHeader--] = apsFragmentationInfo.ClusterId.byte.HB;
            TxBuffer[TxHeader--] = apsFragmentationInfo.ClusterId.byte.LB;
            TxBuffer[TxHeader--] = apsFragmentationInfo.SrcEndpoint;
            TxBuffer[TxHeader--] = apsFragmentFrameControl.Val;

        #endif
    }

    ZigBeeBlockTx();
    return NLDE_DATA_request;
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/*********************************************************************
 * Function:        BOOL SearchForLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails, BYTE encryption)
 *
 * PreCondition:    None
 *
 * Input:           pappLinkKeyDetails - pointer to structure APS_KEY_PAIR_DESCRIPTOR.
 *                                       This pointer should be filled with the extended
 *                                       address of partner.
 *                  encryption - If the value of this variable is set to 0x01 (means this function is called
 *                               to get the Link Key of the partner device for encryption purpose). If the value of
 *                               this variable set to 0x00 (means this function is called to get the Link Key
 *                               of the partner device for decryption purpose).
 *                               FrameCounter should be incremented only for the encryption purpose. For decryption,
 *                               FrameCounter should be assigned to value as in the received frame.
 *
 * Output:          Returns true if the link key with the partner exists, otherwise
 *                  returns false. The link key and the frame countetr will be updated in
 *                  structure pappLinkKeyDetails if it exists. FrameCounter will also be updated onto NVM based
 *                  on the encryption/decryption type.
 *
 * Side Effects:    None
 *
 * Overview:        This function searches whether the application link
 *                  key exists with the given partner address. If it exists
 *                  the structure pointer will be updated with the key and the
 *                  frame counter.
 *
 * Note:            None
 ********************************************************************/
#if I_SUPPORT_LINK_KEY == 1
BOOL SearchForLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails, BYTE encryption)
{
    BYTE    index;
    BYTE i,j;
    LONG_ADDR   tempAddr;
    APS_KEY_PAIR_DESCRIPTOR currentKey;
    WORD pCurrentKey;

    pCurrentKey = appLinkKeyTable;

    /* Reverse copy the partner address */
    for( i =0;i < 8; i++ )
    {
        tempAddr.v[i] = pappLinkKeyDetails->PartnerAddress.v[7-i];
    }

    /*Serch in the link key table whether key exists */
    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey+=(WORD)sizeof(APS_KEY_PAIR_DESCRIPTOR), index++)
    #else
    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey++, index++)
    #endif
    {
        // Read the record into RAM.
        GetLinkKey(&currentKey, pCurrentKey );
        if ( currentKey.InUse == TRUE )
        {
            if ( !memcmp ( &tempAddr, &currentKey.PartnerAddress, 8 ) )
            {
                /* Key exists, copy the link key and frame counter */
                for( j =0; j < KEY_LENGTH; j++ )
                {
                    pappLinkKeyDetails->Link_Key.v[j] = currentKey.Link_Key.v[j];
                }
                /*FrameCounter should be incremented only for the encryption purpose. For decryption,
                FrameCounter should be assigned to value as in the received frame.*/
                if ( encryption )
                {
                    pappLinkKeyDetails->IncomingFrameCounter =  currentKey.IncomingFrameCounter;
                    currentKey.IncomingFrameCounter.Val++;
                }
                else
                {
                    currentKey.IncomingFrameCounter = pappLinkKeyDetails->IncomingFrameCounter;
                }
                PutLinkKey(pCurrentKey, &currentKey);
                return TRUE;
            }
        }
    }
    /* key does not exists */
    return FALSE;
 }

/*********************************************************************
 * Function:        void FormatTxBuffer( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function formats TxBuffer by copying the header
 * at the beginning after shifting data at the beginning.
 *
 * Note:            None
 ********************************************************************/
void FormatTxBuffer( void )
{
    BYTE dataLen;
    BYTE *dataBackUp;
    BYTE *ptr;
    dataLen = TxData;
    TxData = 0;

    dataBackUp =  ( BYTE*)SRAMalloc( dataLen );

    if(dataBackUp != NULL)
    {
         memcpy( (void *)dataBackUp, (void *)&TxBuffer, dataLen);
         ptr = dataBackUp;

         // Copy APS header
         while (TxHeader < TX_HEADER_START)
         {
             TxBuffer[TxData++] = TxBuffer[++TxHeader];
         }
         while (dataLen--)
         {
             TxBuffer[TxData++]  = *ptr++;
         }

        nfree(dataBackUp);
    }
}

#endif
/*********************************************************************
 * Function:        void initAPPLinkKeyTable()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to initailize the application
 * link key table.
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_LINK_KEY == 0x01 )
void initAPPLinkKeyTable()
{
    BYTE    index;
    WORD pCurrentKey;
    APS_KEY_PAIR_DESCRIPTOR currentKey;

    pCurrentKey = appLinkKeyTable;

    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey+=(WORD)sizeof(APS_KEY_PAIR_DESCRIPTOR), index++)
    #else
    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey++, index++)
    #endif
    {
        // Read the record into RAM.
        GetLinkKey(&currentKey, pCurrentKey );
        currentKey.InUse = 0;
        PutLinkKey( pCurrentKey, &currentKey );
    }
    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
        for(index = 0; index < KEY_LENGTH; index++)
        {
            TCLinkKeyInfo.link_key.v[index] = current_SAS.spas.PreconfiguredLinkKey[index];
        }
        TCLinkKeyInfo.frameCounter.Val = 0x00;

        if(!I_AM_TRUST_CENTER){//#ifndef I_AM_TRUST_CENTER
            TCLinkKeyInfo.trustCenterShortAddr.Val = 0x0000; // Always TC is ZC
            for(index = 0; index < KEY_LENGTH; index++)
            {
                TCLinkKeyInfo.link_key.v[index] = current_SAS.spas.PreconfiguredLinkKey[index];
            }
            for (index = 0; index < 8; index++)
            {
                TCLinkKeyInfo.trustCenterLongAddr.v[index] =
                current_SAS.spas.TrustCenterAddress.v[index];
            }
        }//#endif
    #endif
}
#endif // (I_SUPPORT_LINK_KEY == 0x01 )

#if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
void TunnelTransportKey( LONG_ADDR *deviceAddress, SHORT_ADDR parentShortAddress)
{
    BYTE *apsPayload;
    BYTE payloadLength;
    BYTE i;
    APS_FRAME_CONTROL   apsFrameControl;
    payloadLength = TxData;


    /*Take back up of APS payload */
    apsPayload = (BYTE *)SRAMalloc(payloadLength);
    if( apsPayload != NULL)
    {
        memcpy(apsPayload, TxBuffer, payloadLength);
        TxData = 0;
        apsFrameControl.Val = APS_FRAME_COMMAND;
        apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT;
        apsFrameControl.bits.acknowledgeRequest = 0;
        apsFrameControl.bits.security = FALSE;
        TxBuffer[TxData++] = apsFrameControl.Val;
        TxBuffer[TxData++] = APSCounter++;
        TxBuffer[TxData++] = APS_CMD_TUNNEL;
        for(i = 0; i < 8; i++)
        {
            TxBuffer[TxData++] = deviceAddress->v[i];
        }
        memcpy(&TxBuffer[TxData],apsPayload,payloadLength );
        TxData = TxData + payloadLength;

        params.NLDE_DATA_request.DstAddr.Val = parentShortAddress.Val;
        params.NLDE_DATA_request.NsduHandle = NLME_GET_nwkBCSN();
        params.NLDE_DATA_request.BroadcastRadius    = DEFAULT_RADIUS;
        params.NLDE_DATA_request.DiscoverRoute      = ROUTE_DISCOVERY_ENABLE;
        nfree(apsPayload);
    }
}
#endif

/*********************************************************************
 * Function:        BOOL SearchForTCLinkKey(LONG_ADDR deviceLongAddr, TC_LINK_KEY_TABLE *tcLinkKeyTable)
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
//#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
BOOL SearchForTCLinkKey(LONG_ADDR deviceLongAddr, TC_LINK_KEY_TABLE *tcLinkKeyTable)
{
    BYTE    index;
    BYTE    j;
    TC_LINK_KEY_TABLE currentTCLinkKey;
    WORD pCurrentEntry;

    pCurrentEntry = TCLinkKeyTable;//TODO, why use this variable when there is a parameter?

    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry+=(WORD)sizeof(TC_LINK_KEY_TABLE), index++)
    #else
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry++, index++)
    #endif
    {
        // Read the record into RAM.
        GetTCLinkKey(&currentTCLinkKey, pCurrentEntry );
        if ( currentTCLinkKey.InUse == TRUE )
        {
            if ( !memcmp ( &deviceLongAddr.v[0], &currentTCLinkKey.DeviceAddress.v[0], 8 ) )
            {
                tcLinkKeyTable->InUse = currentTCLinkKey.InUse;
                /* Key exists, copy the link key and frame counter */
                for( j =0; j < KEY_LENGTH; j++ )
                {
                    tcLinkKeyTable->LinkKey.v[j] = currentTCLinkKey.LinkKey.v[j];
                }
                for( j =0; j < 8; j++ )
                {
                    tcLinkKeyTable->DeviceAddress.v[j] = currentTCLinkKey.DeviceAddress.v[j];
                }
                return TRUE;
            }
        }
    }
    /* key does not exists */
    return FALSE;
 }
//#endif

 /*********************************************************************
 * Function:        BOOL AddEntryTCLinkKeyTable(TC_LINK_KEY_TABLE *newTCLinkKeyTableEntry)
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
#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1

BOOL AddEntryTCLinkKeyTable(TC_LINK_KEY_TABLE *newTCLinkKeyTableEntry)

{
    BYTE    index;
    BYTE    j;
    TC_LINK_KEY_TABLE currentTCLinkKey;
    WORD pCurrentEntry;
    BYTE    indexToAdd = 0xFF;
    WORD    indexAddressToAdd = 0xFFFF;
    pCurrentEntry = TCLinkKeyTable;
    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry+=(WORD)sizeof(TC_LINK_KEY_TABLE), index++)
    #else
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry++, index++)
    #endif
    {
        // Read the record into RAM.
        GetTCLinkKey(&currentTCLinkKey, pCurrentEntry );
        if ( currentTCLinkKey.InUse == TRUE )
        {
            if ( !memcmp ( &newTCLinkKeyTableEntry->DeviceAddress.v[0], &currentTCLinkKey.DeviceAddress.v[0], 8 ) )
            {
                indexToAdd = index;
                indexAddressToAdd = pCurrentEntry;
                break;
            }
        }
        else
        {
            if ( indexToAdd == 0xFF )
            {
                indexToAdd = index;
                indexAddressToAdd = pCurrentEntry;
            }
        }
    }
    if ( indexToAdd < MAX_TC_LINK_KEY_SUPPORTED )
    {
        //pCurrentEntry = TCLinkKeyTable + indexAddressToAdd;
        GetTCLinkKey(&currentTCLinkKey, indexAddressToAdd);
        currentTCLinkKey.InUse = TRUE;
        for( j =0; j < 8; j++ )
        {
            currentTCLinkKey.DeviceAddress.v[j] = newTCLinkKeyTableEntry->DeviceAddress.v[j];
        }
        for( j =0; j < KEY_LENGTH; j++ )
        {
            currentTCLinkKey.LinkKey.v[j] = newTCLinkKeyTableEntry->LinkKey.v[j];
        }
        PutTCLinkKey(indexAddressToAdd, &currentTCLinkKey);
        return TRUE;
    }
    return FALSE;
 }

 #endif

 /*********************************************************************
 * Function:        BOOL RemoveEntryTCLinkKeyTable( BYTE *pDeviceAddress )
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
#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
BOOL RemoveEntryTCLinkKeyTable( BYTE *pDeviceAddress )
{
    BYTE    index;
    BYTE    j;
    TC_LINK_KEY_TABLE currentTCLinkKey;
    WORD pCurrentEntry;

    pCurrentEntry = TCLinkKeyTable;

    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry+=(WORD)sizeof(TC_LINK_KEY_TABLE), index++)
    #else
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry++, index++)
    #endif
    {
        // Read the record into RAM.
        GetTCLinkKey(&currentTCLinkKey, pCurrentEntry );
        if ( currentTCLinkKey.InUse == TRUE )
        {
            if ( !memcmp ( &pDeviceAddress[0], &currentTCLinkKey.DeviceAddress.v[0], 8 ) )
            {
                currentTCLinkKey.InUse = FALSE;
                for( j =0; j < 8; j++ )
                {
                    currentTCLinkKey.DeviceAddress.v[j] = 0xFF;
                }
                for( j =0; j < KEY_LENGTH; j++ )
                {
                    currentTCLinkKey.LinkKey.v[j] = 0xFF;
                }
                PutTCLinkKey(pCurrentEntry, &currentTCLinkKey);
                return TRUE;
            }
        }
    }
    return FALSE;
 }
 #endif

 /*********************************************************************
 * Function:        void ResetTCLinkKeyTable(void)
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
#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
void ResetTCLinkKeyTable(void)
{
    BYTE    index;
    BYTE    j;
    TC_LINK_KEY_TABLE currentTCLinkKey;
    WORD pCurrentEntry;

    pCurrentEntry = TCLinkKeyTable;

    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry+=(WORD)sizeof(TC_LINK_KEY_TABLE), index++)
    #else
    for (index = 0; (index < MAX_TC_LINK_KEY_SUPPORTED) ; pCurrentEntry++, index++)
    #endif
    {
        // Read the record into RAM.
        GetTCLinkKey(&currentTCLinkKey, pCurrentEntry );
        currentTCLinkKey.InUse = FALSE;
        for( j =0; j < 8; j++ )
        {
            currentTCLinkKey.DeviceAddress.v[j] = 0xFF;
        }
        for( j =0; j < KEY_LENGTH; j++ )
        {
            currentTCLinkKey.LinkKey.v[j] = 0xFF;
        }
        PutTCLinkKey(pCurrentEntry, &currentTCLinkKey);
    }
 }
 #endif


/*********************************************************************
 * Function:        BYTE ApplyAPSSecurityToAPSACK( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function applies Link Key Security to APS ACK
 *
 * Note:            None
 ********************************************************************/
#if I_SUPPORT_LINK_KEY == 1
BYTE ApplyAPSSecurityToAPSACK( void )
{
    KEY_INFO    linkKeyDetails;
    BYTE        *linkKey;
    KEY_VAL     linkKeyToBeUsed;

    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
            TC_LINK_KEY_TABLE   tcLinkKeyTable;
            LONG_ADDR           dstLongAddrReverse;
        #endif
    #endif

    //#ifdef I_AM_TRUST_CENTER
        LONG_ADDR dstLongAddress;
        #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
            LONG_ADDR   dstLongAddrReverse;
            LONG_ADDR   SrcAddr;
            BYTE k;
        #endif
	if(I_AM_TRUST_CENTER){
        if(( IsThisAddressKnown ( params.NLDE_DATA_request.DstAddr, &dstLongAddress.v[0]))
                                  && (params.NLDE_DATA_request.DstAddr.Val < 0xFFFB ))
        {
            #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                GetMACAddress(&SrcAddr);
                for(k=0; k < 8 ; k++)
                {
                    dstLongAddrReverse.v[k] = dstLongAddress.v[7 - k];
                }
                GetHashKey(&dstLongAddrReverse, &SrcAddr, &linkKeyToBeUsed);
            #endif

            #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                BYTE count;
                #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                    for(count = 0; count < 8; count++)
                    {
                        dstLongAddrReverse.v[count] = dstLongAddress.v[7 - count];
                    }
                    if ( SearchForTCLinkKey(dstLongAddrReverse, &tcLinkKeyTable) )
                    {
                        for (count = 0; count < KEY_LENGTH; count++)
                        {
                            linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                        }
                    }
                    else
                    {
                        ZigBeeUnblockTx();
                        return FALSE;
                    }
                #else
                    for(count = 0; count < KEY_LENGTH; count++)
                    {
                        linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                    }
                #endif
            #endif

            linkKey = &linkKeyToBeUsed.v[0];
            linkKeyDetails.frameCounter = TCLinkKeyInfo.frameCounter;
            TCLinkKeyInfo.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val +1;
            apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
        }
        else
        {
            linkKey = NULL;
        }
    }else{//#else
        if( TCLinkKeyInfo.trustCenterShortAddr.Val
            == params.NLDE_DATA_request.DstAddr.Val)
        {
            BYTE count;
            #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                    for(count = 0; count < 8; count++)
                    {
                        dstLongAddrReverse.v[count] = TCLinkKeyInfo.trustCenterLongAddr.v[count];
                    }
                    if ( SearchForTCLinkKey(dstLongAddrReverse, &tcLinkKeyTable) )
                    {
                        for (count = 0; count < KEY_LENGTH; count++)
                        {
                            linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                        }
                    }
                    else
                    {
                        ZigBeeUnblockTx();
                        return FALSE;
                    }
                #else
                    for(count = 0; count < KEY_LENGTH; count++)
                    {
                        linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                    }
                #endif
            #else
                for(count = 0; count < KEY_LENGTH; count++)
                {
                    linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                }
            #endif

            linkKeyDetails.frameCounter = TCLinkKeyInfo.frameCounter;
            linkKey = &linkKeyToBeUsed.v[0];
            TCLinkKeyInfo.frameCounter.Val = TCLinkKeyInfo.frameCounter.Val +1;
            apsStatus.flags.bits.bSaveOutgoingFrameCounter = 1;
        }
        else
        {
            LONG_ADDR dstLongAddress;
            if((IsThisAddressKnown ( params.NLDE_DATA_request.DstAddr, &dstLongAddress.v[0]))
                                     && (params.NLDE_DATA_request.DstAddr.Val < 0xFFFB ))
            {
                APS_KEY_PAIR_DESCRIPTOR appLinkKeyDetails;
                BYTE count;
                for(count=0; count < 8; count++)
                {
                    appLinkKeyDetails.PartnerAddress.v[count] = dstLongAddress.v[count];
                }
                /*0x01 means the Link Key is used for Encryption. In this case,
                the IncomingFrameCounter should be incremented and stored onto NVM*/
                if( SearchForLinkKey(&appLinkKeyDetails, 0x01) )
                {
                    linkKey = &appLinkKeyDetails.Link_Key.v[0];
                    linkKeyDetails.frameCounter = appLinkKeyDetails.IncomingFrameCounter;
                }
                else
                {
                    linkKey = NULL;
                }
            }
            else
            {
                linkKey = NULL;
            }
        }
    }//#endif     // #ifdef I_AM_TRUST_CENTER

    if(linkKey != NULL)
    {
        linkKeyDetails.key = linkKey;
        linkKeyDetails.keyId = ID_LinkKey;
        if (
            !DataEncrypt
            (
                TxBuffer,
                &TxData,
                &(TxBuffer[TxHeader+1]),
                (TX_HEADER_START-TxHeader),
                &linkKeyDetails, FALSE
            )
           )
        {
            ZigBeeUnblockTx();
            return FALSE;
        }
        else
        {
            FormatTxBuffer();
        }
    }
    else
    {
        ZigBeeUnblockTx();
        return FALSE;
    }
    return TRUE;
}
#endif //I_SUPPORT_LINK_KEY == 1


/*********************************************************************
 * Function:        WORD GetGroupByIndex ( BYTE index )
 *
 * PreCondition:    None
 *
 * Input:           index  - the index to the group table entry.
 *
 * Output:          None
 *
 * Return :         Return the group id for the passed index.
 *
 * Side Effects:    None
 *
 * Overview:        This function gives the group id for the given index. 
 *
 * Note:            None
 ********************************************************************/

#if defined(I_SUPPORT_GROUP_ADDRESSING)

WORD GetGroupByIndex ( BYTE index )
{
	/* Check if valid group index */
	if ( index >= MAX_GROUP )
		return ( WORD )0xFFFF;

	 #ifdef USE_EXTERNAL_NVM
		 /* Get the address of the group entry from the NVM */
	     pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(index) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
	 #else
	     pCurrentGroupAddressRecord = &apsGroupAddressTable[index];
	 #endif
	 GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
	
	/* Return the group id */
    return (currentGroupAddressRecord.GroupAddress.Val);
}

#endif /*I_SUPPORT_GROUP_ADDRESSING*/


/*********************************************************************
 * Function:        BYTE VerifyGroupAddrAndEndPoint (
 *						SHORT_ADDR GroupAddr,
 *    					BYTE endPointId,
 *    					BYTE* pIndex
 *					)
 *
 * PreCondition:    None
 *
 * Input:    		GroupAddr - Group address for which the Group entry is to be checked
			       	endPointId  - EndPoint for which the Group entry is to be checked
 *
 * Output:          pIndex - Pointer to the index of the group entry in the apsGroupAddressTable.
 * Return :         GROUP_ENTRY_FOUND - Group entry is found in the table.
 *					GROUP_ENTRY_NOT_FOUND - Group entry is not found in the table.
 * Side Effects:    None
 *
 * Overview:        This function checks for the particular endpoint whether group entry is already
 *                 	present in the apsGroupAddressTable or not.
 .
 ********************************************************************/

#if defined(I_SUPPORT_GROUP_ADDRESSING)

BYTE VerifyGroupAddrAndEndPoint ( SHORT_ADDR GroupAddr, BYTE endPoint, BYTE* pIndex )
{
	BYTE i;			
	/* Get the index of the apsGroupAddressTable */
	*pIndex = GetEndPointsFromGroup ( GroupAddr );

	/* check if group addr is present in the group table or not */
	if ( MAX_GROUP == *pIndex)
	{
		return GROUP_ENTRY_NOT_FOUND;
	}

	#ifdef USE_EXTERNAL_NVM
	 /* Get the address of the group entry from the NVM */
	    pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(*pIndex) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
	#else
	    pCurrentGroupAddressRecord = &apsGroupAddressTable[*pIndex];
	#endif
	GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
	
	/* loop through all the end points for the group entry */
	for ( i =0; i < MAX_GROUP_END_POINT ; i++ )
	{
		/* check if endpoint is matching or not */
		if ( endPoint == currentGroupAddressRecord.EndPoint[i])
		{
			break;
		}
	}
	/* Check if endpoint is there in the group entry */
	if ( MAX_GROUP_END_POINT == i) 
	{
		return GROUP_ENTRY_NOT_FOUND;
	}
	
	return GROUP_ENTRY_FOUND;
}

#endif /*I_SUPPORT_GROUP_ADDRESSING*/


