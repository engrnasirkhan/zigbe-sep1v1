/*********************************************************************

                    ZigBee NWK Layer

 NOTE: Route Request are always sent with Route Repair.

 *********************************************************************
 * FileName:        zNWK.c
 * Dependencies:
 * Processor:       PIC18F
 * Complier:        MCC18 v3.00 or higher'
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
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY             01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

#include "generic.h"
#include "zigbee.def"
#include "sralloc.h"
#include "ZigbeeTasks.h"
#include "zZDO.h"
#include "zNWK.h"
#include "zMAC.h"
#include "SymbolTime.h"
#include "zNVM.h"
#include "Log.h"
#include "sralloc.h"
#include <string.h>         // for memcpy-type functions
#ifdef I_SUPPORT_SECURITY
#include "zSecurity.h"
#endif

#include "zPHY.h"

#include "Console.h"
#if I_SUPPORT_STUB_APS == 1
#include "zStub_APS.h"
#endif

#include "MSDCL_Commissioning.h"

unsigned char RoLongFlage = 0;
LONG_ADDR	RoLoAddr;

extern PANIdList ListOfExtendedPANIdinVicinity[MAX_NUMBER_OF_NETWORK_DETECT];
// ******************************************************************************
// Configuration Definitions
#define NIB_STATIC_IMPLEMENTATION
#define CALCULATE_LINK_QUALITY

// Define how a 16-bit random value will be supplied.
// In this version, a snapshot of running TMR0 value is used as
// a random value.
// An application may select to provide random value from another source
#if defined(__C30__) || defined(__C32__)
    #define RANDOM_LSB                      (TMRL)
    #define RANDOM_MSB                      (TMRH)
#else
    #define RANDOM_LSB                      (TMR0L)
    #define RANDOM_MSB                      (TMR0H)
#endif

// ******************************************************************************
// Constant Definitions
#define ROUTE_RECORD_INITIATOR_LIST_COUNT   0x00
#define MAX_DATA_SIZE                   0x80  // 128
#define CONSTANT_PATH_COST              7
#define INVALID_ROUTE_DISCOVERY_INDEX   ROUTE_DISCOVERY_TABLE_SIZE
#define LAST_END_DEVICE_ADDRESS         0xFFFE
#define MAC_TX_OPTIONS_ACKNOWLEDGE      0x01   // acknowledged transmission
#define MAC_TX_OPTIONS_GTS              0x02   // GTS transmission
#define MAC_TX_OPTIONS_INDIRECT         0x04   // indirect transmission
#define MAC_TX_OPTIONS_SECURITY         0x08   // security enabled transmission
#define MAX_NWK_FRAMES                  (NUM_BUFFERED_BROADCAST_MESSAGES + \
                                         NUM_BUFFERED_ROUTING_MESSAGES + \
                                         MAX_APL_FRAMES + \
                                         NUM_BUFFERED_INDIRECT_MESSAGES * 2 + \
                                         MAX_APS_ACK_FRAMES )
#define NWK_FRAME_CMD                   0x01
#define NWK_FRAME_DATA                  0x00
#define NWK_FRAME_TYPE_MASK             0x03
#define NWK_LEAVE_INDICATION            0x00
#define NWK_LEAVE_REJOIN                0x20
#define NWK_LEAVE_REQUEST               0x40
#define NWK_LEAVE_REMOVE_CHILDREN       0x80
#define nwkProtocolVersion              0x02        //0x01
#define ROUTE_REPAIR                    0x80
#define ZIGBEE_PROTOCOL_ID              0x00        // As per spec
#define NWK_IEEE_DST_ADDR               0x08
#define NWK_IEEE_SRC_ADDR               0x10

#define NON_MEMBER_MODE_MULTICAST       0x00
#define MEMBER_MODE_MULTICAST           0x01

// Route Request Command Payload
#define DEFAULT_COMMAND_OPTIONS         0x00
#define HIGH_CONC_MANY_TO_ONE           0x08
#define LOW_CONC_MANY_TO_ONE            0x10
#define DEST_IEEE_ADDRESS_BIT           0x20
#define MULTICAST_BIT                   0x40

// Route Reply Command Payload
#define ORIGINATOR_IEEE_ADDR_BIT        0x10
#define RESPONDER_IEEE_ADDR_BIT         0x20
#define MULTICAST_BIT                   0x40

// Link Status Command Payload
#if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
    #define LINK_STATUS_COMMAND_OPTIONS_FIRST_FRAME         0x20
    #define LINK_STATUS_COMMAND_OPTIONS_INTERMEDIATE_FRAME  0x00
    #define LINK_STATUS_COMMAND_OPTIONS_LAST_FRAME          0x40
    #define LINK_STATUS_COMMAND_OPTIONS_FIRST_AND_LAST_FRAME 0x60
#endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

#if defined(I_SUPPORT_GROUP_ADDRESSING)
    #define MULTICAST                       0x40  // ZigBee 2006:  Needed for group routing requests
#endif

#if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
    #define ROUTING_TABLE_AGE_RESET     0x00
#endif

// This value marks whether or not the neighbor table contains valid
// values left over from before a loss of power.
#define NEIGHBOR_TABLE_VALID    0xA53C

// NIB Default values not specified by the Profile
#define DEFAULT_nwkPassiveAckTimeout                0x01F4  /* :dec500 changed to fix the BC retries issue */

#define DEFAULT_nwkMaxBroadcastRetries              0x02
#if defined(I_AM_END_DEVICE)
    #define DEFAULT_nwkNeighborTable                NULL
#else
    #define DEFAULT_nwkNeighborTable                neighborTable
#endif
#define DEFAULT_nwkNetworkBroadcastDeliveryTime     (1 *(DEFAULT_nwkMaxBroadcastRetries+1))
#define DEFAULT_nwkReportConstantCost               0
#define DEFAULT_nwkRouteDiscoveryRetriesPermitted   0x03
#if defined (I_SUPPORT_ROUTING)
    #define DEFAULT_nwkRouteTable                   routingTable
#else
    #define DEFAULT_nwkRouteTable                   NULL
#endif
#define DEFAULT_nwkSymLink                          TRUE
#define DEFAULT_nwkCapabilityInformation            0
#define DEFAULT_nwkUseTreeAddrAlloc                 TRUE
#define DEFAULT_nwkUseTreeRouting                   TRUE
#define DEFAULT_nwkAllFresh             1

/* Beacon payload parameters offsets */
#define    EXTENDED_PANID_OFFSET    3
#define    TX_OPTIONS_OFFSET        11
#define    NWK_UPDATE_ID_OFFSET     14
#define    EXTENDED_PAN_SIZE        8
#define MAX_STATUS_COUNT            4

#define ONE_MILLISECOND                     ONE_SECOND/1000     //Intepretation for one millisecond

#define BROADCAST_JITTER              ( nwkcMaxBroadcastJitter/2 + (MAC_LONG_ADDR_BYTE0 & 0x0F)) * ONE_SECOND / 100
#if BROADCAST_JITTER == 0
    #define RREQ_BROADCAST_JITTER   (2 * nwkcMinRREQJitter * (DWORD)ONE_SECOND / 1000)
#else
    #define RREQ_BROADCAST_JITTER   (2 * BROADCAST_JITTER)
#endif
#define NOT_VALID 0xFF
#define MAX_REJOIN_RETRIES 3
#define INTRP_FRAME_DATA                          0x03

SHORT_ADDR ParentAddress;
// ******************************************************************************
// Enumerations

TICK ASSOCIATE_indication_Time;

typedef enum _IEEE_DEVICE_TYPE_VALUES
{
    DEVICE_IEEE_RFD                 = 0x00,
    DEVICE_IEEE_FFD                 = 0x01
} IEEE_DEVICE_TYPE_VALUES;

typedef enum _NWK_COMMANDS
{
    NWK_COMMAND_ROUTE_REQUEST       = 0x01,
    NWK_COMMAND_ROUTE_REPLY         = 0x02,
    NWK_COMMAND_NWK_STATUS          = 0x03,
    NWK_COMMAND_LEAVE               = 0x04,
    NWK_COMMAND_ROUTE_RECORD        = 0x05,
    NWK_COMMAND_REJOIN_REQUEST      = 0x06,
    NWK_COMMAND_REJOIN_RESPONSE     = 0x07,
    NWK_COMMAND_LINK_STATUS         = 0x08,
    NWK_COMMAND_NWK_REPORT          = 0x09,
    NWK_COMMAND_NWK_UPDATE          = 0x0A
} NWK_COMMANDS;

typedef enum _MAC_ASSOCIATION_STATUS_VALUES
{
    ASSOCIATION_SUCCESS             = 0x00,
    ASSOCIATION_PAN_AT_CAPACITY     = 0x01,
    ASSOCIATION_PAN_ACCESS_DENIED   = 0x02
} MAC_ASSOCIATION_STATUS_VALUES;

typedef enum _MESSAGE_ROUTING_STATUS
{
    ROUTE_SEND_TO_MAC_ADDRESS,
    ROUTE_MESSAGE_BUFFERED,
    ROUTE_FAILURE_TREE_LINK,
    ROUTE_FAILURE_NONTREE_LINK,
    ROUTE_FAILURE_NO_CAPACITY,
    ROUTE_AVAILABLE_TO_CONCENTRATOR,
    ROUTE_AS_PER_SOURCE_ROUTE_FRAME,
    ROUTE_DATA_AFTER_ROUTE_RECORD,
    ROUTE_DISCOVERY_ALREADY_INITIATED
} MESSAGE_ROUTING_STATUS;

// ******************************************************************************
// Data Structures

typedef union _LEAVE_COMMAND_OPTIONS
{
    BYTE        Val;
    struct _LEAVE_BITS
    {
        BYTE    : 6;
        BYTE    bIsRequest      : 1;
        BYTE    bRemoveChildren : 1;
    } bits;
} LEAVE_COMMAND_OPTIONS;

typedef struct _BACKUP_DATA_FOR_CONCENTRATOR
{
    BYTE routeRecordSend;
    BYTE msduHandle;
    SHORT_ADDR macDestination;
    BYTE nsduHandle;
    BYTE dataLength;
    BYTE *DataBackUp;
}BACKUP_DATA_FOR_CONCENTRATOR;
typedef struct _NIB
{
    BYTE nwkBCSN;

    #ifndef NIB_STATIC_IMPLEMENTATION
        BYTE nwkPassiveAckTimeout;
        BYTE nwkMaxBroadcastRetries;
        BYTE nwkMaxChildren;
        BYTE nwkMaxDepth;
        BYTE nwkMaxRouters;
        #ifdef USE_EXTERNAL_NVM
            WORD nwkNeighborTable;
        #else
            ROM NEIGHBOR_RECORD* nwkNeighborTable;
        #endif
        BYTE nwkNetworkBroadcastDeliveryTime;
        BYTE nwkReportConstantCost;
        BYTE nwkRouteDiscoveryRetriesPermitted;
        #ifdef USE_EXTERNAL_NVM
            WORD nwkRouteTable;
        #else
            ROM ROUTING_ENTRY* nwkRouteTable;
        #endif
        BYTE nwkSymLink;
        BYTE nwkCapabilityInformation;
        BOOL nwkUseTreeAddrAlloc;
        BOOL nwkUseTreeRouting;
    #endif
} NIB_TABLE;


typedef struct _NWK_FRAMES
{
    BYTE        msduHandle;
    BYTE        nsduHandle;
    BYTE        nwkFrameId;
} NWK_FRAMES;



typedef struct _ROUTE_DISCOVERY_ENTRY
{
    TICK            timeStamp;
    TICK            rebroadcastTimer;
    SHORT_ADDR      srcAddress;
    SHORT_ADDR      senderAddress;
    BYTE            *forwardRREQ;
    BYTE            routeRequestID;
    BYTE            forwardCost;
    BYTE            residualCost;
    BYTE            previousCost;
    BYTE            routingTableIndex;      // Added so we can find the routing entry during clean-up.
    struct          _status
    {
        BYTE        transmitCounter     : 3;
        BYTE        initialRebroadcast  : 1;
        BYTE        initiator           : 1;
        BYTE        previousStateACTIVE : 1;
    }               status;
} ROUTE_DISCOVERY_ENTRY;


typedef struct _ROUTE_REPLY_COMMAND
{
    BYTE            commandFrameIdentifier;
    BYTE            commandOptions;
    BYTE            routeRequestIdentifier;
    SHORT_ADDR      originatorAddress;
    SHORT_ADDR      responderAddress;
    BYTE            pathCost;
    #if ( ZIGBEE_PRO == 0x01)
        LONG_ADDR       originatorIEEEAddress;
        LONG_ADDR       responderIEEEAddress;
    #endif
} ROUTE_REPLY_COMMAND;


typedef struct _ROUTE_REQUEST_COMMAND
{
    BYTE            commandFrameIdentifier;
    BYTE            commandOptions;
    BYTE            routeRequestIdentifier;
    SHORT_ADDR      destinationAddress;
    BYTE            pathCost;
    #if ( ZIGBEE_PRO == 0x01)
        LONG_ADDR       destinationIEEEAddress;
    #endif
} ROUTE_REQUEST_COMMAND;
#define sizeof_ROUTE_REQUEST_COMMAND 6
#if ( ZIGBEE_PRO == 0x01)
    #define sizeof_RREQ_COMMAND_DST_IEEE_ADDR 8
#endif
#define sizeof_ROUTE_REQUEST_COMMAND_HEADER 16 //8 bytes header + 8 bytes SrcIEEEAddress

/* ZigBee 2006 requirement:  Support the REJOIN procedure */
typedef struct _REJOIN_INDICATION
{
    ASSOCIATE_CAPABILITY_INFO   CapabilityInformation;
    BOOL                        secureJoin;
    BYTE                        filler1[6];
    SHORT_ADDR                  ShortAddress;
    BYTE                        filler2[6];
    LONG_ADDR                   ExtendedAddress;
} REJOIN_INDICATION;
REJOIN_INDICATION rejoinIndication;
BYTE rejoinIndicationPending;
BYTE msduHandle;

// ******************************************************************************
// Variable Definitions
#if I_SUPPORT_CONCENTRATOR ==1
    ROUTE_RECORD RouteRecordTable[MAX_ROUTE_RECORD_TABLE_SIZE];
    static BYTE NewEntry = 0x00;
#endif

NWK_FRAMES              nwkConfirmationHandles[MAX_NWK_FRAMES];

#ifdef NIB_STATIC_IMPLEMENTATION
    NIB_TABLE           NIB;                                  // nwkBCSN initialized in VariableAndTableInitialization

    #define NIB_nwkPassiveAckTimeout                DEFAULT_nwkPassiveAckTimeout
    #define NIB_nwkMaxBroadcastRetries              DEFAULT_nwkMaxBroadcastRetries
    #define NIB_nwkMaxChildren                      PROFILE_nwkMaxChildren
    #define NIB_nwkMaxDepth                         PROFILE_nwkMaxDepth
    #define NIB_nwkMaxRouters                       PROFILE_nwkMaxRouters
    #define NIB_nwkNeighborTable                    DEFAULT_nwkNeighborTable
    #define NIB_nwkNetworkBroadcastDeliveryTime     DEFAULT_nwkNetworkBroadcastDeliveryTime
    #define NIB_nwkReportConstantCost               PROFILE_nwkReportConstantCost
    #define NIB_nwkRouteDiscoveryRetriesPermitted   DEFAULT_nwkRouteDiscoveryRetriesPermitted
    #define NIB_nwkRouteTable                       DEFAULT_nwkRouteTable
    #define NIB_nwkSymLink                          PROFILE_nwkSymLink
    #define NIB_nwkCapabilityInformation            DEFAULT_nwkCapabilityInformation
    #define NIB_nwkUseTreeAddrAlloc                 DEFAULT_nwkUseTreeAddrAlloc
    #define NIB_nwkUseTreeRouting                   DEFAULT_nwkUseTreeRouting

    #if NIB_nwkMaxBroadcastRetries > 15
        #error nwkMaxBroadcastRetries must not be greater than 15.
    #endif
#else
    NIB_TABLE           NIB;                                // Values initialized in VariableAndTableInitialization
#endif

#if defined(I_SUPPORT_SECURITY)
    BYTE nwkSecurityLevel;
    extern SECURITY_STATUS securityStatus;
#endif

#if I_SUPPORT_FREQUENCY_AGILITY == 1

    /* Variable to store the ED received. */
    ENERGY_DETECT_RECORD EdRecords[1];

    /* Variable to store the number of packets transmitted. */
    WORD_VAL TotalTransmittedPackets;

    /* Variable to store the total failed transmission due to interference. */
    WORD_VAL TotalTransmitFailures;

    /* Indicates more than 25% of data transmissions failed. */
    BYTE InterferenceDetected;

    /* Total number of Mgmt_NWK_Update_Notify sent within a span of 1 hour. */
    BYTE UpdateNotifyCount = 0;
#endif

/* Saves frame control of packets to be forwarded via routing */
typedef struct _FWDMSGFRAMECTRL
{
	BYTE frameCtrlLSB;
	BYTE frameCtrlMSB;
} frameCtrl;

frameCtrl messageToForward;

NWK_STATUS              nwkStatus;

#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
    PAN_ID_CONFLICT_STATUS  panIDConflictStatus;
    BYTE    activatePANIDConflict = 0x01;       // This can be used by test profile to deactivate PANIDConflict detection and resolution on particular node.
#endif

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
    BYTE    activateAddConfDetection = 0x01;       // This can be used by test profile to deactivate Address Conflict detection on particular node.
#endif

ROUTE_DST_INFO          routeDstInfo[ROUTE_DST_INFO_SIZE];

#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
ROUTE_DISCOVERY_ENTRY   *routeDiscoveryTablePointer[ROUTE_DISCOVERY_TABLE_SIZE];
#endif

BYTE rejoin_network;
BYTE waitForKey = 0;

#if defined(I_SUPPORT_SECURITY)
    #if !defined(__C30__) && !defined(__C32__)
        #pragma udata SecurityCounterLocation=SECURITY_COUNTER_LOCATION
    #endif
    extern DWORD_VAL IncomingFrameCount[2][MAX_NEIGHBORS];
    extern DWORD_VAL   OutgoingFrameCount[2];
    #if !defined(__C30__) && !defined(__C32__)
        #pragma udata
    #endif
    extern volatile PHY_PENDING_TASKS  PHYTasksPending;
    extern TICK     AuthorizationTimeout;
    extern TICK        WaitForAuthentication;
    extern SECURITY_STATUS  securityStatus;
    //#ifdef I_AM_RFD
        extern TICK lastPollTime;
    //#endif
    #if defined(USE_EXTERNAL_NVM)
        extern NETWORK_KEY_INFO plainSecurityKey[2];
        extern BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey);
        extern BOOL InitSecurityKey(void);
    #endif
#endif
#ifdef ZCP_DEBUG
    BYTE                accessDenied = 0;
//    #ifndef I_AM_COORDINATOR
        extern BYTE AllowJoin;
//    #endif

    extern BOOL bDisableShortAddress;
#endif

#ifdef DEBUG_LOG
    extern BYTE NoOfPANDescriptorFound;
    extern BYTE PotentialParentFound;
    extern BYTE overWriteCheck;
#endif

extern volatile MAC_TASKS_PENDING    macTasksPending;

extern LONG_ADDR tempLongAddress;
extern ZDO_STATUS zdoStatus;
#if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
    /* structure holding the data to concentrator which should be send after
    sending route record */
    BACKUP_DATA_FOR_CONCENTRATOR DataToBeSendAfterRouteRecord;
#endif  /*  I_SUPPORT_MANY_TO_ONE_HANDLING */

#if ( I_SUPPORT_SOURCE_ROUTE_TESTING == 0x01 )
    extern BYTE SourceRouteTestingInProgress;
#endif // ( I_SUPPORT_SOURCE_ROUTE_TESTING == 0x01 )

#if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
    WORD        SuppressLinkStatusAddrList[5];
    BYTE        SuppressLinkStatusAddrListSize;
#endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
BYTE rejoin_retries = 0;

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
    // Used to send hold to address for which Unsolicited Rejoin Response should be sent
    SHORT_ADDR addressConflictOnChildEndDevice;
#endif
extern BOOL appNotification;
extern APP_NOTIFICATION        appNotify;
#ifdef I_SUPPORT_SECURITY
BYTE rejoinWithSec;
#endif
extern BYTE storeMsduHandle;
extern BYTE interPanDataTransmitted;

TICK timeBetweenScanDuringRejoin;

#ifdef I_SUPPORT_SECURITY
//This variable is used for storing the outgoing frame counter into NVM
DWORD_VAL frame_counter[2];
#endif

// ******************************************************************************
// Function Prototypes
#if I_SUPPORT_CONCENTRATOR ==1
    void StoreRouteRecord( SHORT_ADDR nwkDestinationAddress );
    BYTE    GetIndexToAddInRouteRecordTable( SHORT_ADDR destination );
    BYTE *CreateSourceRouteSubframe( BYTE Index, WORD *Hops, SHORT_ADDR *macDestinationAddress );
#endif
void LoadBackUpDataToTxBuffer( BYTE datalength, BYTE *source);
void CreateNwkHeaderForDataAndStoreData( BYTE discoverRoute, BYTE *ptr );
#if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
    static void CheckAndUpdateRouteTableWithConcentratorDetails(ROUTE_REQUEST_COMMAND   *rreq, \
        ROUTE_DISCOVERY_ENTRY   *prouteDiscoveryTable );
    static void CreateRouteRecordFrame(  SHORT_ADDR macDest );
    BYTE CreateRelyingRouteRecord( LONG_ADDR nwkIEEESrcAddress, LONG_ADDR nwkIEEEDstAddress,
                                   BYTE nwkSequenceNumber, BYTE nwkRadius,
                                   SHORT_ADDR  nwkSourceAddress, SHORT_ADDR nwkDestinationAddress,
                                   NWK_FRAME_CONTROL_MSB nwkFrameControlMSB, NWK_FRAME_CONTROL_LSB nwkFrameControlLSB,
                                   BYTE relayCount, BYTE *relayList );
#endif           /*  I_SUPPORT_MANY_TO_ONE_HANDLING*/
static BYTE LoadNwkDataHeader( SHORT_ADDR macAddress, BYTE sourceRouteLength, BYTE *sourceRouteSubframe );
NEIGHBOR_KEY CanAddNeighborNode( void );
void VariableAndTableInitialization( BOOL force );

static void UpdateNwkHeaderFrameContolFields(BYTE includeDstIEEEAddr);
static BYTE AddNwkHeaderIEEEAddresses (SHORT_ADDR shortAddr);
#if !defined (I_AM_END_DEVICE)
    static void CreateNwkCommandNwkStatus(BYTE Status, SHORT_ADDR FailedRoutingAddr);
#endif
static void CreateNwkCommandHeader(BYTE SequenceNumber, BYTE Radius, SHORT_ADDR nwkSrcAddr,
                                SHORT_ADDR nwkDstAddr);

NEIGHBOR_KEY NWKLookupNodeByShortAddrVal( WORD shortAddrVal );
NEIGHBOR_KEY NWKLookupNodeByRxOffDeviceShortAddrVal( WORD shortAddrVal );
NEIGHBOR_KEY NWKLookupNodeByShortAddrValAndPANID( WORD shortAddrVal, WORD PANIDVal );
NEIGHBOR_KEY NWKLookupNodeByLongAddr(LONG_ADDR *longAddr);
BYTE NWKGet( void );
void Prepare_MCPS_DATA_request( WORD macDestAddressVal, BYTE *msduHandle );
void RemoveNeighborTableEntry( void );

//#ifdef I_AM_COORDINATOR
    BOOL RequestedPANIDFound( BYTE channel );
//#endif

#ifndef I_AM_END_DEVICE
    #ifdef I_SUPPORT_SECURITY
        void AddChildNode(BOOL bSecured);
    #else
        void AddChildNode( void );
    #endif
    BOOL CanAddChildNode( void );
    void SetBeaconPayload( BOOL );
#endif
void RemoveNeighborTableEntry( void );
//#ifndef I_AM_COORDINATOR
    NEIGHBOR_KEY NWKLookupNodeByExtendedPANId( LONG_ADDR *ExtPANId );
//#endif
//#ifndef I_AM_RFD
BOOL CreateNewBTR( BYTE *BTTIndex );
WORD GetCSkipVal( BYTE depth );
MESSAGE_ROUTING_STATUS GetRoutingAddress( BOOL fromUpperLayers, SHORT_ADDR nwkAddress, BYTE discoverRoute, SHORT_ADDR *macAddress );
void MarkNeighborAsPasssiveACKed( BYTE BTTindex );
//#endif

BYTE CalculateLinkQuality( BYTE lqi );
#if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
static void AgeOutNeighborTableEntries( void );
static void CreateLinkStatusCommandFrame( void );
#endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

#ifdef I_SUPPORT_ROUTING
    void CreateRouteReply( SHORT_ADDR originatorAddress, BYTE rdIndex, ROUTE_REQUEST_COMMAND *rreq );
    #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
        BOOL IsDescendant( SHORT_ADDR parentAddr, SHORT_ADDR childAddr, BYTE parentDepth );
        BOOL RouteAlongTree( SHORT_ADDR destTarget, SHORT_ADDR *destNextHop );
    #endif
    BOOL FindNextHopAddress( SHORT_ADDR destAddr, SHORT_ADDR *nextHopAddr);
    BOOL GetNextHop( SHORT_ADDR destAddr, SHORT_ADDR *nextHop, BYTE *routeStatus );
    #if !defined(USE_TREE_ROUTING_ONLY)
//NOTE: Check the mismatch?
//        BYTE FindFreeRoutingTableEntry( SHORT_ADDR dstAddr, BOOL isConcentrator );
        BYTE FindFreeRoutingTableEntry( SHORT_ADDR dstAddr, BYTE isConcentrator );
        #if ((I_SUPPORT_SYMMETRIC_LINK == 0x01) || (  I_SUPPORT_MANY_TO_ONE_HANDLING == 1))
            BOOL CreateRoutingTableEntries( SHORT_ADDR targetAddr, BYTE *rdIndex, BYTE *rtIndex,
                                SHORT_ADDR initiatorAddr, SHORT_ADDR nextHopAddr ,BOOL isConcentrator);
        #else
            BOOL CreateRoutingTableEntries( SHORT_ADDR targetAddr, BYTE *rdIndex, BYTE *rtIndex );
        #endif
        BOOL GetNextHop( SHORT_ADDR destAddr, SHORT_ADDR *nextHop, BYTE *routeStatus );
        BOOL HaveRoutingCapacity( BOOL validID, BYTE routeRequestID, SHORT_ADDR routeSrcAddress, SHORT_ADDR routeDestAddress, BYTE commandOptions );
        MESSAGE_ROUTING_STATUS InitiateRouteDiscovery( BOOL fromUpperLayers, BYTE discoverRoute );
        void RemoveRoutingTableEntry( SHORT_ADDR dstAddr );
        BYTE ClearRoutingTableEntry( BYTE rtIndex );
    #endif
    #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
        void AgeOutRoutingTableEntries( void );
    #endif

    NEIGHBOR_KEY NWKFindRandomNeighbor( BYTE NBTStartIndex, SHORT_ADDR shortAddressToBeIgnored1, SHORT_ADDR shortAddressToBeIgnored2 );

    #if !defined(I_AM_END_DEVICE)
        BOOL doingRejoinCommand = FALSE;
    #endif

    BYTE GetDestFromRoutingTable( SHORT_ADDR destAddr, SHORT_ADDR *nextHop, BYTE *routeStatus );
    BYTE GetMulticastRoutingEntry( SHORT_ADDR groupAddr, SHORT_ADDR *nextHop, BYTE *routeStatus );
#endif

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
    #ifndef I_AM_END_DEVICE
        WORD GenerateNewShortAddr(void);
        ZIGBEE_PRIMITIVE PrepareUnsolicitedRejoinRsp( SHORT_ADDR shortAddress );
#define IS_IT_A_ADDR_CONFLICT_ON_CHILD_END_DEV( shortAddress ) ( ( ( NWKLookupNodeByShortAddrVal( shortAddress.Val ) != INVALID_NEIGHBOR_KEY ) &&   \
                                                                   ( currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE ) \
                                                                  )                                                                                 \
                                                                )
    #endif
#endif //I_SUPPORT_STOCHASTIC_ADDRESSING

#define RANDOM_16BIT (TMR2)

#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
#ifndef I_AM_END_DEVICE
    static BOOL IsTherePANIdConflict(void);
    ZIGBEE_PRIMITIVE PrepareNetworkReportCmdFrame(void);
    void PrepareNetworkUpdateCmdFrame(void);
#endif
#endif // I_SUPPORT_PANID_CONFLICT
#ifdef I_SUPPORT_SECURITY
    void clearNWKKEy(void);
#endif
BOOL updatepayload = TRUE;
SHORT_ADDR rejoinShortAddress;

#if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
void SetMulticastParams (BYTE useMulticast, BYTE nonMemberRadius, BYTE maxNonMemberRadius);
extern BYTE    GetEndPointsFromGroup(INPUT SHORT_ADDR GroupAddr);
#endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
extern ZIGBEE_PRIMITIVE ResolveLocalAddressConflict( void );
#endif

unsigned char panIdListCounter = 0;

extern BOOL CheckDeviceJoiningPermission(LONG_ADDR ieeeAddr);

/*********************************************************************
 * Function:        BOOL NWKHasBackgroundTasks( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - NWK layer has background tasks to run
 *                  FALSE - NWK layer does not have background tasks
 *
 * Side Effects:    None
 *
 * Overview:        Determines if the NWK layer has background tasks
 *                  that need to be run.
 *
 * Note:            None
 ********************************************************************/

BOOL NWKHasBackgroundTasks( void )
{
    return ((nwkStatus.flags.Val & NWK_BACKGROUND_TASKS) != 0);
}


/*********************************************************************
 * Function:        void NWKInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    NWK layer data structures are initialized.
 *
 * Overview:        This routine initializes all NWK layer data
 *                  structures.
 *
 * Note:            This routine is intended to be called as part of
 *                  a network or power-up initialization.  If called
 *                  after the network has been running, heap space
 *                  may be lost unless the heap is also reinitialized.
 *                  Routes will also be destroyed.
 ********************************************************************/

void NWKInit( void )
{
    BYTE    i;
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
    BYTE rdIndex;
#endif
#if I_SUPPORT_FREQUENCY_AGILITY == 1

    TotalTransmittedPackets.Val = 0x0000;
    TotalTransmitFailures.Val = 0x0000;
    UpdateNotifyCount = 0;
    nwkStatus.flags.bits.bScanRequestFromZDO = 0;
#endif


/* The following code is added to clean up if  any on going broadcast or routing
   durinng reset.  added to fix buffer issue Bug id:9028*/


    BYTE BTTIndex;
  if (NOW_I_AM_NOT_A_RFD()){//#ifndef I_AM_RFD
    for (BTTIndex=0; BTTIndex<NUM_BUFFERED_BROADCAST_MESSAGES; BTTIndex++)
    {
        if (nwkStatus.BTT[BTTIndex] != NULL)
        {
            SRAMfree( nwkStatus.BTT[BTTIndex]->dataPtr );
            nfree( nwkStatus.BTT[BTTIndex] );
            nwkStatus.BTT[BTTIndex] = NULL;
        }
    }
  }//  #endif      /* not RFD */

     #if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
       for (rdIndex = 0; rdIndex < ROUTE_DISCOVERY_TABLE_SIZE; rdIndex ++ )
       {
           if (routeDiscoveryTablePointer[rdIndex] != NULL)

           {

                if (routeDiscoveryTablePointer[rdIndex]->forwardRREQ)
                {
                   SRAMfree( routeDiscoveryTablePointer[rdIndex]->forwardRREQ );
                }
                nfree( routeDiscoveryTablePointer[rdIndex] );
           }
       }
     #endif

    VariableAndTableInitialization( TRUE );

    // Initialize the neighbor table.
    // Read the record into RAM.
    GetNeighborTableInfo();

    if ( currentNeighborTableInfo.validityKey != NEIGHBOR_TABLE_VALID )
    {
        // The neighbor table is invalid, so clear our RAM and ROM table.
        NWKClearNeighborTable();
    }
    else
    {
        if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
            // If there is a valid old parent address in device's neighbor table, then
            // try to join as an orphan.
            if (currentNeighborTableInfo.parentNeighborTableIndex != INVALID_NEIGHBOR_KEY)
            {
                ZigBeeStatus.flags.bits.bTryOrphanJoin = 1;
            }
        }//#endif
        // Child address information may not be valid - device might not
        // have been able to join a netork last time.  Therefore leave
        // the value that was read from the neighbor table info.
    }

    for(i = 0; i < ROUTE_DST_INFO_SIZE; i++)
    {
        routeDstInfo[i].counter = 0;
    }

 #ifdef I_SUPPORT_ROUTING
    for(i = 0; i < NUM_BUFFERED_ROUTING_MESSAGES; i++)
    {
        nwkStatus.routeHandleRecords[i].SourceAddress.Val = 0xFFFF;
    }
 #endif
  if(NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
    nwkStatus.rejoinCommandSent = 0;
   }//#endif
    rejoin_retries = 0;
#if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
    /* clear the RAM based Neighbor Table "age", this is the ONLY parameter used in the RAM */
    for ( i = 0; i < MAX_NEIGHBORS; i++)
    {
        RAMNeighborTable[i].linkStatusInfo.Age = 0;
        RAMNeighborTable[i].shortAddr.Val = 0xFFFF;
        RAMNeighborTable[i].linkStatusInfo.flags.Val = 0;
    }
#endif /*( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )*/
}


/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE NWKTasks(ZIGBEE_PRIMITIVE inputPrimitive)
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

ZIGBEE_PRIMITIVE NWKTasks(ZIGBEE_PRIMITIVE inputPrimitive)
{
    BYTE        i;

    #if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
        BYTE        j;
    #endif

    BYTE        *ptr;
    SHORT_ADDR  macAddress;
    TICK        tempTick = TickGet();

    // *********************************************************************
    // Manage primitive- and Tx-independent tasks here.  These tasks CANNOT
    // produce a primitive or send a message.

    // ---------------------------------------------------------------------
    // Handle join permit time-out
    if ((NOW_I_AM_A_CORDINATOR())|| (NOW_I_AM_A_ROUTER())){//#if defined( I_AM_COORDINATOR ) || defined( I_AM_ROUTER )
        if (nwkStatus.flags.bits.bTimingJoinPermitDuration)
        {
            // NOTE: Compiler SSR27744, TickGet() output must be assigned to a variable.

            if (TickGetDiff( tempTick, nwkStatus.joinDurationStart) > ONE_SECOND*nwkStatus.joinPermitDuration)
            {
                macPIB.macAssociationPermit = FALSE;
                nwkStatus.flags.bits.bTimingJoinPermitDuration = 0;
            }
        }
    }//#endif

    // *********************************************************************
    // Handle other tasks and primitives that may require a message to be sent.

    if (inputPrimitive == NO_PRIMITIVE)
    {
        // If Tx is blocked, we cannot generate a message or send back another primitive.
        if (!ZigBeeReady())
        {
            return NO_PRIMITIVE;
        }
        
        //*********************************************************************
        // Store Security frame counter values to NVM
        //---------------------------------------------------------------------
        #ifdef I_SUPPORT_SECURITY
        
            if( ((NOW_I_AM_A_CORDINATOR()) //#ifdef I_AM_COORDINATOR
                && ( ZigBeeStatus.flags.bits.bNetworkFormed ))
            ||//#else
                 ((NOW_I_AM_NOT_A_CORDINATOR())&&( ( ZigBeeStatus.flags.bits.bNetworkJoined ) && 
                     ( !securityStatus.flags.bits.bAuthorization ) ))
            )//#endif
            
		    {
                static TICK NVM_Last_Stored_Time ;     
                TICK currentTime = TickGet();
                
                frame_counter[0].Val = 0;
                frame_counter[1].Val = 0;
                GetOutgoingFrameCount(frame_counter);
                
                BYTE ActiveKeyIndex;
                GetNwkActiveKeyNumber(&ActiveKeyIndex);  
                               
                if( ( TickGetDiff( currentTime, NVM_Last_Stored_Time)) > NVM_STORE_INTERVAL ) 
                {
                    NVM_Last_Stored_Time = TickGet(); 
                    frame_counter[ActiveKeyIndex-1].Val = OutgoingFrameCount[ActiveKeyIndex-1].Val;
                    StorePersistancePIB(); //Store the OutgoingFrameCounter,
                }
                else if( ( OutgoingFrameCount[ActiveKeyIndex-1].Val != frame_counter[ActiveKeyIndex-1].Val ) && 
                         ( ( ( OutgoingFrameCount[ActiveKeyIndex-1].Val > frame_counter[ActiveKeyIndex-1].Val )?
                               ( OutgoingFrameCount[ActiveKeyIndex-1].Val - frame_counter[ActiveKeyIndex-1].Val ):
                               ( 0xFFFFFFFF - frame_counter[ActiveKeyIndex-1].Val + OutgoingFrameCount[ActiveKeyIndex-1].Val ) 
                           ) > MAX_PACKETS_BTWN_NVM_STORE  ) )
                {      
                    NVM_Last_Stored_Time = TickGet();
                    frame_counter[ActiveKeyIndex-1].Val = OutgoingFrameCount[ActiveKeyIndex-1].Val;
                    StorePersistancePIB(); //Store the OutgoingFrameCounter
            
                
                
                               
            }
            }
        #endif
        //---------------------------------------------------------------------
        
#if I_SUPPORT_FREQUENCY_AGILITY == 1
        /* Don't process any request when PHY is busy */
        if ( PHYHasBackgroundTasks() )
        {
            return NO_PRIMITIVE;
        }
        /* Verify the TotalTransmittedPackets is more than 20. */
        if (TotalTransmittedPackets.Val > 1000 && !InterferenceDetected)
        {
            /* Verify the failure rate is more than 25%. */
            if ((TotalTransmittedPackets.Val / 2) < TotalTransmitFailures.Val)
            {
                /* Check whether already the device reported 4 times within a span of 1 hour. */
                if (UpdateNotifyCount >= 4)
                {
					/* For  this version do nothing expect let other background tasks run */
                }
                else
                {
                    /* Back up the channel and issue ED scan on all channels. */
                    phyPIB.phyBackupChannel = phyPIB.phyCurrentChannel;
                    nwkStatus.flags.bits.bScanRequestFromZDO = 1;

                    params.MLME_SCAN_request.ScanType = MAC_SCAN_ENERGY_DETECT;
                    params.MLME_SCAN_request.ScanChannels.Val = 0x07FFF800;
                    params.MLME_SCAN_request.ScanDuration = NWK_SCAN_DURTION;
                    InterferenceDetected = 1;
                    return MLME_SCAN_request;
                }
            }
        }
#endif

        // Manage background tasks here

        // ---------------------------------------------------------------------
        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
        #ifndef I_AM_END_DEVICE
            if ( nwkStatus.flags.bits.bNextChildAddressUsed == TRUE )
            {
                currentNeighborTableInfo.nextChildAddr.Val = GenerateNewShortAddr();
                PutNeighborTableInfo();
                nwkStatus.flags.bits.bNextChildAddressUsed = FALSE;
            }
        #endif
        #endif
        
        // ---------------------------------------------------------------------
        /*This block is to provide delay between the scans during rejoing procedure*/
        if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
        if (
            ( ZigBeeStatus.flags.bits.bActiveScanToRejoin ) &&
            ( ZigBeeStatus.flags.bits.bNetworkJoined ) && 
            ( nwkStatus.flags.bits.bRejoinScan ) &&
            ( TickGetDiff(tempTick, timeBetweenScanDuringRejoin) > (ONE_SECOND*2) ) /*2 seconds delay between the scans*/
           )
        {
            ZigBeeStatus.flags.bits.bActiveScanToRejoin = 0;
            
            params.MLME_SCAN_request.ScanType = MAC_SCAN_ACTIVE_SCAN;
            /* Zigbee 2006:  Be sure that these parameters are copied over - dont assume alignment here*/
            params.MLME_SCAN_request.ScanChannels = current_SAS.spas.ChannelMask;
            params.MLME_SCAN_request.ScanDuration = NWK_SCAN_DURTION;
            return MLME_SCAN_request;
        }
        }//#endif

        /********** Data to Concentrator after route record is send here ************************/
         #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                /* Check whether data to concentrator is pending */
                if( DataToBeSendAfterRouteRecord.DataBackUp != NULL )
                {
                    BYTE *ptr;
                    BYTE msduHandle;
                    /* Check whether route record send */
                    if( DataToBeSendAfterRouteRecord.routeRecordSend == TRUE )
                    {
                        /* get a confirmation handle to send data confirm to upper layer */
                        for (i=0; (i<MAX_NWK_FRAMES) && (nwkConfirmationHandles[i].nsduHandle!=INVALID_NWK_HANDLE); i++) {}
                        if (i == MAX_NWK_FRAMES)
                        {
                           params.NLDE_DATA_confirm.NsduHandle = DataToBeSendAfterRouteRecord.nsduHandle;
                           params.NLDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                           SRAMfree( DataToBeSendAfterRouteRecord.DataBackUp );
                           DataToBeSendAfterRouteRecord.DataBackUp = NULL;
                           return NLDE_DATA_confirm;
                        }
                        ptr = DataToBeSendAfterRouteRecord.DataBackUp;
                        /* Fill TxBuffer */
                        LoadBackUpDataToTxBuffer(DataToBeSendAfterRouteRecord.dataLength,ptr );

                        // Save the NWK handle .
                        nwkConfirmationHandles[i].nsduHandle = DataToBeSendAfterRouteRecord.nsduHandle;

                        // Load up the MCPS_DATA.request parameters
                        Prepare_MCPS_DATA_request( DataToBeSendAfterRouteRecord.macDestination.Val, &msduHandle );
                        nwkConfirmationHandles[i].msduHandle = msduHandle;

                        SRAMfree( DataToBeSendAfterRouteRecord.DataBackUp );
                        DataToBeSendAfterRouteRecord.DataBackUp = NULL;
                        return MCPS_DATA_request;
                    }
                }
            #endif    /*  I_SUPPORT_MANY_TO_ONE_HANDLING */
        // Handle any pending broadcast messages

        if (NOW_I_AM_NOT_A_RFD()){//#ifndef I_AM_RFD
            if (nwkStatus.flags.bits.bSendingBroadcastMessage)
            {
                BYTE        BTTIndex;
                #ifndef I_AM_END_DEVICE
                    struct _BROADCAST_INFO
                    {
                        BYTE        destroyPacket   : 1;    // Well, not really any more.  Just send up a confirm.
                        BYTE        firstBroadcast  : 1;
                        BYTE        sendMessage     : 1;
                    }           broadcastInfo;

                    BYTE        neighborIndex;
                    BYTE        queueIndex;
                #endif

                // NOTE: Compiler SSR27744, TickGet() output must be assigned to a variable.

                for (BTTIndex=0; BTTIndex<NUM_BUFFERED_BROADCAST_MESSAGES; BTTIndex++)
                {
                    if (nwkStatus.BTT[BTTIndex] != NULL)
                    {
                        // See if the packet's delivery time has expired, if so free the packet.  Otherwise,
                        // keep it around in case another divice thought it transmitted but didn't, and this device got cascading
                        // messages.
                        /* ZigBee 2006: PIC18 occasionally over runs the BTT in group addressing broadcasts
                         * so aging the broadcast entries faster to avoid over flow
                        */
                        #ifdef NIB_STATIC_IMPLEMENTATION
                        if ((TickGetDiff(tempTick, nwkStatus.BTT[BTTIndex]->broadcastTime)) > (ONE_SECOND * ((DWORD)NIB_nwkNetworkBroadcastDeliveryTime)))
                        #else
                        if ((TickGetDiff(tempTick, nwkStatus.BTT[BTTIndex]->broadcastTime)) > (ONE_SECOND * ((DWORD)NIB.nwkNetworkBroadcastDeliveryTime)))
                        #endif
                        {
                            SRAMfree( nwkStatus.BTT[BTTIndex]->dataPtr );
                            nfree( nwkStatus.BTT[BTTIndex] );

                        }
                         #ifndef    I_AM_END_DEVICE
                        else if((!nwkStatus.BTT[BTTIndex]->btrInfo.bConfirmSent))
                        {
                            // If confirm not sent up, see if there is a need to broadcast/rebroadcast/unicast.

                            broadcastInfo.sendMessage = 0;
                            broadcastInfo.firstBroadcast = 0;
                            if (nwkStatus.BTT[BTTIndex]->currentNeighbor != MAX_NEIGHBORS)
                            {
                                neighborIndex = nwkStatus.BTT[BTTIndex]->currentNeighbor;
                                if (neighborIndex == 0xFF)
                                {
                                    // Wait the jitter time before sending the first broadcast.
                                    if((tempTick.Val - nwkStatus.BTT[BTTIndex]->broadcastJitterTimer.Val) >= (DWORD)BROADCAST_JITTER)
                                    {
                                        // Send the first broadcast packet.
                                        macAddress.Val = 0xFFFF;
                                        broadcastInfo.sendMessage = 1;
                                        broadcastInfo.firstBroadcast = 1;
                                        nwkStatus.BTT[BTTIndex]->broadcastTime = TickGet();
                                        neighborIndex = 0;
                                    }
                                }
                                else
                                {
                                    // For each neighbor, see there is a need to unicast the message.
                                    // If not, mark as frame sent - device should have received the
                                    // original broadcast message.  If so, unicast.  If the
                                    // neighbor record is not in use, mark it already done.  Mark as
                                    // many as possible until there is one that must be unicast.
                                    do
                                    {
                                        #ifdef USE_EXTERNAL_NVM
                                            pCurrentNeighborRecord = neighborTable + (WORD)neighborIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                                        #else
                                            pCurrentNeighborRecord = &(neighborTable[neighborIndex]);
                                        #endif
                                        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

                                        if (!currentNeighborRecord.deviceInfo.bits.bInUse)
                                        {
                                            nwkStatus.BTT[BTTIndex]->flags[neighborIndex].byte = 0;
                                        }
                                        else
                                        {
                                            /* A ZigBee 2006 requirement */
                                            if( (nwkStatus.BTT[BTTIndex]->nwkDestinationAddress.Val == 0xFFFC) ||
                                                (nwkStatus.BTT[BTTIndex]->nwkDestinationAddress.Val == 0xFFFD && currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle == 0 ) ||
                                                (currentNeighborRecord.shortAddr.Val == nwkStatus.BTT[BTTIndex]->nwkSourceAddress.Val) ||
                                                (nwkStatus.BTT[BTTIndex]->nwkFrameControlMSB.bits.multicastFlag))
                                            {
                                                // We should unicast the multicast frame to sleeping child end devices.
                                                // If this is the message source, we do not send it back to them!
                                                nwkStatus.BTT[BTTIndex]->flags[neighborIndex].byte = 0;
                                            }
                                            else if (!currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle)
                                            /* Allow end devices whose recievers are on the get broadcast messages */
                                            {
                                                // Unicast the message.
                                                macAddress.Val = currentNeighborRecord.shortAddr.Val;
                                                broadcastInfo.sendMessage = 1;
                                                // Clear the relayed bit - the MAC will handle any retransmission
                                                nwkStatus.BTT[BTTIndex]->flags[neighborIndex].bits.bMessageNotRelayed = 0;
                                            }
                                        }

                                        neighborIndex++;
                                    } while (!broadcastInfo.sendMessage && (neighborIndex < MAX_NEIGHBORS));

                                }


                                nwkStatus.BTT[BTTIndex]->currentNeighbor = neighborIndex;
                            }
                            else
                            {
                                // Messages have been sent to all of the neighbors, so see if they've all responded.  If not, rebroadcast
                                broadcastInfo.destroyPacket = 1;
                                params.NLDE_DATA_confirm.Status = SUCCESS;
                                for (neighborIndex=0;
                                     (neighborIndex<MAX_NEIGHBORS) && (nwkStatus.BTT[BTTIndex]->flags[neighborIndex].bits.bMessageNotRelayed==0);
                                     neighborIndex++) {}

                                if (neighborIndex!=MAX_NEIGHBORS)
                                {
                                    params.NLDE_DATA_confirm.Status = TRANSACTION_EXPIRED;

                                    // Not every devices has retransmitted the message.  See if need to or can transmit.
                                    if (nwkStatus.BTT[BTTIndex]->btrInfo.nRetries != 0)
                                    {
                                        // See if this broadcast has timed out
                                        #ifdef NIB_STATIC_IMPLEMENTATION
                                            if((TickGetDiff(tempTick, nwkStatus.BTT[BTTIndex]->broadcastTime)) >= (ONE_MILLISECOND * ((DWORD)NIB_nwkPassiveAckTimeout)))
                                        #else
                                            if((TickGetDiff(tempTick, nwkStatus.BTT[BTTIndex]->broadcastTime)) >= (ONE_MILLISECOND * ((DWORD)NIB.nwkPassiveAckTimeout)))
                                        #endif
                                        {
                                            broadcastInfo.destroyPacket = 0;
                                            macAddress.Val = 0xFFFF;
                                            broadcastInfo.sendMessage = 1;
                                            nwkStatus.BTT[BTTIndex]->btrInfo.nRetries--;
                                            nwkStatus.BTT[BTTIndex]->broadcastTime = TickGet();
                                        }
                                        else
                                        {
                                            broadcastInfo.destroyPacket = 0;
                                        }
                                    }
                                }

                                // If all of the flags were cleared or the packet has timed out, destroy it.
                                if (broadcastInfo.destroyPacket)
                                {
                                    goto FinishBroadcastPacket;
                                }
                            }

                            // Unicast or broadcast, do it here.
                            if (broadcastInfo.sendMessage)
                            {
                                // If this is the first broadcast of a packet from our upper layers, add the handle to the
                                // confirmation queue.
                                queueIndex = MAX_NWK_FRAMES;
                                if (broadcastInfo.firstBroadcast && (nwkStatus.BTT[BTTIndex]->nwkSourceAddress.Val == macPIB.macShortAddress.Val))
                                {
                                    // Add this frame to the list of frames waiting confirmation.
                                    // Try to find an empty slot.
                                    for (queueIndex=0; (queueIndex<MAX_NWK_FRAMES) && (nwkConfirmationHandles[queueIndex].nsduHandle!=INVALID_NWK_HANDLE); queueIndex++) {}

                                    // If there are no empty slots, destroy the packet and return an error.
                                    if (queueIndex == MAX_NWK_FRAMES)
                                    {
                                        params.NLDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;

FinishBroadcastPacket:
                                        // Can send up a confirm now if the message came from this device.  But don't free the message
                                        // until it times out. Otherwise, if device hears another device rebroadcast, it will think it's a new
                                        // message!

                                        params.NLDE_DATA_confirm.NsduHandle = nwkStatus.BTT[BTTIndex]->nwkSequenceNumber;
                                        nwkStatus.BTT[BTTIndex]->btrInfo.bConfirmSent = 1;

                                        /*if (nwkStatus.BTT[BTTIndex]->btrInfo.bMessageFromUpperLayers)
                                        {
                                            return NLDE_DATA_confirm;
                                        }
                                        else
                                        {
                                            return NO_PRIMITIVE;
                                        }*/
                                        return NO_PRIMITIVE;
                                    }
                                }

                                // Load up the NWK header information
                                if ( nwkStatus.BTT[BTTIndex]->nwkFrameControlMSB.Val & NWK_IEEE_SRC_ADDR )
                                {
                                     for (i = 0; i < 8; i++)
                                     {
                                         TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkIEEESrcAddress.v[7-i];
                                     }
                                }

                                if ( nwkStatus.BTT[BTTIndex]->nwkFrameControlMSB.bits.multicastFlag )
                                {
                                    TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkMulticastFields.Val;
                                }
                                TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkSequenceNumber;
                                TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkRadius;
                                TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkSourceAddress.byte.MSB;
                                TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkSourceAddress.byte.LSB;
                                TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkDestinationAddress.byte.MSB;
                                TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkDestinationAddress.byte.LSB;
                                TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkFrameControlMSB.Val;
                                TxBuffer[TxHeader--] = nwkStatus.BTT[BTTIndex]->nwkFrameControlLSB.Val;

                                // Load up the NWK payload
                                for (i=0, ptr=nwkStatus.BTT[BTTIndex]->dataPtr; i<nwkStatus.BTT[BTTIndex]->dataLength; i++)
                                {
                                    TxBuffer[TxData++] = *ptr++;
                                }

                                // Load up the MCPS_DATA.request parameters
                                Prepare_MCPS_DATA_request( macAddress.Val, &i );
                                if (queueIndex != MAX_NWK_FRAMES)
                                {
                                    nwkConfirmationHandles[queueIndex].msduHandle = i;
                                    nwkConfirmationHandles[queueIndex].nsduHandle = nwkStatus.BTT[BTTIndex]->nwkSequenceNumber;
                                }
                                return MCPS_DATA_request;
                            }
                        }
                        #endif
                    }
                }

                // See if all of the broadcast messages are done
                for (BTTIndex=0; (BTTIndex<NUM_BUFFERED_BROADCAST_MESSAGES) && (nwkStatus.BTT[BTTIndex]==NULL); BTTIndex++) {}
                if (BTTIndex == NUM_BUFFERED_BROADCAST_MESSAGES)
                {
                    //ConsolePutROMString( (ROM char *)"NWK: All broadcast messages complete\r\n" );
                    nwkStatus.flags.bits.bSendingBroadcastMessage = 0;
                }
            }   // End of handling broadcast messages.
#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
        // ---------------------------------------------------------------------
        // PAN Id conflict handling
            else if ( panIDConflictStatus.flags.bits.bPANIdUpdatePending && (!macTasksPending.bits.indirectPackets)) //This flag uses broadcast timer intrinsically
            {
                panIDConflictStatus.flags.bits.bPANIdUpdatePending = FALSE;
                panIDConflictStatus.flags.bits.bResolutionInProgress = FALSE;
                if ( nwkStatus.panIDUpdate != NULL)
                {
                    currentNeighborTableInfo.nwkUpdateId =  nwkStatus.panIDUpdate->nwkUpdateId;
                    PutNeighborTableInfo();
                    macPIB.macPANId.Val  =     nwkStatus.panIDUpdate->newPANId.Val;
                    MLME_SET_macPANId_hw();
                    nfree(nwkStatus.panIDUpdate);
                    #ifndef I_AM_END_DEVICE
                        SetBeaconPayload(TRUE);
                    #endif
                    StorePersistancePIB();
                    appNotify.current_PANID.Val = macPIB.macPANId.Val;
                    appNotification = TRUE;
                }
            }
#ifndef I_AM_END_DEVICE
            if ( panIDConflictStatus.flags.bits.bActionOnPANIdReportPending )
            {
                if ( VerifyNetworkManager() )
                {
                     BYTE nwkUpdateId;
                     WORD newPANId;

                    while (1)
                    {
                        newPANId = RANDOM_16BIT;
                        for ( i = 0 ; i < nwkStatus.PanIDReport.noOfRecords ; i++)
                        {
                            if ( nwkStatus.PanIDReport.PANIdList[i].Val == newPANId )
                            {
                                break;
                            }
                        }
                        if ( i == nwkStatus.PanIDReport.noOfRecords )
                            break;
                    }
                    if ( nwkStatus.panIDUpdate == NULL )
                    {
                        nwkStatus.panIDUpdate = (PAN_ID_UPDATE*)( SRAMalloc( sizeof( PAN_ID_UPDATE )));
                        if ( nwkStatus.panIDUpdate == NULL )
                        {
                           #ifdef DEBUG_LOG
                            LOG_ASSERT(DEBUG_LOG_INFO, "Mem Aloc Fail for PAN_ID_UPDATE" == 0);
                           #endif
                            return NO_PRIMITIVE;
                        }
                    }
                    nwkUpdateId = currentNeighborTableInfo.nwkUpdateId;
                    nwkUpdateId++;
                    nwkStatus.panIDUpdate->nwkUpdateId =  nwkUpdateId;
                    nwkStatus.panIDUpdate->newPANId.Val = newPANId;

                    PrepareNetworkUpdateCmdFrame();
                }
                panIDConflictStatus.flags.bits.bActionOnPANIdReportPending = FALSE;
            }
#endif
#endif
        }//#endif

		if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
        // ---------------------------------------------------------------------
        // PAN Id conflict handling
            if ( panIDConflictStatus.flags.bits.bPANIdUpdatePending ) //For end device nwkStatus.panIDUpdate has a timer
            {
                #ifdef NIB_STATIC_IMPLEMENTATION
                if ((TickGetDiff(tempTick, nwkStatus.panIDUpdate->waitTime)) > (ONE_SECOND * ((DWORD)NIB_nwkNetworkBroadcastDeliveryTime)))
                #else
                if ((TickGetDiff(tempTick, nwkStatus.panIDUpdate->waitTime)) > (ONE_SECOND * ((DWORD)NIB.nwkNetworkBroadcastDeliveryTime)))
                #endif
                {
                    panIDConflictStatus.flags.bits.bPANIdUpdatePending = FALSE;
                    if ( nwkStatus.panIDUpdate != NULL)
                    {
                        currentNeighborTableInfo.nwkUpdateId =  nwkStatus.panIDUpdate->nwkUpdateId;
                        PutNeighborTableInfo();
                        macPIB.macPANId.Val  =     nwkStatus.panIDUpdate->newPANId.Val;
                        MLME_SET_macPANId_hw();
                        nfree(nwkStatus.panIDUpdate);
                        StorePersistancePIB();
                        appNotify.current_PANID.Val = macPIB.macPANId.Val;
                        appNotification = TRUE;
                    }
                }
            }

#endif
		}//#endif

        // ---------------------------------------------------------------------
        // Handle any pending route discovery maintenance

        #if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
            if (nwkStatus.flags.bits.bAwaitingRouteDiscovery)
            {
                BYTE                rdIndex;
                #if ( ZIGBEE_PRO == 0x01)
                    BYTE            commandOptions = 0;
                #endif
                // See if any of our route requests need to be rebroadcast.
                for (rdIndex = 0; rdIndex < ROUTE_DISCOVERY_TABLE_SIZE; rdIndex ++ )
                {
                    if (routeDiscoveryTablePointer[rdIndex] != NULL)
                    {
                        if (routeDiscoveryTablePointer[rdIndex]->forwardRREQ &&
                            (((routeDiscoveryTablePointer[rdIndex]->status.initialRebroadcast == 0) &&
                              ((tempTick.Val - routeDiscoveryTablePointer[rdIndex]->rebroadcastTimer.Val) >= (DWORD)(ONE_SECOND*nwkcRREQRetryInterval/1000))) ||
                             ((routeDiscoveryTablePointer[rdIndex]->status.initialRebroadcast == 1) &&
                              ((tempTick.Val - routeDiscoveryTablePointer[rdIndex]->rebroadcastTimer.Val) >= (DWORD)RREQ_BROADCAST_JITTER))))
                        {
							printf("rdIndex = ");
							PrintChar(rdIndex);
							printf("-----");
							
							printf("\n\r");
							
							
								ptr = routeDiscoveryTablePointer[rdIndex]->forwardRREQ;

								//Block TX
								ZigBeeBlockTx();
								TxData = 0x00;
								TxHeader = TX_HEADER_START;
								// Load up the NWK payload - the route request command frame.
								for (i=0; i<sizeof_ROUTE_REQUEST_COMMAND; i++)
								{
									#if ( ZIGBEE_PRO == 0x01)
										if ( i == 1 ) //CommandOptions field is always in 2 byte position in the RREQ frame
										{
											commandOptions = *ptr;
										}
									#endif
									TxBuffer[TxData++] = *ptr++;       // Command, options, ID, dest addr LSB, dest addr MSB, path cost
								}
								#if ( ZIGBEE_PRO == 0x01)
									if ( commandOptions & DEST_IEEE_ADDRESS_BIT )
									{
										for(i = 0; i < sizeof_RREQ_COMMAND_DST_IEEE_ADDR; i++)
										{
											TxBuffer[TxData++] = *ptr++;
										}
									}
								#endif
								// Load up the old NWK header (backwards).
								for (i=0; i<sizeof_ROUTE_REQUEST_COMMAND_HEADER; i++)
								{
									TxBuffer[TxHeader--] = *ptr++;
								}

								// We've handled the first rebroadcast of a received RREQ (if any), so the rest of the
								// timing will be based off of the nwkcRREQRetryInterval.  See if we have any retries left.
								// If not, free the message.
								routeDiscoveryTablePointer[rdIndex]->status.initialRebroadcast = 0;
								routeDiscoveryTablePointer[rdIndex]->status.transmitCounter--;
								routeDiscoveryTablePointer[rdIndex]->rebroadcastTimer = tempTick;
								if (routeDiscoveryTablePointer[rdIndex]->status.transmitCounter == 0)
								{
									nfree( routeDiscoveryTablePointer[rdIndex]->forwardRREQ );
								}

								// Load up the MCPS_DATA.request parameters.
								Prepare_MCPS_DATA_request( 0xFFFF, &i );
								return MCPS_DATA_request;
						}
                    }
                }

                // See if any of our route discoveries have timed out.
                for (rdIndex = 0; rdIndex < ROUTE_DISCOVERY_TABLE_SIZE; rdIndex ++ )
                {
                    if ((routeDiscoveryTablePointer[rdIndex] != NULL) &&
                        ((tempTick.Val - routeDiscoveryTablePointer[rdIndex]->timeStamp.Val) > (DWORD)(ONE_SECOND*nwkcRouteDiscoveryTime/1000)))
                    {
                        /* In case of many to one routing rtindex on initiator will be 0xff )*/
                        if( routeDiscoveryTablePointer[rdIndex]->routingTableIndex != 0xff )
                        {
                            #ifdef USE_EXTERNAL_NVM
                                pCurrentRoutingEntry = routingTable + (WORD)(routeDiscoveryTablePointer[rdIndex]->routingTableIndex) * (WORD)sizeof(ROUTING_ENTRY);
                            #else
                                pCurrentRoutingEntry = &(routingTable[routeDiscoveryTablePointer[rdIndex]->routingTableIndex]);
                            #endif
                            GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
                            // Make sure that a valid route is not distroyed
                            if (currentRoutingEntry.rtflags.rtbits.bStatus == ROUTE_DISCOVERY_UNDERWAY)
                            {
                                /*Retain the ACTIVE status of the Routing table entry if
                                we have reused the routing table entry.*/
                                if ((!routeDiscoveryTablePointer[rdIndex]->status.initiator) &
                                    routeDiscoveryTablePointer[rdIndex]->status.previousStateACTIVE)
                                {
                                    /*Previous status was ACTIVE. So maintain the same*/
                                    currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_ACTIVE;
                                }
                                else
                                {
                                    // Mark the record as a failed discovery attempt.
                                    currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_DISCOVERY_FAILED;
                                }
                                #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
                                    currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                                #endif
                                PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
                            }
                        }
                        // Free the route discovery table entry.
                        if (routeDiscoveryTablePointer[rdIndex]->forwardRREQ)
                        {
                           SRAMfree( routeDiscoveryTablePointer[rdIndex]->forwardRREQ );
                        }
                        nfree( routeDiscoveryTablePointer[rdIndex] );
                    }
                }

                // If  waiting on route discovery for a message, see if there is now
                // an active route and can send the message, or if discovery
                // has timed out or failed then device can report an error and discard the message.
                for (i=0; i<NUM_BUFFERED_ROUTING_MESSAGES; i++)
                {
                    if (nwkStatus.routingMessages[i])
                    {
                        BYTE        queueIndex = 0;
                        BYTE        routeStatus;
                        BYTE        SendRRERFrame;
                        SHORT_ADDR  nextHopAddr;


                        GetNextHop( nwkStatus.routingMessages[i]->destinationAddress, &(macAddress), &routeStatus );

                        if (routeStatus == ROUTE_ACTIVE)
                        {
                            // Can now send the message now.
                            // If the message is from the upper layers, get ready to add the frame to the confirmation queue.
                            if (nwkStatus.routingMessages[i]->sourceAddress.Val == macPIB.macShortAddress.Val)
                            {
                                // Try to find an empty slot.
                                for (queueIndex=0; (queueIndex<MAX_NWK_FRAMES) && (nwkConfirmationHandles[queueIndex].nsduHandle!=INVALID_NWK_HANDLE); queueIndex++) {}

                                // If there are no empty slots, destroy the message and return an error.
                                if (queueIndex == MAX_NWK_FRAMES)
                                {
                                    params.NLDE_DATA_confirm.NsduHandle = *(nwkStatus.routingMessages[i]->dataPtr+7);
                                    params.NLDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                                    SRAMfree( nwkStatus.routingMessages[i]->dataPtr );
                                    nfree( nwkStatus.routingMessages[i] );
                                    ZigBeeUnblockTx();
                                    return NLDE_DATA_confirm;
                                }
                            }

                            // Load up the saved NWK header and payload.

                            ptr = nwkStatus.routingMessages[i]->dataPtr;

                                   LoadBackUpDataToTxBuffer(  nwkStatus.routingMessages[i]->dataLength, ptr );

                            // Load up the MCPS_DATA.request parameters.
                            Prepare_MCPS_DATA_request( macAddress.Val, &j );

                            // If the message is from the upper layers, add the frame to the confirmation queue.
                            if (nwkStatus.routingMessages[i]->sourceAddress.Val == macPIB.macShortAddress.Val)
                            {
                                nwkConfirmationHandles[queueIndex].msduHandle = j;
                                nwkConfirmationHandles[queueIndex].nsduHandle = *(nwkStatus.routingMessages[i]->dataPtr+7);
                            }

                            SRAMfree( nwkStatus.routingMessages[i]->dataPtr );
                            nfree( nwkStatus.routingMessages[i] );
                            return MCPS_DATA_request;
                        }
                        else if (routeStatus == ROUTE_DISCOVERY_FAILED)
                        {
                            // The route has failed.  Either notify the upper layers with a confirm or
                            // send a route error.  Destroy the buffered message.
                            if (nwkStatus.routingMessages[i]->sourceAddress.Val == macPIB.macShortAddress.Val)
                            {
// This label is no longer used
//SendUpNoRouteAvailable:
                                params.NLDE_DATA_confirm.NsduHandle = *(nwkStatus.routingMessages[i]->dataPtr+7);
                                params.NLDE_DATA_confirm.Status = NWK_ROUTE_ERROR;
                                SRAMfree( nwkStatus.routingMessages[i]->dataPtr );
                                nfree( nwkStatus.routingMessages[i] );
                                ZigBeeUnblockTx();
                                return NLDE_DATA_confirm;
                            }
                            else
                            {
                                BYTE rtStatus;
                                /*SendRRERFrame = FindNextHopAddress
                                                (
                                                    nwkStatus.routingMessages[i]->sourceAddress,
                                                    &nextHopAddr
                                                );*/
                                SendRRERFrame = GetNextHop
                                                (
                                                    nwkStatus.routingMessages[i]->sourceAddress,
                                                    &nextHopAddr,
                                                    &rtStatus
                                                );

                                if (SendRRERFrame)
                                {
                                    /*Update the TxBuffer with the Network Header for Route Error Command*/
                                    CreateNwkCommandHeader
                                    (
                                        NLME_GET_nwkBCSN(),
                                        DEFAULT_RADIUS,
                                        macPIB.macShortAddress,
                                        nwkStatus.routingMessages[i]->sourceAddress
                                    );

                                    /*Update the TxBuffer with the Network Payload for Nwk Status Command*/
                                    CreateNwkCommandNwkStatus
                                    (
                                        NWK_STATUS_NO_ROUTE_AVAILABLE,
                                        nwkStatus.routingMessages[i]->destinationAddress
                                    );

                                    // Load up the MCPS_DATA.request parameters.
                                    Prepare_MCPS_DATA_request( nextHopAddr.Val, &j );

                                    nfree( nwkStatus.routingMessages[i]->dataPtr );
                                    nfree( nwkStatus.routingMessages[i] );

                                    return MCPS_DATA_request;
                                }
                                else
                                {
                                    nfree( nwkStatus.routingMessages[i]->dataPtr );
                                    nfree( nwkStatus.routingMessages[i] );
                                    ZigBeeUnblockTx(); //Is this required? Assuming somewhere it is blocked?
                                    return NO_PRIMITIVE;
                                }
                            }
                        }

                    }
                }

                // See if we can clean up the routing table.  Purge any routes that failed.  We have
                // already sent errors for any messages buffered for these routes.
                #ifdef USE_EXTERNAL_NVM
                for (i=0, pCurrentRoutingEntry = routingTable; i<ROUTING_TABLE_SIZE; i++, pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY))
                #else
                for (i=0, pCurrentRoutingEntry = routingTable; i<ROUTING_TABLE_SIZE; i++, pCurrentRoutingEntry++)
                #endif
                {
                    GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
                    if (currentRoutingEntry.rtflags.rtbits.bStatus == ROUTE_DISCOVERY_FAILED)
                    {
                        // Mark the record as inactive and available for reuse.
                        currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_INACTIVE;
                        currentRoutingEntry.destAddress.Val = 0xFFFF;
                        #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
                            currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                        #endif
                        PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );

                        /* ZigBee 2006 requirement:  Timed out or failed, so notify NHLE */
                        params.NLME_ROUTE_DISCOVERY_confirm.Status = ROUTE_DISCOVERY_FAILED;
                        return NLME_ROUTE_DISCOVERY_confirm;
                    }
                }

                // See if we are done with all route discoveries and all buffered messages.
                for (i=0; (i<NUM_BUFFERED_ROUTING_MESSAGES) && (nwkStatus.routingMessages[i]==NULL); i++) {}
                if (i == NUM_BUFFERED_ROUTING_MESSAGES)
                {
                    for (i=0; (i<ROUTE_DISCOVERY_TABLE_SIZE) && (routeDiscoveryTablePointer[i]==NULL); i++) {}
                    if (i == ROUTE_DISCOVERY_TABLE_SIZE)
                    {

                        nwkStatus.flags.bits.bAwaitingRouteDiscovery = 0;
                    }
                }
            }
        #endif

        //----------------------------------------------------------------------

        #if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
        #ifndef I_AM_END_DEVICE
            if( panIDConflictStatus.flags.bits.bPANIdConflictDetected )
            {
                panIDConflictStatus.flags.bits.bPANIdConflictDetected = FALSE;
                if ( VerifyNetworkManager() )
                {
                    panIDConflictStatus.flags.bits.bActionOnPANIdReportPending = TRUE;
                    return NO_PRIMITIVE;
                }
                else
                {
                    return PrepareNetworkReportCmdFrame();
                }
            }
        #endif
        #endif
        // ---------------------------------------------------------------------
        // ZigBee 2006 requirement
        if(NOW_I_AM_NOT_A_CORDINATOR()){//#if !defined(I_AM_COORDINATOR)
        if ( nwkStatus.flags.bits.bRejoinInProgress )
        {
            if(TickGetDiff(tempTick, nwkStatus.rejoinStartTick) > SYMBOLS_TO_TICKS((DWORD)aResponseWaitTime))
            {
                if (NOW_I_AM_A_RFD()){//#if defined(I_AM_RFD)
                    // bRejoinInProgress bit is used to trigger the Authentication procedure also.
                    // In case we received unsolicited Rejoin Response we should not trigger the
                    // Authentication procedure.
                    // This block of code is executed when we aResponseWaitTime expires we have to
                    // start polling to extract the Rejoin Response.
                    // If we reset bRejoinInProgress bit then Authentication will not be triggered in
                    // normal rejoin scenario. So, we need to stay for some time so that after polling
                    // rejoin response can be processed and make use of bRejoinInProgress bit for triggering
                    // Authentication procedure.
                    // Since the time to wait before resetting is purely based on the design,
                    // we have decided to wait for twice the aResponseWaitTime. By this time authentication
                    // would have been triggered. So, when next time the timer expires, we can just clear
                    // the bRejoinInProgress bit.
                    //if (TickGetDiff(tempTick, nwkStatus.rejoinStartTick) >  ( 2 * (SYMBOLS_TO_TICKS((DWORD)aResponseWaitTime))))
                    if (waitForKey)
                    {
                        nwkStatus.flags.bits.bRejoinInProgress = 0;
                        params.NLME_JOIN_confirm.Status = NWK_NO_NETWORKS;
                        params.NLME_JOIN_confirm.ShortAddress.Val = 0xFFFF;
                        waitForKey = 0;
                        return NLME_JOIN_confirm;
                    }
                    nwkStatus.rejoinStartTick = tempTick;
                    waitForKey = 1;
                    params.NLME_SYNC_request.Track = FALSE;
                    return NLME_SYNC_request;
                }else{//#else
                    nwkStatus.flags.bits.bRejoinInProgress = 0;
                    params.NLME_JOIN_confirm.Status = NWK_NO_NETWORKS;
                    params.NLME_JOIN_confirm.ShortAddress.Val = 0xFFFF;
                    return NLME_JOIN_confirm;
                }//#endif
            }
        }
        }//#endif
        /* ZigBee 2006 requirement:  Send a single poll request to new parent after rejoinCommand */
        if (NOW_I_AM_A_RFD()){//#if defined(I_AM_RFD)
            if (nwkStatus.rejoinCommandSent)
            {
                if( TickGetDiff(tempTick, nwkStatus.rejoinStartTick) > SYMBOLS_TO_TICKS((DWORD)aResponseWaitTime))
                {
                    nwkStatus.rejoinCommandSent = 0x00;

                    params.NLME_SYNC_request.Track = FALSE;
                    return NLME_SYNC_request;

                }
            }
        }//#endif


        // ZigBee 2006 requirement:   process rejoin properly */
        #if !defined(I_AM_END_DEVICE)
            if ( nwkStatus.flags.bits.bRejoinIndication )
            {
                /* Update the rejoin indication parameters */
                nwkStatus.flags.bits.bRejoinIndication = 0;
                params.NLME_JOIN_indication.CapabilityInformation.Val = rejoinIndication.CapabilityInformation.Val;
                params.NLME_JOIN_indication.secureRejoin = rejoinIndication.secureJoin;
                params.NLME_JOIN_indication.NetworkAddress.Val = rejoinIndication.ShortAddress.Val;
                params.NLME_JOIN_indication.ExtendedAddress = rejoinIndication.ExtendedAddress;
                params.NLME_JOIN_indication.RejoinNetwork = REJOIN_PROCESS;

                return NLME_JOIN_indication;
            }
        #endif

        // ---------------------------------------------------------------------
        // Handle any pending network leave operation
        if (nwkStatus.flags.bits.bLeaveInProgress)
        {
            SHORT_ADDR macDstShortAddr;
            BYTE nwkBCSN;
            /*CAUTION: DO NOT CHANGE THE ORDER OF EXECUTION.
            First Condition should check whether we have to do final clean-up*/
            // See if device is doing the final leave clean-up.
            if (nwkStatus.flags.bits.bLeaveReset )
            {
                // ZigBee 2006 requirement:
                nwkStatus.flags.bits.bLeaveReset = 0;
                nwkStatus.flags.bits.bLeaveInProgress = 0;

                // Reset the MAC layer.  Note that the NWK layer will automatically reset upon
                // MLME_RESET_confirm, so we do not have to clear any flags here.
                params.MLME_RESET_request.SetDefaultPIB = TRUE;
                return MLME_RESET_request;
            }
            /*CAUTION: DO NOT CHANGE THE ORDER OF EXECUTION.
            Second Condition should check we have sent leave command frame.
            If we do not have this condition check, then device will keep
            transmitting the leave command frame. Since the transmission of
            leave command frame is done in the backgound, we need have this
            condition to avoid multiple transmissions.*/
            if ( nwkStatus.flags.bits.bLeaveCmdSent )
            {
                return NO_PRIMITIVE;
            }
            nwkBCSN = NLME_GET_nwkBCSN();
            /*Check whether the device is leaving the network.*/
            if ( nwkStatus.flags.bits.bSelfLeave )
            {
                SHORT_ADDR  nwkDstShortAddr;

                #if defined (I_AM_END_DEVICE)
                    /*We should not send the leave command frame from End Device, if
                    it is not self initiated. We just have to reset.*/
                    if ( nwkStatus.leaveReason != SELF_INITIATED_LEAVE )
                    {
                        nwkStatus.flags.bits.bLeaveReset = 1;
                        return NO_PRIMITIVE;
                    }
                    nwkDstShortAddr.Val = macPIB.macCoordShortAddress.Val;
                    macDstShortAddr.Val = nwkDstShortAddr.Val;
                #else
                    /*Initially update the nwk and mac destination address as broadcast address.
                    This is required for both Coordinator and Router. If Router, then it has to
                    check whether it has the Routing capability. If it can route, then Leave
                    Command frame can be broadcasted. Otherwise, it shall unicast the Leave
                    Command frame to parent device, even though it is a ZigBee Router.*/
                    nwkDstShortAddr.Val = 0xFFFD;
                    macDstShortAddr.Val = 0xFFFF;
                    if(NOW_I_AM_A_ROUTER()){//#if defined (I_AM_ROUTER)
                        /*In case of Secured network, Router fails to join because of Trust Center
                        did not allow this device to join, Router device should unicast the Leave
                        Command frame to the parent. It should not broadcast because it has not
                        started acting as ZigBee Router, i.e not issued NLME_START_ROUTER.request*/
                        if ( !nwkStatus.flags.bits.bCanRoute )
                        {
                            nwkDstShortAddr.Val = macPIB.macCoordShortAddress.Val;
                            macDstShortAddr.Val = nwkDstShortAddr.Val;
                        }
                    }//#endif
                #endif

                /*CreateNwkCommandHeader
                (
                    NLME_GET_nwkBCSN(),
                    1,
                    macPIB.macShortAddress,
                    nwkDstShortAddr
                );*/
                BYTE        includeDstIEEEAddr;

                /*Add the SrcIEEEAddress and DstIEEEAddress*/
                includeDstIEEEAddr = AddNwkHeaderIEEEAddresses(nwkDstShortAddr);

                TxBuffer[TxHeader--] = nwkBCSN;	
                TxBuffer[TxHeader--] = 1;
                TxBuffer[TxHeader--] = macPIB.macShortAddress.byte.MSB;
                TxBuffer[TxHeader--] = macPIB.macShortAddress.byte.LSB;
                TxBuffer[TxHeader--] = nwkDstShortAddr.byte.MSB;
                TxBuffer[TxHeader--] = nwkDstShortAddr.byte.LSB;

                BYTE secBitPosInNwkHdr = 0x00;

                // This condition is required because when a device fails to join
                // the network due to Network Key could not be received, then the
                // device should leave the network by unicasting the leave command
                // frame to the parent device. This leave command frame should not
                // be encrypted.
                #ifdef I_SUPPORT_SECURITY
                    if ( !securityStatus.flags.bits.bAuthorization )
                    {
                        // The device has been authorized already and part of the network.
                        secBitPosInNwkHdr = 0x02;
                    }
                #endif
                if (includeDstIEEEAddr == TRUE)
                {
                    TxBuffer[TxHeader--] = secBitPosInNwkHdr | NWK_IEEE_SRC_ADDR | NWK_IEEE_DST_ADDR;
                }
                else
                {
                    TxBuffer[TxHeader--] = secBitPosInNwkHdr | NWK_IEEE_SRC_ADDR;
                }

                TxBuffer[TxHeader--] = NWK_FRAME_CMD | (nwkProtocolVersion<<2);    // nwkFrameControlLSB

                /*Update the frame control fields -
                like Security, SrcIEEEAddress, DstIEEEAddress, and ProtocolVerison bits*/
                //UpdateNwkHeaderFrameContolFields(includeDstIEEEAddr);
            }
            else
            {
                CreateNwkCommandHeader
                (
					nwkBCSN,		/* Use previously saved value */
                    1,
                    macPIB.macShortAddress,
                    nwkStatus.leaveDeviceAddress
                );
                macDstShortAddr.Val = nwkStatus.leaveDeviceAddress.Val;
            }

            // Load the NWK payload into the transmit buffer.
            TxBuffer[TxData++] = NWK_COMMAND_LEAVE;
            i = 0x00;
            if ( !nwkStatus.flags.bits.bSelfLeave )
            {
                i |= NWK_LEAVE_REQUEST;
            }
            if (nwkStatus.flags.bits.bRemoveChildren)
            {
                i |= NWK_LEAVE_REMOVE_CHILDREN;
            }
            // A ZigBee 2006 requirement:
            if (nwkStatus.flags.bits.bRejoin)
            {
                i |= NWK_LEAVE_REJOIN;
            }
            TxBuffer[TxData++] = i;

            /*This is important.*/
            nwkStatus.flags.bits.bLeaveCmdSent = 1;

            Prepare_MCPS_DATA_request( macDstShortAddr.Val, &i );

            /*Make a note in the Confirmation Table. This is required to send the Leave Confirmation
            to the above layer.*/
            // Find the empty frame entry.
            for (i=0; (i<MAX_NWK_FRAMES) && (nwkConfirmationHandles[i].nsduHandle != INVALID_NWK_HANDLE); i++) {}

            // If no free entry found, do not send the leave command.
            if (i == MAX_NWK_FRAMES)
            {
                /*Reset the flags. If we do not reset, the leave will not be processed again*/
                nwkStatus.flags.bits.bLeaveInProgress = 0;
                nwkStatus.flags.bits.bLeaveCmdSent = 0;
                nwkStatus.flags.bits.bLeaveReset = 0;
                return NO_PRIMITIVE;
            }
            nwkConfirmationHandles[i].msduHandle = params.MCPS_DATA_request.msduHandle;
            // nwkConfirmationHandles[i].nsduHandle = params.MCPS_DATA_request.msduHandle;
            nwkConfirmationHandles[i].nsduHandle = nwkBCSN;
            nwkConfirmationHandles[i].nwkFrameId = NWK_COMMAND_LEAVE;
            return MCPS_DATA_request;
        }
        // Check whether any of the Routing entries needs to be purged out
        #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
            if ( nwkStatus.flags.bits.bCanRoute )
            {
                if (TickGetDiff( tempTick, nwkStatus.routingTableAgeTick) > ONE_SECOND*ROUTING_TABLE_CLEANUP_PERIOD)
                {
                    //update the RoutingTableAgeTick
                    nwkStatus.routingTableAgeTick = tempTick;
                    AgeOutRoutingTableEntries();
                    // No need to return from here.
                    // We can continue to check for other background tasks.
                }
            }
        #endif

        //--------------------------------------------------------------------------------------
        // CAUTION: DO NOT CHANGE THE ORDER OF EXECUTION OF THIS BLOCK.
        // TRANSMITTION OF LINK STATUS COMMAND FRAME SHOULD BE DONE AFTER NEIGHBOR TABLE AGING
        // PROCEDURE. THIS IS BECAUSE THE TIMER FOR LINK_STATUS_PERIOD IS SAME FOR BOTH
        // NEIGHBOR TABLE AGING AND TRANSMISSION OF LINK STATUS COMMAND.
        // Check for Neighbor table aging
        #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
            if(((NOW_I_AM_A_CORDINATOR()) &&// #ifdef I_AM_COORDINATOR
                 ( ZigBeeStatus.flags.bits.bNetworkFormed ))
            || //#else
                ((NOW_I_AM_NOT_A_CORDINATOR())&&(
                    ( ZigBeeStatus.flags.bits.bNetworkJoined )
                    #ifdef I_SUPPORT_SECURITY
                    && ( !securityStatus.flags.bits.bAuthorization )
                    #endif
                   ))
            )//#endif
                {
                    if ( TickGetDiff( tempTick, nwkStatus.nwkLinkStatusPeriod ) > NWK_LINK_STATUS_PERIOD )
                    {
                        AgeOutNeighborTableEntries();
                        // Do not return from here, because we have to transmit the Link Status command.
                        // If we return from here, then transmission of Link Status command can be delayed,
                        // because there may be an other background/foreground task gets generated.
                    }
                    if ( nwkStatus.flags.bits.bTransmitLinkStatusCommand )
                    {
                        CreateLinkStatusCommandFrame();
                        return MCPS_DATA_request;
                    }
                }
        #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

        return NO_PRIMITIVE;
    }   // End of handling background processing.

    else
    {
        /* handle primitive here */
        switch (inputPrimitive)
        {
            // ---------------------------------------------------------------------
            case MCPS_DATA_confirm:
                {
                    BYTE        rc = NO_PRIMITIVE;
                    #ifdef I_SUPPORT_ROUTING
                        BYTE        index = 0;
                        BYTE        SendRRERFrame = FALSE;
                        SHORT_ADDR  nextHopAddr;
                        SHORT_ADDR  nwkDestination;
                        BYTE nwkStatusCode = 0;
                    #endif

                    #if I_SUPPORT_STUB_APS == 1
                        /* Check mcps data confirm received for the interpan data sent */
                        if (interPanDataTransmitted == 1)
                        {
                            /* verify the msdu handle with stored msdu handle when 
                               inter PAN data request is issued */
                            if (params.MCPS_DATA_confirm.msduHandle == storeMsduHandle)
                            {
                                return handleInterPanDataConf();
                            }
                        }
                    #endif

                    if (rejoinIndicationPending == 1)
                    {
                        if (msduHandle == params.MCPS_DATA_confirm.msduHandle)
                        {
                             nwkStatus.flags.bits.bRejoinIndication = 1;
                        }
                    }

                    #ifdef I_SUPPORT_ROUTING
                        #if    I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                            /* Check whether we received confirmation for route record.
                            Set a flag to send data in this case */
                            if( DataToBeSendAfterRouteRecord.DataBackUp != NULL )
                            {
                                /* Nothing to do for this version - flag already set */
                            }
                         #endif /*  I_SUPPORT_MANY_TO_ONE_HANDLING */
                            // check if this is a route information
                            for(i = 0; i < NUM_BUFFERED_ROUTING_MESSAGES; i++)
                            {
                                if( nwkStatus.routeHandleRecords[i].SourceAddress.Val != 0xFFFF &&
                                params.MCPS_DATA_confirm.msduHandle == nwkStatus.routeHandleRecords[i].macSequence)
                                {
                                    if( params.MCPS_DATA_confirm.status != SUCCESS )
                                    {
                                        BYTE rtStatus;
                                        TxData = 0;
                                        TxHeader = TX_HEADER_START;
                                        #if    I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                        SHORT_ADDR nextHop;
                                        BYTE routeStatus;
                                        #endif
                                        #if    I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                        GetDestFromRoutingTable( nwkStatus.routeHandleRecords[i].DstAddress, &nextHop, &routeStatus );
                                             
                                            if ( ( routeStatus == ROUTE_RECORD_REQUIRED ) ||
                                                 ( routeStatus == NO_ROUTE_RECORD_REQUIRED ) )

                                            {
                                                /* Data to concentrator failed. Try to send nwk status command
                                                to concentrator indicationg from which source device data failed.
                                                NWK status command will reach the concentrator through a random neighbor.*/
                                                if( NWKFindRandomNeighbor( 0,
                                                                           nwkStatus.routeHandleRecords[i].macSourceAddress,
                                                                           nwkStatus.routeHandleRecords[i].SourceAddress ) != INVALID_NEIGHBOR_KEY )
                                                {
                                                    SendRRERFrame = TRUE;
                                                    nextHopAddr = currentNeighborRecord.shortAddr;
                                                    nwkStatusCode = NWK_STATUS_MANY_TO_ONE_ROUTE_FAILURE;
                                                    nwkDestination = nwkStatus.routeHandleRecords[i].DstAddress;

                                                }
                                            }
                                            else
                                        #endif    /*  I_SUPPORT_MANY_TO_ONE_HANDLING */
                                            {
                                                /*Remove the Routing Table entry corresponding to the destination*/
                                                //RemoveRoutingTableEntry( nwkStatus.routeHandleRecords[i].DstAddress );
                                                SendRRERFrame = GetNextHop
                                                                (
                                                                    nwkStatus.routeHandleRecords[i].SourceAddress,
                                                                    &nextHopAddr,
                                                                    &rtStatus
                                                                );
                                                #if   I_SUPPORT_MANY_TO_ONE_HANDLING
                                                    /* Put the status code as source route failure if forwarding source routed
                                                    frame is failed */
                                                    if( nwkStatus.routeHandleRecords[i].isToConcentrator == ROUTE_AS_PER_SOURCE_ROUTE_FRAME )
                                                    {
                                                         nwkStatusCode = NWK_STATUS_SOURCE_ROUTE_FAILURE;
                                                    }
                                                    else
                                                #endif
                                                    {
                                                        nwkStatusCode = NWK_STATUS_NO_ROUTE_AVAILABLE;
                                                    }
                                                nwkDestination = nwkStatus.routeHandleRecords[i].SourceAddress;
                                            }


                                        if (SendRRERFrame)
                                        {
                                            /*Update the TxBuffer with the Network Header for status Command*/
                                            CreateNwkCommandHeader
                                            (
                                                nwkStatus.routeHandleRecords[i].nwkSequence,
                                                DEFAULT_RADIUS,
                                                macPIB.macShortAddress,
                                                nwkDestination
                                            );


                                            /* check if the dest is a RFD, then return EXPIRY instead, that's the cause */
                                            #ifdef USE_EXTERNAL_NVM
                                                for (index=0, pCurrentNeighborRecord = neighborTable; index < MAX_NEIGHBORS; index++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
                                            #else
                                                for (index=0, pCurrentNeighborRecord = neighborTable; index < MAX_NEIGHBORS; index++, pCurrentNeighborRecord++)
                                            #endif // #ifdef USE_EXTERNAL_NVM
                                            {
                                                GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
                                                if ( currentNeighborRecord.deviceInfo.bits.bInUse &&
                                                     (currentNeighborRecord.shortAddr.Val == nwkStatus.routeHandleRecords[i].DstAddress.Val) &&
                                                    (!currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle)
 )
                                                {
                                                    nwkStatusCode = NWK_STATUS_INDIRECT_TRANSACTION_EXPIRY;
                                                    break;

                                                }
                                            }
                                           
                                            /*Update the TxBuffer with the Network Payload for Nwk Status Command*/
                                            CreateNwkCommandNwkStatus
                                            (
                                                nwkStatusCode,
                                                nwkStatus.routeHandleRecords[i].DstAddress
                                            );

                                            nwkStatus.routeHandleRecords[i].SourceAddress.Val = 0xFFFF;
                                            // Load up the MCPS_DATA.request parameters.
                                            Prepare_MCPS_DATA_request( nextHopAddr.Val, &i );
                                           
                                            ZigBeeBlockTx();
                                            return MCPS_DATA_request;
                                        }
                                        else
                                        {
                                            nwkStatus.routeHandleRecords[i].SourceAddress.Val = 0xFFFF;
                                            //ZigBeeUnblockTx(); //Is this required? Assuming somewhere it is blocked?
                                            return NO_PRIMITIVE;
                                        }
                                    }
                                    nwkStatus.routeHandleRecords[i].SourceAddress.Val = 0xFFFF;
                                }
                            }

                    #endif       /* I_SUPPORT_ROUTING */
                    // Find the matching frame entry.
                    for (i=0; (i<MAX_NWK_FRAMES) && (nwkConfirmationHandles[i].msduHandle!=params.MCPS_DATA_confirm.msduHandle); i++) {}

                    // If no matching frame, ignore the primitive.
                    if (i == MAX_NWK_FRAMES)
                    {
                        return NO_PRIMITIVE;
                    }

                    // If the next layer handle is not invalid, set up parameters for the next level confirm.
                    if (nwkConfirmationHandles[i].nsduHandle != INVALID_NWK_HANDLE)
                    {
                        if (nwkConfirmationHandles[i].nwkFrameId == NWK_FRAME_DATA)
                        {
                            // These overlay - params.NLDE_DATA_confirm.Status     = params.MCPS_DATA_confirm.status;
                            params.NLDE_DATA_confirm.NsduHandle = nwkConfirmationHandles[i].nsduHandle;
                            rc = NLDE_DATA_confirm;
                        }
                        if (nwkConfirmationHandles[i].nwkFrameId == NWK_COMMAND_LEAVE)
                        {
                            nwkConfirmationHandles[i].nwkFrameId = 0;
                            if( nwkStatus.flags.bits.bLeaveInProgress )
                            {
                                /*If the device is leaving the network due to self initiated, or the
                                parent device requested, we need to clean-up internal variables.
                                This clean-up is done in the MLME_RESET_confirm primitive processing.*/
                                if ( nwkStatus.leaveReason == SELF_INITIATED_LEAVE ||
                                     nwkStatus.leaveReason == COORDINATOR_FORCED_LEAVE )
                                {
                                    BYTE tempRejoin;
                                    tempRejoin = nwkStatus.flags.bits.bRejoin;

                                    /*Enable final clean-up*/
                                    nwkStatus.flags.bits.bLeaveReset = 1;

                                    if ( nwkStatus.leaveReason == SELF_INITIATED_LEAVE )
                                    {
                                        /* self initiated leaves requires null address returns, else use your long address */
                                        params.NLME_LEAVE_confirm.DeviceAddress.v[0]    = 0x00;
                                        params.NLME_LEAVE_confirm.DeviceAddress.v[1]    = 0x00;
                                        params.NLME_LEAVE_confirm.DeviceAddress.v[2]    = 0x00;
                                        params.NLME_LEAVE_confirm.DeviceAddress.v[3]    = 0x00;
                                        params.NLME_LEAVE_confirm.DeviceAddress.v[4]    = 0x00;
                                        params.NLME_LEAVE_confirm.DeviceAddress.v[5]    = 0x00;
                                        params.NLME_LEAVE_confirm.DeviceAddress.v[6]    = 0x00;
                                        params.NLME_LEAVE_confirm.DeviceAddress.v[7]    = 0x00;

                                        params.NLME_LEAVE_confirm.Status                = params.MCPS_DATA_confirm.status;
                                        rc = NLME_LEAVE_confirm;
                                    }
                                    else
                                    {
                                        rc = NO_PRIMITIVE;
                                    }
                                }
                                /*Check whether the data confirmation is received due to sending
                                leave command frame to one of the child device.*/
                                else if( nwkStatus.leaveReason == REQUEST_CHILD_TO_LEAVE )
                                {
                                    BYTE nbtIndex;

                                    nbtIndex = NWKLookupNodeByShortAddrVal( nwkStatus.leaveDeviceAddress.Val );
                                    if (nbtIndex != INVALID_NEIGHBOR_KEY)
                                    {
                                        #ifdef I_SUPPORT_SECURITY
                                        if(!I_AM_TRUST_CENTER){//#ifndef I_AM_TRUST_CENTER	//MSEB
                                                if(currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD)
                                                {
                                                    leavingChildDetails.DeviceShortAddress.Val =                                                        currentNeighborRecord.shortAddr.Val;
                                                    memcpy(leavingChildDetails.DeviceLongAddress.v,                                                         currentNeighborRecord.longAddr.v,8 );
                                                    zdoStatus.flags.bits.SendUpdateDeviceRequestAfterChildLeave = 1;
                                                }
                                        }//#endif
                                        #endif
                                        memcpy( (void *)&(params.NLME_LEAVE_confirm.DeviceAddress),
                                                (void *)&(currentNeighborRecord.longAddr), 8 );
                                        params.NLME_LEAVE_confirm.Status = params.MCPS_DATA_confirm.status;
                                        if (!params.NLME_LEAVE_confirm.Status)
                                        {
                                            RemoveNeighborTableEntry();
                                        }
                                        rc = NLME_LEAVE_confirm;
                                    }
                                    /*Leave procedure is complete. So, reset the variables so that next leave
                                    request can be processed.*/
                                    nwkStatus.flags.bits.bLeaveInProgress = 0;
                                    nwkStatus.flags.bits.bLeaveCmdSent = 0;
                                }
                            }
                        }
                    }
                    else
                    {
                        rc = NO_PRIMITIVE;
                    }

                    // Destroy the frame.
                    nwkConfirmationHandles[i].nsduHandle = INVALID_NWK_HANDLE;

                    return rc;
                }
                break;


            case MLME_SYNC_LOSS_indication:
                #ifdef I_SUPPORT_SECURITY
                rejoinWithSec = 1;
                #endif
                return NLME_SYNC_LOSS_indication;
            break;
            // ---------------------------------------------------------------------
            case MCPS_DATA_indication:
                {
                    NWK_FRAME_CONTROL_LSB   nwkFrameControlLSB;
                    NWK_FRAME_CONTROL_MSB   nwkFrameControlMSB;
                    SHORT_ADDR              nwkDestinationAddress;
                    SHORT_ADDR              nwkSourceAddress;
                    BYTE                    nwkRadius;
                    BYTE                    nwkSequenceNumber;
                    MULTICAST_FIELDS        nwkMulticast;
                    #ifdef I_SUPPORT_SECURITY
                        BYTE                    *nwkHeader = params.MCPS_DATA_indication.msdu;
                    #endif // I_SUPPORT_SECURITY
                    // Note: The macro I_SUPPORT_MANY_TO_ONE_HANDLING can be uncommented if we don't need
                    // processing of  source route sub frame on the end device and final destination router
                    // where source route  information is redundant
                    //#if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                        BYTE relayCount = 0;
                        BYTE relayIndex = 0;
                        BYTE *pRelaylist = NULL;
                   // #endif //  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                    LONG_ADDR               nwkIEEEDstAddress;
                    LONG_ADDR               nwkIEEESrcAddress;
                    BYTE                    nwkHeaderSize = 8;

                    // Extract the NWK header
                    nwkFrameControlLSB.Val         = NWKGet();
                    nwkFrameControlMSB.Val         = NWKGet();
                    messageToForward.frameCtrlLSB = nwkFrameControlLSB.Val;
                    messageToForward.frameCtrlMSB = nwkFrameControlMSB.Val;
                    if ( INTRP_FRAME_DATA == nwkFrameControlLSB.bits.frameType )
                    {
                        #if I_SUPPORT_STUB_APS == 1
                            return handleInterPanDataInd();
                        #endif

                        NWKDiscardRx();
                        return NO_PRIMITIVE;
                    }

                    nwkDestinationAddress.byte.LSB = NWKGet();
                    nwkDestinationAddress.byte.MSB = NWKGet();
                    nwkSourceAddress.byte.LSB      = NWKGet();
                    nwkSourceAddress.byte.MSB      = NWKGet();
                    nwkRadius                      = NWKGet();
                    nwkSequenceNumber              = NWKGet();

                    if( nwkFrameControlMSB.bits.multicastFlag )
                    {
                        nwkMulticast.Val = NWKGet();
                        nwkHeaderSize += 1;
                    }


                    // For ZigBee 2006
                    if( nwkFrameControlMSB.bits.dstIEEEAddr )
                    {
                        for(i = 0; i < 8; i++)
                        {
                            nwkIEEEDstAddress.v[i] = NWKGet();
                        }
                        nwkHeaderSize += 8;
                        // Not Sure whether Address Conflict needs to be detected for this address
                    }
                    if( nwkFrameControlMSB.bits.srcIEEEAddr )
                    {
                        for(i = 0; i < 8; i++)
                        {
                            nwkIEEESrcAddress.v[i] = NWKGet();
                        }
                        nwkHeaderSize += 8;
                        // Not Sure whether Address Conflict needs to be detected for this address
                    }
                    // note: The macro can be uncommented if we don't need processing of
                    // source route sub frame on the end device and final destination router  where source route
                    //information is redundant
                   // #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                        // Forward the source routed subframe
                        if( nwkFrameControlMSB.bits.sourceRoute )
                        {
                            relayCount = NWKGet();
                            relayIndex = NWKGet();

                            if(relayCount > 0)
                            {
                                /* 2 * relayCount because we need to get 2 byte short adress
                                for each relay count */
                                pRelaylist =  ( unsigned char *) SRAMalloc( 2 * relayCount);
                                if(pRelaylist != NULL )
                                {
                                    for(i = 0; i < 2 * relayCount; i++)
                                    {
                                        /* store the relay list */
                                        pRelaylist[i] = NWKGet();
                                    }
                                }
                                else
                                {
                                    NWKDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                            }
                            else
                            {
                                pRelaylist = NULL;
                            }
                            // 2 is added for relay count and realy index. relay count
                            // is multiplied by 2 because the address is 2 byte
                            nwkHeaderSize +=  relayCount * 2 + 2;
                        }
                  //  #endif  //  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                    #ifdef I_SUPPORT_SECURITY
                        // in network layer, all frames are secured by network key
                        if( nwkFrameControlMSB.Val & 0x02 )
                        {
                            if( !DataDecrypt(params.MCPS_DATA_indication.msdu, &params.MCPS_DATA_indication.msduLength, nwkHeader, nwkHeaderSize, ID_NetworkKey, NULL) )
                            {
                                NWKDiscardRx();
                                return NO_PRIMITIVE;
                            }
                            params.NLDE_DATA_indication.SecurityUse = 0x01;
                            params.NLDE_DATA_request.SecurityEnable = TRUE;
                        }

                        /* When the device is operating in the secured network, it should not
                        receive unsecured packet which is not meant for itself, because it should
                        not route the packet. Following code in the else condition is included to
                        support the above explained feature. */
                        else
                        {
                            
                            #if PROFILE_nwkSecureAllFrames == 0
                                params.NLDE_DATA_request.SecurityEnable = FALSE;
                                if (params.MCPS_DATA_indication.DstAddr.ShortAddr.Val == macPIB.macShortAddress.Val)
                                {
                                    params.NLDE_DATA_indication.SecurityUse = 0x00;
                                }
                                else
                                {
                                    NWKDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                            #else
                                params.NLDE_DATA_indication.SecurityUse = 0x00; //UNSECURED
                            #endif
                        }
                    #else
                        params.NLDE_DATA_indication.SecurityUse = 0x00; //UNSECURED
                    #endif // I_SUPPORT_SECURITY

                    //#ifndef I_AM_RFD
                        // See if this is a broadcast packet.
                        if ( NOW_I_AM_NOT_A_RFD() && (( params.MCPS_DATA_indication.DstAddr.ShortAddr.Val == 0xFFFF ) ||
                             ( nwkDestinationAddress.Val > 0xFFFB )) )
                        {
                            // This is a broadcast packet.  If this packet is the same as a
                            // packet that was already seen and is not a route request command frame,
                            // mark the appropriate BTR as having received the relay message, then discard it.
                            // Note that currently Route Request is the only
                            // broadcast command frame.  When there are more, device may have to
                            // peek at the first NWK payload byte.

                            // A ZigBee 2006 requirment:
                            if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
                                if (!ZigBeeStatus.flags.bits.bNetworkJoined)
                                {
                                    NWKDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                            }//#endif

                            if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                                if( nwkDestinationAddress.Val == 0xFFFD )
                                {
                                    NWKDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                            }//#endif // I_AM_RFD

                            #ifdef I_AM_END_DEVICE
                                if( nwkDestinationAddress.Val == 0xFFFC )
                                {
                                    NWKDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                            #endif // I_AM_END_DEVICE

                            for (i=0; i<NUM_BUFFERED_BROADCAST_MESSAGES; i++)
                            {
                                if (nwkStatus.BTT[i] != NULL)
                                {
                                    if ((nwkSourceAddress.Val == nwkStatus.BTT[i]->nwkSourceAddress.Val) &&
                                        (nwkSequenceNumber == nwkStatus.BTT[i]->nwkSequenceNumber))
                                    {

                                        MarkNeighborAsPasssiveACKed( i );
                                        NWKDiscardRx();
                                        return NO_PRIMITIVE;
                                    }
                                }
                            }


                            // This is a new broadcast packet or a broadcast NWK command.


                            // See if the packet is a routing command
                            if (nwkFrameControlLSB.bits.frameType == NWK_FRAME_CMD)
                            {
                                BYTE nwkCommand;
                                nwkCommand = NWKGet();

                                #ifdef I_SUPPORT_ROUTING
                                    // See if it a route request packet
                                    if ((nwkCommand == NWK_COMMAND_ROUTE_REQUEST) && nwkStatus.flags.bits.bCanRoute)
                                    {
                                        // A ZigBee 2006 requirement: End devices should not take part in Routing
                                        #ifdef I_AM_END_DEVICE //I_AM_RFD
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                        #else
                                            // If receiving a broadcast packet from another device for
                                            // our own route request, ignore it.
                                            if (nwkSourceAddress.Val == macPIB.macShortAddress.Val)
                                            {
                                                /*We have received a broadcasted packet from another device
                                                but was initialzed by me. So discard it.*/
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;
                                            }

HandleRouteRequest:
                                            {
                                                BOOL                    forwardRouteRequest;
                                                ROUTE_REQUEST_COMMAND   rreq;
                                                BYTE                    originalCost;
                                                BYTE                    commandOptions;

                                                forwardRouteRequest = FALSE;

                                                // Get the Route Request command frame
                                                #if defined(__C30__) || defined(__C32__)
                                                    rreq.commandFrameIdentifier = NWK_COMMAND_ROUTE_REQUEST;
                                                    commandOptions              = NWKGet();
                                                    rreq.commandOptions         = commandOptions;
                                                    rreq.routeRequestIdentifier = NWKGet();
                                                    rreq.destinationAddress.v[0]= NWKGet();
                                                    rreq.destinationAddress.v[1]= NWKGet();
                                                    rreq.pathCost               = NWKGet();
                                                    #if ( ZIGBEE_PRO == 0x01)
                                                        if (commandOptions & DEST_IEEE_ADDRESS_BIT)
                                                        {
                                                            for (i=0; i<8; i++)
                                                            {
                                                                rreq.destinationIEEEAddress.v[i] = NWKGet();
                                                            }
                                                        }
                                                    #endif // ( ZIGBEE_PRO == 0x01)
                                                #else
                                                    for ( ptr = (BYTE *)&rreq, *ptr++ = NWK_COMMAND_ROUTE_REQUEST, i = 1,
                                                          commandOptions = *ptr;
                                                          (i< sizeof(ROUTE_REQUEST_COMMAND));
                                                          *ptr++ = NWKGet(), i++ ) {}
                                                    #if ( ZIGBEE_PRO == 0x01)
                                                        if (commandOptions & DEST_IEEE_ADDRESS_BIT)
                                                        {
                                                            for (i=0; i<8; i++)
                                                            {
                                                                *ptr++ = NWKGet();
                                                            }
                                                        }
                                                    #endif // ( ZIGBEE_PRO == 0x01)
                                                #endif // defined(__C30__) || defined(__C32__)

                                                originalCost = rreq.pathCost;

                                                #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                                    // This condition is required because, we should not process any of the Route Request
                                                    // command frame, if we have received from a device for which the Neighbor Record entry
                                                    // does not exist.
                                                    if ( NWKLookupNodeByShortAddrVal( params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val ) != INVALID_NEIGHBOR_KEY )
                                                    {
                                                        // We need to validate that the device who broadcasted the Route Request command
                                                        // frame is active in my neighbor record. The neighbor record entry is active, if
                                                        // the OutgoingCost to this neighbor is NON ZERO. If it is ZERO, then the device,
                                                        // is not active. So, stop processing the Route Request command frame.
                                                        if ( currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost == 0x00 )
                                                        {
                                                            NWKDiscardRx();
                                                            return NO_PRIMITIVE;
                                                        }
                                                        else
                                                        {
                                                            // The Neighbor record is active. So, update the Pathcost field by adding the
                                                            // OutgoingCost to the Pathcost received in the Route Request command frame.
                                                            // We need not calculate the PathCost based on the link quality of the
                                                            // received Route Request command frame, because we always trust the Link Status
                                                            // command. By using this Link Status command, best symmetric links will be assued.
                                                            //rreq.pathCost += currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost;

                                                            if (currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost >= currentNeighborRecord.linkStatusInfo.flags.bits.bIncomingCost)
                                                            {
                                                                rreq.pathCost += currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost;
                                                            }
                                                            else
                                                            {
                                                                rreq.pathCost += currentNeighborRecord.linkStatusInfo.flags.bits.bIncomingCost;
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        // Neighbor record entry does not exists for the device who has broadcasted the
                                                        // Route Request command frame. So, stop processing it.
                                                        NWKDiscardRx();
                                                        return NO_PRIMITIVE;
                                                    }
                                                #else
                                                    // Since Link Status is disabled, we need to calculate the pathcost and update to
                                                    // the cost present in the received frame
                                                    rreq.pathCost += CalculateLinkQuality( params.MCPS_DATA_indication.mpduLinkQuality );
                                                #endif // ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

                                                #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                                    #if ( ZIGBEE_PRO == 0x01)
                                                        if( commandOptions & DEST_IEEE_ADDRESS_BIT )
                                                        {
                                                            if(IsThereAddressConflict( rreq.destinationAddress , &rreq.destinationIEEEAddress ))
                                                            {
                                                                // We have detected address conflict
                                                                return NLME_NWK_STATUS_indication;
                                                            }
                                                        }
                                                    #endif
                                                #endif // ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )

                                                #ifndef USE_TREE_ROUTING_ONLY
                                                    if (HaveRoutingCapacity( TRUE, rreq.routeRequestIdentifier, nwkSourceAddress,
                                                    rreq.destinationAddress, rreq.commandOptions ) &&
                                                        (params.MCPS_DATA_indication.DstAddr.ShortAddr.Val == 0xFFFF))
                                                    {
                                                        BYTE    rdIndex;
                                                        BYTE    rtIndex;

                                                        // Try to find the matching route discovery entry.
                                                        for (rdIndex = 0;
                                                             (rdIndex < ROUTE_DISCOVERY_TABLE_SIZE) &&
                                                              !((routeDiscoveryTablePointer[rdIndex] != NULL) &&
                                                              (routeDiscoveryTablePointer[rdIndex]->routeRequestID == rreq.routeRequestIdentifier) &&
                                                              (routeDiscoveryTablePointer[rdIndex]->srcAddress.Val == nwkSourceAddress.Val));
                                                             rdIndex++ ) {}

                                                        if (rdIndex < ROUTE_DISCOVERY_TABLE_SIZE)
                                                        {

                                                            // Device is in the process of discovering this route.  Update the time stamp, in case
                                                            // two nodes are trying to discover the same route, so as not to time out too soon
                                                            // on one of them.
                                                            routeDiscoveryTablePointer[rdIndex]->timeStamp = TickGet();

                                                            // See if this new path is better than the old one.
                                                            // NOTE: we have to go through this branch if the path cost equals
                                                            // our current cost, in case another node is attempting a route
                                                            // discovery of the same node.
                                                            if (rreq.pathCost < routeDiscoveryTablePointer[rdIndex]->forwardCost)
                                                            {
                                                                // Update Routing Tables with the route
                                                                routeDiscoveryTablePointer[rdIndex]->senderAddress = params.MCPS_DATA_indication.SrcAddr.ShortAddr;
                                                                routeDiscoveryTablePointer[rdIndex]->forwardCost = rreq.pathCost;

                                                                #if ( I_SUPPORT_SYMMETRIC_LINK == 0x01 )
                                                                    if ((rreq.pathCost - originalCost) < routeDiscoveryTablePointer[rdIndex]->previousCost)
                                                                    {
                                                                        if ( FindFreeRoutingTableEntry(nwkSourceAddress, FALSE) != 0xFF )
                                                                        {
                                                                            if (currentRoutingEntry.destAddress.Val == nwkSourceAddress.Val)
                                                                            {
                                                                                currentRoutingEntry.nextHop.Val = params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val;
                                                                                PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
                                                                            }
                                                                        }
                                                                    }
                                                                #endif // #if ( ( I_SUPPORT_SYMMETRIC_LINK == 0x01 )
                                                                routeDiscoveryTablePointer[rdIndex]->previousCost = rreq.pathCost - originalCost;

                                                                #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                                                    CheckAndUpdateRouteTableWithConcentratorDetails
                                                                    (
                                                                         &rreq,
                                                                         routeDiscoveryTablePointer[rdIndex]
                                                                    );
                                                                #endif //  I_SUPPORT_MANY_TO_ONE_HANDLING == 1

                                                                if (commandOptions & MULTICAST_BIT)
                                                                {
                                                                    #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                                                        // Check I am part of the Group
                                                                        if( ( GetEndPointsFromGroup(rreq.destinationAddress) == MAX_GROUP ) )
                                                                        {
                                                                            // Pass on the route request.
                                                                            forwardRouteRequest = TRUE;
                                                                        }
                                                                        else
                                                                        {
                                                                            // Destroy the current packet and start the Route Reply
                                                                            NWKDiscardRx();
                                                                            if (ZigBeeReady())
                                                                            {
                                                                                rreq.pathCost = routeDiscoveryTablePointer[rdIndex]->previousCost;
                                                                                CreateRouteReply( nwkSourceAddress, rdIndex, &rreq );
                                                                                return MCPS_DATA_request;
                                                                            }
                                                                            else
                                                                            {
                                                                                return NO_PRIMITIVE;
                                                                            }
                                                                        }
                                                                    #else
                                                                        // Pass on the route request.
                                                                        forwardRouteRequest = TRUE;
                                                                    #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                                                }
                                                                // See if I am or one of my end devices is the destination of the route request.
                                                                // Otherwise, update the path cost and forward the route request.
                                                                else if ((macPIB.macShortAddress.Val == rreq.destinationAddress.Val) ||
                                                                         ((NWKLookupNodeByShortAddrVal( rreq.destinationAddress.Val ) != INVALID_NEIGHBOR_KEY) &&
                                                                          (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD) &&
                                                                          (currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE)))
                                                                {
                                                                    // Destroy the current packet and start the Route Reply
                                                                    NWKDiscardRx();
                                                                    if (ZigBeeReady())
                                                                    {
                                                                        rreq.pathCost = routeDiscoveryTablePointer[rdIndex]->previousCost;
                                                                        CreateRouteReply( nwkSourceAddress, rdIndex, &rreq );
                                                                        return MCPS_DATA_request;
                                                                    }
                                                                    else
                                                                    {
                                                                        return NO_PRIMITIVE;
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    // Pass on the route request.
                                                                    forwardRouteRequest = TRUE;
                                                                }
                                                            }
                                                            else
                                                            {

                                                                // This path is worse than the one we currently have, so ignore it.
                                                                NWKDiscardRx();
                                                                return NO_PRIMITIVE;
                                                            }
                                                        }
                                                        else // Route Discovery entry does not exist
                                                        {
                                                            SHORT_ADDR rreq_target_addr;
                                                            BOOL isConcentrator;
                                                            rreq_target_addr = rreq.destinationAddress;
                                                            isConcentrator =  (( rreq.commandOptions & HIGH_CONC_MANY_TO_ONE ) ||
                                                                               ( rreq.commandOptions & LOW_CONC_MANY_TO_ONE ));
															printf("Creat Route Table 1\n\r");
                                                            if (
                                                                    #if ( ( I_SUPPORT_SYMMETRIC_LINK == 0x01 ) || (  I_SUPPORT_MANY_TO_ONE_HANDLING == 1 ) )
                                                                        !CreateRoutingTableEntries
                                                                        (
                                                                            rreq_target_addr,
                                                                            &rdIndex,
                                                                            &rtIndex,
                                                                            nwkSourceAddress, // rreq initiator
                                                                            params.MCPS_DATA_indication.SrcAddr.ShortAddr,// next hop,
                                                                            isConcentrator
                                                                        )
                                                                    #else
																		
                                                                        !CreateRoutingTableEntries
                                                                        (
                                                                            rreq_target_addr,
                                                                            &rdIndex,
                                                                            &rtIndex
                                                                        )
                                                                    #endif // ( ( I_SUPPORT_SYMMETRIC_LINK == 0x01 ) || (  I_SUPPORT_MANY_TO_ONE_HANDLING == 1 ) )
                                                               )
                                                            {
                                                                NWKDiscardRx();
                                                                return NO_PRIMITIVE;
                                                            }

                                                            if (commandOptions & MULTICAST_BIT)
                                                            {
                                                                if ( FindFreeRoutingTableEntry(rreq_target_addr, FALSE) != 0xFF )
                                                                {
                                                                    if (currentRoutingEntry.destAddress.Val == rreq_target_addr.Val)
                                                                    {
                                                                        // Update the bGroupIDFlag as the route is being discovered for the group.
                                                                        currentRoutingEntry.rtflags.rtbits.bGroupIDFlag = 1;
                                                                        PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
                                                                    }
                                                                }
                                                            }

                                                            // Populate the remainder of the Route Discovery Table
                                                            routeDiscoveryTablePointer[rdIndex]->routeRequestID             = rreq.routeRequestIdentifier;
                                                            routeDiscoveryTablePointer[rdIndex]->srcAddress                 = nwkSourceAddress;
                                                            routeDiscoveryTablePointer[rdIndex]->senderAddress              = params.MCPS_DATA_indication.SrcAddr.ShortAddr;
                                                            routeDiscoveryTablePointer[rdIndex]->forwardCost                = rreq.pathCost;
                                                            routeDiscoveryTablePointer[rdIndex]->timeStamp                  = TickGet();

                                                            routeDiscoveryTablePointer[rdIndex]->status.transmitCounter  = nwkcRREQRetries + 1;

                                                            routeDiscoveryTablePointer[rdIndex]->status.initialRebroadcast  = 1;
                                                            routeDiscoveryTablePointer[rdIndex]->previousCost = rreq.pathCost - originalCost;

                                                            /*Mark whether the device is an intiator or intermediate.
                                                            This information is used to retain the status of Routing table entry as ACTIVE.*/
                                                            routeDiscoveryTablePointer[rdIndex]->status.initiator = 0x00; //We are not the intiator
                                                            // update the routing table in case we got a many to one request
                                                            #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                                                CheckAndUpdateRouteTableWithConcentratorDetails
                                                                (
                                                                     &rreq,
                                                                     routeDiscoveryTablePointer[rdIndex]
                                                                );
                                                            #endif   //  I_SUPPORT_MANY_TO_ONE_HANDLING == 1

                                                            if (commandOptions & MULTICAST_BIT)
                                                            {
                                                                #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                                                    // Check I am part of the Group
                                                                    if( ( GetEndPointsFromGroup(rreq.destinationAddress) == MAX_GROUP ) )
                                                                    {
                                                                        // Pass on the route request.
                                                                        forwardRouteRequest = TRUE;
                                                                    }
                                                                    else
                                                                    {
                                                                        // Destroy the current packet and start the Route Reply
                                                                        NWKDiscardRx();
                                                                        if (ZigBeeReady())
                                                                        {
                                                                            rreq.pathCost = routeDiscoveryTablePointer[rdIndex]->previousCost;
                                                                            CreateRouteReply( nwkSourceAddress, rdIndex, &rreq );
                                                                            return MCPS_DATA_request;
                                                                        }
                                                                        else
                                                                        {
																			nwkStatus.flags.bits.bAwaitingRouteDiscovery = 0;
																			nfree( routeDiscoveryTablePointer[rdIndex] );
				
																			// Destroy the buffered message.
																			SRAMfree( nwkStatus.routingMessages[rdIndex]->dataPtr );
																			nfree( nwkStatus.routingMessages[rdIndex] );
                                                                            return NO_PRIMITIVE;
                                                                        }
                                                                    }
                                                                #else
                                                                    // Pass on the route request.
                                                                    forwardRouteRequest = TRUE;
                                                                #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                                            }

                                                            else if ((macPIB.macShortAddress.Val == rreq.destinationAddress.Val) ||
                                                                     ((NWKLookupNodeByShortAddrVal( rreq.destinationAddress.Val ) != INVALID_NEIGHBOR_KEY) &&
                                                                      (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD) &&
                                                                      (currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE)))
                                                            {
                                                                // Destroy the current packet and start the Route Reply
                                                                NWKDiscardRx();
                                                                if (ZigBeeReady())
                                                                {
                                                                    rreq.pathCost -= originalCost;
                                                                    CreateRouteReply( nwkSourceAddress, rdIndex, &rreq );
                                                                    return MCPS_DATA_request;
                                                                }
                                                                else
                                                                {
																	nwkStatus.flags.bits.bAwaitingRouteDiscovery = 0;
																	nfree( routeDiscoveryTablePointer[rdIndex] );

																	// Destroy the buffered message.
																	SRAMfree( nwkStatus.routingMessages[rdIndex]->dataPtr );
																	nfree( nwkStatus.routingMessages[rdIndex] );
																	printf("Zigbee is not ready\n\r");
                                                                    return NO_PRIMITIVE;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                 // Pass on the route request.
                                                                forwardRouteRequest = TRUE;
                                                                // Update the Routing Table in case we received Many_To_One

                                                            }
                                                        }
                                                        if (forwardRouteRequest && ZigBeeReady())
                                                        {
                                                            if ( !( nwkRadius > 1 ) )
                                                            {
																nwkStatus.flags.bits.bAwaitingRouteDiscovery = 0;
																nfree( routeDiscoveryTablePointer[rdIndex] );

																// Destroy the buffered message.
																SRAMfree( nwkStatus.routingMessages[rdIndex]->dataPtr );
																nfree( nwkStatus.routingMessages[rdIndex] );
                                                                // If Radius is not greater than one, then
                                                                // we should not rebroadcast the RREQ frame.
                                                                NWKDiscardRx();
                                                                return NO_PRIMITIVE;
                                                            }

                                                            // Either this is a new route request we are forwarding, or we have a better path cost.
                                                            // Buffer the RREQ so we can send it after the jitter has timed out.
                                                            // If it's a new RREQ, allocate a new block of memory to buffer the message.  If it's
                                                            // an existing one, overwrite the pervious information.
                                                            if (!routeDiscoveryTablePointer[rdIndex]->forwardRREQ)
                                                            {
                                                                    // sizeof_ROUTE_REQUEST_COMMAND + sizeof_ROUTE_REQUEST_COMMAND_HEADER = 6 + 16 = 22Bytes
                                                                    routeDiscoveryTablePointer[rdIndex]->forwardRREQ = SRAMalloc(sizeof_ROUTE_REQUEST_COMMAND +
                                                                                                                              #if ( ZIGBEE_PRO == 0x01)
                                                                                                                                 sizeof_RREQ_COMMAND_DST_IEEE_ADDR +
                                                                                                                              #endif // ( ZIGBEE_PRO == 0x01)
                                                                                                                                 sizeof_ROUTE_REQUEST_COMMAND_HEADER);
                                                            }
                                                            if (routeDiscoveryTablePointer[rdIndex]->forwardRREQ != NULL)
                                                            {
                                                                BYTE *ptr2;
                                                                BYTE commandOptions;
                                                                // Load the RREQ information.
                                                                ptr = (BYTE *)&rreq;
                                                                ptr2 = routeDiscoveryTablePointer[rdIndex]->forwardRREQ;
                                                                #if defined(__C30__) || defined(__C32__)
                                                                    *ptr2++ = rreq.commandFrameIdentifier;
                                                                    commandOptions = rreq.commandOptions;
                                                                    *ptr2++ = rreq.commandOptions;
                                                                    *ptr2++ = rreq.routeRequestIdentifier;
                                                                    *ptr2++ = rreq.destinationAddress.v[0];
                                                                    *ptr2++ = rreq.destinationAddress.v[1];
                                                                    *ptr2++ = rreq.pathCost;
                                                                    #if ( ZIGBEE_PRO == 0x01)
                                                                        if (commandOptions & DEST_IEEE_ADDRESS_BIT)
                                                                        {
                                                                            for (i=0; i<8; i++)
                                                                            {
                                                                                *ptr2++ = rreq.destinationIEEEAddress.v[i];
                                                                            }
                                                                        }
                                                                    #endif // ( ZIGBEE_PRO == 0x01)
                                                                #else
                                                                    for (i=0; i<sizeof(ROUTE_REQUEST_COMMAND); i++)
                                                                    {
                                                                        #if ( ZIGBEE_PRO == 0x01)
                                                                            if ( i == 1 )
                                                                            {
                                                                                commandOptions = *ptr;
                                                                            }
                                                                        #endif
                                                                        *ptr2++ = *ptr++;       // Command, options, ID, dest addr LSB, dest addr MSB, path cost
                                                                    }
                                                                    #if ( ZIGBEE_PRO == 0x01)
                                                                        if (commandOptions & DEST_IEEE_ADDRESS_BIT)
                                                                        {
                                                                            for (i=0; i<8; i++)
                                                                            {
                                                                                *ptr2++ = *ptr++;
                                                                            }
                                                                        }
                                                                    #endif // ( ZIGBEE_PRO == 0x01)
                                                                #endif // defined(__C30__) || defined(__C32__)

                                                                // Load up the old NWK header (backwards).
                                                                // Add the SrcIEEEAddress field received in the frame
                                                                // in the NWK Header
                                                                for(i = 0; i < 8; i++)
                                                                {
                                                                    *ptr2++ = nwkIEEESrcAddress.v[7-i];
                                                                }

                                                                *ptr2++ = nwkSequenceNumber; //nwkSequenceNumber;
                                                                *ptr2++ = nwkRadius - 1;
                                                                *ptr2++ = nwkSourceAddress.byte.MSB;
                                                                *ptr2++ = nwkSourceAddress.byte.LSB;
                                                                *ptr2++ = nwkDestinationAddress.byte.MSB;
                                                                *ptr2++ = nwkDestinationAddress.byte.LSB;
                                                                #ifdef I_SUPPORT_SECURITY
                                                                    nwkFrameControlMSB.Val |= 0x02 | NWK_IEEE_SRC_ADDR;
                                                                #else
                                                                    nwkFrameControlMSB.Val |= 0x00 | NWK_IEEE_SRC_ADDR;
                                                                #endif // I_SUPPORT_SECURITY
                                                                *ptr2++ = nwkFrameControlMSB.Val;
                                                                *ptr2++ = nwkFrameControlLSB.Val;

                                                                // Reset the rebroadcast timer and the transmit counter
                                                                routeDiscoveryTablePointer[rdIndex]->rebroadcastTimer       = TickGet();
                                                                //routeDiscoveryTablePointer[rdIndex]->status.transmitCounter = nwkcInitialRREQRetries + 1;
                                                            }
                                                            NWKDiscardRx();
                                                            return NO_PRIMITIVE;
                                                        }
                                                    }
                                                    else // Do not have route capacity
                                                #endif // #ifndef USE_TREE_ROUTING_ONLY
                                                    {
                                                        #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
                                                            #ifndef I_AM_END_DEVICE
                                                                // See if we got the message along a valid path.  The path is valid if
                                                                // if was received from one of the device’s child devices and the source
                                                                // device is a descendant of that child device (or that child device itself),
                                                                // or if the frame was received from the device’s parent device and the
                                                                // source device is not my descendant.
                                                                if (((NWKLookupNodeByShortAddrVal( params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val ) != INVALID_NEIGHBOR_KEY) &&
                                                                         (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD) &&
                                                                         ((params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val == nwkSourceAddress.Val) ||
                                                                          IsDescendant( params.MCPS_DATA_indication.SrcAddr.ShortAddr, nwkSourceAddress, currentNeighborTableInfo.depth+1 ))) ||
                                                                     ((params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val == macPIB.macCoordShortAddress.Val) &&
                                                                        !IsDescendant( macPIB.macShortAddress, nwkSourceAddress, currentNeighborTableInfo.depth )))
                                                                {
                                                                    // See if I am or one of my end devices is the destination of the route request.
                                                                    // Otherwise, unicast the route request.
                                                                    if ((macPIB.macShortAddress.Val == rreq.destinationAddress.Val) ||
                                                                        ((NWKLookupNodeByShortAddrVal( rreq.destinationAddress.Val ) != INVALID_NEIGHBOR_KEY) &&
                                                                         (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD) &&
                                                                         (currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE)))
                                                                    {
                                                                        // Destroy the current packet and start the Route Reply
                                                                        NWKDiscardRx();
                                                                        if (ZigBeeReady())
                                                                        {
                                                                            rreq.pathCost -= originalCost;
                                                                            CreateRouteReply( nwkSourceAddress, INVALID_ROUTE_DISCOVERY_INDEX, &rreq );
                                                                            return MCPS_DATA_request;
                                                                        }
                                                                        else
                                                                        {
                                                                            return NO_PRIMITIVE;
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        // Unicast the Route Request
                                                                        if (RouteAlongTree( rreq.destinationAddress, &(macAddress) ) && ZigBeeReady())
                                                                        {
                                                                            // Prepare the Route Request message.
                                                                            // Load up the NWK payload - the route request command frame.
                                                                            ptr = (BYTE *)&rreq;
                                                                            #if defined(__C30__) || defined(__C32__)
                                                                                TxBuffer[TxData++] = rreq.commandFrameIdentifier;       // Command
                                                                                TxBuffer[TxData++] = rreq.commandOptions;       // options
                                                                                TxBuffer[TxData++] = rreq.routeRequestIdentifier;       // ID
                                                                                TxBuffer[TxData++] = rreq.destinationAddress.byte.LSB;       // destination address LSB
                                                                                TxBuffer[TxData++] = rreq.destinationAddress.byte.MSB;       // destination address MSB
                                                                                TxBuffer[TxData++] = rreq.pathCost;       // path cost
                                                                            #else
                                                                                for (i=0; i<sizeof(ROUTE_REQUEST_COMMAND); i++)
                                                                                {
                                                                                    TxBuffer[TxData++] = *ptr++;       // Command, options, ID, dest addr LSB, dest addr MSB, path cost
                                                                                }
                                                                            #endif // #if defined(__C30__) || defined(__C32__)
                                                                            /*Add the SrcIEEEAddress field received in the frame
                                                                            in the NWK Header*/
                                                                            for(i = 0; i < 8; i++)
                                                                            {
                                                                                TxBuffer[TxHeader--] = nwkIEEESrcAddress.v[7-i];
                                                                            }

                                                                            TxBuffer[TxHeader--] = nwkSequenceNumber;
                                                                            TxBuffer[TxHeader--] = nwkRadius - 1;
                                                                            TxBuffer[TxHeader--] = nwkSourceAddress.byte.MSB;
                                                                            TxBuffer[TxHeader--] = nwkSourceAddress.byte.LSB;
                                                                            TxBuffer[TxHeader--] = nwkDestinationAddress.byte.MSB;
                                                                            TxBuffer[TxHeader--] = nwkDestinationAddress.byte.LSB;

                                                                            #ifdef I_SUPPORT_SECURITY
                                                                                nwkFrameControlMSB.Val |= 0x02 | NWK_IEEE_SRC_ADDR;
                                                                            #else
                                                                                nwkFrameControlMSB.Val |= 0x00 | NWK_IEEE_SRC_ADDR;
                                                                            #endif
                                                                            TxBuffer[TxHeader--] = nwkFrameControlMSB.Val;
                                                                            TxBuffer[TxHeader--] = nwkFrameControlLSB.Val;

                                                                            // Load up the MCPS_DATA.request parameters.
                                                                            Prepare_MCPS_DATA_request( macAddress.Val, &i );
                                                                        }
                                                                        NWKDiscardRx();
                                                                        return MCPS_DATA_request;
                                                                    }
                                                                } // did not receive along the valid path. So, ignore it.
                                                            #endif // I_AM_END_DEVICE
                                                        #endif // (USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
                                                    }
                                            }
                                            // We cannot process the Route Request command frame, so ignore it.
                                            // Reason may be beacause of,
                                            // 1. Not received along the valid path
                                            // 2. Not support of TREE ROUTING
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;

                                        #endif // #ifdef I_AM_END_DEVICE
                                    }
                                    // There are no other broadcast network commands.  Plus catch anywhere else
                                    // that we didn't discard the packet.
                                #endif // I_SUPPORT_ROUTING

                                #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                    #ifndef I_AM_END_DEVICE
                                        //Required for ZigBeePro
                                        if( nwkCommand == NWK_COMMAND_NWK_STATUS )
                                        {
                                            BYTE nwkStatusCode;
                                            SHORT_ADDR nwkStatusDestAddr;

                                            nwkStatusCode = NWKGet();
                                            nwkStatusDestAddr.byte.LSB =  NWKGet();
                                            nwkStatusDestAddr.byte.MSB =  NWKGet();

                                            if ( nwkStatusCode == NWK_STATUS_ADDRESS_CONFLICT )
                                            {
                                                BTR *pCurrent_BTR;
                                                for (i=0; i<NUM_BUFFERED_BROADCAST_MESSAGES; i++)
                                                {
                                        /* checking if I have too detected the same conflict and waiting to initiate the NWK_COMMAND_NWK_STATUS BC.
                                           If yes, I will drop my packet from BTT and continue with packet I just received */

                                                    pCurrent_BTR = nwkStatus.BTT[i];
                                                    if (pCurrent_BTR != NULL)
                                                    {
                                                        if ( ( pCurrent_BTR->dataPtr[0] == NWK_COMMAND_NWK_STATUS ) &&
                                                        ( pCurrent_BTR->dataPtr[1] == NWK_STATUS_ADDRESS_CONFLICT ) &&
                                                        ( pCurrent_BTR->dataPtr[2] == nwkStatusDestAddr.byte.LSB ) &&
                                                        ( pCurrent_BTR->dataPtr[3] == nwkStatusDestAddr.byte.MSB )
                                                        )
                                                        {
                                                            SRAMfree( pCurrent_BTR->dataPtr );
                                                            nfree( nwkStatus.BTT[i] );
                                                        }
                                                    }
                                                }
                                                if( nwkStatusDestAddr.Val == macPIB.macShortAddress.Val )
                                                {
                                /* If I received this NWK_COMMAND_NWK_STATUS with my own short addr as its payload
                                   I shall get my short address changed */
                                                    if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
                                                        params.NLME_NWK_STATUS_indication.nwkStatusCode = nwkStatusCode;
                                                        params.NLME_NWK_STATUS_indication.ShortAddress = nwkStatusDestAddr;
                                                        nwkStatus.addressConflictType = LOCAL_ADDRESS_CONFLICT;
                                                        return NLME_NWK_STATUS_indication;
                                                    }//#endif // I_AM_COORDINATOR
                                                }
                                                else if ( NWKLookupNodeByRxOffDeviceShortAddrVal( nwkStatusDestAddr.Val ) != INVALID_NEIGHBOR_KEY )
                                                {
                                /* If I received this NWK_COMMAND_NWK_STATUS with one of my child end device's short addr as its payload
                                   I shall get its address changed */
                                                    params.NLME_NWK_STATUS_indication.nwkStatusCode = nwkStatusCode;
                                                    params.NLME_NWK_STATUS_indication.ShortAddress = nwkStatusDestAddr;
                                                    nwkStatus.addressConflictType = ADDRESS_CONFLICT_ON_A_CHILD_END_DEVICE;
                                                    return NLME_NWK_STATUS_indication;
                                                }
                                            }
                                            if (nwkRadius > 1)
                                            {
                                                BYTE BTTIndex , *pData;
                                                BTR *pCurrent_BTR;

                                                if (CreateNewBTR( &BTTIndex ))
                                                {
                                                    pCurrent_BTR = nwkStatus.BTT[BTTIndex];

                                                    // Indicate that the message did not originate from our upper layers.
                                                    pCurrent_BTR->btrInfo.bMessageFromUpperLayers = 0;

                                                    // Mark the originator as having passive ACK'd
                                                    MarkNeighborAsPasssiveACKed( BTTIndex );

                                                    // Save off the NWK header information
                                                    pCurrent_BTR->dataLength                   = 0x04;
                                                    pCurrent_BTR->nwkFrameControlLSB           = nwkFrameControlLSB;
                                                    pCurrent_BTR->nwkFrameControlMSB           = nwkFrameControlMSB;
                                                    pCurrent_BTR->nwkDestinationAddress.Val    = nwkDestinationAddress.Val;
                                                    pCurrent_BTR->nwkSourceAddress.Val         = nwkSourceAddress.Val;
                                                    pCurrent_BTR->nwkRadius                    = nwkRadius - 1;
                                                    pCurrent_BTR->nwkSequenceNumber            = nwkSequenceNumber;
                                                    pCurrent_BTR->nwkIEEESrcAddress            = nwkIEEESrcAddress;

                                                    // Save off the NWK payload
                                                    if ((pCurrent_BTR->dataPtr = (BYTE *)SRAMalloc( 0x04 )) == NULL)
                                                    {
                                                        // We do not have room to store this broadcast packet.  Destroy the BTR
                                                        // and ignore the packet.
                                                        nfree( nwkStatus.BTT[i] );

                                                        // NWK errata - if we cannot rebroadcast the packet, then do not send up to higher layers.  We'll
                                                        // see it again when the source doesn't see our rebroadcast and sends it again.
                                                        NWKDiscardRx();
                                                        return NO_PRIMITIVE;
                                                    }
                                                    else
                                                    {
                                                        pData = pCurrent_BTR->dataPtr;
                                                        *pData++ = NWK_COMMAND_NWK_STATUS;
                                                        *pData++ = nwkStatusCode;
                                                        *pData++ = nwkStatusDestAddr.byte.LSB;
                                                        *pData++ = nwkStatusDestAddr.byte.MSB;

                                                        // Set the network status so we can begin transmitting these messages
                                                        // in the background.  We have to process the message ourselves first!
                                                        nwkStatus.flags.bits.bSendingBroadcastMessage = 1;
                                                    }
                                                }
                                                else
                                                {
                                                    // NWK errata - if we cannot rebroadcast the packet, then do not send up to higher layers.
                                                    NWKDiscardRx();
                                                   return NO_PRIMITIVE;

                                                }
                                            }

                                        }
                                    #endif // I_AM_END_DEVICE
                                #endif // ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )

                                // A ZigBee 2006 requirement: handle leave indication from other devices
                                if( nwkCommand == NWK_COMMAND_LEAVE )
                                {
                                    BYTE leaveOption;

                                    leaveOption = NWKGet();
                                    // If device is receiving a broadcast packet from another device, but originated it, ignore it.
                                    // current device still has to rebroadcast it for another device's ACK.
                                    if (nwkSourceAddress.Val == macPIB.macShortAddress.Val)
                                    {
                                        NWKDiscardRx();
                                        return NO_PRIMITIVE;
                                    }

                                    if (nwkRadius > 1)
                                    {
                                        BYTE BTTIndex;
                                        BYTE    dataLength;

                                        if (CreateNewBTR( &BTTIndex ))
                                        {
                                            // There is room to store the broadcast packet.  Indicate that the message did not
                                            // originate from our upper layers.
                                            nwkStatus.BTT[BTTIndex]->btrInfo.bMessageFromUpperLayers = 0;

                                            // Mark the originator as having passive ACK'd
                                            MarkNeighborAsPasssiveACKed( BTTIndex );

                                            // Save off the NWK header information
                                            nwkStatus.BTT[BTTIndex]->dataLength                   = params.MCPS_DATA_indication.msduLength;
                                            nwkStatus.BTT[BTTIndex]->nwkFrameControlLSB           = nwkFrameControlLSB;
                                            nwkStatus.BTT[BTTIndex]->nwkFrameControlMSB           = nwkFrameControlMSB;
                                            nwkStatus.BTT[BTTIndex]->nwkDestinationAddress.Val    = nwkDestinationAddress.Val;
                                            nwkStatus.BTT[BTTIndex]->nwkSourceAddress.Val         = nwkSourceAddress.Val;
                                            nwkStatus.BTT[BTTIndex]->nwkRadius                    = nwkRadius - 1;
                                            nwkStatus.BTT[BTTIndex]->nwkSequenceNumber            = nwkSequenceNumber;

                                            // Save off the NWK payload
                                            if ((nwkStatus.BTT[BTTIndex]->dataPtr = (BYTE *)SRAMalloc( params.MCPS_DATA_indication.msduLength )) == NULL)
                                            {
                                                // We do not have room to store this broadcast packet.  Destroy the BTR
                                                // and ignore the packet.
                                                nfree( nwkStatus.BTT[BTTIndex] );

                                                // NWK errata - if we cannot rebroadcast the packet, then do not send up to higher layers.  We'll
                                                // see it again when the source doesn't see our rebroadcast and sends it again.
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;
                                            }
                                            else
                                            {
                                                i = 0;
                                                nwkStatus.BTT[BTTIndex]->dataPtr[i++] = NWK_COMMAND_LEAVE;
                                                ptr = params.MCPS_DATA_indication.msdu;
                                                dataLength = params.MCPS_DATA_indication.msduLength;
                                                while (params.MCPS_DATA_indication.msduLength)
                                                {
                                                    nwkStatus.BTT[BTTIndex]->dataPtr[i++] = NWKGet();
                                                }

                                                // Set the network status so we can begin transmitting these messages
                                                // in the background.  We have to process the message ourselves first!
                                                nwkStatus.flags.bits.bSendingBroadcastMessage = 1;
                                            }
                                        }
                                        else
                                        {
                                            // NWK errata - if we cannot rebroadcast the packet, then do not send up to higher layers.
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                        }
                                    }
                                    NWKDiscardRx();
                                    /*Update the Leave Indication structure*/
                                    memcpy( (void *)&(params.NLME_LEAVE_indication.DeviceAddress),
                                        (void *)&(nwkIEEESrcAddress), 8 );
                                    params.NLME_LEAVE_indication.Rejoin = (leaveOption & NWK_LEAVE_REJOIN) ? 1 : 0;

                                    /*Check whether leave command frame is received by one of the child or
                                    neighbor Routers. If yes, the remove the Neighbor table corresponding
                                    to that Router. Also remove the addressMap entry for that Router, if exists.*/
                                    i = NWKLookupNodeByShortAddrVal( nwkSourceAddress.Val );
                                    if (i != INVALID_NEIGHBOR_KEY)
                                    {
                                        if(currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD)
                                        {
                                            params.NLME_LEAVE_indication.isChild = TRUE;
                                            #if defined(I_SUPPORT_SECURITY)
                                                leavingChildDetails.DeviceShortAddress.Val = nwkSourceAddress.Val;
                                                memcpy(leavingChildDetails.DeviceLongAddress.v, nwkIEEESrcAddress.v, 8);
                                                zdoStatus.flags.bits.SendUpdateDeviceRequestAfterChildLeave = 1;
                                            #endif // #if defined(I_SUPPORT_SECURITY)
                                        }
                                        else
                                        {
                                            params.NLME_LEAVE_indication.isChild = FALSE;
                                        }
                                        params.NLME_LEAVE_indication.DeviceShortAddress.Val =
                                            nwkSourceAddress.Val ;
                                        RemoveNeighborTableEntry();
                                        // Update the DeviceAddress parameter in the Leave Indication structure
                                        memcpy( (void *)&(params.NLME_LEAVE_indication.DeviceAddress),
                                            (void *)&(nwkIEEESrcAddress), 8 );
                                        /* A Zigbee 2006 requirement - ZCP Tests requires that the APS table be cleanned up as well */
                                        for( i = 0; i < apscMaxAddrMapEntries; i++)
                                        {
                                            #ifdef USE_EXTERNAL_NVM
                                                GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                                            #else
                                                GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                                            #endif // USE_EXTERNAL_NVM
                                            if (currentAPSAddress.shortAddr.Val == nwkSourceAddress.Val)
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
                                                #endif // USE_EXTERNAL_NVM

                                                break;
                                            }
                                        }
                                    }
                                    return NLME_LEAVE_indication;
                                }

                                #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                    if ( nwkCommand == NWK_COMMAND_LINK_STATUS )
                                    {
                                        BYTE                linkStatusCmdOptions;
                                        SHORT_ADDR          nwkAddr;
                                        LINK_STATUS_INFO    linkStatus;
                                        BYTE        i;
                                        BYTE                nextPrimitive;
    		                            BOOL                thisIsANewneighbor = FALSE;
    		                            BYTE                previousInComingCost, previousOutGoingCost;
    		                            BYTE                indexInTable;
                                        // Address conflict detection shall be done for Link Status Command frames
                                        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                            if( nwkFrameControlMSB.bits.srcIEEEAddr )
                                            {
                                                if( IsThereAddressConflict( nwkSourceAddress , &nwkIEEESrcAddress ) )
                                                {
                                                    if ( nwkStatus.addressConflictType == LOCAL_ADDRESS_CONFLICT )
                                                    {
                                                        if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
                                                            nextPrimitive = ResolveLocalAddressConflict();
                                                        }//#endif
                                                    }
                                                    else if ( nwkStatus.addressConflictType == ADDRESS_CONFLICT_ON_A_CHILD_END_DEVICE )
                                                    {
                                                        addressConflictOnChildEndDevice.Val = nwkSourceAddress.Val;
                                                        zdoStatus.flags.bits.bSendUnsolicitedRejoinRsp = TRUE;
                                                    }
                                                    else if ( nwkStatus.addressConflictType == REMOTE_ADDRESS_CONFLICT )
                                                    {
                                                    }
                                                    nextPrimitive = PrepareNwkStatusCmdBroadcast( NWK_STATUS_ADDRESS_CONFLICT , nwkSourceAddress );
                                                    nwkStatus.addressConflictType = NO_CONFLICT;
                                                    NWKDiscardRx();
                                                    return nextPrimitive;
                                                }
                                            }
                                        #endif // #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )

                                        #if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
                                            for ( i=0; i<SuppressLinkStatusAddrListSize; i++ )
                                            {
                                                if ( SuppressLinkStatusAddrList[i] == nwkSourceAddress.Val )
                                                {
                                                    NWKDiscardRx();
                                                    return NO_PRIMITIVE;
                                                }
                                            }
                                        #endif //#if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )

                                        // Check whether we received a Link Status command from one of our Neighbor Device
    		                            /* keep track of the index so as to mirrow the record in RAM */
    		                            indexInTable = NWKLookupNodeByShortAddrVal(nwkSourceAddress.Val);
    		                            if(indexInTable != INVALID_NEIGHBOR_KEY ) 
                                        {
                                            BYTE        i;
                                            BYTE        validLongAddr;
                                            LONG_ADDR   invalidLongAddr;

                                            // Update the Extended Address. This is required because at the time of joining,
                                            // we will be creating/updating the Neighbor record with the beacon information.
                                            // But we will not be adding the Extended Address as this information is not
                                            // available in the beacon frame.
                                            // Upon receipt of Link Status we need to check whether the corresponding
                                            // Extended Address is null or not. If not null then only we have to update
                                            // the extended address.
                                            // If it is not null, then we should not update the extended address. This is
                                            // because address conflict may happen. We may receive the Link Status with
                                            // same short address but different extended address.

                                            //Prepare invalid IEEE Address, i.e 0x00.
                                            for(i = 0; i < 8; i++)
                                            {
                                                invalidLongAddr.v[i] = 0x00;
                                            }
                                            /* returns 0 if they are equal */
                                            validLongAddr = memcmp( (void *)&currentNeighborRecord.longAddr, (void *)&invalidLongAddr.v[0], 8 );
                                            if ( !validLongAddr )
                                            {
                                                // Memory matched, i.e extended address is NULL.
                                                if( nwkFrameControlMSB.bits.srcIEEEAddr )
                                                {
                                                    for (i=0; i<8; i++)
                                                    {
                                                        currentNeighborRecord.longAddr.v[i] = nwkIEEESrcAddress.v[i];
                                                    }
                                                }
                                            }
    			                    		thisIsANewneighbor = FALSE;
                                        }
                                        else
                                        {
                                            // else create the Neighbor Record
                                            // This is a new ZigBee node, so try to add it to the neighbor table.
    				                        /* and must update NVM */
    				                        /* keep track of the index so as to mirrow the record in RAM */
    				                        indexInTable = CanAddNeighborNode();
    				                        if ( indexInTable != INVALID_NEIGHBOR_KEY )
                                            {
                                                // Set all the information for a new node, that could get updated later.
                                                currentNeighborRecord.deviceInfo.bits.bInUse            = 1;
                                                if ( params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val == 0x0000 )
                                                {
                                                    currentNeighborRecord.deviceInfo.bits.deviceType    = DEVICE_ZIGBEE_COORDINATOR;
                                                }
                                                else
                                                {
                                                    currentNeighborRecord.deviceInfo.bits.deviceType    = DEVICE_ZIGBEE_ROUTER;
                                                }

                                                currentNeighborRecord.deviceInfo.bits.Relationship      = NEIGHBOR_IS_NONE; //NEIGHBOR_IS_SIBLING;

                                                if( nwkFrameControlMSB.bits.srcIEEEAddr )
                                                {
                                                    for (i=0; i<8; i++)
                                                    {
                                                        currentNeighborRecord.longAddr.v[i] = nwkIEEESrcAddress.v[i];
                                                    }
                                                }

                                                // Update the neighbor table size.
                                                currentNeighborTableInfo.neighborTableSize++;
                                                PutNeighborTableInfo();

                                                currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle      = 1;

                                                memcpy((BYTE *)&currentNeighborRecord.ExtendedPANID,
                                                       (BYTE *)&currentNeighborTableInfo.nwkExtendedPANID, 8);

    										    /* OutgoingCost and Age field shall be reset to 0, because this entry
    										     * is being created for the first time
    										    */
                                                currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost = 0;
                                                currentNeighborRecord.linkStatusInfo.Age = 0;

                                                #if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
                                                    currentNeighborRecord.suppressLinkStatusUpdate = 0x00;
                                                #endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
    										    /* record that this is a new neighbor so must update NVM */
    										    thisIsANewneighbor = TRUE;
                                            }
    				                    	
    				                    	/*  Neighbor entry could not be created , so discard packet and move onon */
                                            else
                                            {
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;
                                            }
                                        }
                                        RAMNeighborTable[indexInTable].shortAddr.Val = nwkSourceAddress.Val;

                                        // These fields needs to be updated because, after PANID_CONFLICT and
                                        // and Frequency Agility, these information might have changed. So, we
                                        // need to update with the latest information.
                                        // We also need to update if we have created a new neighbor entry.
                                        // Keeping this block here will serve both the requirements.
                                        currentNeighborRecord.panID.Val         = macPIB.macPANId.Val;
                                        currentNeighborRecord.shortAddr.Val     = nwkSourceAddress.Val;
                                        currentNeighborRecord.LogicalChannel    = phyPIB.phyCurrentChannel;
                                        currentNeighborRecord.nwkUpdateId       = currentNeighborTableInfo.nwkUpdateId;

                                        #if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
                                            if ( currentNeighborRecord.suppressLinkStatusUpdate == 0x00 )
                                        #endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
                                            {
                                                // Neighbor Record is created or already exists. We need to update the IncomingCost
                                                // field corresponding to this neighbor record. This is required because, for each
                                                // link status command received from the neighbor, IncomingCost may vary if the
                                                // device is not in fixed location. So, everytime we receive this command, we
                                                // need to calulate and update the IncomingCost.
            		                            previousInComingCost = currentNeighborRecord.linkStatusInfo.flags.bits.bIncomingCost;
                                                RAMNeighborTable[indexInTable].linkStatusInfo.flags.bits.bIncomingCost = 
																CalculateLinkQuality( params.MCPS_DATA_indication.mpduLinkQuality );   
                                            }
    		                            // Save the Neighbor Record
    		                            
    		                            /* If this is a brand new link status from the device then update NVM.
    		                             * if not a new device, then in the else block do comparison to old value first before writing 
    		                            */
    		                            if(thisIsANewneighbor)
    		                            {
    		                                PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord);
    		                            }
                                       /*  doing the NVM store (from the RAM value) in AgeOutNeighborTableEntries */
        		                       
        		                        /* reset this so that next Link Status command will start 
        		                         * the process over 
        		                        */
    		                            thisIsANewneighbor = FALSE;
                                        linkStatusCmdOptions = NWKGet();
                                        // We need only the Count bits. We do not need the information of
                                        // whether this is first frame or last frame. Only thing we consider
                                        // is that we loop through the Link Status list to search for our address.
                                        // If we do not have our address in the list, then we make the OutgoingCost
                                        // field in the Neighbor Record to 0x00. If our address exists in the
                                        // list, then we need to update the OutgoingCost field in the
                                        // Neighbor record with the IncomingCost field in the received frame.
                                        // Age field in the Neighbor record shall be reset to 0.
                                        linkStatusCmdOptions = linkStatusCmdOptions & 0x1F; // Extract only the Count bits from CommandOptions

                                        for (i=0; i<linkStatusCmdOptions; i++)
                                        {
                                            nwkAddr.byte.LSB        = NWKGet();
                                            nwkAddr.byte.MSB        = NWKGet();
                                            linkStatus.flags.Val    = NWKGet();
                                            // We need to update or create Neighbor record only if my address is known to the neighbor
                                            // who have sent the Link Status command.
                                            if (nwkAddr.Val == macPIB.macShortAddress.Val)
                                            {
                                                // We have received the link status from one of our neighbor.
                                                // We need to reset the age field so that the neighbor record will be active.
                                                // Outgoing cost shall be updated with incoming cost field corresponding to my address
                                                // in the received Link Status command frame
                                                #if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
                                                    if ( currentNeighborRecord.suppressLinkStatusUpdate == 0x00 )
                                                #endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
                                                {
            			                            /* save previous value in order to do comparison, first time this will be zero */
            			                            previousOutGoingCost = currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost;
            			                            RAMNeighborTable[indexInTable].linkStatusInfo.flags.bits.bOutgoingCost = linkStatus.flags.bits.bIncomingCost;
                                                }
                                                
                                                currentNeighborRecord.linkStatusInfo.Age = 0;
                                                RAMNeighborTable[indexInTable].linkStatusInfo.Age = currentNeighborRecord.linkStatusInfo.Age;

    	                                        break;
    				                        } 
    			                        } 
                                        NWKDiscardRx();
                                        return NO_PRIMITIVE;
                                    }
                                #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

                                #if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
                                    if( nwkCommand == NWK_COMMAND_NWK_UPDATE )
                                    {
                                        BYTE cmdOption;
                                        LONG_ADDR EPID;
                                        #ifndef I_AM_END_DEVICE
                                            if ( VerifyNetworkManager() )
                                            {
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;
                                            }
                                        #endif // #ifndef I_AM_END_DEVICE
                                        cmdOption = NWKGet();
                                        for ( i = 0 ; i < 8 ; i++ )
                                        {
                                            EPID.v[i] = NWKGet();
                                        }
                                        // Make sure that the frame came from my nwk
                                        if (memcmp((BYTE *)(&currentNeighborTableInfo.nwkExtendedPANID.v[0]), (BYTE *)(&EPID.v[0]), EXTENDED_PAN_SIZE))
                                        {
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                        }
                                        // We process only PAN Identifier Update
                                        if ( ((cmdOption & 0xE0) == 0x00) && ((cmdOption & 0x1F) == 0x01) )
                                        {
                                            if ( nwkStatus.panIDUpdate == NULL )
                                            {
                                                nwkStatus.panIDUpdate = (PAN_ID_UPDATE*)( SRAMalloc( sizeof( PAN_ID_UPDATE )));
                                                if ( nwkStatus.panIDUpdate == NULL )
                                                {
                                                   #ifdef DEBUG_LOG
                                                    LOG_ASSERT(DEBUG_LOG_INFO, "Mem Aloc Fail for PAN_ID_UPDATE" == 0);
                                                   #endif // #ifdef DEBUG_LOG
                                                    NWKDiscardRx();
                                                    return NO_PRIMITIVE;
                                                }
                                            }
                                            panIDConflictStatus.flags.bits.bPANIdUpdatePending = TRUE;
                                            panIDConflictStatus.flags.bits.bResolutionInProgress  = TRUE;
                                            nwkStatus.panIDUpdate->nwkUpdateId =  NWKGet();
                                            nwkStatus.panIDUpdate->newPANId.byte.LSB = NWKGet();
                                            nwkStatus.panIDUpdate->newPANId.byte.MSB = NWKGet();
                                        }
                                        else
                                        {
                                            #ifdef DEBUG_LOG
                                                LOG_ASSERT(DEBUG_LOG_INFO, "Received Update Id is not PAN Identifier Update" == 0);
                                            #endif // #ifdef DEBUG_LOG
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                        }

                                        if (nwkRadius > 1)
                                        {
                                            BYTE BTTIndex , *pData;
                                            BTR *pCurrent_BTR;

                                            if (CreateNewBTR( &BTTIndex ))
                                            {
                                                pCurrent_BTR = nwkStatus.BTT[BTTIndex]; // code optimisation

                                                // There is room to store the broadcast packet.  Indicate that the message did not
                                                // originate from our upper layers.
                                                pCurrent_BTR->btrInfo.bMessageFromUpperLayers = 0;

                                                // Mark the originator as having passive ACK'd
                                                MarkNeighborAsPasssiveACKed( BTTIndex );

                                                // Save off the NWK header information
                                                pCurrent_BTR->dataLength                   = 0x0D;
                                                pCurrent_BTR->nwkFrameControlLSB           = nwkFrameControlLSB;
                                                pCurrent_BTR->nwkFrameControlMSB           = nwkFrameControlMSB;
                                                pCurrent_BTR->nwkDestinationAddress.Val    = nwkDestinationAddress.Val;
                                                pCurrent_BTR->nwkSourceAddress.Val         = nwkSourceAddress.Val;
                                                pCurrent_BTR->nwkRadius                    = nwkRadius - 1;
                                                pCurrent_BTR->nwkSequenceNumber            = nwkSequenceNumber;
                                                pCurrent_BTR->nwkIEEESrcAddress            = nwkIEEESrcAddress;

                                                // Save off the NWK payload
                                                if ((pCurrent_BTR->dataPtr = (BYTE *)SRAMalloc( 0x0D )) == NULL)
                                                {
                                                    // We do not have room to store this broadcast packet.  Destroy the BTR
                                                    // and ignore the packet.
                                                    nfree( nwkStatus.BTT[BTTIndex] );

                                                    // NWK errata - if we cannot rebroadcast the packet, then do not send up to higher layers.  We'll
                                                    // see it again when the source doesn't see our rebroadcast and sends it again.
                                                    NWKDiscardRx();
                                                    return NO_PRIMITIVE;
                                                }
                                                else
                                                {
                                                    pData = pCurrent_BTR->dataPtr;
                                                    *pData++ = NWK_COMMAND_NWK_UPDATE;
                                                    *pData++ = cmdOption;
                                                     memcpy( pData , (BYTE *)(&EPID.v[0]), EXTENDED_PAN_SIZE );
                                                     pData += 8;
                                                    *pData++ = nwkStatus.panIDUpdate->nwkUpdateId;
                                                    *pData++ = nwkStatus.panIDUpdate->newPANId.byte.LSB;
                                                    *pData++ = nwkStatus.panIDUpdate->newPANId.byte.MSB;
                                                    // Set the network status so we can begin transmitting these messages
                                                    // in the background.  We have to process the message ourselves first!
                                                    nwkStatus.flags.bits.bSendingBroadcastMessage = 1;
                                                }
                                            }
                                            else
                                            {
                                                // NWK errata - if we cannot rebroadcast the packet, then do not send up to higher layers.
                                                NWKDiscardRx();
                                               return NO_PRIMITIVE;

                                            }
                                        }
                                        params.NLME_NWK_STATUS_indication.nwkStatusCode = NWK_STATUS_PAN_ID_UPDATE;
                                        params.NLME_NWK_STATUS_indication.ShortAddress.Val = 0x0000;
                                        return NLME_NWK_STATUS_indication;
                                    }
                                #endif //I_SUPPORT_PANID_CONFLICT

                                NWKDiscardRx();
                                return NO_PRIMITIVE;
                            }
                            // The packet is a broadcast or multicast data frame.
                            else
                            {
                                // The packet is a broadcast or multicast data frame.

                                BYTE    BTTIndex;
                                BYTE    dataLength;
                                BYTE    sendDataIndication = TRUE;

                                if ( nwkFrameControlMSB.bits.multicastFlag )
                                {
                                    // Check whether we are part the group. This is done at APS Layer
                                    //params.NLDE_DATA_indication.DstAddrMode = 0x01; // Multicast Frame
                                    params.NLDE_DATA_indication.DstAddr.Val = nwkDestinationAddress.Val;
                                    params.NLDE_DATA_indication.isMulticastFrame = 0x01; // This is a multicast frame

                                    
                                    #if ( I_SUPPORT_NWK_LAYER_MULTICAST == 0x01 )
                                        if( ( GetEndPointsFromGroup(nwkDestinationAddress) != MAX_GROUP ) )

                                        {
                                            nwkMulticast.bits.nonMemberRadius = nwkMulticast.bits.maxNonMemberRadius;
                                        }
                                        else
                                    #endif // #if ( I_SUPPORT_NWK_LAYER_MULTICAST == 0x01 )
                                        {
                                           if ( nwkMulticast.bits.nonMemberRadius > 1 )
                                           {
                                               if (nwkMulticast.bits.nonMemberRadius < 7)
                                               {
                                                    nwkMulticast.bits.nonMemberRadius--;
                                                    sendDataIndication = FALSE;
                                               }
                                           }
                                        }
                                }
                                else
                                {
                                    params.NLDE_DATA_indication.isMulticastFrame = 0x00; // Not a multicast frame.
                                }

                                // Set up the packet for rebroadcast.  The packet must have a radius > 1 to retransmit
                                if (nwkRadius > 1)
                                {
                                    if ( ( nwkFrameControlMSB.bits.multicastFlag ) &&
                                         ( nwkMulticast.bits.nonMemberRadius <= 1 ) )
                                    {
                                        goto SendBroadcastDataIndication;
                                    }
                                    if (CreateNewBTR( &BTTIndex ))
                                    {
                                        //There is room to store the broadcast packet.  Indicate that the message did not
                                        // originate from our upper layers.
                                        nwkStatus.BTT[BTTIndex]->btrInfo.bMessageFromUpperLayers = 0;

                                        // Mark the originator as having passive ACK'd
                                        MarkNeighborAsPasssiveACKed( BTTIndex );

                                        // Save off the NWK header information
                                        nwkStatus.BTT[BTTIndex]->dataLength                   = params.MCPS_DATA_indication.msduLength;
                                        nwkStatus.BTT[BTTIndex]->nwkFrameControlLSB           = nwkFrameControlLSB;
                                        nwkStatus.BTT[BTTIndex]->nwkFrameControlMSB           = nwkFrameControlMSB;
                                        nwkStatus.BTT[BTTIndex]->nwkDestinationAddress.Val    = nwkDestinationAddress.Val;
                                        nwkStatus.BTT[BTTIndex]->nwkSourceAddress.Val         = nwkSourceAddress.Val;
                                        nwkStatus.BTT[BTTIndex]->nwkRadius                    = nwkRadius - 1;
                                        nwkStatus.BTT[BTTIndex]->nwkSequenceNumber            = nwkSequenceNumber;
                                        if ( nwkFrameControlMSB.bits.multicastFlag )
                                        {
                                            nwkStatus.BTT[BTTIndex]->nwkMulticastFields.Val   = nwkMulticast.Val;
                                        }

                                        /* save the SrcIEEE address if its present in current packet */
                   						if ( nwkFrameControlMSB.bits.srcIEEEAddr )
                   						{
	                   						WORD i;
					                        for(i = 0; i < 8; i++)
					                        {
					                           nwkStatus.BTT[BTTIndex]->nwkIEEESrcAddress.v[i] = 
					                           			 nwkIEEESrcAddress.v[i];
					                        }
					                    }
	             	
                                        // Save off the NWK payload
                                        if ((nwkStatus.BTT[BTTIndex]->dataPtr = (BYTE *)SRAMalloc( params.MCPS_DATA_indication.msduLength )) == NULL)
                                        {
                                            // There is room to store this broadcast packet.  Destroy the BTR
                                            // and ignore the packet.
                                            nfree( nwkStatus.BTT[BTTIndex] );

                                            // NWK errata - if we cannot rebroadcast the packet, then do not send up to higher layers.  We'll
                                            // see it again when the source doesn't see our rebroadcast and sends it again.
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                        }
                                        else
                                        {
                                            i = 0;
                                            ptr = params.MCPS_DATA_indication.msdu;
                                            dataLength = params.MCPS_DATA_indication.msduLength;
                                            while (params.MCPS_DATA_indication.msduLength)
                                            {
                                                nwkStatus.BTT[BTTIndex]->dataPtr[i++] = NWKGet();
                                            }
                                            #ifdef I_SUPPORT_SECURITY
                                                nwkStatus.BTT[BTTIndex]->btrInfo.bAlreadySecured = 0x01;
                                            #endif // #ifdef I_SUPPORT_SECURITY

                                            // Restore the data packet so we can send it to our upper layers
                                            params.MCPS_DATA_indication.msdu = ptr;
                                            params.MCPS_DATA_indication.msduLength = dataLength;

                                            // Set the network status so we can begin transmitting these messages
                                            // in the background.  We have to process the message ourselves first!
                                            nwkStatus.flags.bits.bSendingBroadcastMessage = 1;
                                        }
                                    }
                                    else
                                    {
                                        // NWK errata - if we cannot rebroadcast the packet, then do not send up to higher layers.
                                        NWKDiscardRx();
                                        return NO_PRIMITIVE;
                                    }
                                }

SendBroadcastDataIndication:
                                // If we're receiving a broadcast packet from someone, but we originated it, ignore it.
                                // We still have to rebroadcast it for everyone else's ACK.
                                //if (nwkSourceAddress.Val == macPIB.macShortAddress.Val)
                                //{
                                //    NWKDiscardRx();
                                //    return NO_PRIMITIVE;
                                //}

                                if ( sendDataIndication )
                                {
                                    // Now device needs to process the broadcast packet.  Send up an NLDE-DATA.indication.
                                    // Don't discard the packet!  The upper layers are going to use it.
                                    params.NLDE_DATA_indication.DstAddr.Val = nwkDestinationAddress.Val;
                                    params.NLDE_DATA_indication.SrcAddress.Val = nwkSourceAddress.Val;
                                    
                                    /* initialze all the NLDE_DATA_indication parameters */
                                    params.NLDE_DATA_indication.NsduLength  = params.MCPS_DATA_indication.msduLength;
                                    params.NLDE_DATA_indication.Nsdu        = params.MCPS_DATA_indication.msdu;
                                    params.NLDE_DATA_indication.LinkQuality = params.MCPS_DATA_indication.mpduLinkQuality;
                                    // These overlay - params.NLDE_DATA_indication.NsduLength = params.MCPS_DATA_indication.msduLength;
                                    // These overlay - params.NLDE_DATA_indication.LinkQuality = params.MCPS_DATA_indication.mpduLinkQuality;
                                    return NLDE_DATA_indication;
                                }
                                else
                                {
                                    NWKDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                            }
                        }
                        else
                    //#endif // I_AM_RFD TODO
                    {
                        // Handle all unicast frames.
                        if (nwkFrameControlLSB.bits.frameType == NWK_FRAME_CMD)
                        {
                            BYTE removeNeighborTableEntry = 1;

                            // This is a network command frame.
                            i = NWKGet();
                            switch(i)
                            {
                                #if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
                                    #ifndef I_AM_END_DEVICE
                                        case NWK_COMMAND_NWK_REPORT:
                                        {
                                            if ( (!panIDConflictStatus.flags.bits.bResolutionInProgress) && VerifyNetworkManager() )
                                            {
                                                BYTE cmdOption , n; //(Report information count)
                                                LONG_ADDR EPID;
                                                WORD newPANId;

                                                cmdOption = NWKGet();
                                                for ( i = 0 ; i < 8 ; i++ )
                                                {
                                                    EPID.v[i] = NWKGet();
                                                }
                                                // Make sure that the frame came from my nwk
                                                if (memcmp((BYTE *)(&currentNeighborTableInfo.nwkExtendedPANID.v[0]), (BYTE *)(&EPID.v[0]), EXTENDED_PAN_SIZE))
                                                {
                                                    NWKDiscardRx();
                                                    return NO_PRIMITIVE;
                                                }
                                                // We process only PAN identifier conflict
                                                if ( ((cmdOption & 0xE0) == 0x00) )
                                                {
                                                    n = cmdOption & 0x1F;
                                                    for ( ; n > 0 ; n-- )
                                                    {
                                                        newPANId = NWKGet(); //LSB
                                                        newPANId |= (NWKGet() << 8); //MSB
                                                        nwkStatus.PanIDReport.PANIdList[nwkStatus.PanIDReport.index++].Val = newPANId;
                                                        if ( nwkStatus.PanIDReport.index >= 0x08 )
                                                        {
                                                            nwkStatus.PanIDReport.index = 0;
                                                        }
                                                        if ( nwkStatus.PanIDReport.noOfRecords < 8 )
                                                        {
                                                            nwkStatus.PanIDReport.noOfRecords++;
                                                        }
                                                    }
                                                    panIDConflictStatus.flags.bits.bPANIdConflictDetected = TRUE;
                                                    panIDConflictStatus.flags.bits.bResolutionInProgress  = TRUE;
                                                }
                                            }
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                        }
                                        break;
                                    #endif // #ifndef I_AM_END_DEVICE

                                    //#ifdef I_AM_RFD
                                        case NWK_COMMAND_NWK_UPDATE:
                                        if (NOW_I_AM_A_RFD()){//{
                                            BYTE cmdOption , nwkUpdateId , PANIdLSB , PANIdMSB;
                                            LONG_ADDR EPID;

                                            cmdOption = NWKGet();
                                            for ( i = 0 ; i < 8 ; i++ )
                                            {
                                                EPID.v[i] = NWKGet();
                                            }
                                            // Make sure that the frame came from my nwk
                                            if (memcmp((BYTE *)(&currentNeighborTableInfo.nwkExtendedPANID.v[0]), (BYTE *)(&EPID.v[0]), EXTENDED_PAN_SIZE))
                                            {
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;
                                            }
                                            // We process only PAN Identifier Update
                                            if ( ((cmdOption & 0xE0) == 0x00) && ((cmdOption & 0x1F) == 0x01) )
                                            {
                                                nwkUpdateId = NWKGet();
                                                PANIdLSB = NWKGet();
                                                PANIdMSB = NWKGet();

                                                if ( nwkStatus.panIDUpdate == NULL )
                                                {
                                                    nwkStatus.panIDUpdate = (PAN_ID_UPDATE*)( SRAMalloc( sizeof( PAN_ID_UPDATE )));
                                                    if ( nwkStatus.panIDUpdate == NULL )
                                                    {
                                                        #ifdef DEBUG_LOG
                                                            LOG_ASSERT(DEBUG_LOG_INFO, "Mem Aloc Fail for PAN_ID_UPDATE" == 0);
                                                        #endif // #ifdef DEBUG_LOG
                                                        NWKDiscardRx();
                                                        return NO_PRIMITIVE;
                                                    }
                                                }
                                                nwkStatus.panIDUpdate->nwkUpdateId =  nwkUpdateId;
                                                nwkStatus.panIDUpdate->newPANId.byte.LSB = PANIdLSB;
                                                nwkStatus.panIDUpdate->newPANId.byte.MSB = PANIdMSB;
                                                nwkStatus.panIDUpdate->waitTime = TickGet();
                                                panIDConflictStatus.flags.bits.bPANIdUpdatePending = TRUE;
                                                params.NLME_NWK_STATUS_indication.nwkStatusCode = NWK_STATUS_PAN_ID_UPDATE;
                                                params.NLME_NWK_STATUS_indication.ShortAddress.Val = 0x0000;
                                                return NLME_NWK_STATUS_indication;
                                            }
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                        }
                                        break;
                                    //#endif // #ifdef I_AM_RFD
                                #endif //I_SUPPORT_PANID_CONFLICT

                                #if (  I_SUPPORT_MANY_TO_ONE_HANDLING == 1 || I_SUPPORT_CONCENTRATOR == 1 )
                                    // HAndle received route record
                                    case NWK_COMMAND_ROUTE_RECORD:
                                    {
                                        if( nwkFrameControlMSB.bits.dstIEEEAddr )
                                        {
                                            #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                                #ifndef I_AM_END_DEVICE
                                                    if(IsThereAddressConflict( nwkDestinationAddress , &nwkIEEEDstAddress ))
                                                    {
                                                       // address conflict detected
                                                        return NLME_NWK_STATUS_indication;
                                                    }
                                                #endif // #ifndef I_AM_END_DEVICE
                                            #endif // #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                        }
                                        #if   I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                            BYTE result;
                                            BYTE relayCount;
                                            BYTE addressCount;
                                            BYTE *relayList, *tempRelayList;

                                            relayCount = NWKGet();
                                            relayList = SRAMalloc( 2 * relayCount );
                                            if( relayList == NULL )
                                            {
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;
                                            }
                                            else
                                            {
                                                tempRelayList = relayList;
                                                for( addressCount = 0; addressCount < (relayCount); addressCount++)
                                                {
                                                    *tempRelayList++ = NWKGet(); // LSB of short address
                                                    *tempRelayList++ = NWKGet(); // MSB of short address
                                                }
                                            }
                                            NWKDiscardRx();
                                            // forward the route record to concentrator
                                            result = ( CreateRelyingRouteRecord
                                                       (
                                                            nwkIEEESrcAddress,
                                                            nwkIEEEDstAddress,
                                                            nwkSequenceNumber,
                                                            nwkRadius,
                                                            nwkSourceAddress,
                                                            nwkDestinationAddress,
                                                            nwkFrameControlMSB,
                                                            nwkFrameControlLSB,
                                                            relayCount,
                                                            relayList
                                                       )
                                                     );
                                            nfree(relayList);
                                            return result;

                                        #endif  // #if   I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                        #if I_SUPPORT_CONCENTRATOR ==1
                                            // Store the route record on the concentrator
                                            StoreRouteRecord(nwkSourceAddress );
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                       #endif // #if I_SUPPORT_CONCENTRATOR ==1
                                    }
                                    break;
                                #endif // #if (  I_SUPPORT_MANY_TO_ONE_HANDLING == 1 || I_SUPPORT_CONCENTRATOR == 1 )


                                case NWK_COMMAND_LEAVE:
                                {
                                    i = NWKGet();   // Get the command options byte.
                                    NWKDiscardRx();
                                    if(NOW_I_AM_NOT_A_CORDINATOR()){//#if !defined(I_AM_COORDINATOR)
                                        // Check whether the parent device has requested me to leave
                                        if( NWKThisIsMyLongAddress(&nwkIEEEDstAddress) &&
                                          ( nwkDestinationAddress.Val == macPIB.macShortAddress.Val ) &&
                                          ( i & NWK_LEAVE_REQUEST ) &&
                                          ( nwkSourceAddress.Val == macPIB.macCoordShortAddress.Val ) )
                                        {
                                            removeNeighborTableEntry = 0;
                                            // Device is asked to leave the network
                                            nwkStatus.flags.bits.bLeaveInProgress = 1;

                                            #if !defined(I_AM_END_DEVICE)
                                                // Do not allow any other devices to join.
                                                macPIB.macAssociationPermit = FALSE;
                                            #endif // #if !defined(I_AM_END_DEVICE)

                                            // Update the variables so that leave command frame can be
                                            // transmitted in the background
                                            nwkStatus.flags.bits.bRemoveChildren = (i & NWK_LEAVE_REMOVE_CHILDREN) ? 1 : 0;

                                            nwkStatus.flags.bits.bSelfLeave = 1;
                                            nwkStatus.leaveReason = COORDINATOR_FORCED_LEAVE;
                                            nwkStatus.flags.bits.bRejoin = (i & NWK_LEAVE_REJOIN) ? 1 : 0;
                                            nwkStatus.flags.bits.bRejoinScan = (i & NWK_LEAVE_REJOIN) ? 1 : 0;
                                            // We send first leave indication to the upper layer and then
                                            // initiate the leave procedure in the background.
                                            for (i=0; i<8; i++)
                                            {
                                                params.NLME_LEAVE_indication.DeviceAddress.v[i] = 0x00;
                                            }
                                        }
                                        else
                                        {
                                            // We have received the leave command frame from one of the child
                                            // end device. So, neighbor table entry for that device needs to be
                                            // removed
                                            removeNeighborTableEntry = 1;
                                        }
                                    }else{//#else
                                        // We have received the leave command frame from one of the child
                                        // end device. So, neighbor table entry for that device needs to be
                                        // removed
                                        removeNeighborTableEntry = 1;
                                    }//#endif // #if !defined(I_AM_COORDINATOR)
                                    params.NLME_LEAVE_indication.Rejoin = (i & NWK_LEAVE_REJOIN) ? 1 : 0;
                                    if ( removeNeighborTableEntry )
                                    {
                                        i = NWKLookupNodeByShortAddrVal( nwkSourceAddress.Val );
                                        if (i != INVALID_NEIGHBOR_KEY)
                                        {
                                            if(currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD)
                                            {
                                                params.NLME_LEAVE_indication.isChild = TRUE;
                                                #if defined(I_SUPPORT_SECURITY)
                                                    leavingChildDetails.DeviceShortAddress.Val = nwkSourceAddress.Val;
                                                    memcpy(leavingChildDetails.DeviceLongAddress.v, nwkIEEESrcAddress.v, 8);
                                                    zdoStatus.flags.bits.SendUpdateDeviceRequestAfterChildLeave = 1;
                                                #endif // #if defined(I_SUPPORT_SECURITY)
                                            }
                                            else
                                            {
                                                params.NLME_LEAVE_indication.isChild = FALSE;
                                            }
                                            params.NLME_LEAVE_indication.DeviceShortAddress.Val =
                                                nwkSourceAddress.Val ;
                                            RemoveNeighborTableEntry();
                                            // Update the DeviceAddress parameter in the Leave Indication structure
                                            memcpy( (void *)&(params.NLME_LEAVE_indication.DeviceAddress),
                                                (void *)&(nwkIEEESrcAddress), 8 );

                                            // A Zigbee 2006 requirement - ZCP Tests requires that the APS table be cleanned up as well
                                            for( i = 0; i < apscMaxAddrMapEntries; i++)
                                            {
                                                #ifdef USE_EXTERNAL_NVM
                                                    GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                                                #else
                                                    GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                                                #endif // #ifdef USE_EXTERNAL_NVM
                                                if (currentAPSAddress.shortAddr.Val == nwkSourceAddress.Val)
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
                                    }
                                    return NLME_LEAVE_indication;
                                }

                                // A zigbee2006 requirement: process the rejoin
                                //#if !defined(I_AM_COORDINATOR)
                                    case NWK_COMMAND_REJOIN_RESPONSE:
                                  if(NOW_I_AM_NOT_A_CORDINATOR()){//
	                                    {
                                        SHORT_ADDR myShortAddr;
                                        BYTE status;
                                        BYTE neighborTableHandle;
                                        #ifdef I_SUPPORT_SECURITY
                                            BYTE securityEnable;

                                            securityEnable = params.NLDE_DATA_indication.SecurityUse;
                                        #endif
                                        rejoin_retries = 0;
                                        if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                                            nwkStatus.rejoinCommandSent = 0x00;
                                        }//#endif
                                        ZigBeeStatus.flags.bits.bNetworkJoined = TRUE;
                                        myShortAddr.v[0] = NWKGet();
                                        myShortAddr.v[1] = NWKGet();
                                        status = NWKGet();
                                        NWKDiscardRx();

                                        if( !NWKThisIsMyLongAddress(&nwkIEEEDstAddress) )
                                        // Zigbee 2006 requirement:

                                        {
                                            return NO_PRIMITIVE;
                                        }

                                        if( status ) // not permitted to join
                                        {
                                            // if the status is not successful, don't join network
                                            ZigBeeStatus.flags.bits.bNetworkJoined = 0;
                                            params.NLME_JOIN_confirm.Status = status;
                                            return NLME_JOIN_confirm;

                                        }
                                        else
                                        {
                                            // Zigbee 2006: Lookup OLD parent in NTABLE and update that record
                                            // to add the new parent's information

                                            neighborTableHandle = NWKLookupNodeByLongAddr(&macPIB.macCoordExtendedAddress);
                                            // ZigBee 2006 requirement:  If the new parent is not YET in this Neighbor
                                            // table, BUT its telling this device it can join, SO add it to NTABLE, such that rest of the Join process will work.
                                            // Mandatory Testcase 3.73.

                                            if(neighborTableHandle != INVALID_NEIGHBOR_KEY)
                                            {
                                                #ifdef USE_EXTERNAL_NVM
                                                    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
                                                #else
                                                    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
                                                #endif // #ifdef USE_EXTERNAL_NVM
                                                {
                                                    GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
                                                    if ( currentNeighborRecord.deviceInfo.bits.bInUse &&
                                                        (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT) )
                                                    {
                                                        currentNeighborRecord.deviceInfo.bits.Relationship = NEIGHBOR_IS_SIBLING;
                                                        PutNeighborRecord(pCurrentNeighborRecord, &currentNeighborRecord);
                                                        break;

                                                    }
                                                }
                                            }

                                            neighborTableHandle = NWKLookupNodeByShortAddrVal(nwkSourceAddress.Val);
                                            if ( neighborTableHandle == INVALID_NEIGHBOR_KEY )
                                            {
                                                return NO_PRIMITIVE;
                                            }
                                            macPIB.macShortAddress.Val = myShortAddr.Val;
                                            MLME_SET_macShortAddress_hw();
                                            currentNeighborRecord.deviceInfo.bits.Relationship  = NEIGHBOR_IS_PARENT;

                                            // Update NWK Update Id NIB when the device is successfully joined
                                            currentNeighborTableInfo.nwkUpdateId = currentNeighborRecord.nwkUpdateId;
                                            currentNeighborTableInfo.parentNeighborTableIndex = neighborTableHandle;
                                            PutNeighborTableInfo();

                                            macPIB.macCoordExtendedAddress              = nwkIEEESrcAddress; // my new parent
                                            macPIB.macCoordShortAddress                 = nwkSourceAddress;
                                            currentNeighborRecord.shortAddr.v[1]        = nwkSourceAddress.byte.MSB;
                                            currentNeighborRecord.shortAddr.v[0]        = nwkSourceAddress.byte.LSB;
                                            ParentAddress = currentNeighborRecord.shortAddr;
                                            BYTE i;
                                            for (i = 0; i < 8; i++)
                                            {
                                                currentNeighborRecord.longAddr.v[i] = \
                                                    nwkIEEESrcAddress.v[i];
                                                if( NOW_I_AM_A_ROUTER() )
                                                {
                                                	current_SAS.spas.TrustCenterAddress.v[i] = currentNeighborRecord.longAddr.v[i];
                                                	TCLinkKeyInfo.trustCenterLongAddr.v[i] = currentNeighborRecord.longAddr.v[i];
                                                	TCLinkKeyInfo.trustCenterShortAddr.Val = 0;
                                                }
                                            }
                                            PutNeighborRecord(pCurrentNeighborRecord, &currentNeighborRecord);

                                            params.NLME_JOIN_confirm.Status = SUCCESS;
                                            params.NLME_JOIN_confirm.ShortAddress.Val   = myShortAddr.Val;

                                            // Extended PANId and Active channel needs to be updated
                                            // from parent neighbor record.

                                            params.NLME_JOIN_confirm.ExtendedPANId = currentNeighborRecord.ExtendedPANID;
                                            params.NLME_JOIN_confirm.ActiveChannel = currentNeighborRecord.LogicalChannel;
                                            #ifdef I_SUPPORT_ROUTING
                                                nwkStatus.flags.bits.bCanRoute = 1;
                                            #endif // #ifdef I_SUPPORT_ROUTING
                                            #ifndef I_AM_END_DEVICE
                                                currentNeighborTableInfo.depth                  = currentNeighborRecord.deviceInfo.bits.Depth+1; // Our depth in the network
                                                #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
                                                    currentNeighborTableInfo.cSkip.Val              = GetCSkipVal( currentNeighborTableInfo.depth ); // Address block size
                                                    if (!currentNeighborTableInfo.flags.bits.bChildAddressInfoValid)
                                                    {
                                                        currentNeighborTableInfo.nextEndDeviceAddr.Val  = macPIB.macShortAddress.Val + currentNeighborTableInfo.cSkip.Val * NIB_nwkMaxRouters + 1;  // Next address available to give to an end device
                                                        currentNeighborTableInfo.nextRouterAddr.Val     = macPIB.macShortAddress.Val + 1;     // Next address available to give to a router
                                                        currentNeighborTableInfo.numChildren            = 0;
                                                        currentNeighborTableInfo.numChildRouters        = 0;
                                                        currentNeighborTableInfo.flags.bits.bChildAddressInfoValid = 1;
                                                    }

                                                // For Routers, on successful join we should update the nwkUpdateId
                                                // and update the beacon payload.

                                                #else
                                                    currentNeighborTableInfo.numChildren            = 0;
                                                    currentNeighborTableInfo.nextChildAddr.Val = GenerateNewShortAddr();
                                                    nwkStatus.flags.bits.bNextChildAddressUsed = FALSE;
                                                #endif  // #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )

                                                currentNeighborTableInfo.nwkUpdateId = currentNeighborRecord.nwkUpdateId;
                                                // Update the nwkLinkStatusPeriod for transmitting Link Status Command and Neighbor table aging
                                                #ifndef I_SUPPORT_SECURITY
                                                    #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                                        nwkStatus.nwkLinkStatusPeriod = TickGet();
                                                    #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                                #endif // #ifndef I_SUPPORT_SECURITY
                                                PutNeighborTableInfo();
                                                SetBeaconPayload(TRUE);
                                            #endif // #ifndef I_AM_END_DEVICE

                                            //return NLME_JOIN_confirm;
                                            #ifdef I_SUPPORT_SECURITY
                                                // Remember, we will get unsolicited Rejoin Response from our
                                                // parent device in case of Address Conflict. In this case, we
                                                // should not trigger Authentication Procedure.
                                                if(nwkStatus.flags.bits.bRejoinInProgress)
                                                {
                                                    #ifdef USE_EXTERNAL_NVM
                                                        pCurrentNeighborRecord = neighborTable + ( neighborTableHandle * (WORD)sizeof(NEIGHBOR_RECORD) );
                                                    #else
                                                        pCurrentNeighborRecord = neighborTable + neighborTableHandle;
                                                    #endif
                                                    GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

                                                    if (currentNeighborRecord.deviceInfo.bits.StackProfile > 0x01 && !securityEnable)
                                                    {
                                                        securityStatus.flags.bits.bAuthorization = TRUE;
                                                        AuthorizationTimeout = TickGet();
                                                        WaitForAuthentication.Val = AUTHORIZATION_TIMEOUT * 5;
                                                    }
                                                    else
                                                    {
                                                        ZigBeeStatus.flags.bits.bTriggerDeviceAnnce = 1;
                                                        appNotify.current_ADDR.Val = macPIB.macShortAddress.Val;
                                                        appNotification = TRUE;
                                                    }
                                                    if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
                                                        #if ( I_SUPPORT_MANY_TO_ONE_HANDLING == 0x01)
                                                            #if I_SUPPORT_LINK_KEY == 1
                                                                /* set the trust center as concentrator */
                                                                pCurrentRoutingEntry = routingTable;
                                                                GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
                                                                currentRoutingEntry.destAddress.Val = 0x0000;
                                                                currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_ACTIVE;
                                                                currentRoutingEntry.rtflags.rtbits.bNoRouteCache =  TRUE;
                                                                currentRoutingEntry.rtflags.rtbits.bManyToOne = TRUE;
                                                                currentRoutingEntry.rtflags.rtbits.bRouteRecordRequired = TRUE;
                                                                currentRoutingEntry.nextHop  = ParentAddress;
                                                                #if (I_SUPPORT_ROUTING_TABLE_AGING == 0x01)
                                                                    currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                                                                #endif // (I_SUPPORT_ROUTING_TABLE_AGING == 0x01)
                                                                PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
                                                            #endif // I_SUPPORT_LINK_KEY == 1
                                                        #endif //( I_SUPPORT_MANY_TO_ONE_HANDLING == 0x01)
                                                    }//#endif // I_AM_ROUTER
                                                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                                                        PHYTasksPending.bits.PHY_AUTHORIZE = 1;
                                                        lastPollTime = AuthorizationTimeout;
                                                    }//#endif
                                                }
                                                else
                                                {
                                                    //We have received an unsolicited Rejoin Response from our parent
                                                    //Authentication Procedure is not required
                                                    //But device annce is required
                                                    ZigBeeStatus.flags.bits.bTriggerDeviceAnnce = 1;
                                                    appNotify.current_ADDR.Val = macPIB.macShortAddress.Val;
                                                    appNotification = TRUE;
                                                }
                                            #endif // #ifdef I_SUPPORT_SECURITY

                                            if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                                                ZigBeeStatus.flags.bits.bRequestingData = 0;
                                            }//#endif // #ifdef I_AM_RFD
                                            
                                            /*Rejoin is Successful. So, no need to do scan and 
                                            rejoin again.*/
                                            ZigBeeStatus.flags.bits.bActiveScanToRejoin = 0;
                                            nwkStatus.flags.bits.bRejoinScan = 0;

                                        nwkStatus.flags.bits.bRejoinInProgress = 0;
                                        /*Received the Rejoin Response. So, reset this variable*/
                                        waitForKey = 0;

                                        // Status is already in place.
                                        // Send the ED_annoc only after key is recieved in security mode
                                        // Clean up the neighbor table entries that does not belong to my network
                                        NWKTidyNeighborTable();
                                        #if !defined(I_SUPPORT_SECURITY)
                                            /* Updated NLME join confirmation parameters */
                                            params.NLME_JOIN_confirm.ExtendedPANId = currentNeighborRecord.ExtendedPANID;
                                            params.NLME_JOIN_confirm.ActiveChannel = currentNeighborRecord.LogicalChannel;

                                            return NLME_JOIN_confirm;
                                        #else

                                            if (securityEnable)
                                            {
                                                /* Updated NLME join confirmation parameters */
                                                params.NLME_JOIN_confirm.ExtendedPANId = currentNeighborRecord.ExtendedPANID;
                                                params.NLME_JOIN_confirm.ActiveChannel = currentNeighborRecord.LogicalChannel;

                                                return NLME_JOIN_confirm;
                                            }
                                            else
                                            {
                                                /* Updated NLME join confirmation parameters */
                                                params.NLME_JOIN_confirm.ExtendedPANId = currentNeighborRecord.ExtendedPANID;
                                                params.NLME_JOIN_confirm.ActiveChannel = currentNeighborRecord.LogicalChannel;

                                                return NLME_JOIN_confirm;
                                            }


                                        #endif // #if !defined(I_SUPPORT_SECURITY)
                                        }
                                    }
                                }//#endif // #if !defined(I_AM_COORDINATOR)

                                #if !defined(I_AM_END_DEVICE)
                                    case NWK_COMMAND_REJOIN_REQUEST:
                                    {
                                        SHORT_ADDR oldShortAddr;
                                        LONG_ADDR tempLongAddr;
                                        LONG_ADDR tempLongAddr1;
                                       // BYTE test = 0x01;

                                        BOOL    rejoinStatus  = TRUE;

                                       

                                        oldShortAddr.Val = nwkSourceAddress.Val;
                                        params.MLME_ASSOCIATE_indication.CapabilityInformation.Val = NWKGet();

                                        NWKDiscardRx();

                                        TxBuffer[TxData++] = NWK_COMMAND_REJOIN_RESPONSE;
                                        if( NWKLookupNodeByLongAddr(&nwkIEEESrcAddress) != INVALID_NEIGHBOR_KEY )
                                        {
                                            RemoveNeighborTableEntry();
                                        }
                                        // A ZigBee 2006 reqquirement:  Ignore joinPermit flag during in rejoin process
                                        doingRejoinCommand = TRUE;
                                        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                            if ( IsThisAddressKnown( oldShortAddr , &tempLongAddr.v[0] ) )
                                            {
                                                for (i = 0; i < 8; i++)
                                                {
                                                    tempLongAddr1.v[7-i] = tempLongAddr.v[i];
                                                }
                                                if (memcmp((BYTE *)&nwkIEEESrcAddress, (BYTE *)&tempLongAddr1, 8))
                                                {
                                                    nwkSourceAddress.Val = currentNeighborTableInfo.nextChildAddr.Val;
                                                    nwkStatus.flags.bits.bNextChildAddressUsed = TRUE;
                                                }
                                            }
                                        #endif // #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                        if (!CanAddChildNode() || ((i = CanAddNeighborNode()) == INVALID_NEIGHBOR_KEY))
                                        {
                                            // We do not have room to add this node to the neighbor table.
                                            TxBuffer[TxData++] = 0xFD;
                                            TxBuffer[TxData++] = 0xFF;
                                            TxBuffer[TxData++] = ASSOCIATION_PAN_ACCESS_DENIED;
                                        }
                                        else
                                        {
                                            rejoinShortAddress.byte.LSB = nwkSourceAddress.byte.LSB;
                                            rejoinShortAddress.byte.MSB = nwkSourceAddress.byte.MSB;

                                            // Add the node to our network
                                            #ifdef I_SUPPORT_SECURITY
                                                AddChildNode(TRUE);
                                            #else
                                                AddChildNode();
                                            #endif // #ifdef I_SUPPORT_SECURITY

                                            BYTE i;
                                            for (i = 0; i < 8; i++)
                                            {
                                                currentNeighborRecord.longAddr.v[i] = \
                                                    nwkIEEESrcAddress.v[i];
                                            }
                                            PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord);
                                            doingRejoinCommand = FALSE;
                                            TxBuffer[TxData++] = nwkSourceAddress.byte.LSB;
                                            TxBuffer[TxData++] = nwkSourceAddress.byte.MSB;
                                            goto RejoinSuccess;

                                        }
                                        rejoinStatus        = FALSE;
                                        doingRejoinCommand  = FALSE;
                                        goto sendBackRJRsp;

RejoinSuccess:
                                        TxBuffer[TxData++] = SUCCESS;


sendBackRJRsp:
                                        GetMACAddress(&tempLongAddress);
                                        for(i = 0; i < 8; i++)
                                        {
                                            TxBuffer[TxHeader--] = tempLongAddress.v[7-i];
                                        }
                                        for(i = 0; i < 8; i++)
                                        {
                                            TxBuffer[TxHeader--] = nwkIEEESrcAddress.v[7-i];
                                        }
                                        TxBuffer[TxHeader--] = NLME_GET_nwkBCSN();
                                        TxBuffer[TxHeader--] = 1;   // radius of 1, as per errata
                                        TxBuffer[TxHeader--] = macPIB.macShortAddress.byte.MSB;
                                        TxBuffer[TxHeader--] = macPIB.macShortAddress.byte.LSB;
                                        // A ZigBee 2006 requirement: This needs to be old short address of the device that requested
                                        // the rejoin. Because it is been sent a response, before the change over

                                        TxBuffer[TxHeader--] = oldShortAddr.byte.MSB;
                                        TxBuffer[TxHeader--] = oldShortAddr.byte.LSB;


                                        #ifdef I_SUPPORT_SECURITY
                                            if (nwkFrameControlMSB.bits.security)
                                            {
                                                TxBuffer[TxHeader--] = 0x02 | NWK_IEEE_SRC_ADDR | NWK_IEEE_DST_ADDR;
                                                rejoinIndication.secureJoin = TRUE;
                                            }
                                            else
                                        #endif // #ifdef I_SUPPORT_SECURITY
                                            {
                                                TxBuffer[TxHeader--] = NWK_IEEE_SRC_ADDR | NWK_IEEE_DST_ADDR; // nwkFrameControlMSB TODO check security setting for this
                                                rejoinIndication.secureJoin = FALSE;
                                            }

                                        TxBuffer[TxHeader--] = NWK_FRAME_CMD | (nwkProtocolVersion<<2);    // nwkFrameControlLSB

                                        if(rejoinStatus)
                                        {
                                            //nwkStatus.flags.bits.bRejoinIndication = 1;
                                            rejoinIndicationPending = 1;
                                        }
                                        else  // don't send up any indication if request fails
                                        {
                                            //nwkStatus.flags.bits.bRejoinIndication = 0;
                                            rejoinIndicationPending = 0;
                                        }

                                        rejoinIndication.CapabilityInformation.Val = params.MLME_ASSOCIATE_indication.CapabilityInformation.Val;
                                        rejoinIndication.ShortAddress.Val = currentNeighborRecord.shortAddr.Val;
                                        //  A ZigBee 2006 requirement: - For the rejoin, the proper new address must be stored
                                        // so that it can be used at the NLME_JOIN_INDICATION that is upcoming

                                        rejoinIndication.ExtendedAddress = nwkIEEESrcAddress;

                                        // Send it to old short address of requesting device.  See Specs
                                        Prepare_MCPS_DATA_request((WORD )oldShortAddr.Val, &i );
                                        msduHandle = i;

                                        // A ZigBee 2006 requirement: if we are sending this to a RFD, then buffer it and wait
                                        // for its data request before we send the rejoin command response

                                        if( /*(!params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.DeviceType)  ||*/
                                           (!params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.RxOnWhenIdle) )
                                        {
                                            params.MCPS_DATA_request.TxOptions.bits.indirect_transmission = 1;
                                        }

                                        return MCPS_DATA_request;
                                    }
                                #endif // #if !defined(I_AM_END_DEVICE)
                            } // end of switch(i)

                            #ifdef I_SUPPORT_ROUTING
                                // Check for routing commands.  If MLME-START.request has not come back successful,
                                // we cannot perform any routing functions.
                                if (nwkStatus.flags.bits.bCanRoute)
                                {
                                    // See if the packet is a route request
                                    #ifdef I_AM_END_DEVICE
                                        NWKDiscardRx();
                                        return NO_PRIMITIVE;
                                    #else // #ifdef I_AM_END_DEVICE
                                        if (i == NWK_COMMAND_ROUTE_REQUEST)
                                        {
                                            if( nwkFrameControlMSB.bits.dstIEEEAddr )
                                            {
                                                #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                                    #ifndef I_AM_END_DEVICE
                                                        if(IsThereAddressConflict( nwkDestinationAddress , &nwkIEEEDstAddress ))
                                                        {
                                                           // address conflict detected
                                                            return NLME_NWK_STATUS_indication;
                                                        }
                                                    #endif // #ifndef I_AM_END_DEVICE
                                                #endif // #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                            }
                                            goto HandleRouteRequest;
                                        }
                                    #endif // #ifdef I_AM_END_DEVICE

                                    if ((i == NWK_COMMAND_NWK_STATUS) && ZigBeeReady())
                                    {
                                        BYTE            nwkStatusCode;
                                        if( nwkFrameControlMSB.bits.dstIEEEAddr )
                                        {
                                            #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                                #ifndef I_AM_END_DEVICE
                                                    if(IsThereAddressConflict( nwkDestinationAddress , &nwkIEEEDstAddress ))
                                                    {
                                                       // address conflict detected
                                                        return NLME_NWK_STATUS_indication;
                                                    }
                                                #endif // #ifndef I_AM_END_DEVICE
                                            #endif // #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                        }
                                        nwkStatusCode = NWKGet();          //errorCode
                                        if (( nwkStatusCode == NWK_STATUS_NO_ROUTE_AVAILABLE ||
                                             nwkStatusCode == NWK_STATUS_TREE_LINK_FAILURE ||
                                             nwkStatusCode == NWK_STATUS_NONTREE_LINK_FAILURE ||
                                             nwkStatusCode == NWK_STATUS_NO_ROUTING_CAPACITY ||
                                             nwkStatusCode == NWK_STATUS_MANY_TO_ONE_ROUTE_FAILURE ||
                                             nwkStatusCode == NWK_STATUS_SOURCE_ROUTE_FAILURE ))
                                        {

                                            SHORT_ADDR      routeDestAddr;
                                            BYTE            SendRRERFrame;
                                            SHORT_ADDR      nextHopAddr;

                                            // Read out the NWK payload.  We will probably retransmit this, so
                                            // load it into the transmit buffer.
                                            routeDestAddr.byte.LSB  = NWKGet();
                                            routeDestAddr.byte.MSB  = NWKGet();

                                            
                                            if (nwkDestinationAddress.Val == macPIB.macShortAddress.Val)
                                            {
                                                if (( nwkStatusCode == NWK_STATUS_MANY_TO_ONE_ROUTE_FAILURE )||
                                                (nwkStatusCode ==  NWK_STATUS_SOURCE_ROUTE_FAILURE))
                                                {
                                                    params.NLME_NWK_STATUS_indication.nwkStatusCode = nwkStatusCode;
                                                    params.NLME_NWK_STATUS_indication.ShortAddress = routeDestAddr;
                                                    return NLME_NWK_STATUS_indication;
                                                }
                                                else
                                                {
                                                    // Device was the originator of the Route Request.
                                                    // Therefore let the request time out and have the background
                                                    // processing clean up to conserve space.
                                                    TxData = TX_DATA_START;
                                                    NWKDiscardRx();
                                                    return NO_PRIMITIVE;
                                                }
                                            }
                                            else
                                            {
                                                BYTE rtStatus;
                                                BYTE i;
                                                BYTE includeDstIEEEAddr = FALSE;

                                                if ( ! ( nwkRadius > 1 ) )
                                                {
                                                    NWKDiscardRx();
                                                    return NO_PRIMITIVE;
                                                }

                                                SendRRERFrame = GetNextHop
                                                                (
                                                                    nwkDestinationAddress,
                                                                    &nextHopAddr,
                                                                    &rtStatus
                                                                );
                                                #ifdef  ZIGBEE_PRO
                                                    if( SendRRERFrame == FALSE )
                                                    {
                                                       if( NWKFindRandomNeighbor(0,routeDestAddr, routeDestAddr) != INVALID_NEIGHBOR_KEY )
                                                       {
                                                           nextHopAddr = currentNeighborRecord.shortAddr;
                                                       }

                                                    }
                                                #endif // #ifdef  ZIGBEE_PRO
                                                    if (SendRRERFrame)
                                                    {
                                                        // Update the TxBuffer with the Network Header for Route Error Command
                                                        /*CreateNwkCommandHeader
                                                        (
                                                            nwkSequenceNumber,
                                                            nwkRadius - 1,
                                                            nwkSourceAddress,
                                                            nwkDestinationAddress
                                                        );*/

                                                        /*Add SrcIEEEAddress*/
                                                        if( nwkFrameControlMSB.bits.srcIEEEAddr )
                                                        {
                                                            for (i=0; i<8; i++)
                                                            {
                                                                TxBuffer[TxHeader--] = nwkIEEESrcAddress.v[7-i];
                                                            }
                                                        }

                                                        /*Add DstIEEEAddress*/
                                                        if( nwkFrameControlMSB.bits.dstIEEEAddr )
                                                        {
                                                            includeDstIEEEAddr = TRUE;
                                                            for (i=0; i<8; i++)
                                                            {
                                                                TxBuffer[TxHeader--] = nwkIEEEDstAddress.v[7-i];
                                                            }
                                                        }

                                                        TxBuffer[TxHeader--] = nwkSequenceNumber;
                                                        TxBuffer[TxHeader--] = nwkRadius - 1;
                                                        TxBuffer[TxHeader--] = nwkSourceAddress.byte.MSB;
                                                        TxBuffer[TxHeader--] = nwkSourceAddress.byte.LSB;
                                                        TxBuffer[TxHeader--] = nwkDestinationAddress.byte.MSB;
                                                        TxBuffer[TxHeader--] = nwkDestinationAddress.byte.LSB;
                                                        /*Update the frame control fields -
                                                        like Security, SrcIEEEAddress, DstIEEEAddress, and ProtocolVerison bits*/
                                                        UpdateNwkHeaderFrameContolFields(includeDstIEEEAddr);

                                                        // Update the TxBuffer with the Network Payload for Nwk Status Command*/
                                                        CreateNwkCommandNwkStatus
                                                        (
                                                            nwkStatusCode,
                                                            routeDestAddr
                                                        );

                                                        // Load up the MCPS_DATA.request parameters.
                                                         NWKDiscardRx();
                                                        Prepare_MCPS_DATA_request( nextHopAddr.Val, &i );
                                                        return MCPS_DATA_request;
                                                    }
                                                    else
                                                    {
                                                         NWKDiscardRx();
                                                        ZigBeeUnblockTx(); // Is this required? Assuming somewhere it is blocked?
                                                        return NO_PRIMITIVE;
                                                    }
                                            }
                                        }
                                    }

                                    // See if the packet is a route reply
                                    else if (i == NWK_COMMAND_ROUTE_REPLY)
                                    {
                                        BOOL                    forwardRouteReply;
                                        BYTE                    rdIndex;
                                        ROUTE_REPLY_COMMAND     rrep;
                                        BYTE                    commandOptions;

                                        if( nwkFrameControlMSB.bits.dstIEEEAddr )
                                        {
                                            #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                                #ifndef I_AM_END_DEVICE
                                                    if(IsThereAddressConflict( nwkDestinationAddress , &nwkIEEEDstAddress ))
                                                    {
                                                        // address conflict detected
                                                        return NLME_NWK_STATUS_indication;
                                                    }
                                                #endif // #ifndef I_AM_END_DEVICE
                                            #endif // #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                        }
                                        forwardRouteReply = FALSE;
                                        rdIndex = INVALID_ROUTE_DISCOVERY_INDEX;

                                        // Get the Route Reply command frame
                                        #if defined(__C30__) || defined(__C32__)
                                            rrep.commandFrameIdentifier = NWK_COMMAND_ROUTE_REPLY;
                                            commandOptions              = NWKGet();
                                            rrep.commandOptions         = commandOptions;
                                            rrep.routeRequestIdentifier = NWKGet();
                                            rrep.originatorAddress.v[0] = NWKGet();
                                            rrep.originatorAddress.v[1] = NWKGet();
                                            rrep.responderAddress.v[0]  = NWKGet();
                                            rrep.responderAddress.v[1]  = NWKGet();
                                            rrep.pathCost               = NWKGet();

                                            #if ( ZIGBEE_PRO == 0x01)
                                                if ( commandOptions & ORIGINATOR_IEEE_ADDR_BIT )
                                                {
                                                    for (i=0; i<8; i++)
                                                    {
                                                        rrep.originatorIEEEAddress.v[i] = NWKGet();
                                                    }
                                                }
                                                if ( commandOptions & RESPONDER_IEEE_ADDR_BIT )
                                                {
                                                    for (i=0; i<8; i++)
                                                    {
                                                        rrep.responderIEEEAddress.v[i] = NWKGet();
                                                    }
                                                }
                                            #endif // #if ( ZIGBEE_PRO == 0x01)
                                        #else
                                            for ( ptr = (BYTE *)&rrep, *ptr++ = NWK_COMMAND_ROUTE_REPLY, i = 1,
                                                  commandOptions = *ptr;
                                                  i< sizeof(ROUTE_REPLY_COMMAND);
                                                  *ptr++ = NWKGet(), i++ ) {}
                                            #if ( ZIGBEE_PRO == 0x01)
                                                if ( commandOptions & ORIGINATOR_IEEE_ADDR_BIT )
                                                {
                                                    for (i=0; i<8; i++)
                                                    {
                                                        *ptr++ = NWKGet();
                                                    }
                                                }
                                                if ( commandOptions & RESPONDER_IEEE_ADDR_BIT )
                                                {
                                                    for (i=0; i<8; i++)
                                                    {
                                                        *ptr++ = NWKGet();
                                                    }
                                                }
                                            #endif // #if ( ZIGBEE_PRO == 0x01)
                                        #endif // #if defined(__C30__)

                                        #ifndef USE_TREE_ROUTING_ONLY
                                            if (HaveRoutingCapacity( TRUE, rrep.routeRequestIdentifier, nwkSourceAddress, rrep.responderAddress, rrep.commandOptions ))
                                            {
                                                // Try to find the matching route discovery entry.
                                                for (rdIndex = 0;
                                                     (rdIndex < ROUTE_DISCOVERY_TABLE_SIZE) &&
                                                     !((routeDiscoveryTablePointer[rdIndex] != NULL) &&
                                                     ((routeDiscoveryTablePointer[rdIndex]->routeRequestID == rrep.routeRequestIdentifier) &&
                                                      (routeDiscoveryTablePointer[rdIndex]->srcAddress.Val == rrep.originatorAddress.Val)));
                                                     rdIndex++ ) {}

                                                if (rdIndex != ROUTE_DISCOVERY_TABLE_SIZE)  //TableEntriesExist
                                                {
                                                    #ifdef USE_EXTERNAL_NVM
                                                        GetRoutingEntry( &currentRoutingEntry, routingTable + (WORD)routeDiscoveryTablePointer[rdIndex]->routingTableIndex * (WORD)sizeof(ROUTING_ENTRY) );
                                                    #else
                                                        GetRoutingEntry( &currentRoutingEntry, &routingTable[routeDiscoveryTablePointer[rdIndex]->routingTableIndex] );
                                                    #endif // #ifdef USE_EXTERNAL_NVM
                                                    currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_ACTIVE;
                                                    #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
                                                        currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                                                    #endif
                                                    if (rrep.pathCost < routeDiscoveryTablePointer[rdIndex]->residualCost)
                                                    {
                                                        routeDiscoveryTablePointer[rdIndex]->residualCost = rrep.pathCost;
                                                        currentRoutingEntry.nextHop = params.MCPS_DATA_indication.SrcAddr.ShortAddr;
                                                    }

                                                    currentRoutingEntry.rtflags.rtbits.bManyToOne = FALSE;
                                                    currentRoutingEntry.rtflags.rtbits.bNoRouteCache = FALSE;
                                                    currentRoutingEntry.rtflags.rtbits.bRouteRecordRequired = FALSE;

                                                    if (rrep.commandOptions & MULTICAST_BIT)
                                                    {
                                                        currentRoutingEntry.rtflags.rtbits.bGroupIDFlag = 1;
                                                    }

                                                    #ifdef USE_EXTERNAL_NVM
                                                        PutRoutingEntry( routingTable + (WORD)routeDiscoveryTablePointer[rdIndex]->routingTableIndex * (WORD)sizeof(ROUTING_ENTRY), &currentRoutingEntry  );
                                                    #else
                                                        PutRoutingEntry( &routingTable[routeDiscoveryTablePointer[rdIndex]->routingTableIndex], &currentRoutingEntry  );
                                                    #endif // #ifdef USE_EXTERNAL_NVM
                                                    if (macPIB.macShortAddress.Val != rrep.originatorAddress.Val) //I am not Destination
                                                    {
                                                        forwardRouteReply = TRUE;
                                                        rrep.pathCost += routeDiscoveryTablePointer[rdIndex]->previousCost;
                                                    }
                                                }
                                            }
                                            else // Do not have routing capacity
                                        #endif // #ifndef USE_TREE_ROUTING_ONLY
                                            {
                                                if (macPIB.macShortAddress.Val != nwkDestinationAddress.Val) //!IAmDestination
                                                {
                                                    rrep.pathCost += CONSTANT_PATH_COST; // We don't know the previous cost, so use a constant cost.
                                                    forwardRouteReply = TRUE;
                                                }
                                            }

                                        if (forwardRouteReply && ZigBeeReady())
                                        {
                                            BYTE rrepCommandOptions;
                                            NWKDiscardRx();
                                            #ifndef USE_TREE_ROUTING_ONLY
                                                if (rdIndex != INVALID_ROUTE_DISCOVERY_INDEX)
                                                {
                                                    macAddress = routeDiscoveryTablePointer[rdIndex]->senderAddress;
                                                }
                                                else
                                            #endif // #ifndef USE_TREE_ROUTING_ONLY
                                            #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
                                                {
                                                    if (!RouteAlongTree( nwkSourceAddress, &macAddress ))
                                                    {
                                                        return NO_PRIMITIVE;
                                                    }
                                                }
                                            #endif // #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)

                                            // Prepare the Route Reply message.
                                            // Load up the NWK payload - the route reply command frame.
                                            #if defined(__C30__) || defined(__C32__)
                                                TxData = 0x00;
                                                TxBuffer[TxData++] = rrep.commandFrameIdentifier;
                                                rrepCommandOptions = rrep.commandOptions;
                                                TxBuffer[TxData++] = rrep.commandOptions;
                                                TxBuffer[TxData++] = rrep.routeRequestIdentifier;
                                                TxBuffer[TxData++] = rrep.originatorAddress.v[0];
                                                TxBuffer[TxData++] = rrep.originatorAddress.v[1];
                                                TxBuffer[TxData++] = rrep.responderAddress.v[0];
                                                TxBuffer[TxData++] = rrep.responderAddress.v[1];
                                                TxBuffer[TxData++] = rrep.pathCost;
                                            #else
                                                for ( ptr = (BYTE *)&rrep, i = 0; i< sizeof(ROUTE_REPLY_COMMAND); i++ )
                                                {
                                                    if ( i == 1 )
                                                    {
                                                        rrepCommandOptions = *ptr;
                                                    }
                                                    TxBuffer[TxData++] = *ptr++;
                                                }
                                            #endif // #if defined(__C30__) || defined(__C32__)

                                            #if ( ZIGBEE_PRO == 0x01)
                                                if ( rrepCommandOptions & ORIGINATOR_IEEE_ADDR_BIT )
                                                {
                                                    for (i=0; i<8; i++)
                                                    {
                                                        #if defined(__C30__) || defined(__C32__)
                                                            TxBuffer[TxData++] = rrep.originatorIEEEAddress.v[i];
                                                        #else
                                                            TxBuffer[TxData++] = *ptr++;
                                                        #endif // #if defined(__C30__) || defined(__C32__)
                                                    }
                                                }
                                                if ( rrepCommandOptions & RESPONDER_IEEE_ADDR_BIT )
                                                {
                                                    for (i=0; i<8; i++)
                                                    {
                                                        #if defined(__C30__) || defined(__C32__)
                                                            TxBuffer[TxData++] = rrep.responderIEEEAddress.v[i];
                                                        #else
                                                            TxBuffer[TxData++] = *ptr++;
                                                        #endif // #if defined(__C30__) || defined(__C32__)
                                                    }
                                                }
                                            #endif // #if ( ZIGBEE_PRO == 0x01)

                                            // Update the TxBuffer with the Network Header for Route Reply Command
                                            CreateNwkCommandHeader
                                            (
                                                nwkSequenceNumber,
                                                nwkRadius - 1,
                                                macPIB.macShortAddress,
                                                macAddress
                                            );

                                            // Load up the MCPS_DATA.request parameters.
                                            Prepare_MCPS_DATA_request( macAddress.Val, &i );
                                            return MCPS_DATA_request;
                                        }
                                        // For ZigBee 2006: Route Request has been successfully recieved
                                        // so send NLME_ROUTE_DISCOVERY_confirm to upper layer (new primitive)

                                        NWKDiscardRx();
                                        params.NLME_ROUTE_DISCOVERY_confirm.Status = SUCCESS;
                                        return NLME_ROUTE_DISCOVERY_confirm;
                                    } // (i == NWK_COMMAND_ROUTE_REPLY)
                                } // (nwkStatus.flags.bits.bCanRoute)

                                // I cannot route the frame, so discard it.
                                NWKDiscardRx();
                                return NO_PRIMITIVE;
                            #endif // I_SUPPORT_ROUTING
                        } // if (nwkFrameControlLSB.bits.frameType == NWK_FRAME_CMD)

                        else // // The current frame is a data frame.
                        {
                            #ifdef I_SUPPORT_ROUTING
                                // The current frame is a data frame.
                                BYTE  routingStatus = 0;
                            #endif // #ifdef I_SUPPORT_ROUTING

                            /*If the device is currently in the process of Rejoin 
                            procedure, then reject any of the data frame that is received.
                            The data frame should be processed only if hte Rejoin Procedure is Successful*/
                           if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
                            if ( 
                                ( ZigBeeStatus.flags.bits.bActiveScanToRejoin ) || 
                                ( nwkStatus.flags.bits.bRejoinInProgress ) || 
                                ( nwkStatus.flags.bits.bRejoinScan ) )
                            {
                                NWKDiscardRx();
                                return NO_PRIMITIVE;
                            }
                           }//#endif
                            
                            if( nwkFrameControlMSB.bits.multicastFlag )
                            {
                                #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                    if( ( GetEndPointsFromGroup(nwkDestinationAddress) != MAX_GROUP ) )
                                    {
                                        // Since we are part of group and the received frame is a multicast frame,
                                        // We have to start broadcasting the multicast frame as Member Mode Multicast.
                                        // Also we need to send Data Indication to higher layer.
                                        // This is not supported currently.

                                        // Set some flag so that we can initiate broadcast of multicast frame as Member Mode.
                                        // This is currently not supported.

                                        // Updation of DstAddr field in the NLDE_DATA_indication is must.
                                        // This is because, APS Layer group address mapping is done based
                                        // on the DstAddr field. Since, DstAddr in MCPS_DATA_indication and
                                        // DstAddr field in NLDE_DATA_indication does not overlay, we need to
                                        // explicity assign DstAddr in NLDE_DATA_indication with DstAddr in
                                        // MCPS_DATA_indication.
                                        params.NLDE_DATA_indication.DstAddr.Val = nwkDestinationAddress.Val;
                                        params.NLDE_DATA_indication.SrcAddress.Val = nwkSourceAddress.Val;

                                        params.NLDE_DATA_indication.isMulticastFrame = 0x01;
                                        // These overlay - params.NLDE_DATA_indication.NsduLength = params.MCPS_DATA_indication.msduLength;
                                        // These overlay - params.NLDE_DATA_indication.LinkQuality = params.MCPS_DATA_indication.mpduLinkQuality;
                                        return NLDE_DATA_indication;
                                    }
                                    else
                                    {
                                        // We are not part of group. We have to forward the packt as unicast. (Non Member Mode multicast)
                                        // Here we have to check the Routing Table for Multicast group entry (nwkDestinationAddress)
                                        // If routing table entry exists for this group address, then we have to forward to the next
                                        // hop device. Otherwise we have to drop the frame.
                                        // This is done in the next set of block.
                                    }
                                #endif

                                // IF I DO NOT SUPPORT MULTICAST, THEN WE HAVE FORWARD THE PACKET.
                                // Here we have to check the Routing Table for Multicast group entry (nwkDestinationAddress)
                                // If routing table entry exists for this group address, then we have to forward to the next
                                // hop device. Otherwise we have to drop the frame.
                                // This is done in the next set of block.
                            }

                            else if ((nwkDestinationAddress.Val == macPIB.macShortAddress.Val) ||
                                     (nwkDestinationAddress.Val == 0xFFFF))
                            {
                                // I am the final destination for this packet.  Send up an NLDE-DATA.indication.
                                // Do not discard the packet - the upper layers need it.
                                // Note: The macro I_SUPPORT_MANY_TO_ONE_HANDLING can be uncommented if we don't need
                                // processing of  source route sub frame on the end device and final destination router
                                // where source route  information is redundant
                                // #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                    if( nwkFrameControlMSB.bits.sourceRoute)
                                    {
                                        // Free up the relay list created for source routed sub frame
                                        if(pRelaylist != NULL )
                                        {
                                            SRAMfree( pRelaylist);
                                        }
                                    }
                               // #endif //  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                params.NLDE_DATA_indication.SrcAddress = nwkSourceAddress;
                                // For ZigBee 2006: Must use short address at this layer
                                params.NLDE_DATA_indication.DstAddr.Val   = nwkDestinationAddress.Val;
                                params.NLDE_DATA_indication.DstAddrMode   = APS_ADDRESS_16_BIT;
                                params.NLDE_DATA_indication.isMulticastFrame = 0x00;
                                
                                
                                params.NLDE_DATA_indication.NsduLength = params.MCPS_DATA_indication.msduLength;
                                params.NLDE_DATA_indication.Nsdu       = params.MCPS_DATA_indication.msdu;
                                params.NLDE_DATA_indication.LinkQuality = params.MCPS_DATA_indication.mpduLinkQuality;
                                return NLDE_DATA_indication;

                            }
                            #ifdef I_SUPPORT_ROUTING
                                // The packet needs to be routed.  We can route it if the radius has not reached 0,
                                // MLME-START.request has come back successful, and we're not blocked.
                                #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                    else if( nwkFrameControlMSB.bits.sourceRoute)
                                    {
                                        BYTE count;
                                        // This relayIndex is initialized to 1 less than    the relay count by
                                        // the originator of the packet, and is decremented by 1 by each
                                        // receiving relay. The receiving device should forward the packet to
                                        // address at relay index
                                        if ( relayIndex > 0 )
                                        {
                                            if(( pRelaylist[( 2 * relayIndex ) ] == macPIB.macShortAddress.byte.LSB )
                                            &  ( pRelaylist[( 2 * relayIndex ) + 1 ] == macPIB.macShortAddress.byte.MSB ))
                                            {
                                                relayIndex--;
                                                macAddress.byte.LSB =
                                                pRelaylist[ ( 2 * relayIndex )];
                                                macAddress.byte.MSB =
                                                pRelaylist[( 2 * relayIndex ) + 1];
                                            }
                                            else
                                            {
                                                // the address at relay index does not match with ours, drop the frame
                                                if(pRelaylist != NULL )
                                                {
                                                    SRAMfree( pRelaylist);
                                                }
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;
                                             }
                                        }
                                        else
                                        {
                                            // relay index is zero . forward the data to network destination specified
                                            // in the packet
                                            macAddress = nwkDestinationAddress;
                                           //   nwkFrameControlMSB.bits.sourceRoute = 0;
                                        }
                                            // Fill the TxBuffer with source route sub frame backward
                                        // add the relay list
                                        // Note: TxBuffer updation should be removed from here and put in
                                        // if ( relayIndex > 0 ) block if we do not want to add source route
                                        // subframe to final destination.
                                        for( count = 2 * relayCount ; count > 0  ; count-- )
                                        {
                                            TxBuffer[TxHeader--] = pRelaylist[count - 1];

                                        }
                                        // Add relay index and count
                                        TxBuffer[TxHeader--] = relayIndex;
                                        TxBuffer[TxHeader--] = relayCount;
                                        if(pRelaylist != NULL )
                                        {
                                            SRAMfree( pRelaylist);
                                        }
                                        routingStatus = ROUTE_AS_PER_SOURCE_ROUTE_FRAME;
                                   }
                                #endif // #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                else if (nwkStatus.flags.bits.bCanRoute && (nwkRadius > 1) && ZigBeeReady())
                                {
                                    if( nwkFrameControlMSB.bits.multicastFlag )
                                    {
                                        if ( GetMulticastRoutingEntry( nwkDestinationAddress, &macAddress, &routingStatus ) == FALSE )
                                        {
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                        }
                                        routingStatus = ROUTE_SEND_TO_MAC_ADDRESS;
                                    }
                                    else
                                    {
                                        // When we receive data from child end device and if the final destination is a concentrator,
                                        // then we always have to send Route Record frame to concentrator before sending the actual data.
                                        // Even though concentrator is High RAM or Low RAM, we have to send route record only when we
                                        // receive data from child end device to concentrator.

                                        // By calling this function, we will come to know whether the destination device
                                        // is a concentrator or not by making use of routeStatus parameter which will be updated automatically.
                                        GetDestFromRoutingTable( nwkDestinationAddress, &macAddress, &routingStatus );

                                        #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                            // Check whether the initiator is child end device.
                                            if ((NWKLookupNodeByShortAddrVal( nwkSourceAddress.Val ) != INVALID_NEIGHBOR_KEY) &&
                                                (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD) &&
                                                (currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE) &&
                                                ((routingStatus == NO_ROUTE_RECORD_REQUIRED) || (routingStatus == ROUTE_RECORD_REQUIRED)))
                                            {
                                                // As mentioned above, we should always send Route Record first, and then the actual data.
                                                routingStatus = ROUTE_DATA_AFTER_ROUTE_RECORD;
                                            }
                                            else

                                        #endif // #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                            {
                                                // Since the initiator is not child end device get the routing address for the destination device.
                                                routingStatus =  GetRoutingAddress( FALSE, nwkDestinationAddress, nwkFrameControlLSB.bits.routeDiscovery, &macAddress );
                                            }
                                    }
                                }
                                else
                                {
                                    // Cannot Route the Packet.
                                    // Either Radius is less or equal to one or
                                    // Do not have Routing ability
                                    // Currently some transmission is being made. ZigBeeReady()  has returned false.
                                    NWKDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                                switch( routingStatus )
                                {
                                    BYTE routeStatus;
                                    case ROUTE_SEND_TO_MAC_ADDRESS: //fall through, forward data in the available path
                                    case ROUTE_AS_PER_SOURCE_ROUTE_FRAME:
                                    case ROUTE_AVAILABLE_TO_CONCENTRATOR:

                                        if ( nwkFrameControlMSB.bits.multicastFlag )
                                        {
                                            if ( nwkMulticast.bits.nonMemberRadius > 1 )
                                            {
                                                nwkMulticast.bits.nonMemberRadius--;
                                            }
                                            else
                                            {
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;
                                            }
                                            TxBuffer[TxHeader--] = nwkMulticast.Val;
                                        }
                                        
                                        /* put back the long addresses if they were present */
                                        if( nwkFrameControlMSB.bits.srcIEEEAddr )
                                        {
                                            for(i = 0; i < 8; i++)
                                            {
                                                TxBuffer[TxHeader--] = nwkIEEESrcAddress.v[7-i];
                                            }
                                        }
                                        if( nwkFrameControlMSB.bits.dstIEEEAddr )
                                        {
                                            for(i = 0; i < 8; i++)
                                            {
                                                TxBuffer[TxHeader--] = nwkIEEEDstAddress.v[7-i];
                                            }
                                        }
                                        
                                        /* put back the fixed header fields as well */
                                        TxBuffer[TxHeader--] = nwkSequenceNumber;
                                        TxBuffer[TxHeader--] = nwkRadius - 1;
                                        TxBuffer[TxHeader--] = nwkSourceAddress.byte.MSB;
                                        TxBuffer[TxHeader--] = nwkSourceAddress.byte.LSB;
                                        TxBuffer[TxHeader--] = nwkDestinationAddress.byte.MSB;
                                        TxBuffer[TxHeader--] = nwkDestinationAddress.byte.LSB;
                                        TxBuffer[TxHeader--] = nwkFrameControlMSB.Val;
                                        TxBuffer[TxHeader--] = nwkFrameControlLSB.Val;

                                        // Load up the NWK payload
                                        while (params.MCPS_DATA_indication.msduLength)
                                        {
                                            TxBuffer[TxData++] = NWKGet();
                                        }

                                        // Load up the MCPS_DATA.request parameters
                                        Prepare_MCPS_DATA_request( macAddress.Val, &j );
                                        // find the proper route handle record
                                        for(i = 0; i < NUM_BUFFERED_ROUTING_MESSAGES; i++)
                                        {
                                            // Find free entry
                                            if(nwkStatus.routeHandleRecords[i].SourceAddress.Val == 0xFFFF)
                                            {
                                                nwkStatus.routeHandleRecords[i].DstAddress.Val = nwkDestinationAddress.Val;
                                                nwkStatus.routeHandleRecords[i].SourceAddress.Val = nwkSourceAddress.Val;
                                                nwkStatus.routeHandleRecords[i].nwkSequence = nwkSequenceNumber;
                                                nwkStatus.routeHandleRecords[i].macSequence = j;
                                                nwkStatus.routeHandleRecords[i].isToConcentrator = routingStatus;
                                                nwkStatus.routeHandleRecords[i].macSourceAddress   = macAddress;
                                                break;
                                            }
                                        }
                                        NWKDiscardRx();
                                        return MCPS_DATA_request;
                                    break;

                                    #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                        // Send route record on behalf of child end device, then send data
                                        case ROUTE_DATA_AFTER_ROUTE_RECORD:
                                            if( DataToBeSendAfterRouteRecord.DataBackUp == NULL )
                                            {
                                                BYTE *ptrBackUp;
                                                BYTE i;
                                                BYTE includeDstIEEEAddr = FALSE;
                                                LONG_ADDR tempLongAddr;

                                                BYTE dataLength = params.NLDE_DATA_indication.NsduLength;
                                                ptrBackUp = SRAMalloc( MAX_DATA_SIZE);
                                                if( ptrBackUp == NULL )
                                                {
                                                    NWKDiscardRx();
                                                    return NO_PRIMITIVE;
                                                }
                                                else
                                                {
                                                    DataToBeSendAfterRouteRecord.DataBackUp = ptrBackUp;
                                                    DataToBeSendAfterRouteRecord.routeRecordSend = TRUE;
                                                    // Buffer the current message.  Create the NWK header first.
                                                    *ptrBackUp++ = nwkFrameControlLSB.Val;
                                                    *ptrBackUp++ = nwkFrameControlMSB.Val; // Frame Control byte MSB
                                                    *ptrBackUp++ = nwkDestinationAddress.byte.LSB;
                                                    *ptrBackUp++ = nwkDestinationAddress.byte.MSB;
                                                    *ptrBackUp++ = nwkSourceAddress.byte.LSB;
                                                    *ptrBackUp++ = nwkSourceAddress.byte.MSB;
                                                    *ptrBackUp++ = nwkRadius;
                                                    *ptrBackUp++ = nwkSequenceNumber;
                                                    
                                                    /* put back the long addresses if they were present */
                                                    if( nwkFrameControlMSB.bits.dstIEEEAddr )
                                                    {
                                                        for(i = 0; i < 8; i++)
                                                        {
                                                            *ptrBackUp++ = nwkIEEEDstAddress.v[i];
                                                        }
                                                    }
                                                    if( nwkFrameControlMSB.bits.srcIEEEAddr )
                                                    {
                                                        for(i = 0; i < 8; i++)
                                                        {
                                                            *ptrBackUp++ = nwkIEEESrcAddress.v[i];
                                                        }
                                                    }
                                                    
                                                    // Load up the NWK payload
                                                     while (dataLength--)
                                                     {
                                                         *ptrBackUp++ = NWKGet();
                                                     }
                                                     DataToBeSendAfterRouteRecord.dataLength =
                                                     ptrBackUp -  DataToBeSendAfterRouteRecord.DataBackUp ;

                                                    DataToBeSendAfterRouteRecord.macDestination = macAddress;
                                                    /* Route record frame command header */
                                                    /*CreateNwkCommandHeader
                                                    (
                                                        NLME_GET_nwkBCSN(),
                                                        DEFAULT_RADIUS,
                                                        nwkSourceAddress,
                                                        nwkDestinationAddress
                                                    );*/

                                                    /*Add SrcIEEEAddress*/
                                                    if ( NWKLookupNodeByShortAddrVal( nwkSourceAddress.Val ) != INVALID_NEIGHBOR_KEY )
                                                    {
                                                        for (i=0; i<8; i++)
                                                        {
                                                            TxBuffer[TxHeader--] = currentNeighborRecord.longAddr.v[7-i];
                                                        }
                                                    }

                                                    /*Add DstIEEEAddress*/
                                                    if ( IsThisAddressKnown( nwkDestinationAddress, (BYTE *)&tempLongAddr ) )
                                                    {
                                                        includeDstIEEEAddr = TRUE;
                                                        for (i=0; i<8; i++)
                                                        {
                                                            TxBuffer[TxHeader--] = tempLongAddr.v[i];
                                                        }
                                                    }

                                                    TxBuffer[TxHeader--] = NLME_GET_nwkBCSN();
                                                    TxBuffer[TxHeader--] = DEFAULT_RADIUS;
                                                    TxBuffer[TxHeader--] = nwkSourceAddress.byte.MSB;
                                                    TxBuffer[TxHeader--] = nwkSourceAddress.byte.LSB;
                                                    TxBuffer[TxHeader--] = nwkDestinationAddress.byte.MSB;
                                                    TxBuffer[TxHeader--] = nwkDestinationAddress.byte.LSB;
                                                    /*Update the frame control fields -
                                                    like Security, SrcIEEEAddress, DstIEEEAddress, and ProtocolVerison bits*/
                                                    UpdateNwkHeaderFrameContolFields(includeDstIEEEAddr);

                                                    /* Route record frame command payload */
                                                    TxBuffer[TxData++] = NWK_COMMAND_ROUTE_RECORD;
                                                    TxBuffer[TxData++] = 0x01;
                                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                                    // Load up the MCPS_DATA.request parameters.
                                                    Prepare_MCPS_DATA_request( macAddress.Val, &DataToBeSendAfterRouteRecord.msduHandle );
                                                    NWKDiscardRx();
                                                    return MCPS_DATA_request;
                                                }
                                            }
                                            else
                                            {
                                                NWKDiscardRx();
                                                return NO_PRIMITIVE;

                                            }
                                        break;
                                    #endif //  I_SUPPORT_MANY_TO_ONE_HANDLING
                                    case ROUTE_DISCOVERY_ALREADY_INITIATED:
                                    case ROUTE_MESSAGE_BUFFERED:
                                         // Send the Route Request
                                         NWKDiscardRx();
                                         return NO_PRIMITIVE;
                                         break;

                                    case ROUTE_FAILURE_NONTREE_LINK:
                                    case ROUTE_FAILURE_TREE_LINK:
                                    case ROUTE_FAILURE_NO_CAPACITY:
                                    default:

                                        if (routingStatus == ROUTE_FAILURE_TREE_LINK)
                                            routeStatus = NWK_STATUS_TREE_LINK_FAILURE;
                                        else if (routingStatus == ROUTE_FAILURE_NONTREE_LINK)
                                            routeStatus = NWK_STATUS_NONTREE_LINK_FAILURE;
                                        else if (routingStatus == ROUTE_FAILURE_NO_CAPACITY)
                                            routeStatus = NWK_STATUS_NO_ROUTING_CAPACITY;
                                        else
                                            routeStatus = NWK_STATUS_NO_ROUTE_AVAILABLE;

                                        // Update the TxBuffer with the Network Header for Route Error Command
                                        CreateNwkCommandHeader
                                        (
                                            NLME_GET_nwkBCSN(),
                                            DEFAULT_RADIUS,
                                            macPIB.macShortAddress,
                                            nwkSourceAddress
                                        );

                                        // Update the TxBuffer with the Network Payload for Nwk Status Command*/
                                        CreateNwkCommandNwkStatus
                                        (
                                            routeStatus,
                                            nwkSourceAddress
                                        );

                                         // Load up the MCPS_DATA.request parameters.
                                         // We'll just send it back to where it came from.
                                         Prepare_MCPS_DATA_request( params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val, &i );
                                         NWKDiscardRx();
                                         return MCPS_DATA_request;
                                         break;

                                }
                            #endif  // I_SUPPORT_ROUTING
                        } // End of handling Unicast Data Frame
                    } // End of handling Unicast packets.
                } // case MCPS_DATA_indication:
                NWKDiscardRx();
                return NO_PRIMITIVE;


            #ifndef I_AM_END_DEVICE
            case MLME_ASSOCIATE_indication:
				ASSOCIATE_indication_Time = TickGet();
                #if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
                     if ( panIDConflictStatus.flags.bits.bResolutionInProgress )
                     {
                        return NO_PRIMITIVE;
                     }
                #endif
                #ifdef I_SUPPORT_SECURITY
                    // omit since both securityEnable overlap
                    params.MLME_ASSOCIATE_response.SecurityEnable = params.MLME_ASSOCIATE_indication.SecurityUse;
                #endif
                // Copy the indication DeviceAddress to the response DeviceAddress
				printf("\n\rASSOCIATE_indication Device ID-");
                for (i=0; i<8; i++)
                {
                    params.MLME_ASSOCIATE_response.DeviceAddress.v[i] = params.MLME_ASSOCIATE_indication.DeviceAddress.v[i];
					PrintChar(params.MLME_ASSOCIATE_indication.DeviceAddress.v[i]);
					printf(":");
                }
				printf("\n\r");
                if ((i = NWKLookupNodeByLongAddr( &(params.MLME_ASSOCIATE_indication.DeviceAddress) )) != INVALID_NEIGHBOR_KEY)
                {
                    #ifdef I_SUPPORT_SECURITY
                        IncomingFrameCount[0][i].Val = 0;
                        IncomingFrameCount[1][i].Val = 0;
                        currentNeighborRecord.bSecured = FALSE;
                        #ifdef USE_EXTERNAL_NVM
                            PutNeighborRecord( neighborTable + (WORD)i * (WORD)sizeof(NEIGHBOR_RECORD), &currentNeighborRecord);
                        #else
                            PutNeighborRecord( &(neighborTable[i]), &currentNeighborRecord );
                        #endif
                        /*if( params.MLME_ASSOCIATE_indication.SecurityUse != currentNeighborRecord.bSecured )
                        {
                            currentNeighborRecord.bSecured = params.MLME_ASSOCIATE_indication.SecurityUse;
                            #ifdef USE_EXTERNAL_NVM
                                PutNeighborRecord( neighborTable + (WORD)i * (WORD)sizeof(NEIGHBOR_RECORD), &currentNeighborRecord);
                            #else
                                PutNeighborRecord( &(neighborTable[i]), &currentNeighborRecord );
                            #endif
                        }*/
                    #endif
                    params.MLME_ASSOCIATE_response.AssocShortAddress = currentNeighborRecord.shortAddr;
                    params.MLME_ASSOCIATE_response.status = ASSOCIATION_SUCCESS;
                    // DeviceAddress already copied from input
                    // SecurityEnable unchanged from input
                    /*
                     * When we receive COMM status indication on network layer,
                     * for Differentiating orphan join or association join,
                     * rejoin_network variable is set accordingly.
                     */
                    rejoin_network = ASSOCIATION_REJOIN;

                    return MLME_ASSOCIATE_response;
                }
                else
                {
                    // See if we can allow this node to join.
                    // NOTE: if the application wishes to disallow the join, this is the place to do it.
					if(CheckDeviceJoiningPermission(params.MLME_ASSOCIATE_indication.DeviceAddress) == FALSE)
					{
						params.MLME_ASSOCIATE_response.status = ASSOCIATION_PAN_ACCESS_DENIED;
                        params.MLME_ASSOCIATE_response.AssocShortAddress.Val = 0xFFFF;
                        /*
                         * When we receive COMM status indication on network layer,
                         * for Differentiating orphan join or association join,
                         * rejoin_network variable is set accordingly.
                         */
                        rejoin_network = ASSOCIATION_REJOIN;
                        return MLME_ASSOCIATE_response;
					}
                #ifdef ZCP_DEBUG
                    if( accessDenied == 1 )
                    {
                        params.MLME_ASSOCIATE_response.status = ASSOCIATION_PAN_ACCESS_DENIED;
                        params.MLME_ASSOCIATE_response.AssocShortAddress.Val = 0xFFFF;
                        /*
                         * When we receive COMM status indication on network layer,
                         * for Differentiating orphan join or association join,
                         * rejoin_network variable is set accordingly.
                         */
                        rejoin_network = ASSOCIATION_REJOIN;
                        return MLME_ASSOCIATE_response;
                    } else if( accessDenied == 2 )
                    {
                        return NO_PRIMITIVE;
                    }
                #endif
                    if (CanAddChildNode())
                    {
                        if ((CanAddNeighborNode()) != INVALID_NEIGHBOR_KEY)
                        {
                            #ifdef I_SUPPORT_SECURITY
                                AddChildNode(params.MLME_ASSOCIATE_indication.SecurityUse);
                            #else
                                AddChildNode();
                            #endif
                            // Save off the Capability Information, so we can send it back with the NLME_JOIN_indication
                            // after we get the MLME_ASSOCIATE_response.
                            nwkStatus.lastCapabilityInformation = params.MLME_ASSOCIATE_indication.CapabilityInformation;

                        #ifdef ZCP_DEBUG
                            if( bDisableShortAddress )
                            {
                                params.MLME_ASSOCIATE_response.AssocShortAddress.Val = 0xfffe;
                                params.MLME_ASSOCIATE_response.status = ASSOCIATION_SUCCESS;
                                /*
                                 * When we receive COMM status indication on network layer,
                                 * for Differentiating orphan join or association join,
                                 * rejoin_network variable is set accordingly.
                                 */
                                rejoin_network = ASSOCIATION_REJOIN;

                                return MLME_ASSOCIATE_response;
                            } else
                        #endif
                            params.MLME_ASSOCIATE_response.AssocShortAddress = currentNeighborRecord.shortAddr;
                            params.MLME_ASSOCIATE_response.status = ASSOCIATION_SUCCESS;
                            /*
                             * When we receive COMM status indication on network layer,
                             * for Differentiating orphan join or association join,
                             * rejoin_network variable is set accordingly.
                             */
                            rejoin_network = ASSOCIATION_REJOIN;

                            // DeviceAddress already copied from input
                            // SecurityEnable unchanged from input
                            return MLME_ASSOCIATE_response;
                        }
                    }
                    params.MLME_ASSOCIATE_response.status = ASSOCIATION_PAN_AT_CAPACITY;
                    params.MLME_ASSOCIATE_response.AssocShortAddress.Val = 0xFFFF;
                    // DeviceAddress already copied from input
                    // SecurityEnable unchanged from input
                    return MLME_ASSOCIATE_response;
                }
                break;
            #endif

            // ---------------------------------------------------------------------
            //#ifndef I_AM_COORDINATOR
            case MLME_ASSOCIATE_confirm:
            if(NOW_I_AM_NOT_A_CORDINATOR()){//
                if (params.MLME_ASSOCIATE_confirm.status)
                {
                    // Association failed, so set our PAN ID back to 0xFFFF
                    ZigBeeStatus.flags.bits.bNetworkJoined = 0;
                    macPIB.macPANId.Val = 0xFFFF;
                    MLME_SET_macPANId_hw();

                    // If we need to rejoin, it will have to be as a new node.
                    ZigBeeStatus.flags.bits.bTryOrphanJoin = 0;

                    // Clear the potential parent bit and other parent fields, so we
                    // do not try to associate with this device again.
                    #ifdef USE_EXTERNAL_NVM
                        pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                    #else
                        pCurrentNeighborRecord = &neighborTable[currentNeighborTableInfo.parentNeighborTableIndex];
                    #endif
                    GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
                    currentNeighborRecord.deviceInfo.bits.PotentialParent = 0;
                    currentNeighborTableInfo.parentNeighborTableIndex = INVALID_NEIGHBOR_KEY;
                    PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );

                    // If there are any more potential parents, we have to try again...
                    goto TryToJoinPotentialParent;
                }
                else
                {
                    //Device has joined a network.  Set address information
                    macPIB.macShortAddress = params.MLME_ASSOCIATE_confirm.AssocShortAddress;

SetMyAddressInformation:
                    ZigBeeStatus.flags.bits.bNetworkJoined = 1;
                    MLME_SET_macShortAddress_hw();

                    // Set parent relationship in neighbor table
                    #ifdef USE_EXTERNAL_NVM
                        pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
                    #else
                        pCurrentNeighborRecord = &neighborTable[currentNeighborTableInfo.parentNeighborTableIndex];
                        GetNeighborRecord( &currentNeighborRecord, (ROM void*)pCurrentNeighborRecord );
                    #endif
                    currentNeighborRecord.deviceInfo.bits.Relationship = NEIGHBOR_IS_PARENT;
                     ParentAddress = currentNeighborRecord.shortAddr;
                    memcpy( (void *)&currentNeighborRecord.longAddr, (void *)&macPIB.macCoordExtendedAddress, 8 );
                    PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );

                    macPIB.macCoordShortAddress = currentNeighborRecord.shortAddr;

                    // If devices needs to rejoin later, do so as an orphan.
                    ZigBeeStatus.flags.bits.bTryOrphanJoin = 1;

                    // Initialize my information.  We'll do it here since we have our parent's depth.
                    currentNeighborTableInfo.nwkUpdateId = currentNeighborRecord.nwkUpdateId;
                    PutNeighborTableInfo();

                    #ifndef I_AM_END_DEVICE
                        if (currentNeighborRecord.deviceInfo.bits.Depth == 0x0F )
                        {
                            currentNeighborTableInfo.depth                  = currentNeighborRecord.deviceInfo.bits.Depth; //  Joined more than Max Depth
                        }
                        else
                        {
                            currentNeighborTableInfo.depth                  = currentNeighborRecord.deviceInfo.bits.Depth+1; // Our depth in the network
                        }
                        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
                            currentNeighborTableInfo.cSkip.Val              = GetCSkipVal( currentNeighborTableInfo.depth ); // Address block size
                            if (!currentNeighborTableInfo.flags.bits.bChildAddressInfoValid)
                            {
                                currentNeighborTableInfo.nextEndDeviceAddr.Val  = macPIB.macShortAddress.Val + currentNeighborTableInfo.cSkip.Val * NIB_nwkMaxRouters + 1;  // Next address available to give to an end device
                                currentNeighborTableInfo.nextRouterAddr.Val     = macPIB.macShortAddress.Val + 1;     // Next address available to give to a router
                                currentNeighborTableInfo.numChildren            = 0;
                                currentNeighborTableInfo.numChildRouters        = 0;
                                currentNeighborTableInfo.flags.bits.bChildAddressInfoValid = 1;
                            }
                        /*
                         * For Routers, on successful join we should update the nwkUpdateId
                         * and update the beacon payload.
                         */
                        #else
                            currentNeighborTableInfo.numChildren            = 0;
                            currentNeighborTableInfo.nextChildAddr.Val = GenerateNewShortAddr();
                            nwkStatus.flags.bits.bNextChildAddressUsed = FALSE;
                        #endif
                        PutNeighborTableInfo();
                        SetBeaconPayload(TRUE);
                    #endif

                }

                #ifdef I_SUPPORT_SECURITY
                    securityStatus.flags.bits.bAuthorization = TRUE;
                    AuthorizationTimeout = TickGet();
                    WaitForAuthentication.Val = AUTHORIZATION_TIMEOUT * 5;

                    if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
                        #if ( I_SUPPORT_MANY_TO_ONE_HANDLING == 0x01)
                            #if I_SUPPORT_LINK_KEY == 1
                                /* set the trust center as concentrator */
                                {
                                    SHORT_ADDR concentratorAddress;
                                    concentratorAddress.Val = 0x0000;

                                    if ( FindFreeRoutingTableEntry( concentratorAddress, TRUE ) != 0xFF )
                                    {
                                        //pCurrentRoutingEntry = routingTable;
                                        //GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
                                        currentRoutingEntry.destAddress.Val = 0x0000;
                                        currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_ACTIVE;
                                        currentRoutingEntry.rtflags.rtbits.bNoRouteCache =  TRUE;
                                        currentRoutingEntry.rtflags.rtbits.bManyToOne = TRUE;
                                        currentRoutingEntry.rtflags.rtbits.bRouteRecordRequired = TRUE;
                                        currentRoutingEntry.rtflags.rtbits.bGroupIDFlag = FALSE;
                                        currentRoutingEntry.nextHop  = ParentAddress;

                                        #if (I_SUPPORT_ROUTING_TABLE_AGING == 0x01)
                                            currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                                        #endif // (I_SUPPORT_ROUTING_TABLE_AGING == 0x01)
                                        PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
                                    }
                                }
                            #endif // I_SUPPORT_LINK_KEY == 1
                        #endif //( I_SUPPORT_MANY_TO_ONE_HANDLING == 0x01)
                    }//#endif // I_AM_ROUTER

                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        PHYTasksPending.bits.PHY_AUTHORIZE = 1;
                        lastPollTime = AuthorizationTimeout;
                    }//#endif

                #endif

                if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                    ZigBeeStatus.flags.bits.bRequestingData = 0;

                }//#endif

                /*#ifdef I_SUPPORT_ROUTING
                    nwkStatus.flags.bits.bCanRoute = 1;
                #endif*/
                // Status is already in place.
                /* Send the ED_annoc only after key is recieved in security mode */
                #if !defined(I_SUPPORT_SECURITY)
                    /* Updated NLME join confirmation parameters */
                    params.NLME_JOIN_confirm.ExtendedPANId = currentNeighborRecord.ExtendedPANID;
                    params.NLME_JOIN_confirm.ActiveChannel = currentNeighborRecord.LogicalChannel;

                    // Clean up the neighbor table entries that does not belong to my network
                    NWKTidyNeighborTable();
                    return NLME_JOIN_confirm;
                #else

                    // Clean up the neighbor table entries that does not belong to my network
                    NWKTidyNeighborTable();
                    return NO_PRIMITIVE;
                #endif

                break;
            }//#endif
				break;
            // ---------------------------------------------------------------------

//            We do not do anything with this indication, as allowed by the ZigBee spec.
//            We handle everything with the NWK LEAVE command.
            #ifdef ZCP_DEBUG
            case MLME_DISASSOCIATE_indication:
                #ifdef ZCP_PRINTOUT
                printf("MLME_DISASSOCIATE_indication Result: ");
                printf("\r\nDevice Address: ");
                PrintChar(params.MLME_DISASSOCIATE_indication.DeviceAddress.v[7]);
                PrintChar(params.MLME_DISASSOCIATE_indication.DeviceAddress.v[6]);
                PrintChar(params.MLME_DISASSOCIATE_indication.DeviceAddress.v[5]);
                PrintChar(params.MLME_DISASSOCIATE_indication.DeviceAddress.v[4]);
                PrintChar(params.MLME_DISASSOCIATE_indication.DeviceAddress.v[3]);
                PrintChar(params.MLME_DISASSOCIATE_indication.DeviceAddress.v[2]);
                PrintChar(params.MLME_DISASSOCIATE_indication.DeviceAddress.v[1]);
                PrintChar(params.MLME_DISASSOCIATE_indication.DeviceAddress.v[0]);
                printf("\r\nDisassociate Reason: ");
                PrintChar(params.MLME_DISASSOCIATE_indication.DisassociateReason);
                printf("\r\nACL Entry: ");
                PrintChar(params.MLME_DISASSOCIATE_indication.ACLEntry);
                printf("\r\n");
                if( params.MLME_DISASSOCIATE_indication.DisassociateReason == COORDINATOR_FORCED_LEAVE )
                {
                    params.MLME_DISASSOCIATE_confirm.status = SUCCESS;
                    macPIB.macPANId.Val = 0xFFFF;
                    macPIB.macShortAddress.Val = 0xFFFF;
                    printf("\r\nMAC PIB: ");
                    printf("\r\nMAC PANID: ");
                    PrintChar(macPIB.macPANId.v[1]);
                    PrintChar(macPIB.macPANId.v[0]);
                    printf("\r\nMAC Short Address: ");
                    PrintChar(macPIB.macShortAddress.v[1]);
                    PrintChar(macPIB.macShortAddress.v[0]);
                    printf("\r\n");
                    MLME_SET_macShortAddress_hw();
                }
                #endif
                break;
            #endif

            // ---------------------------------------------------------------------
            //#if !defined(I_AM_COORDINATOR)
            case MLME_DISASSOCIATE_confirm:
              if(NOW_I_AM_NOT_A_CORDINATOR()){//
	                // The only way we get this is if we are in the process of leaving the network.
                // The leave is almost complete.  Set the final flag to perform a reset.
                //nwkStatus.flags.bits.bLeaveWaitForConfirm = 0; // Not applicable for ZigBee 2007 and Pro
                nwkStatus.flags.bits.bLeaveReset = 1;

                // We have left the network.  Remove our parent from the neighbor table.
                i = NWKLookupNodeByShortAddrVal( macPIB.macCoordShortAddress.Val );
                if (i != INVALID_NEIGHBOR_KEY)
                {
                    RemoveNeighborTableEntry();
                }

                // Notify our upper layers that we have left the network.  We need to send
                // back a different primitive depending on why we are leaving the network.
                // But both have DeviceAddress at the same location
                GetMACAddress( &params.NLME_LEAVE_confirm.DeviceAddress );

                if (nwkStatus.leaveReason == SELF_INITIATED_LEAVE)
                {
                    #ifdef I_AM_END_DEVICE
                    if (!params.MLME_DISASSOCIATE_confirm.status)
                    #else
                    if (!params.MLME_DISASSOCIATE_confirm.status /*&& // Not applicable for ZigBee 2007 and Pro
                        (!nwkStatus.flags.bits.bRemovingChildren ||
                         (!currentNeighborTableInfo.numChildren && nwkStatus.flags.bits.bAllChildrenLeft))*/)
                    #endif
                    {
                        params.NLME_LEAVE_confirm.Status = NWK_SUCCESS;
                    }
                    else
                    {
                        params.NLME_LEAVE_confirm.Status = NWK_LEAVE_UNCONFIRMED;
                    }
                    return NLME_LEAVE_confirm;
                }
                else
                {
                    params.NLME_LEAVE_indication.Rejoin = (nwkStatus.flags.bits.bNRejoin) ? 0 : 1;
                    return NLME_LEAVE_indication;
                }
                break;
			}//      #endif
			break;

            // ---------------------------------------------------------------------
            case MLME_BEACON_NOTIFY_indication:
                #ifdef DEBUG_LOG
                NoOfPANDescriptorFound++;
                #endif

                // If we did not get a short address or if the protocol identifier is wrong,
                // then the node is not a ZigBee node.
                params.MLME_BEACON_NOTIFY_indication.sdu  = params.PD_DATA_indication.psdu;
            
                if ((params.MLME_BEACON_NOTIFY_indication.CoordAddrMode == 0x02) &&
                   (params.MLME_BEACON_NOTIFY_indication.sdu[0] == ZIGBEE_PROTOCOL_ID))
                {
                    #if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
                        #ifndef I_AM_END_DEVICE
                            if(((NOW_I_AM_A_CORDINATOR())//#ifdef I_AM_COORDINATOR
                                && ( ZigBeeStatus.flags.bits.bNetworkFormed ))
                            ||//#else
                                ((NOW_I_AM_NOT_A_CORDINATOR())&& ( ZigBeeStatus.flags.bits.bNetworkJoined ))
                            )//#endif
                               {
                                     if ( activatePANIDConflict && IsTherePANIdConflict() )
                                     {
                                          if (!panIDConflictStatus.flags.bits.bResolutionInProgress)
                                          {

                                                nwkStatus.PanIDReport.PANIdList[nwkStatus.PanIDReport.index++].Val = params.MLME_BEACON_NOTIFY_indication.CoordPANId.Val;
                                                if ( nwkStatus.PanIDReport.index >= 0x08 )
                                                {
                                                    nwkStatus.PanIDReport.index = 0;
                                                }
                                                if ( nwkStatus.PanIDReport.noOfRecords < 8 )
                                                {
                                                    nwkStatus.PanIDReport.noOfRecords++;
                                                }
                                                panIDConflictStatus.flags.bits.bPANIdConflictDetected = TRUE;
                                                panIDConflictStatus.flags.bits.bResolutionInProgress = TRUE;
                                          }
                                            NWKDiscardRx();
                                            return NO_PRIMITIVE;
                                     }
                                }
                        #endif
                    #endif

                    if(((NOW_I_AM_A_CORDINATOR())//#ifdef I_AM_COORDINATOR
                                && ( ZigBeeStatus.flags.bits.bNetworkFormed ))
                            ||//#else
                                ((NOW_I_AM_NOT_A_CORDINATOR())&& ( ZigBeeStatus.flags.bits.bNetworkJoined ))
                            )//#endif
                    {
                        LONG_ADDR ExtendedPANID;
                    /*I am already member of PAN. I'll process beacons only from my PAN */
                        memcpy((BYTE *)&ExtendedPANID, &params.MLME_BEACON_NOTIFY_indication.sdu[3], 8);
                        if ( memcmp((BYTE *)&ExtendedPANID, (BYTE *)&currentNeighborTableInfo.nwkExtendedPANID, 8))
                        {
                            /* I may end up here only if I receive a beacon with both short PANID and EPID not matching to mine */
                            NWKDiscardRx();
                            return NO_PRIMITIVE;
                        }
                    }

                    #if !defined(I_AM_END_DEVICE)
                        if( (params.MLME_BEACON_NOTIFY_indication.sdu[1] & 0x0F ) != PROFILE_myStackProfileId )
                        {
                            MACDiscardRx();
                            return NO_PRIMITIVE;
                        }
                    #endif

                    if (NWKLookupNodeByShortAddrValAndPANID( params.MLME_BEACON_NOTIFY_indication.CoordAddress.ShortAddr.Val,
                        params.MLME_BEACON_NOTIFY_indication.CoordPANId.Val ) != INVALID_NEIGHBOR_KEY)
                    {
                        #ifdef DEBUG_LOG
                            overWriteCheck = 0x01;
                        #endif
                        // This is an old node.  Reset some of the fields.
                        goto ResetNeighborTableFields;
                    }
                    else
                    {
                        // This is a new ZigBee node, so try to add it to the neighbor table.
                        if (CanAddNeighborNode() != INVALID_NEIGHBOR_KEY)
                        {
                            #ifdef DEBUG_LOG
                                LOG_ASSERT(DEBUG_LOG_INFO, "Adding new NTR" == 0);
                            #endif
                            // Set all the information for a new node, that could get updated later.
                            currentNeighborRecord.deviceInfo.bits.bInUse            = 1;
                            #ifdef DEBUG_LOG
                                overWriteCheck = 0x00;
                            #endif
                            currentNeighborRecord.deviceInfo.bits.Relationship      = NEIGHBOR_IS_NONE; //NEIGHBOR_IS_SIBLING;

                            // We don't know the long address from the beacon, so set it to 0's.
                            for (i=0; i<8; i++)
                            {
                                currentNeighborRecord.longAddr.v[i] = 0x00;
                            }

                            //-------------------------------------------------------------
                            // CAUTION: THIS BLOCK SHOULD NOT BE PUT UNDER ResetNeighborTableFields label.
                            // THIS IS BECAUSE, WE DO NOT WANT RESET THESE VARIABLE AS IT MAY CONTAIN VALID VALUE.
                            // IF THE BEACON IS NEW, THEN ONLY WE NEED TO UPDATE THESE FIELDS
                            #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                currentNeighborRecord.linkStatusInfo.flags.bits.bIncomingCost =
                                            CalculateLinkQuality( params.MLME_BEACON_NOTIFY_indication.LinkQuality );
                                currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost = 0;
                                currentNeighborRecord.linkStatusInfo.Age = 0;
                            #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

                            #if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
                                currentNeighborRecord.suppressLinkStatusUpdate = 0x00;
                            #endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
                            //-------------------------------------------------------------

                            // Update the neighbor table size.
                            currentNeighborTableInfo.neighborTableSize++;
                            PutNeighborTableInfo();

ResetNeighborTableFields:
                            // Set/Update the fields that may need updating.
                            if (params.MLME_BEACON_NOTIFY_indication.SuperframeSpec.bits.PANCoordinator)
                            {
                                currentNeighborRecord.deviceInfo.bits.deviceType        = DEVICE_ZIGBEE_COORDINATOR;
                            }
                            else
                            {
                                currentNeighborRecord.deviceInfo.bits.deviceType        = DEVICE_ZIGBEE_ROUTER;
                            }
                            #ifdef DEBUG_LOG
                                if ( overWriteCheck )
                                {
                                    if ( currentNeighborRecord.deviceInfo.bits.PermitJoining ^ params.MLME_BEACON_NOTIFY_indication.SuperframeSpec.bits.AssociationPermit )
                                       {
                                            NoOfPANDescriptorFound |= 0x80;
                                            NoOfPANDescriptorFound |= (0x40 & currentNeighborRecord.deviceInfo.bits.PermitJoining);
                                            NoOfPANDescriptorFound |= (0x20 & params.MLME_BEACON_NOTIFY_indication.SuperframeSpec.bits.AssociationPermit);
                                            LOG_ASSERT(DEBUG_LOG_INFO, "Got Diff PermitJoining from same neighbour OverWriting" == 0);
                                       }
                                }
                            #endif
                            currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle      = 1;
                            currentNeighborRecord.deviceInfo.bits.Depth             = (params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x78) >> 3;
                            currentNeighborRecord.deviceInfo.bits.StackProfile      = (params.MLME_BEACON_NOTIFY_indication.sdu[1] & 0x0F);
                            currentNeighborRecord.deviceInfo.bits.ZigBeeVersion     = (params.MLME_BEACON_NOTIFY_indication.sdu[1] & 0xF0) >> 4;
                            currentNeighborRecord.deviceInfo.bits.LQI               = params.MLME_BEACON_NOTIFY_indication.LinkQuality;
                            currentNeighborRecord.deviceInfo.bits.PermitJoining     = params.MLME_BEACON_NOTIFY_indication.SuperframeSpec.bits.AssociationPermit;
                    #ifdef DEBUG_LOG
                           PotentialParentFound |= ( 0x01 & currentNeighborRecord.deviceInfo.bits.PermitJoining );
                    #endif
                           if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
                                currentNeighborRecord.deviceInfo.bits.PotentialParent   = ((params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x04) == 0x04);
                           }else{// #else
                                currentNeighborRecord.deviceInfo.bits.PotentialParent   = ((params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x80) == 0x80);
                           }// #endif

                           // LOG( DEBUG_LOG_INFO , "Potential Parent" == currentNeighborRecord.deviceInfo.bits.PotentialParent );

                            /* Store Router capacity and End device capacity into neighbor table for beacon received */
                            if (params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x04)
                            {
                                currentNeighborRecord.deviceInfo.bits.RouterCapacity    = TRUE;
                                //currentNeighborRecord.deviceInfo.bits.PotentialParent = TRUE;
                            }
                            else
                            {
                                currentNeighborRecord.deviceInfo.bits.RouterCapacity    = FALSE;
                                //currentNeighborRecord.deviceInfo.bits.PotentialParent = FALSE;
                            }

                            if (params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x80)
                            {
                                currentNeighborRecord.deviceInfo.bits.EndDeviceCapacity    = TRUE;
                                //currentNeighborRecord.deviceInfo.bits.PotentialParent = TRUE;
                            }
                            else
                            {
                                currentNeighborRecord.deviceInfo.bits.EndDeviceCapacity    = FALSE;
                                //currentNeighborRecord.deviceInfo.bits.PotentialParent = FALSE;
                            }

                            currentNeighborRecord.panID.Val             = params.MLME_BEACON_NOTIFY_indication.CoordPANId.Val;
                            currentNeighborRecord.shortAddr.Val         = params.MLME_BEACON_NOTIFY_indication.CoordAddress.ShortAddr.Val;
                            currentNeighborRecord.LogicalChannel    = params.MLME_BEACON_NOTIFY_indication.LogicalChannel;
                            /* Retrieve the nwkUpdateId and ExtendedPANId from Beacon received */
                            currentNeighborRecord.nwkUpdateId       = params.MLME_BEACON_NOTIFY_indication.sdu[14];

                            memcpy((BYTE *)&currentNeighborRecord.ExtendedPANID,
                                    &params.MLME_BEACON_NOTIFY_indication.sdu[3], 8);
                            PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord);
                        }
                    }
                }
            #ifdef ZCP_DEBUG
                else if(params.MLME_BEACON_NOTIFY_indication.CoordAddrMode == 0x03)
                {
                    if (NWKLookupNodeByLongAddr( &(params.MLME_BEACON_NOTIFY_indication.CoordAddress.LongAddr) ) != INVALID_NEIGHBOR_KEY)
                    {
                        // This is an old node.  Reset some of the fields.
                        goto ResetNeighborTableFields2;
                    }
                    else
                    {
                        // This is a new ZigBee node, so try to add it to the neighbor table.
                        if (CanAddNeighborNode() != INVALID_NEIGHBOR_KEY)
                        {
                            // Set all the information for a new node, that could get updated later.
                            currentNeighborRecord.deviceInfo.bits.bInUse            = 1;
                            currentNeighborRecord.deviceInfo.bits.Relationship      = NEIGHBOR_IS_NONE;

                            // We don't know the long address from the beacon, so set it to 0's.
                            for (i=0; i<8; i++)
                            {
                                currentNeighborRecord.longAddr.v[i] = params.MLME_BEACON_NOTIFY_indication.CoordAddress.LongAddr.v[i];
                            }

                            // Update the neighbor table size.
                            currentNeighborTableInfo.neighborTableSize++;
                            PutNeighborTableInfo();

ResetNeighborTableFields2:
                            // Set/Update the fields that may need updating.
                            if (params.MLME_BEACON_NOTIFY_indication.SuperframeSpec.bits.PANCoordinator)
                            {
                                currentNeighborRecord.deviceInfo.bits.deviceType        = DEVICE_ZIGBEE_COORDINATOR;
                            }
                            else
                            {
                                currentNeighborRecord.deviceInfo.bits.deviceType        = DEVICE_ZIGBEE_ROUTER;
                            }
                            currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle      = 1;
                            currentNeighborRecord.deviceInfo.bits.Depth             = (params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x78) >> 3;
                            currentNeighborRecord.deviceInfo.bits.StackProfile      = (params.MLME_BEACON_NOTIFY_indication.sdu[1] & 0x0F);
                            currentNeighborRecord.deviceInfo.bits.ZigBeeVersion     = (params.MLME_BEACON_NOTIFY_indication.sdu[1] & 0xF0) >> 4;
                            currentNeighborRecord.deviceInfo.bits.LQI               = params.MLME_BEACON_NOTIFY_indication.LinkQuality;
                            currentNeighborRecord.deviceInfo.bits.PermitJoining     = params.MLME_BEACON_NOTIFY_indication.SuperframeSpec.bits.AssociationPermit;

                            if(NOW_I_AM_A_ROUTER()){//#ifdef I_AM_ROUTER
                                currentNeighborRecord.deviceInfo.bits.PotentialParent   = (params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x04) == 0x04;
                            }else{//#else
                                currentNeighborRecord.deviceInfo.bits.PotentialParent   = (params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x80) == 0x80;
                            }//#endif

                            if (params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x04)
                            {
                                currentNeighborRecord.deviceInfo.bits.RouterCapacity    = 1;
                            }
                            else
                            {
                                currentNeighborRecord.deviceInfo.bits.RouterCapacity    = 0;
                            }

                            if (params.MLME_BEACON_NOTIFY_indication.sdu[2] & 0x80)
                            {
                                currentNeighborRecord.deviceInfo.bits.EndDeviceCapacity    = 1;
                            }
                            else
                            {
                                currentNeighborRecord.deviceInfo.bits.EndDeviceCapacity    = 0;
                            }


                            currentNeighborRecord.panID             = params.MLME_BEACON_NOTIFY_indication.CoordPANId;
                            currentNeighborRecord.shortAddr.Val         = 0xfffe;
                            currentNeighborRecord.LogicalChannel    = params.MLME_BEACON_NOTIFY_indication.LogicalChannel;

                            currentNeighborRecord.nwkUpdateId = params.MLME_BEACON_NOTIFY_indication.sdu[14];

                            PutNeighborRecord( pCurrentNeighborRecord,  &currentNeighborRecord);
                        }
                    }
                }

            #endif

                NWKDiscardRx();
                return NO_PRIMITIVE;
                break;

            #ifndef I_AM_END_DEVICE
            case MLME_ORPHAN_indication:
                i = NWKLookupNodeByLongAddr( &(params.MLME_ORPHAN_indication.OrphanAddress) );
                if (i != INVALID_NEIGHBOR_KEY)
                {
                    if (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD)
                    {
                        // currentNeighborRecord already has the neighbor table entry.
                        params.MLME_ORPHAN_response.ShortAddress = currentNeighborRecord.shortAddr;
                        params.MLME_ORPHAN_response.AssociatedMember = TRUE;
                        #ifdef I_SUPPORT_SECURITY
                            #ifdef I_SUPPORT_SECURITY_SPEC
                                params.MLME_ORPHAN_response.SecurityEnable = params.MLME_ORPHAN_indication.SecurityUse;
                                IncomingFrameCount[0][i].Val = 0;
                                IncomingFrameCount[1][i].Val = 0;
                            #else
                                IncomingFrameCount[0][i].Val = 0;
                                IncomingFrameCount[1][i].Val = 0;
                                params.MLME_ORPHAN_response.SecurityEnable = FALSE;
                            #endif
                        #endif

                        /*
                         * When we receive COMM status indication on network layer,
                         * for Differentiating orphan join or association join,
                         * rejoin_network variable is set accordingly.
                         */
                        rejoin_network = ORPHAN_REJOIN;
                        return MLME_ORPHAN_response;
                    }
                }

                break;
            #endif

            // ---------------------------------------------------------------------
            case MLME_RESET_confirm:
            {
                // Reset all NWK layer variables and routing tables.  We do NOT clear the
                // neighbor table.
                // Clear all static variables.
                /* For Zigbee 2006: Need to return the proper confirmation if this was
                 * reached via RESET or LEAVE/RESET path
                */
                #ifdef I_SUPPORT_SECURITY
                    BYTE ActiveKeyIndex;
                #endif
                currentNeighborTableInfo.nwkExtendedPANID.v[0] = 0x00;
                currentNeighborTableInfo.nwkExtendedPANID.v[1] = 0x00;
                currentNeighborTableInfo.nwkExtendedPANID.v[2] = 0x00;
                currentNeighborTableInfo.nwkExtendedPANID.v[3] = 0x00;
                currentNeighborTableInfo.nwkExtendedPANID.v[4] = 0x00;
                currentNeighborTableInfo.nwkExtendedPANID.v[5] = 0x00;
                currentNeighborTableInfo.nwkExtendedPANID.v[6] = 0x00;
                currentNeighborTableInfo.nwkExtendedPANID.v[7] = 0x00;
                PutNeighborTableInfo();
                #ifdef I_SUPPORT_SECURITY
                    ActiveKeyIndex = nwkMAGICResSeq;
                    PutNwkActiveKeyNumber(&ActiveKeyIndex);
                #endif
                VariableAndTableInitialization( FALSE );
                #if defined(I_SUPPORT_SECURITY) //&& !defined(I_AM_COORDINATOR)
                if(NOW_I_AM_NOT_A_CORDINATOR()){	
                    if (securityStatus.flags.bits.bAuthorization)
                    {
                        securityStatus.flags.bits.bAuthorization = FALSE;
                        for (i=0;i<8;i++)
                        {
                            params.NLME_JOIN_confirm.ExtendedPANId.v[i] = 0x00;
                        }
                        params.NLME_JOIN_confirm.ActiveChannel = phyPIB.phyCurrentChannel;
                        params.NLME_JOIN_confirm.ShortAddress = macPIB.macShortAddress;
                        params.NLME_JOIN_confirm.Status = NWK_AUTHENTICATION_FAILURE;
                        return NLME_JOIN_confirm;
                    }
                } 
                #endif

                // Status is already in place
                return NLME_RESET_confirm;
            }
            case MLME_SCAN_confirm:
                /* set this back to auto mode */
                #ifdef USE_EXT_PA_LNA
                    PHYSetShortRAMAddr(GPIO,    0x00);
                    PHYSetShortRAMAddr(GPIODIR, 0x00);
                    PHYSetLongRAMAddr(TESTMODE, 0x0F); 
                #endif
				panIdListCounter = 0;
                if(NOW_I_AM_A_CORDINATOR()){//#if defined(I_AM_COORDINATOR)
                    if (params.MLME_SCAN_confirm.ScanType == MAC_SCAN_ENERGY_DETECT)
                    {
                        CHANNEL_INFO    tempChannel;


                        #if I_SUPPORT_FREQUENCY_AGILITY == 1
                            /* Verify whether ZDO has requested ED scanning for
                               reporting interference. */
                            if( nwkStatus.flags.bits.bScanRequestFromZDO )
                            {
                                zdoStatus.flags.bits.bNwkUpdateEDScan = 1;
                                nwkStatus.flags.bits.bScanRequestFromZDO = 0;
                                return NO_PRIMITIVE;
                            }
                        #endif

                        if (!params.MLME_SCAN_confirm.ResultListSize)
                        {
NetworkStartupFailure:
                            params.NLME_NETWORK_FORMATION_confirm.Status = NWK_STARTUP_FAILURE;
                            return NLME_NETWORK_FORMATION_confirm;
                        }

                        // Reserve space for the largest number of results, since a run time multiplication can be expensive.
                        if ((nwkStatus.discoveryInfo.channelList = (CHANNEL_INFO *)SRAMalloc( 16 * sizeof(CHANNEL_INFO) )) == NULL)
                        {
                            if (params.MLME_SCAN_confirm.EnergyDetectList)
                            {
                                nfree( params.MLME_SCAN_confirm.EnergyDetectList );
                            }
                            goto NetworkStartupFailure;
                        }

                        // Create the list of scanned channels.  If we asked to scan the channel
                        // before, and the channel wasn't skipped, and the energy is below the threshold,
                        // then record the channel and its energy.
                        nwkStatus.discoveryInfo.numChannels = 0;
                        ptr = params.MLME_SCAN_confirm.EnergyDetectList;
                    //#ifdef ZCP_PRINTOUT
                        printf("Scan Confirm Result(Energy Scan):");
                        printf("\r\nUnscanned Channels: ");
                        PrintChar(params.MLME_SCAN_confirm.UnscannedChannels.v[3]);
                        PrintChar(params.MLME_SCAN_confirm.UnscannedChannels.v[2]);
                        PrintChar(params.MLME_SCAN_confirm.UnscannedChannels.v[1]);
                        PrintChar(params.MLME_SCAN_confirm.UnscannedChannels.v[0]);
                        printf("\r\nResultListSize: ");
                        PrintChar(params.MLME_SCAN_confirm.ResultListSize);
                        printf("\r\nEnergyDetectList: ");
                        for(i=0; i < params.MLME_SCAN_confirm.ResultListSize; i++)
						{
                            PrintChar(params.MLME_SCAN_confirm.EnergyDetectList[i]);
							printf("\n\r");
						}
                        printf("\r\n");
                    //#endif


                        for (i=0; i<32; i++)
                        {
                            if (nwkStatus.lastScanChannels.Val & 0x00000001)
                            {
                                if (!(params.MLME_SCAN_confirm.UnscannedChannels.Val & 0x00000001))
                                {
									printf("Energy THR = ");
									PrintChar(*ptr);
									printf("\n\r");
                                    if (*ptr < 256)		//MAX_ENERGY_THRESHOLD) // need to do more work on this // nimish
                                    {
                                        nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.numChannels].channel = i;
                                        nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.numChannels].energy = *ptr;
                                        nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.numChannels].networks = 0;
                                        nwkStatus.discoveryInfo.numChannels++;

                                        /* Store the energy after sacnning */
                                        #if I_SUPPORT_FREQUENCY_AGILITY == 1
                                            EdRecords[0].EnergyReading[i-11] = *ptr;
                                        #endif
                                    }
                                    ptr++;
                                }
                            }
                            // Get ready to check the next channel.
                            params.MLME_SCAN_confirm.UnscannedChannels.Val >>= 1;
                            nwkStatus.lastScanChannels.Val >>= 1;
                        }

                        // Sort the channels from lowest to highest energy.  Since i is unsigned,
                        // our i is always one greater than normal so we can exit on 0.
                        for (i=nwkStatus.discoveryInfo.numChannels; i>0; i--)
                        {
                            for (j=1; j<=i-1; j++)
                            {
                                if (nwkStatus.discoveryInfo.channelList[j].energy < nwkStatus.discoveryInfo.channelList[j-1].energy)
                                {
                                    tempChannel = nwkStatus.discoveryInfo.channelList[j-1];
                                    nwkStatus.discoveryInfo.channelList[j-1] = nwkStatus.discoveryInfo.channelList[j];
                                    nwkStatus.discoveryInfo.channelList[j] = tempChannel;
                                }
                            }
                        }

                        // Destroy the energy detect list.
                        if (params.MLME_SCAN_confirm.EnergyDetectList)
                        {
                            nfree( params.MLME_SCAN_confirm.EnergyDetectList );
                        }

                        if (!nwkStatus.discoveryInfo.numChannels)
                        {
                            // If there were no channels with energy below the threshold, return.
							printf("Network StartupFalure\n\r");
                            nfree( nwkStatus.discoveryInfo.channelList );
                            goto NetworkStartupFailure;
                        }
                        else
                        {
                            // Perform an active scan to check for networks on the first channel.
                            nwkStatus.discoveryInfo.currentIndex = 0;

StartActiveScan:
                            params.MLME_SCAN_request.ScanType = MAC_SCAN_ACTIVE_SCAN;
                            params.MLME_SCAN_request.ScanChannels.Val = (DWORD)1 << nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.currentIndex].channel;
                            params.MLME_SCAN_request.ScanDuration = nwkStatus.lastScanDuration;
                            return MLME_SCAN_request;
                        }
                    }
                    else // active scan
                    {
					
						//x2puts("I Am doing Active Scan\n\r");
                        nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.currentIndex].networks = params.MLME_SCAN_confirm.ResultListSize;

                        // We just need the count of networks.  The beacon info is in our neighbor table.
                        while ((BYTE *)params.MLME_SCAN_confirm.PANDescriptorList)
                        {
							{
								int Tp,Tp1;
								unsigned char SameFound = 0;
								LONG_ADDR n;
								memcpy( &n.v[0], &params.MLME_SCAN_confirm.PANDescriptorList->ExtendedPANID.v[0], sizeof(n) );
								
								for(Tp=0;Tp<=panIdListCounter;Tp++)
								{
									for(Tp1=0;Tp1<8;Tp1++)
									{
										//x2putc(n.v[Tp]);
										if(ListOfExtendedPANIdinVicinity[panIdListCounter].ExtPANId.v[Tp] == n.v[Tp])
										{
											SameFound++;
										}
										else
											SameFound = 0;
									}
								}
								
								if(SameFound<7)
								{
									for(Tp=0;Tp<8;Tp++)
									{
										//x2putc(n.v[Tp]);
										ListOfExtendedPANIdinVicinity[panIdListCounter].ExtPANId.v[Tp] = n.v[Tp];
									}
									ListOfExtendedPANIdinVicinity[panIdListCounter].ValidData = TRUE;
									panIdListCounter++;
								}
								
							}
							
                            ptr = (BYTE *)params.MLME_SCAN_confirm.PANDescriptorList->next;
                            nfree( params.MLME_SCAN_confirm.PANDescriptorList );
                            params.MLME_SCAN_confirm.PANDescriptorList = (PAN_DESCRIPTOR *)ptr;
							
							
						}
							
							

                        if (++nwkStatus.discoveryInfo.currentIndex != nwkStatus.discoveryInfo.numChannels)
                        {
                            // Perform an active scan on the next channel.
                            goto StartActiveScan;
                        }

                        // We have a count of all networks on all available channels.  Now find the channel with the
                        // least number of networks.

                        nwkStatus.discoveryInfo.currentIndex = 0;
                        params.MLME_START_request.PANId = nwkStatus.requestedPANId;

                        // If the upper layers requested a PAN ID, set our index to the first network that
                        // does not have a PAN ID conflict.  If we cannot find a channel, we will override
                        // the requested PAN ID.
                        if (params.MLME_START_request.PANId.Val != 0xFFFF)
                        {
                            while (RequestedPANIDFound(nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.currentIndex].channel) &&
                                   (nwkStatus.discoveryInfo.currentIndex < nwkStatus.discoveryInfo.numChannels))
                            {
                                nwkStatus.discoveryInfo.currentIndex++;
                            }
                        }
                        if (nwkStatus.discoveryInfo.currentIndex == nwkStatus.discoveryInfo.numChannels)
                        {
                            // We cannot use the requested PAN ID, so we'll override it.
                            params.MLME_START_request.PANId.Val = 0xFFFF;
                            nwkStatus.discoveryInfo.currentIndex = 0;
                        }

                        // Find the channel with the least number of networks that does not have a PAN ID conflict.
                        for (i=1; i<nwkStatus.discoveryInfo.numChannels; i++)
                        {
                            if (nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.currentIndex].networks > nwkStatus.discoveryInfo.channelList[i].networks)
                            {
                                if ((params.MLME_START_request.PANId.Val == 0xFFFF) || !RequestedPANIDFound( nwkStatus.discoveryInfo.channelList[i].channel ))
                                {
                                    nwkStatus.discoveryInfo.currentIndex = i;
                                }
                            }
                        }

                        // If the user did not specify a PAN ID, go pick one now.  There's a possibility
                        // here for an infinite loop, but in practice, we should be able to find a unique value.
                        if (params.MLME_START_request.PANId.Val == 0xFFFF)
                        {
                            do
                            {
                                params.MLME_START_request.PANId.byte.LSB    = RANDOM_LSB;
                                params.MLME_START_request.PANId.byte.MSB    = RANDOM_MSB & 0x3F;
                            } while (RequestedPANIDFound( nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.currentIndex].channel ));
                        }

                        // Set our address as ZigBee Coordinator.  Do not set the PAN ID in the PIB -
                        // we might have to send out a realignment first.  The MAC layer will set the value in the PIB.
                        macPIB.macShortAddress.Val = 0x0000;
                        MLME_SET_macShortAddress_hw();

                        // Clear the ability to route frames.
                        nwkStatus.flags.bits.bCanRoute = 0;

                        // Clean up the neighbor table entries that does not belong to my network
                        NWKTidyNeighborTable();

                        params.MLME_START_request.LogicalChannel    = nwkStatus.discoveryInfo.channelList[nwkStatus.discoveryInfo.currentIndex].channel;
                        params.MLME_START_request.BeaconOrder       = MAC_PIB_macBeaconOrder;
                        params.MLME_START_request.SuperframeOrder   = MAC_PIB_macSuperframeOrder;
                        params.MLME_START_request.fields.Val        = MLME_START_IS_PAN_COORDINATOR;
                        params.MLME_START_request.fields.bits.BatteryLifeExtension = macPIB.macBattLifeExtPeriods;
                        //NOTE: add security  params.MLME_START_request.fields.bits.SecurityEnable = ???;
                        return MLME_START_request;
                    }

                }else{//#else // I_AM_COORDINATOR

                    #if I_SUPPORT_FREQUENCY_AGILITY == 1
                    {
                         /* Verify whether ZDO has requested ED scanning for
                            reporting interference. */
                        if( nwkStatus.flags.bits.bScanRequestFromZDO )
                        {
                            zdoStatus.flags.bits.bNwkUpdateEDScan = 1;
                            nwkStatus.flags.bits.bScanRequestFromZDO = 0;
                            return NO_PRIMITIVE;
                        }
                    }
                    #endif

                    // Router and End Device
                    if (params.MLME_SCAN_confirm.ScanType == MAC_SCAN_ACTIVE_SCAN)
                    {
                        phyPIB.phyCurrentChannel = phyPIB.phyBackupChannel;
                        /* choose appropriate channel */
                        PHYSetLongRAMAddr(0x200, (0x02 | (BYTE)((phyPIB.phyCurrentChannel - 11) << 4)));
                        PHYSetShortRAMAddr(PWRCTL,0x04);
                        PHYSetShortRAMAddr(PWRCTL,0x00);
                        // A Router or End Device is doing an active scan to find a network to join

                        NETWORK_DESCRIPTOR *newNetwork;
                        #ifdef ZCP_PRINTOUT
                        PAN_DESCRIPTOR *panDes;
                        #endif
                        BOOL            oldNetworkAvailable = FALSE;

                        if (params.MLME_SCAN_confirm.status && nwkStatus.flags.bits.bRejoinScan && ZigBeeStatus.flags.bits.bNetworkJoined)
                        {
                            if (rejoin_retries == MAX_REJOIN_RETRIES)
                            {
                                ZigBeeStatus.flags.bits.bNetworkJoined = 0;
                            }
                            
                            /* To keep an endless streams of retries going 
                             * do not increment rejoin_retries here.  If it's increment
                             * then 3 beacon requests will be broadcasted, then stoppage. 
                            */
                            //rejoin_retries++;
                            
                            timeBetweenScanDuringRejoin = TickGet();
                            ZigBeeStatus.flags.bits.bActiveScanToRejoin = 1;
                            
                            return NO_PRIMITIVE;
                        }
                        if (params.MLME_SCAN_confirm.status)
                        {
                            params.NLME_NETWORK_DISCOVERY_confirm.NetworkCount = 0;
                            params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor = NULL;
                            // The scan was not successful.  Status is already in place, and PANDescriptorList is already free.
                            return NLME_NETWORK_DISCOVERY_confirm;
                        }


                        params.NLME_NETWORK_DISCOVERY_confirm.NetworkCount = 0;
                        params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor = NULL;

                        BYTE index, PANIdcount = 0, index1;
                        PAN_ADDR PANIdList[6];
                        // Process each PAN descriptor in the list
                        {
	                    
                        while ((BYTE *)params.MLME_SCAN_confirm.PANDescriptorList)
                        {

                            // Make sure we don't already have this network in our list

                            for (index = 0; index < PANIdcount; index++)
                            {
                                if (params.MLME_SCAN_confirm.PANDescriptorList->CoordPANId.Val == PANIdList[index].Val)
                                {
                                    break;
                                }
                            }

                            // Make sure we have the node in our neighbor table.  If not, we cannot fill out the info.
                            if (params.MLME_SCAN_confirm.PANDescriptorList->CoordAddrMode)
                            {
                                i = NWKLookupNodeByLongAddr( &(params.MLME_SCAN_confirm.PANDescriptorList->CoordAddress.LongAddr) );
                            }
                            else
                            {
                                i = NWKLookupNodeByShortAddrVal( params.MLME_SCAN_confirm.PANDescriptorList->CoordAddress.ShortAddr.Val );
                            }

                            if (i != INVALID_NEIGHBOR_KEY)
                            {
                                if (index == PANIdcount)
                                {
                                    PANIdcount++;
                                    PANIdList[index].Val = params.MLME_SCAN_confirm.PANDescriptorList->CoordPANId.Val;
                                    // The PAN is not in our list.  Create an entry for it.


                                    // A zigBee 2006 requirement:
                                    if( nwkStatus.flags.bits.bRejoinScan )
                                    {
                                        // the same way to calculate path cost. link cost 3 is equal to 144
                                        if( params.MLME_SCAN_confirm.PANDescriptorList->LinkQuality < 144 )
                                        {
                                            goto NextPANDescriptor;
                                        }
                                    }
                                    if ((newNetwork = (NETWORK_DESCRIPTOR *)SRAMalloc( sizeof(NETWORK_DESCRIPTOR) )) != NULL)
                                    {
                                        /* Update Network discovery confirm structure */
                                        //newNetwork->PanID           = params.MLME_SCAN_confirm.PANDescriptorList->CoordPANId;
                                        newNetwork->LogicalChannel      = params.MLME_SCAN_confirm.PANDescriptorList->LogicalChannel;
                                        newNetwork->StackProfile        = currentNeighborRecord.deviceInfo.bits.StackProfile;
                                        newNetwork->ZigBeeVersion       = currentNeighborRecord.deviceInfo.bits.ZigBeeVersion;
                                        newNetwork->RouterCapacity      = currentNeighborRecord.deviceInfo.bits.RouterCapacity;
                                        newNetwork->EndDeviceCapacity   = currentNeighborRecord.deviceInfo.bits.EndDeviceCapacity;
                                        newNetwork->BeaconOrder         = params.MLME_SCAN_confirm.PANDescriptorList->SuperframeSpec.bits.BeaconOrder;
                                        newNetwork->SuperframeOrder     = params.MLME_SCAN_confirm.PANDescriptorList->SuperframeSpec.bits.SuperframeOrder;
                                        newNetwork->PermitJoining       = params.MLME_SCAN_confirm.PANDescriptorList->SuperframeSpec.bits.AssociationPermit;
                                        newNetwork->ExtendedPANID       = params.MLME_SCAN_confirm.PANDescriptorList->ExtendedPANID;

                                        newNetwork->next                = params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor;

                                        params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor = newNetwork;
                                        params.NLME_NETWORK_DISCOVERY_confirm.NetworkCount++;
                                    }
                                }
                                else
                                {
                                    ptr = (BYTE *)params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor;

                                    for (index1 = 0; index1 < index; index1++)
                                    {
                                        ptr = (BYTE *)params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor->next;
                                    }

                                    if (!((NETWORK_DESCRIPTOR *)ptr)->RouterCapacity)
                                    {
                                        ((NETWORK_DESCRIPTOR *)ptr)->RouterCapacity = currentNeighborRecord.deviceInfo.bits.RouterCapacity;
                                    }
                                    if (!((NETWORK_DESCRIPTOR *)ptr)->EndDeviceCapacity)
                                    {
                                        ((NETWORK_DESCRIPTOR *)ptr)->EndDeviceCapacity = currentNeighborRecord.deviceInfo.bits.EndDeviceCapacity;
                                    }
                                    if (!((NETWORK_DESCRIPTOR *)ptr)->PermitJoining)
                                    {
                                        ((NETWORK_DESCRIPTOR *)ptr)->PermitJoining = params.MLME_SCAN_confirm.PANDescriptorList->SuperframeSpec.bits.AssociationPermit;
                                    }
                                }
                            }

                            // A ZigBee 2006 requirement:
                            if( nwkStatus.flags.bits.bRejoinScan )
                            {

                                if( params.MLME_SCAN_confirm.PANDescriptorList->CoordAddress.ShortAddr.Val == macPIB.macCoordShortAddress.Val &&
                                    params.MLME_SCAN_confirm.PANDescriptorList->CoordPANId.Val == macPIB.macPANId.Val )
                                {

                                    oldNetworkAvailable = TRUE;
                                }
                            }
NextPANDescriptor:			
							
							
                            ptr = (BYTE *)params.MLME_SCAN_confirm.PANDescriptorList->next;
                            nfree( params.MLME_SCAN_confirm.PANDescriptorList );
                            params.MLME_SCAN_confirm.PANDescriptorList = (PAN_DESCRIPTOR *)ptr;
                        }
						}
                        // If we didn't find any networks, we are no longer in the process of trying to join a network

                        // A ZigBee 2006 requirement: for processing rejoins
                        if( nwkStatus.flags.bits.bRejoinScan )
                        {
                            NETWORK_DESCRIPTOR *netDesc1;
                            NETWORK_DESCRIPTOR *netDesc2;

                            BYTE parentDepth = 0xFF;
                            BYTE latest_nwkUpdateId = 0x00;
                            BYTE extended_pan_match;

                            ZigBeeUnblockTx();
                            
                            ZigBeeBlockTx();

                            netDesc1 = params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor;
                            while (netDesc1)
                            {
                                netDesc2 = netDesc1->next;
                                nfree(netDesc1);
                                netDesc1 = netDesc2;
                            }
                            nwkStatus.flags.bits.bRejoinScan = 0;

                            TxBuffer[TxData++] = NWK_COMMAND_REJOIN_REQUEST;
                            TxBuffer[TxData++] = MY_CAPABILITY_INFO;

                            GetMACAddress(&tempLongAddress);
                            for(i = 0; i < 8; i++)
                            {
                                TxBuffer[TxHeader--] = tempLongAddress.v[7-i];
                            }
                            TxBuffer[TxHeader--] = NLME_GET_nwkBCSN();
                            TxBuffer[TxHeader--] = 1;   // radius of 1, as per errata
                            if ((!macPIB.macShortAddress.v[0]&& !macPIB.macShortAddress.v[1]) || \
                                (macPIB.macShortAddress.v[1] == 0xff && macPIB.macShortAddress.v[0] > 0xf7 ))
                            {
                                if( (!current_SAS.spas.shortAddr.v[0]&& !current_SAS.spas.shortAddr.v[1]) || \
                                    (current_SAS.spas.shortAddr.v[1] == 0xff && current_SAS.spas.shortAddr.v[0] > 0xf7 ) )
                                {
                                    #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
                                        macPIB.macShortAddress.Val = GenerateNewShortAddr();
                                    #endif
                                }

                                else
                                {
                                    macPIB.macShortAddress.v[0] = current_SAS.spas.shortAddr.v[0];
                                    macPIB.macShortAddress.v[1] = current_SAS.spas.shortAddr.v[1];
                                }
                            }

                            MLME_SET_macShortAddress_hw();
                            TxBuffer[TxHeader--] = macPIB.macShortAddress.byte.MSB;
                            TxBuffer[TxHeader--] = macPIB.macShortAddress.byte.LSB;

                            currentNeighborTableInfo.parentNeighborTableIndex = MAX_NEIGHBORS;
                            for (i = 0; i < MAX_NEIGHBORS; i++)
                            {
                                #ifdef USE_EXTERNAL_NVM
                                    pCurrentNeighborRecord = neighborTable + (WORD)i * (WORD)sizeof(NEIGHBOR_RECORD);
                                #else
                                    pCurrentNeighborRecord = &(neighborTable[i]);
                                #endif
                                GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

                                extended_pan_match = memcmp((BYTE *)(&currentNeighborRecord.ExtendedPANID),
                                        (BYTE *)(&currentNeighborTableInfo.nwkExtendedPANID),
                                        EXTENDED_PAN_SIZE);
                                /* This if block for debugging startconfirm bug only */

                                if ( ( currentNeighborRecord.deviceInfo.bits.bInUse ) &&
                                     ( currentNeighborRecord.deviceInfo.bits.LQI >= MINIMUM_JOIN_LQI ) &&
                                     ( !extended_pan_match ) )
                                {
                                    if (latest_nwkUpdateId < currentNeighborRecord.nwkUpdateId)
                                    {
                                        latest_nwkUpdateId = currentNeighborRecord.nwkUpdateId;
                                        currentNeighborTableInfo.parentNeighborTableIndex = i;
                                        parentDepth = currentNeighborRecord.deviceInfo.bits.Depth;
                                    }
                                    if (latest_nwkUpdateId == currentNeighborRecord.nwkUpdateId)
                                    {
                                        if (parentDepth > currentNeighborRecord.deviceInfo.bits.Depth)
                                        {
                                            parentDepth = currentNeighborRecord.deviceInfo.bits.Depth;
                                            latest_nwkUpdateId = currentNeighborRecord.nwkUpdateId;
                                            currentNeighborTableInfo.parentNeighborTableIndex = i;
                                        }
                                    }
                                }
                            }
                            if (currentNeighborTableInfo.parentNeighborTableIndex != MAX_NEIGHBORS)
                            {
                                //BYTE i;
                                #ifdef USE_EXTERNAL_NVM
                                    pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                                #else
                                    pCurrentNeighborRecord = &(neighborTable[currentNeighborTableInfo.parentNeighborTableIndex]);
                                #endif
                                GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

                                // Set the PAN ID to the PAN that we are trying to join.  If this fails, we'll have to clear it.
                                // macPIB.macPANId = params.NLME_JOIN_request.PANId;
                                macPIB.macPANId = currentNeighborRecord.panID;
                                MLME_SET_macPANId_hw();

                                /*for (i = 0; i < 8; i++)
                                {
                                    currentNeighborTableInfo.nwkExtendedPANID.v[i] = \
                                        currentNeighborRecord.ExtendedPANID.v[i];
                                }
                                PutNeighborTableInfo();*/

                                /* Added at NTS */
                                TxBuffer[TxHeader--] = currentNeighborRecord.shortAddr.byte.MSB;
                                TxBuffer[TxHeader--] = currentNeighborRecord.shortAddr.byte.LSB;
                            }
                            else
                            {
                                //rejoin_retries++;
                            
                                timeBetweenScanDuringRejoin = TickGet();
                                ZigBeeStatus.flags.bits.bActiveScanToRejoin = 1;
                                nwkStatus.flags.bits.bRejoinScan = 1;
                                
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }

                        #ifdef I_SUPPORT_SECURITY
                            if ( rejoinWithSec )
                            {
                                /* currently we attempt all rejoins securely */
                                //rejoinWithSec = 0;
                                TxBuffer[TxHeader--] = 0x02 | NWK_IEEE_SRC_ADDR;
                            }
                            else
                        #endif
                            {
                                TxBuffer[TxHeader--] = NWK_IEEE_SRC_ADDR; // nwkFrameControlMSB TODO check security setting for this
                            }

                            TxBuffer[TxHeader--] = NWK_FRAME_CMD | (nwkProtocolVersion<<2);    // nwkFrameControlLSB

                            Prepare_MCPS_DATA_request( currentNeighborRecord.shortAddr.Val, &i );
                            nwkStatus.flags.bits.bRejoinInProgress = 1;
                            /* A ZigBee 2006 requirement:  for rejoin process to send out beacon request first */
                            nwkStatus.rejoinStartTick = TickGet();
                            if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                                nwkStatus.rejoinCommandSent = 0x01;

                                nwkStatus.rejoinStartTick = TickGet();
                            }//#endif
                            /* A ZigBee 2006 requirement: Force a data request from my new parent */
                            macPIB.macCoordShortAddress = currentNeighborRecord.shortAddr;
                            ZigBeeStatus.flags.bits.rejoin = 1;
                            
                            /*Since Rejoin Request command frame is being transmitted, 
                            we should not initiate scanning untill REjoin Response command 
                            frame is received */
                            ZigBeeStatus.flags.bits.bActiveScanToRejoin = 0;
                            
                            phyPIB.phyCurrentChannel = currentNeighborRecord.LogicalChannel;
                            MACEnable();

                            return MCPS_DATA_request;
                        }

                        // Status is already in place
                        return NLME_NETWORK_DISCOVERY_confirm;
                    }
                    else
                    {
                        // We just did an orphan scan.  Return the result to the next layer.  If we got SUCCESS,
                        // pass it back up and go set all of our address, parent, and join information.  Otherwise,
                        // change the status to NWK_NO_NETWORKS (instead of NO_BEACON - NWK errata)
                    #ifdef ZCP_PRINTOUT
                        printf("\r\nScan Confirm Result(Orphan Scan):");
                        printf("\r\nUnscanned Channels: ");
                        PrintChar(params.MLME_SCAN_confirm.UnscannedChannels.v[3]);
                        PrintChar(params.MLME_SCAN_confirm.UnscannedChannels.v[2]);
                        PrintChar(params.MLME_SCAN_confirm.UnscannedChannels.v[1]);
                        PrintChar(params.MLME_SCAN_confirm.UnscannedChannels.v[0]);
                        printf("\r\nResultListSize: ");
                        PrintChar(params.MLME_SCAN_confirm.ResultListSize);
                        printf("\r\nmacShortAddress: ");
                        PrintChar(macPIB.macShortAddress.byte.MSB);
                        PrintChar(macPIB.macShortAddress.byte.LSB);
                        printf("\r\nmacPANID: ");
                        PrintChar(macPIB.macPANId.byte.MSB);
                        PrintChar(macPIB.macPANId.byte.LSB);
                        printf("\r\n");
                    #endif
                        if (!params.MLME_SCAN_confirm.status)
                        {
                            // The orphan scan was successful.  Set our PAN ID in the hardware.
                            MLME_SET_macPANId_hw();

                            // Set the rest of my address and parent information, and return NLME_JOIN_confirm.
                            // Note that parentNeighborTableIndex is already set.
                            params.NLME_JOIN_confirm.ShortAddress.Val = macPIB.macShortAddress.Val;
                            goto SetMyAddressInformation;
                        }
                        else
                        {
                            // Moved to allow more than one orphan attempt.
                            // The orphan scan failed.  Clear our old parent relationship.
                            #if defined(USE_EXTERNAL_NVM)
                                pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                            #else
                                pCurrentNeighborRecord = &(neighborTable[currentNeighborTableInfo.parentNeighborTableIndex]);
                            #endif
                            GetNeighborRecord(&currentNeighborRecord, pCurrentNeighborRecord );
                            currentNeighborRecord.deviceInfo.bits.Relationship = NEIGHBOR_IS_NONE;
                            PutNeighborRecord(pCurrentNeighborRecord, &currentNeighborRecord );

                            currentNeighborTableInfo.parentNeighborTableIndex = INVALID_NEIGHBOR_KEY;
                            PutNeighborTableInfo();

                            // If we try to join again, it will have to be as a new node.
                            ZigBeeStatus.flags.bits.bTryOrphanJoin = 0;

                            params.NLME_JOIN_confirm.ShortAddress.Val = 0xFFFF;
                            params.NLME_JOIN_confirm.Status = NWK_NO_NETWORKS;
                        }
                        return NLME_JOIN_confirm;
                    }
                }//#endif // I_AM_COORDINATOR
                break;

            // ---------------------------------------------------------------------
            case MLME_COMM_STATUS_indication:
                if(NOW_I_AM_A_CORDINATOR() || NOW_I_AM_A_ROUTER()){//#if defined(I_AM_COORDINATOR) || defined(I_AM_ROUTER)
                    #ifdef ZCP_PRINTOUT
                    printf("\r\nMLME_COMM_STATUS_indication Result: ");
                    printf("\r\nPANID: ");
                    PrintChar(params.MLME_COMM_STATUS_indication.PANId.v[1]);
                    PrintChar(params.MLME_COMM_STATUS_indication.PANId.v[0]);
                    printf("\r\nSrcAddress: ");
                    if( params.MLME_COMM_STATUS_indication.SrcAddrMode == 0x03 )
                    {
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.LongAddr.v[7]);
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.LongAddr.v[6]);
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.LongAddr.v[5]);
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.LongAddr.v[4]);
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.LongAddr.v[3]);
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.LongAddr.v[2]);
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.LongAddr.v[1]);
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.LongAddr.v[0]);
                    } else {
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.ShortAddr.v[1]);
                        PrintChar(params.MLME_COMM_STATUS_indication.SrcAddr.ShortAddr.v[0]);
                    }
                    printf("\r\nDstAddress: ");
                    if( params.MLME_COMM_STATUS_indication.DstAddrMode == 0x03 )
                    {
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr.v[7]);
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr.v[6]);
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr.v[5]);
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr.v[4]);
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr.v[3]);
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr.v[2]);
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr.v[1]);
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr.v[0]);
                    } else {
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.ShortAddr.v[1]);
                        PrintChar(params.MLME_COMM_STATUS_indication.DstAddr.ShortAddr.v[0]);
                    }
                    #endif
                    i = NWKLookupNodeByLongAddr( &(params.MLME_COMM_STATUS_indication.DstAddr.LongAddr) );
                    if (params.MLME_COMM_STATUS_indication.status)
                    {
                        // We could not send the message to the node.
                        if (i != INVALID_NEIGHBOR_KEY)
                        {
                            // Remove the neighbor entry.  We cannot reuse the address, because we could only
                            // reuse the last address, and another node might have already received that address.
                            /* Keep the neighbor table for now, until permanent fix is found(rare) */
                            //RemoveNeighborTableEntry();
                        }
                        return NO_PRIMITIVE;
                    }
                    else
                    {
                        // Notify the upper layers that a new node has joined.
                        // NOTE - since the node wasn't on the network yet, we know the destination
                        // destination address is the 64-bit address, not the 16-bit address.
                        if (i != INVALID_NEIGHBOR_KEY)
                        {
                            // ExtendedAddress already in place
                            /* ensure that long address is correctly in place */
                            LONG_ADDR   tempAddr;
                            BYTE i;
                            
                            for(i=0; i < 8; i++)
                            {
                                tempAddr.v[i] = params.MLME_COMM_STATUS_indication.DstAddr.v[i];
                            }
                            for(i=0; i < 8; i++)
                            {
                                params.NLME_JOIN_indication.ExtendedAddress.v[i] = tempAddr.v[i];
                            }
                            
                            params.NLME_JOIN_indication.NetworkAddress = currentNeighborRecord.shortAddr;
                            params.NLME_JOIN_indication.CapabilityInformation = nwkStatus.lastCapabilityInformation;
                            params.NLME_JOIN_indication.secureRejoin = FALSE; // Since association join was done
                            currentNeighborRecord.bSecured = FALSE;
                            params.NLME_JOIN_indication.RejoinNetwork = rejoin_network;
                            return NLME_JOIN_indication;
                        }
                    }
                }//#endif
                break;


            case MLME_START_confirm:
                if(NOW_I_AM_A_CORDINATOR()){//#if defined (I_AM_COORDINATOR)
                    // NOTE: ZigBeeStatus.flags.bits.bNetworkFormed = 1;
                    // should be set in the application code upon receipt of this
                    // primitive to avoid duplication.

                    // Destroy the network discovery information.
                    if (nwkStatus.discoveryInfo.channelList)
                    {
                        nfree( nwkStatus.discoveryInfo.channelList );
                    }

                    // If the start was successful, enable routing capability.
                    if (!params.MLME_START_confirm.status)
                    {
                        nwkStatus.flags.bits.bCanRoute = 1;
                        #if (I_SUPPORT_ROUTING_TABLE_AGING == 1)
                            nwkStatus.routingTableAgeTick = TickGet();
                        #endif
                        // Update the nwkLinkStatusPeriod for transmitting Link Status Command and Neighbor table aging
                        #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                            nwkStatus.nwkLinkStatusPeriod = TickGet();
                            nwkStatus.moreLinkStatusCmdPending = 0;
                        #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                    }

                    // Initialize other internal variables
                    currentNeighborTableInfo.depth                  = 0;
                    #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
                        currentNeighborTableInfo.cSkip.Val              = CSKIP_DEPTH_0;
                        if (!currentNeighborTableInfo.flags.bits.bChildAddressInfoValid)
                        {
                            currentNeighborTableInfo.nextEndDeviceAddr.Val  = 0x0000 + (WORD)CSKIP_DEPTH_0 * (WORD)NIB_nwkMaxRouters + (WORD)1;
                            currentNeighborTableInfo.nextRouterAddr.Val     = 0x0000 + 1;
                            currentNeighborTableInfo.numChildren            = 0;
                            currentNeighborTableInfo.numChildRouters        = 0;
                            currentNeighborTableInfo.flags.bits.bChildAddressInfoValid = 1;
                        }
                    #else
                        currentNeighborTableInfo.numChildren            = 0;
                        currentNeighborTableInfo.nextChildAddr.Val = GenerateNewShortAddr();
                        nwkStatus.flags.bits.bNextChildAddressUsed = FALSE;
                    #endif
                    PutNeighborTableInfo();
                    SetBeaconPayload(TRUE);

                    // Status already in place
                    ZigBeeStatus.flags.bits.bNetworkFormed = 1;
                    return NLME_NETWORK_FORMATION_confirm;
                }else if(NOW_I_AM_A_FFD() || NOW_I_AM_A_ROUTER() || NOW_I_AM_A_CORDINATOR()){//#elif defined(I_AM_FFD)
                    // If the start was successful, enable routing capability.

                    if (!params.MLME_START_confirm.status)
                    {
                        nwkStatus.flags.bits.bCanRoute = 1;
                        #if (I_SUPPORT_ROUTING_TABLE_AGING == 1)
                            nwkStatus.routingTableAgeTick = TickGet();
                        #endif
                        // Update the nwkLinkStatusPeriod for transmitting Link Status Command and Neighbor table aging
                        #ifndef I_SUPPORT_SECURITY
                            #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                                nwkStatus.nwkLinkStatusPeriod = TickGet();
                                nwkStatus.moreLinkStatusCmdPending = 0;
                            #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
                        #endif // #ifndef I_SUPPORT_SECURITY
                    }

                    // Status already in place
                    return NLME_START_ROUTER_confirm;
                }else{//#else
                    return NO_PRIMITIVE;
                }//#endif
                break;


            case MLME_POLL_confirm:
                // If we received SUCCESS, leave the value and don't set bDataRequestComplete -
                // we need to stay awake to receive the data that will be coming.  If we get
                // NO_DATA, return SUCCESS and set bDataRequestComplete - we've heard from
                // our parent, but there is no incoming data.  Otherwise, return SYNC_FAILURE
                // because we cannot talk to our parent.
                #ifdef I_AM_RFD
                    ZigBeeStatus.flags.bits.bRequestingData = 0;
                #endif
                if (params.MLME_POLL_confirm.status)
                {
                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        ZigBeeStatus.flags.bits.bDataRequestComplete = 1;
                    }//#endif
                    if (params.MLME_POLL_confirm.status == NO_DATA)
                    {
                        params.NLME_SYNC_confirm.Status = SUCCESS;
                    }
                    else
                    {
                        params.NLME_SYNC_confirm.Status = NWK_SYNC_FAILURE;
                    }
                }
                return NLME_SYNC_confirm;
                break;

            // ---------------------------------------------------------------------
            case NLDE_DATA_request:
                {
                   // BYTE            msduHandle;
                   //#ifndef I_AM_RFD
                        BYTE        BTTIndex;
                   //     #ifndef I_AM_END_DEVICE
                            BYTE        routeStatus;
                   //     #endif
                   // #endif
                    //if (macPIB.macPANId.Val == 0xFFFF)
                    if(((NOW_I_AM_A_CORDINATOR())//#ifdef I_AM_COORDINATOR
                                && ( !ZigBeeStatus.flags.bits.bNetworkFormed ))
                            ||//#else
                                ((NOW_I_AM_NOT_A_CORDINATOR())&& ( !ZigBeeStatus.flags.bits.bNetworkJoined ))
                            )//#endif
                    {
                        // We are not associated.  We can't send any messages until we are part of a network.
                        params.NLDE_DATA_confirm.NsduHandle = params.NLDE_DATA_request.NsduHandle;
                        params.NLDE_DATA_confirm.Status = NWK_INVALID_REQUEST;
                        ZigBeeUnblockTx();
                        return NLDE_DATA_confirm;
                    }

                    // For all the broadcast frames, Route Discovery should be disabled.
                    if ( params.NLDE_DATA_request.DstAddr.Val >= 0xFFF7 )
                    {
                        params.NLDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
                    }

                    if (NOW_I_AM_NOT_A_RFD()){//#ifndef I_AM_RFD


                        if (
                            (params.NLDE_DATA_request.DstAddr.Val == 0xFFFF) ||
                            (params.NLDE_DATA_request.DstAddr.Val == 0xFFFD) ||
                            (params.NLDE_DATA_request.DstAddr.Val == 0xFFFC)
                            #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                            || ((params.NLDE_DATA_request.DstAddrMode == 0x01) &&
                               (params.NLDE_DATA_request.MulticastMode == MEMBER_MODE_MULTICAST) )
                            #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                           )
                        {
                            #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                PERSISTENCE_PIB currentPIB;

                                GetPersistenceStorage((void *)&currentPIB);
                            #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)

                            // Handle broadcast messages
                            if (!CreateNewBTR( &BTTIndex ))
                            {
                                params.NLDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;


                                ZigBeeUnblockTx();
                                return NLDE_DATA_confirm;
                            }


                            // Indicate that the message was from our upper layers.
                            nwkStatus.BTT[BTTIndex]->btrInfo.bMessageFromUpperLayers = 1;

                            // Set the jitter time so it will expire and transmit right away.
                            nwkStatus.BTT[BTTIndex]->broadcastJitterTimer.Val += BROADCAST_JITTER;

                            // Set the true length of the entire NWK data - current header and data combined.
                            params.NLDE_DATA_request.NsduLength = (TX_BUFFER_SIZE-1-TxHeader) + TxData;

                            // Save off the NWK header information
                            nwkStatus.BTT[BTTIndex]->dataLength                   = params.NLDE_DATA_request.NsduLength;
                            nwkStatus.BTT[BTTIndex]->nwkFrameControlLSB.Val       = NWK_FRAME_DATA | (nwkProtocolVersion<<2) | (params.NLDE_DATA_request.DiscoverRoute<<6);
                            nwkStatus.BTT[BTTIndex]->nwkFrameControlLSB.bits.routeDiscovery = ROUTE_DISCOVERY_SUPPRESS; // For broadcast/multicast frames this bit should be set to Zero.
                            nwkStatus.BTT[BTTIndex]->nwkFrameControlMSB.Val       = (params.NLDE_DATA_request.SecurityEnable<<1);
                            #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                            if ( (params.NLDE_DATA_request.DstAddrMode == 0x01) &&
                                 (currentPIB.nwkUseMulticast) &&
                                 (params.NLDE_DATA_request.MulticastMode == MEMBER_MODE_MULTICAST) )
                            {
                                nwkStatus.BTT[BTTIndex]->nwkFrameControlMSB.bits.multicastFlag = 1;
                            }
                            #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                            nwkStatus.BTT[BTTIndex]->nwkDestinationAddress        = params.NLDE_DATA_request.DstAddr;
                            nwkStatus.BTT[BTTIndex]->nwkSourceAddress             = macPIB.macShortAddress;    //params.NLDE_DATA_request.SrcAddr;
                            if ( params.NLDE_DATA_request.BroadcastRadius == 0x00 )
                            {
                                nwkStatus.BTT[BTTIndex]->nwkRadius                = DEFAULT_RADIUS;
                            }
                            else
                            {
                                nwkStatus.BTT[BTTIndex]->nwkRadius                = params.NLDE_DATA_request.BroadcastRadius;
                            }
                            nwkStatus.BTT[BTTIndex]->nwkSequenceNumber            = params.NLDE_DATA_request.NsduHandle;

                            #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                                nwkStatus.BTT[BTTIndex]->nwkMulticastFields.bits.mode = params.NLDE_DATA_request.MulticastMode;
                                nwkStatus.BTT[BTTIndex]->nwkMulticastFields.bits.nonMemberRadius = currentPIB.nwkMulticastNonMemberRadius;
                                nwkStatus.BTT[BTTIndex]->nwkMulticastFields.bits.maxNonMemberRadius = currentPIB.nwkMulticastMaxNonMemberRadius;
                            #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)

                            // Allocate space to save off the message
                            if ((nwkStatus.BTT[BTTIndex]->dataPtr = (BYTE *)SRAMalloc( params.NLDE_DATA_request.NsduLength )) == NULL)
                            {

                                // We do not have room to store this broadcast packet.  Destroy the BTR.
                                nfree( nwkStatus.BTT[BTTIndex] );
                                params.NLDE_DATA_confirm.NsduHandle = params.NLDE_DATA_request.NsduHandle;
                                params.NLDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                                ZigBeeUnblockTx();

                                return NLDE_DATA_confirm;
                            }

                            // Save off the NWK payload - APS header plus APS payload
                            i = 0;

                            // Set the network status so we can begin transmitting these messages
                            // in the background.
                            nwkStatus.flags.bits.bSendingBroadcastMessage = 1;

                            // Copy the APS Header
                            #ifndef I_AM_ACTIVE_END_DEVICE

                            while (TxHeader < TX_BUFFER_SIZE-1)
                            {
                                nwkStatus.BTT[BTTIndex]->dataPtr[i++] = TxBuffer[++TxHeader];
                            }
                            // Copy the APS Payload
                            params.NLDE_DATA_request.NsduLength = 0;
                            while (TxData)
                            {
                                nwkStatus.BTT[BTTIndex]->dataPtr[i++] = TxBuffer[params.NLDE_DATA_request.NsduLength++];
                                TxData--;
                            }
                        #ifdef I_SUPPORT_SECURITY
                            nwkStatus.BTT[BTTIndex]->btrInfo.bAlreadySecured = 0x00;
                        #endif


                            // The message is buffered, so unblock Tx.
                            ZigBeeUnblockTx();

                            return NO_PRIMITIVE;

                            #endif
                        }
                    }//#endif

                    #if defined (I_AM_END_DEVICE)
                        macAddress = macPIB.macCoordShortAddress;
                    #else
                        #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                            if ( ( params.NLDE_DATA_request.DstAddrMode == 0x01 ) &&
                                 ( params.NLDE_DATA_request.MulticastMode == NON_MEMBER_MODE_MULTICAST ) )
                            {

                                if ( GetMulticastRoutingEntry ( params.NLDE_DATA_request.DstAddr, &macAddress, &routeStatus ) )
                                {
                                    routeStatus = ROUTE_SEND_TO_MAC_ADDRESS;
                                }
                            }
                            else
                        #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
                            {
                                #if I_SUPPORT_CONCENTRATOR
                                    BYTE sourceRouteIndex;
                                    WORD HopsInSourceRoute;
                                    SHORT_ADDR macDestAddress;
                                    BYTE *sourceSubFrame;
                                    BYTE result = NO_PRIMITIVE;

                                    sourceRouteIndex = GetIndexInRouteRecord(params.NLDE_DATA_request.DstAddr);
                                    if( sourceRouteIndex != NOT_VALID)
                                    {
                                        sourceSubFrame = CreateSourceRouteSubframe( sourceRouteIndex,
                                        &HopsInSourceRoute, &macDestAddress );

                                        if(sourceSubFrame != NULL )
                                        {
                                            result = LoadNwkDataHeader(macDestAddress, HopsInSourceRoute, sourceSubFrame);
                                            SRAMfree(sourceSubFrame );
                                        }
                                        return result;

                                    }
                                    else
                                #endif
                                    {
                                        routeStatus = GetRoutingAddress( TRUE, params.NLDE_DATA_request.DstAddr, params.NLDE_DATA_request.DiscoverRoute, &macAddress );
                                    }
                            }

                    #endif

                    #if !defined (I_AM_END_DEVICE)
                    switch (routeStatus)
                    {
                        case ROUTE_SEND_TO_MAC_ADDRESS:
                    #endif
                            // Add this frame to the list of frames waiting confirmation.
                            // Try to find an empty slot.
                             return ( LoadNwkDataHeader(macAddress, 0, NULL) );


                    #ifndef I_AM_END_DEVICE
                            break;
                      case ROUTE_AVAILABLE_TO_CONCENTRATOR:
                                /* back up data to be send here. Data will be send after getting
                                    NLDE DATA CONFIRM for route record frame */
                            #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                                if( DataToBeSendAfterRouteRecord.DataBackUp == NULL )
                                {
                                    BYTE *ptrBackUp;
                                    ptrBackUp = SRAMalloc( MAX_DATA_SIZE);
                                        if( ptrBackUp == NULL )
                                        {
                                            params.NLDE_DATA_confirm.NsduHandle =       params.NLDE_DATA_request.NsduHandle;
                                            params.NLDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                                            ZigBeeUnblockTx();
                                            return NLDE_DATA_confirm;
                                        }
                                        else
                                        {
                                            DataToBeSendAfterRouteRecord.DataBackUp = ptrBackUp;
                                            /* ensure that data packet will be sent after route record command */
                                            DataToBeSendAfterRouteRecord.routeRecordSend = TRUE;
                                            DataToBeSendAfterRouteRecord.dataLength =
                                            (TX_HEADER_START - TxHeader) + 8 + TxData;
                                            CreateNwkHeaderForDataAndStoreData
                                            ( params.APSDE_DATA_request.DiscoverRoute,  ptrBackUp);

                                        
                                            DataToBeSendAfterRouteRecord.macDestination = macAddress;
                                            DataToBeSendAfterRouteRecord.nsduHandle = params.NLDE_DATA_request.NsduHandle;;

                                            /* Send route record frame here */
                                            CreateRouteRecordFrame( macAddress);
                                            return MCPS_DATA_request;
                                        }
                                }
                                else
                                {
                                    params.NLDE_DATA_confirm.NsduHandle = params.NLDE_DATA_request.NsduHandle;
                                    params.NLDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                                    ZigBeeUnblockTx();
                                    return NLDE_DATA_confirm;

                                }



                            #endif

                            break;
                        case ROUTE_MESSAGE_BUFFERED:
                            // The Route Request is buffered for later transmission
                            return NO_PRIMITIVE;
                            break;

                        case ROUTE_DISCOVERY_ALREADY_INITIATED:
                        default:
                            // Cannot route or start a route request
                            params.NLDE_DATA_confirm.NsduHandle = params.NLDE_DATA_request.NsduHandle;
                            params.NLDE_DATA_confirm.Status = NWK_ROUTE_ERROR; //NWK_CANNOT_ROUTE;
                            ZigBeeUnblockTx();
                            return NLDE_DATA_confirm;
                    }
                    #endif
                }
                break;

            // ---------------------------------------------------------------------
            case NLME_NETWORK_DISCOVERY_request:
                if(NOW_I_AM_A_CORDINATOR()){//#ifdef I_AM_COORDINATOR
                    params.NLME_NETWORK_FORMATION_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_NETWORK_FORMATION_confirm;
                }else{//#else

                    if( nwkStatus.flags.bits.bNRejoin )
                    {
                        return NO_PRIMITIVE;
                    }
                    phyPIB.phyBackupChannel = phyPIB.phyCurrentChannel;

                    if (currentNeighborTableInfo.parentNeighborTableIndex != INVALID_NEIGHBOR_KEY)
                    {
                        if (!ZigBeeStatus.flags.bits.bNetworkJoined)
                        {
                            // An orphan scan failed.  Clear our old parent relationship.
                            #ifdef USE_EXTERNAL_NVM
                                pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                            #else
                                pCurrentNeighborRecord = &(neighborTable[currentNeighborTableInfo.parentNeighborTableIndex]);
                            #endif
                            GetNeighborRecord(&currentNeighborRecord, pCurrentNeighborRecord );
                            currentNeighborRecord.deviceInfo.bits.Relationship = NEIGHBOR_IS_NONE;
                            PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );

                            currentNeighborTableInfo.parentNeighborTableIndex = INVALID_NEIGHBOR_KEY;
                            PutNeighborTableInfo();
                        }

                        // If we try to join again, it will have to be as a new node.
                        ZigBeeStatus.flags.bits.bTryOrphanJoin = 0;
                    }

                    // Reset all the current neighbor table entries
                    /* During discovery retries if the last beacon is missed potential
                       parent remains to be zero. There is no need to reset neighbor table
                       entries during discovery. Existing entries are updated during beacon notify */
                   /* for (i=0; i < MAX_NEIGHBORS; i++)
                    {
                        #ifdef USE_EXTERNAL_NVM
                            pCurrentNeighborRecord = neighborTable + (WORD)i * (WORD)sizeof(NEIGHBOR_RECORD);
                        #else
                            pCurrentNeighborRecord = &(neighborTable[i]);
                        #endif
                        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
                        if (currentNeighborRecord.deviceInfo.bits.bInUse)
                        {
                            currentNeighborRecord.deviceInfo.bits.PotentialParent = 0;
                            PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
                        }
                    }*/

                    if (nwkStatus.discoveryInfo.channelList)
                    {
                        nfree( nwkStatus.discoveryInfo.channelList );
                    }

                    // Save off ScanChannels and ScanDuration - we'll need them again.
                    nwkStatus.lastScanChannels = params.NLME_NETWORK_DISCOVERY_request.ScanChannels;
                    nwkStatus.lastScanDuration = params.NLME_NETWORK_DISCOVERY_request.ScanDuration;

                    params.MLME_SCAN_request.ScanType = MAC_SCAN_ACTIVE_SCAN;
                    // ScanChannels is already in place
                    // ScanDuration is already in place
                    return MLME_SCAN_request;
    			}//#endif
                break;

            // ---------------------------------------------------------------------
            case NLME_NETWORK_FORMATION_request:
                if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
                    params.NLME_NETWORK_FORMATION_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_NETWORK_FORMATION_confirm;
                }else{//#else
                    if (nwkStatus.discoveryInfo.channelList)
                    {
                        nfree( nwkStatus.discoveryInfo.channelList );
                    }

                    // Save off ScanChannels and ScanDuration - we'll need them again.
                    nwkStatus.lastScanChannels.Val = params.NLME_NETWORK_FORMATION_request.ScanChannels.Val;
                    nwkStatus.lastScanDuration = params.NLME_NETWORK_FORMATION_request.ScanDuration;
                    nwkStatus.requestedPANId = params.NLME_NETWORK_FORMATION_request.PANId;
                    params.MLME_SCAN_request.ScanType = MAC_SCAN_ENERGY_DETECT;
                    // ScanChannels is already in place
                    // ScanDuration is already in place
                    params.MLME_SCAN_request.ScanChannels.Val = params.NLME_NETWORK_FORMATION_request.ScanChannels.Val;
                    params.MLME_SCAN_request.ScanDuration     = params.NLME_NETWORK_FORMATION_request.ScanDuration;
                    return MLME_SCAN_request;
                }//#endif
                break;

            // ---------------------------------------------------------------------
            case NLME_PERMIT_JOINING_request:
                if(NOW_I_AM_A_CORDINATOR() || NOW_I_AM_A_ROUTER()){//#if defined( I_AM_COORDINATOR ) || defined( I_AM_ROUTER )
                    // Disable any join time countdown that was in progress.
                    nwkStatus.flags.bits.bTimingJoinPermitDuration = 0;

                    if (params.NLME_PERMIT_JOINING_request.PermitDuration == 0x00)
                    {
                        macPIB.macAssociationPermit = FALSE;
                    }
                    else
                    {
                        macPIB.macAssociationPermit = TRUE;
                        
                        if (params.NLME_PERMIT_JOINING_request.PermitDuration != 0xFF)
                        {
                            // Set up the join time countdown.
                            nwkStatus.joinPermitDuration = params.NLME_PERMIT_JOINING_request.PermitDuration;
                            nwkStatus.joinDurationStart = TickGet();
                            nwkStatus.flags.bits.bTimingJoinPermitDuration = 1;
                        }
                    }

                    /* changed at NTS - should not change router/end_device capacity with this call in beacon payload */
                    SetBeaconPayload(params.NLME_PERMIT_JOINING_request._updatePayload);
                    params.NLME_PERMIT_JOINING_confirm.Status = NWK_SUCCESS;
                    return NLME_PERMIT_JOINING_confirm;
                }else{//#else
                    params.NLME_PERMIT_JOINING_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_PERMIT_JOINING_confirm;
                }//#endif
                break;

            // ---------------------------------------------------------------------
            case NLME_START_ROUTER_request:
                // TODO The spec implies that a ZigBee Coordinator can issue this primitive,
                // but it doesn't make sense - especially where it's supposed to pull info
                // from its parent's neighbor table entry.  So we'll just let FFD routers
                // and end devices do it for now.
                if(NOW_I_AM_A_CORDINATOR() || NOW_I_AM_A_RFD()){//#if defined(I_AM_RFD) || defined(I_AM_COORDINATOR)
                    params.NLME_START_ROUTER_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_START_ROUTER_confirm;
                }else{//#else
                    // If we are not associated with a network, we cannot do this.

                    if(((NOW_I_AM_A_CORDINATOR())//#ifdef I_AM_COORDINATOR
                                && ( !ZigBeeStatus.flags.bits.bNetworkFormed ))
                            ||//#else
                                ((NOW_I_AM_NOT_A_CORDINATOR())&& ( !ZigBeeStatus.flags.bits.bNetworkJoined ))
                            )//#endif
                    {
                        params.NLME_START_ROUTER_confirm.Status = NWK_INVALID_REQUEST;
                        return NLME_START_ROUTER_confirm;
                    }

                    i = (BYTE)params.NLME_START_ROUTER_request.BatteryLifeExtension;

                    // Set PANID, Beacon Order, and Superframe to my parent's values.
                    // Since we only support nonbeacon networks, we'll use the constants for
                    // Beacon Order and Superframe Order.
                    #ifdef USE_EXTERNAL_NVM
                        pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                    #else
                        pCurrentNeighborRecord = &neighborTable[currentNeighborTableInfo.parentNeighborTableIndex];
                    #endif
                    GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

                    // Clear the ability to route frames.
                    nwkStatus.flags.bits.bCanRoute = 0;

                    params.MLME_START_request.PANId = currentNeighborRecord.panID;
                    params.MLME_START_request.LogicalChannel = currentNeighborRecord.LogicalChannel;
                    params.MLME_START_request.BeaconOrder = MAC_PIB_macBeaconOrder;
                    params.MLME_START_request.SuperframeOrder = MAC_PIB_macSuperframeOrder;
                    params.MLME_START_request.fields.Val = 0; // PANCoordinator and CoordRealignment = FALSE
                    if (i)
                    {
                        params.MLME_START_request.fields.bits.BatteryLifeExtension = 1;
                    }
                    // TODO how to set params.MLME_START_request.fields.bits.SecurityEnable

                    return MLME_START_request;
                }//#endif
                break;

            // ---------------------------------------------------------------------
            case NLME_JOIN_request:
                if(NOW_I_AM_A_CORDINATOR()){//#ifdef I_AM_COORDINATOR
                    params.NLME_JOIN_confirm.ShortAddress.Val = 0xFFFF;
                    params.NLME_JOIN_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_JOIN_confirm;
                }else{//#else // I_AM_COORDINATOR

                    if (nwkStatus.flags.bits.bNRejoin)
                    {
                          return NO_PRIMITIVE;
                    }
                    #ifndef NIB_STATIC_IMPLEMENTATION
                        NIB.nwkCapabilityInformation = params.NLME_JOIN_request.CapabilityInformation.Val;
                    #endif
                    /* For ZigBee 2006: Handle the new Rejoining procedure when RejoinNetwork parameter = 0x02 */
                    if (/*ZigBeeStatus.flags.bits.bNetworkJoined &&*/(params.NLME_JOIN_request.RejoinNetwork == REJOIN_PROCESS) )
                    {
                        // When all the retries has exhausted and no potential devices
                        // exists, then we have to give confirmation to upper layer.
                        if (INVALID_NEIGHBOR_KEY == NWKLookupNodeByExtendedPANId(&params.NLME_JOIN_request.ExtendedPANID))
                        {
                            params.NLME_JOIN_confirm.ShortAddress.Val = 0xffff;
                            params.NLME_JOIN_confirm.Status = NWK_NO_NETWORKS;
                            return NLME_JOIN_confirm;
                        }

                        // We need to clear the neighbor table entries to perform Rejoin to New Parent.
                        // Neighbor table will be updated when the device performs scanning before rejoin.
                        ClearNeighborTable();
                        GetNeighborTableInfo();
                        if (currentNeighborTableInfo.parentNeighborTableIndex != MAX_NEIGHBORS)
                        {
                            #ifdef USE_EXTERNAL_NVM
                                pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                            #else
                                pCurrentNeighborRecord = &(neighborTable[currentNeighborTableInfo.parentNeighborTableIndex]);
                            #endif
                            GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
                            currentNeighborRecord.deviceInfo.Val = 0;
                            PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
                        }

                        BYTE i;

                        nwkStatus.rejoinExtendedPANId = params.NLME_JOIN_request.ExtendedPANID;

                        /* Added at NTS: do active scan first */
                        nwkStatus.flags.bits.bRejoinScan = 1;
                        params.MLME_SCAN_request.ScanType = MAC_SCAN_ACTIVE_SCAN;
                        /* Zigbee 2006:  Be sure that these parameters are copied over - dont assume alignment here*/
                        params.MLME_SCAN_request.ScanChannels = params.NLME_JOIN_request.ScanChannels;
                        params.MLME_SCAN_request.ScanDuration = NWK_SCAN_DURTION;

                        
                        for (i = 0; i < 8; i++)
                        {
                            currentNeighborTableInfo.nwkExtendedPANID.v[i] = \
                                params.NLME_JOIN_request.ExtendedPANID.v[i];
                        }
                        PutNeighborTableInfo();
                        return MLME_SCAN_request;


                    }
                    else
                    {

                        if (params.NLME_JOIN_request.RejoinNetwork == FALSE)
                        {
                            BYTE    parentDepth;
                            BYTE latest_nwkUpdateId;
                            // We are trying to join as a new node.  Make sure our address allocation
                            // information will be updated.  NOTE - the application is responsible for
                            // making sure all the child nodes have been kicked off!
                            #ifndef I_AM_END_DEVICE
                                #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
                                    currentNeighborTableInfo.flags.bits.bChildAddressInfoValid = 0;
                                    PutNeighborTableInfo();
                                #endif
                            #endif

                            // Set the status here in case we cannot even try to associate.  If we
                            // try and fail, we'll come back to TryToJoinPotentialParent.  If we fail
                            // from that, we need to return the MLME_ASSOCIATE_confirm status, which
                            // will be already in place.
                            params.NLME_JOIN_confirm.Status = NWK_NOT_PERMITTED;

                            // Look for a parent on the specified PAN that is allowing joins and has a good link quality.
                            // If there are more than one, choose the lowest depth.
TryToJoinPotentialParent:
                            currentNeighborTableInfo.parentNeighborTableIndex = MAX_NEIGHBORS;
                            parentDepth = 0xFF;
                            latest_nwkUpdateId = 0x00;
                            BYTE extended_pan_match;
                            for (i = 0; i < MAX_NEIGHBORS; i++)
                            {
                                #ifdef USE_EXTERNAL_NVM
                                    pCurrentNeighborRecord = neighborTable + (WORD)i * (WORD)sizeof(NEIGHBOR_RECORD);
                                #else
                                    pCurrentNeighborRecord = &(neighborTable[i]);
                                #endif
                                GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

                                extended_pan_match = memcmp((BYTE *)(&currentNeighborRecord.ExtendedPANID),
                                        (BYTE *)(&params.NLME_JOIN_request.ExtendedPANID),
                                         EXTENDED_PAN_SIZE);
                                /* This if block for debugging startconfirm bug only */
                   #ifdef DEBUG_LOG
                                if ( extended_pan_match == 0x00 )
                                {
                                    PotentialParentFound |= 0x10;
                                    PotentialParentFound |= ( 0x20 & ( currentNeighborRecord.deviceInfo.bits.bInUse << 5 ) ) ;
                                    PotentialParentFound |= ( 0x02 & ( currentNeighborRecord.deviceInfo.bits.PotentialParent << 1 ) ) ;
                                    PotentialParentFound |= ( 0x80 & ( (currentNeighborRecord.deviceInfo.bits.LQI >= MINIMUM_JOIN_LQI) << 7 ) ) ;
                                }
                   #endif
                                if (currentNeighborRecord.deviceInfo.bits.bInUse &&
                                    currentNeighborRecord.deviceInfo.bits.PermitJoining &&
                                    currentNeighborRecord.deviceInfo.bits.PotentialParent &&
                                    (currentNeighborRecord.deviceInfo.bits.LQI >= MINIMUM_JOIN_LQI) &&
                                    !extended_pan_match )
                                {
                                    if (latest_nwkUpdateId < currentNeighborRecord.nwkUpdateId)
                                    {
                                        latest_nwkUpdateId = currentNeighborRecord.nwkUpdateId;
                                        currentNeighborTableInfo.parentNeighborTableIndex = i;
                                        parentDepth = currentNeighborRecord.deviceInfo.bits.Depth;
                                    }
                                    if (latest_nwkUpdateId == currentNeighborRecord.nwkUpdateId)
                                    {
                                        if (parentDepth > currentNeighborRecord.deviceInfo.bits.Depth)
                                        {
                                            parentDepth = currentNeighborRecord.deviceInfo.bits.Depth;
                                            latest_nwkUpdateId = currentNeighborRecord.nwkUpdateId;
                                            currentNeighborTableInfo.parentNeighborTableIndex = i;
                                        }
                                    }
                                }
                                #ifdef DEBUG_LOG
                                else
                                {
                                    if( currentNeighborRecord.deviceInfo.bits.bInUse )
                                    {
                                        LOG_ASSERT(DEBUG_LOG_INFO, "In use" == 0);

                                        if( extended_pan_match )
                                        {
                                            LOG_ASSERT(DEBUG_LOG_INFO, "EPID Fail" == 0);
                                        }
                                        else if( !currentNeighborRecord.deviceInfo.bits.PermitJoining )
                                        {
                                            LOG_ASSERT(DEBUG_LOG_INFO, "No PJ" == 0);
                                        }
                                        else if( !currentNeighborRecord.deviceInfo.bits.PotentialParent )
                                        {
                                            LOG_ASSERT(DEBUG_LOG_INFO, "No PP" == 0);
                                        }
                                        else if( !(currentNeighborRecord.deviceInfo.bits.LQI >= MINIMUM_JOIN_LQI) )
                                        {
                                            LOG_ASSERT(DEBUG_LOG_INFO, "LQI" == 0);
                                        }
                                        else
                                        {
                                            LOG_ASSERT(DEBUG_LOG_INFO, "Failing Reason Unknown" == 0);
                                        }
                                    }
                                }
                                #endif
                            }
                            if (currentNeighborTableInfo.parentNeighborTableIndex != MAX_NEIGHBORS)
                            {
                                BYTE i;
                                #ifdef USE_EXTERNAL_NVM
                                    pCurrentNeighborRecord = neighborTable + (WORD)currentNeighborTableInfo.parentNeighborTableIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                                #else
                                    pCurrentNeighborRecord = &(neighborTable[currentNeighborTableInfo.parentNeighborTableIndex]);
                                #endif
                                GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

                                // Set the PAN ID to the PAN that we are trying to join.  If this fails, we'll have to clear it.
                               // macPIB.macPANId = params.NLME_JOIN_request.PANId;
                                macPIB.macPANId = currentNeighborRecord.panID;
                                for (i = 0; i < 8; i++)
                                {
                                    currentNeighborTableInfo.nwkExtendedPANID.v[i] = \
                                        params.NLME_JOIN_request.ExtendedPANID.v[i];
                                }
                                PutNeighborTableInfo();

                                MLME_SET_macPANId_hw();

                                // CoordPANId already in place
                                // SecurityEnable already in place
                                ZDOGetCapabilityInfo( &i );
                                params.MLME_ASSOCIATE_request.LogicalChannel = currentNeighborRecord.LogicalChannel;
                            #ifdef ZCP_DEBUG
                                if( currentNeighborRecord.shortAddr.Val == 0xfffe)
                                {
                                    macPIB.macCoordShortAddress.Val = 0xfffe;
                                    params.MLME_ASSOCIATE_request.CoordAddrMode = 0x03;
                                    params.MLME_ASSOCIATE_request.CoordAddress.LongAddr = currentNeighborRecord.longAddr;
                                }  else
                            #endif
                                {
                                    params.MLME_ASSOCIATE_request.CoordAddrMode = 0x02; // short address
                                    params.MLME_ASSOCIATE_request.CoordAddress.ShortAddr = currentNeighborRecord.shortAddr;
                                }
                                params.MLME_ASSOCIATE_request.CapabilityInformation.Val = i;

                                params.MLME_ASSOCIATE_request.CoordPANId.Val = currentNeighborRecord.panID.Val;

                                #ifdef I_SUPPORT_SECURITY_SPEC
                               {
                                    BYTE ActiveKeyIndex;
                                    GetNwkActiveKeyNumber(&ActiveKeyIndex);

                                    if( ActiveKeyIndex == 1 || ActiveKeyIndex == 2 )
                                    {
                                        params.MLME_ASSOCIATE_request.SecurityEnable = TRUE;
                                    } else {
                                        params.MLME_ASSOCIATE_request.SecurityEnable = FALSE;
                                    }
                                }
                                #endif
                                return MLME_ASSOCIATE_request;
                            }
                            else
                            {
                                // Status either set above or left over from MLME_ASSOCIATE_confirm
                                params.NLME_JOIN_confirm.ShortAddress.Val = 0xFFFF;
                                params.NLME_JOIN_confirm.Status = NWK_NOT_PERMITTED; //0x94;
                                return NLME_JOIN_confirm;
                            }
                        }
                        else /* RejoinNetwork = TRUE ---> 0x01 */
                        {
                            BYTE i;
                            //nwkStatus.flags.bits.bRejoinScan = 1;
                            nwkStatus.rejoinExtendedPANId = params.NLME_JOIN_request.ExtendedPANID;
                            // Scan Channels and Scan Duration are already in place.

                            /* Change for Zigbee 2006 -  Manadatory test 3.9. The NLME_LEAVE-request/Rejoin
                               must be handled differently from a NLME_RESET-request
                               A reset must be followed by an orphan scan(orphan notification); while a leave/rejoin
                               is followed by an active scan(association request)
                            */
                            /* if we were ever on the network, but asked to leave with the rejoin = TRUE then we
                             * must send out the rejoin_request when we first come back on the network
                             * Takes care of both 3.9 and 3.73 Mandatory tests here
                            */

                            if(ZigBeeStatus.flags.bits.bTryOrphanJoin && nwkStatus.flags.bits.bRejoinScan)
                            {

                                nwkStatus.flags.bits.bRejoinScan  = 0;
                                params.MLME_SCAN_request.ScanType = MAC_SCAN_ACTIVE_SCAN;

                            }
                            else if(ZigBeeStatus.flags.bits.bTryOrphanJoin && (params.NLME_JOIN_request.RejoinNetwork == TRUE) )
                            {
                                params.MLME_SCAN_request.ScanType = MAC_SCAN_ORPHAN_SCAN;
                            }
                            else
                            {
                                params.MLME_SCAN_request.ScanType = MAC_SCAN_ACTIVE_SCAN;
                            }
                            /* Zigbee 2006:  Be sure that these parameters are copied over - dont assume alignment here*/
                            params.MLME_SCAN_request.ScanChannels = params.NLME_JOIN_request.ScanChannels;
                            params.MLME_SCAN_request.ScanDuration = 8; //params.NLME_JOIN_request.ScanDuration;

                            // Set the PAN ID to the PAN that we are trying to join.  If this fails, we'll have to clear it.
                            //macPIB.macPANId.Val = params.NLME_JOIN_request.PANId.Val;
                            for (i = 0; i < 8; i++)
                            {
                                currentNeighborTableInfo.nwkExtendedPANID.v[i] = \
                                    params.NLME_JOIN_request.ExtendedPANID.v[i];
                            }
                            PutNeighborTableInfo();
                            MLME_SET_macPANId_hw();
                            return MLME_SCAN_request;
                        }
                    }
                }//#endif  // I_AM_COORDINATOR
                break;

            // ---------------------------------------------------------------------
            case NLME_DIRECT_JOIN_request:
                #ifdef I_AM_END_DEVICE
                    params.NLME_DIRECT_JOIN_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_DIRECT_JOIN_confirm;
                #else
                    if (NWKLookupNodeByLongAddr( &(params.NLME_DIRECT_JOIN_request.DeviceAddress) ) != INVALID_NEIGHBOR_KEY)
                    {
                        // The node is already in our neighbor table.
                        params.NLME_DIRECT_JOIN_confirm.Status = NWK_ALREADY_PRESENT;
                        return NLME_DIRECT_JOIN_confirm;
                    }
                    if (!CanAddChildNode() || ((i = CanAddNeighborNode()) == INVALID_NEIGHBOR_KEY))
                    {
                        // We do not have room to add this node to the neighbor table.
                        params.NLME_DIRECT_JOIN_confirm.Status = NWK_TABLE_FULL;
                        return NLME_DIRECT_JOIN_confirm;
                    }
                    else
                    {
                        // Add the node to our network
                        #ifdef I_SUPPORT_SECURITY
                            AddChildNode(TRUE);
                        #else
                            AddChildNode();
                        #endif
                        params.NLME_DIRECT_JOIN_confirm.Status = NWK_SUCCESS;
                        return NLME_DIRECT_JOIN_confirm;
                    }
                #endif
                break;

            // ---------------------------------------------------------------------
            case NLME_LEAVE_request:
                // If we are not associated with a network, then we cannot leave it.
                    if(((NOW_I_AM_A_CORDINATOR())//#ifdef I_AM_COORDINATOR
                                && ( !ZigBeeStatus.flags.bits.bNetworkFormed ))
                            ||//#else
                                ((NOW_I_AM_NOT_A_CORDINATOR())&& ( !ZigBeeStatus.flags.bits.bNetworkJoined ))
                            )//#endif
                {
                    params.NLME_LEAVE_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_LEAVE_confirm;
                }
                if (nwkStatus.flags.bits.bLeaveInProgress)
                {
                    /*Currently we are processing one leave request. So return some error
                    message to the application so that it can be reissued by application
                    at later point. Status message is not defined by the specificaiton.
                    So, we have to specify proprietary status code. Currently we specify
                    it as NWK_INVALID_PARAMETER. Later it can be changed, if required.*/
                    params.NLME_LEAVE_confirm.Status = NWK_INVALID_PARAMETER;
                    return NLME_LEAVE_confirm;
                }

                // See if we are being told to leave the network by the upper layer.
                if( params.NLME_LEAVE_request.DeviceAddress.v[0] == 0x00 &&
                    params.NLME_LEAVE_request.DeviceAddress.v[1] == 0x00 &&
                    params.NLME_LEAVE_request.DeviceAddress.v[2] == 0x00 &&
                    params.NLME_LEAVE_request.DeviceAddress.v[3] == 0x00 &&
                    params.NLME_LEAVE_request.DeviceAddress.v[4] == 0x00 &&
                    params.NLME_LEAVE_request.DeviceAddress.v[5] == 0x00 &&
                    params.NLME_LEAVE_request.DeviceAddress.v[6] == 0x00 &&
                    params.NLME_LEAVE_request.DeviceAddress.v[7] == 0x00 )
                {
                    // My upper layers have told me to leave the network.
                    // Do not allow anyone else to join.
                    macPIB.macAssociationPermit = FALSE;
                    nwkStatus.flags.bits.bLeaveInProgress = 1;
                    #ifndef I_AM_END_DEVICE
                        nwkStatus.flags.bits.bRemoveChildren = params.NLME_LEAVE_request.RemoveChildren;
                    #else
                        /*For an End Device Remove Children should always be false*/
                        nwkStatus.flags.bits.bRemoveChildren = 0x00;
                    #endif
                    /*These are used to send Leave Confirmation/Indication to upper layers*/
                    nwkStatus.flags.bits.bSelfLeave = 1;
                    nwkStatus.flags.bits.bRejoin = params.NLME_LEAVE_request.Rejoin;
                    nwkStatus.flags.bits.bRejoinScan = params.NLME_LEAVE_request.Rejoin;
                    nwkStatus.leaveReason = SELF_INITIATED_LEAVE;
                    return NO_PRIMITIVE;
                }

                #ifdef I_AM_END_DEVICE
                    params.NLME_LEAVE_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_LEAVE_confirm;
                #else

                    if (((i = NWKLookupNodeByLongAddr( &(params.NLME_LEAVE_request.DeviceAddress))) != INVALID_NEIGHBOR_KEY) &&
                        (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD))
                    {
                        if( params.NLME_LEAVE_request.Silent )
                        {
                            #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
                                if (currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_ROUTER)
                                {
                                    currentNeighborTableInfo.nextRouterAddr.Val = currentNeighborRecord.shortAddr.Val;
                                }
                                if (currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE)
                                {
                                    currentNeighborTableInfo.nextEndDeviceAddr.Val = currentNeighborRecord.shortAddr.Val;
                                }
                            #else
                                currentNeighborTableInfo.nextChildAddr.Val = currentNeighborRecord.shortAddr.Val;
                            #endif
                            PutNeighborTableInfo();
                            //RemoveNeighborTableEntry();
                            params.NLME_LEAVE_confirm.Status = NWK_SUCCESS;
                            return NLME_LEAVE_confirm;
                        }
                        /*Update the variables so that leave command frame can be transmitted in the background*/
                        nwkStatus.flags.bits.bLeaveInProgress = 1;
                        nwkStatus.flags.bits.bRemoveChildren = params.NLME_LEAVE_request.RemoveChildren;
                        nwkStatus.flags.bits.bRejoin = params.NLME_LEAVE_request.Rejoin;
                        nwkStatus.leaveDeviceAddress.Val = currentNeighborRecord.shortAddr.Val;
                        nwkStatus.flags.bits.bSelfLeave = 0;
                        nwkStatus.leaveReason = REQUEST_CHILD_TO_LEAVE;
                        return NO_PRIMITIVE;
                    }
                    else
                    {
                        params.NLME_LEAVE_confirm.Status = NWK_UNKNOWN_DEVICE;
                        return NLME_LEAVE_confirm;
                    }
                #endif
                break;

            // ---------------------------------------------------------------------
            case NLME_RESET_request:
                params.MLME_RESET_request.SetDefaultPIB = TRUE;
                return MLME_RESET_request;
                break;

            // ---------------------------------------------------------------------
            case NLME_SYNC_request:
                // only support non-beacon networks, so some of the functionality
                // of this primitive has been omitted.
                if (!params.NLME_SYNC_request.Track)
                {
                    {
                        params.MLME_POLL_request.SecurityEnabled = FALSE;
                    }

                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        ZigBeeStatus.flags.bits.bRequestingData = 1;
                        ZigBeeStatus.flags.bits.bDataRequestComplete = 0;
                    }//#endif


                    params.MLME_POLL_request.CoordPANId             = macPIB.macPANId;
                #ifdef ZCP_DEBUG
                    if( macPIB.macCoordShortAddress.Val == 0xfffe )
                    {
                        params.MLME_POLL_request.CoordAddrMode = 0x03;
                        params.MLME_POLL_request.CoordAddress.LongAddr = macPIB.macCoordExtendedAddress;
                    } else
                #endif
                    {
                        params.MLME_POLL_request.CoordAddrMode          = 0x02;
                        params.MLME_POLL_request.CoordAddress.ShortAddr = macPIB.macCoordShortAddress;
                    }
                    return MLME_POLL_request;
                }
                else
                {
                    if (NOW_I_AM_A_RFD()){//#ifdef I_AM_RFD
                        ZigBeeStatus.flags.bits.bRequestingData = 0;
                        ZigBeeStatus.flags.bits.bDataRequestComplete = 1;
                    }//#endif

                    params.NLME_SYNC_confirm.Status = NWK_INVALID_PARAMETER;
                    return NLME_SYNC_confirm;
                }
                break;


            /* // A ZigBee 2006 requierement:: Must now support the
             * NLME_ROUTE_DISCOVERY_request & confirm primitives
            */
            case NLME_ROUTE_DISCOVERY_request:
            /* RFDs recieving this will just return */
                #ifdef I_AM_END_DEVICE
                    params.NLME_ROUTE_DISCOVERY_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_ROUTE_DISCOVERY_confirm;
                #endif

#if defined( I_SUPPORT_ROUTING)
                // If we are not associated with a network, then we can't route anything
                    if(((NOW_I_AM_A_CORDINATOR())//#ifdef I_AM_COORDINATOR
                                && ( !ZigBeeStatus.flags.bits.bNetworkFormed ))
                            ||//#else
                                ((NOW_I_AM_NOT_A_CORDINATOR())&& ( !ZigBeeStatus.flags.bits.bNetworkJoined ))
                            )//#endif
                {
                    params.NLME_ROUTE_DISCOVERY_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_ROUTE_DISCOVERY_confirm;
                }

                /* A ZigBee 2006 requierement:: Cannot use broadcast address on route requests unless DstAddrMode == 0x00 */
                if( (params.NLME_ROUTE_DISCOVERY_request.DstAddrMode != APS_ADDRESS_NOT_PRESENT )
                    &&  ( (params.NLME_ROUTE_DISCOVERY_request.DstAddr.Val == 0xffff) ||
                          (params.NLME_ROUTE_DISCOVERY_request.DstAddr.Val == 0xfffd) ||
                          (params.NLME_ROUTE_DISCOVERY_request.DstAddr.Val == 0xfffc) ) )
                {
                    params.NLME_ROUTE_DISCOVERY_confirm.Status = NWK_INVALID_REQUEST;
                    return NLME_ROUTE_DISCOVERY_confirm;
                }
                if( params.NLME_ROUTE_DISCOVERY_request.DstAddrMode == APS_ADDRESS_NOT_PRESENT )
                {
                    params.NLME_ROUTE_DISCOVERY_request.DstAddr.Val = 0xFFFC;
                }

                if( params.NLME_ROUTE_DISCOVERY_request.DstAddrMode == 0x01)
                {
                    #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x00)
                        params.NLME_ROUTE_DISCOVERY_confirm.Status = NWK_INVALID_REQUEST;
                        return NLME_ROUTE_DISCOVERY_confirm;
                    #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x00)
                }
                {
                    BYTE        rdIndex, rtIndex;
                    SHORT_ADDR  dummyAddr;
                    dummyAddr.Val = 0xffff;
                    /* Create a routing table entry so that we can keep track of the request */
					
					printf("Creat Route Table 2\n\r");
                    if (
                        #if (I_SUPPORT_SYMMETRIC_LINK == 0x01)
                            !CreateRoutingTableEntries
                            (
                                params.NLME_ROUTE_DISCOVERY_request.DstAddr,
                                &rdIndex,
                                &rtIndex,
                                dummyAddr, // dummy because we are the initiator
                                dummyAddr, // dummy because we are the initiator
                                FALSE
                            )
                        #else
							
							
                            !CreateRoutingTableEntries
                            (
                                params.NLME_ROUTE_DISCOVERY_request.DstAddr,
                                &rdIndex,
                                &rtIndex
                            )
                        #endif
                       )
                    {
                        params.NLME_ROUTE_DISCOVERY_confirm.Status =  ROUTE_DISCOVERY_FAILED;
                        return NLME_ROUTE_DISCOVERY_confirm;
                    }

                    if (params.NLME_ROUTE_DISCOVERY_request.DstAddrMode == 0x01) // Multicast Address Mode
                    {
                        if ( FindFreeRoutingTableEntry(params.NLME_ROUTE_DISCOVERY_request.DstAddr, FALSE) != 0xFF )
                        {
                            if (currentRoutingEntry.destAddress.Val == params.NLME_ROUTE_DISCOVERY_request.DstAddr.Val)
                            {
                                // Update the bGroupIDFlag as the route is being discovered for the group.
                                currentRoutingEntry.rtflags.rtbits.bGroupIDFlag = 1;
                                PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
                            }
                        }
                    }
                    // Populate the remainder of the Route Discovery Table
                    routeDiscoveryTablePointer[rdIndex]->routeRequestID             = nwkStatus.routeRequestID++;
                    routeDiscoveryTablePointer[rdIndex]->srcAddress                 = macPIB.macShortAddress;
                    routeDiscoveryTablePointer[rdIndex]->senderAddress              = macPIB.macShortAddress;
                    routeDiscoveryTablePointer[rdIndex]->forwardCost                = 0;
                    routeDiscoveryTablePointer[rdIndex]->status.transmitCounter     = nwkcInitialRREQRetries + 1;
                    routeDiscoveryTablePointer[rdIndex]->status.initialRebroadcast  = 0;
                    routeDiscoveryTablePointer[rdIndex]->previousCost = 0xff;

                    /*Mark whether the device is an intiator or intermediate.
                    This information is used to retain the status of Routing table entry as ACTIVE.*/
                    routeDiscoveryTablePointer[rdIndex]->status.initiator = 0x01; //We are the intiator

                    dummyAddr.Val = 0xFFFC;

                    if (!routeDiscoveryTablePointer[rdIndex]->forwardRREQ)
                    {
                        /*sizeof_ROUTE_REQUEST_COMMAND + sizeof_ROUTE_REQUEST_COMMAND_HEADER = 6 + 16 = 22Bytes*/
                        routeDiscoveryTablePointer[rdIndex]->forwardRREQ = SRAMalloc(sizeof_ROUTE_REQUEST_COMMAND +
                                                                                  #if ( ZIGBEE_PRO == 0x01)
                                                                                     sizeof_RREQ_COMMAND_DST_IEEE_ADDR +
                                                                                  #endif
                                                                                     sizeof_ROUTE_REQUEST_COMMAND_HEADER);
                    }
                    if (routeDiscoveryTablePointer[rdIndex]->forwardRREQ != NULL)
                    {
                        BYTE *ptr2;
                        LONG_ADDR tmpLongAddr;
                        BYTE commandOptions;

                        // Load the RREQ information.
                        ptr2 = routeDiscoveryTablePointer[rdIndex]->forwardRREQ;
                        *ptr2++ = NWK_COMMAND_ROUTE_REQUEST;
                        #if ( ZIGBEE_PRO == 0x01)
                            if (params.NLME_ROUTE_DISCOVERY_request.DstAddrMode == 0x01)
                            {
                                commandOptions = DEFAULT_COMMAND_OPTIONS | MULTICAST_BIT;
                            }
                            else if (params.NLME_ROUTE_DISCOVERY_request.DstAddrMode == 0x02)
                            {
                                if ( IsThisAddressKnown
                                     (
                                       params.NLME_ROUTE_DISCOVERY_request.DstAddr,
                                       (BYTE *)&tmpLongAddr
                                     )
                                   )
                                {
                                    commandOptions = DEFAULT_COMMAND_OPTIONS | DEST_IEEE_ADDRESS_BIT;
                                }
                                else
                                {
                                   commandOptions = DEFAULT_COMMAND_OPTIONS;
                                }
                            }
                            else
                        #endif
                            {
                               commandOptions = DEFAULT_COMMAND_OPTIONS;
                            }


                     #if I_SUPPORT_CONCENTRATOR == 1
                        if( params.NLME_ROUTE_DISCOVERY_request.DstAddrMode == APS_ADDRESS_NOT_PRESENT )
                        {
                            if( params.NLME_ROUTE_DISCOVERY_request.NoRouteCache == TRUE )
                            {
                                *ptr2++ = LOW_CONC_MANY_TO_ONE | commandOptions;
                            }
                            else
                            {
                                *ptr2++ = HIGH_CONC_MANY_TO_ONE | commandOptions;
                            }
                        }
                        else
                        #endif                        /*  I_SUPPORT_CONCENTRATOR == 1 */
                        {
                            *ptr2++ = commandOptions;
                        }


                        *ptr2++ = routeDiscoveryTablePointer[rdIndex]->routeRequestID;
                        *ptr2++ = params.NLME_ROUTE_DISCOVERY_request.DstAddr.byte.LSB;
                        *ptr2++ = params.NLME_ROUTE_DISCOVERY_request.DstAddr.byte.MSB;
                        *ptr2++ = 0x00;

                        /* Load network header in reverse order */
                        #if ( ZIGBEE_PRO == 0x01)
                            if ( commandOptions & DEST_IEEE_ADDRESS_BIT )
                            {
                                for(i = 0; i < 8; i++)
                                {
                                    *ptr2++ = tmpLongAddr.v[7-i];
                                }
                            }
                        #endif
                        // Load up the old NWK header (backwards).
                        /*Add the SrcIEEEAddress field in the NWK Header*/
                        GetMACAddress(&tmpLongAddr);
                        for(i = 0; i < 8; i++)
                        {
                            *ptr2++ = tmpLongAddr.v[7-i];
                        }

                        *ptr2++ = NLME_GET_nwkBCSN();
                        *ptr2++ = params.NLME_ROUTE_DISCOVERY_request.Radius;
                        *ptr2++ = macPIB.macShortAddress.byte.MSB;
                        *ptr2++ = macPIB.macShortAddress.byte.LSB;
                        *ptr2++ = dummyAddr.byte.MSB;
                        *ptr2++ = dummyAddr.byte.LSB;
                        /*Update the Frame Control field which is of two bytes.
                        First update the MSB part as SrcIEEEAddress field needs to be set.
                        Then update the LSB part. */
                        #ifdef I_SUPPORT_SECURITY
                            *ptr2++ = 0x02 | NWK_IEEE_SRC_ADDR;
                        #else
                            *ptr2++ = 0x00 | NWK_IEEE_SRC_ADDR;
                        #endif
                        /*LSB part of Frame Control field in the Network Header*/
                        *ptr2++ = NWK_FRAME_CMD | (nwkProtocolVersion<<2);

                        // Reset the rebroadcast timer and the transmit counter
                        routeDiscoveryTablePointer[rdIndex]->rebroadcastTimer       = TickGet();
                        //routeDiscoveryTablePointer[rdIndex]->status.transmitCounter = nwkcInitialRREQRetries + 1;
                                  #if I_SUPPORT_CONCENTRATOR == 1
                        if( params.NLME_ROUTE_DISCOVERY_request.DstAddrMode == APS_ADDRESS_NOT_PRESENT )
                        {
                            params.NLME_ROUTE_DISCOVERY_confirm.Status =  SUCCESS;
                            return NLME_ROUTE_DISCOVERY_confirm;
                        }

                 #endif           /* I_SUPPORT_CONCENTRATOR == 1 */

                    }
                    else
                    {
                        params.NLME_ROUTE_DISCOVERY_confirm.Status =  NWK_STATUS_NO_ROUTING_CAPACITY;
                        return NLME_ROUTE_DISCOVERY_confirm;
                    }
                    return NO_PRIMITIVE;
                }
                break;

            // ---------------------------------------------------------------------
#endif
            default:
                break;
        }
    }
    return NO_PRIMITIVE;
}


/*********************************************************************
 * Function:        void AddChildNode( void )
 *
 * PreCondition:    The node must not be already in the neighbor table,
 *                  and pCurrentNeighborRecord must point to a free location.
 *                  This routine can be called from either
 *                  MLME_ASSOCIATE_indication or NLME_DIRECT_JOIN_request.
 *
 * Input:           index       - handle to a free location
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Creates new child entry for given node at the input
 *                  location.
 *
 * Note:            MLME_ASSOCIATE_indication.CapabilityInformation must
 *                  overlay NLME_DIRECT_JOIN_request.CapabilityInformation,
 *                  and MLME_ASSOCIATE_indication.DeviceAddress must overlay
 *                  NLME_DIRECT_JOIN_request.DeviceAddress, since this
 *                  routine can be called from either function.
 ********************************************************************/
#ifndef I_AM_END_DEVICE
#ifdef I_SUPPORT_SECURITY
void AddChildNode(BOOL bSecured)
#else
void AddChildNode( void  )
#endif
{

    memcpy( (void*)&currentNeighborRecord.longAddr, (void*)&(params.MLME_ASSOCIATE_indication.DeviceAddress), (BYTE)(sizeof(LONG_ADDR)));
    currentNeighborRecord.panID = macPIB.macPANId;
    currentNeighborRecord.LogicalChannel = 0xFF;
    currentNeighborRecord.bSecured = FALSE;
    /*#ifdef I_SUPPORT_SECURITY
        #ifdef I_SUPPORT_SECURITY_SPEC
            currentNeighborRecord.bSecured = bSecured;
        #else
            #ifdef PRECONFIG_NWK_KEY
                currentNeighborRecord.bSecured = TRUE;
            #else
                if(bSecured)
                    currentNeighborRecord.bSecured = TRUE;
                else
                    currentNeighborRecord.bSecured = FALSE;
            #endif
        #endif
    #endif*/
#if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
    if (params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.DeviceType == DEVICE_IEEE_RFD)
    {
        currentNeighborRecord.deviceInfo.bits.deviceType = DEVICE_ZIGBEE_END_DEVICE;
        if (doingRejoinCommand)
        {
            currentNeighborRecord.shortAddr.v[0] = rejoinShortAddress.byte.LSB;
            currentNeighborRecord.shortAddr.v[1] = rejoinShortAddress.byte.MSB;
        }
        else
        {

        #ifdef ZCP_DEBUG
            if( bDisableShortAddress )
                currentNeighborRecord.shortAddr.Val = 0xfffe;
            else

        #endif
            {
                    currentNeighborRecord.shortAddr = currentNeighborTableInfo.nextEndDeviceAddr;
                    currentNeighborTableInfo.nextEndDeviceAddr.Val ++;
                    if (currentNeighborTableInfo.nextEndDeviceAddr.Val == LAST_END_DEVICE_ADDRESS)
                    {
                        nwkStatus.flags.bits.bAllEndDeviceAddressesUsed = 1;
                    }
            }
        }

    }
    else
    {
        currentNeighborRecord.deviceInfo.bits.deviceType = DEVICE_ZIGBEE_ROUTER;
        if (doingRejoinCommand)
        {
            currentNeighborRecord.shortAddr.v[0] = rejoinShortAddress.byte.LSB;
            currentNeighborRecord.shortAddr.v[1] = rejoinShortAddress.byte.MSB;
        }
        else
        {
        #ifdef ZCP_DEBUG
            if( bDisableShortAddress )
                currentNeighborRecord.shortAddr.Val = 0xfffe;
            else
        #endif
            currentNeighborRecord.shortAddr = currentNeighborTableInfo.nextRouterAddr;
            currentNeighborTableInfo.numChildRouters++;
            currentNeighborTableInfo.nextRouterAddr.Val += currentNeighborTableInfo.cSkip.Val;
            if (currentNeighborTableInfo.nextRouterAddr.Val > macPIB.macShortAddress.Val + currentNeighborTableInfo.cSkip.Val * NIB_nwkMaxRouters)
            {
                nwkStatus.flags.bits.bAllRouterAddressesUsed = 1;
            }
        }
    }
#else
    if (doingRejoinCommand)
    {
        currentNeighborRecord.shortAddr.v[0] = rejoinShortAddress.byte.LSB;
        currentNeighborRecord.shortAddr.v[1] = rejoinShortAddress.byte.MSB;
    }
    else
    {
        currentNeighborRecord.shortAddr = currentNeighborTableInfo.nextChildAddr;
        nwkStatus.flags.bits.bNextChildAddressUsed = TRUE;
    }

    currentNeighborRecord.deviceInfo.bits.deviceType = DEVICE_ZIGBEE_ROUTER;
    if ( params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.DeviceType == DEVICE_IEEE_RFD )
    {
        currentNeighborRecord.deviceInfo.bits.deviceType = DEVICE_ZIGBEE_END_DEVICE;
    }

/*
    currentNeighborRecord.deviceInfo.bits.deviceType = \
                                    ( params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.DeviceType == DEVICE_IEEE_RFD ) ? \
                                        DEVICE_ZIGBEE_END_DEVICE : DEVICE_ZIGBEE_ROUTER ;*/
/*    if ( params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.DeviceType == DEVICE_IEEE_RFD )
    {
        currentNeighborRecord.deviceInfo.bits.deviceType = DEVICE_ZIGBEE_END_DEVICE;
    }
    else
    {
        currentNeighborRecord.deviceInfo.bits.deviceType = DEVICE_ZIGBEE_ROUTER;
    }*/
#endif
    /* update the parent's NBTable with each child's information  */
    currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle  = params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.RxOnWhenIdle;
    currentNeighborRecord.deviceInfo.bits.Relationship  = NEIGHBOR_IS_CHILD;
    currentNeighborRecord.deviceInfo.bits.bInUse        = TRUE;
    
    /* include these additional parametert to support mgmt lqi requests  */
    currentNeighborRecord.deviceInfo.bits.LQI   = params.MLME_ASSOCIATE_indication.mpduLinkQuality;
    currentNeighborRecord.deviceInfo.bits.Depth = currentNeighborTableInfo.depth + 1;
    currentNeighborRecord.ExtendedPANID         = currentNeighborTableInfo.nwkExtendedPANID;
    
    #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
        currentNeighborRecord.linkStatusInfo.flags.bits.bIncomingCost =
                    CalculateLinkQuality( params.MLME_ASSOCIATE_indication.mpduLinkQuality );
        currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost = 0;
        currentNeighborRecord.linkStatusInfo.Age = 0;
    #endif // ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

    PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord);

    currentNeighborTableInfo.numChildren++;
    currentNeighborTableInfo.neighborTableSize++;
    PutNeighborTableInfo();

    SetBeaconPayload(TRUE);
}
#endif

/*********************************************************************
 * Function:        BOOL CanAddChildNode ( void )
 *
 * PreCondition:    This routine must be called from either the
 *                  MLME_ASSOCIATE_indication or NLME_DIRECT_JOIN_request
 *                  primitive.
 *
 * Input:           None
 *
 * Output:          If we have the capacity to add the node as our child.
 *
 * Side Effects:    None
 *
 * Overview:        This routine determines if there is capacity to add
 *                  a child node of the given type.  First, macAssociationPermit
 *                  must be TRUE.  Then, we must not have exceeded our
 *                  maximum number of children.  If the device is an
 *                  RFD, we must not have run out of RFD addresses (since
 *                  we cannot reuse them).  If the device is an FFD, then
 *                  there must be depth available for the FFD to add its
 *                  own children, we must not have exceeded our maximum
 *                  number of routers, and we must not have run out of
 *                  FFD addresses.
 *
 * Note:            MLME_ASSOCIATE_indication.CapabilityInformation must
 *                  overlay NLME_DIRECT_JOIN_request.CapabilityInformation,
 *                  and MLME_ASSOCIATE_indication.DeviceAddress must overlay
 *                  NLME_DIRECT_JOIN_request.DeviceAddress, since this
 *                  routine can be called from either function.
 ********************************************************************/
#ifndef I_AM_END_DEVICE
BOOL CanAddChildNode( void )
{
    /* A ZigBee 2006 requierement:Routers/FFD are allowed to join at MaxDepth
     * see mandatory test 3_14 for the expected behavior for 2006
    */
    if(!doingRejoinCommand)  /* Ignore JoinPermit Flag if doing rejoin Process */
    {
        return (macPIB.macAssociationPermit &&
            (currentNeighborTableInfo.numChildren < NIB_nwkMaxChildren)
             #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
             &&
              (((params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.DeviceType == DEVICE_IEEE_RFD) &&
              (!nwkStatus.flags.bits.bAllEndDeviceAddressesUsed)) ||

              ((currentNeighborTableInfo.depth < NIB_nwkMaxDepth)

                &&
                (currentNeighborTableInfo.numChildRouters < NIB_nwkMaxRouters) &&
                (!nwkStatus.flags.bits.bAllRouterAddressesUsed)
               ))
               #endif
            );
     }
     else
     {
        return (
            (currentNeighborTableInfo.numChildren < NIB_nwkMaxChildren)
             #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
             &&
             (((params.MLME_ASSOCIATE_indication.CapabilityInformation.CapBits.DeviceType == DEVICE_IEEE_RFD) &&
             (!nwkStatus.flags.bits.bAllEndDeviceAddressesUsed)) ||

             ((currentNeighborTableInfo.depth < NIB_nwkMaxDepth)

                &&
              (currentNeighborTableInfo.numChildRouters < NIB_nwkMaxRouters) &&
              (!nwkStatus.flags.bits.bAllRouterAddressesUsed)
              ))
              #endif
            );
     }
}
#endif

/*********************************************************************
 * Function:        NEIGHBOR_KEY CanAddNeighborNode( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Handle to available space
 *
 * Side Effects:    None
 *
 * Overview:        This routine determines if there is a free space
 *                  in the neighbor table and returns a handle to it.
 *
 * Note:            If the function is successful, pCurrentNeighborRecord
 *                  points to the available location.
 ********************************************************************/
NEIGHBOR_KEY CanAddNeighborNode( void )
{
    BYTE    index;

    // Get ROM address of neighborTable in RAM.
    pCurrentNeighborRecord = neighborTable;

    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_NEIGHBORS) ; pCurrentNeighborRecord+=(WORD)sizeof(NEIGHBOR_RECORD), index++)
    #else
    for (index = 0; (index < MAX_NEIGHBORS) ; pCurrentNeighborRecord++, index++)
    #endif
    {
        // Read the record into RAM.
        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

        // Now check to see if it is in use or not.
        if ( !currentNeighborRecord.deviceInfo.bits.bInUse )
        {

            return index;
        }
    }



     return INVALID_NEIGHBOR_KEY;

}

/*********************************************************************
 * Function:        BOOL CreateNewBTR( BYTE *BTTIndex )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - new BTR created at indext BTT
 *                  FALSE - could not create a new BTR
 *
 * Side Effects:    None
 *
 * Overview:        This function tries to create a new BTR.  If if
 *                  cannot because there is no room in the BTT or it
 *                  cannot allocate enough memory, it returns FALSE.
 *                  Otherwise, it returns TRUE, and BTTIndex indicates
 *                  the position of the new BTR.  The flags of the BTR
 *                  are initialized and currentNeighbor is set to 0,
 *                  but no other data is populated.
 *
 * Note:            bMessageFromUpperLayers is set to TRUE.
 ********************************************************************/
//#ifndef I_AM_RFD
BOOL CreateNewBTR( BYTE *BTTIndex )
{
    BYTE    i;
    BYTE    j;

    // Find an empty place in the BTT.
    for (i=0; (i<NUM_BUFFERED_BROADCAST_MESSAGES) && (nwkStatus.BTT[i]!=NULL); i++) {}
    if (i==NUM_BUFFERED_BROADCAST_MESSAGES)
    {
        return FALSE;
    }

    // Try to allocate space for a new BTR.
    if ((nwkStatus.BTT[i] = (BTR *)SRAMalloc(sizeof(BTR))) == NULL)
    {
        return FALSE;
    }

    // Set the BTR index to the new entry.
    *BTTIndex = i;

    // Initialize the BTR the best we can.  Set the broadcast time so it
    // won't time out before send it.
    nwkStatus.BTT[i]->currentNeighbor = 0xFF;
    nwkStatus.BTT[i]->broadcastTime = nwkStatus.BTT[i]->broadcastJitterTimer = TickGet();
    nwkStatus.BTT[i]->btrInfo.bConfirmSent = 0;

    #ifdef NIB_STATIC_IMPLEMENTATION
    nwkStatus.BTT[i]->btrInfo.nRetries         = NIB_nwkMaxBroadcastRetries;
    #else
    nwkStatus.BTT[i]->btrInfo.nRetries         = NIB.nwkMaxBroadcastRetries;
    #endif
    for (j=0; j<MAX_NEIGHBORS; j++)
    {
        nwkStatus.BTT[i]->flags[j].bits.bMessageNotRelayed = 0x01;
    }
    #ifdef I_SUPPORT_SECURITY
    nwkStatus.BTT[i]->btrInfo.bAlreadySecured = 0x00;
    #endif
    return TRUE;
}
//#endif

/*********************************************************************
 * Function:        void CreateRouteReply( SHORT_ADDR originatorAddress,
 *                      BYTE rdIndex, ROUTE_REQUEST_COMMAND *rreq )
 *
 * PreCondition:    Must be called from MCPS_DATA_indication
 *
 * Input:           nwkSourceAddress - NWK source address of message
 *                  rdIndex - index into Route Discovery Table
 *                  rreq - pointer to route request command packet
 *
 * Side Effects:    None
 *
 * Overview:        Sends the Route Reply command frame to the sender
 *                  of the previous message.
 *
 * Note:            None
 ********************************************************************/
#ifdef I_SUPPORT_ROUTING
void CreateRouteReply( SHORT_ADDR nwkSourceAddress, BYTE rdIndex, ROUTE_REQUEST_COMMAND *rreq )
{
    BYTE                temp;
    #if ( ZIGBEE_PRO == 0x01)
        LONG_ADDR           tempLongAddr;
        LONG_ADDR           originatorIEEEAddr;
        LONG_ADDR           responderIEEEAddr;
        BYTE                i;
    #endif
    BYTE                commandOptions = DEFAULT_COMMAND_OPTIONS;

/*#ifndef USE_TREE_ROUTING_ONLY
    if (rdIndex != INVALID_ROUTE_DISCOVERY_INDEX)
    {
        // Mark the routing table entry as valid for use.
        #ifdef USE_EXTERNAL_NVM
            pCurrentRoutingEntry = routingTable + (WORD)(routeDiscoveryTablePointer[rdIndex]->routingTableIndex) * (WORD)sizeof(ROUTING_ENTRY);
        #else
            pCurrentRoutingEntry = &(routingTable[routeDiscoveryTablePointer[rdIndex]->routingTableIndex]);
        #endif
        GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
        currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_ACTIVE;
        PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );

        // Do not free the route discovery table entry.  We might get a better
        // route within the allowed time.  We'll let the clean-up routine
        // free the route discovery table entry.
    }
#endif*/

    // Load up the NWK payload.
    TxBuffer[TxData++] = NWK_COMMAND_ROUTE_REPLY;           // rrep.commandFrameIdentifier
    #if ( ZIGBEE_PRO == 0x01)
        if ( IsThisAddressKnown( nwkSourceAddress, (BYTE *)&tempLongAddr ) )
        {
            commandOptions = commandOptions | ORIGINATOR_IEEE_ADDR_BIT;
            for (i=0; i<8; i++)
            {
                originatorIEEEAddr.v[i] = tempLongAddr.v[i];
            }
        }
        if (rreq->commandOptions & MULTICAST_BIT)
        {
            commandOptions = commandOptions | MULTICAST_BIT;
        }
        else if ( IsThisAddressKnown( rreq->destinationAddress, (BYTE *)&tempLongAddr ) )
        {
            commandOptions = commandOptions | RESPONDER_IEEE_ADDR_BIT;
            for (i=0; i<8; i++)
            {
                responderIEEEAddr.v[i] = tempLongAddr.v[i];
            }
        }
    #endif

    TxBuffer[TxData++] = commandOptions;                    // rrep.commandOptions
    TxBuffer[TxData++] = rreq->routeRequestIdentifier;      // rrep.routeRequestIdentifier
    TxBuffer[TxData++] = nwkSourceAddress.byte.LSB;         // rrep.originatorAddress
    TxBuffer[TxData++] = nwkSourceAddress.byte.MSB;         // rrep.originatorAddress
    TxBuffer[TxData++] = rreq->destinationAddress.byte.LSB; // rrep.responderAddress
    TxBuffer[TxData++] = rreq->destinationAddress.byte.MSB; // rrep.responderAddress
    TxBuffer[TxData++] = rreq->pathCost;                    // rrep.pathCost

    #if ( ZIGBEE_PRO == 0x01)
        if ( commandOptions & ORIGINATOR_IEEE_ADDR_BIT )
        {
            for (i=0; i<8; i++)
            {
                TxBuffer[TxData++] = originatorIEEEAddr.v[7-i];
            }
        }
        if ( commandOptions & RESPONDER_IEEE_ADDR_BIT )
        {
            for (i=0; i<8; i++)
            {
                TxBuffer[TxData++] = responderIEEEAddr.v[7-i];
            }
        }
    #endif

    /*Update the TxBuffer with the Network Header for Route Reply Command*/
    CreateNwkCommandHeader
    (
        NLME_GET_nwkBCSN(),
        DEFAULT_RADIUS,
        macPIB.macShortAddress,
        routeDiscoveryTablePointer[rdIndex]->senderAddress
    );

    // Load up the MCPS_DATA.request parameters
    // We'll just send it back to where it came from, since rdIndex may be invalid.
    Prepare_MCPS_DATA_request( params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val, &temp );
}
#endif

/*********************************************************************
 * Function:        BYTE FindFreeRoutingTableEntry( SHORT_ADDR dstAddr, BOOL isConcentrator )
 *
 * PreCondition:    None
 *
 * Input:           dstAddr = The address of device to which entry has to be created.
 *                  isConcentrator = TRUE if the dest device is a concentrator and FALSE
 *                  otherwise
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks whether the dstAddr is valid unicast
 *                  address. It will also check whether dstAddr is self address.
 *                  If both fails, then it searches for the existing entry in the
 *                  Neighbor Table. If no entry found then, it searches for the
 *                  already existing routing table entry. If no entry found, then
 *                  it tries to find free routing entry. If free entry found, then
 *                  it returns the index of the routing entry. Otherwise it returns 0xFF
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
BYTE FindFreeRoutingTableEntry( SHORT_ADDR dstAddr, BYTE isConcentrator )
{
    BYTE                temp = 0;

    // See if an entry already exists for the target address
    // in the routing table.  Otherwise, find a free location in the
    // Routing Table.  We know one or the other is possible since
    // HasRoutingCapacity was true.  We need to see if the destination
    // address is already in the table in case the route is being
    // re-discovered.  Since we have a local variable, use it instead
    // of the pointer.

    // Check whether the address is valid unicast address. If Valid,
    // then check whether it is self address. If not self address,
    // then check whether Neighbor table entry exists. If exists, then return 0xFF
    // These checking conditions are required, because, we dont want to create the
    // Routing Table entries for any of these addresses. We should not create the
    // Routing table entries for our neighbors.
    #if ( I_SUPPORT_SOURCE_ROUTE_TESTING == 0x01 )
        if (!SourceRouteTestingInProgress)
    #endif // ( I_SUPPORT_SOURCE_ROUTE_TESTING == 0x01 )
    {
        /* if the destination is a concentartor, create routing table entry irrespective of
        presence in neighbor table */
        if(!isConcentrator )
        {
            /*if ( ( dstAddr.Val > 0xFFF7 ) ||
                 ( dstAddr.Val == macPIB.macShortAddress.Val ) ||
                 ( ( NWKLookupNodeByShortAddrVal( dstAddr.Val ) != INVALID_NEIGHBOR_KEY ) &&
                   ( ( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ) ||
                     ( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT )||
                     ( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_SIBLING ) ) ) )
            {
                return 0xFF;
            }*/
        }
    }
    pCurrentRoutingEntry = routingTable;
    GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
    while ((temp < ROUTING_TABLE_SIZE) &&
           (currentRoutingEntry.destAddress.Val != dstAddr.Val))
    {
        temp++;
        #ifdef USE_EXTERNAL_NVM
            pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY);
        #else
            pCurrentRoutingEntry++;
        #endif
        GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
    }
    if (temp == ROUTING_TABLE_SIZE)
    {
        temp = 0;
        pCurrentRoutingEntry = routingTable;
        GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
        while ((temp < ROUTING_TABLE_SIZE) &&
               (currentRoutingEntry.rtflags.rtbits.bStatus != ROUTE_INACTIVE))
        {
            temp++;
            #ifdef USE_EXTERNAL_NVM
                pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY);
            #else
                pCurrentRoutingEntry++;
            #endif
            GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
        }
    }

    /*Check whether free Routing table entry exists. If not, then no need to
    create Route discovery table entry. Just return failure.*/
    if (temp == ROUTING_TABLE_SIZE)
    {
        /*No free Routing table entries were found*/
        return 0xFF;
    }
    return temp;
}
#endif

/*********************************************************************
 * Function:        BOOL CreateRoutingTableEntries( SHORT_ADDR targetAddr,
 *                      BYTE *rdIndex, BYTE *rtIndex ,SHORT_ADDR nextHopAddr,BYTE isConcentrator )
 *
 * PreCondition:    HasRoutingCapacity must be TRUE
 *
 * Input:           targetAddr: the device to whom routing entries have to be created
 *                  initiatorAddr =- The device that is the originator of route request
 *                  nextHopAddr: The next hop address for initiator
 *                  isConcentrator - whether originator is a concentator
 *
 * Output:          *rdIndex - Route Discovery Table index of new entry
 *                  *rtIndex - Routing Table index of new entry
 *                  TRUE if entries created successfully.
 *
 * Side Effects:    None
 *
 * Overview:        This routine creates Route Discovery Table and
 *                  Routing Table entries.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
#if (I_SUPPORT_SYMMETRIC_LINK == 0x01)
BOOL CreateRoutingTableEntries( SHORT_ADDR targetAddr, BYTE *rdIndex, BYTE *rtIndex,
                                SHORT_ADDR initiatorAddr, SHORT_ADDR nextHopAddr,BOOL isConcentrator )
#else
BOOL CreateRoutingTableEntries( SHORT_ADDR targetAddr, BYTE *rdIndex, BYTE *rtIndex )
#endif
{
    BYTE                temp = 0;
    BOOL                createRoutingTableEntry;
    #if( (I_SUPPORT_SYMMETRIC_LINK == 0x01) || ( I_SUPPORT_MANY_TO_ONE_HANDLING == 0x01 ))
        BOOL                CreateRouteForInitiator = FALSE;
    #endif

    // Find a free location in the Route Discovery Table.  We have to use a
    // temporary variable - the compiler cannot properly index off of *rdIndex.
    for (temp = 0;
         (temp < ROUTE_DISCOVERY_TABLE_SIZE) &&
         (routeDiscoveryTablePointer[temp] != NULL);
         temp++ ) {}

    //This if condition is required to check whether free Route Discovery Table
    //entry has been found or not. If not, then just return
    if (temp == ROUTE_DISCOVERY_TABLE_SIZE)
    {
        return FALSE;
    }
    *rdIndex = temp;

    if ((routeDiscoveryTablePointer[temp] = (ROUTE_DISCOVERY_ENTRY *)SRAMalloc( sizeof(ROUTE_DISCOVERY_ENTRY) )) == NULL)
    {
        return FALSE;
    }

    // Populate what we can in the Route Discovery Table entry.
    routeDiscoveryTablePointer[temp]->residualCost = 0xff;
    routeDiscoveryTablePointer[temp]->timeStamp = TickGet();
    routeDiscoveryTablePointer[temp]->routingTableIndex = 0xff;
    routeDiscoveryTablePointer[temp]->forwardRREQ = NULL;
    #if ( I_SUPPORT_MANY_TO_ONE_HANDLING == 0x01 )
    {
        if(isConcentrator)
        {
            CreateRouteForInitiator = TRUE;
        }
    }
    #endif
    #if (I_SUPPORT_SYMMETRIC_LINK == 0x01)
            CreateRouteForInitiator = TRUE;
    #endif
    #if( (I_SUPPORT_SYMMETRIC_LINK == 0x01) || ( I_SUPPORT_MANY_TO_ONE_HANDLING == 0x01 ))
        if(CreateRouteForInitiator )
        {
            if ( FindFreeRoutingTableEntry(initiatorAddr,isConcentrator ) != 0xFF )
            {
                // Populate the Routing Table entry for Backward Path.
                currentRoutingEntry.destAddress = initiatorAddr;
                currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_ACTIVE;
                currentRoutingEntry.nextHop = nextHopAddr;
                #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
                    currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                #endif
                currentRoutingEntry.rtflags.rtbits.bManyToOne = FALSE;
                currentRoutingEntry.rtflags.rtbits.bNoRouteCache = FALSE;
                currentRoutingEntry.rtflags.rtbits.bRouteRecordRequired = FALSE;

                PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
            }
        }
    #endif /* ( (I_SUPPORT_SYMMETRIC_LINK == 0x01) || ( I_SUPPORT_MANY_TO_ONE_HANDLING == 0x01 ))*/
    if ( ( createRoutingTableEntry = FindFreeRoutingTableEntry(targetAddr, FALSE) ) != 0xFF )
    {
        *rtIndex = createRoutingTableEntry;

        // Populate the Routing Table entry.
        currentRoutingEntry.destAddress = targetAddr;

        /*If we already have an ACTIVE ROUTE to the destination, then mark it.
        This information will be used to retain the status of the Routing table
        entry. This is also used to know whether we are reusing the Routing table
        entry or fresh one is created.*/
        if (currentRoutingEntry.rtflags.rtbits.bStatus == ROUTE_ACTIVE)
        {
            routeDiscoveryTablePointer[temp]->status.previousStateACTIVE = 0x01;
        }

        currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_DISCOVERY_UNDERWAY;
        #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
            currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
        #endif
        PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
        routeDiscoveryTablePointer[*rdIndex]->routingTableIndex = *rtIndex;
    }
    else
    {
        *rtIndex = 0xFF;
    }

    // We just made a Route Discovery Table and Routing Table entries, so set the status flag
    // so we can watch it for time out.
    nwkStatus.flags.bits.bAwaitingRouteDiscovery = 1;

    return TRUE;
}
#endif


/*********************************************************************
 * Function:        WORD GetCSkipVal( BYTE depth )
 *
 * PreCondition:    None
 *
 * Input:           Depth of a node in the tree
 *
 * Output:          Cskip value for a router at the given depth
 *
 * Side Effects:    None
 *
 * Overview:        This routine determines the Cskip value for
 *                  address allocation at a given tree depth.  Since
 *                  this calculation is pretty nasty, the values are
 *                  placed in a header file as constants rather than
 *                  determining them at run time.
 *
 * Note:            If allowed tree depths are expanded, this routine
 *                  will have to be expanded as well.
 ********************************************************************/
#ifdef I_SUPPORT_ROUTING
#if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01)
WORD GetCSkipVal( BYTE depth )
{
    WORD    cSkip = 0;

    switch (depth)
    {
            case 0:
                cSkip = CSKIP_DEPTH_0;
                break;
        #ifdef CSKIP_DEPTH_1
            case 1:
                cSkip = CSKIP_DEPTH_1;
                break;
        #endif
        #ifdef CSKIP_DEPTH_2
            case 2:
                cSkip = CSKIP_DEPTH_2;
                break;
        #endif
        #ifdef CSKIP_DEPTH_3
            case 3:
                cSkip = CSKIP_DEPTH_3;
                break;
        #endif
        #ifdef CSKIP_DEPTH_4
            case 4:
                cSkip = CSKIP_DEPTH_4;
                break;
        #endif
        #ifdef CSKIP_DEPTH_5
            case 5:
                cSkip = CSKIP_DEPTH_5;
                break;
        #endif
        #ifdef CSKIP_DEPTH_6
            case 6:
                cSkip = CSKIP_DEPTH_6;
                break;
        #endif
        #ifdef CSKIP_DEPTH_7
            case 7:
                cSkip = CSKIP_DEPTH_7;
                break;
        #endif
        #ifdef CSKIP_DEPTH_8
            case 8:
                cSkip = CSKIP_DEPTH_8;
                break;
        #endif
        #ifdef CSKIP_DEPTH_9
            #error CSKIP look-up table needs to be expanded.
        #endif
    }

    return cSkip;
}
#endif
#endif

/*********************************************************************
 * Function:        BOOL FindNextHopAddress(SHORT_ADDR destAddr, SHORT_ADDR *nextHop)
 *
 * PreCondition:    None
 *
 * Input:           destAddr  - desired short address
 *                  *nextHop - pointer to next hop address
 *
 * Output:          TRUE - if an entry found in the routing table or neighbor table.
 *                  FALSE - if no entry found in both the tables.
 *
 * Side Effects:    None
 *
 * Overview:        This routine finds an entry in the routing table or neighbor table
 *                  for the given destination address.  If an entry exists in the
 *                  neighbor table, then the nextHop address will be updated with the
 *                  same destAddr parameter, beacuse it is in physical proximity. If
 *                  an entry exists in the routing table, then the nextHop address will
 *                  be updated with the nexthop field in the Routing table.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING)
BOOL FindNextHopAddress( SHORT_ADDR destAddr, SHORT_ADDR *nextHopAddr)
{
    BOOL                foundNextHop = FALSE;
    BYTE                i;

#if !defined( USE_TREE_ROUTING_ONLY)
    #ifdef USE_EXTERNAL_NVM
    for(i=0, pCurrentRoutingEntry = routingTable;
        i<ROUTING_TABLE_SIZE;
        i++, pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY))
    #else
    for(i=0, pCurrentRoutingEntry = routingTable;
        i<ROUTING_TABLE_SIZE;
        i++, pCurrentRoutingEntry++)
    #endif
    {
        GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
        if (currentRoutingEntry.destAddress.Val == destAddr.Val)
        {
            if (currentRoutingEntry.rtflags.rtbits.bStatus == ROUTE_ACTIVE)
            {
                foundNextHop = TRUE;
                *nextHopAddr = currentRoutingEntry.nextHop;
                #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
                    currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                    PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
                #endif
                return foundNextHop;
            }
        }
    }
#endif
    if ((NWKLookupNodeByShortAddrVal( destAddr.Val ) != INVALID_NEIGHBOR_KEY) &&
             (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ||
               currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT ||
               currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_SIBLING))
    {
        foundNextHop = TRUE;
        *nextHopAddr = destAddr;
        return foundNextHop;
    }

#if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
    if (RouteAlongTree( destAddr, nextHopAddr ))
    {
        foundNextHop = TRUE;
    }
    else
    {
        foundNextHop = FALSE;
    }
#endif

    return foundNextHop;
}
#endif

/*********************************************************************
 * Function:        BOOL GetNextHop(    SHORT_ADDR destAddr,
 *                                      SHORT_ADDR *nextHop,
 *                                      BYTE *routeStatus )
 *
 * PreCondition:    None
 *
 * Input:           destAddr  - desired short address
 *                  *nextHop - pointer to next hop address
 *                  *routeStatus - pointer to route status
 *
 * Output:          TRUE if an entry found in the routing table.
 *                      currentRoutingEntry and pCurrentRoutingEntry
 *                      will be set the the Routing Table entry
 *                      that matches the request.
 *                  FALSE if no entry found.
 *
 * Side Effects:    None
 *
 * Overview:        This routine finds an entry in the routing table
 *                  for the given destination address.  Note that the
 *                  caller must check the route status to see how to
 *                  use the next hop destination.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
BOOL GetNextHop( SHORT_ADDR destAddr, SHORT_ADDR *nextHop, BYTE *routeStatus )
{
    BOOL                foundNextHop;
    foundNextHop = FALSE;
    *routeStatus = ROUTE_INACTIVE;

    #if !defined( USE_TREE_ROUTING_ONLY)
        if(GetDestFromRoutingTable(destAddr,nextHop,routeStatus ))
        {
                return TRUE;
        }
    #endif //!( USE_TREE_ROUTING_ONLY)

//    #ifdef I_AM_ROUTER
        // Reason behind including this condition check is beacuse,
        // In Non Preconfig mode, the path from Trust Center to joining device needs
        // be established to support the transmission of TC_LINK_KEY from TC to the
        // joining device. Now, if we have this condition, then Request Key will be
        // transmitted directly to Trust Center if we have heard the Beacon from it at the time
        // scanning before association. And when Trust Center receives the Request Key, he will
        // not have a path to joining device to send the Transport Key with Link Key. So, the
        // device fails to join the network.

        // To overcome this issue, we create the Routing Table entry at the time of joining with
        // the value DestAddr = TrustCenterAddress, NextHop = ParentAddress, Status = ACTIVE,
        // ManyToOne bit = TRUE, RouteRecord required = TRUE (Assuming Low RAM concentrator always)
        // By having this condition, we will override checking of Neighbor Table. Since, we have
        // created the routing table entry, next condition will be satisfied. Then the joining
        // device sends the Route Record before sending the Request Key. When Trust Center receives
        // the Route Record, he creates the Source Routing Table entry and uses this entry to send
        // the Transport key command for TC_LINK_KEY. In this way the device joins and gets authenticated
        // successfully in the network.

        #ifdef I_SUPPORT_SECURITY
            if( ((NOW_I_AM_A_ROUTER()) && !securityStatus.flags.bits.bAuthorization ) || NOW_I_AM_NOT_A_ROUTER() )
        #endif
  //  #endif // #ifdef I_AM_ROUTER
            {
                if ((NWKLookupNodeByShortAddrVal( destAddr.Val ) != INVALID_NEIGHBOR_KEY) &&
                 (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ||
                   currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT ||
                   currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_SIBLING))
                {
                    #if ( I_SUPPORT_SOURCE_ROUTE_TESTING == 0x01 )
                        if ( SourceRouteTestingInProgress )
                        {
                            if ( ( currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE ) &&
                                 ( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ) )
                            {
                                foundNextHop = TRUE;
                                *nextHop = destAddr;
                                *routeStatus = ROUTE_ACTIVE;
                                return foundNextHop;
                            }
                        }
                        else
                    #endif // ( I_SUPPORT_SOURCE_ROUTE_TESTING == 0x01 )
                        {
                            foundNextHop = TRUE;
                            *nextHop = destAddr;
                            *routeStatus = ROUTE_ACTIVE;
                            return foundNextHop;
                        }
                }
            }


    /*#if !defined( USE_TREE_ROUTING_ONLY)
        if(GetDestFromRoutingTable(destAddr,nextHop,routeStatus ))
        {
                return TRUE;
        }
    #endif //!( USE_TREE_ROUTING_ONLY)*/

#if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
    if (RouteAlongTree( destAddr, nextHop ))
    {
        foundNextHop = TRUE;
        *routeStatus = ROUTE_ACTIVE;
    }
   /* else
    {
        foundNextHop = FALSE;
        *routeStatus = ROUTE_INACTIVE;
    }
#else
    *routeStatus = ROUTE_INACTIVE;*/
#endif

    return foundNextHop;
}
#endif

/*******************************************************************************
 * Function:        BYTE GetDestFromRoutingTable( SHORT_ADDR destAddr,
 *                  SHORT_ADDR *nextHop, BYTE *routeStatus )
 *
 * PreCondition:    none
 *
 * Input:           destAddr - The address to be searched in routing table
 *
 * Output:          *nextHop - The nest hop mac address to send data to destAddr
 *                  *routeStatus - The routing table status of device
 * Side Effects:    None
 *
 * Overview:        This function searches the routing table for the given
 *                  dest address and gives the nest hop address and state of device
 *                  like route active, whether destination is a concentrator
 *                  and whether route record has to be send\.
 ******************************************************************************/

 #ifdef I_SUPPORT_ROUTING
    BYTE GetDestFromRoutingTable( SHORT_ADDR destAddr, SHORT_ADDR *nextHop, BYTE *routeStatus )
    {
              BOOL                foundNextHop;
              BYTE                i;
              foundNextHop = FALSE;
              *routeStatus = 0xFF;
            #ifdef USE_EXTERNAL_NVM
                for(i=0, pCurrentRoutingEntry = routingTable;
                    i<ROUTING_TABLE_SIZE;
                    i++, pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY))
            #else
                for(i=0, pCurrentRoutingEntry = routingTable;
                    i<ROUTING_TABLE_SIZE;
                    i++, pCurrentRoutingEntry++)
            #endif
        {
            GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
            if ((currentRoutingEntry.destAddress.Val == destAddr.Val))
            {
                *routeStatus = currentRoutingEntry.rtflags.rtbits.bStatus;

                if ((currentRoutingEntry.rtflags.rtbits.bStatus == ROUTE_ACTIVE))
                {
                    foundNextHop = TRUE;
                    *nextHop = currentRoutingEntry.nextHop;
                    #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
                        currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                        PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
                    #endif
                    #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
                        if ( currentRoutingEntry.rtflags.rtbits.bManyToOne )
                        {
                            *routeStatus = NO_ROUTE_RECORD_REQUIRED;
                            if ( currentRoutingEntry.rtflags.rtbits.bRouteRecordRequired )
                            {
                                *routeStatus = ROUTE_RECORD_REQUIRED;
                                if(currentRoutingEntry.rtflags.rtbits.bNoRouteCache == FALSE  )
                                {
                                    currentRoutingEntry.rtflags.rtbits.bRouteRecordRequired = FALSE;
                                }
                                PutRoutingEntry( pCurrentRoutingEntry , &currentRoutingEntry  );
                            }
                        }

                    #endif /*  I_SUPPORT_MANY_TO_ONE_HANDLING == 1 */
                    return foundNextHop;
                }

            }
        }
        return foundNextHop;
    }
 #endif

 /*******************************************************************************
 * Function:        BYTE GetMulticastRoutingEntry( SHORT_ADDR groupAddr,
 *                  SHORT_ADDR *nextHop, BYTE *routeStatus )
 *
 * PreCondition:    none
 *
 * Input:           groupAddr - The group address to be searched in routing table
 *
 * Output:          *nextHop - The nest hop mac address to send data to destAddr
 *                  *routeStatus - The routing table status of device
 * Side Effects:    None
 *
 * Overview:        This function searches for multicast routing table for the given
 *                  group address and gives the next hop address and state of device
 *                  like route active.
 ******************************************************************************/

#ifdef I_SUPPORT_ROUTING
BYTE GetMulticastRoutingEntry( SHORT_ADDR groupAddr, SHORT_ADDR *nextHop, BYTE *routeStatus )
{
    BOOL    foundNextHop;
    BYTE    i;
    foundNextHop = FALSE;
    *routeStatus = 0xFF;
    #ifdef USE_EXTERNAL_NVM
        for(i=0, pCurrentRoutingEntry = routingTable;
            i<ROUTING_TABLE_SIZE;
            i++, pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY))
    #else
        for(i=0, pCurrentRoutingEntry = routingTable;
            i<ROUTING_TABLE_SIZE;
            i++, pCurrentRoutingEntry++)
    #endif
    {
        GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
        if ( ( currentRoutingEntry.destAddress.Val == groupAddr.Val ) &&
             ( currentRoutingEntry.rtflags.rtbits.bStatus == ROUTE_ACTIVE )  &&
             ( currentRoutingEntry.rtflags.rtbits.bGroupIDFlag == TRUE ) )
        {
            *routeStatus = currentRoutingEntry.rtflags.rtbits.bStatus;
            foundNextHop = TRUE;
            *nextHop = currentRoutingEntry.nextHop;
            #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
                currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
                PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
            #endif
            return foundNextHop;
        }
    }
    return foundNextHop;
}
#endif
/*******************************************************************************
 * Function:        BYTE GetRoutingAddress( BOOL fromUpperLayers, SHORT_ADDR nwkAddress,
 *                      BYTE discoverRoute, SHORT_ADDR *macAddress )
 *
 * PreCondition:    none
 *
 * Input:           fromUpperLayers - if we are currently in a NLDE_DATA_request or
 *                          MCPS_DATA_indication
 *                  nwkAddress - final destination address
 *                  discoverRoute - Route Discovery indication
 *
 * Output:          *macAddress - address of next hop to the final destination
 *                  ROUTE_FAILURE               message cannot be sent or buffered
 *                  ROUTE_MESSAGE_BUFFERED      message buffered pending route discovery
 *                  ROUTE_SEND_TO_MAC_ADDRESS   valid destination address found
 *
 * Side Effects:    None
 *
 * Overview:        This function takes the target NWK address and determines
 *                  the next hop MAC address to send the message to.  If the
 *                  destination address is not our child and we do not have a
 *                  route, this routine buffers the message and initiates
 *                  Route Discovery.  If macAddress is a valid next hop
 *                  destination, the routine returns TRUE.
 *
 * Note:            There is a hole in this algorithm.  HaveRoutingCapacity is
 *                  checked before we look for the next hop address in the routing
 *                  table.  It is possible that the route discovery table could
 *                  be full discovering other routes, which causes HaveRoutingCapacity
 *                  to fail, while a route already exists in the routing table.
 *                  The spec may change...
 ******************************************************************************/
/*#ifndef I_AM_RFD
MESSAGE_ROUTING_STATUS GetRoutingAddress( BOOL fromUpperLayers, SHORT_ADDR nwkAddress, BYTE discoverRoute, SHORT_ADDR *macAddress )
{
    if ((NWKLookupNodeByShortAddrVal( nwkAddress.Val ) != INVALID_NEIGHBOR_KEY) &&
             (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ||
               currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT ||
               currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_SIBLING))
    {
        // The destination is one of my neighbor.

        *macAddress = nwkAddress;
        // We can send the message.
        return ROUTE_SEND_TO_MAC_ADDRESS;
    }
    else
    {

        #ifdef I_SUPPORT_ROUTING
            #if !defined( USE_TREE_ROUTING_ONLY)
                if (HaveRoutingCapacity( FALSE, 0, macPIB.macShortAddress, nwkAddress, DEFAULT_COMMAND_OPTIONS ))
                {
                    // Try to route the current packet.
                    if (discoverRoute == ROUTE_DISCOVERY_FORCE)
                    {
                        // We don't want the next hop to force a route discovery again!
                        return InitiateRouteDiscovery( fromUpperLayers, ROUTE_DISCOVERY_ENABLE );
                    }
                    else
                    {
                        BYTE    routeStatus;

                        if (GetNextHop( nwkAddress, macAddress, &routeStatus ))
                        {
                            if (routeStatus == ROUTE_ACTIVE)
                            {
                                // We can send the message.
                                return ROUTE_SEND_TO_MAC_ADDRESS;
                            }
                            else if( routeStatus == ROUTE_RECORD_REQUIRED)
                            {
                                return ROUTE_AVAILABLE_TO_CONCENTRATOR;
                            }
                            else
                            {
                                #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
                                    discoverRoute = ROUTE_DISCOVERY_SUPPRESS;
                                    goto DoTreeRouting;
                                #endif
                            }
                        }
                        else
                        {
                            if (discoverRoute == ROUTE_DISCOVERY_ENABLE)
                            {
                                return InitiateRouteDiscovery( fromUpperLayers, ROUTE_DISCOVERY_ENABLE );
                            }
                            else
                            {
                                #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
                                    goto DoTreeRouting;
                                #endif
                            }
                        }
                    }
                }
                else
            #endif // USE_TREE_ROUTING_ONLY
                {
                    #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
DoTreeRouting:
                        if (!RouteAlongTree( nwkAddress, macAddress ))
                        {
                            params.NLDE_DATA_confirm.Status = NWK_ROUTE_ERROR;

                            // cannot route the message.

                            return ROUTE_FAILURE_TREE_LINK;
                        }
                        else
                        {
                            // send the message.
                            return ROUTE_SEND_TO_MAC_ADDRESS;
                        }
                    #endif
                }
        #else
            return ROUTE_FAILURE_TREE_LINK;
        #endif  // I_SUPPORT_ROUTING
   }
   return ROUTE_FAILURE_TREE_LINK;
}
#endif*/

#ifndef I_AM_END_DEVICE
MESSAGE_ROUTING_STATUS GetRoutingAddress( BOOL fromUpperLayers, SHORT_ADDR nwkAddress, BYTE discoverRoute, SHORT_ADDR *macAddress )
{
    #ifdef I_SUPPORT_ROUTING
        BYTE    routeStatus;


        if (discoverRoute == ROUTE_DISCOVERY_FORCE)
        {
            #if !defined( USE_TREE_ROUTING_ONLY)
                if (HaveRoutingCapacity( fromUpperLayers, 0, macPIB.macShortAddress, nwkAddress, DEFAULT_COMMAND_OPTIONS ))
                {
                    return InitiateRouteDiscovery( fromUpperLayers, ROUTE_DISCOVERY_ENABLE );
                }
            #endif
        }

        if (GetNextHop( nwkAddress, macAddress, &routeStatus ))
        {
            if ((routeStatus == ROUTE_ACTIVE)||(routeStatus == NO_ROUTE_RECORD_REQUIRED))
            {
                // We can send the message.
                return ROUTE_SEND_TO_MAC_ADDRESS;
            }
            else if( routeStatus == ROUTE_RECORD_REQUIRED)
            {
                return ROUTE_AVAILABLE_TO_CONCENTRATOR;
            }
            else
            {
                #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
                    discoverRoute = ROUTE_DISCOVERY_SUPPRESS;
                    goto DoTreeRouting;
                #endif
            }
        }
        else
        {
            if (routeStatus == ROUTE_DISCOVERY_UNDERWAY)
            {
                return ROUTE_DISCOVERY_ALREADY_INITIATED;
            }
        }
        if (discoverRoute == ROUTE_DISCOVERY_ENABLE)
        {
            #if !defined( USE_TREE_ROUTING_ONLY)
                if (HaveRoutingCapacity( fromUpperLayers, 0, macPIB.macShortAddress, nwkAddress, DEFAULT_COMMAND_OPTIONS ))
                {
                    return InitiateRouteDiscovery( fromUpperLayers, ROUTE_DISCOVERY_ENABLE );
                }
            #endif
        }
        else
        {
            #if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
DoTreeRouting:
                if (!RouteAlongTree( nwkAddress, macAddress ))
                {
                    params.NLDE_DATA_confirm.Status = NWK_ROUTE_ERROR;

                    // cannot route the message.

                    return ROUTE_FAILURE_TREE_LINK;
                }
                else
                {
                    // send the message.
                    return ROUTE_SEND_TO_MAC_ADDRESS;
                }
            #endif
        }
    #endif  // I_SUPPORT_ROUTING

    return ROUTE_FAILURE_TREE_LINK;
}
#endif //I_AM_END_DEVICE

/*********************************************************************
 * Function:        BOOL HaveRoutingCapacity( BOOL validID, BYTE routeRequestID,
 *                      SHORT_ADDR routeSourceAddress,
 *                      SHORT_ADDR routeDestAddress,
 *                      BYTE commandOptions )
 *
 * PreCondition:    None
 *
 * Input:           validID - routeRequestID is a valid ID
 *                  routeRequestID - ID of current route request.  Valid
 *                      if routeSourceAddress is not our address
 *                  routeSourceAddress - source of the route
 *                  routeDestAddress - destination of the route
 *                  routeRepair - if this is a route repair operation
 *
 * Output:          TRUE - we have routing capacity
 *                  FALSE - we do not have routing capacity
 *
 * Side Effects:    None
 *
 * Overview:        This function determines if we have routing
 *                  capacity based on the spec definition:
 *
 *                  A device is said to have routing table capacity if:
 *                    1. It is a ZigBee coordinator or ZigBee router.
 *                    2. It maintains a routing table.
 *                    3. It has a free routing table entry or it already
 *                       has a routing table entry corresponding to the
 *                       destination.
 *                    4. The device is attempting route repair and it has
 *                       reserved some entries for this purpose as described
 *                       above.
 *                  A device is said to have “route discovery table capacity” if:
 *                    1. It maintains a route discovery table.
 *                    2. It has a free entry in its route discovery table.
 *                  If a device has both routing table capacity and route
 *                  discovery table capacity then it may be said the have
 *                  “routing capacity”.
 *
 * Note:            "Route Discovery Table Capacity" does not take into
 *                  account if we are already discovering the current
 *                  route.  This will occur often, since route discovery
 *                  is broadcast.  It has therefore been added to this
 *                  implementation.
 *
 *                  If we are in this routine as a response to upper
 *                  layers sending an NLME_DATA_request, we do not
 *                  have a route ID. Therefore, the route is new, and
 *                  routeRequestID is invalid.
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
BOOL HaveRoutingCapacity( BOOL validID, BYTE routeRequestID, SHORT_ADDR routeSourceAddress,
    SHORT_ADDR routeDestAddress, BYTE commandOptions )
{
    BYTE                i;
    BYTE                maxTableEntry;
    BOOL                result;
    #if (I_SUPPORT_SYMMETRIC_LINK == 0x01)
        BYTE            numOfEntriesToSupportSymLink = 0;
    #endif

    result = FALSE;

    // Check for capacity in the route discovery table.
    for( i=0; i<ROUTE_DISCOVERY_TABLE_SIZE; i++)
    {
        if ((routeDiscoveryTablePointer[i] == NULL) ||
            ( validID &&   //(routeSourceAddress.Val != macPIB.macShortAddress.Val) &&
             (routeDiscoveryTablePointer[i]->routeRequestID == routeRequestID) &&
             (routeDiscoveryTablePointer[i]->srcAddress.Val == routeSourceAddress.Val)))
        {
            result = TRUE;
        }
    }

    // If we have capacity in the route discovery table, make sure
    // we also have capacity in the routing table.  Note that we cannot
    // use the reserved routing table entries unless we are doing a
    // route repair.
    maxTableEntry = ROUTING_TABLE_SIZE;
    /*if (commandOptions & ROUTE_REPAIR)
    {
        maxTableEntry = ROUTING_TABLE_SIZE;
    }
    else
    {
        maxTableEntry = ROUTING_TABLE_SIZE-RESERVED_ROUTING_TABLE_ENTRIES;
    }*/

    if (result)
    {
        result = FALSE;
        #ifdef USE_EXTERNAL_NVM
        for( i=0, pCurrentRoutingEntry = routingTable;
            i<maxTableEntry;
            i++, pCurrentRoutingEntry+=(WORD)sizeof(ROUTING_ENTRY) )
        #else
        for( i=0, pCurrentRoutingEntry = routingTable;
            i<maxTableEntry;
            i++, pCurrentRoutingEntry++ )
        #endif
        {
            GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
            if ((currentRoutingEntry.destAddress.Val == routeDestAddress.Val) ||
                (currentRoutingEntry.destAddress.Val == routeSourceAddress.Val) ||
                (currentRoutingEntry.rtflags.rtbits.bStatus == ROUTE_INACTIVE))
            {
                // If higher layer is initiated, then we need only one free
                // routing table entry. In other words, if we are the initiator
                // then only one routing table entry will be created. If we are
                // the intermediate device, then we need two free entries to
                // support both forward and backward paths. Using validID parameter,
                // we can differentiate whether we are the initiator or intermediate.
                if (validID)
                {
                    #if (I_SUPPORT_SYMMETRIC_LINK == 0x01)
                        numOfEntriesToSupportSymLink++;
                        if (numOfEntriesToSupportSymLink == 0x02) //0x02 is for forward and backward paths.
                    #endif
                        {
                            result = TRUE;
                        }
                }
                else
                {
                    result = TRUE;
                }
            }
        }
    }

    return result;
}
#endif

/*********************************************************************
 * Function:        MESSAGE_ROUTING_STATUS InitiateRouteDiscovery( BOOL fromUpperLayers, BYTE discoverRoute )
 *
 * PreCondition:    None
 *
 * Input:           fromUpperLayers - if we are currently in a NLDE_DATA_request or
 *                          MCPS_DATA_indication
 *                  discoverRoute - Route Discovery indication
 *
 * Output:          None
 *
 * Side Effects:    routedMessage.message is allocated
 *
 * Overview:        If no message is currently awaiting route discovery,
 *                  this routine buffers the current message and
 *                  initiates route discovery.  If we already have a
 *                  message buffered, this message will be discarded.
 *
 * Note:            if fromUpperLayers is TRUE, we are servicing an
 *                      NLDE_DATA_request
 *                  if fromUpperLayers is FALSE, we are servicing an
 *                      MCPS_DATA_indication
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
MESSAGE_ROUTING_STATUS InitiateRouteDiscovery( BOOL fromUpperLayers, BYTE discoverRoute )
{
    BYTE        i;
    BYTE        commandOptions;
    BYTE        messageIndex;
    BYTE        *ptr;
    BYTE        radius;
    BYTE        rdIndex;
    BYTE        rtIndex;
    TICK        tempTime;
    LONG_ADDR   tmpLongAddr;
    SHORT_ADDR  dummyAddr;

    dummyAddr.Val = 0xffff;

    // Find an empty place to store the buffered message
    for (messageIndex=0; (messageIndex<NUM_BUFFERED_ROUTING_MESSAGES) && (nwkStatus.routingMessages[messageIndex]); messageIndex++) {}

    // If there are no empty places, we cannot route this message.
    if (messageIndex==NUM_BUFFERED_ROUTING_MESSAGES)
    {
        return ROUTE_FAILURE_NO_CAPACITY;
    }

    // Start to buffer the message
    if ((nwkStatus.routingMessages[messageIndex] = (BUFFERED_MESSAGE_INFO *)SRAMalloc( sizeof(BUFFERED_MESSAGE_INFO) )) == NULL)
    {
        return ROUTE_FAILURE_NO_CAPACITY;
    }

    if (fromUpperLayers)
    {
        nwkStatus.routingMessages[messageIndex]->sourceAddress.Val      = macPIB.macShortAddress.Val;
        nwkStatus.routingMessages[messageIndex]->destinationAddress.Val = params.NLDE_DATA_request.DstAddr.Val;
        nwkStatus.routingMessages[messageIndex]->dataLength             = (TX_HEADER_START - TxHeader) + 8 + TxData;

        if ((ptr = (BYTE *)SRAMalloc(nwkStatus.routingMessages[messageIndex]->dataLength)) == NULL)
        {
            nfree( nwkStatus.routingMessages[messageIndex] );

            return ROUTE_FAILURE_NO_CAPACITY;
        }

        nwkStatus.routingMessages[messageIndex]->dataPtr = ptr;

        radius = params.NLDE_DATA_request.BroadcastRadius;

        // Buffer the current message.
        CreateNwkHeaderForDataAndStoreData( discoverRoute, ptr );

    }
    else
    {
        NWK_FRAME_CONTROL_MSB   nwkRcvdFrameControlMSB;
        NWK_FRAME_CONTROL_LSB   nwkRcvdFrameControlLSB;
        
        /* update the  route discovery field */
        nwkRcvdFrameControlLSB.Val = (messageToForward.frameCtrlLSB & 0x3F) | (discoverRoute << 6); 
        nwkRcvdFrameControlMSB.Val = messageToForward.frameCtrlMSB; 
        
        /*Check whether the received frame contains variable length Src and Dst IEEE addresses*/
        if( nwkRcvdFrameControlMSB.bits.multicastFlag )
        {
            params.MCPS_DATA_indication.msdu        -= 1;
        }
        if( nwkRcvdFrameControlMSB.bits.srcIEEEAddr )
        {
            params.MCPS_DATA_indication.msdu        -= 8;
        }
        if( nwkRcvdFrameControlMSB.bits.dstIEEEAddr )
        {
            params.MCPS_DATA_indication.msdu        -= 8;
        }
        
        // Back up the packet to point to the beginning of the NWK header.  Pull the final route destination out
        // of the NWK header while we're doing it.
        params.MCPS_DATA_indication.msdu        -= 4;
        nwkStatus.routingMessages[messageIndex]->sourceAddress.Val           = params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val;
        nwkStatus.routingMessages[messageIndex]->destinationAddress.byte.MSB = *(--params.MCPS_DATA_indication.msdu);
        nwkStatus.routingMessages[messageIndex]->destinationAddress.byte.LSB = *(--params.MCPS_DATA_indication.msdu);
        params.MCPS_DATA_indication.msdu        -= 2;
        params.MCPS_DATA_indication.msduLength  += 8;
        
        if( nwkRcvdFrameControlMSB.bits.multicastFlag )
        {
            params.MCPS_DATA_indication.msduLength  += 1;
        }
        if( nwkRcvdFrameControlMSB.bits.srcIEEEAddr )
        {
            params.MCPS_DATA_indication.msduLength  += 8;
        }
        if( nwkRcvdFrameControlMSB.bits.dstIEEEAddr )
        {
            params.MCPS_DATA_indication.msduLength  += 8;
        }
        
        nwkStatus.routingMessages[messageIndex]->dataLength                  = params.MCPS_DATA_indication.msduLength;

        if ((ptr = (BYTE *)SRAMalloc(nwkStatus.routingMessages[messageIndex]->dataLength)) == NULL)
        {
            nfree( nwkStatus.routingMessages[messageIndex] );
            return ROUTE_FAILURE_NO_CAPACITY;
        }

        nwkStatus.routingMessages[messageIndex]->dataPtr = ptr;

        // Buffer the current message.  Create the NWK header first.
        /* Use NWKGet() and the discoverRoute parameter. */
        *ptr++ = (NWKGet()& 0x3F) | (discoverRoute << 6); 	
        *ptr++ = NWKGet(); 
        *ptr++ = NWKGet();      // destAddr.byte.LSB;
        *ptr++ = NWKGet();      // destAddr.byte.MSB;
        *ptr++ = NWKGet();      // srcAddr.byte.LSB;
        *ptr++ = NWKGet();      // srcAddr.byte.MSB;
        radius = NWKGet()-1;
        *ptr++ = radius;        // broadcastRadius;
        *ptr++ = NWKGet();      // broadcastSequence;
        
        /* handle the variable header length */
        if ( nwkRcvdFrameControlMSB.bits.dstIEEEAddr )
        {
            for(i = 0; i < 8; i++)
            {
                *ptr++ = NWKGet();
            }
        }
        if ( nwkRcvdFrameControlMSB.bits.srcIEEEAddr )
        {
            for(i = 0; i < 8; i++)
            {
                *ptr++ = NWKGet();
            }
        }
        if( nwkRcvdFrameControlMSB.bits.multicastFlag )
        {
            *ptr++ = NWKGet();
        }    

        // Buffer the NWK payload.
        while (params.MCPS_DATA_indication.msduLength)
        {
            *ptr++ = NWKGet();
        }

        // We will discard the message when we return from this function. - NWKDiscardRx();
    }
    // Record when we started route discovery so we can purge this
    // message in case route discovery fails.
    nwkStatus.routingMessages[messageIndex]->timeStamp = TickGet();
	
	printf("Creat Route Table 3\n\r");
    // Create the Routing and Route Discovery table entries.
    if (
        #if (I_SUPPORT_SYMMETRIC_LINK == 0x01)
            !CreateRoutingTableEntries
            (
                nwkStatus.routingMessages[messageIndex]->destinationAddress,
                &rdIndex,
                &rtIndex,
                dummyAddr, // dummy because we are the initiator
                dummyAddr ,// dummy because we are the initiator
                FALSE
            )
        #else
			
			
            !CreateRoutingTableEntries
            (
                nwkStatus.routingMessages[messageIndex]->destinationAddress,
                &rdIndex,
                &rtIndex
            )
        #endif
       )
    {
        // Destroy the buffered message.
        SRAMfree( nwkStatus.routingMessages[messageIndex]->dataPtr );
        nfree( nwkStatus.routingMessages[messageIndex] );
        return ROUTE_FAILURE_NO_CAPACITY;
    }

    // We successfully buffered the current message and created routing table entries.  Since
    // route discovery broadcasts must be triggered in the background, unblock the transmit path.
    ZigBeeUnblockTx();

    /*sizeof_ROUTE_REQUEST_COMMAND + sizeof_ROUTE_REQUEST_COMMAND_HEADER = 6 + 16 = 22Bytes*/
    if (!(ptr = routeDiscoveryTablePointer[rdIndex]->forwardRREQ = SRAMalloc(sizeof_ROUTE_REQUEST_COMMAND +
                                                                         #if ( ZIGBEE_PRO == 0x01)
                                                                             sizeof_RREQ_COMMAND_DST_IEEE_ADDR +
                                                                         #endif
                                                                             sizeof_ROUTE_REQUEST_COMMAND_HEADER)))
    {
        // Destroy the Routing Table entry.
        currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_DISCOVERY_UNDERWAY;
        PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );

        // Destroy the Route Discovery Table entry.
        nfree( routeDiscoveryTablePointer[rdIndex] );

        // Destroy the buffered message.
        SRAMfree( nwkStatus.routingMessages[messageIndex]->dataPtr );
        nfree( nwkStatus.routingMessages[messageIndex] );
        //printf("ENwk_4");
        return ROUTE_FAILURE_NO_CAPACITY;
    }

    // Populate the remainder of the Route Discovery table entry.
    routeDiscoveryTablePointer[rdIndex]->routeRequestID             = nwkStatus.routeRequestID;
    routeDiscoveryTablePointer[rdIndex]->srcAddress                 = macPIB.macShortAddress;
    //routeDiscoveryTablePointer[rdIndex]->senderAddress              = macPIB.macShortAddress;
    routeDiscoveryTablePointer[rdIndex]->forwardCost                = 0;
    routeDiscoveryTablePointer[rdIndex]->status.transmitCounter     = nwkcInitialRREQRetries + 1;
    routeDiscoveryTablePointer[rdIndex]->status.initialRebroadcast  = 0;     // We are initiating, so we use only nwkcRREQRetryInterval

    /*Mark whether the device is an intiator or intermediate.
    This information is used to retain the status of Routing table entry as ACTIVE.*/
    routeDiscoveryTablePointer[rdIndex]->status.initiator = 0x01; //We are the intiator

    // Buffer the RREQ so we can transmit (and retransmit...) from the background.  Watch the order that it's loaded.

    // Load up the NWK payload - the route request command frame.
    *ptr++ = NWK_COMMAND_ROUTE_REQUEST;
    #if ( ZIGBEE_PRO == 0x01)
        if ( IsThisAddressKnown
             (
               nwkStatus.routingMessages[messageIndex]->destinationAddress,
               (BYTE *)&tmpLongAddr
             )
           )
        {
			printf("\n\rLong Address Put into Route discovery\n\r");
            commandOptions = DEFAULT_COMMAND_OPTIONS | DEST_IEEE_ADDRESS_BIT;
        }
        else
    #endif
        {
            commandOptions = DEFAULT_COMMAND_OPTIONS;
        }
		
		if(RoLongFlage == 1)
			commandOptions = DEFAULT_COMMAND_OPTIONS | DEST_IEEE_ADDRESS_BIT;
		
    *ptr++ = commandOptions;
    *ptr++ = nwkStatus.routeRequestID++;
    *ptr++ = nwkStatus.routingMessages[messageIndex]->destinationAddress.byte.LSB;
    *ptr++ = nwkStatus.routingMessages[messageIndex]->destinationAddress.byte.MSB;
    *ptr++ = 0x00; // path cost
    #if ( ZIGBEE_PRO == 0x01)
        if ( commandOptions & DEST_IEEE_ADDRESS_BIT )
        {
			printf("\n\rLong Address Put into Route discovery success\n\r");
            for(i = 0; i < 8; i++)
            {
                *ptr++ = tmpLongAddr.v[7-i];
            }
        }
    #endif
    // Load up the NWK header (backwards).
    /*Add the SrcIEEEAddress field in the NWK Header*/
    GetMACAddress(&tmpLongAddr);
    for(i = 0; i < 8; i++)
    {
        *ptr++ = tmpLongAddr.v[7-i];
    }

    *ptr++ = NLME_GET_nwkBCSN();
    *ptr++ = radius;
    *ptr++ = macPIB.macShortAddress.byte.MSB;
    *ptr++ = macPIB.macShortAddress.byte.LSB;
    // A ZigBee 2006 requierement:, addressing all routers
    *ptr++ = 0xFF; //nwkStatus.routingMessages[messageIndex]->destinationAddress.byte.MSB;
    *ptr++ = 0xFC; //nwkStatus.routingMessages[messageIndex]->destinationAddress.byte.LSB;

#ifdef I_SUPPORT_SECURITY
    *ptr++ = 0x02 | NWK_IEEE_SRC_ADDR;
#else
    *ptr++ = 0x00 | NWK_IEEE_SRC_ADDR;
#endif
    *ptr++ = NWK_FRAME_CMD | (nwkProtocolVersion<<2);    // nwkFrameControlLSB

    // Set the timer for the first transmission, so it goes off immediately.
    tempTime = TickGet();
    routeDiscoveryTablePointer[rdIndex]->rebroadcastTimer.Val = tempTime.Val - (DWORD)(ONE_SECOND*nwkcRREQRetryInterval/1000);

    return ROUTE_MESSAGE_BUFFERED;
}
#endif


/*********************************************************************
 * Function:        BOOL IsDescendant( SHORT_ADDR parentAddr,
 *                                     SHORT_ADDR childAddr,
 *                                     BYTE parentDepth )
 *
 * PreCondition:    None
 *
 * Input:           parentAddr - short address of the parent device
 *                  childAddr - short address of the child device
 *                  parentDepth - depth of the parent
 *
 * Output:          TRUE - child is a descendant of the parent
 *                  FALSE otherwise
 *
 * Side Effects:    None
 *
 * Overview:        This function determines if the child device is a
 *                  descendant of the parent device.  If the
 *                  If I am the coordinator, the device is always my
 *                  descendant.  Otherwise, the device is my descendant
 *                  if its address is within the address range that I
 *                  can allocate.
 *
 * Note:            None
 ********************************************************************/
#ifdef I_SUPPORT_ROUTING
#if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
BOOL IsDescendant( SHORT_ADDR parentAddr, SHORT_ADDR childAddr, BYTE parentDepth )
{
    if (parentDepth == 0)
    {
        return TRUE;
    }
    else
    {
        if ((parentAddr.Val < childAddr.Val) &&
            (childAddr.Val < (parentAddr.Val + GetCSkipVal( parentDepth-1 ))))
            return TRUE;
        else
            return FALSE;
    }
}
#endif
#endif

/*********************************************************************
 * Function:        void MarkNeighborAsPasssiveACKed( BYTE BTTindex )
 *
 * PreCondition:    This function must be called while MCPS_DATA_indication
 *                  parameters are valid.
 *
 * Input:           BTTindex
 *
 * Output:          None
 *
 * Side Effects:    The indicated BTR will be updated to reflect that
 *                  the source of the current message has rebroadcast
 *                  the message.
 *
 * Overview:        This routine marks indicated BTR as having been
 *                  passive ACK'd by the source of the current message.
 *
 * Note:            None
 ********************************************************************/
//#ifndef I_AM_RFD
void MarkNeighborAsPasssiveACKed( BYTE BTTindex )
{
    BYTE    j;

    j = 0;

    while (j<MAX_NEIGHBORS)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentNeighborRecord = neighborTable + (WORD)j * (WORD)sizeof(NEIGHBOR_RECORD);
        #else
            pCurrentNeighborRecord = &neighborTable[j];
        #endif
        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
        if (currentNeighborRecord.shortAddr.Val == params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val)
        {
            nwkStatus.BTT[BTTindex]->flags[j].bits.bMessageNotRelayed = 0;
            break;
        }
        j++;
    }
}
//#endif

/*******************************************************************************
 * Function:        BYTE NLME_GET_nwkBCSN( void )
 *
 * PreCondition:    none
 *
 * Input:           none
 *
 * Output:          a valid sequence number to use in the NWK header
 *
 * Side Effects:    None
 *
 * Overview:        This function provides the caller with a valid sequence
 *                  number to use in the NWK header.
 *
 * Note:            INVALID_NWK_HANDLE is reserved
 ******************************************************************************/
BYTE NLME_GET_nwkBCSN( void )
{
    NIB.nwkBCSN++;
    if (NIB.nwkBCSN == INVALID_NWK_HANDLE) NIB.nwkBCSN++;
    return NIB.nwkBCSN;
}

/*********************************************************************
 * Function:        void NWKClearNeighborTable( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Clears entire neighbor table.
 *
 * Note:            None
 ********************************************************************/
void NWKClearNeighborTable( void )
{
    BYTE i;

    // We are clearing the neighbor table, so we will lose all parent information.
    if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
        ZigBeeStatus.flags.bits.bTryOrphanJoin = 0;
    }//#endif


    currentNeighborRecord.deviceInfo.Val = 0;

    #ifdef USE_EXTERNAL_NVM
    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
    }

    // Reset count fields, indicate that the address info is invalid, clear
    // the parent record index, and mark the table as valid.

    currentNeighborTableInfo.neighborTableSize  = 0;
    if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
        currentNeighborTableInfo.parentNeighborTableIndex = INVALID_NEIGHBOR_KEY;
    }//#endif
    #ifndef I_AM_END_DEVICE
        currentNeighborTableInfo.numChildren                        = 0;
        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
            currentNeighborTableInfo.numChildRouters                    = 0;
            currentNeighborTableInfo.flags.bits.bChildAddressInfoValid  = 0;
        #endif
    #endif
    currentNeighborTableInfo.validityKey = NEIGHBOR_TABLE_VALID;
    currentNeighborTableInfo.nwkUpdateId = 0;
    currentNeighborTableInfo.nwkExtendedPANID.v[0]  = 0;
    currentNeighborTableInfo.nwkExtendedPANID.v[1]  = 0;
    currentNeighborTableInfo.nwkExtendedPANID.v[2]  = 0;
    currentNeighborTableInfo.nwkExtendedPANID.v[3]  = 0;
    currentNeighborTableInfo.nwkExtendedPANID.v[4]  = 0;
    currentNeighborTableInfo.nwkExtendedPANID.v[5]  = 0;
    currentNeighborTableInfo.nwkExtendedPANID.v[6]  = 0;
    currentNeighborTableInfo.nwkExtendedPANID.v[7]  = 0;

    PutNeighborTableInfo();
}

/*********************************************************************
 * Function:        void NWKClearRoutingTable( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function clears the routing table.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
void NWKClearRoutingTable( void )
{
    BYTE    i;

    currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_INACTIVE;
    currentRoutingEntry.destAddress.Val = 0xFFFF;
    pCurrentRoutingEntry = routingTable;

    #ifdef USE_EXTERNAL_NVM
    for (i=0; i<ROUTING_TABLE_SIZE; i++, pCurrentRoutingEntry+=(WORD)sizeof(ROUTING_ENTRY))
    #else
    for (i=0; i<ROUTING_TABLE_SIZE; i++, pCurrentRoutingEntry++)
    #endif
    {
        PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
    }
}
#endif

/*********************************************************************
 * Function:        BYTE NWKGet( void )
 *
 * PreCondition:    Must be called from the MCPS_DATA_indication
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

BYTE NWKGet( void )
{
    if (params.MCPS_DATA_indication.msduLength == 0)
    {
        return 0;
    }
    else
    {
        params.MCPS_DATA_indication.msduLength--;
        return *params.MCPS_DATA_indication.msdu++;
    }
}

/*********************************************************************
 * Function:        static void UpdateNwkHeaderFrameContolFields(BYTE includeDstIEEEAddr)
 *
 * PreCondition:    None
 *
 * Input:           includeDstIEEEAddr - Used to set the DstIEEAddress bit field
 *                                       in the frame control field of the NWK
 *                                       Header
 *
 * Output:          TxBuffer will be updated with frame control field of the NWK
 *                  Header.
 *
 * Side Effects:    None.
 *
 * Overview:        Updates the frame control field based for Security,
 *                  SrcIEEEAddress, and DstIEEEAddress
 *
 * Return:          Void.
 *
 * Note:            None.
********************************************************************/
static void UpdateNwkHeaderFrameContolFields(BYTE includeDstIEEEAddr)
{
    BYTE secBitPosInNwkHdr = 0x00;

    // This condition is required because when a device fails to join
    // the network due to Network Key could not be received, then the
    // device should leave the network by unicasting the leave command
    // frame to the parent device. This leave command frame should not
    // be encrypted.
    #ifdef I_SUPPORT_SECURITY
        //if ( !securityStatus.flags.bits.bAuthorization )
        {
            // The device has been authorized already and part of the network.
            secBitPosInNwkHdr = 0x02;
        }
    #endif
    if (includeDstIEEEAddr == TRUE)
    {
        TxBuffer[TxHeader--] = secBitPosInNwkHdr | NWK_IEEE_SRC_ADDR | NWK_IEEE_DST_ADDR;
    }
    else
    {
        TxBuffer[TxHeader--] = secBitPosInNwkHdr | NWK_IEEE_SRC_ADDR;
    }

    TxBuffer[TxHeader--] = NWK_FRAME_CMD | (nwkProtocolVersion<<2);    // nwkFrameControlLSB
}

/*********************************************************************
 * Function:        static BYTE AddNwkHeaderIEEEAddresses (SHORT_ADDR shortAddr)
 *
 * PreCondition:    None
 *
 * Input:           shortAddr               - Short address of node for which
 *                                            the IEEEAddress needs to be added
 *                                            in the DstIEEEAddress field in the
 *                                            NWK Header
 *
 * Output:          TxBuffer will be updated with Nwk Header with IEEEAddress.
 *
 * Side Effects:    None.
 *
 * Overview:        Searches neighbor table and Address Map table for
 *                  matching short address
 *
 * Return:          TRUE, if DstIEEEAddress field is added. FALSE, otherwise.
 *
 * Note:            SrcIEEEAddress field in the NWK Header is always added for
 *                  command frames. DstIEEEAddress field will be added if the
 *                  address is known. So, return value is based only on the
 *                  DstIEEEAddress field.
********************************************************************/
static BYTE AddNwkHeaderIEEEAddresses (SHORT_ADDR shortAddr)
{
    APS_ADDRESS_MAP     tmpMap;
    LONG_ADDR           tmpLongAddr;
    LONG_ADDR           invalidLongAddr;
    BYTE                validLongAddr;
    BYTE                i;

    /*Prepare invalid IEEE Address, i.e 0x00.*/
    for(i = 0; i < 8; i++)
    {
        invalidLongAddr.v[i] = 0x00;
    }

    /*Add the SrcIEEEAddress field in the NWK Header*/
    GetMACAddress(&tmpLongAddr);
    for(i = 0; i < 8; i++)
    {
        TxBuffer[TxHeader--] = tmpLongAddr.v[7-i];
    }

    /*DstIEEEAddress field in the Nwk Header should not be added
    if the shortAddr is not in valid range (0x0000 to 0xFFF7)*/
    if (shortAddr.Val >= 0xFFF7)
    {
        return FALSE;
    }

    /*Add the DstIEEEAddress field in the NWK Header*/
    if (NWKLookupNodeByShortAddrVal(shortAddr.Val) != INVALID_NEIGHBOR_KEY)
    {
        /*Check whether we have valid IEEE address. This is required because, neighbor table will
        not be updated with valid IEEE address when a beacon is received. During this time, instead
        of leaving the longAddr field with junk values, we always update with all zeros. Having this
        condition, we will not send out any command frame with wrong IEEE address.
        */
        validLongAddr = memcmp( (void *)&currentNeighborRecord.longAddr, (void *)&invalidLongAddr.v[0], 8 );

        if (validLongAddr)
        {
            /*Add the DstIEEEAddress field in the NWK Header*/
            for (i = 0; i < 8; i++)
            {
                TxBuffer[TxHeader--] = currentNeighborRecord.longAddr.v[7-i];
            }
            return TRUE;
        }
    }
    else
    {
        /*Search in the Address Map*/
        for( i = 0; i < apscMaxAddrMapEntries; i++)
        {
            #ifdef USE_EXTERNAL_NVM
                GetAPSAddress( &tmpMap,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
            #else
                GetAPSAddress( &tmpMap,  &apsAddressMap[i] );
            #endif
            if (tmpMap.shortAddr.Val == shortAddr.Val)
            {
                /*Add the DstIEEEAddress field in the NWK Header*/
                for (i = 0; i < 8; i++)
                {
                    TxBuffer[TxHeader--] = tmpMap.longAddr.v[7-i];
                }
                return TRUE;
            }
        }
    }
    return FALSE; //DstIEEEAddress is not updated. Command frame should not contain this field.
}

/*********************************************************************
 * Function:        static BYTE IsThisAddressKnown (SHORT_ADDR shortAddr, BYTE* longAddr)
 *
 * PreCondition:    None
 *
 * Input:           shortAddr               - Short address of node for which
 *                                            a search is made
 *                  longAddr                - This is an output parameter. If the
 *                                            short address is known, then the
 *                                            corresponding long address will be
 *                                            will be copied to this parameter.
 * Output:          longAddr will be updated with IEEEAddress of the device for which
 *                  search is made.
 *
 * Side Effects:    None.
 *
 * Overview:        Searches neighbor table and Address Map table for
 *                  matching short address
 *
 * Return:          TRUE, if matches and has valid IEEEAddress. FALSE, otherwise.
 *
 * Note:            If short address matches, but the Extended Address is NULL, then the
 *                  return value will be FALSE.
********************************************************************/
BYTE IsThisAddressKnown (SHORT_ADDR shortAddr, BYTE * longAddr)
{
    APS_ADDRESS_MAP     tmpMap;
    LONG_ADDR           tmpLongAddr;
    LONG_ADDR           invalidLongAddr;
    BYTE                validLongAddr;
    BYTE                i;

    /*Prepare invalid IEEE Address, i.e 0x00.*/
    for(i = 0; i < 8; i++)
    {
        invalidLongAddr.v[i] = 0x00;

        // Make sure we set the longAddr value to Zero, otherwise it may have a junk value.
        *(longAddr + i) = 0x00;
    }

    if ( shortAddr.Val > 0xFFF7 )
    {
        // ShortAddress passed is a broadcast address or reserved. So, it does not match with
        // any of the neighbor or address map table entries. So, just return FALSE.
        return FALSE;
    }

    if ( shortAddr.Val == macPIB.macShortAddress.Val )
    {
        GetMACAddress(&tmpLongAddr);
        for (i = 0; i < 8; i++)
        {
            *longAddr++ = tmpLongAddr.v[7-i];
        }
        return TRUE;
    }

    if (NWKLookupNodeByShortAddrVal(shortAddr.Val) != INVALID_NEIGHBOR_KEY)
    {
        /*Check whether we have valid IEEE address. This is required because, neighbor table will
        not be updated with valid IEEE address when a beacon is received. During this time, instead
        of leaving the longAddr field with junk values, we always update with all zeros. Having this
        condition, we will not send out any command frame with wrong IEEE address.
        */
        validLongAddr = memcmp( (void *)&currentNeighborRecord.longAddr, (void *)&invalidLongAddr.v[0], 8 );

        if (validLongAddr)
        {
            for (i = 0; i < 8; i++)
            {
                *longAddr++ = currentNeighborRecord.longAddr.v[7-i];
            }
            return TRUE;
        }
    }
    /*Search in the Address Map*/
    for( i = 0; i < apscMaxAddrMapEntries; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            GetAPSAddress( &tmpMap,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
        #else
            GetAPSAddress( &tmpMap,  &apsAddressMap[i] );
        #endif
        if (tmpMap.shortAddr.Val == shortAddr.Val)
        {
            for (i = 0; i < 8; i++)
            {
                *longAddr++ = tmpMap.longAddr.v[7-i];
            }
            return TRUE;
        }
    }

    // ShortAddr information is not exists. So, return FALSE.
    return FALSE;
}

/*********************************************************************
 * Function:        BYTE IsThisLongAddressKnown (LONG_ADDR *longAddr, BYTE * shortAddr, BYTE searchInTable);
 *
 * PreCondition:    None
 *
 * Input:           longAddr                - Long address of node for which
 *                                            a search is made
 *                  shortAddr               - This is an output parameter. If the
 *                                            long address is known, then the
 *                                            corresponding short address will be
 *                                            copied to this parameter.
 *                  searchInTable           - Specifies in which table the extended address
 *                                            needs to be searched.
 *                                            0x01 -> Both the table (Neighbor and Address Map)
 *                                            0x02 -> Only in Neighbor Table
 *                                            0x03 -> Only in Address Map Table
 * Output:          shortAddr will be updated with Network Address of the device for which
 *                  search is made by long adderess.
 *
 * Side Effects:    None.
 *
 * Overview:        Searches neighbor table and Address Map table for
 *                  matching long address
 *
 * Return:          TRUE, if matches and has valid Network Address. FALSE, otherwise.
 *
 * Note:            If long address matches, but the Network Address is invalid, then the
 *                  return value will be FALSE.
********************************************************************/
BYTE IsThisLongAddressKnown (LONG_ADDR *longAddr, BYTE * shortAddr, BYTE searchInTable)
{
    APS_ADDRESS_MAP     tmpMap;
    LONG_ADDR           myLongAddr;
    LONG_ADDR           invalidLongAddr;
    BYTE                validLongAddr;
    BYTE                i;

    //Prepare invalid IEEE Address, i.e 0x00.
    for(i = 0; i < 8; i++)
    {
        invalidLongAddr.v[i] = 0x00;
    }

    //Prepare invalid Network Address, i.e 0xFF not 0x00 because this can be Network Address of ZC
    for(i = 0; i < 2; i++)
    {
        *(shortAddr + i) = 0xFF;
    }

    // Validate whether the longAddr passed has a valid value.
    validLongAddr = memcmp( (void *)longAddr, (void *)&invalidLongAddr.v[0], 8 );
    if ( !validLongAddr )
    {
        // Memory matched, i.e longAddr is invalid.
        return FALSE;
    }

    // Check whether the longAddr is my own address
    GetMACAddress(&myLongAddr);
    validLongAddr = memcmp( (void *)longAddr, (void *)&myLongAddr.v[0], 8 );
    if ( !validLongAddr )
    {
        // Memory matched, i.e longAddr is my own address.
        *(shortAddr) = macPIB.macShortAddress.v[0];
        *(shortAddr + 1) = macPIB.macShortAddress.v[1];
        return TRUE;
    }

    if ( ( searchInTable == 0x01 ) ||
         ( searchInTable == 0x02 ) )
    {
        // Search in Neighbor Table
        if ( NWKLookupNodeByLongAddr( longAddr ) != INVALID_NEIGHBOR_KEY )
        {
            if ( currentNeighborRecord.shortAddr.Val != 0xFFFF )
            {
                // Memory matched, i.e longAddr is in my Neighbor Table.
                *(shortAddr) = currentNeighborRecord.shortAddr.v[0];
                *(shortAddr + 1) = currentNeighborRecord.shortAddr.v[1];
                return TRUE;
            }
        }
    }

    if ( ( searchInTable == 0x01 ) ||
         ( searchInTable == 0x03 ) )
    {
        // Search in the Address Map
        for( i = 0; i < apscMaxAddrMapEntries; i++)
        {
            #ifdef USE_EXTERNAL_NVM
                GetAPSAddress( &tmpMap,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
            #else
                GetAPSAddress( &tmpMap,  &apsAddressMap[i] );
            #endif
            validLongAddr = memcmp( (void *)longAddr, (void *)&tmpMap.longAddr.v[0], 8 );
            if ( !validLongAddr )
            {
                if (tmpMap.shortAddr.Val != 0xFFFF )
                {
                    // Memory matched, i.e longAddr is in my Address Map Table.
                    *(shortAddr) = tmpMap.shortAddr.v[0];
                    *(shortAddr + 1) = tmpMap.shortAddr.v[1];
                    return TRUE;
                }
            }
        }
    }

    // longAddr information does not exists. So, return FALSE.
    return FALSE;
}

/*********************************************************************
 * Function:        static void CreateNwkCommandNwkStatus(BYTE Status, SHORT_ADDR FailedRoutingAddr)
 *
 * PreCondition:    None
 *
 * Input:           Status              - Failure reason
 *                  FailedRoutingAddr   - Failure happened when trying to reach device with this address
 *
 * Output:          TxBuffer will be updated with Nwk Payload for Route Error command frame.
 *
 * Side Effects:    None.
 *
 * Overview:        Creates the Nwk Payload for Route Error command frame
 *
 * Return:          void.
 *
 * Note:            None.
********************************************************************/
#if !defined (I_AM_END_DEVICE)
static void CreateNwkCommandNwkStatus(BYTE Status, SHORT_ADDR FailedRoutingAddr)
{
    // Load the NWK payload into the transmit buffer.
    TxBuffer[TxData++] = NWK_COMMAND_NWK_STATUS;
    TxBuffer[TxData++] = Status;
    TxBuffer[TxData++] = FailedRoutingAddr.byte.LSB;
    TxBuffer[TxData++] = FailedRoutingAddr.byte.MSB;
}
#endif

/*********************************************************************
 * Function:        static void CreateNwkCommandHeader(BYTE SequenceNumber, BYTE Radius,
 *                          SHORT_ADDR nwkSrcAddr, SHORT_ADDR nwkDstAddr)
 *
 * PreCondition:    None
 *
 * Input:           SequenceNumber      - nwkSequenceNumber of the frame
 *                  Radius              - Number of hops the frame can be forwarded
 *                  nwkSrcAddr          - Originator of the frame
 *                  nwkDstAddr          - Final destination address of the frame
 *
 * Output:          TxBuffer will be updated with Nwk Header for any network command frame.
 *
 * Side Effects:    None.
 *
 * Overview:        Creates the Nwk Header for any network command frame
 *
 * Return:          void.
 *
 * Note:            None.
********************************************************************/
static void CreateNwkCommandHeader(BYTE SequenceNumber, BYTE Radius, SHORT_ADDR nwkSrcAddr,
                                SHORT_ADDR nwkDstAddr)
{
    BYTE        includeDstIEEEAddr;

    /*Add the SrcIEEEAddress and DstIEEEAddress*/
    includeDstIEEEAddr = AddNwkHeaderIEEEAddresses(nwkDstAddr);

    TxBuffer[TxHeader--] = SequenceNumber;
    TxBuffer[TxHeader--] = Radius;
    TxBuffer[TxHeader--] = nwkSrcAddr.byte.MSB;
    TxBuffer[TxHeader--] = nwkSrcAddr.byte.LSB;
    TxBuffer[TxHeader--] = nwkDstAddr.byte.MSB;
    TxBuffer[TxHeader--] = nwkDstAddr.byte.LSB;

    /*Update the frame control fields -
    like Security, SrcIEEEAddress, DstIEEEAddress, and ProtocolVerison bits*/
    UpdateNwkHeaderFrameContolFields(includeDstIEEEAddr);
}

/*********************************************************************
 * Function:        NEIGHBOR_KEY NWKLookupNodeByLongAddr(LONG_ADDR *longAddr)
 *
 * PreCondition:    None
 *
 * Input:           longAddr        - Long address of node that is
 *                                    to be looked up in table.
 *
 * Output:          Handle to matching node.
 *
 * Side Effects:    If found, the matching neighbor table entry is in
 *                  currentNeighborRecord and pCurrentNeighborRecord
 *                  points to its location in the table.
 *
 * Overview:        Searches neighbor table for matching short address
 *
 * Note:            If the neighbor table is enlarged, it may be
 *                  necessary to read only the address and device info
 *                  fields until the record is found.
 ********************************************************************/
NEIGHBOR_KEY NWKLookupNodeByLongAddr( LONG_ADDR *longAddr )
{
    BYTE i;

    pCurrentNeighborRecord = neighborTable;

    #ifdef USE_EXTERNAL_NVM
    for ( i=0; i<MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for ( i=0; i<MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        // Read the address into RAM.
        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

        // Make sure that this record is in use
        if ( currentNeighborRecord.deviceInfo.bits.bInUse )
        {

            if ( !memcmp((void*)longAddr,
                        (void*)&currentNeighborRecord.longAddr,
                        (BYTE)(sizeof(LONG_ADDR))) )
            {
                return i;
            }
        }
    }

    return INVALID_NEIGHBOR_KEY;
}

//#ifndef I_AM_COORDINATOR
/*********************************************************************
 * Function:        NEIGHBOR_KEY NWKLookupNodeByExtendedPANId(LONG_ADDR *ExtPANId)
 *
 * PreCondition:    None
 *
 * Input:           longAddr        - Long address of node that is
 *                                    to be looked up in table.
 *
 * Output:          Handle to matching node.
 *
 * Side Effects:    If found, the matching neighbor table entry is in
 *                  currentNeighborRecord and pCurrentNeighborRecord
 *                  points to its location in the table.
 *
 * Overview:        Searches neighbor table for matching short address
 *
 * Note:            If the neighbor table is enlarged, it may be
 *                  necessary to read only the address and device info
 *                  fields until the record is found.
 ********************************************************************/
NEIGHBOR_KEY NWKLookupNodeByExtendedPANId( LONG_ADDR *ExtPANId )
{
    BYTE i;

    pCurrentNeighborRecord = neighborTable;

    #ifdef USE_EXTERNAL_NVM
    for ( i=0; i<MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for ( i=0; i<MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        // Read the address into RAM.
        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

        // Make sure that this record is in use
        if ( currentNeighborRecord.deviceInfo.bits.bInUse )
        {

            if ( !memcmp((void*)ExtPANId,
                        (void*)&currentNeighborRecord.ExtendedPANID ,
                        (BYTE)(sizeof(LONG_ADDR))) )
            {
                return i;
            }
        }
    }

    return INVALID_NEIGHBOR_KEY;
}
//#endif
/*********************************************************************
 * Function:        NEIGHBOR_KEY NWKLookupNodeByShortAddrVal( WORD shortAddrVal )
 *
 * PreCondition:    None
 *
 * Input:           shortAddrVal   - Short address word value of node that is
 *                                    to be looked up in table.
 *
 * Output:          Handle to matching node or INVALID_NEIGHBOR_KEY
 *
 * Side Effects:    If found, the matching neighbor table entry is in
 *                  currentNeighborRecord and pCurrentNeighborRecord
 *                  points to its location in the table.
 *
 * Overview:        Searches neighbor table for matching short address
 *
 * Note:            If the neighbor table is enlarged, it may be
 *                  necessary to read only the address and device info
 *                  fields until the record is found.
 ********************************************************************/
NEIGHBOR_KEY NWKLookupNodeByShortAddrVal( WORD shortAddrVal )
{
    BYTE i;

    pCurrentNeighborRecord = neighborTable;

    #ifdef USE_EXTERNAL_NVM
    for ( i = 0; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for ( i = 0; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        // Read the record into RAM.
        GetNeighborRecord(&currentNeighborRecord, pCurrentNeighborRecord );

        // Make sure that this record is in use
        if ( currentNeighborRecord.deviceInfo.bits.bInUse )
        {

            if ( currentNeighborRecord.shortAddr.Val == shortAddrVal )
            {
                return i;
            }
        }
    }

    return INVALID_NEIGHBOR_KEY;
}

/*********************************************************************
 * Function:        NEIGHBOR_KEY NWKLookupNodeByRxOffDeviceShortAddrVal( WORD shortAddrVal )
 *
 * PreCondition:    None
 *
 * Input:           shortAddrVal   - Short address word value of Rx Off node that is
 *                                    to be looked up in table.
 *
 * Output:          Handle to matching node or INVALID_NEIGHBOR_KEY
 *
 * Side Effects:    If found, the matching neighbor table entry is in
 *                  currentNeighborRecord and pCurrentNeighborRecord
 *                  points to its location in the table.
 *
 * Overview:        Searches neighbor table for matching short address of the Rx Off Node
 *
 * Note:            If the neighbor table is enlarged, it may be
 *                  necessary to read only the address and device info
 *                  fields until the record is found.
 ********************************************************************/
NEIGHBOR_KEY NWKLookupNodeByRxOffDeviceShortAddrVal( WORD shortAddrVal )
{
    BYTE i;

    pCurrentNeighborRecord = neighborTable;

    #ifdef USE_EXTERNAL_NVM
    for ( i = 0; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for ( i = 0; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        // Read the record into RAM.
        GetNeighborRecord(&currentNeighborRecord, pCurrentNeighborRecord );

        // Make sure that this record is in use
        if ( ( currentNeighborRecord.deviceInfo.bits.bInUse ) &&
             ( currentNeighborRecord.shortAddr.Val == shortAddrVal ) &&
             ( currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_END_DEVICE ) &&
             ( currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle == 0 ) )
        {
            return i;
        }
    }

    return INVALID_NEIGHBOR_KEY;
}

NEIGHBOR_KEY NWKFindRandomNeighbor( BYTE NBTStartIndex, SHORT_ADDR shortAddressToBeIgnored1, SHORT_ADDR shortAddressToBeIgnored2 )
{
   // BYTE i;

    pCurrentNeighborRecord = neighborTable;

    #ifdef USE_EXTERNAL_NVM
    for ( ; NBTStartIndex < MAX_NEIGHBORS; NBTStartIndex++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for ( ; NBTStartIndex < MAX_NEIGHBORS; NBTStartIndex++, pCurrentNeighborRecord++)
    #endif
    {
        // Read the record into RAM.
        GetNeighborRecord(&currentNeighborRecord, pCurrentNeighborRecord );

        // Make sure that this record is in use
        if ( currentNeighborRecord.deviceInfo.bits.bInUse )
        {
            if (( currentNeighborRecord.shortAddr.Val != INVALID_SHORT_ADDRESS ) &&
                ( currentNeighborRecord.shortAddr.Val != shortAddressToBeIgnored1.Val )&&
                ( currentNeighborRecord.shortAddr.Val != shortAddressToBeIgnored2.Val ))
            {
                return NBTStartIndex;
            }
        }
    }

    return INVALID_NEIGHBOR_KEY;
}
NEIGHBOR_KEY NWKLookupNodeByShortAddrValAndPANID( WORD shortAddrVal, WORD PANIDVal )
{
    BYTE i;

    pCurrentNeighborRecord = neighborTable;

    #ifdef USE_EXTERNAL_NVM
    for ( i = 0; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for ( i = 0; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        // Read the record into RAM.
        GetNeighborRecord(&currentNeighborRecord, pCurrentNeighborRecord );

        // Make sure that this record is in use
        if ( currentNeighborRecord.deviceInfo.bits.bInUse )
        {
            if ( currentNeighborRecord.shortAddr.Val == shortAddrVal &&
                currentNeighborRecord.panID.Val == PANIDVal)
            {
                return i;
            }
        }
    }

    return INVALID_NEIGHBOR_KEY;
}

/*********************************************************************
 * Function:        BOOL NWKThisIsMyLongAddress( LONG_ADDR *address )
 *
 * PreCondition:    None
 *
 * Input:           address - pointer to a long address
 *
 * Output:          TRUE - values at address match my long address
 *                  FALSE - no match
 *
 * Side Effects:    None
 *
 * Overview:        This routine determines if a long address is my
 *                  long address.
 *
 * Note:            None
 ********************************************************************/

BOOL NWKThisIsMyLongAddress( LONG_ADDR *address )
{
    LONG_ADDR   tempMACAddr;

    GetMACAddress( &tempMACAddr );

    if (!memcmp( (void *)address, (void *)&tempMACAddr, 8 ))
        return TRUE;
    else
        return FALSE;
}

/*********************************************************************
 * Function:        void NWKTidyNeighborTable( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Removes neighbor table entries that does not belong to current operating network.
 *
 * Note:            None
 ********************************************************************/
void NWKTidyNeighborTable( void )
{
    BYTE i;
    BYTE ExtPANIdNotMatched;

    #ifdef USE_EXTERNAL_NVM
    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
        //if (currentNeighborRecord.deviceInfo.bits.bInUse &&
        //    ((currentNeighborRecord.deviceInfo.bits.Relationship != NEIGHBOR_IS_CHILD) &&
        //     (currentNeighborRecord.deviceInfo.bits.Relationship != NEIGHBOR_IS_PARENT)))
        ExtPANIdNotMatched = memcmp( (BYTE *)(&currentNeighborTableInfo.nwkExtendedPANID.v[0]),
                                     (BYTE *)(&currentNeighborRecord.ExtendedPANID.v[0]),
                                     EXTENDED_PAN_SIZE );
        if ( ( currentNeighborRecord.deviceInfo.bits.bInUse ) &&
            (( currentNeighborRecord.panID.Val != macPIB.macPANId.Val ) ||  // Not my PANId
             ( currentNeighborRecord.LogicalChannel != phyPIB.phyCurrentChannel ) || // Not my channel
             ( currentNeighborRecord.nwkUpdateId != currentNeighborTableInfo.nwkUpdateId ) ||  // Not my UpdateId
             ( ExtPANIdNotMatched ))) // Not my ExtendedPANId
        {
            // This record isn't a child or parent, so remove it.
            currentNeighborRecord.deviceInfo.Val = 0;
            currentNeighborTableInfo.neighborTableSize--;
            PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
        }
    }

    PutNeighborTableInfo();
}

/*********************************************************************
 * Function:        void Prepare_MCPS_DATA_request( WORD macDestAddressVal, BYTE *msduHandle )
 *
 * PreCondition:    None
 *
 * Input:           macDestAddressVal - the MAC destination address of
 *                          the message as a WORD to allow for constants
 *
 * Output:          MCPS_DATA_request parameters are loaded.
 *
 * Side Effects:    None
 *
 * Overview:        This routine loads all of the MCPS_DATA_request
 *                  parameters based on the input MAC address.
 *
 * Note:            None
 ********************************************************************/
void Prepare_MCPS_DATA_request( WORD macDestAddressVal, BYTE *msduHandle )
{
    // Block the transmit path
    ZigBeeBlockTx();

    params.MCPS_DATA_request.SrcAddrMode = 0x02;
    params.MCPS_DATA_request.SrcPANId = macPIB.macPANId;
    params.MCPS_DATA_request.SrcAddr.ShortAddr.Val = macPIB.macShortAddress.Val;
    params.MCPS_DATA_request.DstAddrMode = 0x02;
    params.MCPS_DATA_request.DstPANId = macPIB.macPANId;
    params.MCPS_DATA_request.DstAddr.ShortAddr.Val = macDestAddressVal;
    params.MCPS_DATA_request.msduHandle = *msduHandle = MLME_GET_macDSN();
    params.MCPS_DATA_request.TxOptions.Val = 0x00;


    if (macDestAddressVal != 0xFFFF)
    {
        params.MCPS_DATA_request.TxOptions.bits.acknowledged_transmission = 1;
        if (NWKLookupNodeByShortAddrVal( macDestAddressVal ) != INVALID_NEIGHBOR_KEY)
        {
            if (!currentNeighborRecord.deviceInfo.bits.RxOnWhenIdle)
            {
                params.MCPS_DATA_request.TxOptions.bits.indirect_transmission = 1;
            }
        }
#ifdef I_SUPPORT_SECURITY
     // this request is not originated from MAC layer, so no security requirement
     params.MCPS_DATA_request.TxOptions.bits.security_enabled_transmission = FALSE;
#endif
        // NOTE: params.MCPS_DATA_request.TxOptions.bits.security_enabled_transmission = ???
    }
    params.MCPS_DATA_request.frameType = FRAME_DATA;

}

/*********************************************************************
 * Function:        void RemoveNeighborTableEntry( void )
 *
 * PreCondition:    currentNeighborRecord and pCurrentNeighborRecord
 *                  are set to the record to be removed, and the
 *                  record is in use (bInUse == 1)
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    Beacon payload is updated.
 *
 * Overview:        Removes the current node from the neighbor table.
 *
 * Note:            None
 ********************************************************************/
void RemoveNeighborTableEntry( void )
{
    #ifndef I_AM_END_DEVICE
    if (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD)
    {
        currentNeighborTableInfo.numChildren--;
        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
            if (currentNeighborRecord.deviceInfo.bits.deviceType == DEVICE_ZIGBEE_ROUTER)
            {
                currentNeighborTableInfo.numChildRouters--;
            }
        #endif
        SetBeaconPayload(TRUE);
    }
    else
    #endif
    if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
        if (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT)
        {
            // My parent is no longer on the network.

            currentNeighborTableInfo.parentNeighborTableIndex = INVALID_NEIGHBOR_KEY;
        }
    }//#endif

    // Remove the neighbor record by clearing the bInUse flag.
    currentNeighborRecord.deviceInfo.bits.bInUse = 0;
    currentNeighborRecord.deviceInfo.Val = 0;
    PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );

    // Update the table size.
    currentNeighborTableInfo.neighborTableSize--;

    // Commit all neighbor table info changes, from here and from the caller.
    PutNeighborTableInfo();
}

/*********************************************************************
 * Function:        void RemoveRoutingTableEntry( SHORT_ADDR destAddr )
 *
 * PreCondition:    None
 *
 * Input:           Short Address for which the Routing Table entry exists
 *
 * Output:          Routing Table entry will be updated
 *
 * Side Effects:    None
 *
 * Overview:        Removes the Routing Table entry corressponding to the destAddr
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
void RemoveRoutingTableEntry( SHORT_ADDR destAddr )
{
    BYTE                i;

    #ifdef USE_EXTERNAL_NVM
    for(i=0, pCurrentRoutingEntry = routingTable;
        i<ROUTING_TABLE_SIZE;
        i++, pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY))
    #else
    for(i=0, pCurrentRoutingEntry = routingTable;
        i<ROUTING_TABLE_SIZE;
        i++, pCurrentRoutingEntry++)
    #endif
    {
        GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
        if (currentRoutingEntry.destAddress.Val == destAddr.Val)
        {
            ClearRoutingTableEntry( i );
            break; //Only one entry will be there per destination device. So no need to continue looping.
        }
    }
}
#endif

/*********************************************************************
 * Function:        void AgeOutRoutingTableEntries( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Routing Table entries which are aged out will be removed
 *
 * Side Effects:    None
 *
 * Overview:        Routing Table entries which are aged out will be removed
 *
 * Note:            None
 ********************************************************************/
#if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
void AgeOutRoutingTableEntries( void )
{
    BYTE                i;

    #ifdef USE_EXTERNAL_NVM
    for(i=0, pCurrentRoutingEntry = routingTable;
        i<ROUTING_TABLE_SIZE;
        i++, pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY))
    #else
    for(i=0, pCurrentRoutingEntry = routingTable;
        i<ROUTING_TABLE_SIZE;
        i++, pCurrentRoutingEntry++)
    #endif
    {
        GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
        if (
             ( currentRoutingEntry.rtflags.rtbits.bStatus == ROUTE_ACTIVE )
             #ifdef I_SUPPORT_SECURITY
                && ( currentRoutingEntry.destAddress.Val != 0x0000 ) // We should never age out TC entry
             #endif
           )
        {
            if ( currentRoutingEntry.lastModified == ROUTING_TABLE_AGE_LIMIT )
            {
                ClearRoutingTableEntry( i );
            }
            else
            {
                currentRoutingEntry.lastModified++;
                PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
            }
        }
    }
}
#endif

/*********************************************************************
 * Function:        BYTE ClearRoutingTableEntry( BYTE rtIndex )
 *
 * PreCondition:    None
 *
 * Input:           rtIndex - Specifies the index at which Routing Entry needs to be resetted
 *
 * Output:          Routing Table entry specified by rtIndex will be cleared with default values
 *
 * Side Effects:    None
 *
 * Overview:        Routing Table entry specified by rtIndex will be cleared with default values
 *
 * Note:            Returns TRUE if Routing entry is cleared, FALSE, otherwise.
 ********************************************************************/
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
BYTE ClearRoutingTableEntry( BYTE rtIndex )
{
    if ( rtIndex == 0xff )
    {
        return FALSE;
    }
    if ( rtIndex >= ROUTING_TABLE_SIZE )
    {
        return FALSE;
    }
    #ifdef USE_EXTERNAL_NVM
        pCurrentRoutingEntry = routingTable + ( rtIndex * (WORD)sizeof(ROUTING_ENTRY) );
    #else
        pCurrentRoutingEntry = routingTable + ( rtIndex );
    #endif

    GetRoutingEntry( &currentRoutingEntry, pCurrentRoutingEntry );
    // Mark the record as inactive and available for reuse.
    currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_INACTIVE;
    currentRoutingEntry.destAddress.Val = 0xFFFF;
    #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
        currentRoutingEntry.lastModified = ROUTING_TABLE_AGE_RESET;
    #endif
    PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );

    return TRUE;

}
#endif

/*********************************************************************
 * Function:        BOOL RequestedPANIDFound( BYTE channel )
 *
 * PreCondition:    params.MLME_START_request.PANId must have the PAN ID
 *                  being checked for a conflict
 *
 * Input:           channel - the channel to check for PAN ID conflicts
 *
 * Output:          TRUE - conflict found
 *                  FALSE - no conflict found
 *
 * Side Effects:    None
 *
 * Overview:        This function determines if a node in our neighbor
 *                  table on the specified channel has the same PAN ID
 *                  as the one requested.
 *
 * Note:            This routine may not work if the destination
 *                  is our child.
 ********************************************************************/
//#ifdef I_AM_COORDINATOR
BOOL RequestedPANIDFound( BYTE channel )
{
    BYTE    i;

    pCurrentNeighborRecord = neighborTable;

    #ifdef USE_EXTERNAL_NVM
    for ( i=0; i<MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for ( i=0; i<MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        // Read the address into RAM.
        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );

        // Make sure that this record is in use
        if (currentNeighborRecord.deviceInfo.bits.bInUse)
        {
            if ((currentNeighborRecord.LogicalChannel == channel) &&
                (currentNeighborRecord.panID.Val == params.MLME_START_request.PANId.Val))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
//#endif

/*********************************************************************
 * Function:        BOOL RouteAlongTree( SHORT_ADDR destTarget, SHORT_ADDR *dest )
 *
 * PreCondition:    The destination must not be our child (?).
 *
 * Input:           destTarget - final target destination
 *                  dest - pointer to calculated destination address
 *
 * Output:          TRUE - valid destination address found
 *                  FALSE - could not find valid destination address
 *
 * Side Effects:    None
 *
 * Overview:        This function determines the address of the
 *                  next node to send a routing message to based on
 *                  tree hierarchy.  We do not check the path of the
 *                  message, only the destination address.  If the
 *                  message is going to one of our descendants, pass
 *                  it to the next level parent.  Otherwise, pass it
 *                  to our parent.  If we are a coordinator and we
 *                  cannot find the next level parent, return FALSE.
 *
 * Note:            This routine may not work if the destination
 *                  is our child.
 ********************************************************************/
#ifdef I_SUPPORT_ROUTING
#if defined(USE_TREE_ROUTING_ONLY) || (USE_TREE_ROUTING == 0x01)
BOOL RouteAlongTree( SHORT_ADDR destTarget, SHORT_ADDR *destNextHop )
{
    #ifdef I_AM_END_DEVICE

        // If I am an FFD end device, I have no children, so send the message to my parent.
        *destNextHop = macPIB.macCoordShortAddress;
        return TRUE;

    #else

        BOOL found;
        BYTE i;

        // Find which of my child nodes is the parent of the destination.
        found = FALSE;
        // Get ROM address of neighborTable in RAM.
        pCurrentNeighborRecord = neighborTable;

        for ( i=0; i < MAX_NEIGHBORS; i++ )
        {
            GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
            if ((currentNeighborRecord.deviceInfo.bits.bInUse) &&
                (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD))
            {
                if ((destTarget.Val == currentNeighborRecord.shortAddr.Val) ||
                     IsDescendant( currentNeighborRecord.shortAddr, destTarget, currentNeighborTableInfo.depth+1 ))
                {
                    // The destination address is either my child or a
                    // descendant of one of my children.  Pass the message
                    // to that child.
                    *destNextHop = currentNeighborRecord.shortAddr;
                    found = TRUE;
                    break;
                }
            }
            #ifdef USE_EXTERNAL_NVM
                pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD);
            #else
                pCurrentNeighborRecord++;
            #endif
        }

        if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
            // If none of my child nodes is the parent and the destination
            // should not be one of my descendants, so send the message to
            // my parent.
            if (!found && !IsDescendant( macPIB.macShortAddress, destTarget, currentNeighborTableInfo.depth ))
            {
                *destNextHop = macPIB.macCoordShortAddress;
                found = TRUE;
            }
        }//#endif

        return found;

    #endif
}
#endif
#endif

/*********************************************************************
 * Function:        void SetBeaconPayload( BOOL )
 *
 * PreCondition:    None
 *
 * Input:           none
 *
 * Output:          none
 *
 * Side Effects:    Beacon Payload in MAC PIB is updated.
 *
 * Overview:        Sets the beacon payload, particularly the indications
 *                  as to whether or not end devices and routers can
 *                  join to this node.
 *
 * Note:            None
 ********************************************************************/
#ifndef I_AM_END_DEVICE
void SetBeaconPayload( BOOL updatepayload )
{
    BYTE i;
    /* the macPIB.permitassociation bit already in place, don't update router/end device capacity fields */
    if(!updatepayload)
    {
        return;
    }
    macPIB.macBeaconPayload[0] = ZIGBEE_PROTOCOL_ID;
    macPIB.macBeaconPayload[1] = (nwkcProtocolVersion  << 4) | PROFILE_myStackProfileId;
    macPIB.macBeaconPayload[2] = currentNeighborTableInfo.depth << 3;

    if ((currentNeighborTableInfo.numChildren < NIB_nwkMaxChildren) &&
        (currentNeighborTableInfo.neighborTableSize < MAX_NEIGHBORS)&&
        (currentNeighborTableInfo.depth < NIB_nwkMaxDepth))
    {
        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
            if (!nwkStatus.flags.bits.bAllEndDeviceAddressesUsed)
            {
                macPIB.macBeaconPayload[2] |= 0x80; // End Devices can join
            }
        /* A ZigBee 2006 requirement: Routers/FFD are allowed to join at MaxDepth
        * see mandatory test 3.14 for the expected behavior for verison 2006
        */
            if ((currentNeighborTableInfo.numChildRouters < NIB_nwkMaxRouters) &&
                !nwkStatus.flags.bits.bAllRouterAddressesUsed )
            {
                macPIB.macBeaconPayload[2] |= 0x04; // Routers can join
            }
        #else
            macPIB.macBeaconPayload[2] |= (0x80 | 0x04);  // Any Devices can join
        #endif
    }

    for (i = 0; i < 8; i++)
    {
             macPIB.macBeaconPayload[i + EXTENDED_PANID_OFFSET] = currentNeighborTableInfo.nwkExtendedPANID.v[i];
    }

    /* Tx offset on beacon payload */
    macPIB.macBeaconPayload[TX_OPTIONS_OFFSET] = 0xFF;
    macPIB.macBeaconPayload[TX_OPTIONS_OFFSET + 1] = 0xFF;
    macPIB.macBeaconPayload[TX_OPTIONS_OFFSET + 2] = 0xFF;

    /* nwkUpdateId added onto beacon payload */
    macPIB.macBeaconPayload[NWK_UPDATE_ID_OFFSET] = currentNeighborTableInfo.nwkUpdateId;
}
#endif
/*********************************************************************
 * Function:        void VariableAndTableInitialization( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    All NWK layer variables and tables are
 *                  initialized except the neighbor table
 *
 * Overview:        This routine initializes all variables and constants
 *                  except the neighbor table.
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/
void VariableAndTableInitialization( BOOL force )
{
    BYTE    i;

    // Reset simple variables
    NIB.nwkBCSN = RANDOM_LSB;

    ZigBeeStatus.flags.Val = 0;
    // A ZigBee 2006 requierement:
    i = nwkStatus.flags.bits.bNRejoin;
    nwkStatus.flags.Val = 0;
    nwkStatus.flags.bits.bNRejoin = i;

    #ifndef NIB_STATIC_IMPLEMENTATION
        NIB.nwkPassiveAckTimeout                = DEFAULT_nwkPassiveAckTimeout;
        NIB.nwkMaxBroadcastRetries              = DEFAULT_nwkMaxBroadcastRetries;
        NIB.nwkMaxChildren                      = PROFILE_nwkMaxChildren;
        NIB.nwkMaxDepth                         = PROFILE_nwkMaxDepth;
        NIB.nwkMaxRouters                       = PROFILE_nwkMaxRouters;
        NIB.nwkNeighborTable                    = DEFAULT_nwkNeighborTable;
        NIB.nwkNetworkBroadcastDeliveryTime     = DEFAULT_nwkNetworkBroadcastDeliveryTime;
        NIB.nwkReportConstantCost               = PROFILE_nwkReportConstantCost;
        NIB.nwkRouteDiscoveryRetriesPermitted   = DEFAULT_nwkRouteDiscoveryRetriesPermitted;
        NIB.nwkRouteTable                       = DEFAULT_nwkRouteTable;
        NIB.nwkSymLink                          = PROFILE_nwkSymLink;
        NIB.nwkCapabilityInformation            = DEFAULT_nwkCapabilityInformation;
        NIB.nwkUseTreeAddrAlloc                 = DEFAULT_nwkUseTreeAddrAlloc;
        NIB.nwkUseTreeRouting                   = DEFAULT_nwkUseTreeRouting;
    #endif
     #if I_SUPPORT_CONCENTRATOR ==1
        for( i=0; i < MAX_ROUTE_RECORD_TABLE_SIZE; i++)
        {
            RouteRecordTable[i].destinationAddress.Val = INVALID_SHORT_ADDRESS;
        }
    #endif
    #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
        DataToBeSendAfterRouteRecord.DataBackUp = NULL;
    #endif
    #ifdef I_SUPPORT_SECURITY
        nwkSecurityLevel = PROFILE_nwkSecurityLevel;
        securityStatus.flags.bits.nwkSecureAllFrames = PROFILE_nwkSecureAllFrames;
        securityStatus.flags.bits.nwkAllFresh = DEFAULT_nwkAllFresh;
    #endif

    #if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
    #ifndef I_AM_END_DEVICE
        nwkStatus.PanIDReport.noOfRecords = 0;
        nwkStatus.PanIDReport.index = 0;
    #endif
    nwkStatus.panIDUpdate = NULL;
    panIDConflictStatus.flags.Val = 0;
    activatePANIDConflict = 0x01;
    #endif

    #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
        activateAddConfDetection = 0x01;
    #endif

    // Initialize NWK frame handles awaiting confirmation.
    for (i=0; i<MAX_NWK_FRAMES; i++)
    {
        nwkConfirmationHandles[i].nsduHandle = INVALID_NWK_HANDLE;
    }

    if (!force)
    {
        if (nwkStatus.discoveryInfo.channelList)
        {
            SRAMfree( (unsigned char *)nwkStatus.discoveryInfo.channelList );
        }
    }
    nwkStatus.discoveryInfo.channelList = NULL;

    if (NOW_I_AM_NOT_A_RFD()){//#ifndef I_AM_RFD
        // Initialize the Broadcast Transaction Table pointers.
        for (i=0; i<NUM_BUFFERED_BROADCAST_MESSAGES; i++)
        {
            if (!force)
            {
                if (nwkStatus.BTT[i])
                {
                    SRAMfree( nwkStatus.BTT[i]->dataPtr );
                    SRAMfree( (unsigned char *)nwkStatus.BTT[i] );
                }
            }
            nwkStatus.BTT[i] = NULL;
        }
    }//#endif

    #if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
    {
        // Initialize the buffered messages awaiting routing.
        for (i=0; i<NUM_BUFFERED_ROUTING_MESSAGES; i++)
        {
            if (!force)
            {
                if (nwkStatus.routingMessages[i])
                {
                    SRAMfree( nwkStatus.routingMessages[i]->dataPtr );
                    SRAMfree( (unsigned char *)nwkStatus.routingMessages[i] );
                }
            }
            nwkStatus.routingMessages[i] = NULL;
        }

        // Initialize the Route Discovery Table.
        for (i=0; i<ROUTE_DISCOVERY_TABLE_SIZE; i++)
        {
            if (!force)
            {
                if (routeDiscoveryTablePointer[i])
                {
                    SRAMfree( (unsigned char *)routeDiscoveryTablePointer[i] );
                }
            }
            routeDiscoveryTablePointer[i] = NULL;
        }

        // Clear the Routing Table.
        NWKClearRoutingTable();

    }
    #endif

    #ifdef I_SUPPORT_SECURITY
        #ifdef I_SUPPORT_RES_SECURITY
            GetNeighborTableInfo();
            if(!(currentNeighborTableInfo.nwkExtendedPANID.v[0]|
                 currentNeighborTableInfo.nwkExtendedPANID.v[1]|
                 currentNeighborTableInfo.nwkExtendedPANID.v[2]|
                 currentNeighborTableInfo.nwkExtendedPANID.v[3]|
                 currentNeighborTableInfo.nwkExtendedPANID.v[4]|
                 currentNeighborTableInfo.nwkExtendedPANID.v[5]|
                 currentNeighborTableInfo.nwkExtendedPANID.v[6]|
                 currentNeighborTableInfo.nwkExtendedPANID.v[7]) )
            {
                clearNWKKEy();
            }


        #endif

    #endif

    #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
    SetMulticastParams (DEFAULT_nwkUseMulticast, DEFAULT_nwkMulticastNonMemberRadius, DEFAULT_nwkMulticastMaxNonMemberRadius);
    #endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
}


#ifdef I_SUPPORT_SECURITY
    void clearNWKKEy(void)
    {

        #if defined(USE_EXTERNAL_NVM)
            InitSecurityKey();
        #endif

        BYTE key[16];
        BYTE i;

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
        i = 1;
        PutNwkActiveKeyNumber(&i);

        /*BYTE ActiveKeyIndex;
        BYTE i;
        GetNwkActiveKeyNumber(&ActiveKeyIndex);*/

        /*if( ActiveKeyIndex == 0x01 || ActiveKeyIndex == 0x02 )
        {
            #ifdef USE_EXTERNAL_NVM
            GetNwkKeyInfo( &currentNetworkKeyInfo, networkKeyInfo + (ActiveKeyIndex-1) * sizeof(NETWORK_KEY_INFO) );
            #else
            GetNwkKeyInfo( &currentNetworkKeyInfo, &(networkKeyInfo[ActiveKeyIndex-1]) );
            #endif
            if( currentNetworkKeyInfo.SeqNumber.v[1] == nwkMAGICResSeq )
            {
                #if defined(USE_EXTERNAL_NVM)
                InitSecurityKey();
                #endif
                goto InitializeFrame;
            }
            else
            {
                ActiveKeyIndex = nwkMAGICResSeq;
                PutNwkActiveKeyNumber(&ActiveKeyIndex);
            }
        }   */
        /*else
        {
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
                i = 1;
                PutNwkActiveKeyNumber(&i);
            #endif          // end NETWORK_KEY_BYTE00


        }  */

//InitializeFrame:
        OutgoingFrameCount[0].Val = 0;
        OutgoingFrameCount[1].Val = 0;
        for(i = 0; i < MAX_NEIGHBORS; i++)
        {
            IncomingFrameCount[0][i].Val = 0;
            IncomingFrameCount[1][i].Val = 0;
        }

    }
#endif

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
/*********************************************************************
 * Function:        WORD GenerateNewShortAddr( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          A new 16 bit random number
 *
 * Side Effects:
 *
 * Overview:        This routine generate a 16 bit random number
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/
WORD GenerateNewShortAddr( void )
{
    SHORT_ADDR newAddress;
    LONG_ADDR temp;
    while(1)
    {
        newAddress.Val = RANDOM_16BIT;
        if ( ( newAddress.Val > 0x0000 ) && ( newAddress.Val < 0xFFF7 ) )
        {
            if ( !IsThisAddressKnown( newAddress , &temp.v[0] ) )
            {
                break;
            }
        }
    }
   return (newAddress.Val);
}
#ifndef I_AM_END_DEVICE
/*********************************************************************
 * Function:        BOOL IsThereAddressConflict( SHORT_ADDR RemoteAddr, LONG_ADDR *pRemoteLongAddr )
 *
 * PreCondition:    None
 *
 * Input:           RemoteAddr , pRemoteLongAddr
 *
 * Output:          TRUE/FALSE
 *
 * Side Effects:
 *
 * Overview:        Returns a TRUE if found a address conflict on RemoteAddr.
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/
BOOL IsThereAddressConflict( SHORT_ADDR remoteShortAddr , LONG_ADDR *pRemoteLongAddr )
{
    LONG_ADDR tempLongAddr;

    if (activateAddConfDetection != 0x01)
    {
       nwkStatus.addressConflictType = NO_CONFLICT;
       return FALSE;
    }

    if ( IsThisAddressKnown( remoteShortAddr , &tempLongAddr.v[0] ) )
    {
        //The "remoteShortAddr" is known to me. Lets compare the "RemoteLongAddr" with "tempLongAddr" to find out address conflict
        BYTE i;
        /* we got tempLongAddr in reverse order. So Compare in reverse order*/
        for ( i = 0 ; i < 8 ; i++ )
        {
            if( tempLongAddr.v[i] != pRemoteLongAddr->v[7-i] )
                break;  // break if one of bytes in "RemoteLongAddr" with "tempLongAddr" mismatched
        }
        if ( i == 8 )
        {
           // All 8 bytes in "RemoteLongAddr" with "tempLongAddr" are same.
           nwkStatus.addressConflictType = NO_CONFLICT;
           return FALSE;
        }
        else
        {
            // We have detected address conflict as "RemoteLongAddr" and "tempLongAddr" are different
            // Now See what kind of conflict it is
            params.NLME_NWK_STATUS_indication.nwkStatusCode = NWK_STATUS_ADDRESS_CONFLICT;
            params.NLME_NWK_STATUS_indication.ShortAddress = remoteShortAddr;
            if ( remoteShortAddr.Val == macPIB.macShortAddress.Val )
            {
                // remoteShortAddr is my own short address, so its a local address conflcit
                if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
                   nwkStatus.addressConflictType = LOCAL_ADDRESS_CONFLICT;
                }else{//#else
                   nwkStatus.addressConflictType = REMOTE_ADDRESS_CONFLICT; // As I am coordiantor, I shall not change my address
                }//#endif
            }
            else if ( IS_IT_A_ADDR_CONFLICT_ON_CHILD_END_DEV( remoteShortAddr ) )
            {
                // remoteShortAddr belongs to one of my child end devices
                nwkStatus.addressConflictType = ADDRESS_CONFLICT_ON_A_CHILD_END_DEVICE;
            }
            else
            {
                // remoteShortAddr does not belong to me or one of my child end devices
                nwkStatus.addressConflictType = REMOTE_ADDRESS_CONFLICT;
            }
            return TRUE;
        }
    }
    nwkStatus.addressConflictType = NO_CONFLICT;
    return FALSE;
}

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE PrepareUnsolicitedRejoinRsp( SHORT_ADDR shortAddress )
 *
 * PreCondition:    None
 *
 * Input:           SHORT_ADDR
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        Sends Unsolicited rejoin command.
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/

ZIGBEE_PRIMITIVE PrepareUnsolicitedRejoinRsp( SHORT_ADDR shortAddress )
{
    BYTE i , *pData;

    pData = &TxBuffer[TxData];

    ZigBeeBlockTx();
    *pData++ = NWK_COMMAND_REJOIN_RESPONSE;
    *pData++ = currentNeighborTableInfo.nextChildAddr.v[0];
    *pData++ = currentNeighborTableInfo.nextChildAddr.v[1];

    *pData++ = SUCCESS;

    CreateNwkCommandHeader
    (
        NLME_GET_nwkBCSN(),
        1,
        macPIB.macShortAddress,
        shortAddress
    );

    /* Send it to old short address of requesting device.  See Specs */
    Prepare_MCPS_DATA_request((WORD )shortAddress.Val, &i );

    TxData +=  pData - &TxBuffer[TxData];

    if ( NWKLookupNodeByShortAddrVal(shortAddress.Val) != INVALID_NEIGHBOR_KEY )
    {
        // NWKLookupNodeByShortAddrVal called here just to get correct record in
        //currentNeighborRecord. The above if condition must always be true as device "shortAddress" is my child
        currentNeighborRecord.shortAddr.Val = currentNeighborTableInfo.nextChildAddr.Val;
        PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
    }
    nwkStatus.flags.bits.bNextChildAddressUsed = TRUE;
    return MCPS_DATA_request;
}

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE PrepareNwkStatusCmdBroadcast( BYTE statusCode , SHORT_ADDR shortAddress )
 *
 * PreCondition:    None
 *
 * Input:           SHORT_ADDR
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        Resolves Remote Address conflict
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/

ZIGBEE_PRIMITIVE PrepareNwkStatusCmdBroadcast( BYTE statusCode , SHORT_ADDR shortAddress )
{
    BYTE BTTIndex;
    LONG_ADDR           tmpLongAddr;
    BYTE *pData;
    BTR *pCurrent_BTR;

    if (!CreateNewBTR( &BTTIndex ))
    {

       return NO_PRIMITIVE;
    }

     pCurrent_BTR = nwkStatus.BTT[BTTIndex];  //Code optimisation
     // Indicate that the message was not from our upper layers.
     pCurrent_BTR->btrInfo.bMessageFromUpperLayers = 0;

     // Set the jitter time so it will expire and transmit right away.
     pCurrent_BTR->broadcastJitterTimer = TickGet();

     // Save off the NWK header information
     pCurrent_BTR->dataLength                   = 0x04;
     pCurrent_BTR->nwkFrameControlLSB.Val       = NWK_FRAME_CMD | (nwkProtocolVersion<<2) | (ROUTE_DISCOVERY_SUPPRESS<<6);
#ifdef I_SUPPORT_SECURITY
     pCurrent_BTR->nwkFrameControlMSB.Val       = ( TRUE << 1 ) | NWK_IEEE_SRC_ADDR;
#else
     pCurrent_BTR->nwkFrameControlMSB.Val       = ( FALSE << 1 ) | NWK_IEEE_SRC_ADDR;
#endif
     pCurrent_BTR->nwkDestinationAddress.Val    = 0xFFFD;
     pCurrent_BTR->nwkSourceAddress             = macPIB.macShortAddress;    //params.NLDE_DATA_request.SrcAddr;
     pCurrent_BTR->nwkRadius                    = DEFAULT_RADIUS;
     pCurrent_BTR->nwkSequenceNumber            = NLME_GET_nwkBCSN();

     GetMACAddress(&tmpLongAddr);
/*     for (i = 0; i < 8; i++)
     {
         *longAddr++ = tmpLongAddr.v[7-i];
     }*/
     pCurrent_BTR->nwkIEEESrcAddress            = tmpLongAddr;

     // Allocate space to save off the message
     if ((pCurrent_BTR->dataPtr = (BYTE *)SRAMalloc( 0x04 )) == NULL)
     {

        // We do not have room to store this broadcast packet.  Destroy the BTR.
        nfree( nwkStatus.BTT[BTTIndex] );


        return NO_PRIMITIVE;
    }

    // Save off the NWK Cmd payload
    pData = pCurrent_BTR->dataPtr;
    // Copy the Cmd Payload
    *pData++ = NWK_COMMAND_NWK_STATUS;
    *pData++ = statusCode;
    *pData++ = shortAddress.byte.LSB;
    *pData++ = shortAddress.byte.MSB;

    #ifdef I_SUPPORT_SECURITY
    pCurrent_BTR->btrInfo.bAlreadySecured = 0x00;
    #endif

    // Set the network status so we can begin transmitting these messages
    // in the background.
    nwkStatus.flags.bits.bSendingBroadcastMessage = 1;

    // The message is buffered, so unblock Tx.
    ZigBeeUnblockTx();

    return NO_PRIMITIVE;
}
#endif
#endif
/*********************************************************************
 * Function:       CheckAndUpdateRouteTableWithConcentratorDetails
 *
 * PreCondition:    Route request is received
 *
 * Input:           rreq - pointer to rreq payload received
 *                  prouteDiscoveryTable - pointer to RDT table where current RREQ is updated
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       This function updates the routing table with concentrator
 * details if the received rreq is a route request
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/
 #if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
static void CheckAndUpdateRouteTableWithConcentratorDetails(ROUTE_REQUEST_COMMAND   *rreq,
    ROUTE_DISCOVERY_ENTRY   *prouteDiscoveryTable )
{
    if (( rreq->commandOptions & HIGH_CONC_MANY_TO_ONE ) ||
    ( rreq->commandOptions & LOW_CONC_MANY_TO_ONE ))
    {

        if( FindFreeRoutingTableEntry( prouteDiscoveryTable->srcAddress, TRUE) != 0xFF )
        {
            currentRoutingEntry.rtflags.rtbits.bStatus = ROUTE_ACTIVE;
            currentRoutingEntry.rtflags.rtbits.bNoRouteCache =  (( rreq->commandOptions & LOW_CONC_MANY_TO_ONE ) ? (1):(0));

            currentRoutingEntry.rtflags.rtbits.bManyToOne = TRUE;
            currentRoutingEntry.rtflags.rtbits.bRouteRecordRequired = TRUE;

            currentRoutingEntry.nextHop.Val = params.MCPS_DATA_indication.SrcAddr.ShortAddr.Val;
            PutRoutingEntry( pCurrentRoutingEntry, &currentRoutingEntry );
        }
    }

}
#endif
/*********************************************************************
 * Function:      BYTE *CreateSourceRouteSubframe( BYTE Index, WORD *Hops,
 *                  SHORT_ADDR *macDestinationAddress )
 *
 * PreCondition:    None
 *
 * Input:           Index: The route record table index of destination device
 *                  Hops
 *
 * Output:          Hops: The number of hops present in the source route subframe.
 *                  macDestinationAddress: The next hop address to which source route
 *                  subframe has to be sent.
 *                  Returns a pointer to source route subframe.
 *
 * Side Effects:    None
 *
 * Overview:        This routine allocates memory for source route subframe,
 *                  and fills the source route subframe
 *
 * Note             The memory allocated for source route subframe should be
 *                  freed by invoking routine
 ********************************************************************/
#if I_SUPPORT_CONCENTRATOR == 1
BYTE *CreateSourceRouteSubframe( BYTE Index, WORD *Hops, SHORT_ADDR *macDestinationAddress )
{
   BYTE nodeCount = 0;
   BYTE countIndex;
   BYTE *subFrame;
   BYTE *ptr;
    /* Find number of nodes */
    countIndex = Index;
    *Hops = 0;
    /* Count the number of hops till the destination */
    while( RouteRecordTable[countIndex].nextHopIndex != NOT_VALID )
    {
        nodeCount++;
        countIndex = RouteRecordTable[countIndex].nextHopIndex;

    }
    /* update the next hop device */
    *macDestinationAddress = RouteRecordTable[countIndex].destinationAddress;
    /* Allocate memory */
     subFrame = SRAMalloc(2 * nodeCount + 2  ); // 2 for index and count in source route subframe
    if( subFrame != NULL )
    {
        ptr = subFrame;
        *ptr++ = nodeCount;
        *ptr++  = nodeCount - 1; /* This field is initialized to 1 less than the relay count */
        countIndex = Index;
        /* Put the list of relaying devices */
        while( nodeCount--)
        {
            countIndex = RouteRecordTable[countIndex].nextHopIndex;
            *ptr++ = RouteRecordTable[countIndex].destinationAddress.byte.LSB;
            *ptr++ = RouteRecordTable[countIndex].destinationAddress.byte.MSB;

        }
        *Hops = ptr- subFrame;
    }

    return subFrame;
}
/*********************************************************************
 * Function:       BYTE GetIndexInRouteRecord( SHORT_ADDR address )
 *
 * PreCondition:    None
 *
 * Input:           address: The address of device whose route record entry
 *                  has to be retreived.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks whether route record entry is avaialble
 *                  for given address.Returns the index if avaiable otherwise
 *                  returns false.
 ********************************************************************/
BYTE GetIndexInRouteRecord( SHORT_ADDR address )
{
    BYTE count;
    for( count = 0; count < MAX_ROUTE_RECORD_TABLE_SIZE; count++)
    {
        if( RouteRecordTable[count].destinationAddress.Val == address.Val )
        {
            return count;
        }
    }
    return NOT_VALID;
}
#endif
/*********************************************************************
 * Function:       static BYTE LoadNwkDataHeader(SHORT_ADDR macAddress,
 * BYTE sourceRouteLength, BYTE *sourceRouteSubframe )
 *
 * PreCondition:    None
 *
 * Input:           macAddress: The next hop address
 *                  sourceRouteLength: The length of source route subframe.
 *                  sourceRouteSubframe: A pointer to source route subframe
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine fills the TxBuffer with network header
 * when upper layer initates data.
 ********************************************************************/
static BYTE LoadNwkDataHeader(SHORT_ADDR macAddress, BYTE sourceRouteLength,
BYTE *sourceRouteSubframe )
{

     BYTE sourceRoutePresent;
     BYTE i;
     BYTE msduHandle;
     BYTE   frameControlMSB = 0x00;
     
     /* reclaim old handles for messages whose recipients are not responding */
     for (i = 0; i < MAX_NWK_FRAMES; i++)
     {
        /* if handle is stale then reclaim the slot */
        if( nwkConfirmationHandles[i].nsduHandle == params.NLDE_DATA_request.NsduHandle )
        {
            nwkConfirmationHandles[i].nsduHandle = INVALID_NWK_HANDLE;
            nwkConfirmationHandles[i].msduHandle = 0x00;
            nwkConfirmationHandles[i].nwkFrameId = 0x00;
        }
     }
     
     // Load up the NWK header information (backwards)
     for (i=0; (i<MAX_NWK_FRAMES) && (nwkConfirmationHandles[i].nsduHandle!=INVALID_NWK_HANDLE); i++) {}

    // If we have no empty slots, return an error.
    if (i == MAX_NWK_FRAMES)
    {
        params.NLDE_DATA_confirm.NsduHandle = params.NLDE_DATA_request.NsduHandle;
        params.NLDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
        ZigBeeUnblockTx();
        return NLDE_DATA_confirm;
    }

    // Save the NWK handle
    nwkConfirmationHandles[i].nsduHandle = params.NLDE_DATA_request.NsduHandle;
    sourceRoutePresent = ( sourceRouteLength > 0 )? (TRUE ):(FALSE);

    if ( params.NLDE_DATA_request.DstAddrMode == 0x01 ) // Multicast
    {
        frameControlMSB = frameControlMSB | 0x01; // Enabling Multicast Bit.
    }
    frameControlMSB = frameControlMSB | (params.NLDE_DATA_request.SecurityEnable<<1);
    frameControlMSB = frameControlMSB | (sourceRoutePresent << 2 );

    #if I_SUPPORT_CONCENTRATOR == 1
    /* Create source route subframe */
    while(sourceRouteLength )
    {

        TxBuffer[TxHeader--] = *( sourceRouteSubframe + sourceRouteLength -1 );
        sourceRouteLength--;
    }
    #endif
    if ( params.NLDE_DATA_request.DstAddrMode == 0x01 ) // Multicast
    {
        MULTICAST_FIELDS multicastFields;
        PERSISTENCE_PIB currentPIB;

        GetPersistenceStorage((void *)&currentPIB);
        multicastFields.bits.mode = NON_MEMBER_MODE_MULTICAST;
        multicastFields.bits.nonMemberRadius = currentPIB.nwkMulticastNonMemberRadius;
        multicastFields.bits.maxNonMemberRadius = currentPIB.nwkMulticastMaxNonMemberRadius;
        TxBuffer[TxHeader--] = multicastFields.Val;
    }
     TxBuffer[TxHeader--] = params.NLDE_DATA_request.NsduHandle; //sequence number
     TxBuffer[TxHeader--] = params.NLDE_DATA_request.BroadcastRadius;
     TxBuffer[TxHeader--] = macPIB.macShortAddress.byte.MSB;    //params.NLDE_DATA_request.SrcAddr.byte.MSB;
     TxBuffer[TxHeader--] = macPIB.macShortAddress.byte.LSB;    //params.NLDE_DATA_request.SrcAddr.byte.LSB;
     TxBuffer[TxHeader--] = params.NLDE_DATA_request.DstAddr.byte.MSB;
     TxBuffer[TxHeader--] = params.NLDE_DATA_request.DstAddr.byte.LSB;
     TxBuffer[TxHeader--] = frameControlMSB; // Frame Control byte MSB
     TxBuffer[TxHeader--] = NWK_FRAME_DATA | (nwkProtocolVersion<<2) | (params.NLDE_DATA_request.DiscoverRoute<<6); // Frame Control byte LSB
    // Load up the MCPS_DATA.request parameters
    Prepare_MCPS_DATA_request( macAddress.Val, &msduHandle );
    nwkConfirmationHandles[i].msduHandle = msduHandle;
    return MCPS_DATA_request;

}

/*********************************************************************
 * Function:       static void CreateRouteRecordFrame(  SHORT_ADDR macDest )
 *
 * PreCondition:    Data received from upper layer that requires a route record frame
 * and data is backed up.
 *
 * Input:           macDest: The next hop to which route record has to be sent.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine creates route record frame.
 *
 * Note:            None
 ********************************************************************/
#if  I_SUPPORT_MANY_TO_ONE_HANDLING == 1
static void CreateRouteRecordFrame(  SHORT_ADDR macDest )
{
    ZigBeeUnblockTx();

    /* Route record frame command header */
    CreateNwkCommandHeader
    (
        NLME_GET_nwkBCSN(),
        DEFAULT_RADIUS,
        macPIB.macShortAddress,
        params.NLDE_DATA_request.DstAddr
    );

    /* Route record frame command payload */
    TxBuffer[TxData++] = NWK_COMMAND_ROUTE_RECORD;
    TxBuffer[TxData++] = ROUTE_RECORD_INITIATOR_LIST_COUNT;


    // Load up the MCPS_DATA.request parameters.
    Prepare_MCPS_DATA_request( macDest.Val, &DataToBeSendAfterRouteRecord.msduHandle );
}

#endif

/*********************************************************************
 * Function:        void StoreRouteRecord( SHORT_ADDR nwkDestinationAddress )
 *
 * PreCondition:    None
 *
 * Input:           nwkDestinationAddress: The source address from which
 *                  route record is received.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine creates the route record table with
 *                  the received route record
 *
 * Note:            Route record received on a concentrator.
 ********************************************************************/
#if I_SUPPORT_CONCENTRATOR ==1
    void StoreRouteRecord( SHORT_ADDR nwkDestinationAddress )
    {
        BYTE relayCount;
        BYTE addressCount;
        BYTE routeRecordIndex;
        BYTE prevIndex = NOT_VALID;
        SHORT_ADDR destinationAddress = nwkDestinationAddress;
        /* get the relay count */
        relayCount = NWKGet();
        for( addressCount=0; addressCount <= relayCount; addressCount++)
        {
            /* get a free/ existing entry */
            routeRecordIndex = GetIndexToAddInRouteRecordTable(destinationAddress);

            /* Update previous index */
            RouteRecordTable[routeRecordIndex].prevHopIndex = prevIndex;

            /* Update the destination address */
            RouteRecordTable[routeRecordIndex].destinationAddress = destinationAddress;
            /* Update the next hop index of previous entry as the current index */
            if( prevIndex != NOT_VALID )
            {
                RouteRecordTable[prevIndex].nextHopIndex = routeRecordIndex;
            }
            else
            {
               NewEntry =  routeRecordIndex;
            }
            /* Update parameter for next loop */
            prevIndex = routeRecordIndex;
            destinationAddress.byte.LSB = NWKGet();
            destinationAddress.byte.MSB = NWKGet();
        }
        /* Update the last link to 0xff */
        RouteRecordTable[routeRecordIndex].nextHopIndex = NOT_VALID;
    }
 /*********************************************************************
 * Function:      BYTE GetIndexToAddInRouteRecordTable( SHORT_ADDR destination)
 *
 * PreCondition:    None
 *
 * Input:           destination: The address of device that has to be added
 *                  into route record table.This is used for getting already
 *                  existing entry.
 *
 * Output:          Returns index to route record table where entry has to be
 *                  added.
 *
 * Side Effects:    None
 *
 * Overview:        This routine creates the route record frame that
 *                  has to be relayed.
 *
 * Note:            None
 ********************************************************************/
    BYTE GetIndexToAddInRouteRecordTable( SHORT_ADDR destination  )
    {
        BYTE count;
        BYTE nextIndex;
        /* Get the entry if existing */
        for( count=0; count < MAX_ROUTE_RECORD_TABLE_SIZE; count++)
        {
            if (RouteRecordTable[count].destinationAddress.Val == destination.Val )
            {
                return count;
            }
        }
        /* Entry does not exists, get a free entry */
         for( count=0; count < MAX_ROUTE_RECORD_TABLE_SIZE; count++)
        {
            if (RouteRecordTable[count].destinationAddress.Val == INVALID_SHORT_ADDRESS  )
            {
                return count;
            }
        }
         /* Neither existing entry nor free entry exists, overwrite an
         old entry */
        for( count=0; count < MAX_ROUTE_RECORD_TABLE_SIZE; count++)
         {
             if (( RouteRecordTable[count].prevHopIndex == NOT_VALID ) &&
             ( count != NewEntry ))
             {
                 nextIndex = RouteRecordTable[count].nextHopIndex;
                 RouteRecordTable[nextIndex].prevHopIndex = NOT_VALID;
                 return count;
             }
         }
         /* getting entry is not possible. */
         return NOT_VALID;
    }
#endif
/*********************************************************************
 * Function:       BYTE CreateRelyingRouteRecord( LONG_ADDR nwkIEEESrcAddress, LONG_ADDR nwkIEEEDstAddress,
 *                             BYTE nwkSequenceNumber, BYTE nwkRadius,
 *                             SHORT_ADDR  nwkSourceAddress, SHORT_ADDR nwkDestinationAddress,
 *                             NWK_FRAME_CONTROL_MSB nwkFrameControlMSB, NWK_FRAME_CONTROL_LSB nwkFrameControlLSB,
 *                             BYTE relayCount, BYTE *relayList )
 *
 * PreCondition:    None
 *
 * Input:
 *                  nwkIEEESrcAddress: IEEEAddress of the device initiated the Route Record frame
 *                  nwkIEEEDstAddress: IEEEAddress of the destination concentrator device
 *                                     to which this Route Record frame needs to be forwarded
 *                  nwkSequenceNumber: Sequence Number of the Route Record Frame
 *                  nwkRadius:         The number of hops this Route Record frame can the forwarded
 *                  nwkSourceAddress:  The originator of Route Record Frame.
 *                  nwkDestinationAddress: The destination concentrator device of Route Record Frame.
 *                  nwkFrameControlMSB: Frame Control fields received in Route Record Frame.
 *                  nwkFrameControlLSB: Frame Control fields received in Route Record Frame.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine creates the route record frame that
 *                  has to be relayed.
 *
 * Note:            None
 ********************************************************************/
#if   I_SUPPORT_MANY_TO_ONE_HANDLING == 1
BYTE CreateRelyingRouteRecord( LONG_ADDR nwkIEEESrcAddress, LONG_ADDR nwkIEEEDstAddress,
                               BYTE nwkSequenceNumber, BYTE nwkRadius,
                               SHORT_ADDR  nwkSourceAddress, SHORT_ADDR nwkDestinationAddress,
                               NWK_FRAME_CONTROL_MSB nwkFrameControlMSB, NWK_FRAME_CONTROL_LSB nwkFrameControlLSB,
                               BYTE relayCount, BYTE *relayList )
{
    SHORT_ADDR nextHopAddr;
    //BYTE relayCount;
    BYTE addressCount;
    BOOL SendRouteRecord;
    BYTE msduHandle;
    BYTE rtStatus;
    BYTE i;
    BYTE includeDstIEEEAddr = FALSE;

    if ( !( nwkRadius > 1 ) )
    {
        return NO_PRIMITIVE;
    }

    /* get mac destination*/
    SendRouteRecord = GetNextHop
                      (
                        nwkDestinationAddress,
                        &nextHopAddr,
                        &rtStatus
                      );
    if (SendRouteRecord)
    {
        /*Update the TxBuffer with the Network Header for Route Record Command*/
        /*CreateNwkCommandHeader
        (
            nwkSequenceNumber,
            nwkRadius - 1,
            nwkSourceAddress,
            nwkDestinationAddress
        );*/

        /*Add SrcIEEEAddress*/
        for (i=0; i<8; i++)
        {
            TxBuffer[TxHeader--] = nwkIEEESrcAddress.v[7-i];
        }

        /*Add DstIEEEAddress*/
        if( nwkFrameControlMSB.bits.dstIEEEAddr )
        {
            includeDstIEEEAddr = TRUE;
            for (i=0; i<8; i++)
            {
                TxBuffer[TxHeader--] = nwkIEEEDstAddress.v[7-i];
            }
        }

        TxBuffer[TxHeader--] = nwkSequenceNumber;
        TxBuffer[TxHeader--] = nwkRadius - 1;
        TxBuffer[TxHeader--] = nwkSourceAddress.byte.MSB;
        TxBuffer[TxHeader--] = nwkSourceAddress.byte.LSB;
        TxBuffer[TxHeader--] = nwkDestinationAddress.byte.MSB;
        TxBuffer[TxHeader--] = nwkDestinationAddress.byte.LSB;
        /*Update the frame control fields -
        like Security, SrcIEEEAddress, DstIEEEAddress, and ProtocolVerison bits*/
        UpdateNwkHeaderFrameContolFields(includeDstIEEEAddr);

        /*Update the TxBuffer with the Network Payload for Route Error Command*/
        TxBuffer[TxData++] = NWK_COMMAND_ROUTE_RECORD;

        /*  Relay count will be recieved route record relay frame + 1*/
        //relayCount = NWKGet();

        TxBuffer[TxData++] = relayCount + 1;
        for( addressCount = 0; addressCount < (relayCount); addressCount++)
        {
            //TxBuffer[TxData++] = NWKGet(); // LSB of short address
            //TxBuffer[TxData++] = NWKGet(); // MSB of short address
            TxBuffer[TxData++] = *relayList++; // LSB of short address
            TxBuffer[TxData++] = *relayList++; // MSB of short address
        }
        TxBuffer[TxData++] =  macPIB.macShortAddress.byte.LSB;
        TxBuffer[TxData++] =  macPIB.macShortAddress.byte.MSB;

        // Load up the MCPS_DATA.request parameters.
        Prepare_MCPS_DATA_request( nextHopAddr.Val, &msduHandle );
        return MCPS_DATA_request;
    }
    else
    {
         return NO_PRIMITIVE;
    }
}
#endif

/*********************************************************************
 * Function:        void CreateNwkHeaderForDataAndStoreData( BYTE discoverRoute, BYTE *ptr )
 *
 * PreCondition:    None
 *
 * Input:           discoverRoute: TRUE if route has to be disabled for
 *                  this NLDE_DATA_Request, FALSE otherwise.
 *                  ptr: ptr where the back up of data has to be taken.
 *
 * Output:          None
 *
 * Side Effects:    None
 * Overview:        This routine creates network header in ptr given and
 *                  then copies the payload available in TxBuffer.
 *
 * Note:            None
 ********************************************************************/
void CreateNwkHeaderForDataAndStoreData( BYTE discoverRoute, BYTE *ptr )
{

     BYTE i;
     // Create the NWK header first.
     *ptr++ = NWK_FRAME_DATA | (nwkProtocolVersion<<2) | (discoverRoute<<6); // Frame Control byte LSB
     *ptr++ = (params.NLDE_DATA_request.SecurityEnable<<1); // Frame Control byte MSB
     *ptr++ = params.NLDE_DATA_request.DstAddr.byte.LSB;
     *ptr++ = params.NLDE_DATA_request.DstAddr.byte.MSB;
     *ptr++ = macPIB.macShortAddress.byte.LSB;
     *ptr++ = macPIB.macShortAddress.byte.MSB;
     *ptr++ = params.NLDE_DATA_request.BroadcastRadius;
     *ptr++ = params.NLDE_DATA_request.NsduHandle;

     // Buffer any headers from upper layers
     while (TxHeader < TX_HEADER_START)
     {
         *ptr++ = TxBuffer[++TxHeader];
     }

     // Buffer the NWK payload.
     i = 0;
     while (TxData--)
     {
         *ptr++ = TxBuffer[i++];
     }
}
/*********************************************************************
 * Function:        void LoadBackUpDataToTxBuffer( BYTE datalength, BYTE *source)
 *
 * PreCondition:    None
 *
 * Input:           datalength: The length of message to be put in TXBuffer
 *                  source: Pointer to message to be put into TxBuffer
 *
 * Output:          None
 *
 * Side Effects:    None
 * Overview:        This routine loads the data in source to TxBuffer
 *
 * Note:            None
 ********************************************************************/
void LoadBackUpDataToTxBuffer( BYTE datalength, BYTE *source)
{
    BYTE j;
    BYTE varHeaderLen;
     
    
#ifdef I_SUPPORT_SECURITY
    NWK_FRAME_CONTROL_MSB   nwkFrameControlMSB;    
    /* pick up the  frame control MSB, it contains extra 
     * header lenght information - 
    */
    nwkFrameControlMSB.Val = source[1];
    varHeaderLen = 0;
    if( nwkFrameControlMSB.bits.srcIEEEAddr ) 
    {
        varHeaderLen = varHeaderLen + 8;  
    }
    if( nwkFrameControlMSB.bits.dstIEEEAddr )
    {
        varHeaderLen = varHeaderLen + 8;  
    }
    if( nwkFrameControlMSB.bits.multicastFlag )
    {
        varHeaderLen = varHeaderLen + 1;
    }
   
    
    /* Load the network header( 8 bytes) on the TxHeader
    part of TxBuffer if security is to be enabled. */
    for (j = 0; j < (8 + varHeaderLen); j++)
    {
        TxHeader = TX_HEADER_START - 8 - varHeaderLen;
        TxBuffer[TxHeader+1+j] = *source++;
    }
    
    
    /* Load network payload into TxData part of TxBuffer */
    for(j = 0; j < (datalength-8 - varHeaderLen); j++)
    {
        TxBuffer[TxData++] = *source++;
    }
#else
    /* Load both network header and payload into
    TxData part of TxBuffer */
    for (j=0; j<datalength; j++)
    {
        TxBuffer[TxData++] = *source++;
    }
#endif

}


#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
#ifndef I_AM_END_DEVICE
/*********************************************************************
 * Function:        static BOOL IsTherePANIdConflict(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE or FALSE
 *
 * Side Effects:    None
 * Overview:        This routine determines if there's a PANid conflict or not
 *
 * Note:            This function shall be called immediatly on beacon reception and nowhere else.
 *                  The function will be called even before the device has joined the network, But this doesn't have any side effect
 *              as the macPIB.macPANId.Val for not yet joined device will be 0xFFFF and the first if condition will be false.
 *              so the fn will return false.
 ********************************************************************/
static BOOL IsTherePANIdConflict(void)
{
    if ( macPIB.macPANId.Val == params.MLME_BEACON_NOTIFY_indication.CoordPANId.Val )
    {
        if ( memcmp( (BYTE *)(&currentNeighborTableInfo.nwkExtendedPANID.v[0]), (BYTE *)(&params.MLME_BEACON_NOTIFY_indication.sdu[3] ),
                                EXTENDED_PAN_SIZE )
           )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE PrepareNetworkReportCmdFrame( void )
 *
 * PreCondition:    PAN ID Conflict detected.
 *
 * Input:           None
 *
 * Output:          MCPS_DATA_request
 *
 * Side Effects:    None
 *
 * Overview:        Sends Network Report Cmd frame.
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/

ZIGBEE_PRIMITIVE PrepareNetworkReportCmdFrame( void )
{
    BYTE *pData , i;

    pData = &TxBuffer[TxData];

    ZigBeeBlockTx();
     /*Cmd Identifier*/
    *pData++ = NWK_COMMAND_NWK_REPORT;
    *pData   = (0x1F & nwkStatus.PanIDReport.noOfRecords);
     /* Cmd Options:(Report Command Identifier Sub-Field) PAN identifier conflict */
    *pData  |= ( 0x00 << 5);
     pData++;
     /* EPID */
     memcpy( pData , (BYTE *)(&currentNeighborTableInfo.nwkExtendedPANID.v[0]), EXTENDED_PAN_SIZE );
     pData += 8;

    /*Report Information*/

    for ( i = 0 ; i < nwkStatus.PanIDReport.noOfRecords ; i++)
    {
        *pData++ = nwkStatus.PanIDReport.PANIdList[i].byte.LSB;
        *pData++ = nwkStatus.PanIDReport.PANIdList[i].byte.MSB;
    }

    GetNeighborTableInfo();
    CreateNwkCommandHeader
    (
        NLME_GET_nwkBCSN(),
        1,
        macPIB.macShortAddress,
        currentNeighborTableInfo.nwkManagerAddr
    );

    Prepare_MCPS_DATA_request((WORD )currentNeighborTableInfo.nwkManagerAddr.Val, &i );

    TxData +=  pData - &TxBuffer[TxData];

    return MCPS_DATA_request;
}
/*********************************************************************
 * Function:        void PrepareNetworkUpdateCmdFrame(void)
 *
 * PreCondition:    None
 *
 * Input:           pointer to PAN_ID_UPDATE
 *
 * Output:
 *
 * Side Effects:
 *
 * Overview:        Will braodcast NetworkUpdateCmdFrame
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/

void PrepareNetworkUpdateCmdFrame( void )
{
    BYTE BTTIndex;
    LONG_ADDR           tmpLongAddr;
    BYTE *pData;
    BTR *pCurrent_BTR;

    if (!CreateNewBTR( &BTTIndex ))
    {
        #ifdef DEBUG_LOG
            LOG_ASSERT(DEBUG_LOG_INFO, "Mem Allocation failed for BTR while sending NetworkUpdateCmdFrame " == 0);
        #endif
    }

     pCurrent_BTR = nwkStatus.BTT[BTTIndex];  //Code optimisation
     // Indicate that the message was not from our upper layers.
     pCurrent_BTR->btrInfo.bMessageFromUpperLayers = 0;

     // Save off the NWK header information
     pCurrent_BTR->dataLength                   = 0x0D;
     pCurrent_BTR->nwkFrameControlLSB.Val       = NWK_FRAME_CMD | (nwkProtocolVersion<<2) | (ROUTE_DISCOVERY_SUPPRESS<<6);
#ifdef I_SUPPORT_SECURITY
     pCurrent_BTR->nwkFrameControlMSB.Val       = ( TRUE << 1 ) | NWK_IEEE_SRC_ADDR;
#else
     pCurrent_BTR->nwkFrameControlMSB.Val       = ( FALSE << 1 ) | NWK_IEEE_SRC_ADDR;
#endif
     pCurrent_BTR->nwkDestinationAddress.Val    = 0xFFFF;
     pCurrent_BTR->nwkSourceAddress             = macPIB.macShortAddress;    //params.NLDE_DATA_request.SrcAddr;
     pCurrent_BTR->nwkRadius                    = DEFAULT_RADIUS;
     pCurrent_BTR->nwkSequenceNumber            = NLME_GET_nwkBCSN();

     GetMACAddress(&tmpLongAddr);

     pCurrent_BTR->nwkIEEESrcAddress            = tmpLongAddr;

     // Allocate space to save off the message
     if ((pCurrent_BTR->dataPtr = (BYTE *)SRAMalloc( 0x0D )) == NULL)
     {
        #ifdef DEBUG_LOG
            LOG_ASSERT(DEBUG_LOG_INFO, "Mem Allocation failed for BTR_data while sending NetworkUpdateCmdFrame " == 0);
        #endif

        // We do not have room to store this broadcast packet.  Destroy the BTR.
        nfree( nwkStatus.BTT[BTTIndex] );
    }

    // Save off the NWK Cmd payload
    pData = pCurrent_BTR->dataPtr;
    // Copy the Cmd Payload
    *pData++ = NWK_COMMAND_NWK_UPDATE;
     /*Cmd Options:(Update Information Count Sub-Field)number of records contained within the Update Information field*/
    *pData   = (0x1F & 0x01);
     /* Cmd Options:(Update Command Identifier Sub-Field) PAN identifier conflict */
    *pData  |= ( 0x00 << 5);
     pData++;
     memcpy( pData , (BYTE *)(&currentNeighborTableInfo.nwkExtendedPANID.v[0]), EXTENDED_PAN_SIZE );
     pData += 8;
    *pData++ = nwkStatus.panIDUpdate->nwkUpdateId;
    *pData++ = nwkStatus.panIDUpdate->newPANId.byte.LSB;
    *pData++ = nwkStatus.panIDUpdate->newPANId.byte.MSB;

    #ifdef I_SUPPORT_SECURITY
    pCurrent_BTR->btrInfo.bAlreadySecured = 0x00;
    #endif

    // Set the network status so we can begin transmitting these messages
    // in the background.
    nwkStatus.flags.bits.bSendingBroadcastMessage = 1;

    panIDConflictStatus.flags.bits.bPANIdUpdatePending = TRUE; // So that nwk manager can update his own PAN id after broadcast.
    // The message is buffered, so unblock Tx.
    ZigBeeUnblockTx();
}
#endif
#endif

/*********************************************************************
 * Function:        BYTE CalculateLinkQuality ( BYTE lqi )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Returns the total calculated Link Cost
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to calculate teh Link Cost
 *
 * Note:            None
 ********************************************************************/
BYTE CalculateLinkQuality ( BYTE lqi )
{
    BYTE    linkCost;

    #ifdef CALCULATE_LINK_QUALITY
        linkCost = 1; // In case LinkQuality is greater than 216
        if (lqi < 216)
            linkCost += 1;
        if (lqi < 180)
            linkCost += 1;
        if (lqi < 144)
            linkCost += 1;
        if (lqi < 108)
            linkCost += 1;
        if (lqi < 72)
            linkCost += 1;
        if (lqi < 36)
            linkCost += 1;
    #else
        linkCost = CONSTANT_PATH_COST;
    #endif
    return linkCost;
}

/*********************************************************************
 * Function:        static void AgeOutNeighborTableEntries ( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function verifies whether any of the Neighbor table
 *                  entries needs to resetted so that it can be reused for
 *                  other devices.
 *
 * Note:            None
 ********************************************************************/
#if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
static void AgeOutNeighborTableEntries ( void )
{
    BYTE i;
    BOOL UpdateNVMflag; 
   
    // Multile Link Status command needs to be transmitted.
    // This means that all the addresses in the Neighbor Record could
    // be transmitted in one frame. So, remaining address should be
    // transmitted following the first link status command transmission.
    // This condition is required because to avoid setting of
    // nbtIndexForLinkStatus field to Zero because this field has the
    // information about the starting Neighbor record from which the
    // next link status command shall be generated
    if ( nwkStatus.moreLinkStatusCmdPending )
    {
        return;
    }

    // Update the nbtIndexForLinkStatus to Zero. This is because, we have to transmit the Link Status command frame.
    // We have to loop through the Neighbor table from the begining (index from 0) to create the Link Status List.
    nwkStatus.nbtIndexForLinkStatus = 0;
    nwkStatus.flags.bits.bTransmitLinkStatusCommand = 1;

    if( nwkStatus.flags.bits.bSendingBroadcastMessage )
    {
        return;
    }

    #ifdef USE_EXTERNAL_NVM
    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        UpdateNVMflag = FALSE;
        GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
        if ( ( currentNeighborRecord.deviceInfo.bits.bInUse ) &&
             ( currentNeighborRecord.deviceInfo.bits.deviceType != DEVICE_ZIGBEE_END_DEVICE ) )
        {
            
            if( RAMNeighborTable[i].shortAddr.Val == currentNeighborRecord.shortAddr.Val )
            {
                RAMNeighborTable[i].linkStatusInfo.Age++;
                currentNeighborRecord.linkStatusInfo.Age = RAMNeighborTable[i].linkStatusInfo.Age;
                   
                //if ( currentNeighborRecord.linkStatusInfo.Age >= NWK_ROUTER_AGE_LIMIT )
                if ( RAMNeighborTable[i].linkStatusInfo.Age >= NWK_ROUTER_AGE_LIMIT )
                {
                    // Neighbor Record aged out. So mark it to not used so that it can be reused for other device.
                    currentNeighborRecord.deviceInfo.bits.bInUse = 0;
                    currentNeighborRecord.deviceInfo.Val = 0;
                    currentNeighborTableInfo.neighborTableSize--;
                    PutNeighborTableInfo();
                    
                    UpdateNVMflag = TRUE;
                    
                    
                    RAMNeighborTable[i].linkStatusInfo.flags.Val = 0;
                    RAMNeighborTable[i].linkStatusInfo.Age = 0;
                    RAMNeighborTable[i].shortAddr.Val = 0xFFFF;
    
                }
                
                if ( ( currentNeighborRecord.linkStatusInfo.flags.bits.bIncomingCost != 
                          RAMNeighborTable[i].linkStatusInfo.flags.bits.bIncomingCost ) || 
                      ( currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost != 
                          RAMNeighborTable[i].linkStatusInfo.flags.bits.bOutgoingCost ) )
                {
                    currentNeighborRecord.linkStatusInfo.flags.bits.bIncomingCost = 
                                    RAMNeighborTable[i].linkStatusInfo.flags.bits.bIncomingCost;
                    currentNeighborRecord.linkStatusInfo.flags.bits.bOutgoingCost = 
                                    RAMNeighborTable[i].linkStatusInfo.flags.bits.bOutgoingCost;
                    UpdateNVMflag = TRUE;
                    
                }
                /*// Update the corresponding Neighbor Record
                PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );*/
            }
        }
        
        /* Store into NVM only if there is an update */
        if( TRUE == UpdateNVMflag )
        {
            // Update the corresponding Neighbor Record
            PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
        }
    }
}
#endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

/*********************************************************************
 * Function:        static void CreateLinkStatusCommandFrame( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine creates Link Status frame. This will also check whether is the
 *                  last Link Status command frame to be sent. If yes, the it will reset
 *                  flag. Then Link Status command frame should be transmitted only in the next
 *                  cycle.
 *
 * Note:            No pointers are initialized here.
 ********************************************************************/
#if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
static void CreateLinkStatusCommandFrame( void )
{
    SHORT_ADDR  dstAddr;
    BYTE        i, index, count = 1;
    BYTE        *commandOptionsPosition;

    NODE_DESCRIPTOR     nodeDesc;
    ProfileGetNodeDesc(&nodeDesc);

    dstAddr.Val = 0xFFFC;

    index = nwkStatus.nbtIndexForLinkStatus;

    //ZigBeeUnblockTx();

    /* Create Link Status Command Header */
    CreateNwkCommandHeader
    (
        NLME_GET_nwkBCSN(),
        1, // Link Status Command frame should always be one hop broadcast without retries.
        macPIB.macShortAddress,
        dstAddr
    );

    /* Create Link Status Command Payload */
    TxBuffer[TxData++] = NWK_COMMAND_LINK_STATUS;
    commandOptionsPosition = &TxBuffer[TxData++];

    #ifdef USE_EXTERNAL_NVM
    for (i=index, pCurrentNeighborRecord = neighborTable + ( index * (WORD)sizeof(NEIGHBOR_RECORD) );
         i < MAX_NEIGHBORS;
         i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for (i=index, pCurrentNeighborRecord = neighborTable + index;
         i < MAX_NEIGHBORS; i++,
         pCurrentNeighborRecord++)
    #endif
    {
        // Make sure we dont exceed the Max Payload size per frame
        if ( ( count * 3 ) < nodeDesc.NodeMaxBufferSize )
        {
            GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
            if ( ( currentNeighborRecord.deviceInfo.bits.bInUse ) &&
                 ( currentNeighborRecord.deviceInfo.bits.deviceType != DEVICE_ZIGBEE_END_DEVICE ) )
            {
                TxBuffer[TxData++] = currentNeighborRecord.shortAddr.byte.LSB;
                TxBuffer[TxData++] = currentNeighborRecord.shortAddr.byte.MSB;
                TxBuffer[TxData++] = currentNeighborRecord.linkStatusInfo.flags.Val;
                count++;
            }
        }
        else
        {
            break;
        }
    }

    // Update the nbtIndexForLinkStatus so that next time we need to start from this point, not from the beginning.
    if ( i == MAX_NEIGHBORS )
    {
        nwkStatus.nbtIndexForLinkStatus = 0;
        // Next link status cmd shall be transmitted in the next cycle of LinkStausPeriod expiry
        nwkStatus.moreLinkStatusCmdPending = 0;
    }
    else
    {
        nwkStatus.nbtIndexForLinkStatus = i;
        // Some more link status command needs to be sent
        nwkStatus.moreLinkStatusCmdPending = 1;
    }

    // Update the Count bit fields in the Link Status Command payload.
    *commandOptionsPosition = ( count - 1 ) ;

    if ( ( index == 0 ) && ( i == MAX_NEIGHBORS ) )
    {
        // Update the FirstFrame and LastFrame bit fields in the Link Status Command payload.
        // We should OR because to retain the Count Field information as these are bit fields.
        *commandOptionsPosition = ( *commandOptionsPosition | LINK_STATUS_COMMAND_OPTIONS_FIRST_AND_LAST_FRAME );
        // All the addresses in the neighbor record can be fit in one frame. So no more pending
        // Link Status command transmission. So, reset the variables.
        nwkStatus.flags.bits.bTransmitLinkStatusCommand = 0;
        nwkStatus.nwkLinkStatusPeriod = TickGet();
    }
    else if ( ( index == 0 ) && ( i != MAX_NEIGHBORS ) )
    {
        // Update the FirstFrame and LastFrame bit fields in the Link Status Command payload.
        // We should OR because to retain the Count Field information as these are bit fields.
        *commandOptionsPosition = ( *commandOptionsPosition | LINK_STATUS_COMMAND_OPTIONS_FIRST_FRAME );
        // Some more link status command needs to be sent
    }
    else if ( ( index != 0 ) && ( i == MAX_NEIGHBORS ) )
    {
        // Update the FirstFrame and LastFrame bit fields in the Link Status Command payload.
        // We should OR because to retain the Count Field information as these are bit fields.
        *commandOptionsPosition = ( *commandOptionsPosition | LINK_STATUS_COMMAND_OPTIONS_LAST_FRAME );
        // No more pending link status command beacause this is the last frame.
        nwkStatus.flags.bits.bTransmitLinkStatusCommand = 0;
        nwkStatus.nwkLinkStatusPeriod = TickGet();
    }
    else if ( ( index != 0 ) && ( i != MAX_NEIGHBORS ) )
    {
        // Update the FirstFrame and LastFrame bit fields in the Link Status Command payload.
        // We should OR because to retain the Count Field information as these are bit fields.
        *commandOptionsPosition = ( *commandOptionsPosition | LINK_STATUS_COMMAND_OPTIONS_INTERMEDIATE_FRAME );
        // Some more link status command needs to be sent
    }

    // Load up the MCPS_DATA.request parameters.
    Prepare_MCPS_DATA_request( 0xFFFF, &i );
}
#endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )


/***************************************************************************************
 * Function:        BOOL VerifyNetworkManager()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          True-if the device is a network manager, false otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        Verifies the device, whether it is a network manager or not.
 *
 * Note:            None
 ***************************************************************************************/
BOOL VerifyNetworkManager()
{
    GetNeighborTableInfo();
    if( macPIB.macShortAddress.Val == currentNeighborTableInfo.nwkManagerAddr.Val )
    {
        return TRUE;
    }
    return FALSE;
}

/***************************************************************************************
 * Function:        void StorePersistancePIB()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Store the PIB's into NVM, so that it will be restored
 *                  after silent start.
 *
 * Note:            None
 ***************************************************************************************/

void StorePersistancePIB()
{
    #if I_SUPPORT_LINK_KEY == 1
        BYTE i;
    #endif
    PERSISTENCE_PIB current_pib;
    GetPersistenceStorage((void *)&current_pib);
    current_pib.macCoordExtendedAddress = macPIB.macCoordExtendedAddress;
    current_pib.macCoordShortAddress = macPIB.macCoordShortAddress;
    current_pib.macPANId = macPIB.macPANId;
    current_pib.macShortAddress = macPIB.macShortAddress;
    current_pib.phyCurrentChannel = phyPIB.phyCurrentChannel;
    #if I_SUPPORT_LINK_KEY == 1
        if(!I_AM_TRUST_CENTER){//#ifndef I_AM_TRUST_CENTER	//MSEB
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
    #endif
    PutPersistenceStorage((void *)&current_pib);
    
    #ifdef I_SUPPORT_SECURITY
        PutOutgoingFrameCount(OutgoingFrameCount);
    #endif
}


/***************************************************************************************
 * Function:        void RestorePersistencePIB()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Restore the PIB's from NVM.
 *
 * Note:            None
 ***************************************************************************************/

void RestorePersistencePIB()
{
    #if I_SUPPORT_LINK_KEY == 1
        BYTE i;
    #endif
    PERSISTENCE_PIB current_pib;
    GetPersistenceStorage((void *)&current_pib);
    macPIB.macCoordExtendedAddress = current_pib.macCoordExtendedAddress;
    macPIB.macCoordShortAddress = current_pib.macCoordShortAddress;
    macPIB.macPANId = current_pib.macPANId;
    macPIB.macShortAddress = current_pib.macShortAddress;
    phyPIB.phyCurrentChannel = current_pib.phyCurrentChannel;
    #if I_SUPPORT_LINK_KEY == 1
        if(!I_AM_TRUST_CENTER){//#ifndef I_AM_TRUST_CENTER	//MSEB
            //current_pib.TrustCenterShortAddr = TCLinkKeyInfo.trustCenterShortAddr;
            TCLinkKeyInfo.trustCenterShortAddr.Val = current_pib.TrustCenterShortAddr.Val;
            for(i=0; i < 8; i++)
            {
                 TCLinkKeyInfo.trustCenterLongAddr.v[i] = current_pib.TrustCentreLongAddr.v[i] ;
            }
        }//#endif /* I_AM_TRUST_CENTER */
        for(i=0; i < KEY_LENGTH; i++)
        {
            TCLinkKeyInfo.link_key.v[i] = current_pib.linkKey.v[i];
        }
        TCLinkKeyInfo.frameCounter = current_pib.frameCounter;
    #endif
    MACEnable();
}

/***************************************************************************************
 * Function:        void ClearNeighborTable()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function will clear all the entries in neighbour table except the parent's entry.
 *
 * Note:            None
 ***************************************************************************************/

void ClearNeighborTable()
{
    BYTE i;

    currentNeighborRecord.deviceInfo.Val = 0;

    #ifdef USE_EXTERNAL_NVM
    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
    #else
    for (i=0, pCurrentNeighborRecord = neighborTable; i < MAX_NEIGHBORS; i++, pCurrentNeighborRecord++)
    #endif
    {
        //#ifndef I_AM_COORDINATOR
            if (((NOW_I_AM_NOT_A_CORDINATOR()) && ( i != currentNeighborTableInfo.parentNeighborTableIndex )) || (NOW_I_AM_A_CORDINATOR()))
        //#endif
                PutNeighborRecord( pCurrentNeighborRecord, &currentNeighborRecord );
    }

    // Reset count fields, indicate that the address info is invalid, clear
    // the parent record index, and mark the table as valid.

    if(NOW_I_AM_NOT_A_CORDINATOR()){//#ifndef I_AM_COORDINATOR
        currentNeighborTableInfo.neighborTableSize  = 1;
    }else{//#else
        currentNeighborTableInfo.neighborTableSize  = 0;
    }//#endif

    #ifndef I_AM_END_DEVICE
        currentNeighborTableInfo.numChildren                        = 0;
        #if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
            currentNeighborTableInfo.numChildRouters                    = 0;
            currentNeighborTableInfo.flags.bits.bChildAddressInfoValid  = 0;
        #endif
    #endif
    PutNeighborTableInfo();
}

/***************************************************************************************
 * Function:        void SetMulticastParams (BYTE useMulticast, BYTE nonMemberRadius, BYTE maxNonMemberRadius)
 *
 * PreCondition:    None
 *
 * Input:           useMulticast -> Specifies whether NWK Layer Multicast to be used or APS Layer Group Addressing to be used
 *                  nonMemberRadius -> Used to fill the NonMemberRadius field in the NWK Layer Multicast frame. Ignored for APS Layer Group Addressing
 *                  maxNonMemberRadius -> Used to fill the MaxNonMemberRadius field in the NWK Layer Multicast frame. Ignored for APS Layer Group Addressing
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function will set the necessary parameters used for NWK Layer Multicast.
 *
 * Note:            None
 ***************************************************************************************/
#if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
void SetMulticastParams (BYTE useMulticast, BYTE nonMemberRadius, BYTE maxNonMemberRadius)
{
    PERSISTENCE_PIB currentPIB;

    GetPersistenceStorage((void *)&currentPIB);
    currentPIB.nwkUseMulticast                  = useMulticast;
    currentPIB.nwkMulticastNonMemberRadius      = nonMemberRadius;
    currentPIB.nwkMulticastMaxNonMemberRadius   = maxNonMemberRadius;
    PutPersistenceStorage((void *)&currentPIB);
}
#endif // #if (I_SUPPORT_NWK_LAYER_MULTICAST == 0x01)
