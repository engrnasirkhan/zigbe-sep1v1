/*********************************************************************
 *
 *                  Application Common
 *
 *********************************************************************
 * FileName:        App_Common.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / PIC32
 * Complier:        MCC18 v3.00 or higher
 *                  MCC30 v2.05 or higher
 *                  MCC32 v2.05 or higher
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
 *
 *****************************************************************************/

 /****************************************************************************
   Includes
 *****************************************************************************/
/* Generic files */
#include <string.h>
#include "generic.h"
#include "Console.h"

/* Stack related files */
#include "ZigBeeTasks.h"
#include "zigbee.h"
#include "zAPS.h"
#include "zNVM.h"
#include "App_Common.h"

/* ZCL Related Files */
#include "ZCL_Interface.h"

/* SE Related Files */
#include "SE_Interface.h"
#include "SE_Profile.h"

/* App realated files */
#include "Menu_ESP.h"
#include "Menu_MTR.h"

/* Libraries */
#if defined(__C30__)
    #include "TLS2130.h"
#else
    #include "lcd PIC32.h"
#endif

/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/
/* None */

/*****************************************************************************
   Customizable Macros
 *****************************************************************************/
/* None */

/*****************************************************************************
   Data Structures
 *****************************************************************************/
/* None */

/*****************************************************************************
   Variable Definitions
 *****************************************************************************/
BYTE destinationEndPoint = 0x01;


/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
/* None */

/*****************************************************************************
  Private Functions
 *****************************************************************************/
/* None */

/*****************************************************************************
  Public Functions
 *****************************************************************************/

/***************************************************************************************
 * Function:        BYTE GetMACByte(BYTE *ptr)
 *
 * PreCondition:    None
 *
 * Input:           ptr-Pointer to the HEX bytes which needs to be converted into BYTE.
 *
 * Output:          converted BYTE
 *
 * Side Effects:    None
 *
 * Overview:        2 HEX digits(in ascii format) converted into a BYTE
 *
 * Note:            None
 *
 ***************************************************************************************/
BYTE GetMACByte(BYTE *ptr)
{
    BYTE    oneByte, c;

    c = *ptr;
    if (('0' <= c) && (c <= '9'))
        c -= '0';
    else if (('a' <= c) && (c <= 'f'))
        c = c - 'a' + 10;
    else if (('A' <= c) && (c <= 'F'))
        c = c - 'A' + 10;
    else
        c = 0;

    oneByte =  c << 4;
    c = *(ptr + 1);
    if (('0' <= c) && (c <= '9'))
        c -= '0';
    else if (('a' <= c) && (c <= 'f'))
        c = c - 'a' + 10;
    else if (('A' <= c) && (c <= 'F'))
        c = c - 'A' + 10;
    else
        c = 0;

    oneByte += c;
    return oneByte;
}

/***************************************************************************************
 * Function:        BYTE GetMACByteDec(BYTE *ptr)
 *
 * PreCondition:    None
 *
 * Input:           ptr-Pointer to the Decimal values which needs to be converted into BYTE.
 *
 * Output:          converted BYTE
 *
 * Side Effects:    None
 *
 * Overview:        decimal values(in ascii format) converted into a BYTE
 *
 * Note:            None
 *
 ***************************************************************************************/
void GetMACByteDec(void *value, BYTE *ptr, BYTE dataTypeSize)
{
    BYTE i = 0;
    DWORD multiplier = 1;

    while( ptr[i] != 13 )
    {
        i++;
    }
    if( dataTypeSize == 0x01 )
    {
        *((BYTE *)value) = 0;
    }
    while(i != 0)
    {
        if( dataTypeSize == 0x01 )
        {
            *((BYTE *)value) += (ptr[i - 1] - 0x30) * multiplier;
        }
        multiplier *= 10;
        i--;
    }
}

/***************************************************************************************
 * Function:        void HandleUARTRequests()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handle requests issued by terminal
 *
 * Note:            None
 *
 ***************************************************************************************/
void HandleUARTRequests(void)
{
    BYTE c;

    if ( ConsoleIsGetReady() )
    {
        c = ConsoleGet();
        ConsolePut(c);
        if (c == 13)
        {
	        if( NOW_I_AM_A_ROUTER() )
	        {
            	inputBuf_MTR.used = 0;
            	inputBuf_MTR.buffer[inputIndex] = c;
         	}
            else if( NOW_I_AM_A_CORDINATOR() )
            {
            	inputBuf_ESP.used = 0;
            	inputBuf_ESP.buffer[inputIndex] = c; 
	        }
            inputIndex = 0;
	        if( NOW_I_AM_A_ROUTER() )
	        {
            	ProcessMenu_MTR();
         	}
            else if( NOW_I_AM_A_CORDINATOR() )
            {
            	ProcessMenu_ESP();
	        }
            //ProcessMenu();
        }
        else
        {
	        if( NOW_I_AM_A_ROUTER() )
	        {
            	inputBuf_MTR.buffer[inputIndex] = c;
         	}
            else if( NOW_I_AM_A_CORDINATOR() )
            {
            	inputBuf_ESP.buffer[inputIndex] = c;
	        }
            inputIndex++;
        }
    }
}

/******************************************************************************
 * Function:        void AppSendData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData)
 *
 * PreCondition:    None
 *
 * Input:           direction - direction of the command - server to client
 *                                                         or client to server
 *                  clusterID - cluster id
 *                  cmdId - command id
 *                  asduLength - length of the payload
 *                  pData - payload
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to format the Data in the required format
 *                  for updating the data request structure.
 *****************************************************************************/
void AppSendData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData)
{
    BYTE payload[71];
    
    SHORT_ADDR  bindDestAddr;
    unsigned char destEndPoint;
    
    /* ZCL Header */
    
    
    if( !(direction & ZCL_ManufacturerCodeFieldPresent) )
    {
        /* Update the direction bit in frame control */
        payload[0] = direction;
        /* Update the application sequence number */
        payload[1] = appNextSeqNum++;
        /* Update the command id to read attribute request */
        payload[2] = cmdId;
    
        /* Update ZCL Payload length and payload.*/
        /*Here ZCL header is conidered as 3 bytes because we are sending command
          without manfacturer code*/
        payload[70] = asduLength + ZCL_HEADER_LENGTH_WO_MANUF_CODE;
        /*copy the zcl header and payload in to the allocated buffer*/
        memcpy
        (
            &(payload[3]),
            &(pData[2]),
            asduLength
        );
    }
    
    /* the 2-byte manufacturer ID is present in header */
    else
    {
        /* Update the direction bit in frame control */
        payload[0] = direction;
        
        payload[1] = pData[2];
        payload[2] = pData[3];
        
        /* Update the application sequence number */
        payload[3] = appNextSeqNum++;
        /* Update the command id to read attribute request */
        payload[4] = cmdId;
    
        /* Update ZCL Payload length and payload.*/
        /*Here ZCL header is conidered as 5 bytes because we are sending command
          with manfacturer code*/
        payload[70] = asduLength + ZCL_HEADER_LENGTH_WITH_MANUF_CODE;
        /*copy the zcl header and payload in to the allocated buffer*/
        memcpy
        (
            &(payload[5]),
            &(pData[4]),
            asduLength
        );
        
        
        
    }
    
    /* v4.0:  Check the binding table.  If there is an entRY for this End/Cluster/Device combo 
     *        then use the proper destination endpoint when sending out the message,
     *        it no match is found, then the default endpoint of 0x01 will be used.
    */
    bindDestAddr.byte.LSB = pData[0];
    bindDestAddr.byte.MSB = pData[1];
    destEndPoint = App_FindDestEndpointFromBindings(bindDestAddr, clusterID);

    if(destEndPoint != 0xFF)
    {
        App_SendData ( APS_ADDRESS_16_BIT, pData, destEndPoint, payload[70], clusterID, payload );   
    }
    else
    {
    
        App_SendData ( APS_ADDRESS_16_BIT, pData, destinationEndPoint, payload[70], clusterID, payload );
    }
}

/******************************************************************************
 * Function:        void AppSendGroupData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData)
 *
 * PreCondition:    None
 *
 * Input:           direction - direction of the command - server to client
 *                                                         or client to server
 *                  clusterID - cluster id
 *                  cmdId - command id
 *                  asduLength - length of the payload
 *                  pData - payload
 * Output:          None
 *
 * Return :         None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to format the Data in the required format
 *                  for updating the data request structure.
 *****************************************************************************/
void AppSendGroupData(BYTE direction, WORD clusterID, BYTE cmdId, BYTE asduLength, BYTE *pData)
{
    BYTE payload[71];
    /* ZCL Header */
    /* Update the direction bit in frame control */
    payload[0] = direction;
    /* Update the application sequence number */
    payload[1] = appNextSeqNum++;
    /* Update the command id to read attribute request */
    payload[2] = cmdId;

    /* Update ZCL Payload length and payload.*/
    /*Here ZCL header is conidered as 3 bytes because we are sending command
      without manfacturer code*/
    payload[70] = asduLength + ZCL_HEADER_LENGTH_WO_MANUF_CODE;
    /*copy the zcl header and payload in to the allocated buffer*/
    memcpy
    (
        &(payload[3]),
        &pData[2],
        asduLength
    );

    App_SendData ( APS_ADDRESS_GROUP, pData, destinationEndPoint, payload[70], clusterID, payload );
}


#if defined(I_SUPPORT_BINDINGS)
BYTE App_FindDestEndpointFromBindings(SHORT_ADDR destAddr, WORD clusterId)
{
    BYTE                bindingIndex    = 0;
    BYTE                bindingMapSourceByte;
    BYTE                bindingMapUsageByte;
    
    unsigned char       endpoint = 0xFF;  /* invalid endpoint */
    
    
    WORD                pCurrentBindingRecord;
    BINDING_RECORD      currentBindingRecord;
    
    WORD                destBinding;
    BINDING_RECORD      destBindingRecord;

    while (bindingIndex < MAX_BINDINGS)
    {
        /* Get the bind maps for each check, */
        GetBindingSourceMap( &bindingMapSourceByte, bindingIndex );
        GetBindingUsageMap( &bindingMapUsageByte, bindingIndex );

        if (BindingIsUsed( bindingMapUsageByte,  bindingIndex ) &&
            BindingIsUsed( bindingMapSourceByte, bindingIndex ))
        {
            /* Read the source node record into RAM */
            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(bindingIndex) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[bindingIndex];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
            
            /* get the destination node to which the cluster is bound */
            if(currentBindingRecord.nextBindingRecord != 0xFF)
            {
                destBinding = apsBindingTable + (WORD)(currentBindingRecord.nextBindingRecord) * (WORD)sizeof(BINDING_RECORD);
                GetBindingRecord(&destBindingRecord, destBinding);
          
                /* if there is a complete match, then send the packet to that destination EndPoint */
                if(  (destBindingRecord.shortAddr.Val == destAddr.Val)  &&
                     (destBindingRecord.clusterID.Val  == clusterId)  
                  )
                {
                    return(destBindingRecord.endPoint);
                }
            }
        }
        
        /* continue search at next binding table slot */
        bindingIndex++;
        
    }
    
    /* getting here means that the destination node was not bound 
     * to this device with this clusterId of interest
    */
    return(endpoint);
}
#endif

