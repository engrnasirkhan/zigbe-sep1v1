/*********************************************************************
 *
 *                  ZigBee ZDO Layer
 *
 *********************************************************************
 * FileName:        zZDO.c
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
#include "MSDCL_Commissioning.h"

// ******************************************************************************
// Constant Definitions

#define MAX_APS_PACKET_SIZE                 (127-11-8-6-2)  // Max size - MAC, NWK, APS, and AF headers
#define MAX_LONGADDS_TO_SEND                ((MAX_APS_PACKET_SIZE-13)/8)
#define MAX_SHORTADDS_TO_SEND               ((MAX_APS_PACKET_SIZE-13)/2)
#define MSG_HEADER_SIZE                     1

#define BIND_SOURCE_MASK            0x02
#define BIND_FROM_UPPER_LAYERS      0x02
#define UNBIND_FROM_UPPER_LAYERS    0x02
#define BIND_FROM_EXTERNAL          0x00
#define UNBIND_FROM_EXTERNAL        0x00

#define BIND_DIRECTION_MASK         0x01
#define BIND_NOT_DETERMINED         0x02    // NOTE - does not need mask
#define BIND_NODES                  0x01
#define UNBIND_NODES                0x00
#define MAXJOINRETRIES              0x04
#define MAXNETDESCSUPPORTED         0x05
#define STARTFROMNVM                0x00
#define NOT_VALID                   0xFF
#ifdef RFD_POLL_RATE		//MSEB
    #define I_SUPPORT_AUTO_POLLING
#endif

extern BYTE MAXDISCOVERYYRETRIES;

#ifdef I_SUPPORT_SECURITY
    #define DEFAULT_TX_OPTIONS          0x02 // NWK KEY ENABLED
#else
    #define DEFAULT_TX_OPTIONS          0x00 // UNSECURED
#endif


#if I_SUPPORT_FREQUENCY_AGILITY == 1
    /* After reception of Mgmt_NWK_Update_Notify, the network manager will wait
       for MGMT_NWK_UPDATE_WAIT(in seconds) for another Mgmt_NWK_Update_Notify
       for resolving frequency agilility. */
    #define MGMT_NWK_UPDATE_WAIT    0x60

    /* Time(in seconds) to wait before switching channel */
    #define SWITCH_CHANNEL_WAIT     0x09

    #define INTER_SCAN_DELAY        0x01
#endif

#if defined(USE_EXTERNAL_NVM) && defined(STORE_MAC_EXTERNAL)
    extern BYTE macLongAddrByte[];
#endif
// ******************************************************************************
// Macros

// Since the APSDE_DATA_indication and ZDO_DATA_indication parameters align,
// we can use the APSGet function.
#define ZDOGet()        APSGet()
#define ZDODiscardRx()  APSDiscardRx()

#if defined(__C30__) || defined(__C32__)
    #define RANDOM_LSB                      (TMRL)
    #define RANDOM_MSB                      (TMRH)
#else
    #define RANDOM_LSB                      (TMR0L)
    #define RANDOM_MSB                      (TMR0H)
#endif
// ******************************************************************************
// Data Structures

#define ONE_MILLISECOND                     ONE_SECOND/1000     //Intepretation for one millisecond

unsigned char sendrequestSource;			//nimish
unsigned char AskDeviceAddress = 0;
LONG_ADDR SendIee;
void MySendAddressRequest( BYTE requestSource,LONG_ADDR IeeeAddr);
extern BOOL CheckDeviceJoiningPermission(LONG_ADDR ieeeAddr);

#if defined(I_SUPPORT_BINDINGS)
typedef struct _BIND_IN_PROGRESS_INFO
{
        LONG_ADDR   sourceAddressLong;
        LONG_ADDR   destinationAddressLong;
        TICK        timeStamp;
        SHORT_ADDR  sourceAddressShort;
        SHORT_ADDR  destinationAddressShort;
        SHORT_ADDR  requestorAddress;
        BYTE        sourceEP;
        WORD_VAL    cluster;
        BYTE        destinationEP;
        BYTE        sequenceNumber;
        union
        {
            struct
            {
                unsigned char    bSourceRequested        : 1;
                unsigned char    bDestinationRequested   : 1;
                unsigned char    bBindNodes              : 1;
                unsigned char    bFromUpperLayers        : 1;
            } bits;
            BYTE    val;
        } status;
} BIND_IN_PROGRESS_INFO;
#endif


#if defined(SUPPORT_END_DEVICE_BINDING)
typedef enum _EB_STATUS
{
    WAIT_FOR_SECOND_BIND = 0x00,
    SEND_TEST_UNBIND_TO_FIRST_DEVICE,
    WAIT_FOR_TEST_UNBIND_RESPONSE_FROM_FIRST_DEVICE,
    SEND_TEST_UNBIND_TO_SECOND_DEVICE,
    WAIT_FOR_TEST_UNBIND_RESPONSE_FROM_SECOND_DEVICE,
    SEND_UNBIND_TO_FIRST_DEVICE,
    WAIT_FOR_UNBIND_RESPONSE_FROM_FIRST_DEVICE,
    SEND_UNBIND_TO_SECOND_DEVICE,
    WAIT_FOR_UNBIND_RESPONSE_FROM_SECOND_DEVICE,
    SEND_BIND_TO_FIRST_DEVICE,
    WAIT_FOR_BIND_RESPONSE_FROM_FIRST_DEVICE,
    SEND_BIND_TO_SECOND_DEVICE,
    WAIT_FOR_BIND_RESPONSE_FROM_SECOND_DEVICE,
    SEND_EB_RESPONSE_TO_FIRST_DEVICE,
    SEND_EB_RESPONSE_TO_SECOND_DEVICE,
    EB_DONE
} EB_STATUS;
typedef struct _END_DEVICE_BIND_request
{

    WORD_VAL    profileID;
    BYTE        endpoint;
    BYTE        numInClusters;
    BYTE        numOutClusters;
    BYTE        *inClusterList;
    BYTE        *outClusterList;
    SHORT_ADDR  deviceShortAddress;
    WORD_VAL        *MatchList;
    BYTE        MatchCount;
    BYTE        status;
    LONG_ADDR   longAddr;
    BYTE sequenceNumber;

} END_DEVICE_BIND_request;
typedef struct _END_DEVICE_BIND_INFO
{
    TICK        lastTick;
     BYTE        BindRspStatus;
     END_DEVICE_BIND_request *pFirstEndDeviceBindReq;
     END_DEVICE_BIND_request *pSecondEndDeviceBindReq;

    EB_STATUS EBstate;
} END_DEVICE_BIND_INFO;
#endif
enum
{
    WAIT_FOR_SECOND_REQUEST_KEY_REQUEST = 0x01,
    PENDING_TRANSPORT_KEY_FOR_FIRST_DEVICE,
    READY_STATE
};

typedef struct _REQUEST_KEY_DETAILS
{
    BYTE state;
    TICK firstRequestTime;
    SHORT_ADDR  FirstRequestInitiatorShortAddress;
    LONG_ADDR FirstRequestInitiatorAddress;
    LONG_ADDR FirstRequestPartnerAddress;
    KEY_VAL GeneratedAPPLinkKey;
}REQUEST_KEY_DETAILS;

typedef struct _MGMT_NWK_UPDATE_REQ_DETAILS
{
    BYTE scanCount;
    DWORD_VAL ScanChannels;
    BYTE ScanDuration;

} MGMT_NWK_UPDATE_REQ_DETAILS;

// ******************************************************************************
// Variable Definitions
#if I_SUPPORT_LINK_KEY == 1
//#ifdef I_AM_TRUST_CENTER
    static REQUEST_KEY_DETAILS *pRequestKeyDetails;
//#endif
#endif
#ifdef I_SUPPORT_BINDINGS
    BIND_IN_PROGRESS_INFO *pBindInProgressInfo;
#endif

#ifdef SUPPORT_END_DEVICE_BINDING
    static END_DEVICE_BIND_INFO *pEndBindRequest;
#endif
BYTE        sequenceNumber;            // Received sequence number, for sending response back
ZDO_STATUS  zdoStatus;
BYTE        ZDOCounter;
SHORT_ADDR dAddr;
#if defined(I_SUPPORT_SECURITY)
    LEAVING_CHILD_DEVICE_DETAILS leavingChildDetails;
    extern APS_ADDRESS_MAP     currentAPSAddress;
    extern SECURITY_STATUS      securityStatus;
    //#ifdef I_AM_RFD
        extern volatile PHY_PENDING_TASKS  PHYTasksPending;
    //#endif
    extern KEY_VAL KeyVal;
    extern BOOL APSSaveAPSAddress( APS_ADDRESS_MAP *AddressMap);
    extern MAC_STATUS macStatus;

    #if defined(USE_EXTERNAL_NVM)
        extern NETWORK_KEY_INFO plainSecurityKey[2];
        extern BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey);
    #else
//      extern ROM LONG_ADDR trustCenterLongAddr;
    #endif
    #define NIB_nwkNetworkBroadcastDeliveryTime  0x02
#endif

extern DWORD_VAL apsChannelMask;
#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 ) || (I_SUPPORT_FREQUENCY_AGILITY == 1)
    extern NWK_STATUS nwkStatus;
#endif

#if I_SUPPORT_FREQUENCY_AGILITY == 1
    extern PHY_PIB phyPIB;
    extern ENERGY_DETECT_RECORD EdRecords[];

    /* The variable will hold the new channel to which network will be switched. */
    BYTE NewChannel = 0xFF;

    extern WORD_VAL TotalTransmittedPackets;
    extern WORD_VAL TotalTransmitFailures;
    extern BYTE InterferenceDetected;

    /* This variable is used for setting the current channel energy. If the value
       of this variable is zero, then the current channel energy will be the
       energy scanned by the device. */
    BYTE CurrentChannelEnergy = 0;


    extern BYTE UpdateNotifyCount;

    /* For setting the acceptable threshold energy,
       so that the network can switch the channel
       while testing*/
    //BYTE MaxEnergyThreshold = MAX_ENERGY_THRESHOLD;
#endif

extern APS_STATUS apsStatus;
extern MAC_STATUS macStatus;
extern BYTE QuickPoll;
extern  BOOL AnnceSent;
int     lcount = 0;
NODE_SIMPLE_DESCRIPTOR  simpleDescriptor;

BYTE AllowJoin = 1;
BYTE NwkJoinTries;
NETWORK_DESCRIPTOR  *NetworkDescriptor;
NETWORK_DESCRIPTOR  *currentNetworkDescriptor;
BYTE NwkDiscoveryTries;
BYTE State = ZDOStateOFF;
BYTE NetworkDescIndex = 0;
NETWORK_DESCRIPTOR NetworkDesc[MAXNETDESCSUPPORTED];
extern DWORD_VAL OutgoingFrameCount[2];
extern DWORD_VAL IncomingFrameCount[2][MAX_NEIGHBORS];
static BYTE tryToRejoin;
//merged #ifndef I_AM_COORDINATOR
    BYTE syncLoss;
//#endif
#ifdef I_SUPPORT_SECURITY
    extern BYTE rejoinWithSec;
#endif

//BYTE ScanCount;
MGMT_NWK_UPDATE_REQ_DETAILS mgmtNwkUpateRequestDetails;

//#if !defined(I_AM_COORDINATOR)
    BYTE joinReqType;
//#endif
#ifdef I_SUPPORT_AUTO_POLLING
    #if defined( __18CXX)
        static TICK startPollingTime = 0x00;
    #else
        static TICK startPollingTime;
    #endif
#endif
#if I_SUPPORT_CONCENTRATOR == 1
    static TICK  ManyToOneRequestTime ;
#endif

#ifdef DEBUG_LOG
    extern BYTE NoOfPANDescriptorFound;
    extern BYTE PotentialParentFound;
#endif
#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
    extern PAN_ID_CONFLICT_STATUS  panIDConflictStatus;
#endif
BYTE startMode;

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
    // Used to send hold to address for which Unsolicited Rejoin Response should be sent
    extern SHORT_ADDR addressConflictOnChildEndDevice;
#endif

BOOL appNotification;
extern APP_NOTIFICATION        appNotify;

// ******************************************************************************
// Function Prototypes

void AppNotification( void );
#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
//#ifndef I_AM_COORDINATOR
    ZIGBEE_PRIMITIVE ResolveLocalAddressConflict( void );
//#endif
#endif
#if I_SUPPORT_CONCENTRATOR == 1
    static ZIGBEE_PRIMITIVE BroadcastManyToOneRequest( void );
#endif
void FinishAddressResponses( WORD clusterID, LONG_ADDR IEEEAddr, SHORT_ADDR nwkAddr, BYTE requestType, BYTE startIndex );
BOOL IsThisMyShortAddr( void );
void PrepareMessageResponse( WORD clusterID );
BOOL handleMatchDescReq(void);
BOOL handleFindNodeCacheReq(void);
void handleActiveEPReq(void);
void handleLQIReq(void);
void handleRTGReq(void);
void DeleteRouteRecord(SHORT_ADDR shortAddress);
//#if defined(MERGED_STACK) || !defined(I_AM_COORDINATOR)
    static ZIGBEE_PRIMITIVE SubmitJoinRequest();
    void JoinRequest( LONG_ADDR nwkDesExtendedPANID, BYTE joinType, BYTE security );
    void AppJoinRequest(  NETWORK_DESCRIPTOR *nwkDescriptor, BYTE numberOfNetworks );
//#endif

void SetBeaconPayload(BOOL);
#ifdef I_SUPPORT_AUTO_POLLING
    static ZIGBEE_PRIMITIVE StartEndDevicePolling( void );
#endif

static void ZigBeeInitSilentStartup();

    /* FFDs can  participate in source binding */

    BOOL ProcessBindAndUnbind( BYTE bindInfo, LONG_ADDR *sourceAddress, BYTE dstAddrMode, ADDR *destinationAddress );


#ifdef I_SUPPORT_BINDINGS
    void SendBindAddressRequest( BYTE requestSource );
    BOOL SendUpBindResult( BYTE status, BYTE bindNodes );
    extern BYTE SentBindRequest;
#endif

#ifdef SUPPORT_END_DEVICE_BINDING
    ZIGBEE_PRIMITIVE ProcessEndDeviceBind( END_DEVICE_BIND_INFO *pRequestInfo );
    ZIGBEE_PRIMITIVE Send_END_DEVICE_BIND_rsp( END_DEVICE_BIND_request *pBindRequest );
     void ClearMemoryAllocatedForEB( void );
     ZIGBEE_PRIMITIVE SendBindOrUnbind(ZDO_CLUSTER cluster, END_DEVICE_BIND_request *bindSource,
 END_DEVICE_BIND_request *bindTarget, BYTE clusterUpdate);
 WORD_VAL *FindMatch( BYTE firstClusterCount, BYTE secondClusterCount,
WORD_VAL * FirstClusterList,WORD_VAL *SecondClusterList, BYTE *matchCount);
#endif
extern BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap);

#ifdef SUPPORT_END_DEVICE_BINDING
    WORD    matchCluster1;
    WORD    matchCluster2;
#endif

#ifdef I_SUPPORT_SECURITY
    extern BOOL    firstKeyHasBeenSent;
#endif

#ifdef I_SUPPORT_GROUP_ADDRESSING
    extern void RemoveAllGroups(void);
#endif
extern BOOL LookupAPSAddress(LONG_ADDR *);

extern ZIGBEE_PRIMITIVE InitiateStartup(void);

#if I_SUPPORT_LINK_KEY == 1
//#ifdef I_AM_TRUST_CENTER
static void BKDRHash(BYTE input, KEY_VAL *hash, BYTE index);
//#endif
#endif
#if I_SUPPORT_FREQUENCY_AGILITY == 1
    static ZIGBEE_PRIMITIVE HandleMgmtNwkUpdateNotify(BOOL LoopBack, BYTE status,
        DWORD_VAL ScannedChannels, WORD_VAL TotalTransmissions,
        WORD_VAL FailedTransmission, BYTE ChannelListCount);
#endif

#if (I_SUPPORT_LINK_KEY == 0x01)
        extern void initAPPLinkKeyTable();
#endif // (I_SUPPORT_LINK_KEY == 0x01)

/*********************************************************************
 * Function:        void InitNVMWithDefaultValues(  void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       All tables like neighbor table, Routing table ,
 * Binding table, Grooup table will be reset. Start Up attribute set will be
 * initialized with default SAS
 *
 * Note:            This function has to be called for a fresh start,
 * ignoring all the  states, tables before the power up. If this
 * function is not called then NVM values will be restored.
 ********************************************************************/
#ifdef USE_EXTERNAL_NVM
void InitNVMWithDefaultValues( void )
{
    BYTE *memBlock;
    WORD validitykey;

    memBlock = SRAMalloc( sizeof(NODE_DESCRIPTOR));
    if ( memBlock != NULL)
    {
        memcpy( memBlock, (void *)&Config_Node_Descriptor, sizeof(NODE_DESCRIPTOR) );
        NVMWrite( configNodeDescriptor, memBlock, sizeof(NODE_DESCRIPTOR) );
        SRAMfree( memBlock );
    }

    #if defined(USE_EXTERNAL_NVM) && defined(STORE_MAC_EXTERNAL)
        GetMACAddressValidityKey(&validitykey);
        if (validitykey != MAC_ADDRESS_VALID)
        {
            PutMACAddress(macLongAddrByte);
            APLEnable();
        }

    #endif

    Initdefault_SAS();
    #ifdef I_SUPPORT_COMMISSIONING
        SetActiveIndex(0x00,0xFF);
    #endif
    NWKClearNeighborTable();

    #ifdef I_SUPPORT_ROUTING
        NWKClearRoutingTable();
    #endif

#if defined(I_SUPPORT_BINDINGS)
    ClearBindingTable();
//  RemoveAllBindings(macPIB.macShortAddress);
#endif
    #if MAX_APS_ADDRESSES > 0
        APSClearAPSAddressTable();
    #endif

#ifdef I_SUPPORT_GROUP_ADDRESSING
    RemoveAllGroups();
#endif
#ifdef I_SUPPORT_SECURITY
    #ifdef I_SUPPORT_RES_SECURITY
    BYTE i;
    i = 1;
    PutNwkActiveKeyNumber(&i);
        #ifdef NETWORK_KEY_BYTE00

            BYTE key[16];
            for ( i = 0 ; i < 16 ; i++)
            {
                key[i] = current_SAS.spas.NetworkKey[i];
            }

            memcpy((void *)&(currentNetworkKeyInfo.NetKey), (void *)key, 16);
            currentNetworkKeyInfo.SeqNumber.v[0] = current_SAS.spas.NetworkKeySeqNum;
            currentNetworkKeyInfo.SeqNumber.v[1] = nwkMAGICResSeq;
            #ifdef USE_EXTERNAL_NVM
                plainSecurityKey[0] = currentNetworkKeyInfo;
                SetSecurityKey(0, currentNetworkKeyInfo);
                plainSecurityKey[1].SeqNumber.v[1] = 0xFF; // disable key 2
            #else
                PutNwkKeyInfo( &networkKeyInfo , &currentNetworkKeyInfo );
            #endif
        #endif          /* end NETWORK_KEY_BYTE00 */


        
            OutgoingFrameCount[0].Val = 0;
            OutgoingFrameCount[1].Val = 0;
        PutOutgoingFrameCount(OutgoingFrameCount);
            for(i = 0; i < MAX_NEIGHBORS; i++)
            {
                IncomingFrameCount[0][i].Val = 0;
                IncomingFrameCount[1][i].Val = 0;
            }
    #endif //I_SUPPORT_RES_SECURITY
	#endif // I_SUPPORT_SECURITY

    /* Reset the frequency agility related global variables. */
    currentNeighborTableInfo.nwkManagerAddr.Val = 0x0000;
    PutNeighborTableInfo();
    #if (I_SUPPORT_LINK_KEY == 0x01)
        initAPPLinkKeyTable();
        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
            ResetTCLinkKeyTable();
        #endif
    #endif // (I_SUPPORT_LINK_KEY == 0x01)

}
#endif //USE_EXTERNAL_NVM

/*********************************************************************
 * Function:        BOOL ZDOHasBackgroundTasks( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - ZDO layer has background tasks to run
 *                  FALSE - ZDO layer does not have background tasks
 *
 * Side Effects:    None
 *
 * Overview:        Determines if the ZDO layer has background tasks
 *                  that need to be run.
 *
 * Note:            None
 ********************************************************************/

BOOL ZDOHasBackgroundTasks( void )
{
    return ((zdoStatus.flags.Val & ZDO_BACKGROUND_TASKS) != 0);
}

/*********************************************************************
 * Function:        void ZDOInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    ZDO layer data structures are initialized.
 *
 * Overview:        This routine initializes all ZDO layer data
 *                  structures.
 *
 * Note:            This routine is intended to be called as part of
 *                  a network or power-up initialization.  If called
 *                  after the network has been running, heap space
 *                  may be lost unless the heap is also reinitialized.
 *                  End device binding in progress will be lost.
 ********************************************************************/

void ZDOInit( void )
{
    zdoStatus.flags.Val         = 0;
    ZDOCounter                  = 0;
    tryToRejoin                 = 1;
#ifdef I_SUPPORT_SECURITY
    rejoinWithSec               = 1;
#endif

#if I_SUPPORT_FREQUENCY_AGILITY == 1
    /* Set the last ED record received by the reporting
       device to invalid */
    EdRecords[0].ScanDeviceAddr.Val = 0xFFFF;
#endif
    #ifdef I_SUPPORT_BINDINGS
        pBindInProgressInfo     = NULL;
    #endif

    #ifdef SUPPORT_END_DEVICE_BINDING
       // pEndBindRequest->pFirstEndDeviceBindReq    = NULL;
        //pEndBindRequest->pSecondEndDeviceBindReq   = NULL;
        pEndBindRequest = NULL;
    #endif

    #ifdef I_AM_END_DEVICE
        QuickPoll = 0;
    #endif

    #ifdef I_SUPPORT_COMMISSIONING
        GetActiveSAS ();
    #endif

    #if I_SUPPORT_LINK_KEY == 1
    if(I_AM_TRUST_CENTER){ //#ifdef I_AM_TRUST_CENTER
        pRequestKeyDetails = NULL;
    }//#endif
    #endif

    appNotification = FALSE;
    appNotify.current_ADDR.Val = 0xFFFF;
    appNotify.current_PANID.Val = 0xFFFF;
    appNotify.currentChannel = 0xFF;
}

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE ZDOTasks(ZIGBEE_PRIMITIVE inputPrimitive)
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
 * Note:            This routine may be called while the TX path is blocked.
 *                  Therefore, we must check the Tx path before doing any
 *                  processing that may generate a message.
 *                  It is the responsibility of this task to ensure that
 *                  only one output primitive is generated by any path.
 *                  If multiple output primitives are generated, they
 *                  must be generated one at a time by background processing.
 ********************************************************************/

ZIGBEE_PRIMITIVE ZDOTasks(ZIGBEE_PRIMITIVE inputPrimitive)
{
    ZIGBEE_PRIMITIVE    nextPrimitive;
    BYTE                i;

    nextPrimitive = NO_PRIMITIVE;

    // Manage primitive- and Tx-independent tasks here.  These tasks CANNOT
    // produce a primitive or send a message.

    // Handle other tasks and primitives that may require a message to be sent.
    if (inputPrimitive == NO_PRIMITIVE)
    {

        // If Tx is blocked, we cannot generate a message or send back another primitive.
        if (!ZigBeeReady())
        {
            if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
               ZigBeeUnblockTx();
               MACEnable();
               printf("..Not Ready \r\n");
            }//#endif
                
            return NO_PRIMITIVE;
        }
        
        #ifdef I_SUPPORT_AUTO_POLLING
        #if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
            if (!panIDConflictStatus.flags.bits.bPANIdUpdatePending)
        #endif
            {
                nextPrimitive = StartEndDevicePolling();
                if (nextPrimitive)
                {
                    return nextPrimitive;
                }
            }
        #endif
        TICK tempTick = TickGet();

        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
        #ifndef I_AM_END_DEVICE
            if (zdoStatus.flags.bits.bSendUnsolicitedRejoinRsp)
            {
                zdoStatus.flags.bits.bSendUnsolicitedRejoinRsp = FALSE;
                nextPrimitive = PrepareUnsolicitedRejoinRsp( addressConflictOnChildEndDevice );
                return nextPrimitive;
            }
        #endif
        #endif
#if I_SUPPORT_FREQUENCY_AGILITY == 1
        if (mgmtNwkUpateRequestDetails.scanCount > 0)
        {
            if (zdoStatus.flags.bits.bWaitForUpdateNotifyTransmissionProgress)
            {
                if(TickGetDiff(tempTick, zdoStatus.WaitDurationForMgmtNwkUpdateNotifyTransmission) > (ONE_SECOND * INTER_SCAN_DELAY) )
                {
                    // backup the current channel
                    phyPIB.phyBackupChannel = phyPIB.phyCurrentChannel;

                    /* Indicates Scanning issued from ZDO for transmitting
                       Mgmt_NWK_Update_Notify. */
                    nwkStatus.flags.bits.bScanRequestFromZDO = 1;

                    params.MLME_SCAN_request.ScanType = MAC_SCAN_ENERGY_DETECT;
                    params.MLME_SCAN_request.ScanChannels.Val = mgmtNwkUpateRequestDetails.ScanChannels.Val;
                    params.MLME_SCAN_request.ScanDuration = mgmtNwkUpateRequestDetails.ScanDuration;

                    nextPrimitive = MLME_SCAN_request;
                    mgmtNwkUpateRequestDetails.scanCount--;
                    zdoStatus.flags.bits.bWaitForUpdateNotifyTransmissionProgress = 0;
                }
            }
        }
        /* This is required to reset "zdoStatus.flags.bits.bWaitForUpdateNotifyTransmissionProgress" if the
        scan was initiated with initial value 0x01.      */
        else
        {
            zdoStatus.flags.bits.bWaitForUpdateNotifyTransmissionProgress = 0;
        }

        /* After reception of Mgmt_NWK_update_Notify, the network manager
           will wait for another Mgmt_NWK_update_Notify. Verify the timer
           for that is expired. */

        if (zdoStatus.flags.bits.bWaitForUpdateNotifyTimerInProgress)
        {
            if (TickGetDiff(tempTick, zdoStatus.WaitForUpdateNotifyTick) > (ONE_SECOND * MGMT_NWK_UPDATE_WAIT))
            {
                zdoStatus.flags.bits.bWaitForUpdateNotifyTimerInProgress = 0;
            }
        }

        /* Mgmt_NWK_update_Notify should not be transmitted more than 4 times per hour.
           Verify the timer for it is expired */
        if( !VerifyNetworkManager())
        {
            if (zdoStatus.flags.bits.bUpdateNotifyTimerInProgress)
            {
                if (TickGetDiff(tempTick, zdoStatus.UpdateNotifyTimerTick) > (ONE_SECOND * 3600))
                {
                    zdoStatus.flags.bits.bUpdateNotifyTimerInProgress = 0;
                    UpdateNotifyCount = 0;
                }
            }
        }
        if (zdoStatus.flags.bits.bSwitchChannelTimerInProgress)
        {
            if (TickGetDiff(tempTick, zdoStatus.SwitchChannelTick) > (ONE_SECOND * SWITCH_CHANNEL_WAIT))
            {
                /* Switch the channel after broadcasting Mgmt_NWK_Update_req
                   for channel change by Network manager, or rebroadcasted by
                   other devices which supports frequency agility. */
                if( nwkStatus.flags.bits.bChannelSwitch)
                {
                    nwkStatus.flags.bits.bChannelSwitch = 0;
                    phyPIB.phyCurrentChannel = NewChannel;
                    PHYSetLongRAMAddr(0x200, (0x02 | (BYTE)((phyPIB.phyCurrentChannel - 11) << 4)));
                    PHYSetShortRAMAddr(PWRCTL,0x04);
                    PHYSetShortRAMAddr(PWRCTL,0x00);
                    StorePersistancePIB();
                    appNotify.currentChannel = phyPIB.phyCurrentChannel;
                    appNotification = TRUE;
                }
            }
        }
#endif


        // Manage background tasks here

        if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR

            if( ZigBeeStatus.flags.bits.bTriggerDeviceAnnce)
            {
                ZigBeeStatus.flags.bits.bTriggerDeviceAnnce = 0;

                LONG_ADDR myLongAddress;

                GetMACAddress(&myLongAddress);

                ZigBeeBlockTx();
                TxBuffer[TxData++] = ZDOCounter++;
                TxBuffer[TxData++] = macPIB.macShortAddress.v[0];
                TxBuffer[TxData++] = macPIB.macShortAddress.v[1];

                TxBuffer[TxData++] = myLongAddress.v[0];
                TxBuffer[TxData++] = myLongAddress.v[1];
                TxBuffer[TxData++] = myLongAddress.v[2];
                TxBuffer[TxData++] = myLongAddress.v[3];
                TxBuffer[TxData++] = myLongAddress.v[4];
                TxBuffer[TxData++] = myLongAddress.v[5];
                TxBuffer[TxData++] = myLongAddress.v[6];
                TxBuffer[TxData++] = myLongAddress.v[7];

                TxBuffer[TxData++] = MY_CAPABILITY_INFO;

                params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
                params.APSDE_DATA_request.DstEndpoint = EP_ZDO;
                //As per Spec Destination Address should be BCTORXONWHENIDLEDEVICES;
                // But, due to address conflict, this short address may change. In this case,
                // End devices which has binding table entry should receive Device_Annce and
                // update with new address in the binding table.
                params.APSDE_DATA_request.DstAddress.ShortAddr.Val = 0xFFFD;

                params.APSDE_DATA_request.ProfileId.Val = ZDO_PROFILE_ID;
                params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
                params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;

                params.APSDE_DATA_request.TxOptions.Val = DEFAULT_TX_OPTIONS;

                params.APSDE_DATA_request.SrcEndpoint = EP_ZDO;
                params.APSDE_DATA_request.ClusterId.Val = END_DEVICE_annce;
                /* Change state which helps to give permit join confirm to test tool */
                State = ZDOStateON;
                return APSDE_DATA_request;

            }
        }//    #endif
            #if I_SUPPORT_LINK_KEY == 1
            if(I_AM_TRUST_CENTER){ //#ifdef I_AM_TRUST_CENTER
            if( pRequestKeyDetails != NULL)
            {
                TICK currentTime = TickGet();
                if((pRequestKeyDetails->state == WAIT_FOR_SECOND_REQUEST_KEY_REQUEST )
                && (TickGetDiff(currentTime, pRequestKeyDetails->firstRequestTime) >=
                (ONE_SECOND * TIME_TO_WAIT_FOR_APP_LINK_KEY_REQUEST)))
                {
                    nfree(pRequestKeyDetails);
                    pRequestKeyDetails = NULL;
                }
                else if(pRequestKeyDetails->state == PENDING_TRANSPORT_KEY_FOR_FIRST_DEVICE )
                {
                    BYTE i;

                    pRequestKeyDetails->state = READY_STATE;
                    params.APSME_TRANSPORT_KEY_request.Key = &(pRequestKeyDetails->GeneratedAPPLinkKey);
                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = FALSE;
                    params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val =
                        pRequestKeyDetails->FirstRequestInitiatorShortAddress.Val;

                    for (i=0; i<8; i++)
                    {
                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.APPLICATION_LINKKEY.PartnerAddress.v[i] =
                            pRequestKeyDetails->FirstRequestPartnerAddress.v[i];
                    }
                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.APPLICATION_LINKKEY.Initiator = 0x01; // it will always be true
                    params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val =
                       pRequestKeyDetails->FirstRequestInitiatorShortAddress.Val;
                    params.APSME_TRANSPORT_KEY_request.KeyType = APP_LINK_KEY;
                    params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = TRUE;
                    params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = TRUE;

                    for (i=0; i<8; i++)
                    {
                        params.APSME_TRANSPORT_KEY_request.DestinationAddress.v[i] =
                        pRequestKeyDetails->FirstRequestInitiatorAddress.v[i];
                    }
                    nfree(pRequestKeyDetails);
                    pRequestKeyDetails = NULL;
                    nextPrimitive = APSME_TRANSPORT_KEY_request;
                    return nextPrimitive;
                }

            }

            }//#endif
            #endif
            #if I_SUPPORT_CONCENTRATOR == 1
                if( ZigBeeStatus.flags.bits.bTransmitManyToOneRouting == TRUE )
                {
                      TICK currentTick = TickGet();

                      if (TickGetDiff( currentTick, ManyToOneRequestTime) >
                      ONE_SECOND * current_SAS.cpas.ConcentratorDiscoveryTime)
                      {

                          if ( current_SAS.cpas.ConcentratorDiscoveryTime > 0x00 )
                          {
                              ZigBeeStatus.flags.bits.bTransmitManyToOneRouting = TRUE;
                              ManyToOneRequestTime = TickGet();
                          }
                          else
                          {
                              ZigBeeStatus.flags.bits.bTransmitManyToOneRouting = FALSE;
                          }
                          return( BroadcastManyToOneRequest());
                      }


                }
            #endif
            #if I_SUPPORT_FREQUENCY_AGILITY == 1
            /* Check whether ED scan for transmitting Mgmt_NWK_Update_Notify is done */
            if(zdoStatus.flags.bits.bNwkUpdateEDScan)
            {
                DWORD ChannelMask = 0x00000800;
                DWORD_VAL ScannedChannels;
                BYTE Channel = 11, j = 0;

                zdoStatus.flags.bits.bNwkUpdateEDScan = 0;

                // restore the channel
                phyPIB.phyCurrentChannel = phyPIB.phyBackupChannel;
                PHYSetLongRAMAddr(0x200, (0x01 | (BYTE)((phyPIB.phyCurrentChannel-11)<<4)));
                PHYSetShortRAMAddr(PWRCTL,0x04);
                PHYSetShortRAMAddr(PWRCTL,0x00);


                TxData = TX_DATA_START + MSG_HEADER_SIZE;
                TxBuffer[TxData++] = SUCCESS;

                /* Get scanned channels from unscanned channels. */
                params.MLME_SCAN_confirm.UnscannedChannels.Val |= ((POSSIBLE_CHANNEL_MASK) ^ 0xFFFFFFFF);

                for(i = 0; i < 4; i++)
                {
                    ScannedChannels.v[i] = params.MLME_SCAN_confirm.UnscannedChannels.v[i] ^ 0xFF;
                }

                for (i = 0; i < 16; i++)
                {
                    if (ScannedChannels.Val & ChannelMask)
                    {
                        if( phyPIB.phyCurrentChannel == Channel )
                        {
                        #if I_SUPPORT_FREQUENCY_AGILITY_TESTING == 1
                            if (!CurrentChannelEnergy)
                        #endif
                            {
                                /* Use the scanned energy value for the current channel
                                   to send the Mgmt_NWK_Update_Notify  */
                                CurrentChannelEnergy = params.MLME_SCAN_confirm.EnergyDetectList[j];
                            }
                        #if I_SUPPORT_FREQUENCY_AGILITY_TESTING == 1
                            else
                            {
                                 /* Use the current channel energy set using
                                    CurrentChannelEnergy variable. */
                                params.MLME_SCAN_confirm.EnergyDetectList[j] = CurrentChannelEnergy;
                            }
                        #endif
                            break;
                        }
                        j++;
                    }
                    ChannelMask <<= 1;
                    Channel++;
                }

                if (InterferenceDetected)
                {
                    for( i = 0; i < params.MLME_SCAN_confirm.ResultListSize; i++)
                    {
                       /* Verify any of the channels scanned, having lesser energy
                          than the current channel */
                       if( params.MLME_SCAN_confirm.EnergyDetectList[i] < CurrentChannelEnergy )
                       {
                           break;
                       }
                    }
                    CurrentChannelEnergy = 0;
                    if ( i == params.MLME_SCAN_confirm.ResultListSize)
                    {
                        if (params.MLME_SCAN_confirm.EnergyDetectList)
                        {
                            /* Free the memory allocated for storing energy list. */
                            nfree( params.MLME_SCAN_confirm.EnergyDetectList );
                        }
                        /* No Channel is having lesser energy than current channel, hence return */
                        InterferenceDetected = 0;
                        return NO_PRIMITIVE;
                    }
                }

                /* Create the Mgmt_NWK_Update_Notify frame for transmission */
                for(i = 0; i < 4; i++)
                {
                    TxBuffer[TxData++] = ScannedChannels.v[i];
                }
                TxBuffer[TxData++] = TotalTransmittedPackets.v[0];     // total transmissions LSB
                TxBuffer[TxData++] = TotalTransmittedPackets.v[1];     // total transmissions MSB
                TxBuffer[TxData++] = TotalTransmitFailures.v[0];       // transmission failures LSB
                TxBuffer[TxData++] = TotalTransmitFailures.v[1];       // transmission failures MSB
                TxBuffer[TxData++] = params.MLME_SCAN_confirm.ResultListSize;  // scanned channels list count
                for(i = 0; i < params.MLME_SCAN_confirm.ResultListSize; i++)
                {
                    TxBuffer[TxData++] = params.MLME_SCAN_confirm.EnergyDetectList[i];
                }

                /* Verify whether Network manager has detected channel interference */
                if (VerifyNetworkManager())
                {
                    BYTE i;
                    DWORD ChannelMask = 0x00000800;
                    ENERGY_DETECT_RECORD *EdRecordPtr = EdRecords;

                    for(i = 0; i < 16; i++)
                    {
                        /* Store the energy detected on scanned channels, so that it will
                           report to himself. */
                        if( ScannedChannels.Val & ChannelMask )
                        {
                            /* Store the energy only of allowed channels */
                            if (current_SAS.spas.ChannelMask.Val & ChannelMask)
                            {
                                EdRecordPtr->EnergyReading[i] = params.MLME_SCAN_confirm.EnergyDetectList[i];
                            }
                            else
                            {
                                EdRecordPtr->EnergyReading[i] = 0xFF;
                            }
                        }
                        ChannelMask = ChannelMask << 1;
                    }
                    EdRecordPtr->EnergyReading[15] = 0xFF;
                }
                else
                {
                    GetNeighborTableInfo();
                    /* Prepare the MGMT_NWK_UPDATE_notify command frame */
                    params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val = currentNeighborTableInfo.nwkManagerAddr.Val;
                    PrepareMessageResponse( MGMT_NWK_UPDATE_notify );
                    nextPrimitive = APSDE_DATA_request;
                }

                if (params.MLME_SCAN_confirm.EnergyDetectList)
                {
                    /* Free the memory allocated for storing energy list. */
                    nfree( params.MLME_SCAN_confirm.EnergyDetectList );
                }
                if (VerifyNetworkManager())
                {
                    /* Netwok manager report the channel interference to himself(Loopback) */
                    nextPrimitive = HandleMgmtNwkUpdateNotify(TRUE, SUCCESS, ScannedChannels,
                        TotalTransmittedPackets, TotalTransmitFailures,
                        params.MLME_SCAN_confirm.ResultListSize);
                }

                if (InterferenceDetected)
                {
                    if (!zdoStatus.flags.bits.bUpdateNotifyTimerInProgress)
                    {
                        /* Start the timer so that device should not send Mgmt_NWK_update_Notify
                           more than 4 times per hour */
                        zdoStatus.flags.bits.bUpdateNotifyTimerInProgress = 1;
                        zdoStatus.UpdateNotifyTimerTick = TickGet();
                    }

                    /* Number of times Mgmt_NWK_Update_notify sent is incremented. */
                    UpdateNotifyCount++;

                    /* Reset the counters for total transmitted packets and total failure */
                    TotalTransmittedPackets.Val = 0;
                    TotalTransmitFailures.Val = 0;
                    InterferenceDetected = 0;
                }
                zdoStatus.WaitDurationForMgmtNwkUpdateNotifyTransmission = TickGet();
                zdoStatus.flags.bits.bWaitForUpdateNotifyTransmissionProgress = 1;
                return nextPrimitive;
            }
        #endif
		
		if(AskDeviceAddress == 1)
		{
			MySendAddressRequest(sendrequestSource,SendIee);
			//return APSDE_DATA_request;
			nextPrimitive = APSDE_DATA_request;
		}

        // ---------------------------------------------------------------------
        // Handle pending end device bind request
        #ifdef SUPPORT_END_DEVICE_BINDING
            if (zdoStatus.flags.bits.bEndDeviceBinding)
            {

                // NOTE: Compiler SSR27744, TickGet() output must be assigned to a variable.
                tempTick = TickGet() ;
                if ( ( pEndBindRequest->EBstate < SEND_EB_RESPONSE_TO_FIRST_DEVICE )&&
                    (TickGetDiff(tempTick, pEndBindRequest->lastTick) >= CONFIG_ENDDEV_BIND_TIMEOUT) )
                {
                    pEndBindRequest->EBstate = SEND_EB_RESPONSE_TO_FIRST_DEVICE;
                    pEndBindRequest->lastTick = TickGet();
                }

                 if (pEndBindRequest->EBstate == SEND_EB_RESPONSE_TO_FIRST_DEVICE)
                 {

                     if( pEndBindRequest->pSecondEndDeviceBindReq != NULL )
                     {
                         pEndBindRequest->EBstate = SEND_EB_RESPONSE_TO_SECOND_DEVICE;
                     }
                     else
                     {
                         pEndBindRequest->EBstate =  EB_DONE;
                     }
                     return(Send_END_DEVICE_BIND_rsp(pEndBindRequest->pFirstEndDeviceBindReq));
                 }

                 if (pEndBindRequest->EBstate == SEND_EB_RESPONSE_TO_SECOND_DEVICE)
                 {
                     pEndBindRequest->EBstate =  EB_DONE;
                     return(Send_END_DEVICE_BIND_rsp(pEndBindRequest->pSecondEndDeviceBindReq));

                 }

                 if (pEndBindRequest->EBstate == SEND_TEST_UNBIND_TO_FIRST_DEVICE)
                 {
                     return( SendBindOrUnbind( UNBIND_req, pEndBindRequest->pFirstEndDeviceBindReq,
                     pEndBindRequest->pSecondEndDeviceBindReq, FALSE ) );
                 }

                  if (pEndBindRequest->EBstate == SEND_TEST_UNBIND_TO_SECOND_DEVICE)
                 {
                    return (SendBindOrUnbind( UNBIND_req, pEndBindRequest->pSecondEndDeviceBindReq,
                     pEndBindRequest->pFirstEndDeviceBindReq, FALSE ));
                 }

                  if (pEndBindRequest->EBstate == SEND_UNBIND_TO_FIRST_DEVICE)
                 {
                      return(SendBindOrUnbind( UNBIND_req, pEndBindRequest->pFirstEndDeviceBindReq,
                     pEndBindRequest->pSecondEndDeviceBindReq, TRUE ));
                 }

                  if (pEndBindRequest->EBstate == SEND_UNBIND_TO_SECOND_DEVICE)
                 {
                      return (SendBindOrUnbind( UNBIND_req, pEndBindRequest->pSecondEndDeviceBindReq,
                     pEndBindRequest->pFirstEndDeviceBindReq, TRUE ));
                 }

                  if (pEndBindRequest->EBstate == SEND_BIND_TO_FIRST_DEVICE)
                 {
                      return (SendBindOrUnbind( BIND_req, pEndBindRequest->pFirstEndDeviceBindReq,
                     pEndBindRequest->pSecondEndDeviceBindReq, TRUE ));
                 }

                  if (pEndBindRequest->EBstate == SEND_BIND_TO_SECOND_DEVICE)
                 {
                      return(SendBindOrUnbind( BIND_req, pEndBindRequest->pSecondEndDeviceBindReq,
                     pEndBindRequest->pFirstEndDeviceBindReq, TRUE ));
                 }

                  if (pEndBindRequest->EBstate == EB_DONE)
                 {
                     ClearMemoryAllocatedForEB();
                     zdoStatus.flags.bits.bEndDeviceBinding =0;
                     return NO_PRIMITIVE;
                 }
            }


        #endif
		
		

        #if defined(I_SUPPORT_BINDINGS)
            if (zdoStatus.flags.bits.bBinding)
            {
                BYTE    returnCode;
                if (TickGetDiff(tempTick, pBindInProgressInfo->timeStamp) >= (CONFIG_ENDDEV_BIND_TIMEOUT) * 2 )
                {
                    // Send a bind/unbind confirmation with failure
                    returnCode = ZDO_TIMEOUT;
                    goto SendBackgroundBindResponse;

                }
                else
                {
                    if (!pBindInProgressInfo->status.bits.bSourceRequested)
                    {
                        // Send a NWK_ADDR_req for the source
                        SendBindAddressRequest( TRUE );
                        pBindInProgressInfo->status.bits.bSourceRequested = 1;

                        return APSDE_DATA_request;
                    }
                    else if (!pBindInProgressInfo->status.bits.bDestinationRequested)
                    {
                        // Send a NWK_ADDR_req for the destination
                        SendBindAddressRequest( FALSE );
                        pBindInProgressInfo->status.bits.bDestinationRequested = 1;

                        return APSDE_DATA_request;
                    }
                }

                if ((pBindInProgressInfo->sourceAddressShort.Val != 0xFFFF) &&
                    (pBindInProgressInfo->destinationAddressShort.Val != 0xFFFF))
                {
                    // We have all the information.  Try to create the binding.
                    // If the binding came from the upper layers, make sure we can send up the response.  Otherwise, wait.
                    if (!pBindInProgressInfo->status.bits.bFromUpperLayers || CurrentRxPacket == NULL)
                    {
                        returnCode = SUCCESS;

                        if (pBindInProgressInfo->status.bits.bBindNodes)
                        {
                            if (APSAddBindingInfo( pBindInProgressInfo->sourceAddressShort, pBindInProgressInfo->sourceEP,
                                pBindInProgressInfo->cluster, pBindInProgressInfo->destinationAddressShort, pBindInProgressInfo->destinationEP))
                            {
                                returnCode = ZDO_TABLE_FULL;
                            }
                        }
                        else
                        {
                            BYTE rcode;

                            if (  (rcode = APSRemoveBindingInfo( pBindInProgressInfo->sourceAddressShort, pBindInProgressInfo->sourceEP,
                                pBindInProgressInfo->cluster, pBindInProgressInfo->destinationAddressShort, pBindInProgressInfo->destinationEP))  )
                            {
                                returnCode = ZDO_NO_ENTRY;
                            }
                        }

SendBackgroundBindResponse:
                        if (pBindInProgressInfo->status.bits.bFromUpperLayers)
                        {
                            if (SendUpBindResult( returnCode, pBindInProgressInfo->status.bits.bBindNodes ))
                            {
                                nextPrimitive = APSDE_DATA_indication;
                            }
                            else
                            {
                                nextPrimitive = NO_PRIMITIVE;
                            }
                        }
                        else
                        {
                            ZigBeeBlockTx();
                            TxData = TX_DATA_START + MSG_HEADER_SIZE;
                            TxBuffer[TxData++] = returnCode;
                            sequenceNumber = pBindInProgressInfo->sequenceNumber;
                            params.ZDO_DATA_indication.SrcAddress.ShortAddr = pBindInProgressInfo->requestorAddress;
                            if (pBindInProgressInfo->status.bits.bBindNodes)
                            {
                                PrepareMessageResponse( BIND_rsp );
                            }
                            else
                            {
                                PrepareMessageResponse( UNBIND_rsp );
                            }
                            nextPrimitive = APSDE_DATA_request;
                        }

                        nfree( pBindInProgressInfo );
                        zdoStatus.flags.bits.bBinding = 0;
                        return nextPrimitive;
                    }
                }
            }
        #endif


        #if defined(I_SUPPORT_SECURITY)

            if(zdoStatus.flags.bits.SendUpdateDeviceRequestAfterChildLeave)
            {
                params.APSME_UPDATE_DEVICE_request.Status = DEVICE_LEFT;
                memcpy(params.APSME_UPDATE_DEVICE_request.DeviceAddress.v,leavingChildDetails.DeviceLongAddress.v,8);
                params.APSME_UPDATE_DEVICE_request.DeviceShortAddress.Val = leavingChildDetails.DeviceShortAddress.Val;
                memcpy(params.APSME_UPDATE_DEVICE_request.DestAddress.v, current_SAS.spas.TrustCenterAddress.v,8);
                zdoStatus.flags.bits.SendUpdateDeviceRequestAfterChildLeave = 0;
                return APSME_UPDATE_DEVICE_request;
            }

            if ( zdoStatus.flags.bits.bSwitchKey )
            {
                if( TickGetDiff(tempTick, zdoStatus.SwitchKeyTick) > (ONE_SECOND * ((DWORD)NIB_nwkNetworkBroadcastDeliveryTime)) )
                {

                    zdoStatus.flags.bits.bSwitchKey = 0;
                    for(i = 0; i < 2; i++)
                    {
                        #ifdef USE_EXTERNAL_NVM
                            currentNetworkKeyInfo = plainSecurityKey[i];
                        #else
                            GetNwkKeyInfo(&currentNetworkKeyInfo, &networkKeyInfo[i]);
                        #endif
                        if( currentNetworkKeyInfo.SeqNumber.v[0] ==  zdoStatus.KeySeq &&
                            currentNetworkKeyInfo.SeqNumber.v[1] == nwkMAGICResSeq )
                        {

                                i++;
                                PutNwkActiveKeyNumber(&i);

                            break;
                        }
                    }
                }
            }
        #endif

        /* The stack is free. Notify the application about changes in stack parameters. (Short address, channel, PANid etc) */
        if( appNotification == TRUE )
        {
            AppNotification();
            appNotification = FALSE;
        }
    }
    else
    {
        switch( inputPrimitive )
        {
            case APP_START_DEVICE_request:
            #ifdef DEBUG_LOG
                NoOfPANDescriptorFound = 0;
                PotentialParentFound = 0;
            #endif
                /* Use the mode of start, which is set from NVM */
                params.APP_START_DEVICE_req.mode = startMode;
                /* Trigger the startup procedure */
                return (InitiateStartup());
                
             break;

        //#ifdef I_AM_COORDINATOR
            case NLME_NETWORK_FORMATION_confirm:
                if(NOW_I_AM_A_CORDINATOR()){//
                if ( !params.NLME_NETWORK_FORMATION_confirm.Status )
                {
                    #ifdef I_SUPPORT_COMMISSIONING
                        if(current_SAS.spas.ExtendedPANId.v[0]|
                           current_SAS.spas.ExtendedPANId.v[1]|
                           current_SAS.spas.ExtendedPANId.v[2]|
                           current_SAS.spas.ExtendedPANId.v[3]|
                           current_SAS.spas.ExtendedPANId.v[4]|
                           current_SAS.spas.ExtendedPANId.v[5]|
                           current_SAS.spas.ExtendedPANId.v[6]|
                           current_SAS.spas.ExtendedPANId.v[7])
                        {
                            BYTE i;
                            for(i = 0; i < 8; i++)
                            {
                                /* Set Extended PANId from SAS, as extended PANId is set in SAS. */
                                currentNeighborTableInfo.nwkExtendedPANID.v[i]= current_SAS.spas.ExtendedPANId.v[i];
                            }
                        }
                        else
                        {
                            BYTE i;
                            for(i = 0; i < 8; i++)
                            {
                                GetMACAddressByte(i, (BYTE *)&(currentNeighborTableInfo.nwkExtendedPANID.v[i]));
                            }
                        }
                    #else
                        BYTE i;
                        for(i = 0; i < 8; i++)
                        {
                            GetMACAddressByte(i, &(currentNeighborTableInfo.nwkExtendedPANID.v[i]));
                        }
                    #endif

                    params.APP_START_DEVICE_conf.ActiveChannel = phyPIB.phyCurrentChannel;
                    params.APP_START_DEVICE_conf.PANId.Val = macPIB.macPANId.Val;
                    params.APP_START_DEVICE_conf.ShortAddress.v[0] = macPIB.macShortAddress.v[0];
                    params.APP_START_DEVICE_conf.ShortAddress.v[1] = macPIB.macShortAddress.v[1];
                    nextPrimitive = APP_START_DEVICE_confirm;

                    macPIB.macAssociationPermit = DEFAULT_ASSOC_PERMIT;
                    #if I_SUPPORT_CONCENTRATOR == 1
                        ZigBeeStatus.flags.bits.bTransmitManyToOneRouting  = 1;
                        ManyToOneRequestTime = TickGet();
                    #endif
                    // Commit all neighbor table info changes, from here and from the caller.
                    PutNeighborTableInfo();
                    SetBeaconPayload(TRUE);
                }
                else
                {
                    nextPrimitive = NO_PRIMITIVE;
                }
                return nextPrimitive;

            break;
			}//#endif
			break;
            case NLME_LEAVE_indication:
                
                return APP_LEAVE_indication; //all parameter in NLME_LEAVE_indication with APP_LEAVE_indication
                break;
            case NLME_NETWORK_DISCOVERY_confirm:

                nextPrimitive = NO_PRIMITIVE;

                if( State != ZDOJoiningState )
                {
                    phyPIB.phyCurrentChannel = phyPIB.phyBackupChannel;
                    PHYSetLongRAMAddr(0x200, (0x02 | (BYTE)((phyPIB.phyCurrentChannel - 11) << 4)));
                    PHYSetShortRAMAddr(PWRCTL,0x04);
                    PHYSetShortRAMAddr(PWRCTL,0x00);
                    return APP_NETWORK_DISCOVERY_confirm;
                }
				if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
                NwkDiscoveryTries++;
				NetworkDescIndex = 0;
                NetworkDescriptor = params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor;
                
                /* ensure that status is successful and network count is not zero before using the descriptor */
                if(params.NLME_NETWORK_DISCOVERY_confirm.Status != SUCCESS || params.NLME_NETWORK_DISCOVERY_confirm.NetworkCount == 0)
                {
                    NetworkDescriptor = NULL;
                }
                
                while (NetworkDescriptor)
                {
                    for (i = 0; i < NetworkDescIndex; i++)
                    {
                        if (i == MAXNETDESCSUPPORTED)
                        {
                            break;
                        }
						/* If the permitJoining bit is toggling we need to check it as 
						 * as well, and only if they are the same should we break, else update NetworkDesc to the latest 
						 * state - tricky state discovered when many devices are on a 
						 * given channel and their permitJoining bits are toggling
					    */
                        if (!memcmp((BYTE *)&NetworkDescriptor->ExtendedPANID, (BYTE *)&(NetworkDesc[i].ExtendedPANID), 8))
                        {
                            if (NetworkDescriptor->PermitJoining == NetworkDesc[i].PermitJoining) 
    						{
                            	break;
							}
                        }

                    }
                    if (i == NetworkDescIndex)
                    {
                        memcpy((BYTE *)&NetworkDesc[i].ExtendedPANID,(BYTE *)&NetworkDescriptor->ExtendedPANID, 8);
                        NetworkDesc[i].PermitJoining = NetworkDescriptor->PermitJoining;
                        NetworkDesc[i].LogicalChannel = NetworkDescriptor->LogicalChannel;
                        NetworkDescIndex++;
                    }
                    currentNetworkDescriptor = NetworkDescriptor->next;
                    nfree( NetworkDescriptor );
                    NetworkDescriptor = currentNetworkDescriptor;
                }
                if (NwkDiscoveryTries >= MAXDISCOVERYYRETRIES)
                {
                    nextPrimitive = SubmitJoinRequest();
                }
                else
                {
                    params.NLME_NETWORK_DISCOVERY_request.ScanDuration          = NWK_SCAN_DURTION;
                    params.NLME_NETWORK_DISCOVERY_request.ScanChannels          = current_SAS.spas.ChannelMask;
                    nextPrimitive = NLME_NETWORK_DISCOVERY_request;
                }
                return nextPrimitive;
    		}// #endif
            break;

//#ifndef I_AM_COORDINATOR
            case NLME_JOIN_confirm:
				if(NOW_I_AM_NOT_A_CORDINATOR()){//
                State = ZDOJoiningState;
                nextPrimitive = NO_PRIMITIVE;
                startMode = 0;
                
                if (joinReqType == REJOIN_PROCESS && params.NLME_JOIN_confirm.Status != 0x00)
                {
                    if (current_SAS.spas.UseInsecureJoin)
                    {
                        #ifdef I_SUPPORT_SECURITY
                        if (syncLoss)
                        {
                            syncLoss = 0;
                            if ( rejoinWithSec)
                            {
                                rejoinWithSec = 0;
                                JoinRequest( currentNeighborTableInfo.nwkExtendedPANID, REJOIN_PROCESS, 0 );
                                return NLME_JOIN_request;
                            }
                        }
                        #endif
                        #ifdef I_SUPPORT_SECURITY
                            rejoinWithSec = 0;
                        #endif
                        return SubmitJoinRequest();

                    }
                    else
                    {
                        NwkJoinTries = 0;
                        NwkDiscoveryTries = 0;
                        params.APP_START_DEVICE_conf.Status = params.NLME_JOIN_confirm.Status;
                        params.APP_START_DEVICE_conf.ActiveChannel = 0xFF;
                        params.APP_START_DEVICE_conf.PANId.Val = 0xFFFF;
                        params.APP_START_DEVICE_conf.ShortAddress.Val = 0xFFFF;
                        State = ZDOStateOFF;
                        return APP_START_DEVICE_confirm;
                    }
                }
                NetworkDescIndex = 0;

                if( params.NLME_JOIN_confirm.Status == 0x00 )
                {
                    NwkJoinTries = 0;
                    NwkDiscoveryTries = 0;
                    GetNeighborTableInfo();
                    if (currentNeighborTableInfo.parentNeighborTableIndex != MAX_NEIGHBORS)
                    {
                        #ifdef USE_EXTERNAL_NVM
                            pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                        #else
                            pCurrentNeighborRecord = &(neighborTable[currentNeighborTableInfo.parentNeighborTableIndex]);
                        #endif
                        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

                        if ( currentNeighborRecord.deviceInfo.bits.Depth == ( PROFILE_nwkMaxDepth - 1 ) )
                        {
                            // I am at max depth. I should not allow new devices to join.
                            macPIB.macAssociationPermit = FALSE;
                        }
                        else
                        {
                            // I am not at max depth.
                            macPIB.macAssociationPermit = DEFAULT_ASSOC_PERMIT;
                        }
                    }
                    if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
                        // Start routing capability.
                        params.NLME_START_ROUTER_request.BeaconOrder = MAC_PIB_macBeaconOrder;
                        params.NLME_START_ROUTER_request.SuperframeOrder = MAC_PIB_macSuperframeOrder;
                        params.NLME_START_ROUTER_request.BatteryLifeExtension = FALSE;
                        return NLME_START_ROUTER_request;
                    }else{//#else
                        params.APP_START_DEVICE_conf.ActiveChannel = phyPIB.phyCurrentChannel;
                        params.APP_START_DEVICE_conf.PANId.Val = macPIB.macPANId.Val;
                        params.APP_START_DEVICE_conf.ShortAddress.v[0] = macPIB.macShortAddress.v[0];
                        params.APP_START_DEVICE_conf.ShortAddress.v[1] = macPIB.macShortAddress.v[1];
                        nextPrimitive = APP_START_DEVICE_confirm;
                        State = ZDOStateOFF;
                        ZigBeeStatus.flags.bits.bTriggerDeviceAnnce = 1;
                    }//#endif
                    #if I_SUPPORT_CONCENTRATOR == 1
                        ZigBeeStatus.flags.bits.bTransmitManyToOneRouting  = 1;
                        ManyToOneRequestTime = TickGet();
                    #endif
                    
                    /*The device part of the network now*/
                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        ZigBeeStatus.flags.bits.bLostSyncWithParent = 0;
                    }//#endif
                    
                }
                else if( NwkJoinTries == MAXJOINRETRIES )
                {
                    NwkJoinTries = 0;
                    NwkDiscoveryTries = 0;
                    State = ZDOStateOFF;
                    nextPrimitive = APP_START_DEVICE_confirm;

                }
                else
                {
                    NwkDiscoveryTries = 0;
                    params.NLME_NETWORK_DISCOVERY_request.ScanDuration          = NWK_SCAN_DURTION;
                    params.NLME_NETWORK_DISCOVERY_request.ScanChannels          = current_SAS.spas.ChannelMask;
                    nextPrimitive = NLME_NETWORK_DISCOVERY_request;
                }
                return nextPrimitive;
            break;
    	}//#endif
    	break;

//#ifdef I_AM_ROUTER
            case NLME_START_ROUTER_confirm:
  			if(NOW_I_AM_A_ROUTER()){//
                if (!params.NLME_START_ROUTER_confirm.Status)
                {
                    params.APP_START_DEVICE_conf.ActiveChannel = phyPIB.phyCurrentChannel;
                    params.APP_START_DEVICE_conf.PANId.Val = macPIB.macPANId.Val;
                    params.APP_START_DEVICE_conf.ShortAddress.v[0] = macPIB.macShortAddress.v[0];
                    params.APP_START_DEVICE_conf.ShortAddress.v[1] = macPIB.macShortAddress.v[1];
                    params.APP_START_DEVICE_conf.Status = SUCCESS;
                    State = ZDOStateOFF;
                    nextPrimitive = APP_START_DEVICE_confirm;
                    ZigBeeStatus.flags.bits.bTriggerDeviceAnnce = 1;
                }

                return nextPrimitive;
            break;
			}//#endif
			break;

            case NLME_JOIN_indication:

				if(NOW_I_AM_A_CORDINATOR()){//#ifdef I_AM_COORDINATOR

                /* For Zigbee 2006: 11/13/07 If a new device with the same old longAddress address
                * joins the PAN, then make sure the old short address is no longer used and is
                * overwritten with the new shortAddress & longAddress combo
                */
            {
                    APS_ADDRESS_MAP currentAPSAddress1;
                    currentAPSAddress1.shortAddr   =   params.NLME_JOIN_indication.NetworkAddress;
                    currentAPSAddress1.longAddr    =   params.NLME_JOIN_indication.ExtendedAddress;
                    #if I_SUPPORT_CONCENTRATOR == 1
                        DeleteRouteRecord(currentAPSAddress1.shortAddr);
                    #endif
                 if(LookupAPSAddress(&params.NLME_JOIN_indication.ExtendedAddress) )
                 {

                    for( i = 0; i < apscMaxAddrMapEntries; i++)
                    {
                        #ifdef USE_EXTERNAL_NVM
                            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                        #else
                            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                        #endif
                            if (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&currentAPSAddress1.longAddr, 8 ))
                            {
                                /* overwrite the old with the new short/long address combo  */
                                #ifdef USE_EXTERNAL_NVM
                                    PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress1);
                                #else
                                    PutAPSAddress( &apsAddressMap[i], &currentAPSAddress1 );
                                #endif
                            }
                    }
                }
           }

#ifdef I_SUPPORT_SECURITY

  if(I_AM_TRUST_CENTER){ //#ifdef I_AM_TRUST_CENTER
         {
             SHORT_ADDR tempAddr;
             
             /*keep this address safe from overwrite to APSME_TRANSPORT_KEY_request */
            tempAddr.Val = params.NLME_JOIN_indication.NetworkAddress.Val;
            if(CheckDeviceJoiningPermission(params.APSME_UPDATE_DEVICE_indication.DeviceAddress) == FALSE)
			{
				// no need to set deviceAddress, since it is overlap with NLME_JOIN_indication
                params.NLME_LEAVE_request.RemoveChildren = TRUE;
                nextPrimitive = NLME_LEAVE_request;
				break;
			}  
            // decide if you allow this device to join
            if( !AllowJoin )
            {
                // no need to set deviceAddress, since it is overlap with NLME_JOIN_indication
                params.NLME_LEAVE_request.RemoveChildren = TRUE;
                nextPrimitive = NLME_LEAVE_request;
                break;
            }

         #ifdef I_SUPPORT_SECURITY_SPEC
            if( params.NLME_JOIN_indication.secureRejoin )
            {
                /*BYTE i;
                for(i = 0; i < 16; i++)
                {
                    KeyVal.v[i] = 0;
                }
                params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                */
                return NO_PRIMITIVE;
            }
            else
            {
                BYTE i;
                GetNwkActiveKeyNumber(&i);
                #ifdef USE_EXTERNAL_NVM
                    currentNetworkKeyInfo = plainSecurityKey[i-1];
                #else
                    GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(networkKeyInfo[i-1]));
                #endif
                params.APSME_TRANSPORT_KEY_request.Key = &(currentNetworkKeyInfo.NetKey);
                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];

            }
        #else
            #ifdef PRECONFIG_NWK_KEY
            {

                BYTE i;

                if( params.NLME_JOIN_indication.secureRejoin )
                {
                    return NO_PRIMITIVE;
                }

                for(i = 0; i < 16; i++)
                {
                    KeyVal.v[i] = 0;
                }
                params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = FALSE;
                #if I_SUPPORT_LINK_KEY == 1
                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                        params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = TRUE;
                    #else
                        params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = FALSE;
                    #endif
                #endif
                params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = TRUE;
            }
            #else  // PRECONFIG_NWK_KEY
                if( params.NLME_JOIN_indication.secureRejoin )
                {
                    
                    return NO_PRIMITIVE;
                }
                else
                {
                    BYTE i;
                    GetNwkActiveKeyNumber(&i);
                    #ifdef USE_EXTERNAL_NVM
                        currentNetworkKeyInfo = plainSecurityKey[i-1];
                    #else
                        GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(networkKeyInfo[i-1]));
                    #endif
                    params.APSME_TRANSPORT_KEY_request.Key = &(currentNetworkKeyInfo.NetKey);
                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
                    params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = FALSE;
                    params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = FALSE;
                    #if I_SUPPORT_LINK_KEY == 1
                        #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                            params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = TRUE;
                        #else
                            params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = FALSE;
                        #endif
                    #endif
                }
            #endif   // PRECONFIG_NWK_KEY

        #endif   // I_SUPPORT_SECURITY_SPEC
            
            //params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val = params.NLME_JOIN_indication.NetworkAddress.Val;
            params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val = tempAddr.Val;
            params.APSME_TRANSPORT_KEY_request.KeyType = ID_NetworkKey;
            params.APSME_TRANSPORT_KEY_request.DestinationAddress = params.NLME_JOIN_indication.ExtendedAddress;
            params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = FALSE;

            nextPrimitive = APSME_TRANSPORT_KEY_request;
          }
  }else{//#else // I_AM_TRUST_CENTER

          if(params.NLME_JOIN_indication.RejoinNetwork == ASSOCIATION_REJOIN)
          {
              if ( params.NLME_JOIN_indication.CapabilityInformation.CapBits.SecurityCapability )
              {
                  // Device has the support for High Security
                  params.APSME_UPDATE_DEVICE_request.Status = HIGH_SECURITY_DEVICE_UNSECURED_JOIN;
              }
              else
              {
                  // Device does not has the support for High Security. Only Standard Security is supported.
                  params.APSME_UPDATE_DEVICE_request.Status = STANDARD_DEVICE_UNSECURED_JOIN;
              }
          }
          else if( params.NLME_JOIN_indication.RejoinNetwork == REJOIN_PROCESS )
          {
              if ( params.NLME_JOIN_indication.CapabilityInformation.CapBits.SecurityCapability )
              {
                  // Device has the support for High Security
                  params.APSME_UPDATE_DEVICE_request.Status = ( params.NLME_JOIN_indication.secureRejoin ) ?HIGH_SECURITY_DEVICE_SECURED_REJOIN : HIGH_SECURITY_DEVICE_UNSECURED_REJOIN;
              }
              else
              {
                  // Device does not has the support for High Security. Only Standard Security is supported.
                  params.APSME_UPDATE_DEVICE_request.Status = ( params.NLME_JOIN_indication.secureRejoin ) ?STANDARD_DEVICE_SECURED_REJOIN : STANDARD_DEVICE_UNSECURED_REJOIN;
              }
          }
          
          {
            SHORT_ADDR  tempAddr;
            LONG_ADDR   tempLongAddr;
            BYTE        index;
            
            tempAddr.Val = params.NLME_JOIN_indication.NetworkAddress.Val;
            
            for(index = 0; index < 8; index++)
            {
                tempLongAddr.v[index] = params.NLME_JOIN_indication.ExtendedAddress.v[index];
            }
            params.APSME_UPDATE_DEVICE_request.DeviceShortAddress   = tempAddr;
            params.APSME_UPDATE_DEVICE_request.DeviceAddress        = tempLongAddr;
          }
          params.APSME_UPDATE_DEVICE_request.DestAddress = current_SAS.spas.TrustCenterAddress;
          nextPrimitive = APSME_UPDATE_DEVICE_request;
  }//#endif // I_AM_TRUST_CENTER

#else // I_SUPPORT_SECURITY
                nextPrimitive = NO_PRIMITIVE;
#endif  // I_SUPPORT_SECURITY

		}else{//#else // I_AM_COORDINATOR
            #ifdef I_SUPPORT_SECURITY

                if(I_AM_TRUST_CENTER){ //#ifdef I_AM_TRUST_CENTER
                {
					if(CheckDeviceJoiningPermission(params.APSME_UPDATE_DEVICE_indication.DeviceAddress) == FALSE)
					{
						// no need to set deviceAddress, since it is overlap with NLME_JOIN_indication
						params.NLME_LEAVE_request.RemoveChildren = TRUE;
						nextPrimitive = NLME_LEAVE_request;
						break;
					} 
                    // decide if you allow this device to join
                    if( !AllowJoin )
                    {
                        // no need to set deviceAddress, since it is overlap with NLME_JOIN_indication
                        params.NLME_LEAVE_request.RemoveChildren = TRUE;
                        nextPrimitive = NLME_LEAVE_request;
                        break;
                    }

                    #ifdef I_SUPPORT_SECURITY_SPEC
                        if( params.NLME_JOIN_indication.secureRejoin )
                        {
                            BYTE i;
                            for(i = 0; i < 16; i++)
                            {
                                KeyVal.v[i] = 0;
                            }
                            params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                            params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;

                        } else {
                            BYTE i;
                            GetNwkActiveKeyNumber(&i);
                            GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(NetworkKeyInfo[i-1]));
                            params.APSME_TRANSPORT_KEY_request.Key = &(currentNetworkKeyInfo.NetKey);
                            params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];

                        }
                    #else
                        #ifdef PRECONFIG_NWK_KEY
                        {
                            BYTE i;
                            for(i = 0; i < 16; i++)
                            {
                                KeyVal.v[i] = 0;
                            }
                            params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                            params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                        }
                        #else // PRECONFIG_NWK_KEY
                        {
                            BYTE i;
                            GetNwkActiveKeyNumber(&i);
                            GetNwkKeyInfo(&currentNetworkKeyInfo, (networkKeyInfo));	//(ROM void *)&(NetworkKeyInfo[i-1])-->This was the Original Code. However could not find the definition for this anywhere
                            params.APSME_TRANSPORT_KEY_request.Key = &(currentNetworkKeyInfo.NetKey);
                            params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
                        }
                        #endif // PRECONFIG_NWK_KEY

                    #endif
                    params.APSME_TRANSPORT_KEY_request.KeyType = ID_NetworkKey;
                    params.APSME_TRANSPORT_KEY_request.DestinationAddress = params.NLME_JOIN_indication.ExtendedAddress;
                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = FALSE;
                    nextPrimitive = APSME_TRANSPORT_KEY_request;
                }
                }else{//#else
                    if( params.NLME_JOIN_indication.RejoinNetwork == ASSOCIATION_REJOIN )
                    {
                        if ( params.NLME_JOIN_indication.CapabilityInformation.CapBits.SecurityCapability )
                        {
                            // Device has the support for High Security
                            params.APSME_UPDATE_DEVICE_request.Status = HIGH_SECURITY_DEVICE_UNSECURED_JOIN;
                        }
                        else
                        {
                            // Device does not has the support for High Security. Only Standard Security is supported.
                            params.APSME_UPDATE_DEVICE_request.Status = STANDARD_DEVICE_UNSECURED_JOIN;
                        }
                    }
                    else if( params.NLME_JOIN_indication.RejoinNetwork == REJOIN_PROCESS )
                    {
                        if(params.NLME_JOIN_indication.secureRejoin )
                        {
                            return NO_PRIMITIVE;
                        }
                        else
                        {
                            if ( params.NLME_JOIN_indication.CapabilityInformation.CapBits.SecurityCapability )
                            {
                                // Device has the support for High Security
                                params.APSME_UPDATE_DEVICE_request.Status = ( params.NLME_JOIN_indication.secureRejoin ) ?HIGH_SECURITY_DEVICE_SECURED_REJOIN : HIGH_SECURITY_DEVICE_UNSECURED_REJOIN;
                            }
                            else
                            {
                                // Device does not has the support for High Security. Only Standard Security is supported.
                                params.APSME_UPDATE_DEVICE_request.Status = ( params.NLME_JOIN_indication.secureRejoin ) ?STANDARD_DEVICE_SECURED_REJOIN : STANDARD_DEVICE_UNSECURED_REJOIN;
                            }
                        }
                    }
                    
                    /* need to preserve long and short device address for c32 else they may be overwritten overwritting */
                    {
                        SHORT_ADDR  tempAddr;
                        LONG_ADDR   tempLongAddr;
                        BYTE        index;
                        
                        tempAddr.Val = params.NLME_JOIN_indication.NetworkAddress.Val;
                        
                        for(index = 0; index < 8; index++)
                        {
                            tempLongAddr.v[index] = params.NLME_JOIN_indication.ExtendedAddress.v[index];
                        }
                        params.APSME_UPDATE_DEVICE_request.DeviceShortAddress   = tempAddr;
                        params.APSME_UPDATE_DEVICE_request.DeviceAddress        = tempLongAddr;
                        params.APSME_UPDATE_DEVICE_request.DestAddress = current_SAS.spas.TrustCenterAddress;
                        nextPrimitive = APSME_UPDATE_DEVICE_request;
                    } 
                }//#endif  // I_AM_TRUST_CENTER
            #else // I_SUPPORT_SECURITY
                nextPrimitive = NO_PRIMITIVE;
            #endif // I_SUPPORT_SECURITY
		}//#endif  // I_AM_COORDINATOR
            return nextPrimitive;
            break;

            case ZDO_DATA_indication:
            {
                    #ifdef I_SUPPORT_BINDINGS
                    BOOL        discardMessage;
                    #endif

                    // Limitation - the AF command frame allows more than one transaction to be included
                    // in a frame, but we can only generate one response.  Therefore, we will support only
                    // one transaction per ZDO frame.
                    if ((params.ZDO_DATA_indication.ClusterId.Val & 0x8000) != 0x0000)
                    {
                        switch( params.ZDO_DATA_indication.ClusterId.Val )
                        {
                            /* application doesn't need to see this response so stack handles it */
                            case MGMT_PERMIT_JOINING_rsp:
                                ZDODiscardRx();
                                return NO_PRIMITIVE; 
                                break;
                                

                            case SYSTEM_SERVER_DISCOVERY_rsp:
                                /* Change the Network manager address after receiving
                                   SYSTEM_SERVER_DISCOVERY_rsp */
                                currentNeighborTableInfo.nwkManagerAddr.Val = \
                                    params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val;
                                PutNeighborTableInfo();
                                return APSDE_DATA_indication;

                            #ifdef I_SUPPORT_BINDINGS
                            {
                                APS_ADDRESS_MAP currentAPSAddress1;
                                case NWK_ADDR_rsp:
                                    discardMessage = FALSE;

                                    /* For Zigbee 2006: For End_device Binding to work, we must save
                                     * the long/short address in APS table else the bind_req will
                                     * not have short address to use in its source binding table
                                    */
                                    currentAPSAddress1.shortAddr.v[0] = *(params.ZDO_DATA_indication.asdu+10);
                                    currentAPSAddress1.shortAddr.v[1] = *(params.ZDO_DATA_indication.asdu+11);

                                    currentAPSAddress1.longAddr.v[0] = *(params.ZDO_DATA_indication.asdu+2);
                                    currentAPSAddress1.longAddr.v[1] = *(params.ZDO_DATA_indication.asdu+3);
                                    currentAPSAddress1.longAddr.v[2] = *(params.ZDO_DATA_indication.asdu+4);
                                    currentAPSAddress1.longAddr.v[3] = *(params.ZDO_DATA_indication.asdu+5);
                                    currentAPSAddress1.longAddr.v[4] = *(params.ZDO_DATA_indication.asdu+6);
                                    currentAPSAddress1.longAddr.v[5] = *(params.ZDO_DATA_indication.asdu+7);
                                    currentAPSAddress1.longAddr.v[6] = *(params.ZDO_DATA_indication.asdu+8);
                                    currentAPSAddress1.longAddr.v[7] = *(params.ZDO_DATA_indication.asdu+9);
									
									printf("\n\rAddr Req Resp Short Address = ");
									PrintChar(currentAPSAddress1.shortAddr.v[0]);
									PrintChar(currentAPSAddress1.shortAddr.v[1]);
									
									{
										int Tp;
										printf("\n\rAddr Req Resp Long Address = ");
										for(Tp=7;Tp>=0;Tp--)
										{
											PrintChar(currentAPSAddress1.longAddr.v[Tp]);
										}
										printf("\n\r");
									}
									AddNWKAddrReq(currentAPSAddress1.shortAddr,currentAPSAddress1.longAddr);

                                    /* For Zigbee 2006:  If a new device with the same old longAddress address
                                    * joins the PAN, then make sure the old short address is no longer used and is
                                    * overwritten with the new shortAddress & longAddress combo
                                    */
                                    if ( NWKLookupNodeByLongAddr(&currentAPSAddress1.longAddr) != INVALID_NEIGHBOR_KEY )
                                    {
                                        currentNeighborRecord.shortAddr = currentAPSAddress1.shortAddr;
                                        PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
                                    }
                                    else if( LookupAPSAddress(&currentAPSAddress1.longAddr) == TRUE)
                                    {
                                        for( i = 0; i < apscMaxAddrMapEntries; i++)
                                        {
                                            #ifdef USE_EXTERNAL_NVM
                                                GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                                            #else
                                                GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                                            #endif
                                            if (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&currentAPSAddress1.longAddr, 8 ))
                                            {
                                                /* overwrite the old with the new short/long address combo  */
                                                #ifdef USE_EXTERNAL_NVM
                                                    PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress1);
                                                #else
                                                    PutAPSAddress( &apsAddressMap[i], &currentAPSAddress1 );
                                                #endif
                                            }
                                        }

                                    }
                                    else
                                    {
                                        /* LongAddress was not in the APS Table, so just save it */
                                        APSSaveAPSAddress(&currentAPSAddress1);
                                    }


                                    // See if this is a NWK_ADDR_rsp for a NWK_ADDR_req that was sent for a bind operation.
                                    // Note - we are not doing a lot of error checking here.
                                    // if (zdoStatus.flags.bits.bBinding &&
                                        // /* For Zigbee 2006: These offsets are different since no MSG/KVP + Count */
                                        // (*(params.ZDO_DATA_indication.asdu+1) == SUCCESS))
                                    // {
                                        //See if one of our addresses has responded.  Note that the source and destination address
                                       // could be the same.
                                        // if (!memcmp( (void *)(params.ZDO_DATA_indication.asdu+2), (void *)&(pBindInProgressInfo->sourceAddressLong.v[0]), 8 ))
                                        // {
                                            //This is the response to the source address request
                                            // /* For Zigbee 2006: These offsets are different since no MSG/KVP + Count */
                                            // pBindInProgressInfo->sourceAddressShort.v[0] = *(params.ZDO_DATA_indication.asdu+10);
                                            // pBindInProgressInfo->sourceAddressShort.v[1] = *(params.ZDO_DATA_indication.asdu+11);
                                            // discardMessage = TRUE;
                                        // }
                                        // if (!memcmp( (void *)(params.ZDO_DATA_indication.asdu+2), (void *)&(pBindInProgressInfo->destinationAddressLong.v[0]), 8 ))
                                        // {
                                            //This is the response to the destination address request
                                            // pBindInProgressInfo->destinationAddressShort.v[0] = *(params.ZDO_DATA_indication.asdu+10);
                                            // pBindInProgressInfo->destinationAddressShort.v[1] = *(params.ZDO_DATA_indication.asdu+11);
                                            // discardMessage = TRUE;
                                        // }
                                    // }
									
									discardMessage = TRUE;

                                    if (discardMessage)
                                    {
                                        ZDODiscardRx();
                                        break;
                                    }
                                }
                            #endif
                            // These are ZDO responses that the application has requested.
                            // Send them back to the user.  The parameters are all in place.

                            return APSDE_DATA_indication;
                            break;

                            #if I_SUPPORT_FREQUENCY_AGILITY == 1

                                case MGMT_NWK_UPDATE_notify:
                                    {
                                        BYTE        status;
                                        DWORD_VAL   ScannedChannels;
                                        WORD_VAL    TotalTransmissions;
                                        WORD_VAL    FailedTransmission;
                                        BYTE        ChannelListCount;
                                        DWORD       ChannelMask = 0x00000800;

                                        nextPrimitive = NO_PRIMITIVE;

                                        /* Only NetworkManager can process MGMT_NWK_UPDATE_notify */
                                        if (!VerifyNetworkManager())
                                        {
                                            ZDODiscardRx();
                                            return NO_PRIMITIVE;
                                        }

                                        ZDOGet();       // Sequence number
                                        status = ZDOGet();

                                        ENERGY_DETECT_RECORD *EdRecordPtr = EdRecords;

                                        for(i = 0; i < 4; i++)
                                        {
                                            ScannedChannels.v[i] = ZDOGet();
                                        }
                                        TotalTransmissions.v[0] = ZDOGet();   // total transmission
                                        TotalTransmissions.v[1] = ZDOGet();
                                        FailedTransmission.v[0] = ZDOGet();   // total transmission failures.
                                        FailedTransmission.v[1] = ZDOGet();
                                        ChannelListCount        = ZDOGet();   // channel list count

                                        for(i = 0; i < 16; i++)
                                        {
                                            EdRecordPtr->EnergyReading[i] = 0xFF;
                                        }

                                        /* Store the energy received */
                                        for(i = 0; i < 16; i++)
                                        {
                                            if( ScannedChannels.Val & ChannelMask )
                                            {
                                                /* Store the energy only of allowed channels */
                                                if (current_SAS.spas.ChannelMask.Val & ChannelMask)
                                                {
                                                    EdRecordPtr->EnergyReading[i] = ZDOGet();
                                                }
                                                else
                                                {
                                                    ZDOGet();
                                                }
                                            }
                                            ChannelMask = ChannelMask << 1;
                                        }
                                        /* Exclude channel 26 */
                                        EdRecordPtr->EnergyReading[15] = 0xFF;
                                        nextPrimitive = HandleMgmtNwkUpdateNotify(FALSE, status, ScannedChannels,
                                            TotalTransmissions, FailedTransmission, ChannelListCount);
                                        ZDODiscardRx();
                                        return nextPrimitive;
                                    }
                                    break;

                            #endif
                            default:
                                // These are ZDO responses that the application has requested.
                                // Send them back to the user.  The parameters are all in place.
                               #ifdef I_SUPPORT_BINDINGS
                                    if(!zdoStatus.flags.bits.bEndDeviceBinding && (params.ZDO_DATA_indication.ClusterId.Val == BIND_rsp)  && SentBindRequest)
                                    {
                                        SentBindRequest = 0;
                                        goto ret_indy;
                                    }
                                    else if(!zdoStatus.flags.bits.bEndDeviceBinding && (params.ZDO_DATA_indication.ClusterId.Val == UNBIND_rsp)  && SentBindRequest )
                                    {
                                        SentBindRequest = 0;
                                        goto ret_indy;
                                    }
                                #endif

                                if (  (params.ZDO_DATA_indication.ClusterId.Val == UNBIND_rsp) ||
                                      (params.ZDO_DATA_indication.ClusterId.Val == BIND_rsp)  ||
                                       (params.ZDO_DATA_indication.ClusterId.Val == UNBIND_req) ||
                                       (params.ZDO_DATA_indication.ClusterId.Val == BIND_rsp)  )
                                 {
                                     goto contchecking;
                                 }
                #ifdef I_SUPPORT_BINDINGS
                    ret_indy:
                #endif
                                return APSDE_DATA_indication;
                        }
                        break;
                    }

                    // should always be EP0 sourceEP                = params.ZDO_DATA_indication.SrcEndpoint;
contchecking:       sequenceNumber            = ZDOGet();
                switch (params.ZDO_DATA_indication.ClusterId.Val)
                {
                        case NWK_ADDR_req:
                        {
                                LONG_ADDR   myLongAddr;
                                LONG_ADDR   IEEEAddr;
                                SHORT_ADDR  nwkAddr;
                                BYTE        matchFound;
                                BYTE        sendRsp = FALSE;
                                BYTE        requestType;
                                BYTE        startIndex;
                                for (i=0; i<8; i++)
                                {
                                    IEEEAddr.v[i] = ZDOGet();
                                }
                                requestType = ZDOGet();
                                startIndex  = ZDOGet();

                                // Check whether the longAddr is my own address
                                GetMACAddress(&myLongAddr);
                                matchFound = memcmp( (void *)&IEEEAddr.v[0], (void *)&myLongAddr.v[0], 8 );
                                if ( !matchFound )
                                {
                                    // Memory matched, i.e IEEEAddr is my own address.
                                    sendRsp = TRUE;
                                    nwkAddr.Val = macPIB.macShortAddress.Val;
                                }
                                #ifndef I_AM_END_DEVICE
                                    else if ( IsThisLongAddressKnown(&IEEEAddr, &nwkAddr.v[0], 0x02 ) ) // 0x02 means search only in Neighbor table
                                    {
                                        if ( ( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ) &&
                                             ( currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE ) &&
                                             ( currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle == 0 ) )
                                        {
                                            // IEEEAddr matched with one of my sleeping child end device address.
                                            // So, we need to send response on behalf of my sleeping child end device.
                                            sendRsp = TRUE;

                                            // We should explicitely overwrite teh requestType to SINGLE_DEVICE_RESPONSE.
                                            // This is because, when IEEEAddress matches for one of the child end device,
                                            // then NumOfAssocDevices should be zero, because end devices will never have
                                            // child devices.
                                            requestType = SINGLE_DEVICE_RESPONSE;
                                        }
                                    }
                                #endif // #ifndef I_AM_END_DEVICE
                                if ( sendRsp )
                                {
                                    FinishAddressResponses( NWK_ADDR_rsp, IEEEAddr, nwkAddr, requestType, startIndex );
                                    // This request is broadcast, so we must request an APS ACK on the response.
                                    params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
                                    nextPrimitive = APSDE_DATA_request;
                                }
                            }
                            break;

                        case IEEE_ADDR_req:
                            if (IsThisMyShortAddr())
                            {
                                BYTE        requestType;
                                BYTE        startIndex;
                                LONG_ADDR   IEEEAddr;
                                SHORT_ADDR  nwkAddr;

                                requestType = ZDOGet();
                                startIndex  = ZDOGet();

                                GetMACAddress(&IEEEAddr);
                                nwkAddr.Val = macPIB.macShortAddress.Val;

                                FinishAddressResponses( IEEE_ADDR_rsp, IEEEAddr, nwkAddr, requestType, startIndex  );
                                nextPrimitive = APSDE_DATA_request;
                            }
                            break;

                        case NODE_DESC_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if (IsThisMyShortAddr())
                            {
                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = SUCCESS;
                                TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                ProfileGetNodeDesc( &TxBuffer[TxData] );

                                #if defined(__C30__) || defined(__C32__)
                                    TxBuffer[TxData+3] = TxBuffer[TxData+4];
                                    TxBuffer[TxData+4] = TxBuffer[TxData+5];
                                    TxBuffer[TxData+5] = TxBuffer[TxData+6];
                                    TxBuffer[TxData+6] = TxBuffer[TxData+8];
                                    TxBuffer[TxData+7] = TxBuffer[TxData+9];
                                    TxBuffer[TxData+8] = TxBuffer[TxData+10];
                                    TxBuffer[TxData+9] = TxBuffer[TxData+11];
                                    TxBuffer[TxData+10] = TxBuffer[TxData+12];
                                    TxBuffer[TxData+11] = TxBuffer[TxData+13];
                                    TxBuffer[TxData+12] = TxBuffer[TxData+14];

                                #endif

                                TxData += sizeof_NODE_DESCRIPTOR;

                                PrepareMessageResponse( NODE_DESC_rsp );
                                nextPrimitive = APSDE_DATA_request;
                            }
                            break;

                        case POWER_DESC_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if (IsThisMyShortAddr())
                            {
                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = SUCCESS;
                                TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                ProfileGetNodePowerDesc( &TxBuffer[TxData] );
                                TxData += sizeof(NODE_POWER_DESCRIPTOR);

                                PrepareMessageResponse( POWER_DESC_rsp );
                                nextPrimitive = APSDE_DATA_request;
                            }

                            break;

                        case SIMPLE_DESC_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if ( IsThisMyShortAddr())
                            {
                                BYTE                    endPoint;


                                endPoint = ZDOGet();

                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                if ((endPoint > 0) && (endPoint <= 240))
                                {
                                    // Find the simple descriptor of the desired endpoint.  Note that we cannot just shove this
                                    // into a message buffer, because it may have extra padding in the cluster lists.
                                    i = 0;
                                    do
                                    {
                                        ProfileGetSimpleDesc( &simpleDescriptor, i );
                                        i++;
                                    } while ((simpleDescriptor.Endpoint != endPoint) &&
                                           (i <= NUM_DEFINED_ENDPOINTS));
                                    if (i > NUM_DEFINED_ENDPOINTS)
                                    {
                                        // Load result code
                                        TxBuffer[TxData++] = ZDO_NOT_ACTIVE;
                                        goto SendInactiveEndpoint;
                                    }

                                    // Load result code
                                    TxBuffer[TxData++] = SUCCESS;

                                    // Load our short address.
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                    // Load length of simple descriptor.
                                    TxBuffer[TxData++] = SIMPLE_DESCRIPTOR_BASE_SIZE +
                                            ( (simpleDescriptor.AppInClusterCount)*2 ) + ( (simpleDescriptor.AppOutClusterCount) *2 );

                                    // Load the descriptor. Since we can't send extra padding in the cluster lists,
                                    // we can't load the descriptor as one big array.
                                    TxBuffer[TxData++] = simpleDescriptor.Endpoint;

                                    TxBuffer[TxData++] = simpleDescriptor.AppProfId.byte.LB;
                                    TxBuffer[TxData++] = simpleDescriptor.AppProfId.byte.HB;
                                    TxBuffer[TxData++] = simpleDescriptor.AppDevId.byte.LB;
                                    TxBuffer[TxData++] = simpleDescriptor.AppDevId.byte.HB;

                                    TxBuffer[TxData++] = simpleDescriptor.AppDevVer | (simpleDescriptor.AppFlags << 4);
                                    /* For Zigbee 2006: clusterID needs to now be WORD size not BYTE Size 3.25 */
                                    TxBuffer[TxData++] = simpleDescriptor.AppInClusterCount;
                                    memcpy( (void *)&TxBuffer[TxData], (void *)simpleDescriptor.AppInClusterList, simpleDescriptor.AppInClusterCount * sizeof(WORD_VAL) );
                                    TxData += simpleDescriptor.AppInClusterCount * sizeof(WORD_VAL);
                                    TxBuffer[TxData++] = simpleDescriptor.AppOutClusterCount;
                                    memcpy( (void *)&TxBuffer[TxData], (void *)simpleDescriptor.AppOutClusterList, simpleDescriptor.AppOutClusterCount * sizeof(WORD_VAL));
                                    TxData += simpleDescriptor.AppOutClusterCount * sizeof(WORD_VAL);

                                    PrepareMessageResponse( SIMPLE_DESC_rsp );
                                    nextPrimitive = APSDE_DATA_request;
                                }
                                else
                                {
                                    // Invalid or Inactive Endpoint

                                    // Load result code
                                    TxBuffer[TxData++] = ZDO_INVALID_EP;

SendInactiveEndpoint:
                                    // Load our short address.
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                    // Indicate no descriptor
                                    TxBuffer[TxData++] = 0;

                                    PrepareMessageResponse( SIMPLE_DESC_rsp );
                                    nextPrimitive = APSDE_DATA_request;
                                }
                            }
                            break;

                        case ACTIVE_EP_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if (IsThisMyShortAddr())
                            {
                                handleActiveEPReq();
                                nextPrimitive = APSDE_DATA_request;
                            }
                            break;

                        case MATCH_DESC_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            #if 1
                            if (IsThisMyShortAddr())
                            {
                                if( handleMatchDescReq() )
                                {
                                    nextPrimitive = APSDE_DATA_request;
                                }
                            }
                            #endif
                            break;

                        case COMPLEX_DESC_req:
                        case USER_DESC_req:
                        case USER_DESC_set:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if (IsThisMyShortAddr())
                            {
                                #ifdef INCLUDE_OPTIONAL_SERVICE_DISCOVERY_REQUESTS 
                                    // Skip over the header info
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                    TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;


                                    PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                    nextPrimitive = APSDE_DATA_request;
                                #else

                                    // Skip over the header info
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                    TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;

                                    // Load our short address.
                                    //TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                    //TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                    PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                    nextPrimitive = APSDE_DATA_request;
                                #endif

                            }
                            break;

                        case END_DEVICE_annce:
                            /* Zigbee 2006: support is now mandatory test case 3.25 */
                            {
                                APS_ADDRESS_MAP currentAPSAddress1;
                                BYTE *pTemp, i;
                                
                                currentAPSAddress1.shortAddr.v[0] = ZDOGet();
                                currentAPSAddress1.shortAddr.v[1] = ZDOGet();

                                pTemp = &currentAPSAddress1.longAddr.v[0];
                                for ( i = 0 ; i < 8 ; i++ )
                                {
                                    *pTemp++ = ZDOGet();
                                }
								
								printf("\n\rEnd Device Anounce Short Address = ");
								PrintChar(currentAPSAddress1.shortAddr.v[0]);
								PrintChar(currentAPSAddress1.shortAddr.v[1]);
									
								{
									int Tp;
									printf("\n\rEnd Device Anounce Address = ");
									for(Tp=7;Tp>=0;Tp--)
									{
										PrintChar(currentAPSAddress1.longAddr.v[Tp]);
									}
									printf("\n\r");
								}
								
								AddEndDeviceAnnceDevice(currentAPSAddress1.shortAddr,currentAPSAddress1.longAddr);

                                #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                #ifndef I_AM_END_DEVICE
                                    if( IsThereAddressConflict( currentAPSAddress1.shortAddr , &currentAPSAddress1.longAddr ) )
                                    {
                                        // Address Conflict
                                        if ( nwkStatus.addressConflictType == LOCAL_ADDRESS_CONFLICT )
                                        {
                                            if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
                                                nextPrimitive = ResolveLocalAddressConflict();
                                            }//#endif
                                        // If a new device has come up with my address, Both of us shall change to new address
                                        // I'll do a nwk status cmd for the other devic to change its address
                                        // I changed my own address above.
                                        }
                                        else if ( nwkStatus.addressConflictType == ADDRESS_CONFLICT_ON_A_CHILD_END_DEVICE )
                                        {
                                            addressConflictOnChildEndDevice.Val = currentAPSAddress1.shortAddr.Val;
                                            zdoStatus.flags.bits.bSendUnsolicitedRejoinRsp = TRUE;
                                        }
                                        else if ( nwkStatus.addressConflictType == REMOTE_ADDRESS_CONFLICT )
                                        {
                                        }
                                        nextPrimitive = PrepareNwkStatusCmdBroadcast( NWK_STATUS_ADDRESS_CONFLICT , currentAPSAddress1.shortAddr );
                                        nwkStatus.addressConflictType = NO_CONFLICT;
                                    }
                                    else
                                #endif
                                #endif

                                /* For Zigbee 2006:  If a new device with the same old longAddress address
                                 * joins the PAN, then make sure the old short address is no longer used and is
                                 * overwritten with the new shortAddress & longAddress combo
                                */
                                {
                                    if ( NWKLookupNodeByLongAddr(&currentAPSAddress1.longAddr) != INVALID_NEIGHBOR_KEY )
                                    {
                                        currentNeighborRecord.shortAddr = currentAPSAddress1.shortAddr;
                                        PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
                                    }
                                    else if( LookupAPSAddress(&currentAPSAddress1.longAddr) == TRUE)
                                    {
                                        for( i = 0; i < apscMaxAddrMapEntries; i++)
                                        {
                                            #ifdef USE_EXTERNAL_NVM
                                                GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                                            #else
                                                GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                                            #endif
                                            if (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&currentAPSAddress1.longAddr, 8 ))
                                            {
                                                /* overwrite the old with the new short/long address combo  */
                                                #ifdef USE_EXTERNAL_NVM
                                                    PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress1);
                                                #else
                                                    PutAPSAddress( &apsAddressMap[i], &currentAPSAddress1 );
                                                #endif
                                            }
                                        }
                                    }
                                    else
                                    {
                                        /* LongAddress was not in the APS Table, so just save it */
                                        APSSaveAPSAddress(&currentAPSAddress1);
                                    }
                                    nextPrimitive = NO_PRIMITIVE;
                                }
                             }
                            break;


                            case SYSTEM_SERVER_DISCOVERY_req:
                            {
                                WORD_VAL ServerMask;
                                NODE_DESCRIPTOR NodeDescriptor;

                                ServerMask.v[0] = ZDOGet();
                                ServerMask.v[1] = ZDOGet();

                                /* Get the server mask field in the node descriptor */
                                ProfileGetNodeDesc(&NodeDescriptor);

                                /* If any field of requested server mask is set on
                                   node descriptor server mask field on this device
                                   send the SYSTEM_SERVER_DISCOVERY_rsp */
                                ServerMask.Val &= NodeDescriptor.NodeServerMask.Val;
                                if( ServerMask.Val )
                                {
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                    /* Prepare SYSTEM_SERVER_DISCOVERY_rsp frame */
                                    TxBuffer[TxData++] = SUCCESS;
                                    TxBuffer[TxData++] = ServerMask.v[0];
                                    TxBuffer[TxData++] = ServerMask.v[1];
                                    PrepareMessageResponse( SYSTEM_SERVER_DISCOVERY_rsp );
                                    nextPrimitive = APSDE_DATA_request;
                                }
                            }
                            break;


                        /* For Zigbee 2006 FFD can now participate in source bindings */
//                        #if defined(I_AM_COORDINATOR) || defined(I_AM_ROUTER) || defined (I_AM_END_DEVICE)

                        #define BR_OFFSET_SOURCE_ADDRESS        0x00
                        #define BR_OFFSET_DEST_ADDR_MODE        0x0B
                        #define BR_OFFSET_DESTINATION_ADDRESS   0x0C

                        #ifdef SUPPORT_END_DEVICE_BINDING
                        case END_DEVICE_BIND_req:
                            nextPrimitive = ProcessEndDeviceBind( NULL );
                            break;
                        #endif

                        case BIND_req:
                            #if !defined(I_SUPPORT_BINDINGS)
                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( BIND_rsp );
                                nextPrimitive = APSDE_DATA_request;
                            #else
                                {
                                    SHORT_ADDR dstAddr;
                                    if( params.ZDO_DATA_indication.SrcAddrMode == APS_ADDRESS_64_BIT )
                                    {
                                        if( APSFromLongToShort(&params.ZDO_DATA_indication.SrcAddress.LongAddr) )
                                        {
                                            dstAddr = currentAPSAddress.shortAddr;
                                        }
                                        else
                                        {
                                            nextPrimitive = NO_PRIMITIVE;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        dstAddr = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
                                    }

                                    if (ProcessBindAndUnbind( BIND_FROM_EXTERNAL | BIND_NODES,
                                        (LONG_ADDR *)&params.ZDO_DATA_indication.asdu[BR_OFFSET_SOURCE_ADDRESS],
                                        params.ZDO_DATA_indication.asdu[BR_OFFSET_DEST_ADDR_MODE],
                                        (ADDR *)&params.ZDO_DATA_indication.asdu[BR_OFFSET_DESTINATION_ADDRESS] ))
                                    {
                                        // Skip over the header info
                                        ZigBeeBlockTx();
                                        TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                        TxBuffer[TxData++] = params.ZDO_BIND_req.Status;
                                        params.ZDO_DATA_indication.SrcAddress.ShortAddr = dstAddr;
                                        PrepareMessageResponse( BIND_rsp );
                                        nextPrimitive = APSDE_DATA_request;
                                    }
                                    else
                                    {
                                        nextPrimitive = NO_PRIMITIVE;
                                    }
                                }
                            #endif
                            break;


                        case BIND_rsp:
                            #ifdef SUPPORT_END_DEVICE_BINDING

                                pEndBindRequest->lastTick = TickGet();
                                 pEndBindRequest->EBstate  -= 1;

                            #endif
                            break;

                        case UNBIND_rsp:
                            #ifdef SUPPORT_END_DEVICE_BINDING

                             if (pEndBindRequest->EBstate == WAIT_FOR_TEST_UNBIND_RESPONSE_FROM_FIRST_DEVICE)

                             {
                                  if( ZDOGet() == SUCCESS )
                                  {
                                      pEndBindRequest->EBstate  = SEND_UNBIND_TO_FIRST_DEVICE;
                                      pEndBindRequest->pFirstEndDeviceBindReq->MatchCount--;
                                  }
                                  else
                                  {
                                      pEndBindRequest->EBstate  = SEND_BIND_TO_FIRST_DEVICE;
                                  }
                             }
                            else if (pEndBindRequest->EBstate == WAIT_FOR_TEST_UNBIND_RESPONSE_FROM_SECOND_DEVICE)

                             {
                                  if( ZDOGet() == SUCCESS )
                                  {
                                      pEndBindRequest->EBstate  = SEND_UNBIND_TO_SECOND_DEVICE;
                                      pEndBindRequest->pSecondEndDeviceBindReq->MatchCount--;
                                  }
                                  else
                                  {
                                      pEndBindRequest->EBstate  = SEND_BIND_TO_SECOND_DEVICE;
                                  }
                             }
                             else
                             {
                                 pEndBindRequest->EBstate  -= 1;
                             }
                            #endif
                            break;

                        case UNBIND_req:
                            #if !defined(I_SUPPORT_BINDINGS) || (MAX_BINDINGS == 0)
                                // Skip over the header info
                                ZigBeeBlockTx();
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( UNBIND_rsp );
                                nextPrimitive =  APSDE_DATA_request;
                            #else
                            {
                                SHORT_ADDR dstAddr;

                                if( params.ZDO_DATA_indication.SrcAddrMode == APS_ADDRESS_64_BIT )
                                {
                                    if( APSFromLongToShort(&params.ZDO_DATA_indication.SrcAddress.LongAddr) )
                                    {
                                        dstAddr = currentAPSAddress.shortAddr;
                                    }
                                    else
                                    {
                                        nextPrimitive = NO_PRIMITIVE;
                                        break;
                                    }
                                }
                                else
                                {
                                   dstAddr = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
                                }
                                if (ProcessBindAndUnbind( UNBIND_FROM_EXTERNAL | UNBIND_NODES,
                                    (LONG_ADDR *)&params.ZDO_DATA_indication.asdu[BR_OFFSET_SOURCE_ADDRESS],
                                    params.ZDO_DATA_indication.asdu[BR_OFFSET_DEST_ADDR_MODE],
                                    (ADDR *)&params.ZDO_DATA_indication.asdu[BR_OFFSET_DESTINATION_ADDRESS] ))
                                {
                                    // Skip over the header info
                                    ZigBeeBlockTx();
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                    TxBuffer[TxData++] = params.ZDO_UNBIND_req.Status;
                                    params.ZDO_DATA_indication.SrcAddress.ShortAddr = dstAddr;
                                    PrepareMessageResponse( UNBIND_rsp );
                                    nextPrimitive =  APSDE_DATA_request;
                                }
                                else
                                {
                                    nextPrimitive = NO_PRIMITIVE;
                                }
                            }
                            #endif

                            break;

                        //#endif
                        case MGMT_PERMIT_JOINING_req:
                        {
                            /* will hold the primitive's parameter */
                            BYTE        permitDuration;
                            BYTE        TC_Significance;
                            
                            #ifdef I_AM_END_DEVICE
                                ZDODiscardRx();
                                nextPrimitive = NO_PRIMITIVE;
                                break;
                            #endif
                            
                            
                            /* The destination address specified by the sender */ 
                            WORD_VAL    DstAddr;
                            
                            /* pick up duration and TC_Sig from ZDO payload */
                            permitDuration  = ZDOGet();
                            TC_Significance = ZDOGet();
                            
                            /* Setup the permit joining process */
                            params.NLME_PERMIT_JOINING_request.PermitDuration = permitDuration;
                            params.NLME_PERMIT_JOINING_request._updatePayload = FALSE;
                            nextPrimitive = NLME_PERMIT_JOINING_request;
                            nwkStatus.mgmtPermitJoiningRspPending = TRUE;
                            
                            /* Save off the source's address to be used in the mgmtpermitjoiningrsp command */
                            if( params.ZDO_DATA_indication.SrcAddrMode == APS_ADDRESS_64_BIT )
                            {
                                if( APSFromLongToShort(&params.ZDO_DATA_indication.SrcAddress.LongAddr) )
                                {
                                    nwkStatus.mgmtPermitJoiningRspDstAddr = currentAPSAddress.shortAddr;
                                }
                                else
                                {
                                    /* Send it back to Coordinator you don't know about the real source device */
                                    nwkStatus.mgmtPermitJoiningRspDstAddr.v[0] = 0x00;
                                    nwkStatus.mgmtPermitJoiningRspDstAddr.v[1] = 0x00;                              
                                }
                            }
                            else
                            {
                               nwkStatus.mgmtPermitJoiningRspDstAddr = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
                            }
                            
                            /* The apsde_data_indication mirrors the zdo_data_indication 
                             * therefore the sender's specified destination address remains in place
                            */
                            DstAddr.Val     =  nwkStatus.mgmtPermitJoiningRequestDestination.Val;
                            
                            /* if this was a broadcast request, do not send a response */
                            if( (DstAddr.Val == 0xfffc) || (DstAddr.Val == 0xfffd) || (DstAddr.Val == 0xffff) )
                            {
                               nwkStatus.mgmtPermitJoiningRspPending = FALSE; 
                            }
                            
                            break; 
                        }    
                        
                        
                        case MGMT_LQI_req:
                        {
                            #ifdef I_AM_END_DEVICE
                                ZDODiscardRx();
                                nextPrimitive = NO_PRIMITIVE;
                                break;
                            #endif   
                            
                            handleLQIReq();
                            nextPrimitive = APSDE_DATA_request;
                            break;
                            
                        }
                        
                        
                        case MGMT_RTG_req:
                        {
                            #ifdef I_AM_END_DEVICE
                                ZDODiscardRx();
                                nextPrimitive = NO_PRIMITIVE;
                                break;
                            #else
                                /* a 256K PIC or greater is needed for this primitive */   
                                #if defined (__PIC24FJ256GB110__) || defined (__PIC24FJ256GB106__) || defined(__PIC32MX__)
                                    handleRTGReq();
                                    nextPrimitive = APSDE_DATA_request;
                                    break;
                                #else
                                    // Skip over the header info
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;
    
                                    TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                    PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                    nextPrimitive = APSDE_DATA_request;
                                    break;
                                #endif 
                            #endif
                            
                        }
                         
                        
                        case MGMT_NWK_DISC_req:
                        case MGMT_BIND_req:
                        case MGMT_DIRECT_JOIN_req:
                        case MGMT_LEAVE_req:
                            #ifdef INCLUDE_OPTIONAL_NODE_MANAGEMENT_SERVICES
                                // Right now this is not supported at all.  When added,
                                // put full functionality code here and change status.

                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                nextPrimitive = APSDE_DATA_request;
                            #else

                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                nextPrimitive = APSDE_DATA_request;
                            #endif
                            break;


                        //#ifdef I_AM_FFD
                            /* Any FFD device can receive MGMT_NWK_UPDATE_req for switching channel.
                               MGMT_NWK_UPDATE_req for scanning the channel and changing the
                               network manager can only received by frequency agility supporting
                               devices. */
                            case MGMT_NWK_UPDATE_req:
                            if(NOW_I_AM_A_FFD()){
                                nextPrimitive = NO_PRIMITIVE;
                                GetNeighborTableInfo();
                                if (params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val != currentNeighborTableInfo.nwkManagerAddr.Val)
                                {
                                    ZDODiscardRx();
                                    return NO_PRIMITIVE;
                                }

                            #if I_SUPPORT_FREQUENCY_AGILITY == 1
                                /* MGMT_NWK_UPDATE_req will not be processed on network manager */
                                if (VerifyNetworkManager())
                                {
                                    ZDODiscardRx();
                                    return NO_PRIMITIVE;
                                }
                            #endif

                                /* Store the scan channels list received from the frame */
                                for(i = 0; i < 4; i++)
                                {
                                    mgmtNwkUpateRequestDetails.ScanChannels.v[i] = ZDOGet();
                                }
                                /* Store the scan duration field received */
                                mgmtNwkUpateRequestDetails.ScanDuration = ZDOGet();

                                /* Verify whether the MGMT_NWK_UPDATE_req for changing the channel */
                                if( mgmtNwkUpateRequestDetails.ScanDuration == 0xFE )
                                {
                                    DWORD channelMask = 1;

                                    i = 0;
                                    while((mgmtNwkUpateRequestDetails.ScanChannels.Val & channelMask) == 0 )
                                    {
                                        /* Get the channel to which network needs to be switched */
                                        channelMask <<= 1;
                                        i++;
                                    }
                                    /* Update the nwkUpdateId of the device */
                                    currentNeighborTableInfo.nwkUpdateId = ZDOGet();
                                    PutNeighborTableInfo();
                                #ifdef I_AM_END_DEVICE
                                    /* If the device is end device just switch the channel
                                       Note: Only active end device can receive this frame */
                                    phyPIB.phyCurrentChannel = i;
                                    PHYSetLongRAMAddr(0x200, (0x02 | (BYTE)((phyPIB.phyCurrentChannel - 11) << 4)));
                                    PHYSetShortRAMAddr(PWRCTL,0x04);
                                    PHYSetShortRAMAddr(PWRCTL,0x00);
                                    StorePersistancePIB();
                                    appNotify.currentChannel = phyPIB.phyCurrentChannel;
                                    appNotification = TRUE;

                                #else
                                    /* Set the beacon payload as nwkUpdateId is changed */
                                    SetBeaconPayload(TRUE);
                                    #if I_SUPPORT_FREQUENCY_AGILITY == 1

                                        /* Channel switch should be done after rebroadcasting
                                           the MGMT_NWK_UPDATE_req. Device will wait for
                                           broadcast delivery time then switches the channel */
                                        nwkStatus.flags.bits.bChannelSwitch = 1;
                                        zdoStatus.flags.bits.bSwitchChannelTimerInProgress = 1;
                                        zdoStatus.SwitchChannelTick = TickGet();

                                        /* The channel to be switched is stored in a global variable. */
                                        NewChannel = i;

                                        nextPrimitive = NO_PRIMITIVE;

                                        /* After switching channel, clear the counters for total packets
                                           transmitted and total transmission failures */
                                        TotalTransmittedPackets.Val = 0;
                                        TotalTransmitFailures.Val = 0;
                                    #else
                                        /* If Router or Coordinator is not supporting frequency agility,
                                           just switch the channel */
                                        phyPIB.phyCurrentChannel = i;
                                        PHYSetLongRAMAddr(0x200, (0x02 | (BYTE)((phyPIB.phyCurrentChannel - 11) << 4)));
                                        PHYSetShortRAMAddr(PWRCTL,0x04);
                                        PHYSetShortRAMAddr(PWRCTL,0x00);
                                        StorePersistancePIB();
                                    #endif
                                #endif
                                }

                            #if I_SUPPORT_FREQUENCY_AGILITY == 1
                                else if( mgmtNwkUpateRequestDetails.ScanDuration <= 0x05 && !params.ZDO_DATA_indication.WasBroadcast )
                                {
                                    if( nwkStatus.flags.bits.bScanRequestFromZDO )
                                    {
                                        /* Scanning in progress */
                                        nextPrimitive = NO_PRIMITIVE;
                                    }
                                    else
                                    {

                                        /* Store the scan count field received */
                                        mgmtNwkUpateRequestDetails.scanCount = ZDOGet();

                                        if (mgmtNwkUpateRequestDetails.scanCount < 6)
                                        {
                                            if ( mgmtNwkUpateRequestDetails.scanCount > 0 )
                                            {
                                                // backup the current channel
                                                phyPIB.phyBackupChannel = phyPIB.phyCurrentChannel;

                                                /* Indicates Scanning issued from ZDO for transmitting
                                                   Mgmt_NWK_Update_Notify. */
                                                nwkStatus.flags.bits.bScanRequestFromZDO = 1;

                                                params.MLME_SCAN_request.ScanType = MAC_SCAN_ENERGY_DETECT;
                                                params.MLME_SCAN_request.ScanChannels.Val = mgmtNwkUpateRequestDetails.ScanChannels.Val;
                                                params.MLME_SCAN_request.ScanDuration = mgmtNwkUpateRequestDetails.ScanDuration;

                                                nextPrimitive = MLME_SCAN_request;
                                                mgmtNwkUpateRequestDetails.scanCount--;
                                            }
                                        }
                                        else
                                        {
                                            /* Invalid request, send the MGMT_NWK_UPDATE_notify
                                               with status as INVALID_REQUEST. */
                                            ZigBeeBlockTx();

                                            TxBuffer[TxData++] = ZDOCounter++;
                                            TxBuffer[TxData++] = ZDO_INV_REQUESTTYPE;

                                            for(i = 0; i < 4; i++)
                                            {
                                                TxBuffer[TxData++] = mgmtNwkUpateRequestDetails.ScanChannels.v[i];
                                            }
                                            TxBuffer[TxData++] = TotalTransmittedPackets.v[0]; // total transmission
                                            TxBuffer[TxData++] = TotalTransmittedPackets.v[1];

                                            TxBuffer[TxData++] = TotalTransmitFailures.v[0];   // transmission failure
                                            TxBuffer[TxData++] = TotalTransmitFailures.v[1];

                                            TxBuffer[TxData++] = 0; // channel count

                                            PrepareMessageResponse( MGMT_NWK_UPDATE_notify );

                                            nextPrimitive = APSDE_DATA_request;
                                        }

                                    }
                                }
                                /* Check whether MGMT_NWK_UPDATE_req is received to change
                                   the network manager. */
                                else if( mgmtNwkUpateRequestDetails.ScanDuration == 0xFF )
                                {
                                     ZDOGet();                 // nwkUpdateId
                                     /* Store the address of new nwkManager */
                                     currentNeighborTableInfo.nwkManagerAddr.v[0] = ZDOGet();
                                     currentNeighborTableInfo.nwkManagerAddr.v[1] = ZDOGet();

                                     /* Verify the network manager address is my address. */
                                     if (currentNeighborTableInfo.nwkManagerAddr.Val == macPIB.macShortAddress.Val)
                                     {
                                        NODE_DESCRIPTOR NodeDescriptor;

                                        /* Update the server mask(Network manager field)
                                           in node descriptor. */
                                        ProfileGetNodeDesc(&NodeDescriptor);
                                        NodeDescriptor.NodeServerMask.Val |= 0x0040;
                                        ProfilePutNodeDesc(&NodeDescriptor);
                                     }

                                     PutNeighborTableInfo();
                                     apsChannelMask.Val = mgmtNwkUpateRequestDetails.ScanChannels.Val;

                                     nextPrimitive = NO_PRIMITIVE;
                                }

                                else
                                {
                                    /* Invalid request, send the MGMT_NWK_UPDATE_notify
                                       with status as INVALID_REQUEST. */
                                    ZigBeeBlockTx();

                                    TxBuffer[TxData++] = ZDOCounter++;
                                    TxBuffer[TxData++] = ZDO_INV_REQUESTTYPE;

                                    for(i = 0; i < 4; i++)
                                    {
                                        TxBuffer[TxData++] = mgmtNwkUpateRequestDetails.ScanChannels.v[i];
                                    }
                                    TxBuffer[TxData++] = TotalTransmittedPackets.v[0]; // total transmission
                                    TxBuffer[TxData++] = TotalTransmittedPackets.v[1];

                                    TxBuffer[TxData++] = TotalTransmitFailures.v[0];   // transmission failure
                                    TxBuffer[TxData++] = TotalTransmitFailures.v[1];

                                    TxBuffer[TxData++] = 0; // channel count

                                    PrepareMessageResponse( MGMT_NWK_UPDATE_notify );

                                    nextPrimitive = APSDE_DATA_request;

                                }
                              #endif /*I_SUPPORT_FREQUENCY_AGILITY == 1 */
                            }
                            break;
                        //#endif /* I_AM_FFD */

                        default:
                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                nextPrimitive = APSDE_DATA_request;
                            break;
                    }

                    ZDODiscardRx();
                }
                break;  // ZDO_DATA_indication

            #if defined (SUPPORT_END_DEVICE_BINDING)

            #endif

            #if defined(I_SUPPORT_BINDINGS)

            case ZDO_BIND_req:
                if (ProcessBindAndUnbind( BIND_FROM_UPPER_LAYERS | BIND_NODES,
                    (LONG_ADDR *)&params.ZDO_BIND_req.SrcAddress,
                    params.ZDO_BIND_req.DstAddrMode,
                    (ADDR *)&params.ZDO_BIND_req.DstAddress ))
                {
                    // If successful, a fake APSDE_DATA_indication was created with the response.
                    nextPrimitive = APSDE_DATA_indication;
                }
                else
                {
                    nextPrimitive = NO_PRIMITIVE;
                }
                break;

            case ZDO_UNBIND_req:
                if (ProcessBindAndUnbind( UNBIND_FROM_UPPER_LAYERS | UNBIND_NODES,
                    (LONG_ADDR *)&params.ZDO_UNBIND_req.SrcAddress,
                    params.ZDO_UNBIND_req.DstAddrMode,
                    (ADDR *)&params.ZDO_UNBIND_req.DstAddress ))
                {
                    // If successful, a fake APSDE_DATA_indication was created with the response.
                    nextPrimitive = APSDE_DATA_indication;
                }
                else
                {
                    nextPrimitive = NO_PRIMITIVE;
                }
                break;
            #endif

            #ifdef I_SUPPORT_SECURITY
                //#if !defined(I_AM_TRUST_CENTER)
                    case APSME_TRANSPORT_KEY_indication:
                    if(!I_AM_TRUST_CENTER){ //{  
	                    BYTE JoinConfirmPending = 0;
                        if (NOW_I_AM_NOT_A_CORDINATOR()){//#if !defined(I_AM_COORDINATOR)
                            JoinConfirmPending = FALSE;
                        }//#endif
                        if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                            PHYTasksPending.bits.PHY_AUTHORIZE = 0;
                        }//#endif
                        switch( params.APSME_TRANSPORT_KEY_indication.KeyType)
                        {
                            case NETWORK_KEY:  // Network key
                            {
                                BYTE    KeyIndex;
                                // handle trust center
                                #if (I_SUPPORT_LINK_KEY == 1)
                                    for(i=0; i < 8; i++ )
                                    {
                                        TCLinkKeyInfo.trustCenterLongAddr.v[i] = params.APSME_TRANSPORT_KEY_indication.SrcAddr.v[i];
                                    }
                                    TCLinkKeyInfo.trustCenterShortAddr.Val = 0x0000;  // Assume always TC is ZC.
                                #endif
                                
                                for (i=0; i<16; i++)
                                {
                                    // We are using KeyVal because in TRANPORT_KEY_inidication the Key field which
                                    // is pointing to the KeyVal is getting corrupted. So, we are forcefully using
                                    // this global variable KeyVal
                                    currentNetworkKeyInfo.NetKey.v[i] = KeyVal.v[i];
									current_SAS.spas.NetworkKey[i] = KeyVal.v[i];
                                }
                                currentNetworkKeyInfo.SeqNumber.v[0] = params.APSME_TRANSPORT_KEY_indication.TransportKeyData.NetworkKey.KeySeqNumber;
                                currentNetworkKeyInfo.SeqNumber.v[1] = nwkMAGICResSeq;
								current_SAS.spas.NetworkKeySeqNum = params.APSME_TRANSPORT_KEY_indication.TransportKeyData.NetworkKey.KeySeqNumber;

                                /* This is done to check whether we got dummy(all zeros)keys */
                                for(i = 0; i < 16; i++)
                                {
                                    if( currentNetworkKeyInfo.NetKey.v[i] != 0x00 )
                                    {
                                        break;
                                    }
                                }

                                if( i != 16 )
                                {
                                    GetNwkActiveKeyNumber(&KeyIndex);

                                    /* Zigbee 2006: KeyIndex will be 0xff during authorization
                                     * so this first key will now become the active key
                                     * otherwise the else part will be executed
                                    */
                                    if( ( KeyIndex != 0x01 && KeyIndex != 0x02 ) ||
                                        ( securityStatus.flags.bits.bAuthorization ) )
                                    //if( KeyIndex == 0xFF )
                                    {
                                        // If KeyIndex is not updated, then Put the Key in the first slot.
                                        KeyIndex = 0x01;
                                        PutNwkActiveKeyNumber(&KeyIndex);
                                        #ifdef USE_EXTERNAL_NVM
                                            SetSecurityKey(0, currentNetworkKeyInfo);
                                        #else
                                            PutNwkKeyInfo( &networkKeyInfo, &currentNetworkKeyInfo );
                                        #endif
                                    }
                                    else
                                    {
                                        // Put Key in the Non Active Slot.
                                        KeyIndex = (KeyIndex == 0x01) ? 0:0;

                                        #ifdef USE_EXTERNAL_NVM
                                            SetSecurityKey(KeyIndex, currentNetworkKeyInfo);
                                        #else
                                            PutNwkKeyInfo( &networkKeyInfo[KeyIndex], &currentNetworkKeyInfo );
                                        #endif
                                    }

                                    /*if ( securityStatus.flags.bits.bAuthorization )
                                    {
                                        PutNwkActiveKeyNumber(&KeyIndex);
                                    }*/
                                }
                                /* On a non preconfigured link key network, request for link key */
                                if(NOW_I_AM_NOT_A_CORDINATOR()){//#if !defined(I_AM_COORDINATOR)
                                    #if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 0
                                        if( securityStatus.flags.bits.bAuthorization )
                                        {
                                            /*Request for link key */

                                            /* time to wait for link key */
                                            AuthorizationTimeout = TickGet();
                                            WaitForAuthentication.Val = PROFILE_apsSecurityTimeoutPeriod;
                                            JoinConfirmPending = FALSE;
                                            memcpy(params.APSME_REQUEST_KEY_request.DestAddress.v
                                            ,current_SAS.spas.TrustCenterAddress.v,sizeof(LONG_ADDR) );
                                            params.APSME_REQUEST_KEY_request.KeyType = TC_LINK_KEY;
                                            return APSME_REQUEST_KEY_request;
                                        }

                                    #else
                                        JoinConfirmPending = securityStatus.flags.bits.bAuthorization;
                                        securityStatus.flags.bits.bAuthorization = 0;
                                        #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                            nwkStatus.nwkLinkStatusPeriod = TickGet();
                                            nwkStatus.moreLinkStatusCmdPending = 0;
                                        #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

                                    #endif
                                }//#endif /*!defined I_AM_COORDINATOR */

                                break;
                            }
                            #if I_SUPPORT_LINK_KEY == 1
                            case TC_LINK_KEY:

                                for (i=0; i < KEY_LENGTH; i++)
                                {
                                    // We are using KeyVal because in TRANPORT_KEY_inidication the Key field which
                                    // is pointing to the KeyVal is getting corrupted. So, we are forcefully using
                                    // this global variable KeyVal
                                    TCLinkKeyInfo.link_key.v[i] = KeyVal.v[i];
                                }
                                /* Reset the frame counter */
                                TCLinkKeyInfo.frameCounter.Val = 0x00;

                                /* reset the flag used for authentication as key is obtained within security time */
                                if (securityStatus.flags.bits.bAuthorization)
                                {
                                    JoinConfirmPending = securityStatus.flags.bits.bAuthorization;
                                    securityStatus.flags.bits.bAuthorization = 0;
                                    #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                        nwkStatus.nwkLinkStatusPeriod = TickGet();
                                        nwkStatus.moreLinkStatusCmdPending = 0;
                                    #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                }
                            break;
                            case APP_LINK_KEY:
                            {
                                APS_KEY_PAIR_DESCRIPTOR LinkKey;
                                for(i=0; i<8; i++)
                                {
                                    LinkKey.PartnerAddress.v[i] =
                                     params.APSME_TRANSPORT_KEY_indication.TransportKeyData.Application_LinkKey.PartnerAddress.v[i];
                                }
                                for (i=0; i<16; i++)
                                {
                                    // We are using KeyVal because in TRANPORT_KEY_inidication the Key field which
                                    // is pointing to the KeyVal is getting corrupted. So, we are forcefully using
                                    // this global variable KeyVal
                                    LinkKey.Link_Key.v[i] = KeyVal.v[i];
                                }
                                /* Store the key */
                                SetAppLinkKey(&LinkKey );
                                return NO_PRIMITIVE;
                            }
                            #endif

                        }
                        if(NOW_I_AM_NOT_A_CORDINATOR()){//#if !defined(I_AM_COORDINATOR)
                            /* Issue join confirm */
                            if(JoinConfirmPending)
                            {
                                params.NLME_JOIN_confirm.Status = SUCCESS;
                                //params.NLME_JOIN_confirm.PANId = macPIB.macPANId;
                                params.NLME_JOIN_confirm.ExtendedPANId = currentNeighborRecord.ExtendedPANID;
                                params.NLME_JOIN_confirm.ActiveChannel = currentNeighborRecord.LogicalChannel;
                                nextPrimitive = NLME_JOIN_confirm;
                            }
                            else
                            {
                               nextPrimitive = NO_PRIMITIVE;
                            }
                        }//#endif
                        break;
                    }
                #endif // !I_AM_TRUST_CENTER

                //#ifdef I_AM_TRUST_CENTER
                    case APSME_UPDATE_DEVICE_indication:
                    if(I_AM_TRUST_CENTER){ //{
                        APS_ADDRESS_MAP currentAPSAddress1;
                        BYTE i;
                        
                        /*Route Record Table entry should be removed for the device for which the 
                        Update Device Indication is received. 
                        Irrespective of the Status of the Update Device Indication, 
                        Status == DEVICE_LEFT: Device has left. So need to store the Route Record.
                        Status == DEVICE_JOINED: Device has joined. Delete old Route Record entry, if exists.
                        Status == DEVICE_REJOINED: Device has re-joined. Delete old Route Record entry, if exists. New entry should be created for this device.
                        Route Record Table entry should be removed*/
                        #if I_SUPPORT_CONCENTRATOR == 1
                            DeleteRouteRecord(params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress);
                        #endif
                        
                        if(params.APSME_UPDATE_DEVICE_request.Status != DEVICE_LEFT )
                        {
							if(CheckDeviceJoiningPermission(params.APSME_UPDATE_DEVICE_indication.DeviceAddress) == FALSE)
							{
								for (i=0; i<8; i++)
                                {
                                    params.APSME_REMOVE_DEVICE_request.ChildAddress.v[i] = params.APSME_UPDATE_DEVICE_indication.DeviceAddress.v[i];
                                    params.APSME_REMOVE_DEVICE_request.ParentAddress.v[i] = params.APSME_UPDATE_DEVICE_indication.SrcAddress.v[7-i];
                                }
                                nextPrimitive = APSME_REMOVE_DEVICE_request;
                                break;
							}
                            if( !AllowJoin ) 
							{
                                for (i=0; i<8; i++)
                                {
                                    params.APSME_REMOVE_DEVICE_request.ChildAddress.v[i] = params.APSME_UPDATE_DEVICE_indication.DeviceAddress.v[i];
                                    params.APSME_REMOVE_DEVICE_request.ParentAddress.v[i] = params.APSME_UPDATE_DEVICE_indication.SrcAddress.v[7-i];
                                }
                                nextPrimitive = APSME_REMOVE_DEVICE_request;
                                break;
                            }
                            params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val =
                                   params.APSME_UPDATE_DEVICE_indication.ParentShortAddress.Val;
                            // if allow to join, send network key to the router with security on
                            for (i=0; i<8; i++)
                            {
                                currentAPSAddress1.longAddr.v[i] = params.APSME_UPDATE_DEVICE_indication.DeviceAddress.v[i];
                            }
                            currentAPSAddress1.shortAddr.Val = params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.Val;

                            APSSaveAPSAddress(&currentAPSAddress1);

                            // DestinationAddress overlap with Device address, no need to assign
                            #ifdef I_SUPPORT_SECURITY_SPEC
                                if( params.APSME_UPDATE_DEVICE_indication.Status == 0x00 )
                                {
                                    for(i = 0; i < 16; i++)
                                    {
                                        KeyVal.v[i] = 0;
                                    }
                                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = 0x00;
                                }
                                else
                                {
                                    BYTE activeNwkKeyIndex;

                                    GetNwkActiveKeyNumber(&activeNwkKeyIndex);
                                    #ifdef USE_EXTERNAL_NVM
                                        KeyVal = plainSecurityKey[activeNwkKeyIndex-1].NetKey;
                                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = plainSecurityKey[activeNwkKeyIndex-1].SeqNumber.v[0];
                                    #else
                                        GetNwkKeyInfo(&currentNetworkKeyInfo, &networkKeyInfo[activeNwkKeyIndex-1]);
                                        KeyVal = currentNetworkKeyInfo.NetKey;
                                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
                                    #endif
                                    params.APSME_TRANSPORT_KEY_request.ParentAddress = params.APSME_UPDATE_DEVICE_indication.SrcAddress;
                                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = TRUE;
                                }
                            #else
                                params.APSME_TRANSPORT_KEY_request.ParentAddress = params.APSME_UPDATE_DEVICE_indication.SrcAddress;
                                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = TRUE;


                                #ifdef PRECONFIG_NWK_KEY
                                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                                    for(i = 0; i < 16; i++)
                                    {
                                        KeyVal.v[i] = 0x00;
                                    }
                                #else
                                    GetNwkActiveKeyNumber(&i);
                                    #ifdef USE_EXTERNAL_NVM
                                        KeyVal = plainSecurityKey[i-1].NetKey;
                                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = plainSecurityKey[i-1].SeqNumber.v[0];
                                    #else
                                        GetNwkKeyInfo(&currentNetworkKeyInfo, &networkKeyInfo[i-1]);
                                        KeyVal = currentNetworkKeyInfo.NetKey;
                                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
                                    #endif
                                #endif

                            #endif

                            params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                            params.APSME_TRANSPORT_KEY_request.KeyType = 0x01;
                            params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = TRUE;
                            #if I_SUPPORT_LINK_KEY == 1
                                params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = TRUE;
                            #else
                                params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = FALSE;
                            #endif
                            firstKeyHasBeenSent = FALSE;
                            nextPrimitive = APSME_TRANSPORT_KEY_request;

                            return APSME_TRANSPORT_KEY_request;

                        }
                        else
                        {
                            // If the Status is DEVICE_LEFT, then we have to delete the AddressMap entry, if exists.
                            // This is required because, when a new device joins and parent of that device assigns the
                            // same address, then we might end up detecting addressing conflict for this address which
                            // is not being used at all (in other words - we might end up detecting addressing conflict
                            // for the device which has left the network)

                            BYTE i;
                            // A Zigbee 2006 requirement - ZCP Tests requires that the APS table be cleanned up as well
                            for( i = 0; i < apscMaxAddrMapEntries; i++)
                            {
                                #ifdef USE_EXTERNAL_NVM
                                    GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                                #else
                                    GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                                #endif // #ifdef USE_EXTERNAL_NVM
                                if (currentAPSAddress.shortAddr.Val == params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.Val)
                                {
                                    BYTE j;
                                    currentAPSAddress.shortAddr.Val = 0xffff;   /* mark as unused */
                                    for (j=0; j<8; j++)
                                    {
                                        currentAPSAddress.longAddr.v[j] = 0xff;
                                    }
                                    #ifdef USE_EXTERNAL_NVM
                                        PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress );
                                    #else
                                        PutAPSAddress( &apsAddressMap[i], &currentAPSAddress );
                                    #endif // #ifdef USE_EXTERNAL_NVM

                                    break;
                                }
                            }
                        }
                        return NO_PRIMITIVE;
                    }
                //#endif      // if I_AM_TRUST_CENTER

                //#ifndef I_AM_TRUST_CENTER
                    #ifndef I_AM_END_DEVICE
                        case APSME_REMOVE_DEVICE_indication:
                        if(!I_AM_TRUST_CENTER){ //{
                            if (((i = NWKLookupNodeByLongAddr( &(params.APSME_REMOVE_DEVICE_indication.ChildAddress)))
                                != INVALID_NEIGHBOR_KEY) &&
                                (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD))
                            {
                                if ( currentNeighborRecord.bSecured )
                                {
                                    params.NLME_LEAVE_request.Silent        = FALSE;
                                }
                                else
                                {
                                    params.NLME_LEAVE_request.Silent        = TRUE;
                                }
                                params.NLME_LEAVE_request.DeviceAddress     = params.APSME_REMOVE_DEVICE_indication.ChildAddress;
                                params.NLME_LEAVE_request.RemoveChildren    = FALSE; //As per the profile
                                params.NLME_LEAVE_request.ReuseAddress  = TRUE;
                                params.NLME_LEAVE_request.Rejoin        = FALSE;
                                nextPrimitive = NLME_LEAVE_request;
                            }

                            break;
                        }
                    #endif //I_AM_END_DEVICE
                //#endif //I_AM_TRUST_CENTER

                //#ifdef I_AM_TRUST_CENTER
                    #if I_SUPPORT_LINK_KEY == 1
                        case APSME_REQUEST_KEY_indication:
                        if(I_AM_TRUST_CENTER){ //{
                            BYTE ActiveKeyIndex = 0;
                            BYTE KeyType = params.APSME_REQUEST_KEY_indication.KeyType;
                            volatile WORD destAddr;
                            destAddr = params.APSME_REQUEST_KEY_indication.SrcShortAddress.Val;
                            params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val = destAddr;
                            switch( KeyType)
                            {
                                case 0x01:  // network key
                                    GetNwkActiveKeyNumber(&ActiveKeyIndex);
                                    #ifdef USE_EXTERNAL_NVM
                                        KeyVal = plainSecurityKey[ActiveKeyIndex-1].NetKey;
                                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber =
                                            plainSecurityKey[ActiveKeyIndex-1].SeqNumber.v[0];
                                    #else
                                        GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void                                             *)&(networkKeyInfo[ActiveKeyIndex-1]));
                                        KeyVal = currentNetworkKeyInfo.NetKey;
                                        params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber =
                                        currentNetworkKeyInfo.SeqNumber.v[0];
                                    #endif
                                    #if I_SUPPORT_LINK_KEY == 1
                                        params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = TRUE;
                                    #else
                                        params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = FALSE;
                                    #endif
                                    params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = FALSE;
                                    break;
                            #if I_SUPPORT_LINK_KEY == 1
                                case TC_LINK_KEY: // appl link key

                                        #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                                            {
                                                BYTE count;
                                                for( count = 0; count < KEY_LENGTH; count++ )
                                                {
                                                    KeyVal.v[count] = current_SAS.spas.PreconfiguredLinkKey[count];
                                                }
                                            }
                                        #else
                                        {
                                            LONG_ADDR   SrcAddr;
                                            GetMACAddress(&SrcAddr);
                                            GetHashKey
                                            (
                                                &(params.APSME_REQUEST_KEY_indication.SrcAddress),
                                                &SrcAddr, &KeyVal
                                            );
                                        }
                                        #endif
                                        params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = TRUE;
                                        params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = FALSE;

                                     break;
                                case APP_MASTER_KEY:// same for appl link key
                                {
                                    BYTE i;

                                    if( pRequestKeyDetails == NULL )
                                    {
                                        /* allocate memory to back up first request key. WE have to wait for
                                        second request to send the key */
                                        pRequestKeyDetails =
                                            ( REQUEST_KEY_DETAILS *) SRAMalloc( sizeof( REQUEST_KEY_DETAILS ));

                                        if ( pRequestKeyDetails != NULL )
                                        {
                                            pRequestKeyDetails->state = WAIT_FOR_SECOND_REQUEST_KEY_REQUEST;

                                            /* Back up the key request */
                                            for(i=0; i < 8; i++)
                                            {
                                                pRequestKeyDetails->FirstRequestInitiatorAddress.v[i]
                                                =  params.APSME_REQUEST_KEY_indication.SrcAddress.v[i];
                                                pRequestKeyDetails->FirstRequestPartnerAddress.v[i] =
                                                params.APSME_REQUEST_KEY_indication.PartnerAddress.v[i] ;
                                            }
                                            pRequestKeyDetails->FirstRequestInitiatorShortAddress =
                                                params.APSME_TRANSPORT_KEY_request.DstShortAddress;
                                            pRequestKeyDetails->firstRequestTime = TickGet();
                                        }
                                        return NO_PRIMITIVE;
                                    }
                                    else
                                    {
                                        if(pRequestKeyDetails->state == WAIT_FOR_SECOND_REQUEST_KEY_REQUEST )
                                        {
                                             /* we have received second request key request. Compare whether
                                             partner address are same */
                                             for(i=0; i < 8; i++)
                                             {
                                                 if((params.APSME_REQUEST_KEY_indication.PartnerAddress.v[i] !=
                                                 pRequestKeyDetails->FirstRequestInitiatorAddress.v[i])
                                                 ||( params.APSME_REQUEST_KEY_indication.SrcAddress.v[i] !=
                                                 pRequestKeyDetails->FirstRequestPartnerAddress.v[i]))
                                                 {
                                                     return NO_PRIMITIVE;
                                                 }
                                             }
                                             /* send key to second device as we have backed up first one.
                                             First one will be handled in the backgound */
                                             pRequestKeyDetails->state = PENDING_TRANSPORT_KEY_FOR_FIRST_DEVICE;
                                             GetHashKey(&(pRequestKeyDetails->FirstRequestInitiatorAddress),
                                                &(pRequestKeyDetails->FirstRequestPartnerAddress), &KeyVal);

                                             for (i=0; i<16; i++)
                                                {
                                                   pRequestKeyDetails->GeneratedAPPLinkKey.v[i] = KeyVal.v[i];
                                                }

                                             KeyType = APP_LINK_KEY;

                                             params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = FALSE;
                                             //params.APSME_TRANSPORT_KEY_request.DstShortAddress.Val =
                                             //      params.APSME_REQUEST_KEY_indication.SrcShortAddress.Val;
                                                for (i=0; i<8; i++)
                                                {
                                                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.
                                                    APPLICATION_LINKKEY.PartnerAddress.v[i] =                                                       pRequestKeyDetails->FirstRequestInitiatorAddress.v[i];
                                                }
                                            params.APSME_TRANSPORT_KEY_request.TransportKeyData.APPLICATION_LINKKEY.Initiator =                                             0x01; // it will always be true
                                            params.APSME_TRANSPORT_KEY_request._UseAPSSecurity = TRUE;
                                            params.APSME_TRANSPORT_KEY_request._UseNwkSecurity = TRUE;
                                    }
                                    else
                                    {
                                       return NO_PRIMITIVE;
                                    }
                                }
                            }
                        break;
                    #endif
                        default:
                            return NO_PRIMITIVE;
                    }

                    params.APSME_TRANSPORT_KEY_request.KeyType = KeyType;
                    params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                    for (i=0; i<8; i++)
                    {
                        params.APSME_TRANSPORT_KEY_request.DestinationAddress.v[i] =
                        params.APSME_REQUEST_KEY_indication.SrcAddress.v[i];
                    }

                    return APSME_TRANSPORT_KEY_request;

                }

                #endif  // #if I_SUPPORT_LINK_KEY == 1

            //#endif // #ifdef I_AM_TRUST_CENTER

            #ifndef I_AM_TRUST_CENTER
                case APSME_SWITCH_KEY_indication:
                if(!I_AM_TRUST_CENTER){ //{
                    zdoStatus.KeySeq = params.APSME_SWITCH_KEY_indication.KeySeqNumber;
                    zdoStatus.SwitchKeyTick = TickGet();
                    zdoStatus.flags.bits.bSwitchKey = 1;
                    break;
                }
            //#endif // !I_AM_TRUST_CENTER

            #endif //I_SUPPORT_SECURITY
            case NLME_NWK_STATUS_indication:
                {
                    #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                    #ifndef I_AM_END_DEVICE
                    {
                    SHORT_ADDR shortAddress;
                    shortAddress = params.NLME_NWK_STATUS_indication.ShortAddress;

                        if ( params.NLME_NWK_STATUS_indication.nwkStatusCode == NWK_STATUS_ADDRESS_CONFLICT )
                        {
                            switch ( nwkStatus.addressConflictType )
                            {
                                nwkStatus.addressConflictType = NO_CONFLICT;

                                case LOCAL_ADDRESS_CONFLICT:
                                        if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
                                        nextPrimitive = ResolveLocalAddressConflict();
                                        }//#endif
                                    break;

                                case ADDRESS_CONFLICT_ON_A_CHILD_END_DEVICE:
                                     //Send Unsolicited Rejoin Response
                                        nextPrimitive = PrepareUnsolicitedRejoinRsp( shortAddress );
                                    break;

                                case REMOTE_ADDRESS_CONFLICT:
                                    nextPrimitive = PrepareNwkStatusCmdBroadcast( NWK_STATUS_ADDRESS_CONFLICT , shortAddress );
                                    break;

                                default:
                                    break;
                            }
                        }
                    }
                    #endif
                    #endif

                    #if (  I_SUPPORT_PANID_CONFLICT == 0x01  )
                        if ( params.NLME_NWK_STATUS_indication.nwkStatusCode == NWK_STATUS_PAN_ID_UPDATE )
                        {
                            nextPrimitive = NO_PRIMITIVE;
                        }
                    #endif

                    #if I_SUPPORT_CONCENTRATOR == 1
                        if (( params.NLME_NWK_STATUS_indication.nwkStatusCode == NWK_STATUS_SOURCE_ROUTE_FAILURE ) ||
                            ( params.NLME_NWK_STATUS_indication.nwkStatusCode == NWK_STATUS_MANY_TO_ONE_ROUTE_FAILURE ))
                        {

                            DeleteRouteRecord(params.NLME_NWK_STATUS_indication.ShortAddress);
                            nextPrimitive = BroadcastManyToOneRequest();
                        }
                    #endif
                    ZDODiscardRx();
                    break;
                }
            default:
                break;
        } // Input Primitive
    }
    return nextPrimitive;
}
#if I_SUPPORT_CONCENTRATOR == 1
    void DeleteRouteRecord(SHORT_ADDR shortAddress)
    {
        BYTE index;
        BYTE prevIndex;
        BYTE nextIndex;
        /* delete the entry from route record table. */
        index = GetIndexInRouteRecord(shortAddress );
        while( index != NOT_VALID )
        {
            nextIndex =  RouteRecordTable[index].nextHopIndex;
            if( nextIndex  != NOT_VALID )
            {
                RouteRecordTable[nextIndex].prevHopIndex = NOT_VALID;
            }
            RouteRecordTable[index].destinationAddress.Val = INVALID_SHORT_ADDRESS;
            prevIndex = RouteRecordTable[index].prevHopIndex;
            RouteRecordTable[index].prevHopIndex = NOT_VALID;
            RouteRecordTable[index].nextHopIndex = NOT_VALID;
            index = prevIndex;
        }
    }
#endif  /* I_SUPPORT_CONCENTRATOR == 1 */
/*********************************************************************
 * Function:       static void BKDRHash(BYTE input, KEY_VAL *hash, BYTE index)
 *
 * PreCondition:    None
 *
 * Input:           input - The input byte on which hash value generated
 *                  index - The index at which the output should be copied
 *
 * Output:          hash - The generated 2 byte hash value
 *
 * Side Effects:    None
 *
 * Overview:        This function generates a hash value bsed
 *                  on the input.
 ********************************************************************/
#if I_SUPPORT_LINK_KEY == 1
//#ifdef I_AM_TRUST_CENTER
static void BKDRHash(BYTE input, KEY_VAL *hash, BYTE index)
{

    WORD i    = 0;
    WORD seed ;
    WORD_VAL    temp;

    temp.Val = 0x0000;

    //*hash = 0;
    seed = 131; /* 31 131 1313 13131 131313 etc.. */

   for(i = 0; i < 4; input++, i++)
   {
      temp.Val = (temp.Val * seed) + (input);
   }
   hash->v[index] = temp.v[0];
   hash->v[index + 1] = temp.v[1];

}
/*********************************************************************
 * Function:        void GetHashKey(LONG_ADDR *longAddress1, LONG_ADDR *longAddress2, KEY_VAL *key)
 *
 * PreCondition:    None
 *
 * Input:           longAddress1 & longAddress2 - The long address of devices
 *                  which shares a link key.The link key will be generated by
 *                  a hash function which takes a combination of bytes of booth
 *                  address as input.
 *
 * Output:          key - The hashed key generated.
 *
 * Side Effects:    None
 *
 * Overview:        This function makes a combination of address passed
 *                  and pass it to hash function as input to get a hash value
 *
 ********************************************************************/

void GetHashKey(LONG_ADDR *longAddress1, LONG_ADDR *longAddress2, KEY_VAL *key)
{
    BYTE i;
    BYTE j;
    BYTE input;


    for(i=0,j=0; i < 8; i++, j = j + 2)
    {

            input = longAddress1->v[i] ^ longAddress2->v[i];


        //BKDRHash(input, (WORD *)&(key->v[j]) );
        BKDRHash(input, key, j);
    }

}
#endif /* I_SUPPORT_LINK_KEY */
//#endif /*I_AM_TRUST_CENTER */
/*********************************************************************
 * Function:        void FinishAddressResponses( WORD clusterID, LONG_ADDR IEEEAddr, SHORT_ADDR nwkAddr, BYTE requestType, BYTE startIndex )
 *
 * PreCondition:    None
 *
 * Input:           clusterID - output cluster ID
 *
 * Output:          None
 *
 * Side Effects:    Message sent
 *
 * Overview:        This function finishes the address responses for the
 *                  NWK_ADDR_req and IEEE_ADDR_req clusters.
 *
 * Note:
 *
 * NOTE: The spec is confusing on the IEEE_ADDR_rsp.  They mention in one place that we're
 * supposed to respond with the "16-bit IEEE addresses", but in the table for the
 * response it says 16-bit short address, which make the response the same
 * as for NWK_addr_rps.  And the Framework spec alludes to being able to request
 * both short and long addresses.  We'll send the short addresses, with code for
 * the long addresses left in the comments in case we need it later.
 ********************************************************************/
void FinishAddressResponses( WORD clusterID, LONG_ADDR IEEEAddr, SHORT_ADDR nwkAddr, BYTE requestType, BYTE startIndex  )
{
#ifndef I_AM_END_DEVICE
    BYTE        count;
#endif
    BYTE        i;

    // Now get the rests of the paramters.

    if (requestType == SINGLE_DEVICE_RESPONSE)
    {
        // Skip over the header info

        TxData = TX_DATA_START + MSG_HEADER_SIZE;

        // Load status byte
        TxBuffer[TxData++] = SUCCESS;

        // Load our long address.
        for (i=0; i<8; i++)
        {
            TxBuffer[TxData++] = IEEEAddr.v[i];
        }

        // Load our short address.
        TxBuffer[TxData++] = nwkAddr.byte.LSB;
        TxBuffer[TxData++] = nwkAddr.byte.MSB;

        PrepareMessageResponse( clusterID );
    }
    else if (requestType == EXTENDED_RESPONSE)
    {
        #if defined(I_AM_END_DEVICE)

            // Skip over the header info
            TxData = TX_DATA_START + MSG_HEADER_SIZE;

            // Load status byte
            TxBuffer[TxData++] = SUCCESS;

            // Load our long address.

            for (i=0; i<8; i++)
            {
                TxBuffer[TxData++] = IEEEAddr.v[i];
            }

            // Load our short address.
            TxBuffer[TxData++] = nwkAddr.byte.LSB;
            TxBuffer[TxData++] = nwkAddr.byte.MSB;

            // Load NumAssocDev value
            // For end device, there are no associated device.  So we send 0 for the
            // number of associated devices and don't send StartIndex and the list
            // of associated devices.
            TxBuffer[TxData++] = 0;

            PrepareMessageResponse( clusterID );
        #else

            // Skip over the header info
            TxData = TX_DATA_START + MSG_HEADER_SIZE;

            // Load status byte
            TxBuffer[TxData++] = SUCCESS;

            // Load our long address.
            for (i=0; i<8; i++)
            {
                TxBuffer[TxData++] = IEEEAddr.v[i];
            }

            // Load our short address.
            TxBuffer[TxData++] = nwkAddr.byte.LSB;
            TxBuffer[TxData++] = nwkAddr.byte.MSB;

            // Load NumAssocDev value
            TxBuffer[TxData++] = currentNeighborTableInfo.numChildren;

            // Send the start index back
            TxBuffer[TxData++] = startIndex;

            if (startIndex < currentNeighborTableInfo.numChildren) // if 1-based, <=
            {
                // Send the associated devices list.
                i = 0;
                count = 0;
                pCurrentNeighborRecord = neighborTable;
                while ((count < MAX_SHORTADDS_TO_SEND) && (i < currentNeighborTableInfo.numChildren))
                {
                    do
                    {
                        GetNeighborRecord(&currentNeighborRecord, pCurrentNeighborRecord );
                        #ifdef USE_EXTERNAL_NVM
                            pCurrentNeighborRecord += sizeof(NEIGHBOR_RECORD);
                        #else
                            pCurrentNeighborRecord++;
                        #endif
                    }
                    while ( (!currentNeighborRecord.deviceInfo.bits.bInUse) ||
                            (currentNeighborRecord.deviceInfo.bits.Relationship != NEIGHBOR_IS_CHILD));

                    if (i >= startIndex)
                    {
                        TxBuffer[TxData++] = currentNeighborRecord.shortAddr.byte.LSB;
                        TxBuffer[TxData++] = currentNeighborRecord.shortAddr.byte.MSB;

                        count++;
                    }

                    i ++;   // if 1-based, move to before if statement
                }

            }
            PrepareMessageResponse( clusterID );
        #endif
    }
    else
    {
        // Load status byte
        TxBuffer[TxData++] = ZDO_INV_REQUESTTYPE;

        PrepareMessageResponse( clusterID );
    }
}

/*********************************************************************
 * Function:        BOOL IsThisMyShortAddr(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - The next two bytes at adsu match my short address
 *                  FALSE - The next two bytes at adsu do not match
 *
 * Side Effects:    None
 *
 * Overview:        This function determines if the next two bytes at
 *                  adsu match my short address or are the broadcast
 *                  address.
 *
 * Note:            The broadcast address is allowed as a match for
 *                  the MATCH_DESC_req.  That is the only one that is
 *                  allowed to be broadcast - all the others must be
 *                  unicast.
 ********************************************************************/

BOOL IsThisMyShortAddr(void)
{
    SHORT_ADDR  address;

    address.byte.LSB = ZDOGet();
    address.byte.MSB = ZDOGet();

    if ((address.Val == macPIB.macShortAddress.Val) ||
        (address.Val == 0xFFFF)
        #if !defined(I_AM_END_DEVICE)
            || (address.Val == 0xFFFC)
        #endif
        //#if !defined(I_AM_RFD)
            || (NOW_I_AM_NOT_A_RFD() && (address.Val == 0xFFFD)) //TODO revisit this condition.
        //#endif
        )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*********************************************************************
 * Function:        END_DEVICE_BIND_request * GetOneEndDeviceBindRequest(
                            void )
 *
 * PreCondition:    None
 *
 * Input:           pRequestInfo - pointer to the end device bind request
 *                  information.  If this is null, the request is being
 *                  received from another device, and the information is
 *                  in the received message.  Otherwise, the information
 *                  must be extracted from the incoming message.
 *
 * Output:          Next ZigBee primitive
 *
 * Side Effects:    Numerous
 *
 * Overview:        This function retrieves end device binding information
 *                  from either the upper layers or an incoming message.
 *
 * Note:            None
 ********************************************************************/
#if defined SUPPORT_END_DEVICE_BINDING && (MAX_BINDINGS > 0)

END_DEVICE_BIND_request * GetOneEndDeviceBindRequest( void )
{
    BYTE                    *firstClusterPtr = NULL;
    BYTE                    i;
    END_DEVICE_BIND_request    *pEndDeviceBindRequest;

    pEndDeviceBindRequest = NULL;

   // if (pRequestInfo == NULL)

        // We are getting this request from another device
        if ((pEndDeviceBindRequest = (END_DEVICE_BIND_request *)SRAMalloc( sizeof(END_DEVICE_BIND_request) )) != NULL)
        {

             /* ignore binding target */
            i = ZDOGet();
            i = ZDOGet();
            for(i = 0; i < 8; i++)
            {
                pEndDeviceBindRequest->longAddr.v[i]    = ZDOGet();
            }
           pEndDeviceBindRequest->sequenceNumber = sequenceNumber;
           pEndDeviceBindRequest->deviceShortAddress
           = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
           pEndDeviceBindRequest->endpoint              = ZDOGet();
           pEndDeviceBindRequest->profileID.byte.LB    = ZDOGet();
           pEndDeviceBindRequest->profileID.byte.HB    = ZDOGet();
           pEndDeviceBindRequest->numInClusters         = ZDOGet();
           pEndDeviceBindRequest->inClusterList = NULL;
           pEndDeviceBindRequest->outClusterList = NULL;
            if (pEndDeviceBindRequest->numInClusters != 0)
            {
                if ((firstClusterPtr = (BYTE *)SRAMalloc( pEndDeviceBindRequest->numInClusters * 2 )) == NULL)
                {
                    nfree( pEndDeviceBindRequest );
                    goto BailFromGatheringInfo;
                }
                else
                    pEndDeviceBindRequest->inClusterList = firstClusterPtr;
            }

            for (i=0; i<pEndDeviceBindRequest->numInClusters; i++)
            {
                *firstClusterPtr++ = (WORD)ZDOGet();
                *firstClusterPtr++ = (WORD)ZDOGet(); //((WORD)ZDOGet())<<8;
            }

            pEndDeviceBindRequest->numOutClusters        = ZDOGet();
            if (pEndDeviceBindRequest->numOutClusters != 0)
            {
                if ((firstClusterPtr = (BYTE *)SRAMalloc( pEndDeviceBindRequest->numOutClusters * sizeof(WORD) )) == NULL)
                {

                   nfree( pEndDeviceBindRequest->inClusterList);
                    nfree( pEndDeviceBindRequest );
                    goto BailFromGatheringInfo;
                }
                else
                    pEndDeviceBindRequest->outClusterList = firstClusterPtr;
            }
            for (i=0; i<pEndDeviceBindRequest->numOutClusters; i++)
            {
                *firstClusterPtr++ = (WORD)ZDOGet();
                *firstClusterPtr++=  (WORD)ZDOGet();
            }

           // pEndDeviceBindRequest->flags.bits.bFromSelf = 0;
        }

BailFromGatheringInfo:
    return pEndDeviceBindRequest;
}
#endif

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE ProcessEndDeviceBind(
                        END_DEVICE_BIND_INFO *pRequestInfo )
 *
 * PreCondition:    sequenceNumber and sourceEP must be set
 *
 * Input:           pRequestInfo - pointer to the end device bind request
 *                  information.  If this is null, the request is being
 *                  received from another device, and the information is
 *                  in the received message.
 *
 * Output:          Next ZigBee primitive
 *
 * Side Effects:    Numerous
 *
 * Overview:        This function performs either the first half or the
 *                  second half of end device binding.  It can be invoked
 *                  either by receiving an END_DEVICE_BIND_req message
 *                  or by the upper layers of the application.  The first
 *                  time the routine is called, the bind information is
 *                  simply stored.  The second time it is called, the two
 *                  lists are checked for a match, and bindings are
 *                  created if possible.  The background processing will
 *                  check for timeout of the first message.
 *
 * Note:            None
 ********************************************************************/
#ifdef SUPPORT_END_DEVICE_BINDING

ZIGBEE_PRIMITIVE ProcessEndDeviceBind( END_DEVICE_BIND_INFO *pRequestInfo )
{

#if defined SUPPORT_END_DEVICE_BINDING && (MAX_BINDINGS > 0)


    BYTE                    firstMatch = FALSE;
    ZIGBEE_PRIMITIVE        nextPrimitive ;
    BYTE                    secondMatch = FALSE;
    BYTE                    status;
    END_DEVICE_BIND_request *EBrequest;

    status = ZDO_NO_MATCH;
    nextPrimitive = NO_PRIMITIVE;


    if (pEndBindRequest == NULL)
    {
        // We are receiving the first request.  Gather all of the information from
        // this request and store it for later.
        pEndBindRequest = (END_DEVICE_BIND_INFO*)SRAMalloc( sizeof( END_DEVICE_BIND_INFO) );
        if(pEndBindRequest == NULL)
        {
            return NO_PRIMITIVE;
        }
        pEndBindRequest->pFirstEndDeviceBindReq = NULL;
        pEndBindRequest->pSecondEndDeviceBindReq = NULL;
        if (( EBrequest = GetOneEndDeviceBindRequest( )) == NULL)
        {
            goto BailFromEndDeviceRequest;
        }

        EBrequest->MatchList = NULL;
        pEndBindRequest->pFirstEndDeviceBindReq = EBrequest;
        // Set the timer and wait for the second request.
        pEndBindRequest->lastTick = TickGet();
        //pFirstEndDeviceBindReq->flags.bits.bSendResponse = 0;
        pEndBindRequest->EBstate = WAIT_FOR_SECOND_BIND;
        // Set the background flag for processing end device binding.
        zdoStatus.flags.bits.bEndDeviceBinding = 1;

        //pEndBindRequest->pSecondEndDeviceBindReq->status = END_DEVICE_BIND_TIMEOUT;
        return NO_PRIMITIVE;
    }
    else
    {
        // We can only process one request at a time!  So make sure we aren't really receiving
        // a new request.  We can tell by checking the bSendResponse flag for the first request.
        // If we are still trying to send a response, then we are not ready for a new request,
        // and we'll have to discard it.
        if ( pEndBindRequest->EBstate != WAIT_FOR_SECOND_BIND)
        {
            return NO_PRIMITIVE;
        }

        //pEndBindRequest->pSecondEndDeviceBindReq->inClusterList = NULL;
        //pEndBindRequest->pSecondEndDeviceBindReq->outClusterList = NULL;
        // We are receiving the second request.  Gather all of the information from
        // this request so we can check for matches.
        if ((EBrequest = GetOneEndDeviceBindRequest( )) == NULL)
        {
            goto BailFromEndDeviceRequest;
        }
        pEndBindRequest->pSecondEndDeviceBindReq = EBrequest;
        EBrequest->MatchList = NULL;
        // See if there are any matches between the first and the second requests.
        if (pEndBindRequest->pFirstEndDeviceBindReq->profileID.Val == EBrequest->profileID.Val)
        {
            // Check first request's input clusters against second request's output clusters
            EBrequest->MatchList =
            FindMatch( EBrequest->numOutClusters,
            pEndBindRequest->pFirstEndDeviceBindReq->numInClusters,
            ( WORD_VAL *)EBrequest->outClusterList,
            ( WORD_VAL *)pEndBindRequest->pFirstEndDeviceBindReq->inClusterList,
            &EBrequest->MatchCount);
            if( EBrequest->MatchList )
            {
               secondMatch = TRUE;
               pEndBindRequest->EBstate = SEND_TEST_UNBIND_TO_SECOND_DEVICE;

            }

            // Check second request's input clusters against first request's output clusters
            pEndBindRequest->pFirstEndDeviceBindReq->MatchList = FindMatch
            (pEndBindRequest->pFirstEndDeviceBindReq->numOutClusters,
            EBrequest->numInClusters,
            ( WORD_VAL *)pEndBindRequest->pFirstEndDeviceBindReq->outClusterList,
            ( WORD_VAL *)EBrequest->inClusterList,
            &pEndBindRequest->pFirstEndDeviceBindReq->MatchCount);
            if(pEndBindRequest->pFirstEndDeviceBindReq->MatchList)
            {
                firstMatch = TRUE;
                pEndBindRequest->EBstate = SEND_TEST_UNBIND_TO_FIRST_DEVICE;
            }

            if (firstMatch ||  secondMatch )
            {
              status = SUCCESS;
            }
            else
            {
                pEndBindRequest->EBstate = SEND_EB_RESPONSE_TO_FIRST_DEVICE;
            }


        }
    }


BailFromEndDeviceRequest:
    // Send the responses to the two requestors.  Since each requires its own message, we must send one from
    // the background.  We'll end the second request here, and the first from the background, since pSecondEndDeviceBindReq
    // is a local variable.  We'll clear zdoStatus.flags.bits.bEndDeviceBinding from the background after all of the
    // responses are sent.
    if (pEndBindRequest->pSecondEndDeviceBindReq != NULL)
    {
        /* Don't send response immediately, wait until all the bind rsp  are in (testcase) */
        pEndBindRequest->pSecondEndDeviceBindReq->status                   = status;
    }
    if (pEndBindRequest->pFirstEndDeviceBindReq != NULL)
    {
        pEndBindRequest->pFirstEndDeviceBindReq->status               = status;
    }
    else
    {
        if(pEndBindRequest != NULL )
        {
            nfree( pEndBindRequest);
        }
        zdoStatus.flags.bits.bEndDeviceBinding = 0;
    }
    /* Don't send response immediately, wait until all the bind rsp  are in (testcase) */
    nextPrimitive = NO_PRIMITIVE;
    return nextPrimitive;

#else

    // Skip over the header info
    TxData = TX_DATA_START + MSG_HEADER_SIZE;

    TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
    PrepareMessageResponse( END_DEVICE_BIND_rsp );
    return APSDE_DATA_request;

#endif

}
/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE Send_END_DEVICE_BIND_rsp(
                        END_DEVICE_BIND_INFO *pBindRequest, BYTE status )
 *
 * PreCondition:    End Device Binding should be in progress.
 *
 * Input:           *pBindRequest - pointer to the end device bind info
 *                  status - status of the end device bind
 *
 * Output:          Next primitive
 *
 * Side Effects:    None
 *
 * Overview:        This function either notifies the application of the
 *                  result of the end device bind, or sends a message to the
 *                  end device with the result.  It then deallocates the
 *                  memory that was allocated for the end device bind.
 *
 * Note:            Do to compiler limitations, we cannot set the
 *                  xxxBindRequest pointer passed in to 0.  Therefore,
 *                  the application must do the final free of the
 *                  xxxBindRequest memory.
 *
 ********************************************************************/

WORD_VAL *FindMatch( BYTE firstClusterCount, BYTE secondClusterCount,
WORD_VAL * FirstClusterList,WORD_VAL *SecondClusterList, BYTE *matchCount )
{
     BYTE firstIndex;
     BYTE secondIndex;
     BYTE matchFound = FALSE;
     *matchCount = 0;
     WORD_VAL *matchList = NULL;
     WORD_VAL *Incluster;
     WORD_VAL *OutCluster;
     Incluster = FirstClusterList;
     OutCluster = SecondClusterList;
     // Check first request's output clusters against second request's input clusters
    for (firstIndex=0; firstIndex < firstClusterCount; firstIndex++ )
    {
        for (secondIndex=0; secondIndex < secondClusterCount; secondIndex++)
        {
            if (Incluster[firstIndex].Val == OutCluster[secondIndex].Val)
            {
                /* finish the entire loop so that a count of matched Cluster IDs can be obtained */
                    /* just keep the results of the 1st match found */

                   matchFound = TRUE;
                  *matchCount = *matchCount + 1;
            }
        }

    }
    if( matchFound == TRUE )
    {
        matchList = ( WORD_VAL *)SRAMalloc( *matchCount * 2 );
        if( matchList != NULL )
        {

            *matchCount = 0x00;
            for (firstIndex=0; firstIndex < firstClusterCount; firstIndex++)
            {
                for (secondIndex=0; secondIndex < secondClusterCount; secondIndex++)
                {
                   if (Incluster[firstIndex].Val == OutCluster[secondIndex].Val)
                    {
                        /* finish the entire loop so that a count of matched Cluster IDs can be obtained */
                            /* just keep the results of the 1st match found */
                        matchList[*matchCount].Val = Incluster[firstIndex].Val;
                        *matchCount = *matchCount + 1;


                    }
                }

            }
        }
    }
    return matchList;
}

 ZIGBEE_PRIMITIVE SendBindOrUnbind( ZDO_CLUSTER cluster, END_DEVICE_BIND_request *bindSource,
 END_DEVICE_BIND_request *bindTarget, BYTE clusterUpdate )
 {
    BYTE i;
     if( !bindSource->MatchCount )
    {
        if( ( pEndBindRequest->EBstate == SEND_UNBIND_TO_FIRST_DEVICE )||
        (  pEndBindRequest->EBstate == SEND_BIND_TO_FIRST_DEVICE ))
        {
            pEndBindRequest->EBstate = pEndBindRequest->EBstate + 2;
        }
        else
        {
            pEndBindRequest->EBstate = SEND_EB_RESPONSE_TO_FIRST_DEVICE;
        }
         return NO_PRIMITIVE;
    }
    pEndBindRequest->EBstate = pEndBindRequest->EBstate + 1;
    ZigBeeBlockTx();
    TxData = TX_DATA_START + MSG_HEADER_SIZE;
    for(i = 0; i < 8; i++)
    {
        TxBuffer[TxData++] = bindSource->longAddr.v[i];
    }
    TxBuffer[TxData++] = bindSource->endpoint;

    /* reuse i to store matchcount */
    i = bindSource->MatchCount - 1;
    TxBuffer[TxData++]  =  bindSource->MatchList[i].byte.LB;
    TxBuffer[TxData++]  =  bindSource->MatchList[i].byte.HB;

    TxBuffer[TxData++] = 0x03; // dstAddrMode: 64bit
    for(i = 0; i < 8; i++)
    {
        TxBuffer[TxData++] = bindTarget->longAddr.v[i];
    }
    TxBuffer[TxData++] = bindTarget->endpoint;

    params.ZDO_DATA_indication.SrcAddress.ShortAddr = bindSource->deviceShortAddress;
    if( clusterUpdate )
    {
       bindSource->MatchCount--;
    }


    /*  UNbind_req goes before bind_req according to specs/testcases */
    PrepareMessageResponse( cluster );
     pEndBindRequest->lastTick = TickGet();
    return APSDE_DATA_request;
 }

 void ClearMemoryAllocatedForEB( void )
 {
     if( pEndBindRequest->pFirstEndDeviceBindReq != NULL )
     {
        if( pEndBindRequest->pFirstEndDeviceBindReq->MatchList != NULL )
        {
               nfree(pEndBindRequest->pFirstEndDeviceBindReq->MatchList);
        }
        if( pEndBindRequest->pFirstEndDeviceBindReq->inClusterList != NULL )
        {
            nfree(pEndBindRequest->pFirstEndDeviceBindReq->inClusterList);
        }
        if( pEndBindRequest->pFirstEndDeviceBindReq->outClusterList != NULL )
        {
            nfree(pEndBindRequest->pFirstEndDeviceBindReq->outClusterList);
        }

        nfree(pEndBindRequest->pFirstEndDeviceBindReq );
     }
     if( pEndBindRequest->pSecondEndDeviceBindReq != NULL )
     {

         if( pEndBindRequest->pSecondEndDeviceBindReq->MatchList != NULL )
         {
            nfree(pEndBindRequest->pSecondEndDeviceBindReq->MatchList);
         }
         if( pEndBindRequest->pSecondEndDeviceBindReq->inClusterList != NULL )
         {
            nfree(pEndBindRequest->pSecondEndDeviceBindReq->inClusterList);
         }
         if( pEndBindRequest->pSecondEndDeviceBindReq->outClusterList != NULL )
         {
            nfree(pEndBindRequest->pSecondEndDeviceBindReq->outClusterList);
         }
         nfree(pEndBindRequest->pSecondEndDeviceBindReq);
     }
     nfree( pEndBindRequest);
     pEndBindRequest = NULL;
 }
#endif
/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE Send_END_DEVICE_BIND_rsp(
                        END_DEVICE_BIND_INFO *pBindRequest, BYTE status )
 *
 * PreCondition:    End Device Binding should be in progress.
 *
 * Input:           *pBindRequest - pointer to the end device bind info
 *
 * Output:          Next primitive
 *
 * Side Effects:    None
 *
 * Overview:        This function either notifies the application of the
 *                  result of the end device bind, or sends a message to the
 *                  end device with the result.  It then deallocates the
 *                  memory that was allocated for the end device bind.
 *
 * Note:            Do to compiler limitations, we cannot set the
 *                  xxxBindRequest pointer passed in to 0.  Therefore,
 *                  the application must do the final free of the
 *                  xxxBindRequest memory.
 *
 ********************************************************************/
#ifdef SUPPORT_END_DEVICE_BINDING
ZIGBEE_PRIMITIVE Send_END_DEVICE_BIND_rsp( END_DEVICE_BIND_request *pBindRequest)
{
    ZIGBEE_PRIMITIVE    nextPrimitive;

    nextPrimitive = NO_PRIMITIVE;

    // Skip over the header info
    ZigBeeBlockTx();
    TxData = TX_DATA_START + MSG_HEADER_SIZE;

    TxBuffer[TxData++] = pBindRequest->status;

    // Load the message information.  We have to patch the destination address.
    sequenceNumber = pBindRequest->sequenceNumber;
    // should always be EP0 sourceEP = pBindRequest->sourceEP;
    PrepareMessageResponse( END_DEVICE_BIND_rsp );
    params.APSDE_DATA_request.DstAddress.ShortAddr  = pBindRequest->deviceShortAddress;

    #ifdef ENABLE_DEBUG
    {
        BYTE    buffer[60];

        sprintf( (char *)buffer, (ROM char *) "Sending end device bind response %d to %04x.\r\n\0",
            status, pBindRequest->shortAddr.Val );
        ConsolePutString( buffer );
    }
    #endif
    nextPrimitive = APSDE_DATA_request;


    return nextPrimitive;
}
#endif

/*********************************************************************
 * Function:        void PrepareMessageResponse( WORD clusterID )
 *
 * PreCondition:    Tx is not blocked, all data is loaded and TxData
 *                  is accurate.
 *
 * Input:           clusterID - cluster ID of the ZDO packet
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function prepares the APSDE_DATA_request
 *                  parameters as needed by the ZDO responses.
 *
 * Note:            None
 *
 ********************************************************************/
void PrepareMessageResponse( WORD clusterID )
{
    ZigBeeBlockTx();

    // Load the header information
    TxBuffer[TX_DATA_START]   = sequenceNumber;                      // Transaction Sequence Number of the request

    // We only get short addresses from the NWK layer.
    /* For ZigBee 2006 :  We need to verify address mode used by the
     * sender and respond accordingly, because if long address was used previous
     * version would fail here
     * Note:  The NLDE_indication changed this to a long address, so we must be
     * consistent check for the address mode here again.
    */
    params.APSDE_DATA_request.DstAddrMode   = APS_ADDRESS_16_BIT;

    if( params.ZDO_DATA_indication.SrcAddrMode == APS_ADDRESS_64_BIT )
    {
        if( APSFromLongToShort(&params.ZDO_DATA_indication.SrcAddress.LongAddr) )
        {
            params.APSDE_DATA_request.DstAddress.ShortAddr = currentAPSAddress.shortAddr;
        }
    }
    else
    {
        params.APSDE_DATA_request.DstAddress.ShortAddr.Val = \
            params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val;
    }

    // params.APSDE_DATA_request.asduLength; in place with TxData
    params.APSDE_DATA_request.ProfileId.Val = ZDP_PROFILE_ID;
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;

    params.APSDE_DATA_request.TxOptions.Val = DEFAULT_TX_OPTIONS;

    if ((clusterID == SYSTEM_SERVER_DISCOVERY_rsp)
    #if I_SUPPORT_FREQUENCY_AGILITY == 1
     || (clusterID == MGMT_NWK_UPDATE_notify)
    #endif
    )
    {
        params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
    }
    params.APSDE_DATA_request.SrcEndpoint   = EP_ZDO;   // should always be EP0 sourceEP;
    params.APSDE_DATA_request.DstEndpoint   = EP_ZDO;
    params.APSDE_DATA_request.ClusterId.Val     = clusterID;
}


void handleActiveEPReq(void)
{
    NODE_SIMPLE_DESCRIPTOR  simpleDescriptor;
    BYTE                    wasBroadcast;
    BYTE i;

    // Save the broadcast indication for later.
    wasBroadcast = params.ZDO_DATA_indication.WasBroadcast;

    // Skip over the header info
    TxData = TX_DATA_START + MSG_HEADER_SIZE;

    // Load status
    TxBuffer[TxData++] = SUCCESS;

    // Load our short address.
    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

    // Load the endpoint count
    /* For ZigBee 2006: The number of endpoints should exclude EP0 */
    TxBuffer[TxData++] = NUM_USER_ENDPOINTS;

    for (i=1; i<NUM_DEFINED_ENDPOINTS; i++)  /* For ZigBee 2006 - Do not include EP0 */
    {
        ProfileGetSimpleDesc( &simpleDescriptor, i );
        TxBuffer[TxData++] = simpleDescriptor.Endpoint;
    }

    PrepareMessageResponse( ACTIVE_EP_rsp );

    // This request may have been broadcast. If so, we must request an APS ACK on the response.
    if (wasBroadcast)
    {
        params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
    }
}


void handleLQIReq(void)
{
    BYTE i;
    BYTE    startIndex;
    BYTE    listLocation            = 0;
    BYTE    neighborTableListCount  = 0;
    //BOOL    oneValidEntry = FALSE;
    BYTE    count;
    
    
    BYTE    validEntries    = 0;
    NEIGHBOR_RECORD localNeighborTable[MAX_NEIGHBORS];
    
    /*Get the starting index of interest */ 
    startIndex  = ZDOGet();
    
    /* Skip over the header in Transmit Buffer */
    TxData = TX_DATA_START + MSG_HEADER_SIZE;
    
    /* get the current size of Neighbor Table and error check against startIndex */
    GetNeighborTableInfo();
    
    /* ensure that stack can send at least one record */
    if(startIndex >= currentNeighborTableInfo.neighborTableSize)  
    {
        // Load status
        TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
        PrepareMessageResponse( MGMT_LQI_rsp );
        return;   
        
    }

    validEntries = 0;
    #ifdef USE_EXTERNAL_NVM
        for (i=0, pCurrentNeighborRecord = neighborTable;  i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD) )
    #else
        for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif // #ifdef USE_EXTERNAL_NVM
    {
        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
        
        if ( currentNeighborRecord.deviceInfo.bits.bInUse)
        {
            localNeighborTable[validEntries] = currentNeighborRecord;
            validEntries++;
        }
    }
    
    
    /* the indexing starts at zero for this primitive */
    if( (validEntries == 0) || (validEntries <= startIndex) )
    {
        // Load status
        TxBuffer[TxData++] = ZDO_NOT_SUPPORTED; 
        PrepareMessageResponse( MGMT_LQI_rsp );
        return;   
    } 
      
    /* at least one valid entry found beyond the startIndex so prepare packet */      
    TxBuffer[TxData++]  = SUCCESS;
    TxBuffer[TxData++]  = validEntries;  //currentNeighborTableInfo.neighborTableSize;
    TxBuffer[TxData++]  = startIndex;
    listLocation        = TxData;

    TxBuffer[TxData++]  = validEntries;     /* this will be finalized later */
    
    /* Now load up the NBT entries  */
    for( count = startIndex; count < startIndex+2; count++)
    {   
        neighborTableListCount++;
             
        /* load the parameters */
        memcpy((BYTE *)&TxBuffer[TxData],(void*)&localNeighborTable[count].ExtendedPANID, 8);
        TxData += 8;
        memcpy((BYTE *)&TxBuffer[TxData],(void*)&localNeighborTable[count].longAddr, 8);
        TxData += 8;
        memcpy((BYTE *)&TxBuffer[TxData],(void*)&localNeighborTable[count].shortAddr, 2);
        TxData += 2;
        
        
        TxBuffer[TxData] = 0x00;  /*reserved msb */
        TxBuffer[TxData++] = ( ((localNeighborTable[count].deviceInfo.bits.Relationship & 3) << 4)  |
                               ((localNeighborTable[count].deviceInfo.bits.RxOnWhenIdle & 1) << 2)  |
                               ((localNeighborTable[count].deviceInfo.bits.deviceType   & 3) << 0) 
                             );
                          
        TxBuffer[TxData]   = 0x00;  /* reserved msb */
        TxBuffer[TxData++] = (localNeighborTable[count].deviceInfo.bits.PermitJoining & 1); 
        
        TxBuffer[TxData++] = localNeighborTable[count].deviceInfo.bits.Depth;
        
        TxBuffer[TxData++] = localNeighborTable[count].deviceInfo.bits.LQI;
        
        if( (validEntries - count) == 1)
            break;
        
    }
    
    /* update the TxBuffer with total entries that are included */
    TxBuffer[listLocation] = neighborTableListCount;
    PrepareMessageResponse(MGMT_LQI_rsp);
    
    return;
    
 
}
#if defined (__PIC24FJ256GB110__) || defined (__PIC24FJ256GB106__) || defined(__PIC32MX__)

//#if defined(I_AM_ROUTER) || defined(I_AM_COORDINATOR)
void handleRTGReq(void)
{
    BYTE    i;
    BYTE    startIndex;
    BYTE    listLocation;
    
    BYTE count;
    BYTE validEntries = 0;
    
    ROUTING_ENTRY localRoutingTable[ROUTING_TABLE_SIZE];
    
    /*Get the starting index of interest */ 
    startIndex  = ZDOGet();
    /* Skip over the header in Transmit Buffer */
    TxData = TX_DATA_START + MSG_HEADER_SIZE;

    /* error checking */
    if(startIndex > ROUTING_TABLE_SIZE)  
    {
        // Load status
        TxBuffer[TxData++] = ZDO_NOT_SUPPORTED; 
        PrepareMessageResponse( MGMT_RTG_rsp );
        return;   
    }
    
    /* search routing table for valid entries - exclude the many-to-ones broadcast requests */
    #ifdef USE_EXTERNAL_NVM
    for (i=0, pCurrentRoutingEntry = routingTable; i<ROUTING_TABLE_SIZE; i++, pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY))
    #else
    for (i=0, pCurrentRoutingEntry = routingTable; i<ROUTING_TABLE_SIZE; i++, pCurrentRoutingEntry++)
    #endif
    {
        GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
        
        /* exclude the broadcast addresses */
        if ( !( (currentRoutingEntry.destAddress.Val    == INVALID_SHORT_ADDRESS)  ||
             (currentRoutingEntry.destAddress.Val       == BC_TO_ALL_ROUTERS_AND_COORDINATORS) ||
             (currentRoutingEntry.destAddress.Val       == BC_TO_RX_ON_WHEN_IDLE_DEVICES))
           )
        {
            localRoutingTable[validEntries].destAddress.Val = currentRoutingEntry.destAddress.Val;
            localRoutingTable[validEntries].nextHop.Val     = currentRoutingEntry.nextHop.Val;
            localRoutingTable[validEntries].rtflags         =  currentRoutingEntry.rtflags;
            validEntries++;
        }
    }
    
    /* the indexing starts at zero for this primitive */
    if( (validEntries == 0) || (validEntries <= startIndex) )
    {
        // Load status
        TxBuffer[TxData++] = ZDO_NOT_SUPPORTED; 
        PrepareMessageResponse( MGMT_RTG_rsp );
        return;   
    }
    
    /* at least one valid entry found beyond the startIndex so prepare packet */ 
    TxBuffer[TxData++]  = SUCCESS;          /* status */
    TxBuffer[TxData++]  = validEntries;     /* Total Entries */
    TxBuffer[TxData++]  = startIndex;       /* startIndex */
    listLocation        = TxData;           
    TxBuffer[TxData++]  = validEntries;     /* this will be finalized later */
    
    /* Now load up the RoutingTableList entries  */
    for( count = startIndex; count < validEntries; count++)
    {
       TxBuffer[TxData++] =  localRoutingTable[count].destAddress.byte.LSB; 
       TxBuffer[TxData++] =  localRoutingTable[count].destAddress.byte.MSB; 
       
       /* zero out bit fields first, then OR in each flagfield individually */
       TxBuffer[TxData]   = 0x00; 
       TxBuffer[TxData++] = ( ((localRoutingTable[count].rtflags.rtbits.bRouteRecordRequired & 1) << 5 ) | 
                              ((localRoutingTable[count].rtflags.rtbits.bManyToOne           & 1) << 4 ) |
                              ((localRoutingTable[count].rtflags.rtbits.bNoRouteCache        & 1) << 3 ) |
                              ((localRoutingTable[count].rtflags.rtbits.bStatus              & 7) << 0 )
                            );
                              
       TxBuffer[TxData++] =  localRoutingTable[count].nextHop.byte.LSB;
       TxBuffer[TxData++] =  localRoutingTable[count].nextHop.byte.MSB;
    }   
    

    /* update the TxBuffer with total entries that are included */
    TxBuffer[listLocation] = validEntries - startIndex;
    PrepareMessageResponse(MGMT_RTG_rsp);
    
    return;
}

//#endif  /* end I_AM_FFD */

#endif /* end defined (__PIC24FJ256GB110__) || defined(__PIC32MX__)*/

BOOL handleMatchDescReq(void)
{
    BOOL                    checkProfileOnly;
    BYTE                    descIndex;
    WORD_VAL                *inClusterList;
    BYTE                    listIndex;
    BOOL                    match;
    BYTE                    numInClusters;
    BYTE                    numMatchingEPs;
    BYTE                    numOutClusters;
    WORD_VAL                *outClusterList;
    WORD_VAL                profileID;
    NODE_SIMPLE_DESCRIPTOR  *simpleDescriptor;
    BYTE                    wasBroadcast;
    BYTE                    i;
    BOOL                    responseRequired;             // added to fix buffer issue Bug id:9028
    BYTE                    status;

    // Save the broadcast indication for later.
    wasBroadcast = params.ZDO_DATA_indication.WasBroadcast;

    checkProfileOnly   = FALSE;
    inClusterList      = NULL;
    numMatchingEPs     = 0;
    outClusterList     = NULL;
    responseRequired = TRUE;  //  // added to fix buffer issue Bug id:9028
    status = 0x86;
    if ((simpleDescriptor = (NODE_SIMPLE_DESCRIPTOR *)SRAMalloc( sizeof(NODE_SIMPLE_DESCRIPTOR) )) == NULL)
    {
        responseRequired = FALSE;  //  // added to fix buffer issue Bug id:9028
        goto BailFromMatchDesc;
    }

    profileID.byte.LB = ZDOGet();
    profileID.byte.HB = ZDOGet();
    numInClusters = ZDOGet();

    if (numInClusters)
    {
        if ((inClusterList = (WORD_VAL *)SRAMalloc( numInClusters * sizeof(WORD) )) == NULL)
        {
            responseRequired = FALSE;   // added to fix buffer issue Bug id:9028
            goto BailFromMatchDesc;
        }
        for (i=0; i<numInClusters; i++)
        {
            inClusterList[i].v[0] = ZDOGet();
            inClusterList[i].v[1] = ZDOGet();
        }
    }
    numOutClusters = ZDOGet();
    if (numOutClusters)
    {
        if ((outClusterList = (WORD_VAL *)SRAMalloc( numOutClusters * sizeof(WORD) )) == NULL)
        {
            responseRequired = FALSE;   // added to fix buffer issue Bug id:9028
            goto BailFromMatchDesc;
        }
        for (i=0; i<numOutClusters; i++)
        {
            outClusterList[i].v[0] = ZDOGet();
            outClusterList[i].v[1] = ZDOGet();
        }
    }

    if ((numInClusters == 0) && (numOutClusters == 0))
    {
        checkProfileOnly = TRUE;
    }

    // Set the data pointer to the matching endpoint list in the response message.
    TxData += 4 + MSG_HEADER_SIZE;

    // See if any of the input clusters match any of our input clusters, or if
    // any of the output clusters match any of our output clusters, in any of our
    // endpoints with a given profile ID.  Do not check the ZDO endpoint (0).
    for (i=1; i < NUM_DEFINED_ENDPOINTS; i++)
    {
        match = FALSE;
        ProfileGetSimpleDesc( simpleDescriptor, i );
        if (simpleDescriptor->AppProfId.Val == profileID.Val)
        {
            if (checkProfileOnly)
            {
                match = TRUE;
            }
            else
            {
                for (descIndex=0; descIndex<simpleDescriptor->AppInClusterCount; descIndex++)
                {
                    for (listIndex=0; listIndex<numInClusters; listIndex++)
                    {
                        /* for Zigbee 2006: Bug fix so that comparison will work */
                        if (inClusterList[listIndex].Val == simpleDescriptor->AppInClusterList[descIndex])
                        {
                            match = TRUE;
							break;
                        }
                    }
                }
                
                if( FALSE == match )
                {
                    for (descIndex=0; descIndex<simpleDescriptor->AppOutClusterCount; descIndex++)
                    {
                        for (listIndex=0; listIndex<numOutClusters; listIndex++)
                        {
                            /* for ZigBee 2006: Bug fix such that comparison will work */
                            if (outClusterList[listIndex].Val == simpleDescriptor->AppOutClusterList[descIndex])
                            {
                                match = TRUE;
								break;
                            }
                        }
                    }
                }
            }

        }
        
        if( TRUE == match )
        {
          numMatchingEPs++;
          status = SUCCESS;
          TxBuffer[TxData++] = simpleDescriptor->Endpoint;
        }
    }
    
    if ( (status == 0x86) && wasBroadcast )
    {
        responseRequired = FALSE;
        goto BailFromMatchDesc;
    }


    // Load the response message and send it.  TxData is already set from
    // above, so we'll use i to load up the initial part of the response.
    // Skip over the header info.
    i = TX_DATA_START + MSG_HEADER_SIZE;

    // Load status
    TxBuffer[i++] = status;

    // Load our short address.
    TxBuffer[i++] = macPIB.macShortAddress.byte.LSB;
    TxBuffer[i++] = macPIB.macShortAddress.byte.MSB;

    // Load the number of matching endpoints. The endpoints themselves are already there.
    TxBuffer[i++] = numMatchingEPs;

    PrepareMessageResponse( MATCH_DESC_rsp );

    // This request may have been broadcast. If so, we must request an APS ACK on the response.
    if (wasBroadcast)
    {
        params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
    }

BailFromMatchDesc:
    if (simpleDescriptor != NULL)
        nfree( simpleDescriptor );
    if (inClusterList != NULL)
        nfree( inClusterList );
    if (outClusterList != NULL)
        nfree( outClusterList );


    if(responseRequired == FALSE)
    {
        ZigBeeUnblockTx();
    }
    return responseRequired;   // added to fix buffer issue Bug id:9028

}



/*********************************************************************
 * Function:        BOOL ProcessBindAndUnbind( BIND_INFO bindInfo,
                    LONG_ADDR *sourceAddress, LONG_ADDR *destinationAddress )
 *
 * PreCondition:    If bindInfo.bFromUpperLayers is false, we are coming
 *                  from ZDO_DATA_indication, and asdu still points to the
 *                  beginning of the data area.
 *
 * Input:           bindInfo - the bind direction (bind/unbind) and if the
 *                      request is from upper layers or a received message.
 *
 * Output:          TRUE - process completed successfully, response ready
 *                  FALSE - response not generated
 *                  params.ZDO_BIND_req.Status updated
 *
 * Side Effects:    Binding created/destroyed.
 *
 * Overview:        This function performs a bind or unbind request.  The
 *                  request can be from either the upper layers or a
 *                  received message.
 *
 * Note:            The parameters for ZDO_BIND_req and ZDO_UNBIND_req
 *                  overlay each other, so we will just reference
 *                  ZDO_BIND_req parameters.
 *                  NOTE the spec is very vague on this function.  The
 *                  bind/unbind functions receive long addresses, but
 *                  bindings are created with short addresses. The spec
 *                  does not say how to obtain the short address.
 *
 *                  Coordinators may bind anyone to anyone.  Routers
 *                  may only bind themselves as the source.
 *
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS)

BOOL ProcessBindAndUnbind( BYTE bindInfo, LONG_ADDR *sourceAddress, BYTE destAddrMode, ADDR *destinationAddress )
{
    int         i;
    BOOL        needInfo;
    SHORT_ADDR  tempSrcAddress;
    SHORT_ADDR  tempDstAddress;
    SHORT_ADDR  tempRequestorAddress;

    needInfo = FALSE;
    params.ZDO_BIND_req.Status = SUCCESS;
    tempRequestorAddress       = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
    // Make sure that specified source and destination long addresses are
    // in our PAN.

    if( NOW_I_AM_NOT_A_CORDINATOR() && NOW_I_AM_NOT_A_ROUTER()){//#if !defined (I_AM_COORDINATOR) && !defined (I_AM_ROUTER)
        if (NWKThisIsMyLongAddress( sourceAddress ))
        {
            // I am the source
            tempSrcAddress = macPIB.macShortAddress;
        }
        else
        {
            // Routers can only bind themselves as the source.
            params.ZDO_BIND_req.Status = BIND_NOT_SUPPORTED;
        }
    }else{//#else

        if ( NWKLookupNodeByLongAddr( sourceAddress ) == INVALID_NEIGHBOR_KEY )
        {
            if (NWKThisIsMyLongAddress( sourceAddress ))
            {
                // It is my address
                tempSrcAddress = macPIB.macShortAddress;
            }
            else
            {
                #if MAX_APS_ADDRESSES > 0
                if ( APSFromLongToShort(sourceAddress ) )
                {
                    tempSrcAddress = currentAPSAddress.shortAddr;
                }
                else
                #endif
                {
                    // Unknown source address.
                    tempSrcAddress.Val = 0xFFFF;
                    needInfo = TRUE;
                }
            }
        }
        else
        {
            tempSrcAddress = currentNeighborRecord.shortAddr;
        }
    }//#endif

    if( destAddrMode != APS_ADDRESS_GROUP )
    {
        if ( NWKLookupNodeByLongAddr( &destinationAddress->LongAddr ) == INVALID_NEIGHBOR_KEY )
        {
            if (NWKThisIsMyLongAddress( &destinationAddress->LongAddr ))
            {
                // It is my address
                tempDstAddress = macPIB.macShortAddress;
            }
            else
            {
                #if MAX_APS_ADDRESSES > 0
                if ( APSFromLongToShort( &(destinationAddress->LongAddr) ) )
                {
                    tempDstAddress = currentAPSAddress.shortAddr;
                }
                else
                #endif
                {
                    // Unknown destination address
                    tempDstAddress.Val = 0xFFFF;
                    needInfo = TRUE;
                }
            }
        }
        else
        {
            tempDstAddress = currentNeighborRecord.shortAddr;
        }
    } // if destAddMode
    else
    {
        tempDstAddress.Val = destinationAddress->ShortAddr.Val;
    }


    // Load the parameters in case we need them
    if ((bindInfo & BIND_SOURCE_MASK) == BIND_FROM_EXTERNAL)
    {
        /* If the request is from the upper layers, the parameters are already in place.
         * Otherwise, load them.
        * Skip over the source address.
        */
        params.ZDO_BIND_req.SrcEndp         = *(params.ZDO_DATA_indication.asdu + 8);
        params.ZDO_BIND_req.ClusterID.v[0]  = *(params.ZDO_DATA_indication.asdu + 9);
        params.ZDO_BIND_req.ClusterID.v[1]  = *(params.ZDO_DATA_indication.asdu + 10);
        params.ZDO_BIND_req.DstAddrMode     = *(params.ZDO_DATA_indication.asdu + 11);

        params.ZDO_DATA_indication.asdu += 12;


        // Skip over the destination address.
        if( params.ZDO_BIND_req.DstAddrMode != APS_ADDRESS_GROUP )
        {
            params.ZDO_DATA_indication.asdu += 8;
            params.ZDO_BIND_req.DstEndp = ZDOGet();
        }
        else
        {
            params.ZDO_BIND_req.DstEndp = 0xFE;
        }
    }

    // If we need more information, save off what we have and trigger background processing
    if (needInfo)
    {
        if ((pBindInProgressInfo != NULL) ||
            ((pBindInProgressInfo = (BIND_IN_PROGRESS_INFO *)SRAMalloc( sizeof(BIND_IN_PROGRESS_INFO) )) == NULL))
        {
            // We are already waiting for one binding; we cannot do two.
            // Or we could not allocate memory for the bind information.
            params.ZDO_BIND_req.Status = BIND_NOT_SUPPORTED;
            goto ReturnBindResult;
        }

        for (i=0; i<8; i++)
        {
            pBindInProgressInfo->sourceAddressLong.v[i]         = sourceAddress->v[i];
        }
        pBindInProgressInfo->sourceAddressShort                 = tempSrcAddress;
        pBindInProgressInfo->sourceEP                           = params.ZDO_BIND_req.SrcEndp;
        pBindInProgressInfo->cluster.Val                        = params.ZDO_BIND_req.ClusterID.Val;
        for (i=0; i<8; i++)
        {
            pBindInProgressInfo->destinationAddressLong.v[i]    = destinationAddress->v[i];
        }
        pBindInProgressInfo->destinationAddressShort            = tempDstAddress;
        pBindInProgressInfo->destinationEP                      = params.ZDO_BIND_req.DstEndp;
        pBindInProgressInfo->timeStamp                          = TickGet();
        pBindInProgressInfo->sequenceNumber                     = sequenceNumber;   // Note - this is garbage if it came from our upper layers. But it's not used.
        pBindInProgressInfo->status.val                         = 0;

        if ((bindInfo & BIND_SOURCE_MASK) == BIND_FROM_EXTERNAL)
        {
            /* For Zigbee2006: ZDO_indication was over written so can't use here */
            pBindInProgressInfo->requestorAddress   = tempRequestorAddress;
        }
        else
        {
            // We don't really need this - we know it's from our upper layers.
            //pBindInProgressInfo->requestorAddress             = macPIB.macShortAddress;
        }

        // Set the bind direction.
        if ((bindInfo & BIND_DIRECTION_MASK) == BIND_NODES)
        {
            pBindInProgressInfo->status.bits.bBindNodes             = 1;
        }


        // Set where the bind request came from.
        if ((bindInfo & BIND_SOURCE_MASK) == BIND_FROM_UPPER_LAYERS)
        {
            pBindInProgressInfo->status.bits.bFromUpperLayers       = 1;
        }

        // Set the status flags so we don't request addresses that we already have.
        if (tempSrcAddress.Val != 0xFFFF)
        {
            pBindInProgressInfo->status.bits.bSourceRequested       = 1;
        }
        if (tempDstAddress.Val != 0xFFFF)
        {
            pBindInProgressInfo->status.bits.bDestinationRequested  = 1;
        }

        zdoStatus.flags.bits.bBinding = 1;

        return FALSE;
    }
    else
    {
        params.ZDO_BIND_req.SrcAddress.ShortAddr = tempSrcAddress;
        params.ZDO_BIND_req.DstAddress.ShortAddr = tempDstAddress;
    }

    if (params.ZDO_BIND_req.Status == SUCCESS)
    {
        if ((bindInfo & BIND_DIRECTION_MASK) == BIND_NODES)
        {
            if (APSAddBindingInfo( params.ZDO_BIND_req.SrcAddress.ShortAddr, params.ZDO_BIND_req.SrcEndp,
                params.ZDO_BIND_req.ClusterID, params.ZDO_BIND_req.DstAddress.ShortAddr, params.ZDO_BIND_req.DstEndp))
            {
                params.ZDO_BIND_req.Status = ZDO_TABLE_FULL;
            }
        }
        else
        {
            if (APSRemoveBindingInfo( params.ZDO_BIND_req.SrcAddress.ShortAddr, params.ZDO_BIND_req.SrcEndp,
                params.ZDO_BIND_req.ClusterID, params.ZDO_BIND_req.DstAddress.ShortAddr, params.ZDO_BIND_req.DstEndp))
            {
                params.ZDO_BIND_req.Status = ZDO_NO_ENTRY;
            }
        }
    }

ReturnBindResult:

    if ((bindInfo & BIND_SOURCE_MASK) == BIND_FROM_UPPER_LAYERS)
    {
        return SendUpBindResult( params.ZDO_BIND_req.Status, (bindInfo & BIND_DIRECTION_MASK) );
    }
    return TRUE;
}

#endif


/*********************************************************************
 * Function:        void SendBindAddressRequest( BYTE requestSource )
 *
 * PreCondition:    pBindInProgressInfo must be non-NULL and point to
 *                  valid information
 *
 * Input:           requestSource - if we need to request the source
 *                  address (TRUE) or the destination address (FALSE)
 *
 * Output:          None.
 *
 * Side Effects:    Message is loaded for transmission.
 *
 * Overview:        This function requests either the source or destination
 *                  address of a binding that is in progress.
 *
 * Note:            The NWK_ADDR_rsp must be captured.
 ********************************************************************/
 
 

 
#if defined(I_SUPPORT_BINDINGS)



void SendBindAddressRequest( BYTE requestSource )
{
    BYTE i;

    // Send NWK_ADDR_req message
    ZigBeeBlockTx();

    TxBuffer[TxData++] = ZDOCounter++;

    // IEEEAddr
    if (requestSource)
    {
        for(i = 0; i < 8; i++)
        {
            TxBuffer[TxData++] = pBindInProgressInfo->sourceAddressLong.v[i];
        }
    }
    else
    {
        for(i = 0; i < 8; i++)
        {
            TxBuffer[TxData++] = pBindInProgressInfo->destinationAddressLong.v[i];
        }
    }

    // RequestType
    TxBuffer[TxData++] = 0x00;

    // StartIndex
    TxBuffer[TxData++] = 0x00;

    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
    params.APSDE_DATA_request.DstEndpoint = EP_ZDO;
    params.APSDE_DATA_request.DstAddress.ShortAddr.Val = 0xFFFD;

    params.APSDE_DATA_request.ProfileId.Val = ZDO_PROFILE_ID;
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;
    params.APSDE_DATA_request.TxOptions.Val = DEFAULT_TX_OPTIONS;
    params.APSDE_DATA_request.SrcEndpoint = EP_ZDO;
    params.APSDE_DATA_request.ClusterId.Val = NWK_ADDR_req;
}
#endif


/*********************************************************************
 * Function:        BOOL SendUpBindResult( BYTE status, BYTE bindNodes )
 *
 * PreCondition:    None
 *
 * Input:           status - binding status to send
 *                  bindNodes - 1 = bind, 0 = unbind
 *
 * Output:          TRUE - packet created
 *                  FALSE - packet not created
 *
 * Side Effects:    Fake APSDE message is created
 *
 * Overview:        This function creates a fake APSDE message in
 *                  response to a bind request made from the upper
 *                  layers.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS)

BOOL SendUpBindResult( BYTE status, BYTE bindNodes )
{
    BYTE    *ptr;

    // Create a fake APSDE_DATA_indication with the answer, as though it came from
    // a different node. This way the user can capture the response the same way for
    // both sources.
    if (CurrentRxPacket == NULL)
    {
        if ((CurrentRxPacket = SRAMalloc(4)) != NULL)
        {
            ptr = CurrentRxPacket;

            *ptr++ = 0;                                 // Transaction Sequence Number

            // Load data.
            *ptr = status;

            // Populate the remainder of the parameters.
            params.APSDE_DATA_indication.asduLength                 = 4;
            params.APSDE_DATA_indication.SecurityStatus             = FALSE;
            params.APSDE_DATA_indication.asdu                       = CurrentRxPacket;
            params.APSDE_DATA_indication.ProfileId.Val              = ZDP_PROFILE_ID;
            params.APSDE_DATA_indication.SrcAddrMode                = APS_ADDRESS_16_BIT;
            params.APSDE_DATA_indication.WasBroadcast               = FALSE;
            params.APSDE_DATA_indication.SrcAddress.ShortAddr       = macPIB.macShortAddress;
            params.APSDE_DATA_indication.SrcEndpoint                = EP_ZDO;
            params.APSDE_DATA_indication.DstEndpoint                = EP_ZDO;
            if (bindNodes)
            {
                params.APSDE_DATA_indication.ClusterId.Val          = BIND_rsp;
            }
            else
            {
                params.APSDE_DATA_indication.ClusterId.Val          = UNBIND_rsp;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}
#endif


/*********************************************************************
 * Function:        BYTE Initdefault_SAS ()
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:          default_SAS initialised in NVM
 *
 * Side Effects:
 *
 * Overview:        This function initialises default_SAS with factory default settings
 *
 * Note:            None
 ********************************************************************/

void Initdefault_SAS (void)
{
    STARTUP_PARAMETERS_ATTRIBUTE_SET *pSpas;
    JOIN_PARAMETERS_ATTRIBUTE_SET *pJpas;
    END_DEVICE_PARAMETERS_ATTRIBUTE_SET *pEdpas;
    CONCENTRATOR_PARAMETERS_ATTRIBUTE_SET *pCpas;
    BYTE *pTemp;

    pSpas = &current_SAS.spas;
    pJpas = &current_SAS.jpas;
    pEdpas = &current_SAS.edpas;
    pCpas = &current_SAS.cpas;

    current_SAS.validitykey                         = SAS_TABLE_VALID;

    pSpas->shortAddr.Val                    = NWK_SHORT_ADDRESS;
    pTemp = &pSpas->ExtendedPANId.v[0];
    *pTemp++                = NWK_EXTENDED_PAN_ID_BYTE0;
    *pTemp++                = NWK_EXTENDED_PAN_ID_BYTE1;
    *pTemp++                = NWK_EXTENDED_PAN_ID_BYTE2;
    *pTemp++                = NWK_EXTENDED_PAN_ID_BYTE3;
    *pTemp++                = NWK_EXTENDED_PAN_ID_BYTE4;
    *pTemp++                = NWK_EXTENDED_PAN_ID_BYTE5;
    *pTemp++                = NWK_EXTENDED_PAN_ID_BYTE6;
    *pTemp++                = NWK_EXTENDED_PAN_ID_BYTE7;

    pSpas->PANId.Val                        = MAC_DEFAULT_PANID;
    pSpas->ChannelMask.Val              = ALLOWED_CHANNELS;
    pSpas->ProtocolVersion              = nwkcProtocolVersion;
    pSpas->StackProfile                 = PROFILE_myStackProfileId;
    pSpas->StartupControl                   = DEFAULT_STARTUP_CONTROL;
    pTemp = &pSpas->TrustCenterAddress.v[0];
    *pTemp++        = TRUST_CENTER_LONG_ADDR_BYTE0;
    *pTemp++        = TRUST_CENTER_LONG_ADDR_BYTE1;
    *pTemp++        = TRUST_CENTER_LONG_ADDR_BYTE2;
    *pTemp++        = TRUST_CENTER_LONG_ADDR_BYTE3;
    *pTemp++        = TRUST_CENTER_LONG_ADDR_BYTE4;
    *pTemp++        = TRUST_CENTER_LONG_ADDR_BYTE5;
    *pTemp++        = TRUST_CENTER_LONG_ADDR_BYTE6;
    *pTemp++        = TRUST_CENTER_LONG_ADDR_BYTE7;

    pTemp = &pSpas->TrustCenterMasterKey[0];
    *pTemp++        = TRUST_CENTER_MASTER_KEY0;
    *pTemp++        = TRUST_CENTER_MASTER_KEY1;
    *pTemp++        = TRUST_CENTER_MASTER_KEY2;
    *pTemp++        = TRUST_CENTER_MASTER_KEY3;
    *pTemp++        = TRUST_CENTER_MASTER_KEY4;
    *pTemp++        = TRUST_CENTER_MASTER_KEY5;
    *pTemp++        = TRUST_CENTER_MASTER_KEY6;
    *pTemp++        = TRUST_CENTER_MASTER_KEY7;
    *pTemp++        = TRUST_CENTER_MASTER_KEY8;
    *pTemp++        = TRUST_CENTER_MASTER_KEY9;
    *pTemp++        = TRUST_CENTER_MASTER_KEY10;
    *pTemp++        = TRUST_CENTER_MASTER_KEY11;
    *pTemp++        = TRUST_CENTER_MASTER_KEY12;
    *pTemp++        = TRUST_CENTER_MASTER_KEY13;
    *pTemp++        = TRUST_CENTER_MASTER_KEY14;
    *pTemp++        = TRUST_CENTER_MASTER_KEY15;

    pTemp = &pSpas->NetworkKey[0];
    *pTemp++                    = NETWORK_KEY_BYTE00;
    *pTemp++                    = NETWORK_KEY_BYTE01;
    *pTemp++                    = NETWORK_KEY_BYTE02;
    *pTemp++                    = NETWORK_KEY_BYTE03;
    *pTemp++                    = NETWORK_KEY_BYTE04;
    *pTemp++                    = NETWORK_KEY_BYTE05;
    *pTemp++                    = NETWORK_KEY_BYTE06;
    *pTemp++                    = NETWORK_KEY_BYTE07;
    *pTemp++                    = NETWORK_KEY_BYTE08;
    *pTemp++                    = NETWORK_KEY_BYTE09;
    *pTemp++                    = NETWORK_KEY_BYTE10;
    *pTemp++                    = NETWORK_KEY_BYTE11;
    *pTemp++                    = NETWORK_KEY_BYTE12;
    *pTemp++                    = NETWORK_KEY_BYTE13;
    *pTemp++                    = NETWORK_KEY_BYTE14;
    *pTemp++                    = NETWORK_KEY_BYTE15;
    pSpas->UseInsecureJoin              = DEFAULTINSECUREJOIN;
    
    pTemp = &pSpas->PreconfiguredLinkKey[0];
    *pTemp++        = PRECONFIGURED_LINK_KEY00;
    *pTemp++        = PRECONFIGURED_LINK_KEY01;
    *pTemp++        = PRECONFIGURED_LINK_KEY02;
    *pTemp++        = PRECONFIGURED_LINK_KEY03;
    *pTemp++        = PRECONFIGURED_LINK_KEY04;
    *pTemp++        = PRECONFIGURED_LINK_KEY05;
    *pTemp++        = PRECONFIGURED_LINK_KEY06;
    *pTemp++        = PRECONFIGURED_LINK_KEY07;
    *pTemp++        = PRECONFIGURED_LINK_KEY08;
    *pTemp++        = PRECONFIGURED_LINK_KEY09;
    *pTemp++        = PRECONFIGURED_LINK_KEY10;
    *pTemp++        = PRECONFIGURED_LINK_KEY11;
    *pTemp++        = PRECONFIGURED_LINK_KEY12;
    *pTemp++        = PRECONFIGURED_LINK_KEY13;
    *pTemp++        = PRECONFIGURED_LINK_KEY14;
    *pTemp++        = PRECONFIGURED_LINK_KEY15;

    pSpas->NetworkKeySeqNum                     = NETWORK_KEY_SEQ;
    pSpas->NetworkKeyType                       = DEFAULT_NWK_KEY_TYPE; //default when StackProfile is 0x02
    pSpas->NetworkManagerAddress.Val            = DEFAULT_NWK_MGR_ADDR; // by default, the Network Manager is on the ZigBee coordinator.

    pJpas->ScanAttempts                         = DEFAULT_SCAN_ATTEMPS;
    pJpas->TimeBetweenScans                     = DEFAULT_TIME_BETWEEN_SCANS;
    pJpas->RejoinInterval                       = DEFAULT_REJOIN_INTERVAL;
    pJpas->MaxRejoinInterval                    = DEFAULT_MAX_REJOIN_INTERVAL;

    pEdpas->IndirectPollRate                    = DEFAULT_INDIRECT_POLL_RATE;
    pEdpas->ParentRetryThreshold                = DEFAULT_PARENT_RETRY_THRESHOLD;
    pCpas->ConcentratorFlag                     = TRUE;
    pCpas->ConcentratorRadius                   = DEFAULT_CONC_RADIUS;
    pCpas->ConcentratorDiscoveryTime            = DEFAULT_CONC_DISC_TIME;  // seconds

       if( MSDCL_Commission.ValidCleanStartUp == MSDCL_COMMISSION_DATA_VALID)
       {
		memcpy(&current_SAS.spas.ExtendedPANId.v[0], &MSDCL_Commission.ExtendedPANId[0], sizeof(MSDCL_Commission.ExtendedPANId) );
                       
	    current_SAS.spas.PreconfiguredLinkKey[15]= MSDCL_Commission.LinkKey[15];
	    current_SAS.spas.PreconfiguredLinkKey[14]= MSDCL_Commission.LinkKey[14];
	    current_SAS.spas.PreconfiguredLinkKey[13]= MSDCL_Commission.LinkKey[13];
	    current_SAS.spas.PreconfiguredLinkKey[12]= MSDCL_Commission.LinkKey[12];
	    current_SAS.spas.PreconfiguredLinkKey[11]= MSDCL_Commission.LinkKey[11];
	    current_SAS.spas.PreconfiguredLinkKey[10]= MSDCL_Commission.LinkKey[10];
	    current_SAS.spas.PreconfiguredLinkKey[9]= MSDCL_Commission.LinkKey[9];
	    current_SAS.spas.PreconfiguredLinkKey[8]= MSDCL_Commission.LinkKey[8];
	    current_SAS.spas.PreconfiguredLinkKey[7]= MSDCL_Commission.LinkKey[7];
	    current_SAS.spas.PreconfiguredLinkKey[6]= MSDCL_Commission.LinkKey[6];
	    current_SAS.spas.PreconfiguredLinkKey[5]= MSDCL_Commission.LinkKey[5];
	    current_SAS.spas.PreconfiguredLinkKey[4]= MSDCL_Commission.LinkKey[4];
	    current_SAS.spas.PreconfiguredLinkKey[3]= MSDCL_Commission.LinkKey[3];
	    current_SAS.spas.PreconfiguredLinkKey[2]= MSDCL_Commission.LinkKey[2];
	    current_SAS.spas.PreconfiguredLinkKey[1]= MSDCL_Commission.LinkKey[1];
	    current_SAS.spas.PreconfiguredLinkKey[0]= MSDCL_Commission.LinkKey[0];

	    current_SAS.spas.ChannelMask.Val = ALLOWED_CHANNELS & 0x07FFF800UL;

   		current_SAS.spas.StartupControl = MSDCL_Commission.StartupStatus;
    
    	//PutNeighborTableInfo();
    	//MSDCL_Commission.DoCleanStartUp = 0;
    	//NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
	}

    #ifdef USE_EXTERNAL_NVM
        PutSAS(default_SAS, &current_SAS);
    #else
        PutSAS(&default_SAS, &current_SAS);
    #endif
}

#ifdef I_SUPPORT_COMMISSIONING
//#define SUCCESS 0
#define FAILURE 1
#define INVALID_FIELD 0x85
#define INSUFFICIENT_SPACE 0x89
extern NWK_STATUS nwkStatus;

/*********************************************************************
 * Function:        BYTE GetActiveSAS ()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          active SAS
 *
 * Side Effects:
 *
 * Overview:        This API is used to read the Active SAS (the one indexed by “activeSASIndex” ) 
 *                  contents and put them onto current_SAS.
 *
 *
 * Note:            None
 ********************************************************************/

BYTE GetActiveSAS ()
{
    BYTE index;

    GetActiveSASIndex(&index);

    if(index == 0xFF)
    {
        #ifdef USE_EXTERNAL_NVM
            GetSAS(&current_SAS,default_SAS);
        #else
            GetSAS(&current_SAS,&default_SAS);
        #endif
    }
    else if (index < MAX_SAS_TABLE_SIZE)
    {
        #ifdef USE_EXTERNAL_NVM
            GetSAS(&current_SAS,(Commissioned_SAS + index * sizeof(STARTUP_ATTRIBUTE_SET)));
        #else
            GetSAS(&current_SAS,&Commissioned_SAS[index]);
        #endif

        if(current_SAS.validitykey != SAS_TABLE_VALID)
        {
            index = 0xFF;
            PutActiveSASIndex(&index);
            #ifdef USE_EXTERNAL_NVM
                GetSAS(&current_SAS,default_SAS);
            #else
                GetSAS(&current_SAS,&default_SAS);
            #endif
        }
    }
    else
    {
        return FAILURE;
    }
    return SUCCESS;
}

/*********************************************************************
 * Function:        STARTUP_ATTRIBUTE_SET* GetCurrentSAS()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          pointer to current_SAS
 *
 * Side Effects:
 *
 * Overview:        This API is used to read the current_SAS(the RAM copy) by app.
 *
 *
 * Note:            None
 ********************************************************************/

STARTUP_ATTRIBUTE_SET* GetCurrentSAS()
{
    return &current_SAS;
}


/*********************************************************************
 * Function:        BYTE RestartDevice (unsigned char options, unsigned char delay, unsigned char jitter)
 *
 * PreCondition:    None
 *
 * Input:           options, delay, jitter
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        This API is used to restart the device with the values specified in the SAS indexed by “activeSASIndex” based on the options paramter.
 *
 *
 * Note:            None
 ********************************************************************/

BYTE RestartDevice (BYTE options, BYTE delay, BYTE jitter)
{

    NwkDiscoveryTries = 0x00;
    NwkJoinTries = 0x00;
    nwkStatus.flags.Val = 0x00;
    ZigBeeInit();
        NWKClearNeighborTable();
    #ifdef I_SUPPORT_ROUTING
        NWKClearRoutingTable();
    #endif
    #if defined(I_SUPPORT_BINDINGS)
        ClearBindingTable();
    #endif
    #ifdef I_SUPPORT_GROUP_ADDRESSING
        RemoveAllGroups();
    #endif

    if (options & 0x01)
    {
        if(NOW_I_AM_A_CORDINATOR()){//#ifdef I_AM_COORDINATOR
            ZigBeeStatus.flags.bits.bNetworkFormed = 0;
        }else{//#else
            ZigBeeStatus.flags.bits.bNetworkJoined = 0;
        }//#endif
        ZigBeeStatus.nextZigBeeState = NO_PRIMITIVE;
    }
    else
    {
        GetActiveSAS (&current_SAS);
        if(NOW_I_AM_A_CORDINATOR()){//#ifdef I_AM_COORDINATOR
            ZigBeeStatus.flags.bits.bNetworkFormed = 0;
        }else{//#else
            ZigBeeStatus.flags.bits.bNetworkJoined = 0;
        }//#endif
        ZigBeeStatus.nextZigBeeState = NO_PRIMITIVE;
    }
    return NO_PRIMITIVE;

}

/*********************************************************************
 * Function:        BYTE SaveSAS (STARTUP_ATTRIBUTE_SET* ptr_current_SAS, BYTE options, BYTE index)
 *
 * PreCondition:    None
 *
 * Input:           *current_SAS, index
 *
 * Output:          current_SAS store at index
 *
 * Side Effects:
 *
 * Overview:        This API is used by application to store a new set of SAS at a particular index
 *
 *
 * Note:            This function does not free the pointer ptr_current_SAS
 ********************************************************************/
BYTE SaveSAS (STARTUP_ATTRIBUTE_SET* ptr_current_SAS, BYTE options, BYTE index)
{
    if (index >= MAX_SAS_TABLE_SIZE)
    {
        return INSUFFICIENT_SPACE;
    }
    #ifdef USE_EXTERNAL_NVM
        PutSAS((Commissioned_SAS + index * sizeof(STARTUP_ATTRIBUTE_SET)), ptr_current_SAS);
    #else
        PutSAS(&Commissioned_SAS[index], ptr_current_SAS);
    #endif
    return SUCCESS;
}

/*********************************************************************
 * Function:        BYTE RestoreSAS (STARTUP_ATTRIBUTE_SET* ptr_current_SAS, BYTE options, BYTE index)
 *
 * PreCondition:    None
 *
 * Input:           *current_SAS, index
 *
 * Output:          current_SAS store at index
 *
 * Side Effects:
 *
 * Overview:        This API is used by application to restore a new set of SAS from a particular index
 *
 *
 * Note:            This function does not free the pointer ptr_current_SAS
 ********************************************************************/
BYTE RestoreSAS (STARTUP_ATTRIBUTE_SET* ptr_current_SAS, BYTE options, BYTE index)
{
    if (index >= MAX_SAS_TABLE_SIZE)
    {
        return INVALID_FIELD;
    }
    #ifdef USE_EXTERNAL_NVM
        GetSAS(ptr_current_SAS, (Commissioned_SAS + index * sizeof(STARTUP_ATTRIBUTE_SET)));
    #else
        GetSAS(ptr_current_SAS, &Commissioned_SAS[index]);
    #endif
    if( ptr_current_SAS->validitykey != SAS_TABLE_VALID )
    {
        return INVALID_FIELD;
    }
    return SUCCESS;
}

/*********************************************************************
 * Function:        BYTE ResetSAStablevalidity(BYTE index)
 *
 * PreCondition:    None
 *
 * Input:           index
 *
 * Output:          status
 *
 * Side Effects:
 *
 * Overview:        This API is used by application to reset the 
 *                  validity key of the saved table.
 *
 *
 * Note:            None
 ********************************************************************/
BYTE ResetSAStablevalidity(BYTE index)
{
    STARTUP_ATTRIBUTE_SET local_SAS;
    if (index >= MAX_SAS_TABLE_SIZE)
    {
        return FAILURE;
    }
    #ifdef USE_EXTERNAL_NVM
        GetSAS(&local_SAS, (Commissioned_SAS + index * sizeof(STARTUP_ATTRIBUTE_SET)));
    #else
        GetSAS(&local_SAS, &Commissioned_SAS[index]);
    #endif
    local_SAS.validitykey = 0x0000;/*Invalid*/
    #ifdef USE_EXTERNAL_NVM
        PutSAS((Commissioned_SAS + index * sizeof(STARTUP_ATTRIBUTE_SET)), &local_SAS);
    #else
        PutSAS(&Commissioned_SAS[index], &local_SAS);
    #endif
    
    return SUCCESS;
}

/*********************************************************************
 * Function:        void ResetAllSAStablevalidity(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:        This API is used by application to reset the 
 *                  validity key of the all entries saved table.
 *
 *
 * Note:            None
 ********************************************************************/
void ResetAllSAStablevalidity(void)
{
    BYTE index;
    
    for(index = 0; index < MAX_SAS_TABLE_SIZE; index++)
    {
        ResetSAStablevalidity(index);
    }    
}

/*********************************************************************
 * Function:        BYTE SetActiveIndex (BYTE options, BYTE index)
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        This API will be used to copy “index” parameter on to “activeSASIndex”.
 *
 *
 * Note:            None
 ********************************************************************/
void SetActiveIndex (BYTE options, BYTE index)
{
    PutActiveSASIndex(&index);

    #ifdef I_SUPPORT_SECURITY
        BYTE i;
        i = 1;
        PutNwkActiveKeyNumber(&i);

        GetActiveSAS (&current_SAS);
        BYTE key[16];
        for ( i = 0 ; i < 16 ; i++)
        {
            key[i] = current_SAS.spas.NetworkKey[i];
        }

        BYTE zero[16]={0};
        if( memcmp( (void *)&(key[0]), (void *)&(zero[0]), 16 ) )
        {
            // Valid Network Key.
            memcpy((void *)&(currentNetworkKeyInfo.NetKey), (void *)key, 16);
        }
        else
        {
            // Invalid Network Key (All Zeros). So, we need to generate the Random Network Key.
            for ( i = 0 ; i < 16 ; i++)
            {
                key[i] = RANDOM_LSB;
            }
            memcpy((void *)&(currentNetworkKeyInfo.NetKey), (void *)key, 16);
        }

        currentNetworkKeyInfo.SeqNumber.v[0] = current_SAS.spas.NetworkKeySeqNum;
        currentNetworkKeyInfo.SeqNumber.v[1] = nwkMAGICResSeq;
        #ifdef USE_EXTERNAL_NVM
            plainSecurityKey[0] = currentNetworkKeyInfo;
            SetSecurityKey(0, currentNetworkKeyInfo);
            plainSecurityKey[1].SeqNumber.v[1] = 0xFF; // disable key 2
        #else
            PutNwkKeyInfo( &networkKeyInfo , &currentNetworkKeyInfo );
        #endif

        OutgoingFrameCount[0].Val = 0;
        OutgoingFrameCount[1].Val = 0;
        for(i = 0; i < MAX_NEIGHBORS; i++)
        {
            IncomingFrameCount[0][i].Val = 0;
            IncomingFrameCount[1][i].Val = 0;
        }
    #endif // #ifdef I_SUPPORT_SECURITY
}

/*********************************************************************
 * Function:        BYTE ResetSAS (BYTE Options, BYTE index)
 *
 * PreCondition:    None
 *
 * Input:           options-reset options
 *                  index-if the option is reset a particular index 
 *                  this field specifies the index.
 *
 * Output:          Reset status.
 *
 * Side Effects:
 *
 * Overview:        This API will be used to reset the SAS table 
 *                  values to default values depending on option.
 *
 * Note:            None
 ********************************************************************/
BYTE ResetSAS (BYTE options, BYTE index)
{
    STARTUP_ATTRIBUTE_SET local_SAS;

    #ifdef USE_EXTERNAL_NVM
        GetSAS(&local_SAS, default_SAS);
    #else
        GetSAS(&local_SAS, &default_SAS);
    #endif

    if(options & 0x04)  // Reset index
    {
        if(index < MAX_SAS_TABLE_SIZE)
        {
            #ifdef USE_EXTERNAL_NVM
                PutSAS((Commissioned_SAS + index * sizeof(STARTUP_ATTRIBUTE_SET)), &local_SAS);
            #else
                PutSAS(&Commissioned_SAS[index], &local_SAS);
            #endif
            ResetSAStablevalidity(index);
            return SUCCESS;
        }
        else
        {
            return FAILURE; //"invalid Index"
        }
    }
    if(options & 0x02) //reset all
    {
        for(index = 0; index < MAX_SAS_TABLE_SIZE; index++)
        {
            #ifdef USE_EXTERNAL_NVM
                PutSAS((Commissioned_SAS + index * sizeof(STARTUP_ATTRIBUTE_SET)), &local_SAS);
            #else
                PutSAS(&Commissioned_SAS[index], &local_SAS);
            #endif
        }
        index = 0xFF;
        PutActiveSASIndex(&index);
        return SUCCESS;
    }
    return FAILURE; //"invalid option"
 }

#endif

/*********************************************************************
 * Function:        void SaveCurrentSAS (void)
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        This API will be used to copy current_SAS to currently active SAS in NVM.
 *
 *
 * Note:            None
 ********************************************************************/
void SaveCurrentSAS (void)
{
    #ifdef I_SUPPORT_COMMISSIONING
        BYTE activeSasIndex;
        GetActiveSASIndex(&activeSasIndex);
        if(activeSasIndex < MAX_SAS_TABLE_SIZE)
        {
            #ifdef USE_EXTERNAL_NVM
                PutSAS((Commissioned_SAS + activeSasIndex * sizeof(STARTUP_ATTRIBUTE_SET)), &current_SAS);
            #else
                PutSAS(&Commissioned_SAS[activeSasIndex], &current_SAS);
            #endif
        }
        else
        {
            #ifdef USE_EXTERNAL_NVM
                PutSAS(default_SAS, &current_SAS);
            #else
                PutSAS(&default_SAS, &current_SAS);
            #endif
        }
    #else
            #ifdef USE_EXTERNAL_NVM
                PutSAS(default_SAS, &current_SAS);
            #elif
                PutSAS(&default_SAS, &current_SAS);
            #endif
    #endif
}


/*********************************************************************
 * Function:        void updateCurrentSAS( STARTUP_ATTRIBUTE_SET *ptr_current_SAS )
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        This API will be used to copy Given SAS to current_SAS.
 *
 *
 * Note:            None
 ********************************************************************/
void updateCurrentSAS( STARTUP_ATTRIBUTE_SET *ptr_current_SAS )
{
    memcpy
    ( 
        (BYTE *)&current_SAS, 
        (BYTE *)ptr_current_SAS, 
        sizeof( STARTUP_ATTRIBUTE_SET )
    );
}

/***************************************************************************************
 * Function:        void updateCommissioningParameters(void)
 *
 * PreCondition:    None
 *
 * Input:           None.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Updates the global stack parameters which needs to be directly 
 *                  changed from commissioning attribute set.
 *
 * Note:            None
 ***************************************************************************************/
void updateCommissioningParameters(void)
{
#if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
    
    if(!I_AM_TRUST_CENTER){ //#ifndef I_AM_TRUST_CENTER
        BYTE index;
    
        TCLinkKeyInfo.trustCenterShortAddr.Val = 0x0000; // Always TC is ZC
        for (index = 0; index < 8; index++)
        {
            TCLinkKeyInfo.trustCenterLongAddr.v[index] = current_SAS.spas.TrustCenterAddress.v[index];
        }
		
        for (index = 0; index < 16; index++)
        {
            TCLinkKeyInfo.link_key.v[index] = current_SAS.spas.PreconfiguredLinkKey[index];
        }
    }//#endif
#endif
}

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
#ifndef I_AM_END_DEVICE
/*********************************************************************
 * Function:        void SetNextChildAddr(WORD shortAddr)
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        This API will be used to set the next addr to be assigned to a child device.
 *
 *
 * Note:            None
 ********************************************************************/
void SetNextChildAddr(WORD shortAddr)
{
    currentNeighborTableInfo.nextChildAddr.Val = shortAddr;
    PutNeighborTableInfo();
}

//#ifndef I_AM_COORDINATOR
/***************************************************************************************
 * Function:        ZIGBEE_PRIMITIVE ResolveLocalAddressConflict(  )
 *
 * PreCondition:    None
 *
 * Input:           shortAddress
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Resolvs address conflict
 *
 * Note:            None
 ***************************************************************************************/
ZIGBEE_PRIMITIVE ResolveLocalAddressConflict( void )
{
    macPIB.macShortAddress.Val = currentNeighborTableInfo.nextChildAddr.Val;
    MLME_SET_macShortAddress_hw();
    nwkStatus.flags.bits.bNextChildAddressUsed = TRUE;

    appNotify.current_ADDR.Val = macPIB.macShortAddress.Val;
    appNotification = TRUE;

    ZigBeeStatus.flags.bits.bTriggerDeviceAnnce = 1;
    return NO_PRIMITIVE;
}
//#endif
#endif
/*********************************************************************
 * Function:        void SetOwnShortAddr(WORD shortAddr)
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        This API will be used to set the own short addr.
 *
 *
 * Note:            None
 ********************************************************************/
void SetOwnShortAddr(WORD shortAddr)
{
    macPIB.macShortAddress.Val  = shortAddr;
    MLME_SET_macShortAddress_hw();
}
#endif //I_SUPPORT_STOCHASTIC_ADDRESSING

//#if defined(MERGED_STACK) || !defined(I_AM_COORDINATOR)
/***************************************************************************************
 * Function:        static ZIGBEE_PRIMITIVE SubmitJoinRequest()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Next primitive to be executed.
 *
 * Side Effects:    None
 *
 * Overview:        Kind of Join procedure is decided and issued from this function.
 *
 * Note:            None
 ***************************************************************************************/

static ZIGBEE_PRIMITIVE SubmitJoinRequest()
{
    BYTE zero[8]={0};
    ZIGBEE_PRIMITIVE nextPrimitive;

    if (NetworkDescIndex)
    {
        /*  */
        if( memcmp( (void *)&(current_SAS.spas.ExtendedPANId.v[0]), (void *)&(zero[0]), 8 ))
        {
            if (tryToRejoin)
            {
                #ifdef I_SUPPORT_SECURITY
                if( !rejoinWithSec)
                #endif
                {
                    tryToRejoin = 0;
                }
                /* Extended PANID is existing in SAS, Try to do rejoin */
                JoinRequest( current_SAS.spas.ExtendedPANId, REJOIN_PROCESS, 1 );
                macPIB.macPANId.Val = current_SAS.spas.PANId.Val;
                MLME_SET_macPANId_hw();
            }
            else
            {
                NwkJoinTries++;
                JoinRequest( current_SAS.spas.ExtendedPANId, ASSOCIATION_REJOIN,0 );
            }
        }
        else
        {
            NwkJoinTries++;
            /* Application should decide to which network device should join */
            AppJoinRequest( NetworkDesc, NetworkDescIndex );
            tryToRejoin = 0;
            #ifdef I_SUPPORT_SECURITY
            rejoinWithSec = 0;
            #endif
        }

        nextPrimitive = NLME_JOIN_request;
    }
    else
    {
        NwkJoinTries++;
        if( NwkJoinTries == MAXJOINRETRIES )
        {
            params.APP_START_DEVICE_conf.Status = MAXJOINRETRIES;
            params.APP_START_DEVICE_conf.ActiveChannel = 0xFF;
            params.APP_START_DEVICE_conf.PANId.Val = 0xFFFF;
            params.APP_START_DEVICE_conf.ShortAddress.Val = 0xFFFF;
            State = ZDOStateOFF;
            nextPrimitive = APP_START_DEVICE_confirm;
            NwkJoinTries = 0;
            NwkDiscoveryTries = 0;
        }
        else
        {
            NwkDiscoveryTries = 0;
            params.NLME_NETWORK_DISCOVERY_request.ScanDuration          = NWK_SCAN_DURTION;
            params.NLME_NETWORK_DISCOVERY_request.ScanChannels          = current_SAS.spas.ChannelMask;
            nextPrimitive = NLME_NETWORK_DISCOVERY_request;
        }
    }
    return nextPrimitive;
}


/***************************************************************************************
 * Function:        void JoinRequest( LONG_ADDR nwkDesExtendedPANID, BYTE joinType )
 *
 * PreCondition:    None
 *
 * Input:           nwkDesExtendedPANID : Extended PANID to which device
 *                  should try to join.
 *
 *                  joinType:Type of join.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        params structure for join request will be updated.
 *
 * Note:            None
 ***************************************************************************************/

void JoinRequest( LONG_ADDR nwkDesExtendedPANID, BYTE joinType, BYTE security )
{
    BYTE i;

    joinReqType = joinType;
    params.NLME_JOIN_request.RejoinNetwork         = joinType;
    params.NLME_JOIN_request.SecurityEnable        = security;


    for (i=0; i<8; i++)
    {
        params.NLME_JOIN_request.ExtendedPANID.v[i]         = nwkDesExtendedPANID.v[i];
    }
    params.NLME_JOIN_request.ScanChannels          = current_SAS.spas.ChannelMask;

if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
    params.NLME_JOIN_request.CapabilityInformation.CapBits.AlternatePANCoordinator = TRUE;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.DeviceType = 1;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.PowerSource = 1;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.RxOnWhenIdle = 1;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.SecurityCapability = 0;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.AllocateAddress = 1;
}else{//#else
    params.NLME_JOIN_request.CapabilityInformation.CapBits.AlternatePANCoordinator = FALSE;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.DeviceType = 0;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.PowerSource = 0;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.RxOnWhenIdle = 0;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.SecurityCapability = 0;
    params.NLME_JOIN_request.CapabilityInformation.CapBits.AllocateAddress = 0;
}//#endif

}
// #endif

/***************************************************************************************
 * Function:        ZIGBEE_PRIMITIVE InitiateStartup( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Next primitive to be executed.
 *
 * Side Effects:    None
 *
 * Overview:        Initiate startup procedure.
 *
 * Note:            None
 ***************************************************************************************/

ZIGBEE_PRIMITIVE InitiateStartup(void)
{
    if (params.APP_START_DEVICE_req.mode != STARTFROMNVM)
    {
        InitNVMWithDefaultValues();
    }

    GetNeighborTableInfo();

    if((currentNeighborTableInfo.nwkExtendedPANID.v[0]|
         currentNeighborTableInfo.nwkExtendedPANID.v[1]|
         currentNeighborTableInfo.nwkExtendedPANID.v[2]|
         currentNeighborTableInfo.nwkExtendedPANID.v[3]|
         currentNeighborTableInfo.nwkExtendedPANID.v[4]|
         currentNeighborTableInfo.nwkExtendedPANID.v[5]|
         currentNeighborTableInfo.nwkExtendedPANID.v[6]|
         currentNeighborTableInfo.nwkExtendedPANID.v[7]) )
    {
        ZigBeeInitSilentStartup();
        params.APP_START_DEVICE_conf.ActiveChannel = phyPIB.phyCurrentChannel;
        params.APP_START_DEVICE_conf.Status = SUCCESS;
        params.APP_START_DEVICE_conf.PANId = macPIB.macPANId;
        params.APP_START_DEVICE_conf.ShortAddress = macPIB.macShortAddress;
        return APP_START_DEVICE_confirm;

    }
    else
    {
    if(NOW_I_AM_A_CORDINATOR()){//#ifdef I_AM_COORDINATOR
        if (!ZigBeeStatus.flags.bits.bNetworkFormed)
        {
            params.NLME_NETWORK_FORMATION_request.ScanDuration          = NWK_SCAN_DURTION;
            params.NLME_NETWORK_FORMATION_request.ScanChannels          = current_SAS.spas.ChannelMask;
            params.NLME_NETWORK_FORMATION_request.PANId                 = current_SAS.spas.PANId;
            params.NLME_NETWORK_FORMATION_request.BeaconOrder           = MAC_PIB_macBeaconOrder;
            params.NLME_NETWORK_FORMATION_request.SuperframeOrder       = MAC_PIB_macSuperframeOrder;
            params.NLME_NETWORK_FORMATION_request.BatteryLifeExtension  = MAC_PIB_macBattLifeExt;
            
            
            return NLME_NETWORK_FORMATION_request;
        }
    }else{//#else
        if (!ZigBeeStatus.flags.bits.bNetworkJoined)
        {
            State = ZDOJoiningState;
            params.NLME_NETWORK_DISCOVERY_request.ScanDuration          = NWK_SCAN_DURTION;
            params.NLME_NETWORK_DISCOVERY_request.ScanChannels          = current_SAS.spas.ChannelMask;
            return NLME_NETWORK_DISCOVERY_request;
         }
    }//#endif
    }
    return NO_PRIMITIVE;
}


/***************************************************************************************
 * Function:        ZIGBEE_PRIMITIVE StartEndDevicePolling( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Triggers End device polling.
 *
 * Note:            None
 ***************************************************************************************/

#ifdef I_SUPPORT_AUTO_POLLING
ZIGBEE_PRIMITIVE StartEndDevicePolling( void )
{
    TICK currentTime;
    /*  Capture the current time for initiating end device polling  */
    currentTime = TickGet();
    /*  if the diff between current and the start time is greater than
        polling rate than initiate sync request */
    if( 
        (NOW_I_AM_NOT_A_RFD() && ( ZigBeeStatus.flags.bits.bNetworkJoined )) 
        //#ifdef I_AM_RFD
         || (NOW_I_AM_A_RFD() && ( ZigBeeStatus.flags.bits.bNetworkJoined ) && ( !ZigBeeStatus.flags.bits.bLostSyncWithParent ))
        //#endif
      )
    {
        if(QuickPoll == 0)
        {
    	    /* turn on radio at the polling time interval */
	        if( ( TickGetDiff( currentTime, startPollingTime ) ) > RFD_POLL_RATE )
	        {
    	        LATAbits.LATA6 =0X01;
    	        PHY_WAKE = 1;
    	        
    	        /* keep track of the radio state */
    	        ZigBeeStatus.flags.bits.bRadioIsSleeping = 0;
    	        
    	        /* set the radio's PANID, channel and short address */
    	        MACEnable();
    	        
       	        /* allow the device to get pass the enddevice annoc phase */
       	        lcount++;
	            
	            /* capture the start polling time for next cycle */
	            startPollingTime = TickGet();
	            
	            /* send out data_request command */
	            params.NLME_SYNC_request.Track = FALSE;
	            return NLME_SYNC_request;
	        }                  
	        else
	        {
    	        /* keep the radio on a little beyond turnon time to get any messages */
    	        if( (!ZigBeeStatus.flags.bits.bRadioIsSleeping) && (lcount > 2)  &&
    	            ( ( ( TickGetDiff( currentTime, startPollingTime ) ) > RFD_POLL_RATE/2 )
    	          )  && ZigBeeStatus.flags.bits.bDataRequestComplete && ZigBeeReady() )
    	        {
        	        /* keep track of the status of the radio */
                    ZigBeeStatus.flags.bits.bRadioIsSleeping = 1;
                    LATAbits.LATA6 =0X00;
                    
                    //clear the WAKE pin in order to allow the device to go to sleep
                    PHY_WAKE = 0;
                    
                    #if defined(USE_EXT_PA_LNA)
                        PHYSetLongRAMAddr(TESTMODE, 0x08);              // Disable automatic switch on PA/LNA
                        
                        #if defined(RADIO_IS_MRF24J40MC)
                            PHYSetShortRAMAddr(GPIODIR, 0x0F);        // Set GPIO direction
                            PHYSetShortRAMAddr(GPIO,    0x00);        // Enable LNA
                        #else
                            PHYSetShortRAMAddr(GPIODIR, 0x0F);
                            PHYSetShortRAMAddr(GPIO,    0x00);      // Disable PA and LNA
                        #endif            
                    #endif
   
                    // Do a power management reset to ensure device goes to sleep
                    PHYSetShortRAMAddr(SOFTRST, 0x04);
               
                    //write the registers required to place the device into sleep
                    PHYSetShortRAMAddr(TXBCNINTL, 0x80);  /* Wakecon = immediate wake up */
                    PHYSetShortRAMAddr(RXFLUSH,   0x60);  /* polarity high, wake enabled */
                    
                    ZigBeeStatus.flags.bits.bTxFIFOInUse   = 0;
                    
                    /* Put the device to sleep */             
                    PHYSetShortRAMAddr(SLPACK,    0x80);  /* placed to sleep */
  
                              
                   ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
       	        }
    	   }
	    
	    }
	    
	    /* continous polling mode */
	    else
	    {
	       if( ( TickGetDiff( currentTime, startPollingTime ) ) > (ONE_SECOND) )
	       {
    	       
    	       PHY_WAKE = 1;
    	        
    	        /* keep track of the radio state */
    	        ZigBeeStatus.flags.bits.bRadioIsSleeping = 0;
    	        
    	        /* set the radio's PANID, channel and short address */
    	        MACEnable();
                params.NLME_SYNC_request.Track = FALSE;
                /*  Again capture the start polling time for next cycle */
                startPollingTime = TickGet();
                return NLME_SYNC_request;
           }
        }
    }
    return NO_PRIMITIVE;
}
#endif

/***************************************************************************************
 * Function:        static void ZigBeeInitSilentStartup()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Restore the PIB's from NVM and set the global variables to the
 *                  running state, so that device can operate in the network
 *                  after silent start.
 *
 * Note:            None
 ***************************************************************************************/

static void ZigBeeInitSilentStartup()
{
    #ifdef I_SUPPORT_SECURITY
        BYTE KeyIndex;
    #endif

    //device is already part of network so restore from NVM
    RestorePersistencePIB();
    macPIB.macAssociationPermit = DEFAULT_ASSOC_PERMIT;
    AllowJoin = 1;
    if(NOW_I_AM_A_CORDINATOR()){//#ifdef I_AM_COORDINATOR
        ZigBeeStatus.flags.bits.bNetworkFormed = 1;
    }else{//#else
        ZigBeeStatus.flags.bits.bNetworkJoined = 1;
    }//#endif

    #ifndef I_AM_ENDDEVICE
        macStatus.bits.allowBeacon = 1;
    #endif
    if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
        ZigBeeStatus.flags.bits.bTriggerDeviceAnnce = 1;
    }//#endif
    #if I_SUPPORT_CONCENTRATOR == 1
        ZigBeeStatus.flags.bits.bTransmitManyToOneRouting  = 1;
        ManyToOneRequestTime = TickGet();
    #endif
    #ifndef I_AM_END_DEVICE
        nwkStatus.flags.bits.bCanRoute = 1;
    #endif
    #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
        nwkStatus.routingTableAgeTick = TickGet();
    #endif
    #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
        nwkStatus.nwkLinkStatusPeriod = TickGet();
        nwkStatus.moreLinkStatusCmdPending = 0;
    #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
    #ifndef I_AM_END_DEVICE
        SetBeaconPayload(TRUE);
    #endif

    GetActiveSAS();
//	memcpy(&current_SAS.spas.ExtendedPANId.v[0], &MSDCL_Commission.ExtendedPANId[0], sizeof(MSDCL_Commission.ExtendedPANId) );
//                      
//    current_SAS.spas.PreconfiguredLinkKey[0]= MSDCL_Commission.LinkKey[15];
//    current_SAS.spas.PreconfiguredLinkKey[1]= MSDCL_Commission.LinkKey[14];
//    current_SAS.spas.PreconfiguredLinkKey[2]= MSDCL_Commission.LinkKey[13];
//    current_SAS.spas.PreconfiguredLinkKey[3]= MSDCL_Commission.LinkKey[12];
//    current_SAS.spas.PreconfiguredLinkKey[4]= MSDCL_Commission.LinkKey[11];
//    current_SAS.spas.PreconfiguredLinkKey[5]= MSDCL_Commission.LinkKey[10];
//    current_SAS.spas.PreconfiguredLinkKey[6]= MSDCL_Commission.LinkKey[9];
//    current_SAS.spas.PreconfiguredLinkKey[7]= MSDCL_Commission.LinkKey[8];
//    current_SAS.spas.PreconfiguredLinkKey[8]= MSDCL_Commission.LinkKey[7];
//    current_SAS.spas.PreconfiguredLinkKey[9]= MSDCL_Commission.LinkKey[6];
//    current_SAS.spas.PreconfiguredLinkKey[10]= MSDCL_Commission.LinkKey[5];
//    current_SAS.spas.PreconfiguredLinkKey[11]= MSDCL_Commission.LinkKey[4];
//    current_SAS.spas.PreconfiguredLinkKey[12]= MSDCL_Commission.LinkKey[3];
//    current_SAS.spas.PreconfiguredLinkKey[13]= MSDCL_Commission.LinkKey[2];
//    current_SAS.spas.PreconfiguredLinkKey[14]= MSDCL_Commission.LinkKey[1];
//    current_SAS.spas.PreconfiguredLinkKey[15]= MSDCL_Commission.LinkKey[0];
    
//	    current_SAS.spas.PreconfiguredLinkKey[0]= MSDCL_Commission.LinkKey[0];
//	    current_SAS.spas.PreconfiguredLinkKey[1]= MSDCL_Commission.LinkKey[1];
//	    current_SAS.spas.PreconfiguredLinkKey[2]= MSDCL_Commission.LinkKey[2];
//	    current_SAS.spas.PreconfiguredLinkKey[3]= MSDCL_Commission.LinkKey[3];
//	    current_SAS.spas.PreconfiguredLinkKey[4]= MSDCL_Commission.LinkKey[4];
//	    current_SAS.spas.PreconfiguredLinkKey[5]= MSDCL_Commission.LinkKey[5];
//	    current_SAS.spas.PreconfiguredLinkKey[6]= MSDCL_Commission.LinkKey[6];
//	    current_SAS.spas.PreconfiguredLinkKey[7]= MSDCL_Commission.LinkKey[7];
//	    current_SAS.spas.PreconfiguredLinkKey[8]= MSDCL_Commission.LinkKey[8];
//	    current_SAS.spas.PreconfiguredLinkKey[9]= MSDCL_Commission.LinkKey[9];
//	    current_SAS.spas.PreconfiguredLinkKey[10]= MSDCL_Commission.LinkKey[10];
//	    current_SAS.spas.PreconfiguredLinkKey[11]= MSDCL_Commission.LinkKey[11];
//	    current_SAS.spas.PreconfiguredLinkKey[12]= MSDCL_Commission.LinkKey[12];
//	    current_SAS.spas.PreconfiguredLinkKey[13]= MSDCL_Commission.LinkKey[13];
//	    current_SAS.spas.PreconfiguredLinkKey[14]= MSDCL_Commission.LinkKey[14];
//	    current_SAS.spas.PreconfiguredLinkKey[15]= MSDCL_Commission.LinkKey[15];
    #ifdef I_SUPPORT_SECURITY
        GetNwkActiveKeyNumber(&KeyIndex);
        GetNwkKeyInfo(&currentNetworkKeyInfo, (networkKeyInfo+(KeyIndex - 1)*sizeof(NETWORK_KEY_INFO)));

        #ifdef USE_EXTERNAL_NVM
            SetSecurityKey(0, currentNetworkKeyInfo);
        #else
            PutNwkKeyInfo( &networkKeyInfo, &currentNetworkKeyInfo );
        #endif

        /* restore nwk security frame counter */
		GetOutgoingFrameCount(OutgoingFrameCount);
		// Increment by a safe window distance 
		OutgoingFrameCount[0].Val += MAX_PACKETS_BTWN_NVM_STORE;
		OutgoingFrameCount[1].Val += MAX_PACKETS_BTWN_NVM_STORE;
    #endif
}


#if I_SUPPORT_CONCENTRATOR == 1
static ZIGBEE_PRIMITIVE BroadcastManyToOneRequest( void )
 {
    /* put address mode in route discovery request */
    params.NLME_ROUTE_DISCOVERY_request.DstAddrMode = 0x00;

    /* copy radius */
    params.NLME_ROUTE_DISCOVERY_request.Radius = current_SAS.cpas.ConcentratorRadius;


    params.NLME_ROUTE_DISCOVERY_request.DstAddr.Val = BC_TO_ALL_ROUTERS_AND_COORDINATORS ;


    params.NLME_ROUTE_DISCOVERY_request.NoRouteCache =
    CONCENTRATOR_TYPE;

    return( NLME_ROUTE_DISCOVERY_request );
}
#endif

/***************************************************************************************
 * Function:        void SetTCLinkKeyInfo( TC_LINK_KEY_INFO TCLinkKeyInfo )
 *
 * PreCondition:    None
 *
 * Input:           TCLinkKeyInfo - Is a structure which holds the information about the TC.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to set the TC information onto NVM.
 *
 * Note:            This function is used for Silent Startup scenrios.
 ***************************************************************************************/
#if I_SUPPORT_LINK_KEY == 1
void SetTCLinkKeyInfo( TC_LINK_KEY_INFO TCLinkKeyInfo )
{
    BYTE i;
    PERSISTENCE_PIB current_pib;

    GetPersistenceStorage((void *)&current_pib);
    if(!I_AM_TRUST_CENTER){ //#ifndef I_AM_TRUST_CENTER
        current_pib.TrustCenterShortAddr = TCLinkKeyInfo.trustCenterShortAddr;
        for(i=0; i < 8; i++)
        {
             current_pib.TrustCentreLongAddr.v[i] = TCLinkKeyInfo.trustCenterLongAddr.v[i];
        }
    }//#endif /* I_AM_TRUST_CENTER */
    for(i=0; i < KEY_LENGTH; i++)
    {
        current_pib.linkKey.v[i] = TCLinkKeyInfo.link_key.v[i];
    }
    current_pib.frameCounter = TCLinkKeyInfo.frameCounter;
    PutPersistenceStorage((void *)&current_pib);
}
#endif

/***************************************************************************************
 * Function:        void GetTCLinkKeyInfo( TC_LINK_KEY_INFO *TCLinkKeyInfo )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to get the TC information from NVM.
 *
 * Note:            None
 ***************************************************************************************/
#if I_SUPPORT_LINK_KEY == 1
void GetTCLinkKeyInfo( TC_LINK_KEY_INFO *TCLinkKeyInfo )
{
    BYTE i;
    PERSISTENCE_PIB current_pib;

    GetPersistenceStorage((void *)&current_pib);
    if(!I_AM_TRUST_CENTER){ //#ifndef I_AM_TRUST_CENTER
        TCLinkKeyInfo->trustCenterShortAddr = current_pib.TrustCenterShortAddr;
        for(i=0; i < 8; i++)
        {
             TCLinkKeyInfo->trustCenterLongAddr.v[i] = current_pib.TrustCentreLongAddr.v[i];
        }
    }//#endif /* I_AM_TRUST_CENTER */
    for(i=0; i < KEY_LENGTH; i++)
    {
        TCLinkKeyInfo->link_key.v[i] = current_pib.linkKey.v[i];
    }
    TCLinkKeyInfo->frameCounter = current_pib.frameCounter;
}
#endif

/***************************************************************************************
 * Function:        void UpdateTCLinkKey( TC_LINK_KEY_INFO TCLinkKeyInfo )
 *
 * PreCondition:    None
 *
 * Input:           TCLinkKeyInfo - Is a structure which holds the information about the TC.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to update the TC information onto NVM.
 *
 * Note:            This function is used for application to change TC info, if required.
 ***************************************************************************************/
#if I_SUPPORT_LINK_KEY == 1
void UpdateTCLinkKey( TC_LINK_KEY_INFO newTCLinkKeyInfo )
{
    BYTE i;

    if(!I_AM_TRUST_CENTER){ //#ifndef I_AM_TRUST_CENTER
        TCLinkKeyInfo.trustCenterShortAddr.Val = newTCLinkKeyInfo.trustCenterShortAddr.Val;
        for(i=0; i < 8; i++)
        {
             TCLinkKeyInfo.trustCenterLongAddr.v[i] = newTCLinkKeyInfo.trustCenterLongAddr.v[i];
        }
    }//#endif /* I_AM_TRUST_CENTER */
    for(i=0; i < KEY_LENGTH; i++)
    {
        TCLinkKeyInfo.link_key.v[i] = newTCLinkKeyInfo.link_key.v[i];
    }
    TCLinkKeyInfo.frameCounter = newTCLinkKeyInfo.frameCounter;

    SetTCLinkKeyInfo(newTCLinkKeyInfo); // Update NVM too.
}
#endif

#if I_SUPPORT_FREQUENCY_AGILITY == 1

/***************************************************************************************
 * Function:        void ZDOSwitchChannel(BYTE channel)
 *
 * PreCondition:    None
 *
 * Input:           channel-The New channel to which the network to be switched
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sets the channel change flag which will change the channel after
 *                  broadcasting mgmt_nwk_update_req. This function also increments
 *                  the nwkUpdateId.
 *
 * Note:            None
 ***************************************************************************************/
void ZDOSwitchChannel(BYTE channel)
{
    /* Channel will be switched after broadcasting
       Mgmt_Nwk_Update_req. */
    nwkStatus.flags.bits.bChannelSwitch = 1;
    NewChannel = channel;
    /* Increment the nwkUpdateId */
    currentNeighborTableInfo.nwkUpdateId++;
    PutNeighborTableInfo();
    SetBeaconPayload(TRUE);
    zdoStatus.flags.bits.bSwitchChannelTimerInProgress = 1;
    zdoStatus.SwitchChannelTick = TickGet();
}


/***************************************************************************************
 * Function:        void ZDOChangeNetworkManager(WORD_VAL ZDODstAddr)
 *
 * PreCondition:    None
 *
 * Input:           ZDODstAddr-Address of new Network Manager.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Network manager will update the new network manager address and
 *                  removes network manager bit from its server mask in node descriptor.
 *
 * Note:            None
 ***************************************************************************************/
void ZDOChangeNetworkManager(WORD_VAL ZDODstAddr)
{
    if( ZDODstAddr.Val != macPIB.macShortAddress.Val)
    {
        /* Update the new network manager address in its NIB */
        currentNeighborTableInfo.nwkManagerAddr.Val = ZDODstAddr.Val;
        PutNeighborTableInfo();

        NODE_DESCRIPTOR NodeDescriptor;

        /* Removes network manager bit from server mask of node descriptor */
        ProfileGetNodeDesc(&NodeDescriptor);
        NodeDescriptor.NodeServerMask.Val &= 0xFFBF;
        ProfilePutNodeDesc(&NodeDescriptor);
    }
}

/***************************************************************************************
 * Function:        ZIGBEE_PRIMITIVE HandleMgmtNwkUpdateNotify(BOOL LoopBack, BYTE status,
                        DWORD_VAL ScannedChannels, WORD_VAL TotalTransmissions,
                        WORD_VAL FailedTransmission, BYTE ChannelListCount)
 *
 * PreCondition:    None
 *
 * Input:           LoopBack- True if network manager reported interference,
 *                            false otherwise
 *                  status- Status of Mgmt_NWK_update_Notify frame received.
 *                  ScannedChannels- Scanned channels
 *                  TotalTransmissions- Total frames transmitted by reported device.
 *                  FailedTransmission- Total number of failed transmissions on
 *                                      reported device.
 *                  ChannelListCount- No of channels scanned.
 *
 * Output:          Next foreground task to be handled.
 *
 * Side Effects:    None
 *
 * Overview:        Handles the interference reported.
 *
 * Note:            None
 ***************************************************************************************/
ZIGBEE_PRIMITIVE HandleMgmtNwkUpdateNotify(BOOL LoopBack, BYTE status,
    DWORD_VAL ScannedChannels, WORD_VAL TotalTransmissions,
    WORD_VAL FailedTransmission, BYTE ChannelListCount)
{
    ENERGY_DETECT_RECORD *EdRecordPtr = EdRecords;
    BYTE i;

    SHORT_ADDR reportedDevice;
    if (!LoopBack)
    {
        /* Reported device is a remote device, so get the address of the
           reported device from the frame received */
        reportedDevice.Val = params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val;
    }
    else
    {
        /* Reported by same device so use the reported device address as
           MAC PIB short address */
        reportedDevice.Val = macPIB.macShortAddress.Val;
    }
#if APP_RESOLVE_FREQUENCY_AGILITY == 1
    /* Send Mgmt_NWK_Update_Notify to the application, and applcation can write
       the frequency agility resolving module */

    /* Create the APP_MGMT_NWK_UPDATE_NOTIFY, which will be sent to application */
    params.APP_MGMT_NWK_UPDATE_NOTIFY.Status = status;
    params.APP_MGMT_NWK_UPDATE_NOTIFY.ShortAddr.Val = reportedDevice.Val;

    for( i = 0; i < 4; i++)
    {
        params.APP_MGMT_NWK_UPDATE_NOTIFY.ScannedChannels.v[i] = \
            ScannedChannels.v[i];
    }
    params.APP_MGMT_NWK_UPDATE_NOTIFY.ChannelListCount = ChannelListCount;
    params.APP_MGMT_NWK_UPDATE_NOTIFY.TotalTransmissions.v[0] = \
        TotalTransmissions.v[0];
    params.APP_MGMT_NWK_UPDATE_NOTIFY.TotalTransmissions.v[1] = \
        TotalTransmissions.v[1];
    params.APP_MGMT_NWK_UPDATE_NOTIFY.FailedTransmission.v[0] = \
        FailedTransmission.v[0];
    params.APP_MGMT_NWK_UPDATE_NOTIFY.FailedTransmission.v[1] = \
        FailedTransmission.v[1];
    for(i = 0; i < 16; i++)
    {
        params.APP_MGMT_NWK_UPDATE_NOTIFY.EnergyValues[i] = EdRecordPtr->EnergyReading[i];
    }
    return APP_MGMT_NWK_UPDATE_NOTIFY;
#else

    /* ZDO will resolve the frequency agility */

    if( status == SUCCESS )    // operation successful, valid response
    {
        if (EdRecordPtr->EnergyReading[ phyPIB.phyCurrentChannel - 11 ] < MAX_ENERGY_THRESHOLD)
        {
            /* If current channel energy value is below threshold energy reject the frame */
            return NO_PRIMITIVE;
        }

        for(i = 0; i < 16; i++)
        {
            /* Check whether any channel is having ED value below the threshold */
            if (EdRecordPtr->EnergyReading[i] < MAX_ENERGY_THRESHOLD)
            {
                break;
            }
        }
        if (i == 16)
        {
            /* No Channel is having ED value below threshhold, so reject the frame received. */
            return NO_PRIMITIVE;
        }

        /* If any device reports interference, then the network manager
           should start a timer, and it should wait for other devices
           to report interference. */

        /* Check the previously channel interference reported timer is running */
        if (zdoStatus.flags.bits.bWaitForUpdateNotifyTimerInProgress)
        {
            /* Check whether same device has reported interfernce once again */
            if( EdRecordPtr->ScanDeviceAddr.Val != reportedDevice.Val)
            {
                EdRecordPtr->ScanDeviceAddr.Val = reportedDevice.Val;

                zdoStatus.flags.bits.bWaitForUpdateNotifyTimerInProgress = 0;

                BYTE leastEnergy = 0xFF;
                DWORD BestChannel = 0x00000001;

                for(i = 0; i < 16; i++)
                {
                    if( EdRecordPtr->EnergyReading[i] < leastEnergy)
                    {
                        /* Find the channel with least ED value */
                        leastEnergy = EdRecordPtr->EnergyReading[i];
                        NewChannel = i + 11;
                    }
                }

                /* Create MGMT_NWK_UPDATE_req frame with requesting channel change */
                BestChannel <<= NewChannel;
                ScannedChannels.Val = BestChannel;
                ZigBeeUnblockTx();
                ZigBeeBlockTx();

                TxBuffer[TxData++] = ZDOCounter++; // seq

                TxBuffer[TxData++] = ScannedChannels.v[0];//Scan Channels
                TxBuffer[TxData++] = ScannedChannels.v[1];
                TxBuffer[TxData++] = ScannedChannels.v[2];
                TxBuffer[TxData++] = ScannedChannels.v[3];

                TxBuffer[TxData++] = 0xfe;  // scan duration

                /* Switch the channel after broadcasting the frame */
                ZDOSwitchChannel(NewChannel);

                /* Add the changed nwkUpdateId in the frame */
                TxBuffer[TxData++] = currentNeighborTableInfo.nwkUpdateId;

                params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val = 0xFFFD;

                PrepareMessageResponse(MGMT_NWK_UPDATE_req);

                return APSDE_DATA_request;

            }
        }
        else
        {
            /* First device reporting interference within a span of
               interfence wait duration */
            zdoStatus.WaitForUpdateNotifyTick = TickGet();
            zdoStatus.flags.bits.bWaitForUpdateNotifyTimerInProgress = 1;
        }

    }/* mgmt_nwk_update_Notify is success */

    return NO_PRIMITIVE;
#endif /* APP_RESOLVE_FREQUENCY_AGILITY == 1 */
}
#endif /* I_SUPPORT_FREQUENCY_AGILITY == 1 */



#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
#ifndef I_AM_END_DEVICE
/*********************************************************************
 * Function:        void SetPANId(WORD newPANId)
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        This API will be used to set the PAN ID form NTS.
 *
 *
 * Note:            None
 ********************************************************************/
void SetPANId(WORD newPANId)
{
    macPIB.macPANId.Val  =     newPANId;
    MLME_SET_macPANId_hw();
    SetBeaconPayload(TRUE);
}
#endif


void MyAskForDeviceAddress( BYTE requestSource,LONG_ADDR IeeeAddr)
{
	BYTE i;
	sendrequestSource = requestSource;
	xprintf("Iee Addr = ");
	for(i = 0; i < 8; i++)
    {
		SendIee.v[i]=IeeeAddr.v[i];
		PrintChar(SendIee.v[i]);
	}
	AskDeviceAddress = 1;
	printf("ASK For Device Now\n\r");
}
void MySendAddressRequest( BYTE requestSource,LONG_ADDR IeeeAddr)
{
    BYTE i;

	AskDeviceAddress = 0;
    // Send NWK_ADDR_req message
	printf("MySendAddressRequest1\n\r");
    ZigBeeBlockTx();

    TxBuffer[TxData++] = ZDOCounter++;

    // IEEEAddr
    if (requestSource)
    {
        for(i = 0; i < 8; i++)
        {
            TxBuffer[TxData++] = IeeeAddr.v[i];
        }
    }
    else
    {
        for(i = 0; i < 8; i++)
        {
            TxBuffer[TxData++] = IeeeAddr.v[i];
        }
    }

    // RequestType
    TxBuffer[TxData++] = 0x00;

    // StartIndex
    TxBuffer[TxData++] = 0x00;

    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
    params.APSDE_DATA_request.DstEndpoint = EP_ZDO;
    params.APSDE_DATA_request.DstAddress.ShortAddr.Val = 0xFFFF;

    params.APSDE_DATA_request.ProfileId.Val = ZDO_PROFILE_ID;
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;
    params.APSDE_DATA_request.TxOptions.Val = DEFAULT_TX_OPTIONS;
    params.APSDE_DATA_request.SrcEndpoint = EP_ZDO;
    params.APSDE_DATA_request.ClusterId.Val = NWK_ADDR_req;
}


#endif
