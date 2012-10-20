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


#ifndef _ZSTACK_PROFILE_H_
#define _ZSTACK_PROFILE_H_

#define ZIGBEE_2007_STACK_PROFILE           0x01
#define ZIGBEE_PRO_STACK_PROFILE            0x00	//0x02 orginally it was 0x02

#define ONE_MILLISECOND                     ONE_SECOND/1000     //Intepretation for one millisecond

#define AES_ENCRIPT_DECRIPT_TIME            200 * ONE_MILLISECOND

// ------------------------------------------------------------------------
// ZIGBEE 2007 STACK PROFILE SETTINGS
// ------------------------------------------------------------------------

#if ( ZIGBEE_2007 == 0x01)
    // NWK LAYER CONFIGURATIONS
    #define PROFILE_myStackProfileId        ZIGBEE_2007_STACK_PROFILE
    #define PROFILE_nwkMaxChildren          20
    #define PROFILE_nwkMaxDepth             5
    #define PROFILE_nwkMaxRouters           6
    #define PROFILE_nwkReportConstantCost   FALSE
    #define PROFILE_nwkSymLink              FALSE

    // SEC CONFIGURATIONS
    #define PROFILE_nwkSecurityLevel            0x05
    #define PROFILE_nwkSecureAllFrames          0x01
    #define PROFILE_apsSecurityTimeoutPeriod    ( ( 50 * ONE_MILLISECOND ) * ( 2 * PROFILE_nwkMaxDepth ) + ( AES_ENCRIPT_DECRIPT_TIME ) )
#endif


// ------------------------------------------------------------------------
// ZIGBEE PRO STACK PROFILE SETTINGS
// ------------------------------------------------------------------------

#if ( ZIGBEE_PRO == 0x01)
    // NWK LAYER CONFIGURATIONS
    #define PROFILE_myStackProfileId        ZIGBEE_PRO_STACK_PROFILE
    #define PROFILE_nwkMaxChildren          MAX_NEIGHBORS
    #define PROFILE_nwkMaxDepth             15
    #define PROFILE_nwkMaxRouters           MAX_NEIGHBORS
    #define PROFILE_nwkReportConstantCost   FALSE
    #define PROFILE_nwkSymLink              TRUE

    // APS LAYER CONFIGURATIONS
    #define PROFILE_FRAGMENT_WINDOW_SIZE            1 //range from 1 to 8. Can't exceed more than 8.
    #define PROFILE_FRAGMENT_INTERFRAME_DELAY       200 //in miliseconds

    // SEC CONFIGURATIONS
    #define PROFILE_nwkSecurityLevel            0x05
    #define PROFILE_nwkSecureAllFrames          0x01
    #define PROFILE_apsSecurityTimeoutPeriod    ( ( 50 * ONE_MILLISECOND ) * ( 2 * PROFILE_nwkMaxDepth ) + ( AES_ENCRIPT_DECRIPT_TIME ) )
#endif


//******************************************************************************
// Distributed Address Assignment Constants
//
// These should be calculated manually and placed here.  They are calculated by
// the following formulae.  CSKIP values should be generated until the max
// depth is reached or until CSKIP equals 0.
//
//  Cskip(d) =  if PROFILE_nwkMaxRouters is 1 =
//                  1 + Cm * (Lm - d - 1)
//              otherwise =
//                  1 + Cm - Rm - (Cm * Rm^(Lm - d - 1))
//                  ------------------------------------
//                                1 - Rm
//  where
//      Cm = PROFILE_nwkMaxChildren
//      Lm = PROFILE_nwkMaxDepth
//      Rm = PROFILE_nwkMaxRouters
//      d  = depth of node in the network

#define CSKIP_DEPTH_0                       0x143D
#define CSKIP_DEPTH_1                       0x035D
#define CSKIP_DEPTH_2                       0x008D
#define CSKIP_DEPTH_3                       0x0015
#define CSKIP_DEPTH_4                       0x0001
#define CSKIP_DEPTH_5                       0x0000


//******************************************************************************
// Mandatory Clusters

#define MANAGE_NO_DATA_ATTRIBUTE_CLUSTER            0x0000  //Manage no data attribute
#define TRANSMIT_COUNTED_PACKETS_CLUSTER            0x0001  //Transmit Counted packets
#define RESET_PACKET_COUNT_CLUSTER                  0x0002  //Reset packet count
#define RETRIEVE_PACKET_COUNT_CLUSTER               0x0003  //Retrieve packet count
#define PACKET_COUNT_RESPONSE_CLUSTER               0x0004  //Packet count response
#define BUFFER_TEST_REQUEST_CLUSTER                 0x001C  //Buffer test request
#define MANAGE_8BIT_INTEGER_ATTRIBUTES_CLUSTER      0x0038  //Mange 8-bit integer attributes
#define BUFFER_TEST_RESPONSE_CLUSTER                0x0054  //Buffer test response
#define MANAGE_16BIT_INTEGER_ATTRIBUTES_CLUSTER     0x0070  //Manage 16-bit integer attributes
#define MANAGE_SEMI_PRECISION_ATTRIBUTES_CLUSTER    0x008C  //Manage semi-precision attribute
#define FREEFORM_MSG_REQUEST_CLUSTER                0xa0A8  //Freeform MSG request
#define MANAGE_TIME_ATTRIBUTES_CLUSTER              0x00C4  //Manage time atributes
#define FREEFORM_MSG_RESPONSE_CLUSTER               0xe000  //Freeform MSG response
#define MANAGE_STRING_ATTRIBUTES_CLUSTER            0x00FF  //Manage string attributes
#define REVERSE_FREEFORM_MSG_RESPONSE_CLUSTER       0x00e0  //To get ZCP tc 3.25 to pass



//******************************************************************************
// Mandatory Cluster Attributes

//------------------------------------------------------------------------------
// Cluster MANAGE_NO_DATA_ATTRIBUTE_CLUSTER
#define NoDataAttr                          0x0000  //No data test attribute

//------------------------------------------------------------------------------
// Cluster MANAGE_8BIT_INTEGER_ATTRIBUTES
#define UInt8Attr                           0x1C71  // default 0x12
#define Int8Attr                            0x38E2  // default 0x80

//------------------------------------------------------------------------------
// Cluster MANAGE_16BIT_INTEGER_ATTRIBUTES
#define UInt16Attr                          0x5553  // default 0x1234
#define Int16Attr                           0x71C4  // default 0x8000

//------------------------------------------------------------------------------
// Cluster MANAGE_SEMI_PRECISION_INTEGER_ATTRIBUTES
#define SemiPrecAttr                        0x8E35  // default 0x0412

//------------------------------------------------------------------------------
// Cluster MANAGE_TIME_ATTRIBUTES
#define AbsTimeAttr                         0xAAA6  // default 0x12345678
#define RelTimeAttr                         0xC717  // default 0xabcdef01

//------------------------------------------------------------------------------
// Cluster MANAGE_STRING_ATTRIBUTES
#define CharStringAttr                      0xE388  // default 0x0441424344
#define OctetStringAttr                     0xFFFF  // default 0x0412345678



//******************************************************************************
// Check User Assignments

#if ( ZIGBEE_2007 == 0x01) && ( ZIGBEE_PRO == 0x01)
    #error Both ZigBee 2007 and ZigBee PRO Stack Profile enabled. Only one should be active. Check Profile description.
#endif

// Make sure the window size is not more than 8
#if (I_SUPPORT_FRAGMENTATION == 0x01)
    #if PROFILE_FRAGMENT_WINDOW_SIZE > 8
        #error PROFILE_FRAGMENT_WINDOW_SIZE exceeded max limit. Should be less than or equal to 8
    #endif
#endif //(I_SUPPORT_FRAGMENTATION == 1)

#endif
