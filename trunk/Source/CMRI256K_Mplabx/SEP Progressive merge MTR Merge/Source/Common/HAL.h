/*********************************************************************
 *
 *                          Hardware Abstraction Layer
 *
 *********************************************************************
 * FileName:        HAL.h
 * Dependencies:
 * Processor:       PIC24FJ128GA010
 * Complier:        MPLab C30 C
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 ********************************************************************/

#ifndef _HAL_H_
#define _HAL_H_
/****************************************************************************
   Includes
 *****************************************************************************/
 /*None*/
 
#define UART_MODE_COMM
//#define USB_MODE_COMM

//#define YODA_V1
#define YODA_V1A

#if defined(UART_MODE_COMM) && defined(USB_MODE_COMM)
	#error Choose one mode of communication between UART & USB on board YODA evaluation board
#endif

#if defined(YODA_V1) && defined(YODA_V1A)
	#error Choose proper version of YODA being used
#endif
 /*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
#if !defined(YODA_V1A) && !defined(YODA_V1)
	#if defined(__C30__) || defined (__C32__)
	    #define SWITCH0                     PORTDbits.RD6
	    #define SWITCH1                     PORTDbits.RD7
	    #define LIGHT0                      LATAbits.LATA6
	    #define LIGHT1                      LATAbits.LATA7
	    /*This LED is used for indicating identify mode of a device*/
	    #define LIGHT3                      LATAbits.LATA2
	
	    #define LIGHT6                      LATAbits.LATA3
	    #define LIGHT3_ON()                 LIGHT3 = 0x01;
	    #define LIGHT3_TOGGLE()             LIGHT3^= 0x01;
	    #define LIGHT3_OFF()                LIGHT3 = 0x00;
	
	#else
	    #define SWITCH0                     PORTBbits.RB5
	    #define SWITCH1                     PORTBbits.RB4
	    #if defined(__18F4620)
	        #define LIGHT0                  LATAbits.LATA0
	        #define LIGHT1                  LATAbits.LATA1
	    #else
	        #define LIGHT0                  LATDbits.LATD1
	        #define LIGHT1                  LATDbits.LATD1
	    #endif
	#endif
#else
	#if defined(__C30__) || defined (__C32__)
	    #define SWITCH0                     PORTDbits.RD11
	    #define SWITCH1                     PORTDbits.RD11
	    #define LIGHT0                      LATDbits.LATD11
	    #define LIGHT1                      LATDbits.LATD11
	    /*This LED is used for indicating identify mode of a device*/
	    #define LIGHT3                      LATDbits.LATD11
	
	    #define LIGHT6                      LATDbits.LATD11
	    #define LIGHT3_ON()                 LIGHT3 = 0x01;
	    #define LIGHT3_TOGGLE()             LIGHT3^= 0x01;
	    #define LIGHT3_OFF()                LIGHT3 = 0x00;

	#endif
#endif

#ifdef USE_EXTERNAL_NVM
    #define SPISelectEEPROM()   {EEPROM_nCS = 0;}
    #define SPIUnselectEEPROM() {EEPROM_nCS = 1;}
#endif /*USE_EXTERNAL_NVM*/

/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
 /* None */

 /*****************************************************************************
   Variable definitions
 *****************************************************************************/

/*****************************************************************************
  Data Structures
 *****************************************************************************/
 /* None */

 /*****************************************************************************
  Function Prototypes
 *****************************************************************************/
 
 
 /**************************************************************************
   Function:
           void HardwareInit(void);
     
   Description:
     This function Initializes the hardware components of Controller i.e
     GPIO, SPI, External EEPROM etc..
   Conditions:
     None
   Return:
     None
   Example:
     <code>
     
     </code>
   Remarks:
     Before invoking any hardware related functions, this function should be
     called.
     
                                                                            
   **************************************************************************/
void HardwareInit(void);

#endif /*_HAL_H_*/
