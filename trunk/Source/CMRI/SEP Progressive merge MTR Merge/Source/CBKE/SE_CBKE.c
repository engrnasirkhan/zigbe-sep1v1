/*********************************************************************
 *
 *                  CBKE
 *
 *********************************************************************
 * FileName:        SE_CBKE.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v2.05 or higher
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
 *
 *****************************************************************************/

/*****************************************************************************
   Includes
 *****************************************************************************/
/* Configuration files */
#include "zigbee.def"

#if I_SUPPORT_CBKE == 1
/* Generic files */
#include "generic.h"
#include "zNVM.h"

/* Stack related files */
#include "zSecurity.h"
#include "zSecurity_MRF24J40.h"
#include "zNWK.h"
#include "zAPS.h"
#include "zZDO.h"

/* ZCL related files */
#include "ZCL_Interface.h"
#include "ZCL_Foundation.h"
#include "ZCL_Key_Establishment.h"

#include "eccapi.h"
#include "SE_CBKE.h"
/* SE related files */
#include "SE_Interface.h"
#include "SE_Profile.h"
#include "SE_Certificate.h"

/*****************************************************************************
   Constants and Enumerations
*****************************************************************************/
/*Key establlishment suite is identified with the value 0x0001*/
#define CBKE_KEY_ESTABLISHMENT_SUITE                    0x0001
/*The below macro gives the location of the device address in the certifcate*/
#define DEVICE_ADDRESS_LOCATION                         0x16
/*This macro defines the location of the issuer address in the certifcate*/
#define ISSUER_ADDRESS_LOCATION                         0x1E
/*The following macros are used for generating the random number*/
#define RAND_M                                          100000000
#define RAND_M1                                         10000
#define RAND_B                                          31415821

#define TERMINATE_KEY_STATUS_LOCATION                   70

/*****************************************************************************
   Customizable Macros
*****************************************************************************/
/* None */

/*****************************************************************************
   Data Structures
*****************************************************************************/
/* None */


/*****************************************************************************
   Variable Definitions
 *****************************************************************************/
 /*This is a global variable that holds the current state of the key
 establishment mechanism.It is initialised with invalid state.*/
BYTE SE_KeyEstStateMachine = INVALID_STATE;
/*This global variable holds the previous state of the key establishment
mechanism. On receipt of key establishment cluster commands, validations
are performed using this state variable. */
BYTE SE_KeyEstPreviousState = INVALID_STATE;

/*This variable is used for generating random nmber*/
SDWORD randomValue = 12;
/*variable to hold the key establishment commands payload.This is used
for temporary storage and will be copied into the allocated memory before
queuing the data for AIL*/
BYTE aPayload[80];
/*This variable holds the destination address to whom terminate key establishment
command need to be sent.*/
WORD terminateKeyDestAddress;
/*destination endpoint for terminate key establishment command*/
BYTE terminateDestEndpoint;
/*This is global instance of structure CBKE_Info. This variable holds the
U ephemeral public, Private key, V Ephemeral Public, private key,
V certificate, Shared secret, Generated Mac key and Key data, MACU and MACV */
CBKE_Info CBKE_KeyDetails;

#if I_SUPPORT_CBKE_TESTING == 1
/*Byte array of 8 bytes for Issuer Address. This has been added for testing the
inter-op test cases. The issuer address is present in the 32 location of the
test certificate */
BYTE aCorruptedCAIssuerAddress[8] = {0x01, 0x22,0x08, 0x00, 0x00, 0x00, 0x00, 0x01};
CBKE_Testing_Interface CBKETestingData;
#endif /*#if I_SUPPORT_CBKE_TESTING == 1*/

/*Store the remote device's ephemeral data generate time and confirm data generate
time*/
BYTE ephemeralDataTimeout = 0x00;
BYTE confirmDataTimeout = 0x00;
CBKE_DataTime CBKE_DataTimeout;


/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
 /*This function creates Initiate key establishment request/response command*/
static void CreateInitiateKeyEstablishment( BYTE *pCertificate, BYTE initiator );
/*This function creates Ephemeral data request/response establishment command*/
static void CreateEphemeralData( BYTE *pEphemeralData, BYTE initiator ) ;
/*This function creates confirm key data request/response command*/
static void CreateConfirmData( BYTE *pConfirmData, BYTE initiator );
/*This function creates Terminate key establishment request/response command*/
static void CreateTerminateKeyEst( BYTE initiator, BYTE status ) ;
/*This function reads the NVM and gets the key establishment suite*/
static WORD GetKeyEstablishmentSuite( void ) ;
/*Random mulitplier*/
static SDWORD RandomMultiplier(SDWORD poly, SDWORD quot);
/*Generates random number*/
static SDWORD RandomGenerate(void);
/*Randmo data function which will be invoked by ECC library*/
static int MCHPGetRandomDataFunc(BYTE *buffer, DWORD sz);
/*yield function*/
static int MCHPYieldFunc(void);
/*Hash function - internall invokes Matyas-Meyer Algorithm*/
static int MCHPHashFunc(unsigned char *digest, unsigned long sz, unsigned char *data);
/*This function generates MACU and MACV*/
static void DeriveMACUAndMACV( BYTE initiator );
/*This sets the established link key into the Link key table maintained
by stack(APS)*/
static void Set_TCLinkKey( BYTE *pIEEEAddress );

/*****************************************************************************
  Private Functions
 *****************************************************************************/

/******************************************************************************
 * Function:        static int MCHPHashFunc(unsigned char *digest, unsigned long sz, unsigned char *data)
 *
 * PreCondition:    None
 *
 * Input:           data - pointer to the data that need to be hashed
 *                  sz - length of the data
 *
 * Output:          digest - pointer to the hashed data. The length of the hashed
 *                  data is 16 bytes.
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function hashes the data given in the pointer data.The
 *                  output hasged data is available in the pointer digest.The data
 *                  is hashed using Matyas Meyer's Algorithm.
 *
 * Note:
 *****************************************************************************/
static int MCHPHashFunc(unsigned char *digest, unsigned long sz, unsigned char *data)
{
    int retVal = 1;
    BYTE i;
    KEY_VAL output;

    ApplyMatyasMeyerOseasHashfunction(data, (BYTE)sz, &output);

    memcpy(digest, &output.v[0], KEY_LENGTH);


    for (i=0; i< KEY_LENGTH; i++)
    {
        if (digest[i] != 0x00)
        {
            retVal = 0;
            break;
        }
    }

    return retVal;

}

/******************************************************************************
 * Function:        static SDWORD RandomMultiplier(SDWORD poly, SDWORD quot)
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:
 *
 * Note:
 *****************************************************************************/
 /* A version of the Linear Congruential Method */
static SDWORD RandomMultiplier(SDWORD poly, SDWORD quot)
{
    SDWORD p1, p0, q1, q0;

    p1 = poly/RAND_M1;
    p0 = poly % RAND_M1;

    q1 = quot/RAND_M1;
    q0 = quot % RAND_M1;

    return (((p0*q1+p1*q0) % RAND_M1) * RAND_M1+p0*q0) % RAND_M;
}

/******************************************************************************
 * Function:        static SDWORD RandomGenerate(void)
 *
 * PreCondition:    None
 *
 * Input:
 *
 * Output:
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:
 *
 * Note:
 *****************************************************************************/
static SDWORD RandomGenerate(void)
{
    randomValue = TMR2;

    randomValue = (RandomMultiplier(randomValue, (SDWORD)RAND_B) + 1) % RAND_M;
    return(randomValue);
}

/******************************************************************************
 * Function:        static int MCHPGetRandomDataFunc(BYTE *buffer, DWORD sz)
 *
 * PreCondition:    buffer - pointer should be allocated statically or dynamically
 *
 * Input:           sz - This gives the length of the random number to be
 *                  generated.
 *
 * Output:          buffer - This is the pointer that holds the generated random
 *                  number.
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function will be invoked by the ECC library. This function
 *                  generates the random number of required size(passed as a parameter).
 *                  The generated random number is placed in the output parameter
 *                  buffer pointer.
 *
 * Note:
 *****************************************************************************/
 /* Certicom API for a sequence of random numbers */
static int MCHPGetRandomDataFunc(BYTE *buffer, DWORD sz)
{
    SDWORD   ranValue;
    SDWORD  index;
    BYTE  byteChunks;

    for(index = 0; index < sz; index++)
    {
       ranValue    = RandomGenerate();
       byteChunks  = (BYTE) (ranValue >> 16);
       *(buffer + index)    = byteChunks;
    }

    return(0);
}

/******************************************************************************
 * Function:        static int MCHPYieldFunc(void)
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
 * Overview:        This function will be invoked by the ECC library.
 *
 * Note:
 *****************************************************************************/
/* Certicom Yield Function */
static int MCHPYieldFunc(void)
{
    int retVal = 0;
    return retVal;
}

/******************************************************************************
 * Function:        static void CreateInitiateKeyEstablishment( BYTE *pCertificate, BYTE initiator )
 *
 * PreCondition:    None
 *
 * Input:           pCertificate -
                        this holds the certifcate of the device to be
 *                      copied into the initiate key establishment request/response command
 *                  initiator -
                        this indicates if the initiate key establishment request/
 *                      response command is generated.
 *                      TRUE - initiate key establishment request command is generated(initiator device)
 *                      FALSE - initiate key establishment response command is generated(Responder device)
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function creates the initiate key establishment request/response
 *                  command in the global variable aPayload. It creates request/response command
 *                  based on the initiator parameter.It copies the certificate into the
 *                  payload array.
 *
 * Note:
 *****************************************************************************/
static void CreateInitiateKeyEstablishment( BYTE *pCertificate, BYTE initiator )
{
    /*get the key establishment suite from the NVM*/
    WORD keyEstablishmentSuite = GetKeyEstablishmentSuite( ) ;
    /*if it is initiator, the direction is client to server, if the command
    is sent by the responder the direction is server to client*/
    if( initiator == TRUE )
    {
        /*The initiator device generates the initiate key establishment request command from
        client to server, cluster specific with default response bit disabled.*/
        aPayload[0] = ZCL_ClientToServer | ZCL_ClusterSpecificCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
        /*The command id is updated as Request command*/
        aPayload[2] = ZCL_KeyEstInitiateKeyEstReq;
    }
    else
    {
        /*The responder device generates the initiate key establishment response command from
        server to client, cluster specific with default response bit disabled.*/
        aPayload[0] = ZCL_ServerToClient | ZCL_ClusterSpecificCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
        /*The command id is updated as response command*/
        aPayload[2] = ZCL_KeyEstInitiateKeyEstResp;
    }
    /*update the transaction sequence number*/
    aPayload[1] = appNextSeqNum++;
    /*include key establishment suite in the payload*/
    aPayload[3] = keyEstablishmentSuite;
    /*take the LSB of key establishment suite*/
    aPayload[4] = ( keyEstablishmentSuite >> 8 ) ;
    /*ephemeral data generate time*/
    aPayload[5] = EPHEMERAL_DATA_GENERATE_TIME;
    /*confirm data generate time*/
    aPayload[6] = CONFIRM_KEY_DATA_GENERATE_TIME;
    /*copy the certificate of size 48 bytes*/
    memcpy
    (
        &( aPayload[7] ),
        pCertificate,
        SECT163K1_CERTIFICATE_SIZE
    );
    #if I_SUPPORT_CBKE_TESTING == 1
        if( TRUE == CBKETestingData.corruptIssuerAddress )
        {
            /*corrupt the CA issuer address*/
            memcpy
            (
                &(aPayload[ (7 + ISSUER_ADDRESS_LOCATION)]),
                aCorruptedCAIssuerAddress,
                MAX_ADDR_LENGTH
            );
        }
        if( TRUE == CBKETestingData.deviceAddress )
        {
            /*corrupt the device address*/
            memcpy
            (
                &(aPayload[ (7 + DEVICE_ADDRESS_LOCATION)]),
                aCorruptedCAIssuerAddress,
                MAX_ADDR_LENGTH
            );
        }

    #endif /*I_SUPPORT_CBKE_TESTING*/

    /*App_SendData - Allocates memory and queues the data for AIL*/
    App_SendData
    (
        APS_ADDRESS_16_BIT,
        (BYTE*)&CBKE_KeyDetails.partnerShortAddress,
        CBKE_KeyDetails.partnerEndpoint,
        ( sizeof(ZCL_KeyEstInitiateKeyReqCommand) + ZCL_HEADER_LENGTH_WO_MANUF_CODE ),
        ZCL_KEY_ESTABLISHMENT_CLUSTER,
        aPayload
     );

}

/******************************************************************************
 * Function:        static void CreateEphemeralData( BYTE *pEphemeralData, BYTE initiator )
 *
 * PreCondition:    None
 *
 * Input:           pEphemeralData -
                        this holds the epehemeral data of size 22 bytes of the device.
 *                  initiator -
                        this indicates if the epehemeral data request/
 *                      response command is generated.
 *                      TRUE - epehemeral data request command is generated(initiator device)
 *                      FALSE - epehemeral data response command is generated(Responder device)
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function creates the epehemeral data request/response
 *                  command in the global variable aPayload. It creates request/response command
 *                  based on the initiator parameter.It copies the epehemeral data into the
 *                  payload array.
 *
 * Note:
 *****************************************************************************/
static void CreateEphemeralData( BYTE *pEphemeralData, BYTE initiator )
{

    /*if it is initiator, the direction is client to server, if the command
    is sent by the responder the direction is server to client*/
    if( initiator == TRUE )
    {
        /*The initiator device generates the epehemeral data request command from
        client to server, cluster specific with default response bit disabled.*/
        aPayload[0] = ZCL_ClientToServer | ZCL_ClusterSpecificCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
        /*The command id is updated as request command*/
        aPayload[2] = ZCL_KeyEstEphemeralDataReq;
    }
    else
    {
        /*The initiator device generates the epehemeral data response command from
        server to client, cluster specific with default response bit disabled.*/
        aPayload[0] = ZCL_ServerToClient | ZCL_ClusterSpecificCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
        /*The command id is updated as response command*/
        aPayload[2] = ZCL_KeyEstEphemeralDataResp;
    }
    /*update the transaction sequence number*/
    aPayload[1] = appNextSeqNum++;
    /*copy the ephemeral data number of size 22 bytes*/
    memcpy
    (
        &( aPayload[3] ),
        pEphemeralData,
        SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
    );
    /*App_SendData - Allocates memory and queues the data for AIL*/
    App_SendData
    (
        APS_ADDRESS_16_BIT,
        (BYTE*)&CBKE_KeyDetails.partnerShortAddress,
        CBKE_KeyDetails.partnerEndpoint,
        ( sizeof(ZCL_KeyEstEphemeralDataReqCommand) + ZCL_HEADER_LENGTH_WO_MANUF_CODE ),
        ZCL_KEY_ESTABLISHMENT_CLUSTER, aPayload
     );

}

/******************************************************************************
 * Function:        static void CreateConfirmData( BYTE *pConfirmData, BYTE initiator )
 *
 * PreCondition:    None
 *
 * Input:           pConfirmData -
 *                       this holds the MACU/MACV data of size 16 bytes of the device.
 *                  initiator -
 *                       this indicates if the epehemeral data request/
 *                      response command is generated.
 *                      TRUE - Confirm Data request command is generated(initiator device)
 *                      FALSE - Confirm data response command is generated(Responder device)
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function creates the Confirm data request/response
 *                  command in the global variable aPayload. It creates request/response command
 *                  based on the initiator parameter.It copies the MACU/MACV data into the
 *                  payload array.
 *
 * Note:
 *****************************************************************************/
static void CreateConfirmData( BYTE *pConfirmData, BYTE initiator )
{
    /*if it is initiator, the direction is client to server, if the command
    is sent by the responder the direction is server to client*/
    if( initiator == TRUE )
    {
        /*The initiator device generates the confirm data request command from
        client to server, cluster specific with default response bit disabled.*/
        aPayload[0] = ZCL_ClientToServer | ZCL_ClusterSpecificCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
        /*The command id is updated as request command*/
        aPayload[2] = ZCL_KeyEstConfirmKeyDataReq;
    }
    else
    {
        /*The initiator device generates the confirm data request command from
        server to client, cluster specific with default response bit disabled.*/
        aPayload[0] = ZCL_ServerToClient | ZCL_ClusterSpecificCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
        /*The command id is updated as response command*/
        aPayload[2] = ZCL_KeyEstConfirmKeyDataResp;
    }
    /*update the transaction sequence number*/
    aPayload[1] = appNextSeqNum++;
    /*copy the Message Authentication code available in the input parameter pConfirmData
    of size 16 bytes*/
    memcpy
    (
        &( aPayload[3] ),
        pConfirmData,
        AES_MMO_HASH_SIZE
    );
    #if I_SUPPORT_CBKE_TESTING == 1
    if( TRUE == CBKETestingData.corruptMAC )
    {
        /*corrupting the Message Authentication code*/
        aPayload[3] += 0x05;
    }
    #endif /*#if I_SUPPORT_CBKE_TESTING == 1*/
    /*App_SendData - Allocates memory and queues the data for AIL*/
    App_SendData
    (
        APS_ADDRESS_16_BIT,
        (BYTE*)&CBKE_KeyDetails.partnerShortAddress,
        CBKE_KeyDetails.partnerEndpoint,
        ( sizeof(ZCL_KeyEstConfirmDataReqCommand) + ZCL_HEADER_LENGTH_WO_MANUF_CODE ),
        ZCL_KEY_ESTABLISHMENT_CLUSTER,
        aPayload
    );
}

/******************************************************************************
 * Function:        static void CreateTerminateKeyEst( BYTE initiator, BYTE status )
 *
 * PreCondition:    None
 *
 * Input:           initiator -
                        this indicates if the terminate key establishment request/
 *                      response command is generated.
 *                      TRUE - Confirm Data request command is generated(initiator device)
 *                      FALSE - Confirm data response command is generated(Responder device)
 *                  status - this gives the status to be included in the terminate
 *                      request/response command.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function generates the terminate key establishment request
                    - response command based on the parameter initiator.It includes
 *                  status in the payload array.
 *
 * Note:
 *****************************************************************************/
static void CreateTerminateKeyEst( BYTE initiator, BYTE status )
{

    /*get the key establishment suite from the NVM*/
    WORD keyEstablishmentSuite = GetKeyEstablishmentSuite( ) ;

    /*if it is initiator, the direction is client to server, if the command
    is sent by the responder the direction is server to client*/
    if( initiator == TRUE )
    {
        /*The initiator device generates the terminate key request command from
        client to server, cluster specific with default response bit disabled.*/
        aPayload[0] = ZCL_ClientToServer | ZCL_ClusterSpecificCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
    }
    else
    {
        /*The initiator device generates the terminate key response command from
        server to client, cluster specific with default response bit disabled.*/
        aPayload[0] = ZCL_ServerToClient | ZCL_ClusterSpecificCommand | ZCL_DEFAULT_RESPONSE_NOT_REQUIRED;
    }
    /*update the transaction sequence number*/
    aPayload[1] = appNextSeqNum++;
    /*update the command id as terminate key establishment command. The value remains same for
    both request/response command*/
    aPayload[2] = ZCL_KeyEstTerminateKeyEst;
    /*update the status*/
    aPayload[3] = status;
    /*update the wait time*/
    aPayload[4] = TERMINATE_KEY_ESTABLISHMENT_WAIT_TIME;
    /*cbke key establishment suite*/
    aPayload[5] = keyEstablishmentSuite;
    /*take the lsb value*/
    aPayload[6] = ( keyEstablishmentSuite >> 0x08 );
    /*App_SendData - Allocates memory and queues the data for AIL*/
    App_SendData
    (
        APS_ADDRESS_16_BIT,
        (BYTE*)&terminateKeyDestAddress,
        terminateDestEndpoint,
        ( sizeof(ZCL_KeyEstTerminateKeyEstCommand) + ZCL_HEADER_LENGTH_WO_MANUF_CODE ),
        ZCL_KEY_ESTABLISHMENT_CLUSTER,
        aPayload
    );
}

/******************************************************************************
 * Function:        static void DeriveMACUAndMACV( BYTE initiator )
 *
 * PreCondition:    None
 *
 * Input:           initiator -
                        this indicates if the terminate key establishment request/
 *                      response command is generated.
 *                      TRUE - Confirm Data request command is generated(initiator device)
 *                      FALSE - Confirm data response command is generated(Responder device)

 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function generates MACU and MACV based on initiator/responder
 *                  device
 *
 * Note1:            In the global instance CBKEKeyDetails, all the fieldnames
 *                  used with 'u' is self device information. All the field names
 *                  used with 'v' is remote device information.
 * Note2:           Unlike general terminology, U does not mean Initiator device
 *                  and V does not mean Responder device in this structure.
 *****************************************************************************/
static void DeriveMACUAndMACV( BYTE initiator )
{
    BYTE MACArray[64];
    BYTE index = 0x00;
    BYTE aSelfIEEEAddress[8];
    GetMACAddress( aSelfIEEEAddress );

    /*Derive the mac key and key data using the Key derivation function,
    shared info is zero, hence NULL is passed*/
    KeyDerivationFunction( KEY_DATA_SIZE,AES_MMO_HASH_SIZE,SECT163K1_SHARED_SECRET_SIZE,
    CBKE_KeyDetails.sharedSecret,CBKE_KeyDetails.generatedMacKeyKeyData, 0, NULL);

    /*Using the API provided by Certicom, get the Static public key of self device using
    the certifcate*/
    ZSE_ECQVReconstructPublicKey(SE_GetCertificate(),SE_GetCAPublicKey(),CBKE_KeyDetails.self_staticPublicKey,
        MCHPHashFunc, MCHPYieldFunc, YIELD_LEVEL);

    /*Using the API provided by Certicom, get the Static public key of remote device using the
    certificate*/
    ZSE_ECQVReconstructPublicKey(CBKE_KeyDetails.partner_certificate,SE_GetCAPublicKey(),
    CBKE_KeyDetails.partner_staticPublicKey,MCHPHashFunc, MCHPYieldFunc, YIELD_LEVEL);

    /*fill the structure message,Generate MACU using HMAC Algorithm*/
    /*If the initiator is TRUE, then it means the MACU and MACV are derived on the
    initiator device*/
    if( TRUE == initiator )
    {
        /*------------------------------------------------------------------
        Note:
        On the Initiator Device to derive MACU, the following message should
        be prepared
        MACU = MAC(Mackkey)( M || ID(U) || ID(V) || E(U) || E(V) )
        -------------------------------------------------------------------*/

        /*additional component for initiator device is 0x02*/
        MACArray[index++] = ADDITIONAL_COMPONENT_FOR_INITIATOR;

        /*copy the IEEE address of the initiator device in the Big Endian
        format.*/
        MemReverseCopy
        (
            aSelfIEEEAddress,
            &MACArray[index],
            MAX_ADDR_LENGTH
        );
        index += MAX_ADDR_LENGTH;

        /*copy the IEEE address of the responder device in the Big Endian
        format.*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.partnerIEEEAddress,
            MAX_ADDR_LENGTH
        );
        index += MAX_ADDR_LENGTH;

        /*copy the Ephemeral Public key of the initiator device*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.self_ephemeralPublicKey,
            SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
        );
        index += SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE;

        /*copy the Ephemeral Public key of the responder device*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.partner_ephemeralPublicKey,
            SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
        );

        index += SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE;

        /*Apply HMAC algorithm to generate the MACU. The parameter CBKE_KeyDetails.generatedMacKeyKeyData holds the
        MAC key. The parameter message should be hashed using the key CBKE_KeyDetails.generatedMacKeyKeyData.
        The generated MACU is received in the parametyer CBKE_KeyDetails.generatedMacU */
        ApplyHMACAlgorithm( (KEY_VAL*)CBKE_KeyDetails.generatedMacKeyKeyData,MACArray ,
            (KEY_VAL*)CBKE_KeyDetails.generatedMacU,index);

        /*------------------------------------------------------------------
        Note:
        On the Initiator Device to derive MACV, the following message should
        be prepared
        MACV = MAC(Mackkey)( M || ID(V) || ID(U) || E(V) || E(U) )
        -------------------------------------------------------------------*/
        index = 0x00;
        /*Generating MACV on the Initiator device*/
        /*Additional component for responder device is 0x03*/
        MACArray[index++] = ADDITIONAL_COMPONENT_FOR_RESPONDER;

        /*copy the IEEE address of the partner device to derive MACV on the initiator device*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.partnerIEEEAddress,
            MAX_ADDR_LENGTH
        );
        index += MAX_ADDR_LENGTH;

        /*copy the IEEE address of the initiator device to derive MACV on the initiator device
        in BIG endian format*/
        MemReverseCopy
        (
            aSelfIEEEAddress,
            &MACArray[index],
            MAX_ADDR_LENGTH
        );
        index += MAX_ADDR_LENGTH;

        /*copy the ephemeral public key of the responder device of size 22 bytes*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.partner_ephemeralPublicKey,
            SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
        );
        index += SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE;

        /*copy the ephemeral public key of the initiator device of size 22 bytes*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.self_ephemeralPublicKey,
            SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
        );
        index += SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE;

        /*Generate MACV using HMAC Algorithm*/
        /*Apply HMAC algorithm to generate the MACV. The parameter CBKE_KeyDetails.generatedMacKeyKeyData holds the
        MAC key. The parameter message should be hashed using the key CBKE_KeyDetails.generatedMacKeyKeyData.
        The generated MACV is received in the parametyer CBKE_KeyDetails.generatedMacV */
        ApplyHMACAlgorithm( (KEY_VAL*)CBKE_KeyDetails.generatedMacKeyKeyData, MACArray,
            (KEY_VAL*)CBKE_KeyDetails.generatedMacV, index );
    }
    /*Generating MACU and MACV on the responder device*/
    else
    {

        /*------------------------------------------------------------------
        Note:
        On the Responder Device to derive MACU, the following message should
        be prepared
        MACU = MAC(Mackkey)( M || ID(U) || ID(V) || E(U) || E(V) )
        -------------------------------------------------------------------*/
        /*Generating MACU on responder device*/

        /*Additional component for Initiator device is 0x02*/
        MACArray[index++] = ADDITIONAL_COMPONENT_FOR_INITIATOR;
        /*
        NOTE:
        For example:
        CBKE_KeyDetails.v_staticPublicKey - On the responder device, this holds the
        static public key of the intiator device.Since for responder device, all the information
        of initiator(remote) device is stored in the field names statring with 'v'*/

        /*copy the IEEE address of the initiator device of size 8 bytes*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.partnerIEEEAddress,
            MAX_ADDR_LENGTH
        );
        index += MAX_ADDR_LENGTH;

        /*copy the IEEE address of the responder device of size 8 bytes*/
        MemReverseCopy
        (
            aSelfIEEEAddress,
            &MACArray[index],
            MAX_ADDR_LENGTH
        );
        index += MAX_ADDR_LENGTH;

        /*copy the ephemeral public key of the initiator device of size 22 bytes*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.partner_ephemeralPublicKey,
            SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
        );
        index += SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE;

        /*copy the ephemeral public key of the responder device of size 22 bytes*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.self_ephemeralPublicKey,
            SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
        );
        index += SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE;

        /*Generate MACV using HMAC Algorithm*/
        /*Apply HMAC algorithm to generate the MACU. The parameter CBKE_KeyDetails.generatedMacKeyKeyData holds the
        MAC key. The parameter message should be hashed using the key CBKE_KeyDetails.generatedMacKeyKeyData.
        The generated MACU is received in the parametyer CBKE_KeyDetails.generatedMacU */
        ApplyHMACAlgorithm( (KEY_VAL*)CBKE_KeyDetails.generatedMacKeyKeyData, MACArray,
            (KEY_VAL*)CBKE_KeyDetails.generatedMacU,index );


        /*------------------------------------------------------------------
        Note:
        On the Responder Device to derive MACV, the following message should
        be prepared
        MACV = MAC(Mackkey)( M || ID(V) || ID(U) || E(V) || E(U) )
        -------------------------------------------------------------------*/

        index = 0x00;

        /*Generating MACV on Responder device*/
        /*Additional component for Responder device is 0x03*/
        MACArray[index++] = ADDITIONAL_COMPONENT_FOR_RESPONDER;

        /*NOTE:
        All self device information is stored in the global instance CBKE_KeyDetails
        with the field names starting with 'u'. For example, on the responder device
        the static public key of responder device is stored in the field
        CBKE_KeyDetails.u_staticPublicKey */
        /*copy the IEEE Address of the responder device of size 22 bytes*/
        MemReverseCopy
        (
            aSelfIEEEAddress,
            &MACArray[index],
            MAX_ADDR_LENGTH
        );
        index += MAX_ADDR_LENGTH;

        /*copy the IEEE Address of the Initiator device of size 22 bytes*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.partnerIEEEAddress,
            MAX_ADDR_LENGTH
        );
        index += MAX_ADDR_LENGTH;

        /*copy the ephemeral public key of the Responder device of size 22 bytes*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.self_ephemeralPublicKey,
            SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
        );
        index += SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE;

        /*copy the ephemeral public key of the Initiator device of size 22 bytes*/
        memcpy
        (
            &MACArray[index],
            CBKE_KeyDetails.partner_ephemeralPublicKey,
            SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE
        );
        index += SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE;

        /*Generate MACV using HMAC Algorithm*/
        /*Apply HMAC algorithm to generate the MACV. The parameter CBKE_KeyDetails.generatedMacKeyKeyData holds the
        MAC key. The parameter message should be hashed using the key CBKE_KeyDetails.generatedMacKeyKeyData.
        The generated MACV is received in the parametyer CBKE_KeyDetails.generatedMacV */
        ApplyHMACAlgorithm( (KEY_VAL*)CBKE_KeyDetails.generatedMacKeyKeyData, MACArray,
            (KEY_VAL*)CBKE_KeyDetails.generatedMacV,index );
    }
}

/******************************************************************************
 * Function:        static WORD GetKeyEstablishmentSuite( void )
 *
 * PreCondition:    None
 *
 * Input:           None

 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function reads the key establishment suite of key
 *                  establishment cluster from NVM>
 * Note:
 *****************************************************************************/
static WORD GetKeyEstablishmentSuite( void )
{
    /*storage index of key establishment cluster*/
    WORD storageIndex;
    /*get the key establishment suite from the NVM*/
    WORD keyEstablishmentSuite;
    /*Get the storage index of Key establishement cluster from NVM. The value is
    0x01 is endpoint id. TBD endpoint is hard coded*/
    ZCL_Callback_GetClusterIndex
    (
        SOURCE_ENDPOINT,
        ZCL_KEY_ESTABLISHMENT_CLUSTER,
        &storageIndex
    );
    /*get the key establishment attribute from NVM, length of the
    key establishment suite is 0x02 bytes*/
    ZCL_Callback_GetAttributeValue
    (
        storageIndex,
        (BYTE*)&keyEstablishmentSuite,
        0x02
    );
    /*return the key establishment suite*/
    return keyEstablishmentSuite;
}

/******************************************************************************
 * Function:        static void Set_TCLinkKey( BYTE *pIEEEAddress )
 *
 * PreCondition:    None
 *
 * Input:           pIEEEAddress - IEEE address whose Application link key is
 *                  to be stored
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function sets the Application Link key estbalished using
 *                  CBKE into the Link key table maintained by the Stack(NVM)
 * Note:
 *****************************************************************************/
static void Set_TCLinkKey( BYTE *pIEEEAddress )
{
    TC_LINK_KEY_TABLE applicationLinkKey;
    /*copy the IEEE address*/
    MemReverseCopy
    (
        pIEEEAddress,
        &(applicationLinkKey.DeviceAddress.v[0]),
        MAX_ADDR_LENGTH
    );
    /*copy the established link key of size 16 bytes*/
    memcpy
    (
        &(applicationLinkKey.LinkKey.v[0]),
        &(CBKE_KeyDetails.generatedMacKeyKeyData[AES_MMO_HASH_SIZE]),
        AES_MMO_HASH_SIZE
    );
    /*Add this entry into the link key table*/
   AddEntryTCLinkKeyTable( &applicationLinkKey );
}

/*****************************************************************************
 Public Functions
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
void SE_CBKE_StateMachineHandler( void )
{
    /*variable to start broadcast delivery time tick*/
    static TICK startBCastDelTimeTick;
    /*the switch case handles the state machines for both initiator device
    and responder device*/
    switch( SE_KeyEstStateMachine )
    {

        case INITIATE_MATCH_DESC_REQ_STATE:
            {
                ZDP_MatchDescriptorRequest match_Desc_req;
                /*get the Trust Center's short address*/
                if( I_AM_TRUST_CENTER ){
                CBKE_KeyDetails.partnerShortAddress = TCLinkKeyInfo.trustCenterShortAddr.Val; //APS_GetTCShortAddress(); MSEB
                }else{
                CBKE_KeyDetails.partnerShortAddress = macPIB.macShortAddress.Val; //APS_GetTCShortAddress(); MSEB
                }
                /*update the destination address with the passed trust center address*/
                match_Desc_req.nwkAddrOfInterest = CBKE_KeyDetails.partnerShortAddress;
                /*update the profile id as SE_Profile id*/
                match_Desc_req.profileId = SE_PROFILE_ID;
                /*number of incluster is 0x01 , since service discovery is performed only for
                key establishment cluster*/
                match_Desc_req.numInClusters = 0x01;
                match_Desc_req.inclusterList[0] = ZCL_KEY_ESTABLISHMENT_CLUSTER;
                /*number of incluster is 0x00 , since service discovery is performed only for
                key establishment cluster supported on input cluster list*/
                match_Desc_req.numOutClusters = 0x00;
                /*Initiate Match descriptor request to determine on which
                endpoint key estbalishment cluster is supported.*/
                SendZDPRequest
                (
                    (WORD_VAL)CBKE_KeyDetails.partnerShortAddress,
                    MATCH_DESC_req,
                    (BYTE*)&match_Desc_req,
                    sizeof(ZDP_MatchDescriptorRequest)
                );
                CBKE_ChangeState( WAIT_FOR_MATCH_DESC_RESP_STATE );
                CBKE_ChangePreviousState( INITIATE_MATCH_DESC_REQ_STATE );
            }
            break;

        case INITIAL_STATE:

            /*change the state to initiate key establishment request state*/
            CBKE_ChangeState( INITIATE_KEY_EST_REQ_STATE );
            CBKE_ChangePreviousState( INITIAL_STATE );
            break;

        case INITIATE_KEY_EST_REQ_STATE:
            {
                /*create the payload for initiate key establishment request command*/
                CreateInitiateKeyEstablishment( SE_GetCertificate(), TRUE );
                /*change the current state to intermediate state*/
                CBKE_ChangeState( INTERMEDIATE_STATE );
                CBKE_ChangePreviousState( INITIATE_KEY_EST_REQ_STATE );
                /*set the CBKE in process bit to TRUE*/
                CBKE_KeyDetails.cmdsReceived.bits.CBKEInProcess = TRUE;
                CBKE_DataTimeout.initiateKeyStartTimer = TRUE;
                CBKE_DataTimeout.initiateKeyStartTime = TickGet();
            }

            break;

        case PROCESS_KEY_REQ_STATE:
            /*create the payload for initiate key establishment response command*/
            CreateInitiateKeyEstablishment( SE_GetCertificate(), FALSE );
            CBKE_ChangeState( INTERMEDIATE_STATE );
            CBKE_ChangePreviousState( PROCESS_KEY_REQ_STATE );
            break;

        case PROCESS_KEY_EST_RESP_STATE:
            /*invoke the ECC api to create the ephemeral public key and create
            ephemeral data request*/
            if( MCE_SUCCESS == ( ZSE_ECCGenerateKey(CBKE_KeyDetails.self_ephemeralPrivateKey,
            CBKE_KeyDetails.self_ephemeralPublicKey, MCHPGetRandomDataFunc, MCHPYieldFunc, YIELD_LEVEL) ) )
            {
                /*create ephemeral data request command*/
                CreateEphemeralData( CBKE_KeyDetails.self_ephemeralPublicKey , TRUE);
                CBKE_ChangePreviousState( PROCESS_KEY_EST_RESP_STATE );
                CBKE_ChangeState( INTERMEDIATE_STATE );
            }
            else
            {
                App_CBKE_Reset();

                /* Application should take the appropriate action on key generation error */ 
                CBKE_Callback_Handle_Status(CBKE_FAILURE);
            }
            break;

        /*received ephemeral data request command and is creating ephemeral data response command*/
        case PROCESS_EPHEMERAL_DATA_REQ_STATE:
            /*invoke the ECC api to create the ephemeral public key*/
            if( MCE_SUCCESS == ( ZSE_ECCGenerateKey(CBKE_KeyDetails.self_ephemeralPrivateKey,
            CBKE_KeyDetails.self_ephemeralPublicKey, MCHPGetRandomDataFunc, MCHPYieldFunc, YIELD_LEVEL)))
            {
                /*create ephemeral data response command using the self public key*/
                CreateEphemeralData( CBKE_KeyDetails.self_ephemeralPublicKey , FALSE);

                /*Generate the MAC key and Key Data using the ECC API*/
                if( MCE_SUCCESS == ZSE_ECCKeyBitGenerate(SE_GetStaticPrivateKey(), CBKE_KeyDetails.self_ephemeralPrivateKey,
                CBKE_KeyDetails.self_ephemeralPublicKey,CBKE_KeyDetails.partner_certificate,
                CBKE_KeyDetails.partner_ephemeralPublicKey, SE_GetCAPublicKey(),CBKE_KeyDetails.sharedSecret,
                MCHPHashFunc, MCHPYieldFunc, YIELD_LEVEL))
                {
                    /*derive the MACU and MACV on the Responder device*/
                    DeriveMACUAndMACV( FALSE );
                    CBKE_ChangePreviousState( PROCESS_EPHEMERAL_DATA_REQ_STATE );
                    CBKE_ChangeState( INTERMEDIATE_STATE );
                }
                else
                {
                    App_CBKE_Reset();
                    
                    /* Application should take the appropriate action on MAC key Data generation error */ 
                    CBKE_Callback_Handle_Status(CBKE_FAILURE);
                }
            }
            else
            {
                App_CBKE_Reset();

                /* Application should take the appropriate action on key generation error */ 
                CBKE_Callback_Handle_Status(CBKE_FAILURE);
            }
            break;

        case INITIATE_TERMINATE_KEY_REQ_STATE:
            if( NOW_I_AM_NOT_A_CORDINATOR() ){	//#if ( I_AM_ESP == 0 )
                /*create terminate key establishment command*/
                CreateTerminateKeyEst( TRUE, aPayload[TERMINATE_KEY_STATUS_LOCATION]);
            }
            else if( NOW_I_AM_A_CORDINATOR() ){	//#else
                /*create terminate key establishment command*/
                CreateTerminateKeyEst( FALSE, aPayload[TERMINATE_KEY_STATUS_LOCATION]);
            }//#endif /*I_AM_ESP*/
            CBKE_ChangePreviousState( INITIATE_TERMINATE_KEY_REQ_STATE );
            CBKE_ChangeState( INTERMEDIATE_STATE );
            break;


        case PROCESS_EPHEMERAL_DATA_RESP_STATE:
        {
            BYTE status;
            /*generate the shared secret by providing the remote device certificate,
            self static private key, ephemeral public and private key and
            the ca public key*/
            if( MCE_SUCCESS == ( status = ZSE_ECCKeyBitGenerate(SE_GetStaticPrivateKey(), CBKE_KeyDetails.self_ephemeralPrivateKey,
            CBKE_KeyDetails.self_ephemeralPublicKey,CBKE_KeyDetails.partner_certificate,
            CBKE_KeyDetails.partner_ephemeralPublicKey, SE_GetCAPublicKey(),CBKE_KeyDetails.sharedSecret,
            MCHPHashFunc, MCHPYieldFunc, YIELD_LEVEL )))
            {
                /*derive the MACU and MACV on the Initiator device*/
                DeriveMACUAndMACV( TRUE );
                /*generate confirm data request*/
                CreateConfirmData( CBKE_KeyDetails.generatedMacU, TRUE );
                CBKE_ChangePreviousState( PROCESS_EPHEMERAL_DATA_RESP_STATE );
                CBKE_ChangeState( INTERMEDIATE_STATE );
            }
            else
            {
                /*Reset that CBKE is not in use*/
                App_CBKE_Reset();
                
                /* Application should take the appropriate action on shared secret generation error */ 
                CBKE_Callback_Handle_Status(CBKE_FAILURE);
            }
        }

            break;

        case PROCESS_CONFIRM_KEY_DATA_REQ_STATE:
            /*Set the generated Application link key for the IEEE address*/
            Set_TCLinkKey( CBKE_KeyDetails.partnerIEEEAddress );
            /*Generate the confirm key data response command*/
            CreateConfirmData( CBKE_KeyDetails.generatedMacV, FALSE );
            CBKE_ChangeState( READY_STATE );
            CBKE_ChangePreviousState( PROCESS_CONFIRM_KEY_DATA_REQ_STATE );
            /* Application should take the appropriate action on Successful CBKE. */ 
            CBKE_Callback_Handle_Status(CBKE_SUCCESS);
            break;

        case PROCESS_CONFIRM_KEY_DATA_RESP_STATE:
            /*On receipt of confirm key data response command on the initiator
            device, set the Application link key*/
            Set_TCLinkKey( CBKE_KeyDetails.partnerIEEEAddress );
            CBKE_ChangeState( READY_STATE );
            CBKE_ChangePreviousState( PROCESS_CONFIRM_KEY_DATA_RESP_STATE );
            /* Application should take the appropriate action on Successful CBKE. */ 
            CBKE_Callback_Handle_Status(CBKE_SUCCESS);
            break;

        case READY_STATE:
            /*Reset that CBKE is not in use as the CBKE is completed*/
            CBKE_KeyDetails.cmdsReceived.bits.CBKEInProcess = FALSE;
            CBKE_ChangePreviousState( INVALID_STATE );
            /*reset the ephemeral data timer*/
            CBKE_DataTimeout.ephemeralDataStartTimer = FALSE;
            /*reset the confirm data timer*/
            CBKE_DataTimeout.confirmDataStartTimer = FALSE;

            break;

        case RECD_TERMINATE_KEY_CONFIRM_STATE:
           
            App_CBKE_Reset();
            
            /* Application should take the appropriate action on terminate key */ 
            CBKE_Callback_Handle_Status(CBKE_FAILURE);
            break;

        case START_BCAST_DEL_TIMER_STATE:
            /*start the timer*/
            startBCastDelTimeTick = TickGet();
            /*change the state to wait for broadcast delivery time*/
            CBKE_ChangeState( WAIT_FOR_BCAST_DEL_TIME_STATE );
        break;

        case WAIT_FOR_BCAST_DEL_TIME_STATE:
            /*check if the time has elapsed.If the time has elapsed,
            then send match descripto request command,else
            wait until it expires*/
            if( TickGetDiff( TickGet(),startBCastDelTimeTick) >
                    ( BROADCAST_DELIVERY_TIMEOUT * ONE_SECOND ) )
            {
                CBKE_ChangeState( INITIATE_MATCH_DESC_REQ_STATE );
            }
        break;

        case INTERMEDIATE_STATE:
        // fall through
        case INVALID_STATE:
        /*fall through*/
        /*do nothing - these states are intermediate states and they perform no action*/
        default:
            break;
    }
    /*if CBKE is in process, check if the ephemeral data request/response is received
    within the specified duration, if not change the state to INVALID_STATE for
    error handling purpose*/
    if( TRUE == CBKE_KeyDetails.cmdsReceived.bits.CBKEInProcess )
    {
        /*check if initiate key establishment response is received or not within timeout
        value, otherwise leave the network*/
        if( TRUE == CBKE_DataTimeout.initiateKeyStartTimer )
        {
          /*check if time is elapsed*/
            if( TickGetDiff( TickGet(),CBKE_DataTimeout.initiateKeyStartTime) >
                ( INITIATE_KEY_RESPONSE_WAIT_TIME * ONE_SECOND ) )
            {
                /*if the previous state is still initiate key establishment request state ,
                that means the device didnt get key establishment response command,
                so leave the network.*/
                if( INITIATE_KEY_EST_REQ_STATE == CBKE_GetPreviousState())
                {
                    App_CBKE_Reset();
                    
                    /* Application should take the appropriate action on CBKE_FAILRE */ 
                    CBKE_Callback_Handle_Status(CBKE_FAILURE);
                }
            }
        }
        /*check if ephemeral data timer started*/
        if( TRUE == CBKE_DataTimeout.ephemeralDataStartTimer )
        {
            /*check if time is elapsed*/
            if( TickGetDiff( TickGet(),CBKE_DataTimeout.ephemeralDataStartTime) >
                ( ephemeralDataTimeout * ONE_SECOND ) )
            {
                /*if we didnt receive the Ephemeral data request /response within the duration
                then change the state to invalid state and reset the CBKE in process bit to false*/
                if( ! ( ( SE_KeyEstStateMachine == PROCESS_EPHEMERAL_DATA_REQ_STATE ) ||
                   ( SE_KeyEstStateMachine == PROCESS_EPHEMERAL_DATA_RESP_STATE ) ) )
                {
                    /*for error handling, change it to invalid values*/
                    CBKE_ChangeState( INVALID_STATE );

                    /* Application should take the appropriate action on CBKE_FAILURE */ 
                    CBKE_Callback_Handle_Status(CBKE_FAILURE);
                    
                    CBKE_KeyDetails.cmdsReceived.bits.CBKEInProcess = FALSE;
                }
                /*reset the timer*/
                CBKE_DataTimeout.ephemeralDataStartTimer = FALSE;
            }
        }
        /*check if confirm data timer is started*/
        if( TRUE == CBKE_DataTimeout.confirmDataStartTimer )
        {
            /*check if time is elapsed*/
            if( TickGetDiff( TickGet(),CBKE_DataTimeout.confirmDataStartTime) >
                ( confirmDataTimeout * ONE_SECOND ) )
            {
                /*if we didnt receive the confirm data request /response within the duration
                then change the state to invalid state and reset the CBKE in process bit to false*/
                if( ! ( ( SE_KeyEstStateMachine == PROCESS_CONFIRM_KEY_DATA_REQ_STATE ) ||
                   ( SE_KeyEstStateMachine == PROCESS_CONFIRM_KEY_DATA_RESP_STATE ) ) )
                {
                    /*change it to invalid states for error handling*/
                    CBKE_ChangeState( INVALID_STATE );
                    
                    /* Application should take the appropriate action on CBKE_FAILRE */ 
                    CBKE_Callback_Handle_Status(CBKE_FAILURE);
                    
                    CBKE_KeyDetails.cmdsReceived.bits.CBKEInProcess = FALSE;
                }
                /*reset the confirm data timer*/
                CBKE_DataTimeout.confirmDataStartTimer = FALSE;
            }
        }
    }
}

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
void HandleKeyEstablishmentCommands( BYTE eventId, APP_DATA_indication* p_dataInd )
{
    BYTE aIEEEAddress[8];
    /*by default copy the destination address to which the terminate key establishment
    command needs to be sent*/
    terminateKeyDestAddress = p_dataInd->SrcAddress.ShortAddr.Val;
    terminateDestEndpoint = p_dataInd->SrcEndpoint;

    switch( eventId )
    {
        case ZCL_KeyEstInitiateKeyEstReqEvent:
        //fall through
        case ZCL_KeyEstInitiateKeyEstRespEvent:
            {
                /*location 0x03 in the asdu points to the initiate key establishment request
                /response command payload*/
                ZCL_KeyEstInitiateKeyReqCommand *pInitiateKey = (ZCL_KeyEstInitiateKeyReqCommand*)&(p_dataInd->asdu[3]);
                /*if initiate key estblishment response event is received before timeout values,
                whether successful or not, reset the timer. If the response is not valid
                terminate key will be sent.*/
                if( ZCL_KeyEstInitiateKeyEstRespEvent == eventId )
                {
                    CBKE_DataTimeout.initiateKeyStartTimer = FALSE;
                }
                /*This condition is required for mulitple initiate key establishement request command
                sent from other devices. When CBKE procedure is executed for one device
                the other device is not allowed.Hence terminate key establishment command with
                status No resources is returned to the device.*/
                if( ( TRUE == CBKE_KeyDetails.cmdsReceived.bits.CBKEInProcess ) &&
                 ( CBKE_KeyDetails.partnerShortAddress != p_dataInd->SrcAddress.ShortAddr.Val ) )
                {
                    aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateNoResources;
                    CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
                    return;
                }
                
                /*get the key establishment suite from the NVM and compare
                the suite*/
                if( ( pInitiateKey->keyEstSuite & CBKE_KEY_ESTABLISHMENT_SUITE )
                    != ( GetKeyEstablishmentSuite( ) & CBKE_KEY_ESTABLISHMENT_SUITE )  )
                {
                    aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateUnSupportedSuite;
                    CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
                    return;
                }
                /*memcompare returns 0 if success and 1 if failure*/
                /*compare the issuer address for self certficate and remote device
                certificate */
                if( ( memcmp( &( pInitiateKey->certificate[ISSUER_ADDRESS_LOCATION]),
                    &( (SE_GetCertificate())[ISSUER_ADDRESS_LOCATION]),MAX_ADDR_LENGTH ) ) )
                {
                    aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateUnknownIssuer;
                    CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
                    return;
                }
                /*Returns TRUE if address is found in the table and updates the
                aIEEE address*/
                if( IsThisAddressKnown ((SHORT_ADDR)p_dataInd->SrcAddress.ShortAddr.Val, aIEEEAddress) )
                {
                    /*memcompare returns 0x00 for success and 0x01 if doesnt match.
                    If the value returned is 0x01, then the IEEE address is not matching
                    hence send bad message*/
                    if(( memcmp( &( pInitiateKey->certificate[DEVICE_ADDRESS_LOCATION]),
                    aIEEEAddress,MAX_ADDR_LENGTH ) ) )
                    {
                        aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateBadMessage;
                        CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
                        return;
                    }
                }
                /*NOTE:All the above conidtions are applicable for Initiate key
                establishment request/response commands*/
                if( eventId == ZCL_KeyEstInitiateKeyEstReqEvent )
                {
                    BYTE deviceIEEEAddress[8];
                    MemReverseCopy(aIEEEAddress, deviceIEEEAddress, 8);
                    if( !CBKE_Callback_CheckForTCLinkKeyTableEntry(deviceIEEEAddress) )
                    {
                        aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateNoResources;
                        CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
                        return;                    
                    }
                    /*validate if the initiate key request command is received from a Trust Center or
                    the received device is a Trust center.otherwise send Terminate key establishment
                     command with No Resources*/
                    if( ( I_AM_TRUST_CENTER && (( p_dataInd->SrcAddress.ShortAddr.Val != TCLinkKeyInfo.trustCenterShortAddr.Val() ) &&
                    APS_GetMyShortAddress() != TCLinkKeyInfo.trustCenterShortAddr.Val())) ||
                        ( !I_AM_TRUST_CENTER && (( p_dataInd->SrcAddress.ShortAddr.Val != macPIB.macShortAddress.Val() ) &&
                    APS_GetMyShortAddress() != macPIB.macShortAddress.Val())) )
                    {
                        aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateNoResources;
                        CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
                        return;
                    }
                    CBKE_ChangeState( PROCESS_KEY_REQ_STATE );
                    CBKE_KeyDetails.partnerShortAddress = p_dataInd->SrcAddress.ShortAddr.Val;
                    /*copy the destination endpoint*/
                    CBKE_KeyDetails.partnerEndpoint = p_dataInd->SrcEndpoint;
                }
                else
                {
                    CBKE_ChangeState( PROCESS_KEY_EST_RESP_STATE );
                }

                /*copy the remote device certificate into the global variable
                of size 48 bytes*/
                memcpy
                (
                    CBKE_KeyDetails.partner_certificate,
                    pInitiateKey->certificate,
                    SECT163K1_CERTIFICATE_SIZE
                );
                /*copy the IEEE address of the destination device*/
                memcpy
                (
                    CBKE_KeyDetails.partnerIEEEAddress,
                    &( CBKE_KeyDetails.partner_certificate[DEVICE_ADDRESS_LOCATION] ),
                    MAX_ADDR_LENGTH
                );
                ephemeralDataTimeout = pInitiateKey->ephemeralDataGenerateTime;
                confirmDataTimeout = pInitiateKey->confirmKeyGenerateTime;
                CBKE_DataTimeout.ephemeralDataStartTimer = TRUE;
                CBKE_DataTimeout.ephemeralDataStartTime = TickGet();
            }
            break;

        /*Handle Ephemeral Data Request or Response*/
        case ZCL_KeyEstEphemeralDataReqEvent:
        case ZCL_KeyEstEphemeralDataRespEvent:
            /*check if the device is in the middle of CBKE mechanism, if a key
            establishment command is received from a third device, then send
            terminate key establishment command*/
            if( ( CBKE_KeyDetails.cmdsReceived.bits.CBKEInProcess ) &&
            ( p_dataInd->SrcAddress.ShortAddr.Val != CBKE_KeyDetails.partnerShortAddress ) )
            {
                aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateBadMessage;
                CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
            }
            /*if ephemeral data request/response command is received and the previous
            key established state is not in the appropriate state, then
            terminate the key establishment command*/
            if( ( ( eventId == ZCL_KeyEstEphemeralDataRespEvent ) &&
                ( CBKE_GetPreviousState() != PROCESS_KEY_EST_RESP_STATE ) ) ||
                ( ( eventId == ZCL_KeyEstEphemeralDataReqEvent ) &&
                ( CBKE_GetPreviousState() != INITIATE_KEY_EST_REQ_STATE ) )
              )
            {
                aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateBadMessage;
                CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
            }
            /*if all the validations are successful , change the current state to create
            ephemeral data request/response command*/
            if( eventId == ZCL_KeyEstEphemeralDataReqEvent )
            {
                CBKE_ChangeState( PROCESS_EPHEMERAL_DATA_REQ_STATE );
            }
            else
            {
                CBKE_ChangeState( PROCESS_EPHEMERAL_DATA_RESP_STATE );
            }
            /*copy the received ephemeral public key both on initiator/responder device
            location 0x03 - points to the ephemeral data number*/
            memcpy( CBKE_KeyDetails.partner_ephemeralPublicKey,&(p_dataInd->asdu[3]),
                SECT163K1_COMPRESSED_PUBLIC_KEY_SIZE );

            CBKE_DataTimeout.confirmDataStartTimer = TRUE;
            CBKE_DataTimeout.confirmDataStartTime = TickGet();
            /*reset the timer*/
            CBKE_DataTimeout.ephemeralDataStartTimer = FALSE;
            break;

        case ZCL_KeyEstConfirmKeyDataReqEvent:
        // fall through
        case ZCL_KeyEstConfirmKeyDataRespEvent:

            /*check if the device is in the middle of CBKE mechanism, if a key
            establishment command is received from a third device, then send
            terminate key establishment command*/
            if( ( CBKE_KeyDetails.cmdsReceived.bits.CBKEInProcess ) &&
            ( p_dataInd->SrcAddress.ShortAddr.Val != CBKE_KeyDetails.partnerShortAddress ) )
            {
                aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateBadMessage;
                CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
            }
            /*if confirm data request/response command is received and the previous
            key established state is not in the appropriate state, then
            terminate the key establishment command*/
            if( ( ( eventId == ZCL_KeyEstConfirmKeyDataRespEvent ) &&
                ( CBKE_GetPreviousState() != PROCESS_EPHEMERAL_DATA_RESP_STATE ) ) ||
                ( ( eventId == ZCL_KeyEstConfirmKeyDataReqEvent ) &&
                ( CBKE_GetPreviousState() != PROCESS_EPHEMERAL_DATA_REQ_STATE ) )
              )
            {
                aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateBadMessage;
                CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
            }
            /*if all the validations are successful , change the current state to create
            confirm data request/response command*/
            if( eventId ==  ZCL_KeyEstConfirmKeyDataReqEvent )
            {
                /*if the MACU generated and received doesn't match then send
                terminate key establishment command with status as Bad key confirm
                memcmp returns 1 if it doesn't match.*/
                if(( memcmp( &(p_dataInd->asdu[3]),CBKE_KeyDetails.generatedMacU,
                        AES_MMO_HASH_SIZE ) ))
                {
                    aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateBad_Key_Confirm;
                    CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
                }
                else
                {
                    CBKE_ChangeState( PROCESS_CONFIRM_KEY_DATA_REQ_STATE );
                }
            }
            if( eventId ==  ZCL_KeyEstConfirmKeyDataRespEvent )
            {
                if( memcmp( &p_dataInd->asdu[3],CBKE_KeyDetails.generatedMacV,AES_MMO_HASH_SIZE ) )
                {
                    /*if MACV doesnt match then send terminate key establishment command*/
                    aPayload[TERMINATE_KEY_STATUS_LOCATION] = ZCL_KeyEstTerminateBad_Key_Confirm;
                    CBKE_ChangeState( INITIATE_TERMINATE_KEY_REQ_STATE );
                }
                else
                {
                    CBKE_ChangeState( PROCESS_CONFIRM_KEY_DATA_RESP_STATE );
                }
            }
            /*reset the confirm data timer*/
            CBKE_DataTimeout.confirmDataStartTimer = FALSE;
            break;

        case ZCL_KeyEstTerminateKeyEstEvent:
            /*terminated key establishment procedure, reset the state machine to invalid_state*/
            CBKE_ChangeState( INVALID_STATE );

            App_CBKE_Reset();
            
            /* Application should take the appropriate action on CBKE_FAILRE */ 
            CBKE_Callback_Handle_Status(CBKE_FAILURE);
            break;

    }
}

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
void App_CBKE_Reset( void )
{
    /*reset the CBKE key details of the remote device*/
    memset
    (
        (BYTE*)&CBKE_KeyDetails,
        0x00,
        sizeof(CBKE_Info)
    );
    /*reset the current state, previous state machines and the terminate
    key destination address*/
    SE_KeyEstStateMachine = INVALID_STATE;
    SE_KeyEstPreviousState = INVALID_STATE;
    terminateKeyDestAddress = 0xFFFF;
    #if I_SUPPORT_CBKE_TESTING == 1
        memset
        (
            (BYTE*)&CBKETestingData,
            0x00,
            sizeof(CBKE_Testing_Interface)
        );
    #endif /*I_SUPPORT_CBKE_TESTING*/
    /*reset the timeout values*/
    memset
    (
        (BYTE*)&CBKE_DataTimeout,
        0x00,
        sizeof(CBKE_DataTime)
    );
}

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
void CBKE_KeyInfoStoreDestEndpoint( BYTE endpoint )
{
    /*update the destination endpoint*/
    CBKE_KeyDetails.partnerEndpoint = endpoint;
}

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
)
{
    /*variable to hold the status as returned by the ECC API*/
    BYTE status;
    /*variable to hold the hash message*/
    BYTE digest[16];
    /*variable to hold the 'r' component of the signature*/
    BYTE rComponent[21];
    /*variable to hold the 's' component of the signature*/
    BYTE sComponent[21];



   if( TRUE == applyDSA )
   {
       /*hash the received message*/
       MCHPHashFunc( digest, messageLength, pMessage );
       if
        (
            /*generate the DSA signature for the hashed message*/
            MCE_SUCCESS == (    status = ZSE_ECDSASign
                                        (
                                            SE_GetStaticPrivateKey(),
                                            digest,
                                            MCHPGetRandomDataFunc,
                                            rComponent,
                                            sComponent,
                                            MCHPYieldFunc,
                                            YIELD_LEVEL
                                        ) )
       )
       {
           /*increment the pointer message by message length to point to
           the signature field.*/
           pMessage += messageLength;
           /*copy the generated 'r' component into the signature field*/
           memcpy
           (
                pMessage,
                rComponent,
                SECT163K1_POINT_ORDER_SIZE
           );
           /*increment the size for 'r' component*/
           pMessage += SECT163K1_POINT_ORDER_SIZE;
           /*copy the 's' component in the signature field*/
           memcpy
           (
                pMessage,
                sComponent,
                SECT163K1_POINT_ORDER_SIZE
           );
       }
   }
   else
   {
        /*copy the 'r' component of the signature received in the report event status
        command to verify the same.42 bytes is subtracted to copy the 'r' component
        start location*/
        memcpy
        (
            rComponent,
            pMessage + messageLength - ( SECT163K1_POINT_ORDER_SIZE + SECT163K1_POINT_ORDER_SIZE ),
            SECT163K1_POINT_ORDER_SIZE
        );
        /*copy the 's' component of the signature received in the report event status
        command to verify the same.21 bytes is subtracted to copy the 's' component
        start location*/
        memcpy
        (
            sComponent,
            pMessage + messageLength - SECT163K1_POINT_ORDER_SIZE,
            SECT163K1_POINT_ORDER_SIZE
        );

        /*hash the received message leaving the signature*/
        MCHPHashFunc
        (
            digest,
            ( messageLength - ( SECT163K1_POINT_ORDER_SIZE + SECT163K1_POINT_ORDER_SIZE ) ),
            pMessage
        );

        /*verify the signature, the ECC library returns the status
        based on the verification.The ECDSA signature should be verified with the Public key
        pait corresponding to the private key.Since the message will be signed using the
        private key, hence the remote device while verifying the signature should use the partner
        device's static public key to verify the signature.*/
        status =    ZSE_ECDSAVerify
                    (
                        CBKE_KeyDetails.partner_staticPublicKey,
                        digest,
                        rComponent,
                        sComponent,
                        MCHPYieldFunc,
                        YIELD_LEVEL
                    );
   }
   /*return status value as returned by the ECC library*/
   return status;
}
#endif /*I_SUPPORT_CBKE*/
