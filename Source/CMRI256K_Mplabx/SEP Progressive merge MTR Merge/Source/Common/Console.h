/*********************************************************************
 *
 *                  Console Routines
 *
 *********************************************************************
 * FileName:        Console.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Complier:        MCC18 v1.00.50 or higher
 *                  MCC30 v2.05 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright � 2004-2007 Microchip Technology Inc.  All rights reserved.
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
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY OF ANY
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
 * Nilesh Rajbharti     7/12/04 Rel 0.9
 * Nilesh Rajbharti     11/1/04 Pre-release version
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                08/19/05 Microchip ZigBee Stack v1.0-3.2
 * DF/KO                09/08/05 Microchip ZigBee Stack v1.0-3.3
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY             01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
#ifndef  _CONSOLE_H_
#define _CONSOLE_H_

#include "generic.h"
#include "Compiler.h"

#if defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24H__) || defined(__PIC32MX__)
    
 /*********************************************************************************************
  Function:
      void ConsoleInit(void)

  Description:
    This function initializes the UART Serial Communication.Setting the baudrate using clock
    frequency, initializing the interrupt flags and reception mode.
  Precondition:
    None
  Parameters:
    None
  Return:
    None
  Example:
    <code>

    </code>
  Remarks:
  *********************************************************************************************/
    void ConsoleInit(void);
    
    #define ConsoleIsPutReady()     (U2STAbits.TRMT)
    #define ConsoleIsGetReady()     (IFS1bits.U2RXIF)
    void ConsolePut(BYTE c);
    void ConsolePutROMString(ROM char* str);
    BYTE ConsoleGet(void);
    BYTE GetHexByte( void );
	int xatoi (char **str, long *res);
void xputc (char c);
void xputs (char* str);
void xitoa (long val, char radix, char len);
void xprintf (const char* str, ...);
void put_dump (const BYTE *buff, DWORD ofs, int cnt);

#elif defined(__18CXX)
#if 1   /* Useful for disabling the console (saving power) */

 /**************************************************************************
   Function:
           void ConsoleInit(void)
     
   Description:
     This function initializes the UART Serial Communication i.e Setting the
     baud rate using clock frequency and initializing the interrupt flags
     etc..
   Conditions:
     None
   Return:
     None
   Example:
     <code>
     
     </code>
   Remarks:
     Before calling this function, HardwareInit() function needs to be
     called. If application requires UART Serial communication, this
     \function needs to be invoked.
     
                                                                            
   **************************************************************************/
    void ConsoleInit(void);
    
    #define ConsoleIsPutReady()     (TXSTAbits.TRMT)
    void ConsolePut(BYTE c);
    void ConsolePutString(BYTE *s);
    void ConsolePutROMString(ROM char* str);

    #define ConsoleIsGetReady()     (PIR1bits.RCIF)
    BYTE ConsoleGet(void);
    BYTE ConsoleGetString(char *buffer, BYTE bufferLen);
#else
    #define ConsoleInit()
    #define ConsoleIsPutReady() 1
    #define ConsolePut(c)
    #define ConsolePutString(s)
    #define ConsolePutROMString(str)

    #define ConsoleIsGetReady() 1
    #define ConsoleGet()        'a'
    #define ConsoleGetString(buffer, bufferLen) 0
#endif

#endif

//TODO: remove test code
//<test code>
void PrintChar(BYTE);
//</test code>
#define printf(x)   ConsolePutROMString((ROM char *)x)

#endif

