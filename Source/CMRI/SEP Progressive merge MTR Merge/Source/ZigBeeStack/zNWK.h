/*********************************************************************
 *
 *                  ZigBee NWK Header File
 *
 *********************************************************************
 * FileName:        zNWK.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
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

#ifndef _zNWK_H_
#define _zNWK_H_

#include "ZigbeeTasks.h"
#include "generic.h"
#include "zigbee.h"
#include "SymbolTime.h"

// ******************************************************************************
// NWK Layer Spec Constants

// A Boolean flag indicating whether the device is capable of becoming the
// ZigBee coordinator.
#ifdef I_HAVE_COORDINATOR_CAPABILITY
    #define nwkcCoordinatorCapable  0x01
#else
    #define nwkcCoordinatorCapable  0x00
#endif

// The default security level to be used.
#define nwkcDefaultSecurityLevel  ENC-MIC-64

// The maximum number of times a route discovery will be retried.
#define nwkcDiscoveryRetryLimit     0x03

// The maximum depth (minimum number of logical hops from the ZigBee
// coordinator) a device can have.
#define nwkcMaxDepth                0x0f

// The maximum number of octets added by the NWK layer to its payload without
// security. If security is required on a frame, its secure processing may inflate
// the frame length so that it is greater than this value.
#define nwkcMaxFrameOverhead        0x0d

// The maximum number of octets that can be transmitted in the NWK frame payload field.
#define nwkcMaxPayloadSize          (aMaxMACFrameSize – nwkcMaxFrameOverhead)

// The version of the ZigBee NWK protocol in the device.
#define nwkcProtocolVersion         0x02

// Maximum number of allowed communication errors after which the route repair mechanism is initiated.
#define nwkcRepairThreshold         0x03

// Time duration in milliseconds until a route discovery expires.
#define nwkcRouteDiscoveryTime      0x2710

// The maximum broadcast jitter time measured in milliseconds.
#define nwkcMaxBroadcastJitter      0x40

// The number of times the first broadcast transmission of a route request
// command frame is retried.
#define nwkcInitialRREQRetries      0x03

// The number of times the broadcast transmission of a route request command frame is retried on
// relay by an intermediate ZigBee router or ZigBee coordinator.
#define nwkcRREQRetries             0x02

// The number of milliseconds between retries of a broadcast route request command frame.
#define nwkcRREQRetryInterval       0xfe

// The minimum jitter, in 2 millisecond slots, for broadcast retransmission of a route
// request command frame.
#define nwkcMinRREQJitter           0x01

// The maximum jitter, in 2 millisecond slots, for broadcast retransmission of a
// route request command frame.
#define nwkcMaxRREQJitter           0x40

#define nwkMAGICResSeq      0x5A
#define INVALID_SHORT_ADDRESS   0xFFFF

// address to broadcast address to all devices
#define BC_TO_ALL_DEVICES                           0xFFFF
//  address to broadcast address to all RxOnWhenIdle devices
#define BC_TO_RX_ON_WHEN_IDLE_DEVICES               0xFFFD
/*  address to Broadcast to all routers and coordinator */
#define BC_TO_ALL_ROUTERS_AND_COORDINATORS          0xFFFC
// ******************************************************************************
// Constants and Enumerations

#define DEFAULT_RADIUS                  (2*PROFILE_nwkMaxDepth)
#define INVALID_NWK_HANDLE              0x00
#define INVALID_NEIGHBOR_KEY            (NEIGHBOR_KEY)(MAX_NEIGHBORS)
#define ROUTE_DISCOVERY_ENABLE          0x01
#define ROUTE_DISCOVERY_FORCE           0x02
#define ROUTE_DISCOVERY_SUPPRESS        0x00

#ifdef USE_EXTERNAL_NVM
    #define GetPersistenceStorage(x)            NVMRead(x, persistencePIB, sizeof(PERSISTENCE_PIB))

    #define PutPersistenceStorage(x)            NVMWrite(persistencePIB, x, sizeof(PERSISTENCE_PIB))
#endif

typedef enum _NWK_STATUS_CODES
{
    NWK_STATUS_NO_ROUTE_AVAILABLE = 0x00,
    NWK_STATUS_TREE_LINK_FAILURE,
    NWK_STATUS_NONTREE_LINK_FAILURE,
    NWK_STATUS_LOW_BATTERY,
    NWK_STATUS_NO_ROUTING_CAPACITY,
    NWK_STATUS_NO_INDIRECT_CAPACITY,
    NWK_STATUS_INDIRECT_TRANSACTION_EXPIRY,
    NWK_STATUS_ADDRESS_CONFLICT = 0x0d,
    NWK_STATUS_MANY_TO_ONE_ROUTE_FAILURE = 0x0c,
    NWK_STATUS_SOURCE_ROUTE_FAILURE = 0x0b,
    NWK_STATUS_PAN_ID_UPDATE = 0x0F

} NWK_STATUS_CODES;

typedef enum _LEAVE_REASONS
{
    COORDINATOR_FORCED_LEAVE        = 0x01,
    SELF_INITIATED_LEAVE            = 0x02,
    REQUEST_CHILD_TO_LEAVE          = 0x03  // added for ZigBee 2007 and Pro
} LEAVE_REASONS;

/* ZigBee 2006 requirement:   Need the REJOIN PROCESS as well */
typedef enum _REJOIN_NETWORK_PROCESS
{
   ASSOCIATION_REJOIN       = 0x00,
   ORPHAN_REJOIN            = 0X01,
   REJOIN_PROCESS           = 0x02
} REJOIN_NETWORK_PROCESS;


typedef enum _NWK_STATUS_VALUES
{
    NWK_SUCCESS                     = 0x00,
    NWK_INVALID_PARAMETER           = 0xC1,
    NWK_INVALID_REQUEST             = 0xC2,
    NWK_NOT_PERMITTED               = 0xC3,
    NWK_STARTUP_FAILURE             = 0xC4,
    NWK_ALREADY_PRESENT             = 0xC5,
    NWK_SYNC_FAILURE                = 0xC6,
    NWK_TABLE_FULL                  = 0xC7,
    NWK_UNKNOWN_DEVICE              = 0xC8,
    NWK_UNSUPPORTED_ATTRIBUTE       = 0xC9,
    NWK_NO_NETWORKS                 = 0xCA,
    NWK_LEAVE_UNCONFIRMED           = 0xCB,
    NWK_MAX_FRM_CNTR                = 0xCC, // Security failed - frame counter reached maximum
    NWK_NO_KEY                      = 0xCD, // Security failed - no key
    NWK_BAD_CCM_OUTPUT              = 0xCE, // Security failed - bad output
    NWK_CANNOT_ROUTE                = 0xCF,
    NWK_ROUTE_ERROR                 = 0xD1,
    NWK_AUTHENTICATION_FAILURE      = 0xD5
} NWK_STATUS_VALUES;


typedef enum _RELATIONSHIP_TYPE
{
    NEIGHBOR_IS_PARENT  = 0,
    NEIGHBOR_IS_CHILD,
    NEIGHBOR_IS_SIBLING,
    NEIGHBOR_IS_NONE,
    NEIGHBOR_IS_PREVIOUS_CHILD
} RELATIONSHIP_TYPE;

typedef enum _ZIGBEE_DEVICE_TYPE_VALUES
{
    DEVICE_ZIGBEE_COORDINATOR       = 0x00,
    DEVICE_ZIGBEE_ROUTER            = 0x01,
    DEVICE_ZIGBEE_END_DEVICE        = 0x02
} ZIGBEE_DEVICE_TYPE_VALUES;

typedef enum _ROUTE_STATUS
{
    ROUTE_ACTIVE                    = 0x00,
    ROUTE_DISCOVERY_UNDERWAY        = 0x01,
    ROUTE_DISCOVERY_FAILED          = 0x02,
    ROUTE_INACTIVE                  = 0x03,
    ROUTE_RECORD_REQUIRED           = 0x04,
    NO_ROUTE_RECORD_REQUIRED        = 0x05
} ROUTE_STATUS;

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
    #ifndef I_AM_END_DEVICE
        typedef enum _ADDRESS_CONFLICT_TYPE
        {
            NO_CONFLICT                                 = 0x00,
            LOCAL_ADDRESS_CONFLICT                      = 0x01, // My own short address in conflcit
            ADDRESS_CONFLICT_ON_A_CHILD_END_DEVICE      = 0x02, // One of my child end device's short address in conflcit
            REMOTE_ADDRESS_CONFLICT                     = 0x03  // Some other device's short address in conflcit
        } ADDRESS_CONFLICT_TYPE;
    #endif
#endif

// ******************************************************************************
// Structures

typedef struct __Config_NWK_Mode_and_Params
{
     BYTE   ProtocolVersion;
     BYTE   StackProfile;
     BYTE   BeaconOrder;
     BYTE   SuperframeOrder;
     BYTE   BatteryLifeExtension;
     BYTE   SecuritySetting;
     DWORD  Channels;
} _Config_NWK_Mode_and_Params;


typedef BYTE NEIGHBOR_KEY;


typedef union _NEIGHBOR_RECORD_DEVICE_INFO
{
    struct
    {
        BYTE LQI                : 8;
        BYTE Depth              : 4;
        BYTE StackProfile       : 4;    // Needed for network discovery
        BYTE ZigBeeVersion      : 4;    // Needed for network discovery
        BYTE RouterCapacity     : 1;
        BYTE EndDeviceCapacity  : 1;
        BYTE deviceType         : 2;
        BYTE Relationship       : 2;
        BYTE RxOnWhenIdle       : 1;
        BYTE bInUse             : 1;
        BYTE PermitJoining      : 1;
        BYTE PotentialParent    : 1;
    } bits;
    DWORD Val;
} NEIGHBOR_RECORD_DEVICE_INFO;

#if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
typedef struct _LINK_STATUS_INFO
{
    union _flags
    {
        BYTE    Val;
        struct _bits
        {
            BYTE    bIncomingCost   : 3;
            BYTE    bReserved1      : 1;
            BYTE    bOutgoingCost   : 3;
            BYTE    bReserved2      : 1;
        } bits;
    }flags;
    BYTE    Age;
}LINK_STATUS_INFO;

typedef struct _LINK_STATUS_RECORD
{
    SHORT_ADDR              shortAddr;
    LINK_STATUS_INFO        linkStatusInfo;

}LINK_STATUS_RECORD_INFO;

#endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

typedef struct _NEIGHBOR_RECORD
{
    LONG_ADDR                   longAddr;
    SHORT_ADDR                  shortAddr;
    PAN_ADDR                    panID;
    NEIGHBOR_RECORD_DEVICE_INFO deviceInfo;
    BYTE                        LogicalChannel; // Needed to add for NLME_JOIN_request and other things.
    //#ifdef I_SUPPORT_SECURITY
    BOOL                        bSecured;
    //#endif
    BYTE                        nwkUpdateId;
    LONG_ADDR                   ExtendedPANID;
    #if (I_SUPPORT_LINK_STATUS_COMMANDS == 0x01)
        LINK_STATUS_INFO        linkStatusInfo;
    #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
    #if (I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
        BYTE                    suppressLinkStatusUpdate;
    #endif //(I_SUPPORT_LINK_STATUS_COMMAND_TESTING == 0x01 )
} NEIGHBOR_RECORD;


typedef struct _NEIGHBOR_TABLE_INFO
{
    WORD        validityKey;
    BYTE        neighborTableSize;
    BYTE        nwkUpdateId;
    SHORT_ADDR  nwkManagerAddr;

    LONG_ADDR nwkExtendedPANID;

//#ifndef I_AM_COORDINATOR
    BYTE        parentNeighborTableIndex;
//#endif

#ifndef I_AM_END_DEVICE
    BYTE        depth;              // Our depth in the network
    BYTE        numChildren;        // How many children we have
#if ( I_SUPPORT_STOCHASTIC_ADDRESSING != 0x01 )
    SHORT_ADDR  cSkip;              // Address block size
    SHORT_ADDR  nextEndDeviceAddr;  // Next address available to give to an end device
    SHORT_ADDR  nextRouterAddr;     // Next address available to give to a router
    BYTE        numChildRouters;    // How many of our children are routers
    union _flags
    {
        BYTE    Val;
        struct _bits
        {
            BYTE    bChildAddressInfoValid : 1;  // Child addressing information is valid
        } bits;
    }flags;
#else
    SHORT_ADDR  nextChildAddr;  // Next address available to give to a child device
#endif
#endif
} NEIGHBOR_TABLE_INFO;


typedef struct _ROUTING_ENTRY
{
    SHORT_ADDR      destAddress;
    union _rtflags
    {
        struct _rtbits
        {
            BYTE    bStatus             : 3;
            BYTE    bNoRouteCache       : 1;
            BYTE    bManyToOne          : 1;
            BYTE    bRouteRecordRequired: 1;
            BYTE    bGroupIDFlag        : 1;
        } rtbits;
    }rtflags;
    SHORT_ADDR      nextHop;
    #if (I_SUPPORT_ROUTING_TABLE_AGING == 0x01)
        BYTE            lastModified;
    #endif
} ROUTING_ENTRY;

typedef struct _ROUTE_DST_INFO
{
    SHORT_ADDR dstAddr;
    BYTE counter;
} ROUTE_DST_INFO;
#define ROUTE_DST_INFO_SIZE 5

#ifdef I_SUPPORT_SECURITY
typedef struct _NETWORK_KEY_INFO
{
    KEY_VAL     NetKey;
    WORD_VAL    SeqNumber;
} NETWORK_KEY_INFO;
#endif

typedef struct _CHANNEL_INFO
{
    BYTE    channel;
    BYTE    energy;
    BYTE    networks;
} CHANNEL_INFO;

typedef struct _DISCOVERY_INFO
{
    BYTE            currentIndex;
    BYTE            numChannels;
    CHANNEL_INFO    *channelList;
} DISCOVERY_INFO;


typedef union _NWK_FRAME_CONTROL_LSB
{
    BYTE    Val;
    struct _NWK_FRAME_CONTROL_LSB_bits
    {
        BYTE    frameType       : 2;
        BYTE    protocolVersion : 4;
        BYTE    routeDiscovery  : 2;
    } bits;
} NWK_FRAME_CONTROL_LSB;


typedef union _NWK_FRAME_CONTROL_MSB
{
    BYTE    Val;
    struct _NWK_FRAME_CONTROL_MSB_bits
    {
        BYTE    multicastFlag   : 1;
        BYTE    security        : 1;
        BYTE    sourceRoute     : 1;
        BYTE    dstIEEEAddr     : 1;
        BYTE    srcIEEEAddr     : 1;
        BYTE    reserved        : 3;
    } bits;
} NWK_FRAME_CONTROL_MSB;

typedef union _MULTICAST_FIELDS
{
    BYTE    Val;
    struct _MULTICAST_BITS
    {
        BYTE    mode                : 2;
        BYTE    nonMemberRadius     : 3;
        BYTE    maxNonMemberRadius  : 3;
    } bits;
}  MULTICAST_FIELDS;

typedef struct _BTR     // Broadcast Transaction Record
{
    BYTE                    *dataPtr;
    BYTE                    dataLength;
    MULTICAST_FIELDS        nwkMulticastFields;
    BYTE                    nwkSequenceNumber;
    BYTE                    nwkRadius;
    SHORT_ADDR              nwkSourceAddress;
    SHORT_ADDR              nwkDestinationAddress;
    LONG_ADDR               nwkIEEESrcAddress;
    NWK_FRAME_CONTROL_MSB   nwkFrameControlMSB;
    NWK_FRAME_CONTROL_LSB   nwkFrameControlLSB;
    BYTE                    currentNeighbor;
    TICK                    broadcastJitterTimer;
    TICK                    broadcastTime;
    struct _BTR_flags1
    {
        BYTE    nRetries                : 4;
        BYTE    bMessageFromUpperLayers : 1;
        BYTE    bConfirmSent            : 1;
    #ifdef I_SUPPORT_SECURITY
        BYTE    bAlreadySecured         : 1;
    #endif
    } btrInfo;

    union _BTR_flags2
    {
        // TODO Since we are down to one flag per neighbor, would it be worth
        // bitmapping these?  Maybe not - a lot of overhead to calculate mask and index,
        // and this is only alloc'd.
        BYTE    byte;
        struct BTR__bits
        {
            BYTE    bMessageNotRelayed      : 1;
        } bits;
    } flags[MAX_NEIGHBORS];
} BTR;
#define BTR_FLAGS_INIT  0x03


typedef struct _BUFFERED_MESSAGE_INFO
{
    BYTE        *dataPtr;
    BYTE        dataLength;
    SHORT_ADDR  sourceAddress;
    SHORT_ADDR  destinationAddress;
    TICK        timeStamp;
} BUFFERED_MESSAGE_INFO;

#ifdef I_SUPPORT_ROUTING
    typedef struct _ROUTE_HANDLE_RECORD
    {
        BYTE        nwkSequence;
        BYTE        macSequence;
        BYTE        isToConcentrator;
        BYTE        statusRetried;
        SHORT_ADDR  macSourceAddress;
        SHORT_ADDR  SourceAddress;
        SHORT_ADDR  DstAddress;
    } ROUTE_HANDLE_RECORD;
#endif

#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
    typedef struct _PAN_ID_UPDATE
    {
        BYTE nwkUpdateId;
        PAN_ADDR  newPANId;
        //#ifdef I_AM_RFD
            TICK waitTime;
        //#endif
    } PAN_ID_UPDATE;

    typedef struct _PAN_ID_REPORT
    {
        BYTE                     noOfRecords;
        BYTE                     index;
        PAN_ADDR                 PANIdList[8];
    } PAN_ID_REPORT;

    typedef struct _PAN_ID_CONFLICT_STATUS
    {
        union _PAN_ID_CONFLICT_STATUS_flags
        {
            BYTE Val;
            struct _PAN_ID_CONFLICT_STATUS_bits
            {
                // Background Task Flags
                //#ifndef I_AM_RFD
                    BYTE    bPANIdConflictDetected      : 1;
                    BYTE    bActionOnPANIdReportPending : 1;
                    BYTE    bResolutionInProgress       : 1;
                //#endif
                BYTE    bPANIdUpdatePending         : 1;
            } bits;
        } flags;
    } PAN_ID_CONFLICT_STATUS;
#endif

#if I_SUPPORT_FREQUENCY_AGILITY == 1
    /* Structure to store ED records received
       after inteference reporting. */
    typedef struct _ENERGY_DETECT_RECORD
    {
        SHORT_ADDR  ScanDeviceAddr;
        BYTE        EnergyReading[16];
    } ENERGY_DETECT_RECORD;
#endif

typedef struct _NWK_STATUS
{
    union _NWK_STATUS_flags
    {
        DWORD    Val;
        struct _NWK_STATUS_bits
        {
            // Background Task Flags
            BYTE    bSendingBroadcastMessage    : 1;
            BYTE    bAwaitingRouteDiscovery     : 1;
            BYTE    bTimingJoinPermitDuration   : 1;
            BYTE    bLeaveReset                 : 1;
            BYTE    bRejoinIndication           : 1;
            BYTE    bPANIdConflictDetected      : 1;
            BYTE    bPANIdUpdatePending         : 1;
            BYTE    bActionOnPANIdReportPending : 1;
            BYTE    bCanRoute                   : 1;
            BYTE    bAllEndDeviceAddressesUsed  : 1;
            BYTE    bAllRouterAddressesUsed     : 1;
            BYTE    bNextChildAddressUsed       : 1;
            BYTE    bNRejoin                    : 1;    // added for 2006
            BYTE    bRejoinScan                 : 1;    // added for 2006
            BYTE    bRejoinInProgress           : 1;    // added for 2006

            BYTE    bSelfLeave                  : 1;    // added for ZigBee 2007 and Pro
            BYTE    bRemoveChildren             : 1;    // added for ZigBee 2007 and Pro
            BYTE    bRejoin                     : 1;    // added for ZigBee 2007 and Pro
            BYTE    bLeaveInProgress            : 1;    // added for ZigBee 2007 and Pro
            BYTE    bLeaveCmdSent               : 1;    // added for ZigBee 2007 and Pro
            BYTE    bRouteRecordConfirmRcd      : 1;
            BYTE    bScanRequestFromZDO         : 1;    // For indicating ED scan is in progress
                                                        // while sending mgmt_nwk_update_notify.
            BYTE    bChannelSwitch              : 1;    // For Indicating Channel change required
                                                        // after broadcasting Mgmt_Nwk_Update_req.
            BYTE    bTransmitLinkStatusCommand  : 1;
        } bits;
    } flags;

    //#ifndef I_AM_RFD
        BTR                     *BTT[NUM_BUFFERED_BROADCAST_MESSAGES];  // Broadcast Transaction Table
    //#endif
//    #if defined( I_AM_COORDINATOR ) || defined( I_AM_ROUTER )
        TICK                    joinDurationStart;
        BYTE                    joinPermitDuration;
    //#endif
    #if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
        BYTE                    routeRequestID;
        BUFFERED_MESSAGE_INFO   *routingMessages[NUM_BUFFERED_ROUTING_MESSAGES];
        ROUTE_HANDLE_RECORD     routeHandleRecords[NUM_BUFFERED_ROUTING_MESSAGES];
    #endif

    DISCOVERY_INFO              discoveryInfo;
    ASSOCIATE_CAPABILITY_INFO   lastCapabilityInformation;
    DWORD_VAL                   lastScanChannels;
    BYTE                        lastScanDuration;
    SHORT_ADDR                  leaveDeviceAddress;
    #ifndef I_AM_END_DEVICE
        BYTE                    leaveCurrentNode;
        //TICK                    leaveStartTime;   // Not required for ZigBee 2007 and Pro
    #endif
    BYTE                        leaveReason;
//    #ifdef I_AM_COORDINATOR
        PAN_ADDR                requestedPANId;
//    #else
        LONG_ADDR               rejoinExtendedPANId;
        TICK                    rejoinStartTick;
//    #endif
    //#ifdef I_AM_RFD
        BYTE                    rejoinCommandSent;
    //#endif
    #if ( I_SUPPORT_ROUTING_TABLE_AGING == 0x01 )
        TICK                    routingTableAgeTick;
    #endif
    #if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
        BYTE                    addressConflictType;
    #endif

    #if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
        PAN_ID_UPDATE *panIDUpdate;
        PAN_ID_REPORT  PanIDReport;
    #endif
    
    #if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
        TICK    nwkLinkStatusPeriod;
        BYTE    nbtIndexForLinkStatus;
        BYTE    moreLinkStatusCmdPending;
    #endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
    BYTE        mgmtPermitJoiningRspPending;
    SHORT_ADDR  mgmtPermitJoiningRspDstAddr;
    SHORT_ADDR  mgmtPermitJoiningRequestDestination;
     
} NWK_STATUS;
#define NWK_BACKGROUND_TASKS    0x005F

typedef struct _BEACON_PAYLOAD
{
    BYTE        ProtocolId;
    BYTE        bStackProfile       :4;
    BYTE        bnwkProtocolVersion :4;
    BYTE        bReserved           :2;
    BYTE        bRouterCapacity     :1;
    BYTE        bDepth              :4;
    BYTE        bEndDeviceCapacity  :1;
    LONG_ADDR   ExtendedPANId;
    BYTE        TxOffset[3];
    BYTE        nwkUpdateId;
} BEACON_PAYLOAD;

typedef struct _ROUTE_RECORD
{
    SHORT_ADDR destinationAddress;
    BYTE nextHopIndex;
    BYTE prevHopIndex;
}ROUTE_RECORD;


typedef struct _PERSISTENCE_PIB
{
    PAN_ADDR macPANId;
    SHORT_ADDR macShortAddress;
    LONG_ADDR macCoordExtendedAddress;
    SHORT_ADDR macCoordShortAddress;
    BYTE phyCurrentChannel;
    #if I_SUPPORT_LINK_KEY == 1
        SHORT_ADDR  TrustCenterShortAddr;
        LONG_ADDR   TrustCentreLongAddr;
        KEY_VAL linkKey;
        DWORD_VAL   frameCounter;
    #endif

    BYTE    nwkUseMulticast;
    BYTE    nwkMulticastNonMemberRadius;
    BYTE    nwkMulticastMaxNonMemberRadius;

} PERSISTENCE_PIB;

typedef struct _LEAVING_CHILD_DEVICE_DETAILS
{
    SHORT_ADDR  DeviceShortAddress;
    LONG_ADDR   DeviceLongAddress;
} LEAVING_CHILD_DEVICE_DETAILS;

// ******************************************************************************
// Macro Definitions


// ******************************************************************************
// Variable Definitions

extern SHORT_ADDR ParentAddress;

#if I_SUPPORT_CONCENTRATOR ==1
    extern ROUTE_RECORD RouteRecordTable[MAX_ROUTE_RECORD_TABLE_SIZE];
#endif
#if ( I_SUPPORT_PANID_CONFLICT == 0x01 )
    extern BYTE    activatePANIDConflict;
#endif
#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
    extern BYTE    activateAddConfDetection;
#endif
#if defined(I_SUPPORT_SECURITY)
    extern LEAVING_CHILD_DEVICE_DETAILS leavingChildDetails;
#endif

// ******************************************************************************
// Function Prototypes
#define NWKDiscardRx() MACDiscardRx()
void                NWKClearNeighborTable( void );
void                NWKClearRoutingTable( void );
NEIGHBOR_KEY        NWKLookupNodeByLongAddr( LONG_ADDR *longAddr );
BOOL                NWKHasBackgroundTasks( void );
void                NWKTidyNeighborTable( void );
void                NWKInit( void );
ZIGBEE_PRIMITIVE    NWKTasks( ZIGBEE_PRIMITIVE inputPrimitive );
BOOL                NWKThisIsMyLongAddress( LONG_ADDR *address );
BYTE                NLME_GET_nwkBCSN( void );
BOOL                APSFromShortToLong(INPUT SHORT_ADDR *ShortAddr);
BOOL                APSFromLongToShort(INPUT LONG_ADDR *LongAddr);
#endif

#if ( I_SUPPORT_STOCHASTIC_ADDRESSING == 0x01 )
        WORD GenerateNewShortAddr( void );
#ifndef I_AM_END_DEVICE
        ZIGBEE_PRIMITIVE PrepareUnsolicitedRejoinRsp( SHORT_ADDR shortAddress );
        BOOL IsThereAddressConflict( SHORT_ADDR RemoteShortAddr , LONG_ADDR *pRemoteLongAddr );
        ZIGBEE_PRIMITIVE PrepareNwkStatusCmdBroadcast( BYTE statusCode , SHORT_ADDR shortAddress );
#endif
#endif

#if I_SUPPORT_CONCENTRATOR ==1
BYTE GetIndexInRouteRecord( SHORT_ADDR address );
#endif
BYTE IsThisAddressKnown (SHORT_ADDR shortAddr, BYTE * longAddr);
BYTE IsThisLongAddressKnown (LONG_ADDR *longAddr, BYTE * shortAddr, BYTE searchInTable);
BOOL VerifyNetworkManager();
void RestorePersistencePIB();
void StorePersistancePIB();
void ClearNeighborTable(void);
extern BOOL CheckDeviceJoiningPermission(LONG_ADDR ieeeAddr);
