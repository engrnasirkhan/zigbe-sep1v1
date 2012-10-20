/**********************************************************************
 * FileName:        zStack_Profile.h
 * Dependencies:
 * Processor:       PIC18
 * Complier:        MCC18 v3.00 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC® microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC microcontroller products.
 *
 * You may not modify or create derivatives works of the Software.
 *
 * If you intend to use the software in a product for sale, then you must
 * be a member of the ZigBee Alliance and obtain any licenses required by
 * them.  For more information, go to www.zigbee.org.
 *
 * The software is owned by the Company and/or its licensor, and is
 * protected under applicable copyright laws. All rights are reserved.
 *
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 ************************************************************************/


#ifndef _ZSTACK_CONFIGURATION_H_
#define _ZSTACK_CONFIGURATION_H_

#define MY_PROFILE_ID                       0x0103
#define MY_PROFILE_ID_MSB                   0x01
#define MY_PROFILE_ID_LSB                   0x03

#define CUSTOMER_CLUSTER_ID1                     0xFC00
#define CUSTOMER_CLUSTER_ID2                     0xFC01
#define CUSTOMER_CLUSTER_ID3                     0xFC12
#define CUSTOMER_CLUSTER_ID4                     0xFC14
#define CUSTOMER_PROFILE_ID                     0xBF0D 

//#define CUSTOMER_CLUSTER_ID                     0xFC01
//#define CUSTOMER_PROFILE_ID                     0x0104 
#define CUSTOMER_APP_ENDPOINT               0x08              
//#define CUSTOMER_CLUSTER_ID                     0xFC01
//#define CUSTOMER_PROFILE_ID                     0x0104               
#define TEST_PROFILE_ID                     0x7f01
#define TESTZCP_PROFILE_ID_MSB              0x7f
#define TESTZCP_PROFILE_ID_LSB              0x01

#define TEST_DRIVER_DEV_ID      0x0000   //test driver
#define TEST_DRIVER_DEV_ID_MSB  0x00
#define TEST_DRIVER_DEV_ID_LSB  0x00
#define TEST_DRIVER_DEV_VER     0x00

#define KVP_DEV_ID      0x5555   //KVP device under test
#define KVP_DEV_ID_MSB  0x55
#define KVP_DEV_ID_LSB  0x55
#define KVP_DEV_VER     0x00

#define MSG_DEV_ID        0xaaaa   //MSG device under test
#define MSG_DEV_ID_MSB    0xaa
#define MSG_DEV_ID_LSB    0xaa
#define MSG_DEV_VER       0x00

#define FULL_DEV_ID       0xffff   //Full device under test
#define FULL_DEV_ID_MSB   0x00
#define FULL_DEV_ID_LSB   0x00
#define FULL_DEV_VER      0x00

// Transceiver Information
    // #define ALLOWED_CHANNELS 0x00004000  
   //   #define ALLOWED_CHANNELS 0x04000000
  	#define ALLOWED_CHANNELS_PRE_CONFIG 0x07FFF800UL		//ALL CHANEELS
	//#define ALLOWED_CHANNELS_PRE_CONFIG 0x0003F800UL
	//#define ALLOWED_CHANNELS_PRE_CONFIG 0x07FFF800UL
	
extern unsigned long ALLOWED_CHANNELS;	

#if (I_SUPPORT_ROUTING_TABLE_AGING == 0x01)
    #define ROUTING_TABLE_CLEANUP_PERIOD    0x000A  //In Seconds
    #define ROUTING_TABLE_AGE_LIMIT         0x05    //In Byte Counter
#endif

// Link Status Command Settings
#if ( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )
	#define NWK_LINK_STATUS_PERIOD	0x20 * ONE_SECOND
    #define NWK_ROUTER_AGE_LIMIT    0x06
#endif //( I_SUPPORT_LINK_STATUS_COMMANDS == 0x01 )

// Multicast related parameters
#define DEFAULT_nwkUseMulticast   0x00
#define DEFAULT_nwkMulticastNonMemberRadius 0x07
#define DEFAULT_nwkMulticastMaxNonMemberRadius 0x07

#endif
