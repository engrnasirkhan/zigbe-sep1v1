/*********************************************************************
 *
 *                  Security Header File for MRF24J40
 *
 *********************************************************************
 * FileName:        zSecurity_MRF24J40.h
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
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY             01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

 #ifdef I_SUPPORT_SECURITY

 #ifndef ZSECURITY_MRF24J40_H
 #define ZSECURITY_MRF24J40_H

 /*max hash length is considered as 255.This is because for one byte variable,
maximum value that can be stored in the variable is 255*/
 #define HASH_MAX_LENGTH    0xFF

 typedef struct _SECURITY_INPUT
{
    BYTE    *InputText;
    BYTE    *SecurityKey;
    BYTE    *Header;
    BYTE    TextLen;
    BYTE    KeySeq;
    BYTE    cipherMode;
    BYTE    HeaderLen;
    LONG_ADDR   *SourceAddress;
    DWORD_VAL   FrameCounter;
    SECURITY_CONTROL    SecurityControl;
} SECURITY_INPUT;

extern SECURITY_INPUT SecurityInput;
extern DWORD_VAL FrameCounter;
BOOL GetActiveNetworkKey(KEY_INFO *ActiveNetworkKey);
BOOL DataEncrypt(IOPUT BYTE *Input, IOPUT BYTE *DataLen, INPUT BYTE *Header, INPUT BYTE HeaderLen, KEY_INFO *KeyDetails, BOOL bExtendedNonce);
BOOL DataDecrypt(IOPUT BYTE *Input, IOPUT BYTE *DataLen, INPUT BYTE *Header, INPUT BYTE HeaderLen, INPUT KEY_IDENTIFIER KeyIdentifier, INPUT LONG_ADDR *SourceAddress);
void ApplyHMACAlgorithm(KEY_VAL *key, BYTE *text, KEY_VAL *transformedKey, BYTE dataLength);
#if I_SUPPORT_LINK_KEY
    BYTE KeyDerivationFunction( BYTE keyDataLen, BYTE hashLen, BYTE sharedSecretLength,
    BYTE* pSharedSecret, BYTE *pKeyData, BYTE sharedInfolength, BYTE *pSharedInfo );
    void ApplyMatyasMeyerOseasHashfunction(BYTE *message, BYTE length, KEY_VAL *output);
#endif /*I_SUPPORT_LINK_KEY*/
#endif /*ZSECURITY_MRF24J40_H*/
#endif /*I_SUPPORT_SECURITY*/
