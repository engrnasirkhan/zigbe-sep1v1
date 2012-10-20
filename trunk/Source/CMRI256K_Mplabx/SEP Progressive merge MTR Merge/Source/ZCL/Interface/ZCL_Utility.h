/******************************************************************************
 *
 *                  ZCL Utility
 *
 ******************************************************************************
 * FileName:        ZCL_Utility.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v1.05 or higher
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
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      01-04-09

 *****************************************************************************/

#ifndef _ZCL_UTILITY_H_
#define _ZCL_UTILITY_H_

/*****************************************************************************
  Includes
 *****************************************************************************/
 /* None */

/*****************************************************************************
  Constants and Enumerations
 *****************************************************************************/
/*total number of seconds in a minutes*/
#define NUMBER_OF_SECONDS_IN_A_MINUTE                           0x3C
/*Total number of seconds in an hour*/
#define NUMBER_OF_SECONDS_IN_AN_HOUR                            0xE10

/*****************************************************************************
 Customizable Macros
 *****************************************************************************/
/* None */

/*****************************************************************************
   Variable definitions
 *****************************************************************************/
/* None */

/*****************************************************************************
  Data Structures
 *****************************************************************************/
 
/*************************************************************
 Overview: This Structure holds the year, month, day, hour , minute, second elapsed
**************************************************************/
typedef struct __attribute__((packed,aligned(1))) _elapsedTimeInfo
{
    BYTE year; //this holds the year
    BYTE month; //this holds the month
    BYTE day;//this holds the day
    BYTE hour;//this holds hour
    BYTE minute;//this holds minute
    BYTE second;//this holds second
}elapsedTimeInfo;

/*****************************************************************************
  Function Prototypes
 *****************************************************************************/


 /***************************************************************************************
  Function:
      void ZCL_Utility_Time_ConvertSecondsToYear(DWORD seconds,elapsedTimeInfo *pElapsedTime)

  Description:
   This function takes the seconds (in UTC format) as input parameter. 
   This function is called when a given UTC format time needs to be calculated interms of Year, Month, Day, Hour, Minute, and Seconds format.
   The output pointer(pElapsedTime which is of type elapsedTimeInfo structure holds the values for Year, Month, Day, Hour, Minute and Second) is updated with the calculated values.

  Precondition:
           pElapsedTime should be allocated either statically or dynamically. If dynamically allocated, then this needs to be freed up by calling function.
  Parameters:
           seconds	-  Indicates the seconds elapsed from 01 January 2000 00  hours(UTC Time). 
           pElapsedTime -  It is a pointer points to elapsedTimeInfo structure.This structure holds the Year, Month, Day, Hour, Minute and  Second as members. (Output parameter)
   Return:
    None
  Example:
    <code>

    </code>
  Remarks:
    None.
 ***************************************************************************************/
void ZCL_Utility_Time_ConvertSecondsToYear(DWORD seconds,elapsedTimeInfo *pElapsedTime);

/*************************************************************************************
  Function:
          DWORD ZCL_Utility_Time_ConvertYearToSeconds( elapsedTimeInfo *pElapsedTime )
    
  Description:
    This function takes pElapsedTime, a structure which holds the
    information about Year, Month, Day, Hour, Minute, and Seconds, as input
    parameter. This function is called when UTC Time needs to be calculated
    from the given input parameter. This function returns the calculated
    UTC Time.
  Conditions:
    None
  Input:
    pElapsedTime -  It is a pointer pointing to elapsedTimeInfo structure.
                    This structure holds the Year, Month, Day, Hour, Minute
                    and Second as members.
  Return:
    It returns the time elapsed(UTC Time) from 01 January 2000 00 hours
    calculated from the given input parameter.
  Example:
    <code>
    
    </code>
  Remarks:
    None.                                                                             
  *************************************************************************************/
DWORD ZCL_Utility_Time_ConvertYearToSeconds( elapsedTimeInfo *pElapsedTime );

/**************************************************************************
  Function:
      BYTE ZCL_Utility_GetMaxPayloadLength ( void )
    
  Description:
    This function is called when a data needs to be transmitted. This
    \function is used to get the maximum number of bytes that can be used
    as payload (application data) for the application. This function
    \returns MaxPayload that can be used for application payload to
    transmit the data frame.
  Conditions:
    None
  Return:
    \Returns the Max Number of Bytes that application can use as Data
    Payload.
  Example:
    <code>
    
    </code>
  Remarks:
    Max Payload is derived by ZigBee Max Frame - (MAC Header + NWK Header +
    NWK Auxiliary Header + APS Header + APS Auxiliary Header) Security
    Level is assumed to be 5                                               
  **************************************************************************/
BYTE ZCL_Utility_GetMaxPayloadLength ( void );

/******************************************************************************
 * Function:        ZCL_Utility_Is_Client_Time_Cluster_Supported
 *
 * PreCondition:    None
 *
 * Input:           endPointId - endpoint id for which the command is received
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    Returns Success( 1 ) or Failure( 0 )
 *
 * Overview:        This function checks for the given endpoint's simple
 *                  descriptor if Time cluster is supported as client.
 *                  if supported as client returns TRUE(success) else
 *                  failure( 0x00 )
 *
 * Note:
 *****************************************************************************/
BYTE ZCL_Utility_Is_Client_Time_Cluster_Supported( BYTE endPointId );

#endif /* _ZCL_UTILITY_H_ */
