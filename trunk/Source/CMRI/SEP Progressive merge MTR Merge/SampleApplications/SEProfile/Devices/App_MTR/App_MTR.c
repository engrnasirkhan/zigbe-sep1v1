/*********************************************************************
 *
 *                Application Meter Device
 *
 *********************************************************************
 * FileName:        App_MTR.c
 * Dependencies:
 * Processor:       PIC24FJ128GA010
 * Complier:        MPLab C30 C
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright © 2004-2011 Microchip Technology Inc.  All rights reserved.
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                      03/16/09    Original
 ********************************************************************/

/******************************************************************************
   Includes
 *****************************************************************************/
/* Configuration Files */
#include "zigbee.def"

/* Generic Files */
#include "generic.h"
#include "Console.h"
#include "sralloc.h"
#include "MSPI.h"
#include "rtcc.h"
#include "HAL.h"

/* Stack Related Files */
#include "zAPL.h"
#include "zAIL.h"
#include "zAPS.h"
#include "zSecurity.h"
#include "zNVM.h"
#include "zMAC.h"

/* ZCL Related Files */
#include "ZCL_Interface.h"
#include "ZCL_Utility.h"
#include "SE_Interface.h"
#include "ZCL_Identify.h"
#include "ZCL_Time.h"
#include "ZCL_Basic.h"
#include "ZCL_Alarm.h"
#include "ZCL_Foundation.h"
#include "ZCL_Commissioning.h"
#include "ZCL_Power_Configuration.h"
#include "ZCL_Alarm.h"
#include "ZCL_Message.h"
#include "ZCL_Metering.h"
#include "ZCL_DRLC.h"
#include "ZCL_Price.h"
#if I_SUPPORT_CBKE == 1
#include "ZCL_Key_Establishment.h"
#endif /*#if I_SUPPORT_CBKE*/

/* SE Related Files */
#include "SE_Display.h"
#include "SE_Profile.h"
#include "SE_MTR.h"
#if I_SUPPORT_CBKE == 1
    #include "eccapi.h"
    #include "SE_CBKE.h"
#endif /*#if I_SUPPORT_CBKE*/

/* App Related Files */
#include "Menu_MTR.h"
#include "App_Common.h"
#include "App_MTR.h"

/* Libraries */
#if defined(__C30__)
    #include "TLS2130.h"
#else
    #include "lcd PIC32.h"
#endif

#include "MSDCL_Commissioning.h"
#include "msdlclusterhanddle.h"
#include "Delay.h"
#include "monitor.h"
/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/

//#if defined(MCHP_C18)
//    #if defined(__18F4620)
//        #pragma romdata CONFIG1H = 0x300001
//        const rom unsigned char config1H = 0b00000110;      // HSPLL oscillator
//
//        #pragma romdata CONFIG2L = 0x300002
//        const rom unsigned char config2L = 0b00011111;      // Brown-out Reset Enabled in hardware @ 2.0V, PWRTEN disabled
//
//        #pragma romdata CONFIG2H = 0x300003
//        const rom unsigned char config2H = 0b00010010;      // HW WD disabled, 1:512 prescaler
//
//        #pragma romdata CONFIG3H = 0x300005
//        const rom unsigned char config3H = 0b10000000;      // PORTB digital on RESET
//
//        #pragma romdata CONFIG4L = 0x300006
//        const rom unsigned char config4L = 0b10000001;      // DEBUG disabled,
//                                                            // XINST disabled
//                                                            // LVP disabled
//                                                            // STVREN enabled
//    #elif defined(__18F87J10)
//        #pragma romdata CONFIG1L = 0x1FFF8
//        const rom unsigned char config1L = 0b10111110;
//
//        #pragma romdata CONFIG1H = 0x1FFF9
//        const rom unsigned char config1H = 0b11111111;
//
//        #pragma romdata CONFIG2L = 0x1FFFA
//        const rom unsigned char config2L = 0b11111101;
//
//        #pragma romdata CONFIG2H = 0x1FFFB
//        const rom unsigned char config2H = 0b11111010;
//
//        #pragma romdata CONFIG3L = 0x1FFFC
//        const rom unsigned char config3L = 0b11111111;
//
//        #pragma romdata CONFIG3H = 0x1FFFD
//        const rom unsigned char config3H = 0b11111111;
//
//    #else
//        #error Cannot define configuration bits for this device
//    #endif
//
//    #pragma romdata
//#elif defined( __C30__ )
//    #if defined(__PIC24F__)
//        // Explorer 16 board
//        _CONFIG2(FNOSC_PRI & POSCMOD_XT)    // Primary XT OSC with 4X PLL
//        _CONFIG1(JTAGEN_OFF & FWDTEN_OFF & ICS_PGx2 & WDTPS_PS32768)   // JTAG off, watchdog timer off
//    #elif defined(__dsPIC33F__) || defined(__PIC24H__)
//        // Explorer 16 board
//        _FOSCSEL(FNOSC_PRI)             // primary osc
//        _FOSC(OSCIOFNC_OFF & POSCMD_XT) // XT Osc
//        _FWDT(FWDTEN_OFF)               // Disable Watchdog timer
//        // JTAG should be disabled as well
//    #elif defined(__dsPIC30F__)
//        // dsPICDEM 1.1 board
//        _FOSC(XT_PLL16)     // XT Osc + 16X PLL
//        _FWDT(WDT_OFF)      // Disable Watchdog timer
//        _FBORPOR(MCLR_EN & PBOR_OFF & PWRT_OFF)
//    #else
//        #error Cannot define configuration bits for this device
//    #endif
//#elif defined( __C32__ )
//    #pragma config FPLLMUL = MUL_16, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
//    #pragma config POSCMOD = XT, FNOSC = PRIPLL, FPBDIV = DIV_4, WDTPS = PS131072
//
//#else
//    #error Cannot define configuration bits for this device family
//#endif

#ifdef USE_EXTERNAL_NVM

    #define SPISelectEEPROM()   {EEPROM_nCS = 0;}
    #define SPIUnselectEEPROM() {EEPROM_nCS = 1;}

    #define SPIREAD             0x03
    #define SPIWRITE            0x02
    #define SPIWRDI             0x04
    #define SPIWREN             0x06
    #define SPIRDSR             0x05
    #define SPIWRSR             0x01

    #define WIP_MASK            0x03  //0x01

#endif

#define MAX_ADDR_LENGTH 0x08
#define SHORT_ADDRESS_LENGTH    0x02
#define SHORTPANID  0xB8DF

//#define ZDOENDPOINT             0x00

#ifdef I_SUPPORT_SECURITY
    #define DEFAULT_TX_OPTIONS  0x02 // NWK KEY ENABLED
#else
    #define DEFAULT_TX_OPTIONS  0x00 // UNSECURED
#endif

/*****************************************************************************
   Customizable Macros
 *****************************************************************************/
/* None */

/******************************************************************************
 Data Structures
******************************************************************************/
/* None */

/*****************************************************************************
   Variable Definitions
 *****************************************************************************/
extern NWK_STATUS nwkStatus;
extern BYTE AllowJoin;
extern BYTE NwkDiscoveryTries;
extern BYTE NwkJoinTries;
extern BYTE startMode;

extern BYTE ZDOCounter;
extern BYTE QuickPoll;
#ifdef I_SUPPORT_SECURITY
    extern SECURITY_STATUS      securityStatus;
    extern NETWORK_KEY_INFO plainSecurityKey[2];
#endif
extern KEY_VAL KeyVal;

extern NETWORK_DESCRIPTOR  *currentNetworkDescriptor;

extern NETWORK_DESCRIPTOR  *NetworkDescriptor;
extern APP_NOTIFICATION        appNotify;

extern BYTE ZDOCounter;

extern BYTE APSCounter;

extern BYTE *reportAttributeValue;
extern BYTE displayState;



/*This variable indicated the transaction number.
  It used in ZCL header of a ZCL command frame.*/
BYTE appNextSeqNum_MTR = 0x00;
ZIGBEE_PRIMITIVE currentPrimitive_MTR;

InterPANDataRequest interPANDataRequest;
BYTE meterReadingValue[6];

extern LONG_ADDR nwkDesExtendedPANID;

#ifdef DEBUG_LOG
    BYTE NoOfPANDescriptorFound;
    BYTE PotentialParentFound;
    BYTE overWriteCheck;
#endif

#if (I_SUPPORT_FRAGMENTATION == 1)
    /* Global static array to store the Fragmentation data payload */
extern    BYTE fragmentData[MAX_FRAGMENTATION_SIZE];
#endif    
/*****************************************************************************
   Function Prototypes
 *****************************************************************************/
#if I_SUPPORT_LINK_KEY == 1
    extern void SetAppLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails);
#endif
//#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1 && USE_COMMON_TC_LINK_KEY == 0 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
    extern BOOL SelectLinkKey(LONG_ADDR DstAddress, TC_LINK_KEY_TABLE *tcLinkKeyTable);
    extern BOOL SearchForTCLinkKey(LONG_ADDR deviceLongAddr, TC_LINK_KEY_TABLE *tcLinkKeyTable);
//#endif

//#if ( I_AM_ESP == 0 )
 extern void Time_Synchronization( BYTE endpoint, BYTE* pSeconds );
//#endif /*I_AM_ESP*/
void AppJoinRequest( NETWORK_DESCRIPTOR *, BYTE );
void JoinRequest( LONG_ADDR nwkDesExtendedPANID, BYTE joinType );
void AppTasks_MTR();
void RTCCProcessEvents();

void HandleChkConnections(void);

static void HandleDataConfirm (void);
#if (I_SUPPORT_FRAGMENTATION == 1)
    BYTE* GetFragmentDataBuffer( void );
#endif

extern DWORD_VAL ChannelsToBeScanned;
extern BYTE MAXDISCOVERYYRETRIES;
/***************************************************************************************
 * Function:        void AppInit()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the Application Specific Variable and Tables.
 *
 * Note:            None
 ***************************************************************************************/
void AppInit_MTR();

/*****************************************************************************
  Public Functions
 *****************************************************************************/
/*********************************************************************
 * Function:        int main( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main function which initializes uart module
 *                  and remains in infinite loop
 *
 * Note:            None
 ********************************************************************/
int main_MTR( void )
{
    BYTE StackStatus = 0;


    CLRWDT();
    HardwareInit();
    /*Real Time Clock Init*/
    mRTCCInit();
    //lcdInit();
    SRAMInitHeap();

    ConsoleInit();
	uart_init();

    ConsolePutROMString( (ROM char *)"\r\n*********************************" );
	ConsolePutROMString( (ROM char *)"\r\nMicrochip SE Profile 1.0.version.0.5.3" );
	ConsolePutROMString( (ROM char *)"\r\n*********************************" );
	ConsolePutROMString( (ROM char *)"\r\nMTR Device\r\n" );

    NwkDiscoveryTries = 0x00;
    NwkJoinTries = 0x00;
    nwkStatus.flags.Val = 0x00;

    currentPrimitive_MTR = NO_PRIMITIVE;

    /* Explorer 16 has external NVM */
    #ifdef USE_EXTERNAL_NVM
        NVMInit();
    #endif

    /* Initialize the ZigBee stack */
    ZigBeeInit();

    NetworkDescriptor = NULL;

    LIGHT0 = 0x01;
    LIGHT1 = 0x01;


    AllowJoin = TRUE;
    //printf("\r\n0.Start the device with NVM(Silent start)");
    //printf("\r\n1.Start the device without NVM(Clean start)");
    //printf("\r\n1.Start the device Using Configured values(From zigbee.def file)");
    //printf("\r\n2.Start the device without NVM(Clean start)");

    #if defined (__C30__)
        //ConsoleGet();
    #endif
   	inputBuf_MTR.used = 0;
   	inputBuf_MTR.buffer[1] = 13;
	
	if( MSDCL_Commission.ValidCleanStartUp )
	{
		printf("\r\nStart the device without NVM(Clean start)\r\n");
		inputBuf_MTR.buffer[0] = '1';
	}
	else
	{
		printf("\r\nStart the device with NVM(Silent start)\r\n");
		inputBuf_MTR.buffer[0] = '0';
	}	
    AppInit_MTR();

    menuLevel1 = 'z';
    menuLevel2 = 0xFF;
    menuLevel3 = 0x01;
    
    ProcessMenu_MTR();  

    ENABLE_WDT();
    LIGHT3 = 0x00;

    while( 1 )
    {
        CLRWDT();
        HandleUARTRequests();
		HanddleUART2();
		HandleChkConnections();
        StackStatus = ZigBeeTasks( &currentPrimitive_MTR );
        AppTasks_MTR();
        RTCCProcessEvents();
        //App_TimeTick();
        App_Message_Tick();
        //HandleMeterDisplay();
        //ProcessButtonPress();
        #if I_SUPPORT_CBKE == 1
            SE_CBKE_StateMachineHandler();
        #endif /*I_SUPPORT_CBKE*/
        /*if( ExtPANIDStatus.ResquestExtPanId == TRUE )
        {
	     	break;   
	    }
	    if( ExtPANIDStatus.ReadyResponsePANId == TRUE )
	    {
		    SendExtPANIdResponse();
		    ExtPANIDStatus.ReadyResponsePANId = FALSE;
		    ExtPANIDStatus.ResquestExtPanId = FALSE;
		    ExtPANIDStatus.WaitToSendResponse = FALSE;
		    NVMRead ( (BYTE*)&MSDCL_Commission, MSDCL_Commission_Locations, sizeof(MSDCL_Commission));
		    break;
		}
		code removed because all time if Extended pan ID search is there Device will act as ESP not as MTR
		*/

    }
}



/***************************************************************************************
 * Function:        void AppInit()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes the Application Specific Variable and Tables.
 *
 * Note:            None
 ***************************************************************************************/
void AppInit_MTR()
{
    /*App_InitAttributeStorage() should be called befor App_TimeInit()*/
    App_InitAttributeStorage();

    App_TimeInit();
    App_PriceInit_MTR();
    App_Meter_ProfileIntervalTableInit();
}

/***************************************************************************************
 * Function:        ZIGBEE_PRIMITIVE AppTasks(ZIGBEE_PRIMITIVE inputPrimitive)
 *
 * PreCondition:    None
 *
 * Input:           inputPrimitive: Foreground primitive needs to be handled by
 *                                  application.
 *
 * Output:          The next primitive which should be handled by stack.
 *
 * Side Effects:    None
 *
 * Overview:        Application state machine.
 *
 * Note:            None
 ***************************************************************************************/
 extern unsigned char StartNetworkFlag;
 extern unsigned char MyCurrentChannel;
void AppTasks_MTR()
{
    switch ( currentPrimitive_MTR )
    {
        case NO_PRIMITIVE:
        {
            BYTE                    *appMessage;

            if( CommRestartTimerInProgress )
            {
                TICK currentTime = TickGet();
                if
                (
                    TickGetDiff(currentTime, CommRestartTime) >=
                    (
                        ( RestartDeviceCmdPayload.Delay * ONE_SECOND) +
                        ( RestartDeviceCmdPayload.Jitter * ONE_MILLISECOND )
                    )
                )
                {
                    SE_SendNLMELeaveRequest();
                    CommRestartTimerInProgress = FALSE;
                    return;
                }
            }

            appMessage = (BYTE *) AILDequeue( ReceiveQueueID );
            if (appMessage == NULL)
            {
                //currentPrimitive_MTR = NO_PRIMITIVE;

                /*Handle Pending Data in the backgound*/
                App_HandlePendingData_MTR();

                return;
            }

            if ( *appMessage == MessageTypeAppDataIndication )
            {
                APP_DATA_indication     *dataInd;
                dataInd = (APP_DATA_indication *)appMessage;
                if ( dataInd != NULL )
                {
                    if( params.ZDO_DATA_indication.ProfileId.Val == ZDO_PROFILE_ID )
                    {
                       #if I_SUPPORT_CBKE
                            if( ( WAIT_FOR_MATCH_DESC_RESP_STATE == SE_KeyEstStateMachine ) &&
                             ( MATCH_DESC_rsp == params.ZDO_DATA_indication.ClusterId.Val ) )
                            {
                                /*check if Match descriptor response is received with status as success,and
                                match length is not equal to 0 then copy the endpoint of the device*/
                                if( ( params.APSDE_DATA_indication.asdu[1] == 0x00 ) &&
                                  ( params.APSDE_DATA_indication.asdu[4] != 0x00 ) )
                                {
                                    CBKE_KeyInfoStoreDestEndpoint( params.APSDE_DATA_indication.asdu[5] );
                                    CBKE_ChangeState( INITIAL_STATE );
                                }
                            }
                        #endif /*#if I_SUPPORT_CBKE*/

                    }                    
					else if(dataInd->ProfileId.Val == 0xBF0D)
                    {
                        if( (dataInd->ClusterId.Val == 0xFC00) || (dataInd->ClusterId.Val == 0xFC01) ||
							(dataInd->ClusterId.Val == 0xFC12) || (dataInd->ClusterId.Val == 0xFC14))
						{
							BYTE Tp1 = 0;
							xprintf("Length (in Hex) = ");
							PrintChar(dataInd->asduLength);

							xprintf("\n\rData Received = ");					
							for(Tp1 = 0;Tp1<dataInd->asduLength;Tp1++)
							{
								PrintChar(dataInd->asdu[Tp1]);
								xprintf(" ");
							}

							HanddleMSDLClusterParameters(dataInd);
						}
					}
                    
                    /* CUSTOMER Special Cluster */
                    #ifdef PLEASE_SKIP
                    /* Added this to demonstrate processing of generic stack primitive */
                    else if(dataInd->ProfileId.Val == CUSTOMER_PROFILE_ID ||dataInd->ProfileId.Val == MY_PROFILE_ID )
                    {
                        BYTE SeqLen;
                        BYTE i; 
                        BYTE return_length;
                        WORD_VAL ZDODstAddr;
                        BYTE     addr1[2];
                        
                        switch(dataInd->ClusterId.Val)
                        {
                            case BUFFER_TEST_REQUEST_CLUSTER:
                                SeqLen = APLGet();
	                    	
    	                    	if(SeqLen < 66 )
    	                    	{
    		                    	asduData[0] = SeqLen;
    		                    	asduData[1] = SUCCESS;
    		                    	for(i = 2; i < SeqLen+2; i++)
    		                    	{
    			                    	asduData[i] = i;
    			                    }
    			                    return_length = SeqLen+2;
    			                } 
    			                else 
        			            {
    				                asduData[0]     = SeqLen;
                    	            asduData[1]     = 0x01;     /* status fail */
                    	            return_length   = 2;
                    	        }
                    	        
                    	        
                    
                                addr1[0] = dataInd->SrcAddress.ShortAddr.v[0];
                                addr1[1] = dataInd->SrcAddress.ShortAddr.v[1];
                    
                                ZDODstAddr.Val =  ( (addr1[1] << 8) | addr1[0] ); 
                                SendAPPLRequest( ZDODstAddr, BUFFER_TEST_RESPONSE_CLUSTER, asduData, return_length);
       	            	    
                                break;
                                
                                
                            case CUSTOMER_CLUSTER_ID:
                            {
                                
                                BYTE i = 0;
                                
                                /* CUSTOMER Cluster Specific data */
                        	    asduData[i++] = 0x05;          /* frame control */
                        	    asduData[i++] = 0x5f;          /* ManID */
                        	    asduData[i++] = 0x10;          
                        	    asduData[i++] = 0x00;          /* seq */
                        	    asduData[i++] = 0x0A;          /* command */
                        	    
                        	   //leng =  dataInd->asduLength;
                        	   //cmd = dataInd->asdu[leng];
                        	    /* start of payload */
                        	    asduData[i++] = 0x00;
                        	    asduData[i++] = 0x05;
                        	    asduData[i++] = 0x55;
                        	    asduData[i++] = 0x00;
                        	    
                        	    asduData[i++] = 0x06;
                        	    asduData[i++] = 0x66;
                        	    asduData[i++] = 0x00;
                        	    asduData[i++] = 0x07;
                        	    
                        	    asduData[i++] = 0x77;
                        	    asduData[i++] = 0x00;
                        	    asduData[i++] = 0x08;
                        	    asduData[i++] = 0x88;
                        	    
                        	    asduData[i++] = 0x50;
                        	    asduData[i++] = 0x43;
                        	    
                        	    asduData[i++] = 0x50;
                        	    asduData[i++] = 0x43;
                        	    
                        	    asduData[i++]  = 0x50;
                        	    asduData[i++]  = 0x43;
                        	    
                        	    asduData[i++]  = 0x50;
                        	    asduData[i++]  = 0x43;
                        	    
                        	    asduData[i++]   = 0x50;
                        	    asduData[i++]   = 0x43;
                            }
                            
                            /* sending response to the requesting device */
                            if(dataInd->SrcAddrMode == APS_ADDRESS_16_BIT)
                            {
                                addr1[0] = dataInd->SrcAddress.v[0];
                                addr1[1] = dataInd->SrcAddress.v[1];   
                            }
                            else
                            {
                            addr1[0] = 0x00;
                            addr1[1] = 0x00;
                            }
                
                            ZDODstAddr.Val =  ( (addr1[1] << 8) | addr1[0] ); 
                            SendAPPLRequest( ZDODstAddr, CUSTOMER_CLUSTER_ID, asduData, 25);
                            break;   
                                
                            
                            
                            default:
                                break;

                        }
                        
                    }
                    #endif
                    
                    /* else call TestToolHandleDataIndication function  */
                    else
                    {
                        /*Validate whether the specified ProfileId is supported by the
                        end point. If not supported, then we have to ignore the frame. We
                        should not send any response too. If the Validation is successful,
                        then we have to process the received frame. This is required to filter
                        out unsupported messages received by the application.*/
                        if ( App_ValidateProfileId( dataInd->ProfileId.Val, dataInd->DstEndpoint ) )
                        {
                            /*process the received data*/
                            App_HandleReceivedData( dataInd );

                            #if I_SUPPORT_REPORTING == 1
                                SE_UpdateReportReceived( dataInd );
                            #endif /*I_SUPPORT_REPORTING*/
                        }
                    }
                    /*Free up the buffer allocated by the AIL*/
                    nfree(dataInd);
                }
            }
            else if ( *appMessage == MessageTypeAppDataConfirm )
            {
                APP_DATA_confirm    *dataConf;

                dataConf = (APP_DATA_confirm *)appMessage;
                if ( dataConf != NULL )
                {
                    params.APSDE_DATA_confirm.Status                    = dataConf->Status;
                    params.APSDE_DATA_confirm.DstAddrMode               = dataConf->DstAddrMode;
                    params.APSDE_DATA_confirm.DstAddress.ShortAddr.Val  = dataConf->DstAddress.ShortAddr.Val;
                    params.APSDE_DATA_confirm.DstEndpoint               = dataConf->DstEndpoint;
                    params.APSDE_DATA_confirm.SrcEndpoint               = dataConf->SrcEndpoint;
                    params.APSDE_DATA_confirm.TxTime.Val                = dataConf->TxTime.Val;



                    #if I_SUPPORT_REPORTING == 1
                        GetReportingBgTasks( &ReportingBgTasks );
                        if( ReportingBgTasks.flags.bits.bAwaitingReportAttributeConfirm )
                        {
                            ReportingBgTasks.flags.bits.bAwaitingReportAttributeConfirm = FALSE;
                            PutReportingBgTasks(&ReportingBgTasks);
                        }
                    #endif /*I_SUPPORT_REPORTING*/

                    #if I_SUPPORT_CBKE == 1
                        if( ( params.APSDE_DATA_confirm.Status == SUCCESS ) &&
                         ( INTERMEDIATE_STATE == CBKE_GetCurrentState() ) &&
                        ( INITIATE_TERMINATE_KEY_REQ_STATE == CBKE_GetPreviousState() ) )
                        {
                            CBKE_ChangeState( RECD_TERMINATE_KEY_CONFIRM_STATE );
                        }
                    #endif /*#if I_SUPPORT_CBKE == 1*/

                    if( dataConf->SrcEndpoint != 0x00 )
                    {
                        HandleDataConfirm();
                    }

                    /*Update the Confirmation Table information*/
                    /*Currently only the background bit is used. dataConf parameter
                    may be useful in future when actual application is implemented.*/
                    App_UpdateDataConfTable_MTR(dataConf);

                    nfree(dataConf);

                }
            }
        }
            break;

        case APP_START_DEVICE_confirm:

            if ( (!params.NLME_NETWORK_FORMATION_confirm.Status) && ( ExtPANIDStatus.WaitToSendResponse != TRUE ) )
            {
                ConsolePutROMString( (ROM char *)"\r\nPAN " );
                PrintChar( macPIB.macPANId.byte.MSB );
                PrintChar( macPIB.macPANId.byte.LSB );

				StartNetworkFlag = TRUE;
				ConsolePutROMString( (ROM char *)"\r\nWith Channel Number 0x" );
                PrintChar(MyCurrentChannel);
				
                ConsolePutROMString( (ROM char *)" Joined successfully.\r\n" );

                StorePersistancePIB();
                LIGHT0 = 0X00;
				#if (I_SUPPORT_CBKE == 0)
					PrintMenu_MTR();
				#endif
                #if I_SUPPORT_CBKE
				    ConsolePutROMString( (ROM char *)" CBKE Procedure is in Progress...\r\n" );
                    SE_KeyEstStateMachine = START_BCAST_DEL_TIMER_STATE;
                #endif /*#if I_SUPPORT_CBKE*/
				currentPrimitive_MTR = NO_PRIMITIVE;

            }
            else
            {
				if( ExtPANIDStatus.WaitToSendResponse != TRUE )
				{
                	PrintChar( params.NLME_JOIN_confirm.Status );
                	ConsolePutROMString( (ROM char *)" Error joining network.  Try again...\r\n" );

                	printf("\r\n0.Start the device with NVM(Silent start)");
                	printf("\r\n1.Start the device without NVM(Clean start)");
                	currentPrimitive_MTR = APP_START_DEVICE_request;
					if( MSDCL_Commission.ValidCleanStartUp == MSDCL_COMMISSION_DATA_VALID)
					{
						if( ChannelsToBeScanned.Val == 0 )
						{
							ChannelsToBeScanned.Val = MSDCL_Commission.ChannelMask.Val & 0x03FFF800UL;
						}
					}
					else
					{
						if( ChannelsToBeScanned.Val == 0 )
						{
							ChannelsToBeScanned.Val = ALLOWED_CHANNELS_PRE_CONFIG & 0x03FFF800UL;
						}
					}
					{
						unsigned long channelMaskToScan = 0x00000800UL;
						ChannelsToBeScanned.Val &= 0x03FFF800UL;
						if( ( ChannelsToBeScanned.Val & 0x03FFF800UL ) == 0 )
						{
							ChannelsToBeScanned.Val = ALLOWED_CHANNELS_PRE_CONFIG & 0x03FFF800UL;
						}
						while( !(ChannelsToBeScanned.Val & channelMaskToScan) )
						{
							channelMaskToScan <<= 1;	
						}
						ALLOWED_CHANNELS = channelMaskToScan;
						ChannelsToBeScanned.Val &= channelMaskToScan ^ 0xFFFFFFFFUL;
						if( ( ChannelsToBeScanned.Val & 0x03FFF800UL ) == 0 )
						{
							static BYTE numberOfRetriesForDiscovery = 0;
							numberOfRetriesForDiscovery++;
							if( numberOfRetriesForDiscovery >= 12 )
							{
								numberOfRetriesForDiscovery = 12;
								MAXDISCOVERYYRETRIES = 1;
							}
							
						}
					}
    			}
    			else if( ExtPANIDStatus.WaitToSendResponse == TRUE )
    			{
	    			static unsigned char checkForNetwork = 0;
	    			if( checkForNetwork > 3 )
	    			{
		    			checkForNetwork = 0;
		    			ExtPANIDStatus.WaitToSendResponse = FALSE;
		    			ExtPANIDStatus.ReadyResponsePANId = TRUE;	
		    			currentPrimitive_MTR = NO_PRIMITIVE;
		    		}
		    		checkForNetwork++;
	    		}
                menuLevel1 = 'z';
                menuLevel2 = 0xFF;
                
            }
            
            StorePersistancePIB();
            LIGHT0 = 0x00;
            break;

        case NLME_LEAVE_confirm:
            if ( commissioningRestartInProgress )
            {
                App_StartDevice();
            }
            break;
            
        case NLME_PERMIT_JOINING_confirm:
        {
            BYTE returnStatus[2];
            WORD_VAL    DstAddr;
            
            /* Only send out rsp if joining was triggered by mgmtpermitjoiningreq command */
            if(nwkStatus.mgmtPermitJoiningRspPending)
            {
                /* Recovery the sender's address information and use it to respond */
                nwkStatus.mgmtPermitJoiningRspPending = FALSE;
                
                DstAddr.Val     = nwkStatus.mgmtPermitJoiningRspDstAddr.Val;
                returnStatus[0] = params.NLME_PERMIT_JOINING_confirm.Status;
                SendZDPRequest(DstAddr, MGMT_PERMIT_JOINING_rsp, returnStatus, 0x01);
            } 
            
            /* the rsp will be sent out when the stack has the time */
            currentPrimitive_MTR = NO_PRIMITIVE;
            break;
        }

        #if APSME_GRP_ENABLE == 1
        case APSME_ADD_GROUP_confirm:
            HandleAddGroupConfirm();
            currentPrimitive_MTR = NO_PRIMITIVE;
        break;
        case APSME_REMOVE_GROUP_confirm:
            /* Give remove group confirm to test tool */
            HandleRemoveGroupConfirm();
            currentPrimitive_MTR = NO_PRIMITIVE;
        break;

        case APSME_REMOVE_ALL_GROUPS_confirm:
            /* Give remove all groups confirm to test tool */
            HandleRemoveAllGroupsConfirm();
            currentPrimitive_MTR = NO_PRIMITIVE;
        break;
        #endif

    #if I_SUPPORT_STUB_APS == 1
        case INTRP_DATA_confirm:
            APP_HandleInterPANDataConfirm_MTR();

            break;
        case INTRP_DATA_indication:

            if ( params.INTRP_DATA_indication.ProfileId.Val == SE_PROFILE_ID )
            {
                APP_HandleInterPANDataIndication_MTR();
            }
            else
            {
                currentPrimitive_MTR = NO_PRIMITIVE;
            }

            APLDiscardRx();
            break;
    #endif
        default:
            currentPrimitive_MTR = NO_PRIMITIVE;
            break;
    }
}

/***************************************************************************************
 * Function:        void AppJoinRequest(NETWORK_DESCRIPTOR *nwkDescriptor, BYTE numberOfNetworks )
 *
 * PreCondition:    None
 *
 * Input:           nwkDesExtendedPANID: List of Network Extended PANID.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function will decide the network to join after getting the
 *                  list of network available.
 *
 * Note:            Application should implement the selection of network to join
 *                  depending on application criteria.
 *
 ***************************************************************************************/
void AppJoinRequest( NETWORK_DESCRIPTOR *nwkDescriptor, BYTE numberOfNetworks )
{
    BYTE i;
    for (i = 0; i < numberOfNetworks; i++)
    {
        if (nwkDescriptor[i].PermitJoining)
        {
            /* Join the first network in the network list */
            JoinRequest(nwkDescriptor[i].ExtendedPANID, ASSOCIATION_REJOIN);
            break;
        }
    }
    if (i == numberOfNetworks)
    {
        JoinRequest(nwkDescriptor[0].ExtendedPANID, ASSOCIATION_REJOIN);
    }

}

/***************************************************************************************
 * Function:        void HandleFragmentDataIndication ( WORD_VAL dataLength, WORD_VAL *data )
 *
 * PreCondition:    All the fragments have been received and aggregated
 *
 * Input:           dataLength - Total number of bytes after aggregated all the blocks
 *                  data -       Actual data that needs to be sent to the application
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to send the indication to the Test Tool. We have
 *                  Total number of bytes in application payload per data indication is 96. This is
 *                  because we have to send other parameters like, source address, end point
 *                  information etc. This function keeps sending data indication to Test Tool
 *                  for each 96 bytes of application payload.
 *
 * Note:            None
 *
 ***************************************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
void HandleFragmentDataIndication( WORD dataLength, BYTE *data )
{
    if( NOW_I_AM_A_ROUTER() && ( params.APSDE_DATA_indication.ClusterId.Val == ZCL_MESSAGING_CLUSTER ))
    {
        Handle_Fragmented_MessageCluster(data);
    }
    ConsolePutROMString( (ROM char *)"\r\nRecieved fragmented data of length 0x" );
    PrintChar( dataLength >> 0x08 );
    PrintChar( dataLength );
    ConsolePutROMString( (ROM char *)" Bytes ");
    ConsolePutROMString( (ROM char *)"\r\nPress enter to return to main menu" );
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/***************************************************************************************
 * Function:        void HandleFragmentDataConfirm( void )
 *
 * PreCondition:    All the blocks have been transmitted successfully and Acknowledged.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to send the APSDE_DATA_confirm to Test Tool
 *
 * Note:            None
 *
 ***************************************************************************************/
#if (I_SUPPORT_FRAGMENTATION == 1)
//void HandleFragmentDataConfirm( void )
//{
//    ConsolePutROMString( (ROM char *)"\r\nSent fragmented data " );
//    ConsolePutROMString( (ROM char *)"\r\nPress enter to return to main menu" );
//
//}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/***************************************************************************************
 * Function:        void GetFragmentDataBuffer( void )
 *
 * PreCondition:    Fragmentation is supported
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This returns the static buffer used for fragmentation
 *
 * Note:            None
 *
 ***************************************************************************************/
 
#if (I_SUPPORT_FRAGMENTATION == 1)

//BYTE* GetFragmentDataBuffer( void )
//{
//    return fragmentData;
//}

#endif

/***************************************************************************************
 * Function:        void AppNotification( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called back by stack to notify application about stack changes. The imeplementation of the
 *                  function has been left to the application.
 *
 * Note:            None
 *
 ***************************************************************************************/

//void AppNotification( void )
//{
//
//}

/*********************************************************************
 * Function:        void SendZDPRequest( WORD_VAL DstAddr, WORD ClusterId, BYTE * Asdu, BYTE AsduLength)
 *
 * PreCondition:    None
 *
 * Input:           Destination Address, Cluster Identifier, ASDU
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Prepare the APP_DATA_Req and enqueue the same on to AIL
 *
 * Note:            None
 ********************************************************************/
//void SendZDPRequest( WORD_VAL DstAddr, WORD ClusterId, BYTE * Asdu, BYTE AsduLength)
//{
//    APP_DATA_request* pAppDataReq;
//
//    pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);
//    if(pAppDataReq != NULL)
//    {
//        pAppDataReq->asduLength = AsduLength + 1;
//        pAppDataReq->DstAddress.ShortAddr.Val = DstAddr.Val;
//        pAppDataReq->ClusterId.Val = ClusterId;
//        pAppDataReq->DstAddrMode = APS_ADDRESS_16_BIT;
//        pAppDataReq->DstEndpoint =  ZDOENDPOINT;
//        pAppDataReq->ProfileId.Val = ZDO_PROFILE_ID;
//        pAppDataReq->SrcEndpoint = ZDOENDPOINT;
//        switch (pAppDataReq->ClusterId.Val)
//        {
//            case END_DEVICE_BIND_req: /* Fall through */
//            case BIND_req: /* Fall through */
//            case UNBIND_req:
//                {
//                    pAppDataReq->asdu[0] = APSCounter++;
//                }
//                break;
//            default:
//                {
//                    pAppDataReq->asdu[0] = ZDOCounter++;
//                }
//                break;
//        }
//        memcpy(&(pAppDataReq->asdu[1]),Asdu,AsduLength);
//        pAppDataReq->TxOptions.Val = DEFAULT_TX_OPTIONS;
//        pAppDataReq->TxOptions.bits.acknowledged = 0;
//        pAppDataReq->RadiusCounter =  DEFAULT_RADIUS;
//        if ( AILEnqueue( (BYTE *)pAppDataReq, TransmitQueueID ) == AILQueueFailure )
//        {
//            nfree(pAppDataReq);
//        }
//    }
//}


/* send general application primitive - WRITTEN FOR TEACHING CLASS */
//void SendAPPLRequest( WORD_VAL DstAddr, WORD ClusterId, BYTE * Asdu, BYTE AsduLength)
//{
//    APP_DATA_request* pAppDataReq;
//
//    pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);
//    if(pAppDataReq != NULL)
//    {
//        pAppDataReq->asduLength = AsduLength;
//        pAppDataReq->DstAddress.ShortAddr.Val = DstAddr.Val;
//        pAppDataReq->ClusterId.Val = ClusterId;
//        pAppDataReq->DstAddrMode = APS_ADDRESS_16_BIT;
//        pAppDataReq->DstEndpoint =  CUSTOMER_APP_ENDPOINT;
//        pAppDataReq->ProfileId.Val = CUSTOMER_PROFILE_ID;
//        pAppDataReq->SrcEndpoint = CUSTOMER_APP_ENDPOINT;
//        switch (pAppDataReq->ClusterId.Val)
//        {
//            
//            default:
//                {
//                   //pAppDataReq->asdu[0] = APSCounter++;
//                }
//                break;
//        }
//        memcpy(&(pAppDataReq->asdu[0]),Asdu,AsduLength);
//        pAppDataReq->TxOptions.Val = DEFAULT_TX_OPTIONS;
//        pAppDataReq->TxOptions.bits.acknowledged = 0;
//        pAppDataReq->RadiusCounter =  DEFAULT_RADIUS;
//        if ( AILEnqueue( (BYTE *)pAppDataReq, TransmitQueueID ) == AILQueueFailure )
//        {
//            nfree(pAppDataReq);
//        }
//    }
//}

/***************************************************************************************
 * Function:        void SetAppLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails)

 * Input:           pappLinkKeyDetails - The received application link key details
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to store the application link key
 *                  received from TC.
 *
 * Note:            None
 *
 ***************************************************************************************/
#if I_SUPPORT_LINK_KEY == 1
//void SetAppLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails)
//{
//    BYTE    index;
//    BYTE    freeEntry = 0xFF;
//    BYTE    entryNotFound;
//
//    // Get ROM address of neighborTable in RAM.
//    APS_KEY_PAIR_DESCRIPTOR currentKey;
//    WORD pCurrentKey;
//    pCurrentKey = appLinkKeyTable;
//
//
//    #ifdef USE_EXTERNAL_NVM
//    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey+=(WORD)sizeof(APS_KEY_PAIR_DESCRIPTOR), index++)
//    #else
//    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey++, index++)
//    #endif
//    {
//        // Read the record into RAM.
//        GetLinkKey(&currentKey, pCurrentKey );
//
//        // Now check to see if it is in use or not.
//
//        if ( currentKey.InUse )
//        {
//            entryNotFound = memcmp ( (&pappLinkKeyDetails->PartnerAddress), &currentKey.PartnerAddress, 8 );
//            if( entryNotFound == FALSE )
//            {
//                pappLinkKeyDetails->IncomingFrameCounter.Val = 0x00;
//                PutLinkKey( pCurrentKey, pappLinkKeyDetails );
//                return;
//            }
//        }
//        else
//        {
//            freeEntry = index;
//        }
//    }
//    if ( freeEntry != 0xFF )
//    {
//        #ifdef USE_EXTERNAL_NVM
//            pCurrentKey = appLinkKeyTable + (WORD)( sizeof(APS_KEY_PAIR_DESCRIPTOR) * freeEntry);
//        #else
//            pCurrentKey = appLinkKeyTable + freeEntry;
//        #endif
//        pappLinkKeyDetails->InUse = 0x01;
//        pappLinkKeyDetails->IncomingFrameCounter.Val = 0x00;
//        PutLinkKey( pCurrentKey, pappLinkKeyDetails );
//    }
// }


#endif

/***************************************************************************************
 * Function:        void RemoveAppLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails)

 * Input:           pappLinkKeyDetails - The received application link key details
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is used to remove the application link key
 *
 * Note:            None
 *
 ***************************************************************************************/
#if I_SUPPORT_LINK_KEY == 1
//void RemoveAppLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails)
//{
//    BYTE    index;
//    BYTE    entryNotFound;
//
//    // Get ROM address of neighborTable in RAM.
//    APS_KEY_PAIR_DESCRIPTOR currentKey;
//    WORD pCurrentKey;
//    pCurrentKey = appLinkKeyTable;
//
//
//    #ifdef USE_EXTERNAL_NVM
//    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey+=(WORD)sizeof(APS_KEY_PAIR_DESCRIPTOR), index++)
//    #else
//    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey++, index++)
//    #endif
//    {
//        // Read the record into RAM.
//        GetLinkKey(&currentKey, pCurrentKey );
//
//        // Now check to see if it is in use or not.
//
//        if ( currentKey.InUse )
//        {
//            entryNotFound = memcmp ( (&pappLinkKeyDetails->PartnerAddress), &currentKey.PartnerAddress, 8 );
//            if( entryNotFound == FALSE )
//            {
//                pappLinkKeyDetails->InUse = 0x00;
//                PutLinkKey( pCurrentKey, pappLinkKeyDetails );
//                return;
//            }
//        }
//    }
// }
#endif

/***************************************************************************************
 * Function:        BOOL SelectLinkKey(LONG_ADDR DstAddress, TC_LINK_KEY_TABLE *tcLinkKeyTable)

 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 *
 ***************************************************************************************/
//#if I_SUPPORT_LINK_KEY == 1 && I_SUPPORT_PRECONFIGURED_TC_LINK_KEY == 1 && USE_COMMON_TC_LINK_KEY == 0 && I_SUPPORT_MULTIPLE_TC_LINK_KEY == 1
//BOOL SelectLinkKey(LONG_ADDR DstAddress, TC_LINK_KEY_TABLE *tcLinkKeyTable)
//{
//    if ( SearchForTCLinkKey(DstAddress, tcLinkKeyTable) )
//    {
//        return TRUE;
//    }
//    return FALSE;
// }
//#endif


/******************************************************************************
 * Function:        BYTE App_Set_SystemTime
 *                  (
 *                      BYTE year,
 *                      BYTE month,
 *                      BYTE day,
 *                      BYTE hour,
 *                      BYTE minute,
 *                      BYTE second
 *                  );
 *
 * PreCondition:
 *
 * Input:           year - year to be set
 *                  month - current month
 *                  day - current day
 *                  hour - current hour
 *                  minute  current minute
 *                  second  - current second
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    Success( 1 ) or failure( 0 )
 *
 * Overview:        This function sets the time given from the Application
 *                  into the RTC. It also determines the seconds elapsed
 *                  from January 2000 00 hours and writes the value into
 *                  the Time attribute of Time cluster
 *
 * Note:
 *****************************************************************************/
//BYTE App_Set_SystemTime
//(
//     BYTE year,
//     BYTE month,
//     BYTE day,
//     BYTE hour,
//    BYTE minute,
//    BYTE second
// )
//{
//    /*give the time in year format*/
//    elapsedTimeInfo elapsedTime;
//    elapsedTime.year = year;
//    elapsedTime.month = month;
//    elapsedTime.day = day;
//    elapsedTime.hour = hour;
//    elapsedTime.minute = minute;
//    elapsedTime.second = second;
//    /*Set the current time in the RTC*/
//    App_SetCurrentTime( &elapsedTime );
//    return 0;
//}

/******************************************************************************
 * Function:        void App_UpdateParamsForTestTool
 *                  (
 *                      APP_DATA_indication *dataInd,
 *                   );
 *
 *
 * PreCondition:    None
 *
 *  Input:          dataInd - data indication as received from the lower layer
 *
 *
 * Output:
 *
 *
 * Return :
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:
 *****************************************************************************/
//void App_UpdateParamsForTestTool
//(
//    APP_DATA_indication *dataInd
//)
//{
//    /* No Action for Sample Application*/
//}

/******************************************************************************
 * Function:        static void HandleDataConfirm( void )
 *
 * PreCondition:
 *
 * Input:
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:
 *
 * Overview:
 *
 * Note:
 *****************************************************************************/
static void HandleDataConfirm (void)
{

}

/*********************************************************************
 * Function:        void App_HandleDataIndication( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        APSDE Data Indication is given to the test tool
 *
 * Note:            None
 ********************************************************************/
//void App_HandleDataIndication( void )
//{
//}

/*********************************************************************
 * Function:        void App_Handle_CBKE_Status(status)
 *
 * PreCondition:    None
 *
 * Input:           status - status of CBKE procedure
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Application handling, after the CBKE procedure.
 *
 * Note:            None
 ********************************************************************/
#if (I_SUPPORT_CBKE == 0x01) 
void App_Handle_CBKE_Status(BYTE status)
{
	if( NOW_I_AM_NOT_A_CORDINATOR() ){
	    if ( CBKE_SUCCESS != status )
	    {
	        /*The device should leave the network if the CBKE is not
	          successful*/
	        SE_SendNLMELeaveRequest();
	        printf("\r\nCBKE Failed. \r\nPress Reset Button to start the device");
	    }
	    else
	    {
	        PrintMenu_MTR();
	    }
    }   
}
#endif

/************************************************************************************
 * Function:        void ZCL_Callback_HandleTimeAttributes(attributeId,Length,dataIndex)
 *
 * PreCondition:    None
 *
 * Input:           attributeId - attribute id of the particular time attribute
 *                  Length - Length of attribute data
 *                  dataIndex - pointer to the data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Handles the different attributes of the time cluster
 *
 * Note:            None
 ***********************************************************************************/

void ZCL_Callback_HandleTimeAttributes( BYTE endPointId, WORD attributeId, BYTE Length,BYTE* dataIndex)
{
    if( ZCL_TIME_ATTRIBUTE == attributeId  )
    {
       /*synchronize the time, update the time attribute
        and set the RTC to tick with the available seconds*/

        Time_Synchronization( endPointId, dataIndex );
        if( NOW_I_AM_A_ROUTER() ){ //#if( I_AM_MTR == 1 )
            /* For Simulating meterReadings,we need to enable backgroundTask.*/
            meterStatus.flags.bits.bSimulateMeterReading = 1;
            meterStatus.simulateMeterStartTime = TickGet();
        }//#endif      /* I_AM_MTR */
    } 
    else
    {            
        switch( attributeId )
        {
            case ZCL_TIME_STATUS:
            case ZCL_TIME_ZONE:
            case ZCL_TIME_DST_START:
            case ZCL_TIME_DST_END:
            case ZCL_TIME_DST_SHIFT:
            case ZCL_TIME_STANDARD_TIME:
            case ZCL_TIME_LOCAL_TIME:
                ConsolePutROMString( (ROM char *)"\r\nAttribute Value for AttributeId of" );
                PrintChar( attributeId >> 0x08 );
                PrintChar( ( attributeId & 0x00FF ) );
                ConsolePutROMString( (ROM char *)" length " );
                PrintChar(Length);
                ConsolePutROMString( (ROM char *)" 0x " );
                while( Length > 0 )
                {
                    PrintChar(*(dataIndex+(--Length)));
                    //Length--;
                    //dataIndex++;
                }
                ConsolePutROMString( (ROM char *)"\r\nHit enter key to return to the main menu" ); 
         }
     }
    
}

