/**********************************************************************
* © 2006 Microchip Technology Inc.
*
* FileName:        lcd.h
* Dependencies:    none
* Processor:       PIC24H
* Compiler:        MPLAB® C30 v2.01 or higher
*
* SOFTWARE LICENSE AGREEMENT:
* Microchip Technology Inc. (“Microchip”) licenses this software to you
* solely for use with Microchip dsPIC® digital signal controller
* products. The software is owned by Microchip and is protected under
* applicable copyright laws.  All rights reserved.
*
* SOFTWARE IS PROVIDED “AS IS.”  MICROCHIP EXPRESSLY DISCLAIMS ANY
* WARRANTY OF ANY KIND, WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE, OR NON-INFRINGEMENT. IN NO EVENT SHALL MICROCHIP
* BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR CONSEQUENTIAL
* DAMAGES, LOST PROFITS OR LOST DATA, HARM TO YOUR EQUIPMENT, COST OF
* PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
* BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
* ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER SIMILAR COSTS.
*
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author            Date      Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Richard Fischer   07/14/05  Explorer 16 board LCD function support
* Priyabrata Sinha  01/27/06  Ported to non-prototype devices
*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* ADDITIONAL NOTES:
*
**********************************************************************/

#define MAX_DIGITS          6
#define	LCD_F_INSTR		    10
#define	LCD_S_INSTR		    150
#define	LCD_STARTUP		    2000

#define LCD_CLEAR           0x01
#define LCD_HOME            0x02
#define LCD_CURSOR_BACK     0x10
#define LCD_CURSOR_FWD      0x14
#define LCD_PAN_LEFT        0x18
#define LCD_PAN_RIGHT       0x1C

#define BIN                 2
#define OCT                 8
#define DEC                 10
#define HEX                 16


/******	LCD FUNCTION PROTOYPES ******/

void Init_LCD( void );		        // initialize display
void lcdInit ( void );


void lcd_cmd( char cmd );	        // write command to lcd

void lcd_data  ( char data );		    // write data to lcd
void lcdPutChar( char data );


void puts_lcd  ( unsigned char *data, unsigned char count );
void lcdPutStr ( char *str);

void lcdClear( void);
void lcdPutCur(char row, char col);
void lcdPutChar(char data);
void lcdDelay(int count);


/*****	LCD COMMAND FUCNTION PROTOTYPES  *****/
#define cursor_right()  lcd_cmd( 0x14 )
#define cursor_left()   lcd_cmd( 0x10 )
#define display_shift() lcd_cmd( 0x1C )
#define home_clr()      lcd_cmd( 0x01 ) 
#define home_it()       lcd_cmd( 0x02 ) 
#define line_2()        lcd_cmd( 0xC0 ) // (0xC0)


// Macro Definitions
#define lcdHome()       lcdPutSCmd(LCD_HOME)
#define lcdCurFwd()     lcdPutSCmd(LCD_CURSOR_FWD)
#define lcdCurBack()    lcdPutSCmd(LCD_CURSOR_BACK)
#define lcdPanLeft()    lcdPutSCmd(LCD_PAN_LEFT)
#define lcdPanRight()   lcdPutSCmd(LCD_PAN_RIGHT)
