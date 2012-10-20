
/*********************************************************************
 *
 *                Application ESP
 *
 *********************************************************************
 * FileName:        App_ESP.c
 * Dependencies:
 * Processor:       PIC24FJ128GA010
 * Complier:        MPLab C30 C
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright � 2004-2011 Microchip Technology Inc.  All rights reserved.
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
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 ********************************************************************/

/******************************************************************************
   Includes
 *****************************************************************************/
/*Configuration Files*/
#include "zigbee.def"

/*Generic Files*/
#include "generic.h"
#include "sralloc.h"
#include "zNVM.h"
#include "rtcc.h"
#include "HAL.h"
#include "Console.h"

/*Stack Related Files*/
#include "zAPL.h"
#include "zAIL.h"
#include "zAPS.h"
#include "zSecurity.h"
#include "zMAC.h"

/*ZCL Related Files*/
#include "ZCL_Interface.h"

/*SE Related Files*/
#include "SE_Interface.h"
#include "SE_Profile.h"
#include "SE_ESP.h"
#include "SE_MTR.h"
#include "SE_Display.h"

/*Application Related Files*/
#include "App_Common.h"
#include "App_ESP.h"
#include "Menu_ESP.h"

/*Libraries*/
#if defined(__C30__)
    #include "TLS2130.h"
#else
    #include "lcd PIC32.h"
#endif

#if I_SUPPORT_CBKE == 1
    #include "eccapi.h"
    #include "SE_CBKE.h"
    #include "ZCL_Key_Establishment.h"
#endif /*#if I_SUPPORT_CBKE*/

#include "MSDCL_Commissioning.h"
#include "msdlclusterhanddle.h"
#include "Delay.h"
#include "monitor.h"
#include "SymbolTime.h"
/*****************************************************************************
   Constants and Enumerations
 *****************************************************************************/

#if defined(MCHP_C18)
    #if defined(__18F4620)
        #pragma romdata CONFIG1H = 0x300001
        const rom unsigned char config1H = 0b00000110;      // HSPLL oscillator

        #pragma romdata CONFIG2L = 0x300002
        const rom unsigned char config2L = 0b00011111;      // Brown-out Reset Enabled in hardware @ 2.0V, PWRTEN disabled

        #pragma romdata CONFIG2H = 0x300003
        const rom unsigned char config2H = 0b00010010;      // HW WD disabled, 1:512 prescaler

        #pragma romdata CONFIG3H = 0x300005
        const rom unsigned char config3H = 0b10000000;      // PORTB digital on RESET

        #pragma romdata CONFIG4L = 0x300006
        const rom unsigned char config4L = 0b10000001;      // DEBUG disabled,
                                                            // XINST disabled
                                                            // LVP disabled
                                                            // STVREN enabled
    #elif defined(__18F87J10)
        #pragma romdata CONFIG1L = 0x1FFF8
        const rom unsigned char config1L = 0b10111110;

        #pragma romdata CONFIG1H = 0x1FFF9
        const rom unsigned char config1H = 0b11111111;

        #pragma romdata CONFIG2L = 0x1FFFA
        const rom unsigned char config2L = 0b11111101;

        #pragma romdata CONFIG2H = 0x1FFFB
        const rom unsigned char config2H = 0b11111010;

        #pragma romdata CONFIG3L = 0x1FFFC
        const rom unsigned char config3L = 0b11111111;

        #pragma romdata CONFIG3H = 0x1FFFD
        const rom unsigned char config3H = 0b11111111;

    #else
        #error Cannot define configuration bits for this device
    #endif

    #pragma romdata
#elif defined( __C30__ )
    #if defined(__PIC24F__)
        // Explorer 16 board
        _CONFIG2(FNOSC_PRI & POSCMOD_XT)    // Primary XT OSC with 4X PLL
		#if defined(YODA_V1)
			_CONFIG1(JTAGEN_OFF & FWDTEN_OFF & ICS_PGx2 & WDTPS_PS32768)   // JTAG off, watchdog timer off
		#elif defined(YODA_V1A)
			_CONFIG1(JTAGEN_OFF & FWDTEN_OFF & ICS_PGx2 & WDTPS_PS32768)   // JTAG off, watchdog timer off
		#endif
        
    #elif defined(__dsPIC33F__) || defined(__PIC24H__)
        // Explorer 16 board
        _FOSCSEL(FNOSC_PRI)             // primary osc
        _FOSC(OSCIOFNC_OFF & POSCMD_XT) // XT Osc
        _FWDT(FWDTEN_OFF)               // Disable Watchdog timer
        // JTAG should be disabled as well
    #elif defined(__dsPIC30F__)
        // dsPICDEM 1.1 board
        _FOSC(XT_PLL16)     // XT Osc + 16X PLL
        _FWDT(WDT_OFF)      // Disable Watchdog timer
        _FBORPOR(MCLR_EN & PBOR_OFF & PWRT_OFF)
    #else
        #error Cannot define configuration bits for this device
    #endif
#elif defined( __C32__ )
    #pragma config FPLLMUL = MUL_16, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
    #pragma config POSCMOD = XT, FNOSC = PRIPLL, FPBDIV = DIV_4, WDTPS = PS131072

#else
    #error Cannot define configuration bits for this device family
#endif

/*****************************************************************************
  Customizable Macros
 *****************************************************************************/
  /* None */

/******************************************************************************
  Data Structures
******************************************************************************/
  /* None */
TICK StartTime_GetNode;
/*****************************************************************************
   Variable Definitions
 *****************************************************************************/
extern NWK_STATUS nwkStatus;
extern BYTE ZDOCounter;
extern NETWORK_DESCRIPTOR  *NetworkDescriptor;
extern BYTE APSCounter;
extern BYTE startMode;
extern BYTE *reportAttributeValue;
extern BYTE AllowJoin;

#ifdef I_SUPPORT_SECURITY
    extern SECURITY_STATUS      securityStatus;
    extern NETWORK_KEY_INFO plainSecurityKey[2];
#endif




#if (I_SUPPORT_FRAGMENTATION == 1)
    /* Global static array to store the Fragmentation data payload */
    BYTE fragmentData[MAX_FRAGMENTATION_SIZE];
#endif

ZIGBEE_PRIMITIVE currentPrimitive_ESP;
extern ZIGBEE_PRIMITIVE *currentPrimitive_PTR;

extern InterPANDataRequest interPANDataRequest;
LONG_ADDR nwkDesExtendedPANID;
#ifdef DEBUG_LOG
    BYTE NoOfPANDescriptorFound;
    BYTE PotentialParentFound;
    BYTE overWriteCheck;
#endif

/*This variable indicated the transaction number.
  It used in ZCL header of a ZCL command frame.*/
BYTE appNextSeqNum_ESP = 0x00;

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

#ifdef I_SUPPORT_SECURITY
    #define DEFAULT_TX_OPTIONS          0x02 // NWK KEY ENABLED
#else
    #define DEFAULT_TX_OPTIONS          0x00 // UNSECURED
#endif

#if (I_SUPPORT_FRAGMENTATION == 1)
    BYTE* GetFragmentDataBuffer( void );
#endif

extern void RTCCProcessEvents();
void HandleChkConnections(void);
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
void AppInit();

/***************************************************************************************
 * Function:        void AppTasks()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Application state machine. Handling of Data confirmation and indication.
 *
 * Note:            None
 ***************************************************************************************/
void AppTasks();

/******************************************************************************
 * Function:        static void HandleDataConfirm( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function will be called after reception of data conf.
 *                  Handling of confirmation (application specific)
 *                  is left to app developer.
 *
 * Note:
 *****************************************************************************/
static void HandleDataConfirm( void );

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
int main_ESP( void )
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
	ConsolePutROMString( (ROM char *)"\r\nESP Device\r\n" );

    nwkStatus.flags.Val = 0x00;

    currentPrimitive_ESP = NO_PRIMITIVE;

    #if defined(I_SUPPORT_SECURITY)
    {
        BYTE i;
        i = 0xff;
        PutNwkActiveKeyNumber(&i);
    }
    #endif

    #ifdef USE_EXTERNAL_NVM
        NVMInit();
    #endif

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

   	inputBuf_ESP.used = 0;
   	inputBuf_ESP.buffer[1] = 13;
	
	if( MSDCL_Commission.ValidCleanStartUp )
	{
		printf("\r\nStart the device without NVM(Clean start)\r\n");
		inputBuf_ESP.buffer[0] = '1';
	}
	else
	{
		printf("\r\nStart the device with NVM(Silent start)\r\n");
		inputBuf_ESP.buffer[0] = '0';
	}	
   		
    
    AppInit();

    menuLevel1 = 'z';
    menuLevel2 = 0xFF;
    menuLevel3 = 0x01;
    
    ProcessMenu_ESP();  

    ENABLE_WDT();
    LIGHT3 = 0x00;
	StartTime_GetNode = TickGet();
    while( 1 )
    {
        CLRWDT();
        HandleUARTRequests();
		HanddleUART2();
		HandleChkConnections();
        StackStatus = ZigBeeTasks( &currentPrimitive_ESP );
        AppTasks();
        RTCCProcessEvents();
        //HandleESPDisplay();
        #if I_SUPPORT_CBKE == 1
            SE_CBKE_StateMachineHandler();
        #endif /*I_SUPPORT_CBKE*/

    }
}



void HandleChkConnections(void)
{
	TICK currentTime;
	
	currentTime = TickGet();
/*
*/
									//StartTime
/*	if( ( TickGetDiff( currentTime,StartTime_GetNode) ) > (ONE_SECOND)*4 )
	{
		//xprintf("\n\r Now Going to check all Connections\n\r");
		NumberOfDevicesConnected = 0;
		int index;
		#ifdef USE_EXTERNAL_NVM
			for (index=0, pCurrentNeighborRecord = neighborTable; index < MAX_NEIGHBORS; index++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
		#else
			for (index=0, pCurrentNeighborRecord = neighborTable; index < MAX_NEIGHBORS; index++, pCurrentNeighborRecord++)
		#endif // #ifdef USE_EXTERNAL_NVM
			{
				GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
				//xprintf("\n\rcurrentNeighborRecord.shortaddr = %u\n\r",currentNeighborRecord.shortAddr.v[0],currentNeighborRecord.shortAddr.v[1]);
				if(currentNeighborRecord.longAddr.v[0] != 0 || 
				   currentNeighborRecord.longAddr.v[1] != 0 ||
				   currentNeighborRecord.longAddr.v[2] != 0 || 
				   currentNeighborRecord.longAddr.v[3] != 0 ||
				   currentNeighborRecord.longAddr.v[4] != 0 || 
				   currentNeighborRecord.longAddr.v[5] != 0 ||
				   currentNeighborRecord.longAddr.v[6] != 0 || 
				   currentNeighborRecord.longAddr.v[7] != 0)
				{
					
					CurrentDeviceConnectedInfo[NumberOfDevicesConnected].shortAddr = currentNeighborRecord.shortAddr;
					CurrentDeviceConnectedInfo[NumberOfDevicesConnected].longAddr = currentNeighborRecord.longAddr;
					CurrentDeviceConnectedInfo[NumberOfDevicesConnected].panID = currentNeighborRecord.panID;
					NumberOfDevicesConnected++;
				}
			}
		
		StartTime_GetNode = TickGet();

	}*/

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
void AppInit()
{
    /*App_InitAttributeStorage() should be called befor App_TimeInit()*/
    App_InitAttributeStorage();

    App_TimeInit();
    App_PriceInit();
    App_MessageTableInit();
}

/***************************************************************************************
 * Function:        void AppTasks()
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None.
 *
 * Side Effects:    None
 *
 * Overview:        Application state machine. Handling of Data confirmation and indication.
 *
 * Note:            None
 ***************************************************************************************/
 extern unsigned char MyCurrentChannel;
 extern unsigned char StartNetworkFlag;
void AppTasks()
{
    switch ( currentPrimitive_ESP )
    {
        case NO_PRIMITIVE:
        {
            BYTE                    *appMessage;

            appMessage = (BYTE *) AILDequeue( ReceiveQueueID );
            if (appMessage == NULL)
            {
                /*Handle Pending Data in the backgound*/
                App_HandlePendingData_ESP();
                return;
            }

			xprintf("Massage is there :- %u",(*appMessage));
			
            if ( *appMessage == MessageTypeAppDataIndication )
            {
                APP_DATA_indication     *dataInd;
                dataInd = (APP_DATA_indication *)appMessage;
                if ( dataInd != NULL )
                {
                    if( params.ZDO_DATA_indication.ProfileId.Val == ZDO_PROFILE_ID )
                    {
                       /* Do nothing */
                    }
                    /* else call TestToolHandleDataIndication function  */
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
                                
                            case CUSTOMER_CLUSTER_ID1:
							case CUSTOMER_CLUSTER_ID2:						
							case CUSTOMER_CLUSTER_ID3:							
							case CUSTOMER_CLUSTER_ID4:
						    case ZCL_COMMISSIONING_CLUSTER:
                           {
                                
                                BYTE i = 0;
                                BYTE cmd, leng;
                               

                        	   leng =  dataInd->asduLength;
                        	   cmd = dataInd->asdu[leng];
                        	   for(i=0; i < leng; i++)
                        	   {
                            	  PrintChar(dataInd->asdu[i]); 
                                  printf(":"); 
                               }    
                            	HanddleMSDLClusterParameters(dataInd);
                            	   
                            }
                            break;    
                            
                            default:
                                break;

                        }
                        
                    }
                    
                    
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
                    App_UpdateDataConfTable_ESP(dataConf);

                    nfree(dataConf);

                }
            }
        }
            break;

        case APP_START_DEVICE_confirm:

            if (!params.NLME_NETWORK_FORMATION_confirm.Status)
            {
                ConsolePutROMString( (ROM char *)"\r\nPAN " );
                PrintChar( macPIB.macPANId.byte.MSB );
                PrintChar( macPIB.macPANId.byte.LSB );
				ConsolePutROMString( (ROM char *)"\r\nWith Channel Number 0x" );
                PrintChar(MyCurrentChannel);
				
                ConsolePutROMString( (ROM char *)" Network Formed successfully.\r\n" );
				//x2puts("Network Started Properly");
				x2putc(0x2B); 
				x2putc(0x06);
				x2putc(0x82); 
				x2putc(0x73); 
				x2putc(0x63); 
				x2putc(0x00); 
				x2putc(0x01);
				StartNetworkFlag = TRUE;
                StorePersistancePIB();
                LIGHT0 = 0X00;
                PrintMenu_ESP();
                currentPrimitive_ESP = NO_PRIMITIVE;
            }
            else
            {

                PrintChar( params.NLME_NETWORK_FORMATION_confirm.Status );
                ConsolePutROMString( (ROM char *)" Err forming network. Try again...\r\n" );
                printf("\r\n0.Start the device with NVM(Silent start)");
                printf("\r\n1.Start the device without NVM(Clean start)");
                menuLevel1 = 'z';
                menuLevel2 = 0xFF;
                menuLevel3 = 0x01;
                currentPrimitive_ESP = APP_START_DEVICE_request;
            }
            
            StorePersistancePIB();
            LIGHT0 = 0X00;
            break;

        case NLME_LEAVE_confirm:
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
            currentPrimitive_ESP = NO_PRIMITIVE;
            break;
        }
        #if APSME_GRP_ENABLE == 1
        case APSME_ADD_GROUP_confirm:
            HandleAddGroupConfirm();
            currentPrimitive_ESP = NO_PRIMITIVE;
        break;
        case APSME_REMOVE_GROUP_confirm:
            /* Give remove group confirm to test tool */
            HandleRemoveGroupConfirm();
            currentPrimitive_ESP = NO_PRIMITIVE;
        break;

        case APSME_REMOVE_ALL_GROUPS_confirm:
            /* Give remove all groups confirm to test tool */
            HandleRemoveAllGroupsConfirm();
            currentPrimitive_ESP = NO_PRIMITIVE;
        break;
        #endif

    #if I_SUPPORT_STUB_APS == 1
        case INTRP_DATA_confirm:
            APP_HandleInterPANDataConfirm_ESP();

            break;
        case INTRP_DATA_indication:

            if ( params.INTRP_DATA_indication.ProfileId.Val == SE_PROFILE_ID )
            {
                APP_HandleInterPANDataIndication_ESP();
            }
            else
            {
                currentPrimitive_ESP = NO_PRIMITIVE;
            }

            APLDiscardRx();
            break;
    #endif
        default:
            currentPrimitive_ESP = NO_PRIMITIVE;
            break;
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
//void HandleFragmentDataIndication( WORD dataLength, BYTE *data )
//{
//    ConsolePutROMString( (ROM char *)"\r\nRecieved fragmented data of length 0x" );
//    PrintChar( dataLength >> 0x08 );
//    PrintChar( dataLength );
//    ConsolePutROMString( (ROM char *)" Bytes ");
//    ConsolePutROMString( (ROM char *)"\r\nPress enter to return to main menu" );
//    
//    /* No dynamic memory allocated */
//    //nfree(data); // Free the allocated buffer from stack for total aggregated application payload.
//}
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
void HandleFragmentDataConfirm( void )
{
    ConsolePutROMString( (ROM char *)"\r\nSent fragmented data " );
    ConsolePutROMString( (ROM char *)"\r\nPress enter to return to main menu" );
}
#endif //(I_SUPPORT_FRAGMENTATION == 1)

/***************************************************************************************
 * Function:        void GetFragmentDataBuffer( void )
 *
 * PreCondition:    Static buffer is allocated by the application
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

BYTE* GetFragmentDataBuffer( void )
{
    return fragmentData;
}

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
void AppNotification( void )
{

}

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
 
 extern struct __MyCommandFlag MyCommandFlag;
 extern LONG_ADDR	PrvSendIeeeAddr;
 
void SendZDPRequest( WORD_VAL DstAddr, WORD ClusterId, BYTE * Asdu, BYTE AsduLength)
{
    APP_DATA_request* pAppDataReq;

    pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);
    if(pAppDataReq != NULL)
    {
        pAppDataReq->asduLength = AsduLength + 1;
        pAppDataReq->DstAddress.ShortAddr.Val = DstAddr.Val;
        pAppDataReq->ClusterId.Val = ClusterId;
        pAppDataReq->DstAddrMode = APS_ADDRESS_16_BIT;
        pAppDataReq->DstEndpoint =  ZDO_END_POINT;
        pAppDataReq->ProfileId.Val = ZDO_PROFILE_ID;
        pAppDataReq->SrcEndpoint = ZDO_END_POINT;
        switch (pAppDataReq->ClusterId.Val)
        {
            case END_DEVICE_BIND_req: /* Fall through */
            case BIND_req: /* Fall through */
            case UNBIND_req:
                {
                    pAppDataReq->asdu[0] = APSCounter++;
                }
                break;
            default:
                {
                    pAppDataReq->asdu[0] = ZDOCounter++;
                }
                break;
        }
        memcpy(&(pAppDataReq->asdu[1]),Asdu,AsduLength);
        pAppDataReq->TxOptions.Val = ZDP_DEFAULT_TX_OPTIONS;
        pAppDataReq->TxOptions.bits.acknowledged = 0;
        pAppDataReq->RadiusCounter =  DEFAULT_RADIUS;
		
		
		
		
		
        if ( AILEnqueue( (BYTE *)pAppDataReq, TransmitQueueID ) == AILQueueFailure )
        {
            nfree(pAppDataReq);
        }
    }
}



/* send general application primitive - WRITTEN FOR TEACHING CLASS */
void SendAPPLRequest( WORD_VAL DstAddr, WORD ClusterId, BYTE * Asdu, BYTE AsduLength)
{
    APP_DATA_request* pAppDataReq;

    pAppDataReq =  (APP_DATA_request *)SRAMalloc(126);
    if(pAppDataReq != NULL)
    {
        pAppDataReq->asduLength = AsduLength;
        pAppDataReq->DstAddress.ShortAddr.Val = DstAddr.Val;
        pAppDataReq->ClusterId.Val = ClusterId;
        pAppDataReq->DstAddrMode = APS_ADDRESS_16_BIT;
        pAppDataReq->DstEndpoint =  CUSTOMER_APP_ENDPOINT;
        pAppDataReq->ProfileId.Val = CUSTOMER_PROFILE_ID;
        pAppDataReq->SrcEndpoint = CUSTOMER_APP_ENDPOINT;
        switch (pAppDataReq->ClusterId.Val)
        {
            
            default:
                {
                   //pAppDataReq->asdu[0] = APSCounter++;
                }
                break;
        }
		
		{
			unsigned char MyTp;
			int LongPos=-1;
			
			MyCommandFlag.DataSent = TRUE;
			MyCommandFlag.ACK = ACK_PENDING;
			
					
			if(EndDeviceAnounceTable.Counter>0)
			{
				for(MyTp=0;MyTp<=EndDeviceAnounceTable.Counter;MyTp++)
				{
					//xprintf("\n\r SendZDPRequest:Rec Short Address =");
					//PrintChar(DstAddr.v[1]);
					//PrintChar(DstAddr.v[0]);
					
					//xprintf("\n\r SendZDPRequest:Act Short Address =");
					//PrintChar(EndDeviceAnounceTable.EndDevAddr[MyTp].shortaddress.v[1]);
					//PrintChar(EndDeviceAnounceTable.EndDevAddr[MyTp].shortaddress.v[0]);

					if(	DstAddr.v[0] == EndDeviceAnounceTable.EndDevAddr[MyTp].shortaddress.v[0] &&
						DstAddr.v[1] == EndDeviceAnounceTable.EndDevAddr[MyTp].shortaddress.v[1] )
					{
						
						LongPos = MyTp;
						break;

					}
				}			
			}
		
			if(LongPos!=-1)
			{
				for(MyTp=0;MyTp<8;MyTp++)
					PrvSendIeeeAddr.v[MyTp] = EndDeviceAnounceTable.EndDevAddr[LongPos].longaddress.v[MyTp];
			}
		}
		
		
        memcpy(&(pAppDataReq->asdu[0]),Asdu,AsduLength);
        pAppDataReq->TxOptions.Val = DEFAULT_TX_OPTIONS;
        pAppDataReq->TxOptions.bits.acknowledged = 0;
        pAppDataReq->RadiusCounter =  DEFAULT_RADIUS;
        if ( AILEnqueue( (BYTE *)pAppDataReq, TransmitQueueID ) == AILQueueFailure )
        {
            nfree(pAppDataReq);
        }
    }
}

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
void SetAppLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails)
{
    BYTE    index;
    BYTE    freeEntry = 0xFF;
    BYTE    entryNotFound;

    // Get ROM address of neighborTable in RAM.
    APS_KEY_PAIR_DESCRIPTOR currentKey;
    WORD pCurrentKey;
    pCurrentKey = appLinkKeyTable;


    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey+=(WORD)sizeof(APS_KEY_PAIR_DESCRIPTOR), index++)
    #else
    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey++, index++)
    #endif
    {
        // Read the record into RAM.
        GetLinkKey(&currentKey, pCurrentKey );

        // Now check to see if it is in use or not.

        if ( currentKey.InUse )
        {
            entryNotFound = memcmp ( (&pappLinkKeyDetails->PartnerAddress), &currentKey.PartnerAddress, 8 );
            if( entryNotFound == FALSE )
            {
                pappLinkKeyDetails->IncomingFrameCounter.Val = 0x00;
                PutLinkKey( pCurrentKey, pappLinkKeyDetails );
                return;
            }
        }
        else
        {
            freeEntry = index;
        }
    }
    if ( freeEntry != 0xFF )
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentKey = appLinkKeyTable + (WORD)( sizeof(APS_KEY_PAIR_DESCRIPTOR) * freeEntry);
        #else
            pCurrentKey = appLinkKeyTable + freeEntry;
        #endif
        pappLinkKeyDetails->InUse = 0x01;
        pappLinkKeyDetails->IncomingFrameCounter.Val = 0x00;
        PutLinkKey( pCurrentKey, pappLinkKeyDetails );
    }
 }


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
void RemoveAppLinkKey(APS_KEY_PAIR_DESCRIPTOR *pappLinkKeyDetails)
{
    BYTE    index;
    BYTE    entryNotFound;

    // Get ROM address of neighborTable in RAM.
    APS_KEY_PAIR_DESCRIPTOR currentKey;
    WORD pCurrentKey;
    pCurrentKey = appLinkKeyTable;


    #ifdef USE_EXTERNAL_NVM
    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey+=(WORD)sizeof(APS_KEY_PAIR_DESCRIPTOR), index++)
    #else
    for (index = 0; (index < MAX_APPLICATION_LINK_KEY_SUPPORTED) ; pCurrentKey++, index++)
    #endif
    {
        // Read the record into RAM.
        GetLinkKey(&currentKey, pCurrentKey );

        // Now check to see if it is in use or not.

        if ( currentKey.InUse )
        {
            entryNotFound = memcmp ( (&pappLinkKeyDetails->PartnerAddress), &currentKey.PartnerAddress, 8 );
            if( entryNotFound == FALSE )
            {
                pappLinkKeyDetails->InUse = 0x00;
                PutLinkKey( pCurrentKey, pappLinkKeyDetails );
                return;
            }
        }
    }
 }
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
BOOL SelectLinkKey(LONG_ADDR DstAddress, TC_LINK_KEY_TABLE *tcLinkKeyTable)
{
    if ( SearchForTCLinkKey(DstAddress, tcLinkKeyTable) )
    {
        return TRUE;
    }
    return FALSE;
 }
//#endif

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
void App_UpdateParamsForTestTool
(
    APP_DATA_indication *dataInd
)
{
    /* No Action for Sample Application*/
}

/******************************************************************************
 * Function:        static void HandleDataConfirm( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Return Value:    None
 *
 * Overview:        This function will be called after reception of data conf.
 *                  Handling of confirmation (application specific)
 *                  is left to app developer.
 *
 * Note:
 *****************************************************************************/
static void HandleDataConfirm( void )
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
void App_HandleDataIndication( void )
{
}

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
BYTE App_Set_SystemTime
(
     BYTE year,
     BYTE month,
     BYTE day,
     BYTE hour,
    BYTE minute,
    BYTE second
 )
{
    /*give the time in year format*/
    elapsedTimeInfo elapsedTime;
    elapsedTime.year = year;
    elapsedTime.month = month;
    elapsedTime.day = day;
    elapsedTime.hour = hour;
    elapsedTime.minute = minute;
    elapsedTime.second = second;
    /*Set the current time in the RTC*/
    App_SetCurrentTime( &elapsedTime );
    return 0;
}
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

//void ZCL_Callback_HandleTimeAttributes( BYTE endPointId, WORD attributeId, BYTE Length,BYTE* dataIndex)
//{
//}
