/*********************************************************************
 *
 *                  ZigBee Security Module for MRF24J40
 *
 *********************************************************************
 * FileName:        zSecurity_MRF24J40
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
 * DF/KO/YY             11/27/06    Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY             01/12/07    Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07    Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
#include "zigbee.h"
#include "zigbee.def"

#ifdef I_SUPPORT_SECURITY
#include "ZigbeeTasks.h"
#include "Zigbee.def"
#include "zPHY.h"
#include "zMAC.h"
#include "MSPI.h"
#include "sralloc.h"
#include "generic.h"
#include "zSecurity.h"
#include "zPHY_MRF24J40.h"
#include "zNWK.h"
#include "zNVM.h"

#include "Console.h"

#define CIPHER_RETRY 5
#define SOFTWARE_SECURITY
//#if !defined(I_AM_COORDINATOR)
    //#define HARDWARE_SECURITY
//#endif

//#ifdef I_AM_COORDINATOR
    #define SECURITY_HEADER_LEN 1
//#else
    #define SECURITY_HEADER_LEN 1
//#endif

#define IPAD    0x36
#define OPAD    0x5C
#define BLOCK_SIZE  16
#if (RF_CHIP == MRF24J40) || (RF_CHIP == UZ2400)

typedef enum _CIPHER_MODE
{
    MODE_ENCRYPTION,
    MODE_DECRYPTION
} CIPHER_MODE;

typedef enum _CIPHER_STATUS
{
    CIPHER_SUCCESS = 0,
    CIPHER_ERROR,
    CIPHER_MIC_ERROR
} CIPHER_STATUS;

extern volatile TX_STAT TxStat;
extern BYTE nwkSecurityLevel;
extern volatile PHY_PENDING_TASKS  PHYTasksPending;

DWORD_VAL   OutgoingFrameCount[2];
DWORD_VAL IncomingFrameCount[2][MAX_NEIGHBORS];
SECURITY_STATUS securityStatus;

#if !defined(__C30__) && !defined(__C32__)
    rom const BYTE SwitchTable[8] = {0x00, 0x07, 0x06, 0x05, 0x01, 0x04, 0x03, 0x02};
    rom const BYTE MICTable[8] = {0, 4, 8, 16, 0, 4, 8, 16};
#else
    const BYTE SwitchTable[8] = {0x00, 0x07, 0x06, 0x05, 0x01, 0x04, 0x03, 0x02};
    const BYTE MICTable[8] = {0, 4, 8, 16, 0, 4, 8, 16};
#endif



CIPHER_STATUS PHYCipher(INPUT CIPHER_MODE CipherMode, INPUT SECURITY_INPUT SecurityInput, OUTPUT BYTE *OutData, OUTPUT BYTE *OutDataLen);
void Encode(INPUT SECURITY_INPUT *sinput, OUTPUT BYTE *output);
void AESEncode(unsigned char* block, unsigned char* keyPtr);
void ApplyMatyasMeyerOseasHashfunction(BYTE *message, BYTE length, KEY_VAL *output);
#ifdef USE_EXTERNAL_NVM
const BYTE defaultMasterKey[16] = {NVM_KEY_BYTE00, NVM_KEY_BYTE01,NVM_KEY_BYTE02,NVM_KEY_BYTE03,NVM_KEY_BYTE04,NVM_KEY_BYTE05,NVM_KEY_BYTE06,NVM_KEY_BYTE07,NVM_KEY_BYTE08,NVM_KEY_BYTE09,NVM_KEY_BYTE10,NVM_KEY_BYTE11,NVM_KEY_BYTE12,NVM_KEY_BYTE13,NVM_KEY_BYTE14,NVM_KEY_BYTE15};
NETWORK_KEY_INFO plainSecurityKey[2];
/*********************************************************************
 * Function:        void ApplyHMACAlgorithm(KEY_VAL *k0, BYTE *text, BYTE *output,
 *                   BYTE dataLength)
 *
 * PreCondition:    None
 *
 * Input:           key - pointer to key
 *                       text - pointer to data to which HMAC has to be applied.
 *
 *
 * Output:          transformedKey - the new key generated by applying HMAC
 *                  algorithm
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to apply HMAC alogorithm to the
 *                  given input using the given key.
 ********************************************************************/
 #if I_SUPPORT_LINK_KEY == 1
void ApplyHMACAlgorithm(KEY_VAL *key, BYTE *text, KEY_VAL *transformedKey,
BYTE dataLength)
{
    BYTE i;
    BYTE KeyXorIpad[16];
    BYTE *intermediateMessage;
    /* Exclusive-Or Key with ipad to produce a Block size ( B )-byte string: Key ^ ipad
    since length of key = B */
    for(i = 0; i < KEY_LENGTH; i++)
    {
        KeyXorIpad[i] = key->v[i] ^ IPAD;
    }
    /* Append the stream of data 'text' to the KeyXorIpad (Key ^ ipad) || text. */

    /*Allocate memory for intermediateMessage */
    intermediateMessage = ( BYTE*) SRAMalloc( dataLength + 2*sizeof(KeyXorIpad ) );
    if(intermediateMessage != NULL)
    {
        memcpy(intermediateMessage, KeyXorIpad, KEY_LENGTH);
        memcpy(&intermediateMessage[sizeof(KeyXorIpad)], text, dataLength);

        /* Apply an approved hash function to the stream generated */
        ApplyMatyasMeyerOseasHashfunction(intermediateMessage,dataLength + sizeof(KeyXorIpad ),transformedKey );

    /*  for (i=0; i<16; i++)
        {
            intermediateMessage[i] = 0xC0 + i;
        }
        ApplyMatyasMeyerOseasHashfunction(intermediateMessage,16,transformedKey );*/

        /*Exclusive-Or K0 with opad: K0 ^ opad.*/
        for(i = 0; i < KEY_LENGTH; i++)
        {
            intermediateMessage[i] = key->v[i] ^ OPAD;
        }
        /*Append the result (K0 ? opad) || H((K0 ? ipad) || text)..*/
        for(i = 0; i <  KEY_LENGTH; i++)
        {
            intermediateMessage[i + 16] = transformedKey->v[i];
        }
        ApplyMatyasMeyerOseasHashfunction(intermediateMessage,2*KEY_LENGTH,transformedKey );
        nfree(intermediateMessage);
    }

}
#endif

#if I_SUPPORT_LINK_KEY == 1

/*********************************************************************
 * Function:        BYTE ApplyMatyasMeyerOseasHashfunction(BYTE *message,
 *                   BYTE length, BYTE *output)
 *
 * PreCondition:    None
 *
 * Input:           message - pointer to data to which  Matyas Meyer Oseas Hash
 *                  should be applied.
 *                       length - length of given data.
 *
 *
 * Output:          output - the resultant output.
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to apply Matyas Meyer Oseas Hash
 *                   function
 ********************************************************************/
void ApplyMatyasMeyerOseasHashfunction(BYTE *message, BYTE length, KEY_VAL *output)
{
    BYTE i;
    BYTE j;
    BYTE NoOfBlocks;
    BYTE *ptr = NULL;
    BYTE messagePending;
    WORD_VAL  lengthInBits;
    BYTE blockData[16];
    BYTE plain_block[16];
    NoOfBlocks = length / BLOCK_SIZE;

    lengthInBits.Val = 0x0000;

    for(i = 0; i < KEY_LENGTH; i++ )
    {
        output->v[i] = 0x00;
    }
    for(i = 0; i < NoOfBlocks; i++ )
    {
        ptr = &message[i * BLOCK_SIZE];
        memcpy( blockData,ptr,BLOCK_SIZE);
        memcpy( plain_block,ptr,BLOCK_SIZE);

        AESEncode(&blockData[0], ( BYTE *)output);
        for(j = 0; j < BLOCK_SIZE; j++ )
        {
            output->v[j] = plain_block[j]  ^ blockData[j];
        }

    }
    messagePending = length - ( NoOfBlocks * BLOCK_SIZE );
    if (messagePending )
    {
    /* Right-concatenate to the message M the binary consisting of the bit 1?
        followed by k 0 bits, where k is the smallest non-negative solution to the
        equation: l+1+k = 7n (mod 8n) */
        ptr = &message[NoOfBlocks * BLOCK_SIZE];
        memcpy( blockData,ptr,messagePending);

        /*Put all values as zero  */
        for ( i = 0; i < ( BLOCK_SIZE - messagePending ); i++ )
        {
            blockData[messagePending + i] = 0x00;
        }
    }
    else
    {
        for ( i = 0; i < BLOCK_SIZE; i++ )
        {
            blockData[i] = 0x00;
        }
    }

    blockData[messagePending] = 0x80;
    lengthInBits.Val = length * 8;
    blockData[14] = lengthInBits.byte.HB;
    blockData[15] = lengthInBits.byte.LB;

    memcpy( plain_block,blockData,BLOCK_SIZE);
    AESEncode(&blockData[0], ( BYTE*)output );
    for(j = 0; j < BLOCK_SIZE; j++ )
    {
        output->v[j] = plain_block[j]  ^ blockData[j];
    }
}

#endif

/*********************************************************************
 * Function:        BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey)
 *
 * PreCondition:    None
 *
 * Input:           BYTE    index: the index to the security key, usually either 0 or 1
 *                  NETWORK_KEY_INFO: the network key information to be saved in external NVM
 *
 * Output:          BOOL to indicate if operation is successful
 *
 * Side Effects:    The security keys will be encrypted and stored in the external NVM
 *
 * Overview:        This function is used to encrypt the security key and store it in the external
 *                  NVM.
 ********************************************************************/
BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey)
{
    SECURITY_INPUT SecurityInput;
//  BYTE i;
    BYTE tmpBuf[16];
    LONG_ADDR myAddr;
    #if !defined(__C30__) && !defined(__C32__)
        BYTE Counter;
        BYTE EncryptedLen;
    #endif

    memcpy( &plainSecurityKey[index], &newSecurityKey, sizeof(NETWORK_KEY_INFO) );


    #if defined(__C30__) || defined(__C32__)
        SecurityInput.FrameCounter.Val = 0;
        SecurityInput.SecurityControl.Val = nwkSecurityLevel;
        SecurityInput.SecurityKey = (BYTE *)defaultMasterKey;
       // SecurityInput.KeySeq = 0;
        GetMACAddress(&myAddr);
        SecurityInput.SourceAddress = &myAddr;
        SecurityInput.InputText = newSecurityKey.NetKey.v;
     #ifdef ZCP_DEBUG
        printf("\r\nplain key: ");
     #endif
//        for(i = 0; i < 16; i++) PrintChar(newSecurityKey.NetKey.v[15-i]);

        SecurityInput.TextLen = 16;
        Encode(&SecurityInput, tmpBuf);
       #ifdef ZCP_DEBUG
        printf("\r\nencrypted key: ");
       #endif
/*        for(i = 0; i < 16; i++)
        {
            //currentNetworkKeyInfo.NetKey.v[i] = tmpBuf[i];
            PrintChar(tmpBuf[15-i]);
        }
*/
        currentNetworkKeyInfo.SeqNumber.v[0] = newSecurityKey.SeqNumber.v[0];
        currentNetworkKeyInfo.SeqNumber.v[1] = nwkMAGICResSeq;

        PutNwkKeyInfo((networkKeyInfo+index*sizeof(NETWORK_KEY_INFO)), &currentNetworkKeyInfo);

        return TRUE;

    #else
        SecurityInput.cipherMode = 0x01;
        SecurityInput.FrameCounter.Val = 0;
        SecurityInput.SecurityControl.Val = nwkSecurityLevel;
        SecurityInput.SecurityKey = (BYTE *)defaultMasterKey;
        SecurityInput.KeySeq = 0;
        GetMACAddress(&myAddr);
        SecurityInput.SourceAddress = &myAddr;
        SecurityInput.Header = NULL;
        SecurityInput.HeaderLen = 0;

        Counter = CIPHER_RETRY;
        while(Counter)
        {
            SecurityInput.InputText = newSecurityKey.NetKey.v;
            SecurityInput.TextLen = 16;
            PHYCipher(MODE_ENCRYPTION, SecurityInput, tmpBuf, &EncryptedLen);

            SecurityInput.InputText = tmpBuf;
            SecurityInput.TextLen = EncryptedLen;
            if( PHYCipher(MODE_DECRYPTION, SecurityInput, newSecurityKey.NetKey.v, &i) == CIPHER_SUCCESS )
            {
                break;
            }
            Counter--;
        }

        if( Counter )
        {
            for(i = 0; i < 16; i++)
            {
                currentNetworkKeyInfo.NetKey.v[i] = tmpBuf[i];
            }

            currentNetworkKeyInfo.SeqNumber.v[0] = newSecurityKey.SeqNumber.v[0];
            currentNetworkKeyInfo.SeqNumber.v[1] = nwkMAGICResSeq;

            PutNwkKeyInfo((networkKeyInfo+index*sizeof(NETWORK_KEY_INFO)), &currentNetworkKeyInfo);

            return TRUE;
        }

        return FALSE;
    #endif
}

/*********************************************************************
 * Function:        BOOL InitSecurityKey(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          BOOL to indicate if operation is successful
 *
 * Side Effects:    The security keys stored in external NVM encrypted get decrypted and
 *                  stored in the RAM
 *
 * Overview:        This function is used in the system start up to retrieve security key
 *                  stored in external NVM encrypted and decrypt them and store in the
 *                  RAM for future security operation use
 ********************************************************************/
BOOL InitSecurityKey(void)
{
    SECURITY_INPUT SecurityInput;
    BYTE i;
    LONG_ADDR myAddr;

    for(i = 0; i < 2; i++)
    {
        GetNwkKeyInfo(&currentNetworkKeyInfo, (networkKeyInfo+i*sizeof(NETWORK_KEY_INFO)));
        plainSecurityKey[i].SeqNumber.Val = currentNetworkKeyInfo.SeqNumber.Val;
        if( currentNetworkKeyInfo.SeqNumber.v[1] != nwkMAGICResSeq )
        {
            plainSecurityKey[i].SeqNumber = currentNetworkKeyInfo.SeqNumber;
            continue;
        }

        SecurityInput.cipherMode = 0x01;
        SecurityInput.FrameCounter.Val = 0;
        SecurityInput.SecurityControl.Val = nwkSecurityLevel;
        SecurityInput.SecurityKey = (BYTE *)defaultMasterKey;
    //  SecurityInput.KeySeq = 0;
        GetMACAddress(&myAddr);
        SecurityInput.SourceAddress = &myAddr;
        SecurityInput.Header = NULL;
        SecurityInput.HeaderLen = 0;
        SecurityInput.InputText = currentNetworkKeyInfo.NetKey.v;
        SecurityInput.TextLen = 16;

        #if defined(__C30__) || defined(__C32__)
            Encode(&SecurityInput, plainSecurityKey[i].NetKey.v);
        #else
            if( PHYCipher(MODE_DECRYPTION, SecurityInput, plainSecurityKey[i].NetKey.v, &DataLen) != CIPHER_SUCCESS )
            {
                return FALSE;
            }
        #endif

        plainSecurityKey[i].SeqNumber = currentNetworkKeyInfo.SeqNumber;

    }

    return TRUE;
}


#endif


#ifdef HARDWARE_SECURITY
/*********************************************************************
 * Function:        void SetNonce(INPUT LONG_ADDR *SourceAddress, INPUT DWORD_VAL *FrameCounter, INPUT SECURITY_CONTROL SecurityControl)
 *
 * PreCondition:    Input valid data for nonce
 *
 * Input:           LONG_ADDR *SourceAddress            - Extended source address
 *                  DWORD_VAL *FrameCounter             - FrameCounter
 *                  SECURITY_CONTROL SecurityControl    - Security control byte in auxilary header
 *
 * Output:          None
 *
 * Side Effects:    Security nonce being set
 *
 * Overview:        This function set the security nonce for hardware cipher
 ********************************************************************/
void SetNonce(INPUT LONG_ADDR *SourceAddress, INPUT DWORD_VAL *FrameCounter, INPUT SECURITY_CONTROL SecurityControl)
{
    WORD loc = 0x24C;
    BYTE i;

    for(i = 0; i < 8; i++) {
        PHYSetLongRAMAddr(loc--, SourceAddress->v[i]);
    }

    for(i = 0; i < 4; i++)
    {
        PHYSetLongRAMAddr(loc--, FrameCounter->v[i]);
    }

    PHYSetLongRAMAddr(loc--, SecurityControl.Val | nwkSecurityLevel);
}


/*********************************************************************
 * Function:        CIPHER_STATUS PHYCipher(INPUT CIPHER_MODE CipherMode, INPUT SECURITY_INPUT SecurityInput, OUTPUT BYTE *OutData, OUTPUT BYTE *OutDataLen)
 *
 * PreCondition:    Called by DataEncrypt or DataDecrypt
 *
 * Input:           CIPHER_MODE CipherMode       - Either MODE_ENCRYPTION or MODE_DECRYPTION
 *                  SECURITY_INPUT SecurityInput - Cipher operation input. Filled by DataEncryption or DataDecryption
 *
 * Output:          BYTE *OutData                - Encrypted or decrypted data, including MIC
 *                  BYTE *OutDataLen             - Data length after cipher operation, including MIC bytes
 *                  CIPHER_STATUS                - Cipher operation result
 *
 * Side Effects:    Input data get encrypted or decrypted and put into output buffer
 *
 * Overview:        This is the function that invoke the hardware cipher to do encryption and decryption
 ********************************************************************/
CIPHER_STATUS PHYCipher(INPUT CIPHER_MODE CipherMode, INPUT SECURITY_INPUT SecurityInput, OUTPUT BYTE *OutData, OUTPUT BYTE *OutDataLen)
{
    BYTE CipherRetry = CIPHER_RETRY;
    BYTE i;
    WORD loc;

    // make sure that we are not in the process of sending out a packet
    loc = 0;
    while( !TxStat.finished )
    {
        loc++;
        if( loc > 0xfff )
        {
            PHY_RESETn = 0;
            MACEnable();
            TxStat.finished = 1;
        }

        if( RF_INT_PIN == 0 )
        {
            RFIF = 1;
        }
        Nop();
    }

    RFIE = 0;

CipherOperationStart:
    // step 1, set the normal FIFO
    // step 1a, fill the length of the header
    if( SecurityInput.cipherMode > 0x04 )
    {
        PHYSetLongRAMAddr(0x000, SecurityInput.HeaderLen+SecurityInput.TextLen+14);
    }
    else
    {
        PHYSetLongRAMAddr(0x000, SecurityInput.HeaderLen+14);
    }

    // step 1b, fill the length of the packet
    if( CipherMode == MODE_ENCRYPTION )
    {
        PHYSetLongRAMAddr(0x001, SecurityInput.TextLen+SecurityInput.HeaderLen+14);
    }
    else
    {
        PHYSetLongRAMAddr(0x001, SecurityInput.TextLen+SecurityInput.HeaderLen+16);// two additional bytes FCS
    }
    // step 1c, fill the header
    loc = 0x002;
    for(i = 0; i < SecurityInput.HeaderLen; i++)
    {
        PHYSetLongRAMAddr(loc++, SecurityInput.Header[i]);
    }

    // step 1d, fill the auxilary header
    PHYSetLongRAMAddr(loc++, SecurityInput.SecurityControl.Val | nwkSecurityLevel);
    for(i = 0; i < 4; i++)
    {
        PHYSetLongRAMAddr(loc++, SecurityInput.FrameCounter.v[i]);
    }
    for(i = 0; i < 8; i++)
    {
        PHYSetLongRAMAddr(loc++, SecurityInput.SourceAddress->v[i]);
    }
    PHYSetLongRAMAddr(loc++, SecurityInput.KeySeq);

    // step 1e, fill the data to be encrypted or decrypted
    for(i = 0; i < SecurityInput.TextLen; i++)
    {
        PHYSetLongRAMAddr(loc++, SecurityInput.InputText[i]);
    }

    // step 2, set nounce
    SetNonce(SecurityInput.SourceAddress, &(SecurityInput.FrameCounter), SecurityInput.SecurityControl);

    // step 3, set TXNFIFO security key
    loc = 0x280;
    for(i = 0; i < 16; i++)
    {
        PHYSetLongRAMAddr(loc++, SecurityInput.SecurityKey[i]);
    }

    // step 4, set cipher mode either encryption or decryption
    if( CipherMode == MODE_ENCRYPTION )
    {
        PHYSetShortRAMAddr(SECCR2, 0x40);
    } else {
        PHYSetShortRAMAddr(SECCR2, 0x80);
    }

    // step 5, fill the encryption mode
    PHYSetShortRAMAddr(SECCR0, SecurityInput.cipherMode);
    TxStat.cipher = 1;
    // step 6, trigger

    PHYSetShortRAMAddr(TXNMTRIG, 0x03);

    RFIE = 1;

    i = 0;
    while( TxStat.cipher )
    {
        if( RF_INT_PIN == 0 )
        {
            RFIF = 1;
        }
        i++;
        #if 1
        // in rare condition, the hardware cipher will stall. Handle such condition
        // here
        if(i > 0x1f)
        {
            // in certain rare cases, the RX and Upper Cipher will block each other
            // in case that happens, reset the RF chip to avoid total disfunction
            ConsolePutROMString((ROM char*)"X");
            PHYTasksPending.Val = 0;
            PHY_RESETn = 0;
            MACEnable();
            break;
        }
        #endif
    }

    RFIE = 0;

    // if MIC is generated, check MIC here
    if( (CipherMode == MODE_DECRYPTION) && (SecurityInput.cipherMode != 0x01))
    {
        BYTE MIC_check = PHYGetShortRAMAddr(0x30);
        if( MIC_check & 0x40 )
        {
            PHYSetShortRAMAddr(0x30, 0x40);
            // there is a small chance that the hardware cipher will not
            // decrypt for the first time, retry to solve this problem.
            // details documented in errata
            if( CipherRetry )
            {
                CipherRetry--;
                for(loc = 0; loc < 0x255; loc++);
                goto CipherOperationStart;
            }
            PHY_RESETn = 0;
            MACEnable();
            printf("MIC error");

            RFIE = 1;
            return CIPHER_MIC_ERROR;
        }
    }

    if( TxStat.success )
    {
        // get output data length
        *OutDataLen = PHYGetLongRAMAddr(0x001) - SecurityInput.HeaderLen - 14;
        // get the index of data encrypted or decrypted
        loc = 0x002 + SecurityInput.HeaderLen + 14;

        // if this is a decryption operation, get rid of the useless MIC and two bytes of FCS
        if( CipherMode == MODE_DECRYPTION )
        {

            *OutDataLen -= (2 + MICTable[nwkSecurityLevel]);
        }

        // copy the output data
        for(i = 0; i < *OutDataLen; i++)
        {
            OutData[i] = PHYGetLongRAMAddr(loc++);
        }
        RFIE = 1;
        return CIPHER_SUCCESS;
    }

    RFIE = 1;
    return CIPHER_ERROR;
}

#endif

/*********************************************************************
 * Function:        BYTE SecurityLevel_ZIGBEE_2_IEEE(INPUT BYTE SL_ZigBee)
 *
 * PreCondition:    Input a valid ZigBee security level
 *
 * Input:           BYTE SL_ZigBee - ZigBee security level
 *
 * Output:          BYTE           - IEEE 802.15.4 security level
 *
 * Side Effects:    None
 *
 * Overview:        This function transfers ZigBee security level to IEEE 802.15.4 security level
 ********************************************************************/
//BYTE SecurityLevel_ZIGBEE_2_IEEE(INPUT BYTE SL_ZigBee)
//{
//    BYTE SwitchTable[8] = {0x00, 0x07, 0x06, 0x05, 0x01, 0x04, 0x03, 0x02};
//    return SwitchTable[SL_ZigBee];
//}



#ifdef SOFTWARE_SECURITY

#if defined(__C30__) || defined(__C32__)
    const BYTE flagTable[3] = {0x49, 0x59, 0x79};
    const unsigned char STable[] =
    {
        0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76,
        0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0,
        0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15,
        0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75,
        0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84,
        0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF,
        0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8,
        0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2,
        0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73,
        0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB,
        0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79,
        0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08,
        0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A,
        0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E,
        0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF,
        0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16
    };
#else
    rom const BYTE flagTable[3] = {0x49, 0x59, 0x79};
    rom const unsigned char STable[] =
    {
        0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76,
        0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0,
        0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15,
        0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75,
        0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84,
        0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF,
        0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8,
        0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2,
        0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73,
        0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB,
        0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79,
        0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08,
        0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A,
        0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E,
        0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF,
        0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16
    };
#endif

#define BLOCKSIZE 16
#define xtime(a) (((a)<0x80)?(a)<<1:(((a)<<1)^0x1b) )

static unsigned char _roundCounter;
static unsigned char _rcon;


/*********************************************************************
* Function: void EncKeySchedule(unsigned char* key)
*
* PreCondition:
*
* Input: key - array of the key data
*
* Output:
*
* Side Effects: values of the key changed
*
* Overview: None
*
* Note: None
********************************************************************/
void EncKeySchedule(unsigned char* key)
{
    /* column 1 */
    key[0]^=STable[key[13]];
    key[1]^=STable[key[14]];
    key[2]^=STable[key[15]];
    key[3]^=STable[key[12]];

    key[0]^=_rcon;
    _rcon = xtime(_rcon);

    /* column 2 */
    key[4]^=key[0];
    key[5]^=key[1];
    key[6]^=key[2];
    key[7]^=key[3];

    /* column 3 */
    key[8]^=key[4];
    key[9]^=key[5];
    key[10]^=key[6];
    key[11]^=key[7];

    /* column 4 */
    key[12]^=key[8];
    key[13]^=key[9];
    key[14]^=key[10];
    key[15]^=key[11];
}


/*********************************************************************
* Function: void EncodeShiftRow(unsigned char* stateTable)
*
* PreCondition:
*
* Input: stateTable - the data needing to be shifted
*
* Output:
*
* Side Effects: values of the inputed data changed
*
* Overview: None
*
* Note: None
********************************************************************/
void EncodeShiftRow(unsigned char* stateTable)
{
    unsigned char temp;

    /* first row (row 0) unchanged */

    /* second row (row 1) shifted left by one */
    temp=stateTable[1];
    stateTable[1]=stateTable[5];
    stateTable[5]=stateTable[9];
    stateTable[9]=stateTable[13];
    stateTable[13]=temp;

    /* third row (row 2) shifted left by two */
    temp=stateTable[2];
    stateTable[2]=stateTable[10];
    stateTable[10]=temp;
    temp=stateTable[14];
    stateTable[14]=stateTable[6];
    stateTable[6]=temp;

    /* fourth row (row 3) shifted left by three (or right by one) */
    temp=stateTable[3];
    stateTable[3]=stateTable[15];
    stateTable[15]=stateTable[11];
    stateTable[11]=stateTable[7];
    stateTable[7]=temp;
}


/*********************************************************************
 * Function:        void AESEncode(unsigned char* block, unsigned char* key)
 *
 * PreCondition:    None
 *
 * Input:           block - 16 byte block of data to be encrypted
 *                          (padded if needed)
 *
 * Output:          None
 *
 * Side Effects:    block values changed
 *
 * Stack Requirements: None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
unsigned char key[16];
void AESEncode(unsigned char* block, unsigned char* keyPtr)
{
    unsigned char i;


    /* keep the old key */
    for(i = 0; i < BLOCKSIZE; i++)
    {
        key[i] = keyPtr[i];
    }

    /* initiate round counter = 10 */
    _roundCounter = 10;

    _rcon = 0x01;

    /* key addition */
    for(i=0;i<BLOCKSIZE;i++)
    {
        block[i] ^= key[i];
    }

    do
    {

        /* s table substitution */
        for(i=0;i<BLOCKSIZE;i++)
        {
            block[i]=STable[block[i]];
        }

        /*   encode shift row */
        EncodeShiftRow(block);

        /*   if round counter != 1 */
        if(_roundCounter != 1)
        {
            unsigned char aux,aux1,aux2,aux3;
            /*     mix column */
            for(i=0;i<BLOCKSIZE;i+=4)
            {
                aux1= block[i+0] ^ block[i+1];
                aux3= block[i+2]^block[i+3];
                aux = aux1 ^ aux3;
                aux2= block[i+2]^block[i+1];

                aux1 = xtime(aux1);
                aux2 = xtime(aux2);
                aux3 = xtime(aux3);

                block[i+0]= aux^aux1^block[i+0];
                block[i+1]= aux^aux2^block[i+1];
                block[i+2]= aux^aux3^block[i+2];
                block[i+3]= block[i+0]^block[i+1]^block[i+2]^aux;
            }
        }
        /*   encode key schedule */
        EncKeySchedule(key);

        /*   key addition */
        for(i=0;i<BLOCKSIZE;i++)
        {
            block[i] ^= key[i];
        }

        _roundCounter--;
    }
    while(_roundCounter !=0);
}






void SetAi(INPUT SECURITY_INPUT *SecurityInput, OUTPUT BYTE *outAi)
{
    BYTE i;

    outAi[0] = 1;
    for(i = 0; i < 8; i++)
    {
        outAi[1+i] = SecurityInput->SourceAddress->v[i];
    }
    for(i = 0; i < 4; i++)
    {
        outAi[i+9] = SecurityInput->FrameCounter.v[i];
    }

    outAi[13] = SecurityInput->SecurityControl.Val | nwkSecurityLevel;
    outAi[14] = 0;
    outAi[15] = 0;
}



void Authenticate(INPUT SECURITY_INPUT *sinput, OUTPUT BYTE *output)
{
    BYTE i, j;
    BYTE buffer[BLOCKSIZE];

    // 1a, handle the prefix
    SetAi(sinput, output);

    output[0] = flagTable[nwkSecurityLevel-5];
    output[15] = sinput->TextLen;
    AESEncode(output, sinput->SecurityKey);

    // 1b, handle the header
    j = 0;
    for(i = 0; i < sinput->HeaderLen; i++)
    {
        if ( i == 0 )
        {
            output[j++] ^= 0;
            output[j++] ^= sinput->HeaderLen;
        }
        output[j++] ^= sinput->Header[i];

        if( j == BLOCKSIZE )
        {
            AESEncode(output, sinput->SecurityKey);
            j = 0;
        }
    }
    if( j != 0 )
    {
        AESEncode(output, sinput->SecurityKey);
    }

    // 1c, handle the payload
    j = 0;
    for(i = 0; i < sinput->TextLen; i++)
    {
        output[j++] ^= sinput->InputText[i];
        if( j == BLOCKSIZE )
        {
            AESEncode(output, sinput->SecurityKey);
            j = 0;
        }
    }

    // 1d, handle the encryption of MIC
    // right now, the MIC is stored in output
    // first step to to generate E(A0, Key)
    if( j != 0 )
    {
        AESEncode(output, sinput->SecurityKey);
    }
    SetAi(sinput, buffer);
    AESEncode(buffer, sinput->SecurityKey);

    // generate the encrypted MIC
    for(i = 0; i < BLOCKSIZE; i++)
    {
        output[i] ^= buffer[i];
    }
}

BYTE buffer[BLOCKSIZE];

void Encode(INPUT SECURITY_INPUT *sinput, OUTPUT BYTE *output)
{
    BYTE i, j;
    BYTE Ai15 = 0;

    j = 0;
    for(i = 0; i < sinput->TextLen; i++)
    {
        if( j == 0 )
        {
            SetAi(sinput, buffer);
            buffer[15] = ++Ai15;
            AESEncode(buffer, sinput->SecurityKey);
        }
        output[i] = buffer[j++] ^ sinput->InputText[i];
        if( j == BLOCKSIZE )
        {
            j = 0;
        }
    }
}

BOOL SoftwareCCM_Decrypt(INPUT SECURITY_INPUT *SecurityInput, OUTPUT BYTE *OutData, OUTPUT BYTE *OutDataLen)
{
    BYTE i;
    BYTE *ptr;
    BYTE j;
    BYTE MICLen = MICTable[nwkSecurityLevel];

    SecurityInput->TextLen -= MICLen;

    Encode(SecurityInput, OutData);

    ptr = &(SecurityInput->InputText[SecurityInput->TextLen]);
    SecurityInput->InputText = OutData;

    Authenticate(SecurityInput, buffer);

    for(i = 0; i < MICLen; i++)
    {
        if( ptr[i] != buffer[i] )
        {
     #ifdef ZCP_DEBUG
            printf("\r\nAuth error");
            printf("\r\n MIC:  ");
     #endif
            for(j = 0; j < MICLen; j++)
            {
               #ifdef ZCP_DEBUG

                PrintChar(ptr[j]);
                printf(":");
              #endif
            }
            return FALSE;
        }
    }

    *OutDataLen = SecurityInput->TextLen;
    return TRUE;
}

BOOL SoftwareCCM_Encrypt(INPUT SECURITY_INPUT *SecurityInput, OUTPUT BYTE *OutData, OUTPUT BYTE *OutDataLen)
{
    BYTE MICLen = MICTable[nwkSecurityLevel];

    Authenticate(SecurityInput, &OutData[SecurityInput->TextLen]);

    Encode(SecurityInput, OutData);

    *OutDataLen = SecurityInput->TextLen + MICLen;

    return TRUE;

}

#endif

/*********************************************************************
 * Function:        BOOL GetActiveNetworkKey(KEY_INFO *ActiveNetworkKey)
 * PreCondition:    None
 *
 * Input:          None
 *
 * Output:        ActiveNetworkKey - This pointer will be filled with the
 * key type as network key, active key sequence number, active network key
 * and out going frame counter.
 ********************************************************************/
BOOL GetActiveNetworkKey(KEY_INFO *ActiveNetworkKey)
{
    BYTE ActiveKeyIndex;
    ActiveNetworkKey->keyId = ID_NetworkKey;
    // get security key
    GetNwkActiveKeyNumber(&ActiveKeyIndex);


    if(( ActiveKeyIndex > NUM_NWK_KEYS )&& ( ActiveKeyIndex != 0x00 )) // no valid key
    {
        return FALSE;
    }
    ActiveNetworkKey->frameCounter = (OutgoingFrameCount[ActiveKeyIndex-1]);
    OutgoingFrameCount[ActiveKeyIndex-1].Val++;
    //For every outgoing packet, we should not save the Frame Counter into NVM
    //This will be taken care based on the number of transmission and the timer
    //PutOutgoingFrameCount(OutgoingFrameCount);
     #ifdef USE_EXTERNAL_NVM
        currentNetworkKeyInfo = plainSecurityKey[ActiveKeyIndex-1];
    #else
        GetNwkKeyInfo( &currentNetworkKeyInfo, &(networkKeyInfo[ActiveKeyIndex-1]) );
    #endif
    ActiveNetworkKey->activeKeySeqNo = currentNetworkKeyInfo.SeqNumber.v[0];
    ActiveNetworkKey->key = currentNetworkKeyInfo.NetKey.v;;
    return TRUE;
}

/*********************************************************************
 * Function:        BOOL DataEncrypt(IOPUT BYTE *Input, IOPUT BYTE *DataLen, INPUT BYTE *Header,     INPUT BYTE HeaderLen, KEY_INFO *KeyDetails, BOOL bExtendedNonce)
 * PreCondition:    Input and Header has been filled
 *
 * Input:           BYTE *Header                 - Point to MiWi header
 *                  BYTE HeaderLen               - MiWi header length
 *                  BYTE* KeyDetails - Details of the key to be used for encryption
 *
 * Output:          BOOL           - If data encryption successful
 *
 * Input/Output:    BYTE *Input    - Pointer to the data to be encrypted. The encrypted data will be write back to the pointer
 *                  BYTE *DataLen  - Input as the length of the data to be encrypted. The encrypted data length (including MICs) will be written back
 *
 * Side Effects:    Input data get encrypted and written back to the input pointer
 *
 * Overview:        This is the function that call the hardware cipher to encrypt input data
 ********************************************************************/
BOOL DataEncrypt(IOPUT BYTE *Input, IOPUT BYTE *DataLen, INPUT BYTE *Header, INPUT BYTE HeaderLen, KEY_INFO *KeyDetails, BOOL bExtendedNonce)
{
    SECURITY_INPUT SecurityInput;
    BYTE i;
    LONG_ADDR myAddress;
    #if defined(HARDWARE_SECURITY)
        BYTE EncryptedLen;
        BYTE Counter;
    #endif

    // reserve space for multiple try of encryption
    #ifdef HARDWARE_SECURITY
    BYTE *tmpBuf = (BYTE *)SRAMalloc(*DataLen + 18);
    if( tmpBuf == NULL )
    {
        return FALSE;
    }
    #endif
    // get the IEEE 802.15.4 security mode
    SecurityInput.cipherMode = SwitchTable[nwkSecurityLevel]; // SecurityLevel_ZIGBEE_2_IEEE(nwkSecurityLevel);


    // fill the secuirty input
   // if( KeyDetails->keyId == ID_KeyTransportKey)
   // {
        //SecurityInput.SecurityControl.Val = 0x07 | (KeyDetails->keyId << 3);
//  }
//  else
//  {
        SecurityInput.SecurityControl.Val = nwkSecurityLevel | (KeyDetails->keyId << 3);
//  }
    if( bExtendedNonce )
    {
        SecurityInput.SecurityControl.Val |= 0x20;
    }

    SecurityInput.SecurityKey = KeyDetails->key;
    SecurityInput.KeySeq = KeyDetails->activeKeySeqNo;
    GetMACAddress(&myAddress);
    SecurityInput.SourceAddress = &myAddress;
    SecurityInput.Header = Header;
    SecurityInput.HeaderLen = HeaderLen;
    SecurityInput.FrameCounter = KeyDetails->frameCounter;
    #ifdef SOFTWARE_SECURITY
    // there is an erratum for MRF24J40, the hardware security engine cannot handle
    // any packet with header longer than 31 bytes when processing in CCM mode. As
    // the result, we need to use software security engine.
        if( HeaderLen >= SECURITY_HEADER_LEN && SecurityInput.cipherMode > 0x01 && SecurityInput.cipherMode < 0x05 )
        {
            BYTE auxLen = 5;
            BYTE *ptr;
            volatile BYTE IEFlag;

            #ifdef HARDWARE_SECURITY
                nfree(tmpBuf);
            #endif

            if( bExtendedNonce )
            {
                auxLen += 8;
            }
            if( KeyDetails->keyId == ID_NetworkKey )
            {
                auxLen++;
            }


            // fill the input data and data length
            SecurityInput.InputText = Input;
            SecurityInput.TextLen = *DataLen;
            // leave space for auxilary header
            ptr = Header-auxLen;
            for(i = 0; i < HeaderLen; i++)
            {
                *(ptr++) = *(Header+i);
            }
            // fill in the security auxilary header
            *(ptr++) = (SecurityInput.SecurityControl.Val | nwkSecurityLevel);

            for(i = 0; i < 4; i++)
            {

                    *(ptr++) = SecurityInput.FrameCounter.v[i];

            }



            if( bExtendedNonce )
            {
                for(i = 0; i < 8; i++)
                {
                    *(ptr++) = SecurityInput.SourceAddress->v[i];
                }
            }
            if( KeyDetails->keyId == ID_NetworkKey )
            {
                *ptr = SecurityInput.KeySeq;
            }


            SecurityInput.Header = Header-auxLen;
            SecurityInput.HeaderLen += auxLen;

            // call software cipher and store the output to the temporary buffer
            #if defined(MCHP_C18)
                INTCONbits.GIEH = 0;
            #else
                IEFlag = RFIE;
                RFIE = 0;
            #endif
            SoftwareCCM_Encrypt(&SecurityInput, Input, DataLen);
            #if defined(MCHP_C18)
                INTCONbits.GIEH = 1;
            #else
                RFIE = IEFlag;
            #endif
            *(Header-auxLen+HeaderLen) &= 0xF8; // clear security level
            TxHeader -= auxLen;
            return TRUE;
        }
    #endif

    #ifdef HARDWARE_SECURITY
    // in rare cases, the hardware encryption engine may not suceed for the
    // first time. Retry a few times will solve the problem
    Counter = CIPHER_RETRY;
    while(Counter)
    {
        // fill the input data and data length
        SecurityInput.InputText = Input;
        SecurityInput.TextLen = *DataLen;
        // call hardware cipher and store the output to the temporary buffer
        PHYCipher(MODE_ENCRYPTION, SecurityInput, tmpBuf, &EncryptedLen);

        // try to decrypt the buffer to make sure that encryption is correct
        SecurityInput.InputText = tmpBuf;
        SecurityInput.TextLen = EncryptedLen;
        if( PHYCipher(MODE_DECRYPTION, SecurityInput, Input, &i) == CIPHER_SUCCESS )
        {
            break;
        }
        Counter--;
    }

    // fill the auxilary header
    Input[0] = SecurityInput.SecurityControl.Val & 0xF8; // set security level
    for(i = 0; i < 4; i++)
    {
        Input[i+1] = SecurityInput.FrameCounter.v[i];
    }
    Counter = i+1;
    if( bExtendedNonce )
    {
        for(i = 0; i < 8; i++)
        {
            Input[Counter++] = SecurityInput.SourceAddress->v[i];
        }
    }

    if( KeyDetails->keyId == ID_NetworkKey )
    {
        Input[Counter++] = currentNetworkKeyInfo.SeqNumber.v[0];
    }

    // fill the encrypted data
    for(i = 0; i < EncryptedLen; i++)
    {
        Input[Counter++] = tmpBuf[i];
    }

    nfree(tmpBuf);

    *DataLen = Counter;
    #endif

    return TRUE;
}

/*********************************************************************
 * Function:        BOOL DataDecrypt(IOPUT BYTE *Input, IOPUT BYTE *DataLen, INPUT BYTE *Header, INPUT BYTE HeaderLen, INPUT KEY_IDENTIFIER KeyIdentifier, INPUT LONG_ADDR *longAddr)
 *
 * PreCondition:    Input and Header has been filled
 *
 * Input:           BYTE *Header                    - Point to MiWi header
 *                  BYTE HeaderLen                  - MiWi header length
 *                  KEY_IDENTIFIER KeyIdentifier    - Identifier to specify key type
 *                  LONG_ADDRESS *longAddress       - Extended source address if not use extended nonce
 *
 * Output:          BOOL           - If data encryption successful
 *
 * Input/Output:    BYTE *Input    - Pointer to the data to be decrypted. The decrypted data will be write back to the pointer
 *                  BYTE *DataLen  - Input as the length of the data to be decrypted. The encrypted data length (excluding MICs) will be written back
 *
 * Side Effects:    Input data get decrypted and written back to the input pointer
 *
 * Overview:        This is the function that call the hardware cipher to decrypt input data
 ********************************************************************/
BOOL DataDecrypt(IOPUT BYTE *Input, IOPUT BYTE *DataLen, INPUT BYTE *Header, INPUT BYTE HeaderLen,
    INPUT KEY_IDENTIFIER KeyIdentifier, INPUT LONG_ADDR *longAddr)
{
    SECURITY_INPUT SecurityInput;
    SECURITY_CONTROL SecurityControl;
    BYTE i;
    BYTE Counter;
    DWORD_VAL   FrameCounter;
    LONG_ADDR mySourceAddress;
    BYTE KeySeq;
    BYTE ActiveKeyIndex;
    #if I_SUPPORT_LINK_KEY  == 1
        KEY_VAL linkKeyToBeUsed;
        //#ifndef I_AM_TRUST_CENTER
            APS_KEY_PAIR_DESCRIPTOR appLinkKeyDetails;
            KEY_VAL secretKey;
        //#endif
    #endif
    // retrieve information from auxilary header
    SecurityControl.Val = Input[0];
    Counter = 1;
    for(i = 0; i < 4; i++)
    {
        FrameCounter.v[i] = Input[Counter++];
    }
    if( SecurityControl.bits.ExtendedNonce )
    {
        for(i = 0; i < 8; i++)
        {
            mySourceAddress.v[i] = Input[Counter++];
        }
    }
    else
    {
        for(i = 0; i < 8; i++)
        {
            mySourceAddress.v[i] = longAddr->v[7-i];
        }
    }

    if( SecurityControl.bits.KeyIdentifier == ID_NetworkKey )
    {
        KeySeq = Input[Counter++];
        // get security key based on the key sequence number
        for(i = 0; i < 2; i++)
        {
            #ifdef USE_EXTERNAL_NVM
                currentNetworkKeyInfo = plainSecurityKey[i];
            #else
                GetNwkKeyInfo(&currentNetworkKeyInfo, &networkKeyInfo[i]);
            #endif
            if( KeySeq == currentNetworkKeyInfo.SeqNumber.v[0] && currentNetworkKeyInfo.SeqNumber.v[1] == nwkMAGICResSeq )
            {
                BYTE defaultKeyIndex;

                GetNwkActiveKeyNumber(&defaultKeyIndex);
                if( defaultKeyIndex != (i+1) )
                {
                    NETWORK_KEY_INFO otherKeyInfo;

                    ActiveKeyIndex = ( i == 0 ) ? 1 : 0;
                    #ifdef USE_EXTERNAL_NVM
                        otherKeyInfo = plainSecurityKey[ActiveKeyIndex];
                    #else
                        GetNwkKeyInfo(&otherKeyInfo, &networkKeyInfo[ActiveKeyIndex]);
                    #endif
                    if( otherKeyInfo.SeqNumber.v[1] == nwkMAGICResSeq && KeySeq > otherKeyInfo.SeqNumber.v[0] )
                    {
                        ActiveKeyIndex = i+1;
                        PutNwkActiveKeyNumber(&ActiveKeyIndex);
                    }
                }
                ActiveKeyIndex = i;
                break;
            }
        }
        if( i == 2 )
        {
            return FALSE;
        }
         SecurityInput.SecurityKey = currentNetworkKeyInfo.NetKey.v;
    }
    #if I_SUPPORT_LINK_KEY
    else if ( SecurityControl.bits.KeyIdentifier == ID_LinkKey)
    {
        if(I_AM_TRUST_CENTER){//#ifdef I_AM_TRUST_CENTER
            #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                BYTE count;

                #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                    TC_LINK_KEY_TABLE   tcLinkKeyTable;

                    if ( SearchForTCLinkKey(mySourceAddress, &tcLinkKeyTable) )
                    {
                        for (count = 0; count < KEY_LENGTH; count++)
                        {
                            linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                        }
                    }
                    else
                    {
                        return FALSE;
                    }
                #else
                    for (count = 0; count < KEY_LENGTH; count++)
                    {
                        linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                    }
                #endif
            #else
                LONG_ADDR   SrcAddr;
                GetMACAddress(&SrcAddr);
                GetHashKey(&mySourceAddress, &SrcAddr, &linkKeyToBeUsed);
            #endif
            SecurityInput.SecurityKey = linkKeyToBeUsed.v;
        }else{//#else

            /*check whether the data is from TC  */
            if (!memcmp( &TCLinkKeyInfo.trustCenterLongAddr.v[0], &mySourceAddress, 8 ))
            {
                BYTE count;

                if (securityStatus.flags.bits.bAuthorization)
                {
                    for (count = 0; count < KEY_LENGTH; count++)
                    {
                        linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                    }
                }
                else
                {
                    #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
                        #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                            TC_LINK_KEY_TABLE   tcLinkKeyTable;

                            if ( SearchForTCLinkKey(mySourceAddress, &tcLinkKeyTable) )
                            {
                                for (count = 0; count < KEY_LENGTH; count++)
                                {
                                    linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                                }
                            }
                            else
                            {
                                return FALSE;
                            }
                        #else
                            for (count = 0; count < KEY_LENGTH; count++)
                            {
                                linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                            }
                        #endif
                    #else
                        for (count = 0; count < KEY_LENGTH; count++)
                        {
                            linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                        }
                    #endif
                }
                SecurityInput.SecurityKey = linkKeyToBeUsed.v;
                //FrameCounter = TCLinkKeyInfo.frameCounter;
            }
            else
            {

                BYTE count;
                for(count=0; count < 8; count++)
                {
                    appLinkKeyDetails.PartnerAddress.v[count] = longAddr->v[count];
                }
                appLinkKeyDetails.IncomingFrameCounter = FrameCounter;
                /*0x00 means the Link Key is used for Decryption. In this case, the IncomingFrameCounter
                should be updated to NVM to value as in the received frame.*/
                if( SearchForLinkKey(&appLinkKeyDetails, 0x00) )
                {
                    SecurityInput.SecurityKey = &appLinkKeyDetails.Link_Key.v[0];
                    //FrameCounter = appLinkKeyDetails.IncomingFrameCounter;
                }
                else
                {
                    return FALSE;
                }
            }
        }//#endif     /* I_AM_TRUST_CENTER */

    }
    //#ifndef I_AM_TRUST_CENTER
    else if ( (!I_AM_TRUST_CENTER) &&(SecurityControl.bits.KeyIdentifier == ID_KeyTransportKey))
    {

        BYTE inputToHMAC;
        inputToHMAC = 0;
        ApplyHMACAlgorithm(&TCLinkKeyInfo.link_key, &inputToHMAC, &secretKey, 1);
        SecurityInput.SecurityKey = &secretKey.v[0];
    }
    else if( (!I_AM_TRUST_CENTER) &&(SecurityControl.bits.KeyIdentifier == ID_KeyLoadKey))
    {

        BYTE inputToHMAC;
        BYTE count;

        #if I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1
            #if I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
                TC_LINK_KEY_TABLE   tcLinkKeyTable;

                if ( SearchForTCLinkKey(mySourceAddress, &tcLinkKeyTable) )
                {
                    for (count = 0; count < KEY_LENGTH; count++)
                    {
                        linkKeyToBeUsed.v[count] = tcLinkKeyTable.LinkKey.v[count];
                    }
                }
                else
                {
                    return FALSE;
                }
            #else
                for (count = 0; count < KEY_LENGTH; count++)
                {
                    linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
                }
            #endif
        #else
            for (count = 0; count < KEY_LENGTH; count++)
            {
                linkKeyToBeUsed.v[count] = TCLinkKeyInfo.link_key.v[count];
            }
        #endif

        inputToHMAC = 2;
        ApplyHMACAlgorithm(&linkKeyToBeUsed, &inputToHMAC, &secretKey, 1);
        SecurityInput.SecurityKey = &secretKey.v[0];
    }
    //#endif     /* I_AM_TRUST_CENTER */
    #endif     /* I_SUPPORT_LINK_KEY */
    // fill the security input
    SecurityInput.cipherMode = SwitchTable[nwkSecurityLevel]; //SecurityLevel_ZIGBEE_2_IEEE(nwkSecurityLevel);
    SecurityInput.FrameCounter = FrameCounter;
    SecurityInput.InputText = &(Input[Counter]);
    SecurityInput.SecurityControl = SecurityControl;

   // SecurityInput.KeySeq = KeySeq;
    SecurityInput.SourceAddress = &mySourceAddress;
    SecurityInput.TextLen = *DataLen - Counter;

    #ifdef SOFTWARE_SECURITY
        // there is an erratum for MRF24J40, the hardware security engine cannot handle
        // any packet with header longer than 31 bytes when processing in CCM mode. As
        // the result, we need to use software security engine.
        if( HeaderLen > SECURITY_HEADER_LEN && SecurityInput.cipherMode > 0x01 && SecurityInput.cipherMode < 0x05 )
        {
            BYTE j;
            BYTE *header = (BYTE *)SRAMalloc(HeaderLen + Counter);
            volatile BYTE IEFlag;

            if( header == NULL )
            {
             #ifdef ZCP_DEBUG
                printf("ESOFT_1");
                return FALSE;
             #endif
            }
            for(i = 0; i < HeaderLen; i++)
            {
                header[i] = Header[i];
            }
            header[i++] = Input[0] | nwkSecurityLevel;
            for(j = 1; j < Counter; j++)
            {
                header[i++] = Input[j];
            }
            SecurityInput.Header = header;
            SecurityInput.HeaderLen = HeaderLen + Counter;

            #if defined(MCHP_C18)
                INTCONbits.GIEH = 0;
            #else
                IEFlag = RFIE;
                RFIE = 0;
            #endif
            j = SoftwareCCM_Decrypt(&SecurityInput, Input, DataLen);
            #if defined(MCHP_C18)
                INTCONbits.GIEH = 1;
            #else
                RFIE = IEFlag;
            #endif
            nfree(header);
            if(j == FALSE)
            {
             #ifdef ZCP_DEBUG
                printf("\r\ndecrypt output: ");
              #endif
                for(i = 0; i < *DataLen; i++)
                {
                   #ifdef ZCP_DEBUG
                    PrintChar(Input[i]);
                    printf(":");
                   #endif
                }
            }
            return j;
        }
    #endif

    #ifdef HARDWARE_SECURITY

    SecurityInput.Header = Header;
    SecurityInput.HeaderLen = HeaderLen;
    // call hardware cipher
    if( PHYCipher(MODE_DECRYPTION, SecurityInput, Input, DataLen) != 0 )
    {
        printf("decrpt wrong");
        return FALSE;
    }
    // check the frame counter. make sure that the frame counter increase always
    // we only check family members, because only family members know if a node
    // join or leave the network to reset the frame counter
    if( securityStatus.flags.bits.nwkAllFresh )
    {
        if( INVALID_NEIGHBOR_KEY != (i = NWKLookupNodeByLongAddr(&mySourceAddress)) )
        {
            if( (currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ||
                    currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT ) &&
                FrameCounter.Val < IncomingFrameCount[ActiveKeyIndex][i].Val )
            {
                return FALSE;
            }
            IncomingFrameCount[ActiveKeyIndex][i].Val = FrameCounter.Val;
        }
    }
    #endif

    return TRUE;
}

#else
    #error Please link the appropriate security file for the selected transceiver.
#endif      // RF_CHIP == MRF24J40


#if I_SUPPORT_LINK_KEY
BYTE KeyDerivationFunction( BYTE keyDataLen, BYTE hashLen, BYTE sharedSecretLength,
    BYTE* pSharedSecret, BYTE *pKeyData, BYTE sharedInfolength, BYTE *pSharedInfo )
{

    BYTE i;
    BYTE keyDataArray[50];
    /*the counter to be concatenated with the Shared secret*/
    DWORD counter = 0x00000000;
    BYTE dataLength = 0x00;
    /*if the summation of shared secret length, shared info length and
    the size occupied by the counter is greater than equal to the
    max permitted size of the hash function, return the status as INVALID
    and stop hashing*/
    if( ( sharedSecretLength + sharedInfolength + 0x04 ) >= HASH_MAX_LENGTH )
    {
        return 0x01;
    }
    for( i = 0x00; i < ( keyDataLen/hashLen ); i++ )
    {
        counter++;
        /*copy the shared secret into a single array.The shared secret is of size 21 bytes*/
        memcpy
        (
            &(keyDataArray[dataLength]),
            pSharedSecret,
            sharedSecretLength
        );
        dataLength += sharedSecretLength;

        MemReverseCopy
        (
            (BYTE*)&counter,
            &(keyDataArray[dataLength]),
            sizeof(DWORD)
        );
        dataLength += sizeof(DWORD);
        if( sharedInfolength )
        {
            memcpy
            (
                &(keyDataArray[dataLength]),
                pSharedInfo,
                sharedInfolength
            );
            dataLength += sharedInfolength;
        }
        /*The output - mac key and key data is copied into this array*/
        ApplyMatyasMeyerOseasHashfunction(keyDataArray, dataLength, (KEY_VAL*)( pKeyData + ( i * hashLen ) ));
        dataLength = 0x00;

    }
    return 0x00;
}
#endif /*I_SUPPORT_LINK_KEY*/

#endif /*I_SUPPORT_SECURITY*/
