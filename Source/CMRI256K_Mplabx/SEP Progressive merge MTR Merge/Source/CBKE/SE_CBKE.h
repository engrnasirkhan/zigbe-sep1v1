/*********************************************************************
 *
 *                  SE CBKE header file
 *
 *********************************************************************
 * FileName        : SE_CBKE.h
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
#ifndef _SE_CBKE_H
#define _SE_CBKE_H

#if I_SUPPORT_CBKE == 1

/*****************************************************************************
                            Includes
*****************************************************************************/
/* None */

/*****************************************************************************
                      Constants and Enumerations
*****************************************************************************/
/*State machines for key establishment cluster on Initiator Device*/
typedef enum initiatorStates
{
    INITIAL_STATE                           = 0x00,
    INITIATE_KEY_EST_REQ_STATE              = 0x01,
    PROCESS_KEY_REQ_STATE                   = 0x02,
    PROCESS_KEY_EST_RESP_STATE              = 0x03,
    INITIATE_TERMINATE_KEY_REQ_STATE        = 0x04,
    PROCESS_EPHEMERAL_DATA_REQ_STATE        = 0x05,
    PROCESS_EPHEMERAL_DATA_RESP_STATE       = 0x06,
    PROCESS_CONFIRM_KEY_DATA_REQ_STATE      = 0x07,
    PROCESS_CONFIRM_KEY_DATA_RESP_STATE     = 0x08,
    READY_STATE                             = 0x09,
    INTERMEDIATE_STATE                      = 0x0a,
    INITIATE_MATCH_DESC_REQ_STATE           = 0x0b,
    WAIT_FOR_MATCH_DESC_RESP_STATE          = 0x0c,
    RECD_TERMINATE_KEY_CONFIRM_STATE        = 0x0d,
    START_BCAST_DEL_TIMER_STATE             = 0x0e,
    WAIT_FOR_BCAST_DEL_TIME_STATE           = 0x0f,
    INVALID_STATE                           = 0xFF
}initiatorStates;


/*****************************************************************************
                      Customizable Macros
*****************************************************************************/
/*Time taken by the initiator/responder device to generate the
ephemeral public key*/
#define EPHEMERAL_DATA_GENERATE_TIME                0x0d
/*Time taken by the initiator/responder device to generate MACU and MACV*/
#define CONFIRM_KEY_DATA_GENERATE_TIME              0x10

#define INITIATE_KEY_RESPONSE_WAIT_TIME             0x14

#define BROADCAST_DELIVERY_TIMEOUT                  0x03

/*This is the terminate key wait time*/
#define TERMINATE_KEY_ESTABLISHMENT_WAIT_TIME       0x0d //13 seconds
/*total size of mac key and key data is 32 bytes*/
#define KEY_DATA_SIZE                               0x20

#define CBKE_ChangeState( state )                   SE_KeyEstStateMachine = state
#define CBKE_ChangePreviousState( state )           SE_KeyEstPreviousState = state
#define CBKE_GetCurrentState( )                     SE_KeyEstStateMachine
#define CBKE_GetPreviousState( )                    SE_KeyEstPreviousState



#define YIELD_LEVEL                                 0x05
/*this is the additional component for initiator*/
#define ADDITIONAL_COMPONENT_FOR_INITIATOR          0x02
/*this is the additional component for responder*/
#define ADDITIONAL_COMPONENT_FOR_RESPONDER          0x03


/*****************************************************************************
                        Data Structures
*****************************************************************************/
/*structure for storing the values required for CBKE procedure
Fields:
u_staticPublicKey           - self device static public key
u_ephemeralPublicKey        - self device ephemeral public key
u_ephemeralPrivateKey       - Self device ephemeral private key
v_certificate               - remote device's certificate
v_ephemeralPublicKey        - remote device ephemeral public key
v_ephemeralPrivateKey       - remote device ephemeral private key
v_staticPublicKey           - remote device static public key
sharedSecret                - generated shared secret
generatedMacKeyKeyData      - generated MAC key and Key data
                              Key data - is the established Application link key
                              The above are generated using Key derivation function
                              with the generated shared secret.
generatedMacU               - generated MACU using the HMAC Algorithm
generatedMacV               - generated MACV using the HMAC Algorithm
destinationAddress          - 16-bit destination address of the remote device
IEEEDestinationAddress      - 64-bit IEEE address of the remote device
CBKEInProcess               - this keeps the information if CBKE is in process.This is
                                reset when the CBKE mechanism is completed.
*/
typedef struct __attribute__((packed,aligned(1))) CBKE_Info
{
    BYTE self_staticPublicKey[SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE];
    BYTE self_ephemeralPublicKey[SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE];
    BYTE self_ephemeralPrivateKey[SECT163K1_PRIVATE_KEY_SIZE];
    BYTE partner_certificate[SECT163K1_CERTIFICATE_SIZE];
    BYTE partner_ephemeralPublicKey[SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE];
    BYTE partner_ephemeralPrivateKey[SECT163K1_PRIVATE_KEY_SIZE];
    BYTE partner_staticPublicKey[SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE];
    BYTE sharedSecret[SECT163K1_SHARED_SECRET_SIZE];
    BYTE generatedMacKeyKeyData[KEY_DATA_SIZE];
    BYTE generatedMacU[AES_MMO_HASH_SIZE];
    BYTE generatedMacV[AES_MMO_HASH_SIZE];
    BYTE partnerEndpoint;
    WORD partnerShortAddress;
    BYTE partnerIEEEAddress[8];
    union __CMDS_RECEIVED
    {
        BYTE    Val;
        struct __CMDS_RECEIVED_BITS
        {
            BYTE    CBKEInProcess   : 1;
        } bits;
    }cmdsReceived;
}CBKE_Info;

/*Structure - timeout values for ephemeral data generate timeout and
confirm data timeout
*/
typedef struct __attribute__((packed,aligned(1))) CBKE_DataTimeOut
{
    BYTE ephemeralDataStartTimer;
    TICK ephemeralDataStartTime;
    BYTE confirmDataStartTimer;
    TICK confirmDataStartTime;
    BYTE initiateKeyStartTimer;
    TICK initiateKeyStartTime;
}CBKE_DataTime;

#if I_SUPPORT_CBKE_TESTING == 1
typedef struct __attribute__((packed,aligned(1))) CBKE_Testing_Interface
{
    BYTE    corruptIssuerAddress;
    BYTE    deviceAddress;
    BYTE    corruptMAC;
}CBKE_Testing_Interface;
#endif /*I_SUPPORT_CBKE_TESTING*/

typedef struct __attribute__((packed,aligned(1))) ZDP_MatchDescriptorRequest
{
    WORD nwkAddrOfInterest;
    WORD profileId;
    BYTE numInClusters;
    WORD inclusterList[1];
    BYTE numOutClusters;
    WORD outclusterList[1];
}ZDP_MatchDescriptorRequest;
/*****************************************************************************
                      Variable definitions
*****************************************************************************/
extern BYTE SE_KeyEstStateMachine;
extern BYTE SE_KeyEstPreviousState;
extern CBKE_Info CBKE_KeyDetails;

#if I_SUPPORT_CBKE_TESTING == 1
extern CBKE_Testing_Interface CBKETestingData;
#endif /*I_SUPPORT_CBKE_TESTING*/

/*****************************************************************************
                      Function Prototypes
*****************************************************************************/

/******************************************************************************
 * Function:        void SE_CBKE_StateMachineHandler( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function is executed in an infinite loop.The states
 *                  are changed based on the key establishment cluster
 *                  received commands.It initiates the key establishment cluster
 *                  command generation based on the states.
 *
 * Note:
 *****************************************************************************/
void SE_CBKE_StateMachineHandler( void );

/******************************************************************************
 * Function:        void HandleKeyEstablishmentCommands( BYTE eventId, APP_DATA_indication* p_dataInd )
 *
 * PreCondition:    None
 *
 * Input:           eventId - event id based on the received key establishment cluster
 *                  command.
 *                  p_dataInd - received APP data indication.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function is invoked on receipt of Key estbalishment cluster
 *                  commands.
 *
 * Note:
 *****************************************************************************/
void HandleKeyEstablishmentCommands( BYTE eventId, APP_DATA_indication* p_dataInd );

/******************************************************************************
 * Function:        void App_CBKE_Reset( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function is invoked to reset the CBKE details
 *
 * Note:
 *****************************************************************************/
void App_CBKE_Reset( void );

/******************************************************************************
 * Function:        void CBKE_KeyInfoStoreDestEndpoint( BYTE endpoint )
 *
 * PreCondition:    None
 *
 * Input:           endpoint - destination endpoint of the Trust center device
 *                      with whom key establishment should be initiated
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function is invoked to update the detsination endpoint
 *                  before initiating the key establishment command
 *
 * Note:
 *****************************************************************************/
void CBKE_KeyInfoStoreDestEndpoint( BYTE endpoint ) ;

/******************************************************************************
 * Function:        BYTE CBKE_ApplyDSASignature
 *                  (
 *                     BYTE messageLength,
 *                     BYTE* pMessage
 *                  )
 *
 * PreCondition:    None
 *
 * Input:           applyDSA - indicates if the message should be applied
 *                      with DSA or verify the DSA in the message
 *                  messageLength - length of the message
 *                  pMessage - pointer to the message to which signature need
 *                  to be applied.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    Status
 *                  MCE_SUCCESS - if the signature is applied successfully or
 *                  FAILURE
 *
 * Overview:        This function is invoked when the message needs to be
 *                  applied with the signature.
 *
 * Note:
 *****************************************************************************/
BYTE CBKE_ApplyOrVerifyDSASignature
(
    BYTE applyDSA,
    BYTE messageLength,
    BYTE* pMessage
);

#endif /* I_SUPPORT_CBKE */
#endif /*_SE_CBKE_H*/
