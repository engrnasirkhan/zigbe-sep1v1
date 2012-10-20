/*********************************************************************
 *
 *                  MSDCL Commissioning Header File
 *
 *********************************************************************
 * FileName        : MSDCL_Commissioning.h
 * Dependencies    :
 * Processor       : PIC18 / PIC24 / PIC32
 * Complier        : MCC18 v3.00 or higher
 *                   MCC30 v2.05 or higher
 *                   MCC32 v1.05 or higher
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
#ifndef _MSDCL_COMMISSIONING_H
#define _MSDCL_COMMISSIONING_H
#include "generic.h"

#define MAX_NUMBER_OF_NETWORK_DETECT		0x0A

#define MSDCL_COMMISSION_DATA_VALID		0x4841
#define MSDCL_DEFAULT_ESP				0x6983
#define MSDCL_DEFAULT_MTR				0x7784

typedef struct __attribute__((packed,aligned(1))) _MSDCL_Commissioning_Cluster_Attributes
{
	WORD			ValidCleanStartUp;
	WORD			StartupStatus;
	DWORD_VAL		ChannelMask;
	BYTE			ExtendedPANId[8];
	BYTE			LinkKey[16];

}MSDCL_Commissioning_Cluster;

typedef struct _ExtPANIdList
{
	LONG_ADDR ExtPANId;
	BOOL ValidData;
}PANIdList;

typedef struct _ExtPANIdReqStatus
{
	BOOL ResquestExtPanId;
	BOOL ReadyResponsePANId;
	BOOL WaitToSendResponse;
}ExtPANIdRequestStatus;

extern WORD MSDCL_Commission_Locations;	//The location of MSDLC_Commission shoudl always be zero in EEPROM
extern MSDCL_Commissioning_Cluster MSDCL_Commission;
extern ExtPANIdRequestStatus ExtPANIDStatus;
#endif

