/*****************************************************************************
 *
 *              This file contains function that can be used for diagnostic
 *                  purposes
 *****************************************************************************
 * FileName:        log.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Compiler:        C30
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         Microchip Technology Incorporated
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
 *
 *
 * Author               Date        Version     Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *
 *****************************************************************************/


//******************************************************************************
// Header Files
//******************************************************************************

#include <stdio.h>
#include <string.h>
#include "generic.h"
//#include "UART.h"
#include "sralloc.h"
#include "zigbee.def"
#include "Log.h"
#include "zNWK.h"
#include "zigbee.h"
#include "zAPS.h"

#ifdef DEBUG_LOG
//******************************************************************************
// Constants
//******************************************************************************

#define MAX_LOG_MESSAGE_LENGTH   120

//******************************************************************************
// Function Prototypes
//******************************************************************************
extern int snprintf(char *, size_t, const char *, ...);

/*********************************************************************
 * Function:        void log_message( BYTE priority, BYTE expr,
 *                  ROM char *message, BYTE *file, WORD line )
 *
 * PreCondition:    None
 *
 * Input:           priority = Priority of logging message can take below levels
                    1 : this message is not expected to come.
                    This is an error scenario.
                     2 : this message is not desired to come.
                    This is treated as a warning
                     3 : This is message is a information
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This functions prints the lig message to UART
 *
 * Note:            None
 ********************************************************************/
void log_assert( BYTE priority, BYTE expr, ROM char *message, char *file,
 int line )
{
    BYTE *pLogMessage;
    BYTE *pCodeMessage;
    BYTE count;
    BYTE c;
    if( (!expr) && ( priority >= DEBUG_LOG_LEVEL ))
    {
       pLogMessage = ( unsigned char *) SRAMalloc( MAX_LOG_MESSAGE_LENGTH + 3 );
       if( pLogMessage != NULL )
       {
          pCodeMessage = ( unsigned char *) SRAMalloc( MAX_LOG_MESSAGE_LENGTH );
          if( pCodeMessage != NULL )
          {
             for( count = 0; count < MAX_LOG_MESSAGE_LENGTH; count++)
             {
                 c = message[count];
                 *( pCodeMessage + count ) = c;
             }
             // memcpypgm2ram((BYTE *) pCodeMessage, (ROM void*)message, MAX_LOG_MESSAGE_LENGTH);

              pLogMessage[0] = snprintf((char *)&pLogMessage[2], MAX_LOG_MESSAGE_LENGTH, \
              "level=%d, file=%s, line=%d, %s ", priority,file, line, pCodeMessage);
              pLogMessage[1] = 0x01; // NTS id to print the message
              /*  call the uart transmit function to the packet to test tool  */
              SRAMfree( pCodeMessage );
             UartTransmit( pLogMessage );
          }
       }
    }
}

void log_message(BYTE priority, BYTE debugPrimitiveId, void *message, char *file, int line )
{
    BYTE *pLogMessage;
    BYTE *pCodeMessage = NULL;
    BYTE length;
    if(( priority >= DEBUG_LOG_LEVEL ))
    {
       pLogMessage = ( unsigned char *) SRAMalloc( MAX_LOG_MESSAGE_LENGTH + 3 );
       if( pLogMessage != NULL )
       {
            switch (debugPrimitiveId)
            {
                case DEBUG_SHORT_ADDRESS:
                    length  = sizeof(SHORT_ADDR);
                    pCodeMessage = ( BYTE*) SRAMalloc( length );
                    if( pCodeMessage != NULL )
                    {
                        snprintf((char  *)pCodeMessage, length, "%s",(BYTE*)message);
                    }
                    else
                    {
                        break;
                    }
                    break;

                case DEBUG_LONG_ADDRESS:
                    length  = sizeof(LONG_ADDR);
                    pCodeMessage = ( BYTE*) SRAMalloc( length );
                    if( pCodeMessage != NULL )
                    {
                        snprintf((char  *)pCodeMessage, length, "%s",(BYTE*)message);
                    }
                    else
                    {
                        break;
                    }
                    break;

                case DEBUG_APS_RECORD:
                    length  = sizeof(APS_ADDRESS_MAP);
                    pCodeMessage = ( BYTE*) SRAMalloc( length );
                    if( pCodeMessage != NULL )
                    {
                        snprintf((char  *)pCodeMessage, length, "%s",(BYTE*)message);
                    }
                    else
                    {
                        break;
                    }
                    break;

                case DEBUG_NEIGHBOUR_RECORD:
                    length  = sizeof(NEIGHBOR_RECORD);
                    pCodeMessage = ( BYTE*) SRAMalloc( length );
                    if( pCodeMessage != NULL )
                    {
                        snprintf((char  *)pCodeMessage, length, "%s",(BYTE*)message);
                    }
                    else
                    {
                        break;
                    }
                    break;

                case DEBUG_ROUTING_ENTRY:
                    length  = sizeof(ROUTING_ENTRY);
                    pCodeMessage = ( BYTE*) SRAMalloc( length );
                    if( pCodeMessage != NULL )
                    {
                        snprintf((char  *)pCodeMessage, length, "%s",(BYTE*)message);
                    }
                    else
                    {
                        break;
                    }
                    break;

                case DEBUG_KEY_VAL:
                    length  = sizeof(KEY_VAL);
                    pCodeMessage = ( BYTE*) SRAMalloc( length );
                    if( pCodeMessage != NULL )
                    {
                        snprintf((char  *)pCodeMessage, length, "%s",(BYTE*)message);
                    }
                    else
                    {
                        break;
                    }
                    break;

                case DEBUG_BYTE:
                    length  = sizeof(BYTE);
                    pCodeMessage = ( BYTE*) SRAMalloc( length );
                    if( pCodeMessage != NULL )
                    {
                        snprintf((char  *)pCodeMessage, length, "%s",(BYTE*)message);
                    }
                    else
                    {
                        break;
                    }
                    break;
            }
            if( pCodeMessage != NULL )
            {
                  pLogMessage[0] = snprintf((char *)&pLogMessage[3], MAX_LOG_MESSAGE_LENGTH , \
                  "level=%d, file=%s, line=%d, %s ", priority,file, line, (char*)pCodeMessage);
                  pLogMessage[1] = 0x02; // NTS id to print the message
                  pLogMessage[2] = debugPrimitiveId; // debug id
                  /*  call the uart transmit function to the packet to test tool  */
                  SRAMfree( pCodeMessage );
                 UartTransmit( pLogMessage );
            }
      }
   }
}
#endif

