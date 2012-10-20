/*********************************************************************
 *
 *                 Log header file
 *
 *********************************************************************
 * FileName:        Log.h
 * Dependencies:
 * Processor:       PIC24
 * Complier:        C30
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
 *
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      10/16/07    Original
 ********************************************************************/
#ifndef _LOG_H_
#define _LOG_H_

/*****************************************************************************
ENUMS
******************************************************************************/

typedef enum _DEBUG_PRIMITIVE_ID
{
    DEBUG_SHORT_ADDRESS                = 0x00,
    DEBUG_LONG_ADDRESS                 = 0x01,
    DEBUG_APS_RECORD                   = 0x02,
    DEBUG_NEIGHBOUR_RECORD             = 0x03,
    DEBUG_ROUTING_ENTRY                = 0x04,
    DEBUG_KEY_VAL                     = 0x05,
    DEBUG_BYTE                         = 0x06
} DEBUG_PRIMITIVE_ID;


//******************************************************************************
// Macros
//******************************************************************************

#ifdef DEBUG_LOG
    /* Priority 1 messages will be treated as errors .
    So message has to be logged as priority1 where the code should not come in
    normal scenario. */
    #define DEBUG_LOG_ERROR     3

    /*Priorty2 messages are considered as warnings.Priority2 messages are
      put where the condition that are not desired, but happens on some conditions */
    #define DEBUG_LOG_WARNING   2

    /* Priority3 messages are messages printed for information. */
    #define DEBUG_LOG_INFO      1

    /* Log level */
    #define DEBUG_LOG_LEVEL   DEBUG_LOG_INFO

    #define LOG_ASSERT(priority, expr)    \
    log_assert(priority, (BYTE)expr, (ROM char *)( #expr ), __FILE__, __LINE__)

    #define LOG_MESSAGE(priority, debugPrimitiveId, message )    \
    log_message(priority, debugPrimitiveId , (void*)message , __FILE__ , __LINE__ )
#else
    #define LOG(priority, expr) {}
    #define LOG_ASSERT(priority, expr) {}
#endif





/*********************************************************************
 * function declarations
 ********************************************************************/
void log_assert( BYTE priority, BYTE expr, ROM char *message, char *file,
 int line );

void log_message(BYTE priority, BYTE debugPrimitiveId, void *message, char *file, int line );
#endif /*  _UART_H_ */
