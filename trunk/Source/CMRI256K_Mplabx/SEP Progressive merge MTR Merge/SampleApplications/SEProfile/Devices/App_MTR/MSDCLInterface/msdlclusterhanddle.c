

//msdlclusterhanddle.c


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
#include "SE_Display.h"

/*Application Related Files*/
#include "App_Common.h"
#include "App_ESP.h"
#include "Menu_ESP.h"

#include "comm.h"
#include "monitor.h"

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


#include "msdlclusterhanddle.h"
#include "MSDCL_Commissioning.h"

#define SET_NETWORK_DEVICE_TYPE_ESP		0x02
#define SET_NETWORK_DEVICE_TYPE_MTR		0x01



struct __EndDeviceAnounceTable	EndDeviceAnounceTable;
struct __EndDeviceAnounceTable	PerMissionEndDeviceAnounceTable;

struct __MyCommandFlag MyCommandFlag;
struct _MACAddMeter MACAddMeter;


unsigned char AckReceived = 0;
extern unsigned char MyCurrentChannel;

//Nimish Add meter
const unsigned char AddMeterCommand[] = {0x2B,0x11,0x02,0x01,0x64,0x6C,0xFF,0xFF,0x00,0x00};
const unsigned char AddMeterCommandResponce[] = {0x2B,0x11,0x82,0x64,0x6C};

const unsigned char NumberOfAddedMetersCommand[] = {0x1B,0x05,0x01,0x01,0x64,0x6C};
const unsigned char NumberofAddedMeterResponce[] = {0x1B,0x11,0x82,0x64,0x6C};

//MyCurrentChannel;

const unsigned char MyChannelNumberCommand[] = {0x1B,0x05,0x01,0x01,0x65,0x6C};
const unsigned char MyChannelNumberResponce[] = {0x1B,0x11,0x82,0x65,0x6C};
const unsigned char MyRouteRequest[] = {0x1B,0x13,0x01,0x01,0x65,0x6D};

//Network Set Parameters Command
const unsigned char NetworkStatusCommand[] = { 0x2B, 0x05, 0x02, 0x00, 0x73, 0x63 };
const unsigned char NetworkStartCommand[] = { 0x2B, 0x05, 0x02, 0x00, 0x73, 0x74 };
const unsigned char NetworkStopCommand[] = { 0x2B, 0x05, 0x02, 0x01, 0x73, 0x74 };
const unsigned char SetNetworkExtendedPANIdCommand[] = { 0x2B, 0x0D, 0x02, 0x01, 0x65, 0x70 };
const unsigned char SetNetworkLinkKeyCommand[] = { 0x2B, 0x15, 0x02, 0x01, 0x6C, 0x6B };
const unsigned char SetNetworkDeviceTypeCommand[] = { 0x2B, 0x06, 0x02, 0x01, 0x64, 0x74 };
const unsigned char SetNetworkResetCommand[] = { 0x2B, 0x05, 0x02, 0x00, 0x65, 0x78 };
const unsigned char SetNetworkDefaultESPCommand[] = { 0x2B, 0x05, 0x02, 0x00, 0x72, 0x64 };
const unsigned char SetNetworkChannelMaskCommand[] = { 0x2B, 0x09, 0x02, 0x01, 0x63, 0x6D };


//Network Set Parameters Response
const unsigned char DeviceStartIndication[] = "MCHP_Start";
unsigned char NetworkStatusCommandResponse[] = { 0x2B, 0x06, 0x82, 0x73, 0x63, 0x00, 0x01 };
const unsigned char NetworkStartCommandResponse[] = { 0x2B, 0x05, 0x82, 0x73, 0x74, 0x00 };
const unsigned char NetworkStopCommandResponse[] = { 0x2B, 0x05, 0x02, 0x01, 0x73, 0x74, 0x00 };
const unsigned char SetNetworkExtendedPANIdCommandResponse[] = { 0x2B, 0x05, 0x82, 0x65, 0x70, 0x00 };
const unsigned char SetNetworkLinkKeyCommandResponse[] = { 0x2B, 0x05, 0x82, 0x6C, 0x6B, 0x00  };
const unsigned char SetNetworkDeviceTypeCommandResponse[] = { 0x2B, 0x05, 0x82, 0x64, 0x74, 0x00  };
const unsigned char SetNetworkResetCommandResponse[] = { 0x2B, 0x05, 0x02, 0x00, 0x65, 0x78, 0x00 };
const unsigned char SetNetworkDefaultESPCommandResponse[] = { 0x2B, 0x05, 0x82, 0x72, 0x64, 0x00 };
const unsigned char SetNetworkChannelMaskResponce[] = { 0x2B, 0x05, 0x82, 0x63, 0x6D,0x01 };

BOOL StartNetworkFlag = FALSE;

//struct MSDLValue MTRActualValues;

unsigned char waitingForDeta = 0;
int UrSeqNumner,UrCommand,Urlen,UrRW;
APP_DATA_indication* Urp_dataInd;






PANIdList ListOfExtendedPANIdinVicinity[MAX_NUMBER_OF_NETWORK_DETECT];

ExtPANIdRequestStatus RestartDeviceAfterCommission;
unsigned char Uart2Buffer[140] ="nimish";
unsigned char Uart2Datalen;
int HowManyData=0;
int NumberofSeperator = 0;
int NumberofReceived = 0; 
unsigned char numberofdatareceived = 0;

unsigned char ScanCommandSend = 0;
int TimeExitDelayForReset = 0;
unsigned char ScanResponceReceived = 0;
unsigned char AllScanCommandComplet = 0;
unsigned char TotalScanCommandSend = 0;

TICK TimeOutStartTime;
TICK ACKTimeOut;
TICK TotalMeterGetConnceted1;
TICK TotalMeterGetConnceted2;

#define TIMETOCHECKNWKTABLE			(60*(ONE_SECOND))
#define TIMETOCHECKNWKTABLECOUNT	2

ExtPANIdRequestStatus ExtPANIDStatus;

void initMSDCLTimers(void)
{
	TotalMeterGetConnceted1 = TickGet();
	MyCommandFlag.DataSent = FALSE;
	MyCommandFlag.ACK = ACK_RECEIVED;
}

void TimeOutTime(void)
{
	TICK currentTime;
	
	currentTime = TickGet();

	if( ( TickGetDiff( currentTime, TimeOutStartTime ) ) > (ONE_SECOND) *2)
	{
		ScanResponceReceived = 1;		
		TimeOutStartTime = TickGet();

	}

}


void HanddleMSDLClusterParameters(APP_DATA_indication* p_dataInd)
{
	
	int SeqNumner,Command,len;

	len =  p_dataInd->asduLength;
	Command = p_dataInd->asdu[4];
	SeqNumner = p_dataInd->asdu[3];

	Urp_dataInd = p_dataInd;

	xprintf("\n\r len = %u \n\r p_dataInd->asduLength = %u\n\r",len,p_dataInd->asduLength);
	xprintf("\n\r Command = %u \n\r p_dataInd->asd[4] = %u\n\r",Command,p_dataInd->asdu[4]);
	xprintf("\n\r SeqNumner = %u \n\r p_dataInd->asdu[3] = %u\n\r",SeqNumner,p_dataInd->asdu[3]);

	MyCommandFlag.ACK = ACK_RECEIVED;
	
	HanddleUART2Request(SeqNumner,Command,0,p_dataInd);


}

void HanddleUART2Request(int Seq,int Cmd,unsigned char RW,APP_DATA_indication* p_dataInd)
{
	int len;
	int Tp;
	int SenLen;
	unsigned char SenUart2Buffer[40];
	int LogAddrPos;
	unsigned char SendData[100];
	int sendLen;
	


	ScanResponceReceived = 1;

	len =  p_dataInd->asduLength;
	//xprintf("\n\rSeq = %u, Cmd = %u, Rw = %d",Seq,Cmd,RW);
	//xprintf("\n\rData len = %u\n\r",len);
	for(Tp=0;Tp<len;Tp++)
	{
		SendData[Tp] = p_dataInd->asdu[Tp];
		//xprintf("\n\r Tp= %d  Data = ",Tp);
		//PrintChar(SendData[Tp]); 

	}
	sendLen = Tp;
	

	LogAddrPos = -1;
	
	xprintf("\n\r Number of Deveices Connected = %u\n\r",EndDeviceAnounceTable.Counter);

	
	
	
	
	if(EndDeviceAnounceTable.Counter>0)
	{
		for(Tp=0;Tp<=EndDeviceAnounceTable.Counter;Tp++)
		{
			//xprintf("\n\r Rec Short Address =");
			//PrintChar(p_dataInd->SrcAddress.v[1]);
			//PrintChar(p_dataInd->SrcAddress.v[0]);
			
			//xprintf("\n\r Act Short Address =");
			//PrintChar(EndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[1]);
			//PrintChar(EndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[0]);

			if(	p_dataInd->SrcAddress.v[0] == EndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[0] &&
				p_dataInd->SrcAddress.v[1] == EndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[1] )
			{
				
				LogAddrPos = Tp;
				break;

			}
		}	
	
	}

	if(LogAddrPos!= -1)
	{
		SenLen = 0;
		SenUart2Buffer[0] = 0x2d;	
		SenUart2Buffer[1] = 0x01;	
		SenUart2Buffer[2] = 0x01;		
		SenUart2Buffer[3] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].longaddress.v[7];
		SenUart2Buffer[4] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].longaddress.v[6];
		SenUart2Buffer[5] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].longaddress.v[5];
		SenUart2Buffer[6] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].longaddress.v[4];
		SenUart2Buffer[7] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].longaddress.v[3];
		SenUart2Buffer[8] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].longaddress.v[2];
		SenUart2Buffer[9] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].longaddress.v[1];
		SenUart2Buffer[10] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].longaddress.v[0];

		SenUart2Buffer[11] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].shortaddress.v[0];
		SenUart2Buffer[12] = EndDeviceAnounceTable.EndDevAddr[LogAddrPos].shortaddress.v[1];
	
		SenUart2Buffer[13] = p_dataInd->ClusterId.v[1];
		SenUart2Buffer[14] = p_dataInd->ClusterId.v[0];

		for(Tp=0,SenLen=15;Tp<sendLen;Tp++,SenLen++)
		{
			SenUart2Buffer[SenLen] =SendData[Tp];
		}
		SenUart2Buffer[1] = SenLen-1;
		
		xprintf("\n\r I Send Data = ");
		for(Tp=0;Tp<SenLen;Tp++)
		{
			PrintChar(SenUart2Buffer[Tp]); 
			ConsolePut(' '); 
			x2putc(SenUart2Buffer[Tp]); 
		}

	}


}

LONG_ADDR	PrvSendIeeeAddr;

void CompareNWKTable(void)
{
	unsigned char DisplayCounter = 0;
	LONG_ADDR VlongAddr;
	LONG_ADDR LongAddr;
	SHORT_ADDR ShortAddr;
	
	int myTp;
	for(myTp=0;myTp<8;myTp++)
	{
		VlongAddr.v[myTp] = 0;
	}
	
	int index;
		#ifdef USE_EXTERNAL_NVM
			for (index=0, pCurrentNeighborRecord = neighborTable; index < MAX_NEIGHBORS; index++, pCurrentNeighborRecord += (WORD)sizeof(NEIGHBOR_RECORD))
		#else
			for (index=0, pCurrentNeighborRecord = neighborTable; index < MAX_NEIGHBORS; index++, pCurrentNeighborRecord++)
		#endif // #ifdef USE_EXTERNAL_NVM
			{
				GetNeighborRecord( &currentNeighborRecord, pCurrentNeighborRecord );
				if ( memcmp((void*)&VlongAddr,
                        (void*)&currentNeighborRecord.longAddr,
                        (BYTE)(sizeof(LONG_ADDR))) )
				{
					
					
					ShortAddr.Val = currentNeighborRecord.shortAddr.Val;
					for(myTp=0;myTp<8;myTp++)
					{
						LongAddr.v[myTp] = currentNeighborRecord.longAddr.v[myTp];
					}
					
					if(EndDeviceLookupNodeByLongAddr(LongAddr) == -1)
					{
						//ConsolePutROMString( (ROM char *)"\r\nTotal Device Counter No= " );
						//PrintChar(DisplayCounter);
						//ConsolePutROMString( (ROM char *)"Ieee Addr= " );
						//int myTp;
						//for(myTp=0;myTp<8;myTp++)
						//{
						//	PrintChar(LongAddr.v[myTp]);
						//	ConsolePutROMString( (ROM char *)":");
						//}
						//ConsolePutROMString( (ROM char *)"\r\n");
						//DisplayCounter++;
						//ConsolePutROMString( (ROM char *)"\r\n Need to add in Table");
						AddEndDeviceAnnceDevice(ShortAddr,LongAddr);
					}
						
					//ConsolePutROMString( (ROM char *)"\r\nTotal Device Counter No= " );
					//PrintChar(DisplayCounter);
					//ConsolePutROMString( (ROM char *)"Ieee Addr= " );
					//int myTp;
					//for(myTp=0;myTp<8;myTp++)
					//{
					//	PrintChar(LongAddr.v[myTp]);
					//	ConsolePutROMString( (ROM char *)":");
					//}
					//ConsolePutROMString( (ROM char *)"\r\n");
					//DisplayCounter++;
				}
			}
	
	
	/*int index;
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
					DisplayCounter++;
					ConsolePutROMString( (ROM char *)"\r\nTotal Device Counter No= " );
		            PrintChar(DisplayCounter);
					ConsolePutROMString( (ROM char *)"Ieee Addr= " );
					int myTp;
					for(myTp=0;myTp<8;myTp++)
					{
						PrintChar(currentNeighborRecord.longAddr.v[myTp]);
						ConsolePutROMString( (ROM char *)":");
					}
					ConsolePutROMString( (ROM char *)"\r\n");
					
					if(EndDeviceLookupNodeByLongAddr(&currentNeighborRecord.longAddr) == -1)
					{
						//printf("Time Interval reached so adding all NWK Data");
						AddEndDeviceAnnceDevice(currentNeighborRecord.shortAddr,currentNeighborRecord.longAddr);
					}
				}
			}
			ConsolePutROMString( (ROM char *)"\r\nTotal Device Counter Rached= " );
            PrintChar(DisplayCounter);
			ConsolePutROMString( (ROM char *)"\r\n");*/
}


void HanddleUART2(void)
{

	unsigned char c;
	int Tp;	
	
	TICK MeterConcurrentTime;
	
	MeterConcurrentTime = TickGet();
	if( ( TickGetDiff( MeterConcurrentTime, TotalMeterGetConnceted1 ) ) > TIMETOCHECKNWKTABLE)
	{
		CompareNWKTable();
		TotalMeterGetConnceted1 = TickGet();
	}
	
	if(MyCommandFlag.ACK != ACK_PENDING)
	{
		if(MyCommandFlag.DataSent == TRUE)
		{
			if(MyCommandFlag.ACK == ACK_NOTRECEIVED)
			{
				//xprintf("ACK not Received :(\n\r");
				
				MyAskForDeviceAddress(FALSE,PrvSendIeeeAddr);
			}
			else if(MyCommandFlag.ACK == ACK_RECEIVED)
			{
				//xprintf("ACK Received :)\n\r");
			}
			else
			{
				xprintf("This staage I am not Expected\n\t");
			}
			MyCommandFlag.DataSent = FALSE;
		}
		else
		{
			ACKTimeOut = TickGet();
		}
	}
	else
	{
		
		TICK currentTime;
	
		currentTime = TickGet();

		if( ( TickGetDiff( currentTime, ACKTimeOut ) ) > (ONE_SECOND) *5)
		{
			MyCommandFlag.ACK = ACK_NOTRECEIVED;
			ACKTimeOut = TickGet();
		}
	}

	if(uart_kbhit() != 0)
	{
		c = uart_get();
		//x2putc(c);
		Uart2Buffer[NumberofReceived] = c;
		if(Uart2Buffer[0] == 0x2B || Uart2Buffer[0] == 0x1B)
		{
			NumberofReceived++;
		}
		else
		{
			NumberofReceived = 0;
		}
		if(NumberofReceived >=2 && numberofdatareceived == 0)
		{
			HowManyData = c;
			numberofdatareceived = 1;
		}
		if(HowManyData !=0 && NumberofReceived == HowManyData+1)
		{
			//xprintf("\n\r Data Received \n\r");
			//for(Tp=0;Tp<=HowManyData;Tp++)
			//	xprintf("\n\r Data is: %d\n\r",Uart2Buffer[Tp]);
			//xprintf("\n\r How many data = %d",HowManyData);
			Uart2Datalen = HowManyData;
			NumberofReceived = 0;
			numberofdatareceived = 0;
			HowManyData = 0;
			while(uart_kbhit() != 0)
				c = uart_get();
			CheckDataAndSend();
		}
	}

}


extern ZIGBEE_PRIMITIVE currentPrimitive;;


void CheckDataAndSend(void)
{
	LONG_ADDR IeeAddress;
	int Tp;

	WORD_VAL ZDODstAddr;
	BYTE addr1[2];
	BYTE i = 0;

	int LogAddrPos;
	unsigned int SenLen;
	WORD_VAL MSDCL_ClusterID;
	
	unsigned char SenUart2Buffer[800];
	
	//Check for start Network command from HHU
	if( memcmp( (void*)Uart2Buffer, (void*)NetworkStartCommand, sizeof(NetworkStartCommand) ) == 0 )
	{
		StartNetworkFlag = TRUE;	//This will trigger the start of network from the main function. Till this function is sent, it will wait
		SendModuleNetworkSetCommandResponse( (const unsigned char*) NetworkStartCommandResponse, sizeof(NetworkStartCommandResponse) );
	}
	else if( memcmp( (void*)Uart2Buffer, (void*)NetworkStatusCommand, sizeof(NetworkStatusCommand) ) == 0 )
	{
		NetworkStatusCommandResponse[sizeof(NetworkStatusCommandResponse) - 1] = StartNetworkFlag;	//StartNetworkFlag;	//This will trigger the start of network from the main function. Till this function is sent, it will wait
		SendModuleNetworkSetCommandResponse( (unsigned char*) NetworkStatusCommandResponse, sizeof(NetworkStatusCommandResponse) );
	}
	else if( memcmp( (void*)Uart2Buffer, (void*)NetworkStopCommand, sizeof(NetworkStopCommand) ) == 0 )
	{
		SendModuleNetworkSetCommandResponse( (const unsigned char*) NetworkStopCommandResponse, sizeof(NetworkStopCommandResponse) );
		DelayMs(1000);
		Reset();		//Simple reset the device on reception of this command
	}
	else if( memcmp( (void*)Uart2Buffer, (void*)SetNetworkExtendedPANIdCommand, sizeof(SetNetworkExtendedPANIdCommand) ) == 0 )
	{
		NVMRead ( (BYTE*)&MSDCL_Commission, MSDCL_Commission_Locations, sizeof(MSDCL_Commission));
		//Set the Network Extended PAN Id sent from the HHU
		memcpy( (void*)MSDCL_Commission.ExtendedPANId, (void*)&Uart2Buffer[sizeof(SetNetworkExtendedPANIdCommand)], sizeof(MSDCL_Commission.ExtendedPANId) );
		NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
		SendModuleNetworkSetCommandResponse( (const unsigned char*) SetNetworkExtendedPANIdCommandResponse, sizeof(SetNetworkExtendedPANIdCommandResponse) );
	}
	else if( memcmp( (void*)Uart2Buffer, (void*)SetNetworkLinkKeyCommand, sizeof(SetNetworkLinkKeyCommand) ) == 0 )
	{
		NVMRead ( (BYTE*)&MSDCL_Commission, MSDCL_Commission_Locations, sizeof(MSDCL_Commission));
		//Set the Network Link Key sent from the HHU
		memcpy( (void*)MSDCL_Commission.LinkKey, (void*)&Uart2Buffer[sizeof(SetNetworkLinkKeyCommand)], sizeof(MSDCL_Commission.LinkKey) );
		NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
		SendModuleNetworkSetCommandResponse( (const unsigned char*) SetNetworkLinkKeyCommandResponse, sizeof(SetNetworkLinkKeyCommandResponse) );
	}
	else if( memcmp( (void*)Uart2Buffer, (void*)SetNetworkDeviceTypeCommand, sizeof(SetNetworkDeviceTypeCommand) ) == 0 )
	{
		NVMRead ( (BYTE*)&MSDCL_Commission, MSDCL_Commission_Locations, sizeof(MSDCL_Commission));
		//Set the Network StartUp Status sent from the HHU
		if( Uart2Buffer[sizeof(SetNetworkDeviceTypeCommand)] == SET_NETWORK_DEVICE_TYPE_ESP )
		{
			MSDCL_Commission.StartupStatus = STARTUP_CONTROL_FORM_NEW_NETWORK;
		}
		else if( Uart2Buffer[sizeof(SetNetworkDeviceTypeCommand)] == SET_NETWORK_DEVICE_TYPE_MTR )
		{
			MSDCL_Commission.StartupStatus = STARTUP_CONTROL_JOIN_NEW_NETWORK;
		}
		NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
		SendModuleNetworkSetCommandResponse( (const unsigned char*) SetNetworkDeviceTypeCommandResponse, sizeof(SetNetworkDeviceTypeCommandResponse) );
	}
	else if( memcmp( (void*)Uart2Buffer, (void*)SetNetworkResetCommand, sizeof(SetNetworkResetCommand) ) == 0 )
	{
		NVMRead ( (BYTE*)&MSDCL_Commission, MSDCL_Commission_Locations, sizeof(MSDCL_Commission));
		//Set the Network Reset sent from the HHU
		MSDCL_Commission.ChannelMask.Val = ALLOWED_CHANNELS;	//0b00000000000000001000000000000000;	//Channel 15 as default
		MSDCL_Commission.ValidCleanStartUp = MSDCL_COMMISSION_DATA_VALID;
		NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
		SendModuleNetworkSetCommandResponse( (const unsigned char*) SetNetworkResetCommandResponse, sizeof(SetNetworkResetCommandResponse) );
		DelayMs(1000);
		Reset();
	}
	else if( memcmp( (void*)Uart2Buffer, (void*)SetNetworkChannelMaskCommand, sizeof(SetNetworkChannelMaskCommand) ) == 0 )
	{
		SendModuleNetworkSetCommandResponse( (const unsigned char*) SetNetworkChannelMaskResponce, sizeof(SetNetworkChannelMaskResponce) );
		
	}
	else if( memcmp( (void*)Uart2Buffer, (void*)SetNetworkDefaultESPCommand, sizeof(SetNetworkDefaultESPCommand) ) == 0 )
	{
		NVMRead ( (BYTE*)&MSDCL_Commission, MSDCL_Commission_Locations, sizeof(MSDCL_Commission));
		//Set the Network Extended Pan Id Status sent from the HHU
		MSDCL_Commission.ExtendedPANId[0] = NWK_EXTENDED_PAN_ID_BYTE0;
		MSDCL_Commission.ExtendedPANId[1] = NWK_EXTENDED_PAN_ID_BYTE1;
		MSDCL_Commission.ExtendedPANId[2] = NWK_EXTENDED_PAN_ID_BYTE2;
		MSDCL_Commission.ExtendedPANId[3] = NWK_EXTENDED_PAN_ID_BYTE3;
		MSDCL_Commission.ExtendedPANId[4] = NWK_EXTENDED_PAN_ID_BYTE4;
		MSDCL_Commission.ExtendedPANId[5] = NWK_EXTENDED_PAN_ID_BYTE5;
		MSDCL_Commission.ExtendedPANId[6] = NWK_EXTENDED_PAN_ID_BYTE6;
		MSDCL_Commission.ExtendedPANId[7] = NWK_EXTENDED_PAN_ID_BYTE7;
		
		//Set Channel Mask
		MSDCL_Commission.ChannelMask.Val = ALLOWED_CHANNELS_PRE_CONFIG;// ALLOWED_CHANNELS;
		
		//Set Link Key
		MSDCL_Commission.LinkKey[0] = PRECONFIGURED_LINK_KEY00;
		MSDCL_Commission.LinkKey[1] = PRECONFIGURED_LINK_KEY01;
		MSDCL_Commission.LinkKey[2] = PRECONFIGURED_LINK_KEY02;
		MSDCL_Commission.LinkKey[3] = PRECONFIGURED_LINK_KEY03;
		MSDCL_Commission.LinkKey[4] = PRECONFIGURED_LINK_KEY04;
		MSDCL_Commission.LinkKey[5] = PRECONFIGURED_LINK_KEY05;
		MSDCL_Commission.LinkKey[6] = PRECONFIGURED_LINK_KEY06;
		MSDCL_Commission.LinkKey[7] = PRECONFIGURED_LINK_KEY07;
		MSDCL_Commission.LinkKey[8] = PRECONFIGURED_LINK_KEY08;
		MSDCL_Commission.LinkKey[9] = PRECONFIGURED_LINK_KEY09;
		MSDCL_Commission.LinkKey[10] = PRECONFIGURED_LINK_KEY10;
		MSDCL_Commission.LinkKey[11] = PRECONFIGURED_LINK_KEY11;
		MSDCL_Commission.LinkKey[12] = PRECONFIGURED_LINK_KEY12;
		MSDCL_Commission.LinkKey[13] = PRECONFIGURED_LINK_KEY13;
		MSDCL_Commission.LinkKey[14] = PRECONFIGURED_LINK_KEY14;
		MSDCL_Commission.LinkKey[15] = PRECONFIGURED_LINK_KEY15;
		
		//Set Startup Status
		MSDCL_Commission.StartupStatus = STARTUP_CONTROL_FORM_NEW_NETWORK;
		
		MSDCL_Commission.ValidCleanStartUp = MSDCL_COMMISSION_DATA_VALID;
		
		NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
		SendModuleNetworkSetCommandResponse( (const unsigned char*) SetNetworkDefaultESPCommandResponse, sizeof(SetNetworkDefaultESPCommandResponse) );
	}
	else if(memcmp( (void*)Uart2Buffer, (void*)NumberOfAddedMetersCommand, sizeof(NumberOfAddedMetersCommand) ) == 0 )
	{
		FindNumberofAddedMetersResponce();
	}	
	else if(memcmp( (void*)Uart2Buffer, (void*)MyRouteRequest, sizeof(MyRouteRequest) ) == 0 )
	{
		
	}
	else if(memcmp( (void*)Uart2Buffer, (void*)MyChannelNumberCommand, sizeof(MyChannelNumberCommand) ) == 0 )
	{
		FindMyChannel();
	}
	else if(memcmp( (void*)Uart2Buffer, (void*)AddMeterCommand, sizeof(AddMeterCommand) ) == 0 )
	{
		unsigned char Tp2 = 0;
		char TpStr3[20];
		unsigned char success =0;
		if(MACAddMeter.NumberMeter<MAX_NEIGHBORS)
		{
			success = 1;
			Tp2 = 10;
			for(Tp=0;Tp<=7;Tp++)
			{				
					MACAddMeter.AddMeter[MACAddMeter.NumberMeter].v[Tp] = Uart2Buffer[Tp2];
					Tp2++;
			}
			MACAddMeter.NumberMeter++;
		}
		SendAddMeterCommandResponce(success);
	}
	else if( (Uart2Buffer[2] == 0x02) && (Uart2Buffer[4] == 0x72) )
	{

		SenLen = 0;

		SenUart2Buffer[SenLen++] = 0x2D;	
		SenUart2Buffer[SenLen++] = 0x01;	
		SenUart2Buffer[SenLen++] = 0x82;	
		SenUart2Buffer[SenLen++] = 0x72;	
		SenUart2Buffer[SenLen++] = 0x74;	
		SenUart2Buffer[SenLen++] = 0x00;	
		SenUart2Buffer[SenLen++] = EndDeviceAnounceTable.Counter;	

		if(EndDeviceAnounceTable.Counter>0)
		{
			for(Tp=0;Tp<EndDeviceAnounceTable.Counter;Tp++)
			{
				int Tp2;
				for(Tp2=7;Tp2>=0;Tp2--)
				{
					SenUart2Buffer[SenLen++] = EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[Tp2];
				}
			
				SenUart2Buffer[SenLen++] = EndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[1];
				SenUart2Buffer[SenLen++] = EndDeviceAnounceTable.EndDevAddr[Tp].shortaddress.v[0];
			}
		}
	
		
		SenUart2Buffer[1] = SenLen-1;
		
		//xprintf("\n\r I Send Data = ");
		for(Tp=0;Tp<SenLen;Tp++)
		{
			//PrintChar(SenUart2Buffer[Tp]); 
			//ConsolePut(' '); 
			x2putc(SenUart2Buffer[Tp]);
			//Delay10us(1);
		}
		
	}
	else if( (Uart2Buffer[2] == 0x02) && (Uart2Buffer[4] == 0x73) )
	{
		//AckReceived = 1;
		SendExtPANIdResponse();
	}
	else if((Uart2Buffer[2] == 0x04))
	{
	
		i = 0;
		{
			unsigned char cntr;
			
			for( cntr = 15; cntr <=Uart2Datalen; cntr++ )
			{
				asduData[i++] = Uart2Buffer[cntr];
				PrintChar(Uart2Buffer[cntr]); 
				ConsolePut(' '); 
			}
			MSDCL_ClusterID.byte.HB = Uart2Buffer[13];
			MSDCL_ClusterID.byte.LB = Uart2Buffer[14];
		}
		ZDODstAddr.Val =  0xFFFD; 
			
		//xprintf("\n\raddr1[0] = %02x \n\r",addr1[0]);
		//xprintf("\n\raddr1[1] = %02x \n\r",addr1[1]);
		//xprintf("\n\rZDODstAddr.Val = %04X \n\r",ZDODstAddr.Val);
		
		SendAPPLRequest( ZDODstAddr, MSDCL_ClusterID.Val , asduData, i);
	
/*		for(Tp=0;Tp<NumberOfDevicesConnected;Tp++)
		{
			i = 0;
			{
				unsigned char cntr;
				
				for( cntr = 15; cntr <=Uart2Datalen; cntr++ )
				{
					asduData[i++] = Uart2Buffer[cntr];
					PrintChar(Uart2Buffer[cntr]); 
					ConsolePut(' '); 
				}
				MSDCL_ClusterID.byte.HB = Uart2Buffer[13];
				MSDCL_ClusterID.byte.LB = Uart2Buffer[14];
			}
		
			addr1[0] = CurrentDeviceConnectedInfo[Tp].shortAddr.v[0];
			addr1[1] = CurrentDeviceConnectedInfo[Tp].shortAddr.v[1];  
			
			ZDODstAddr.Val =  ( (addr1[1] << 8) | addr1[0] ); 
			
			//xprintf("\n\raddr1[0] = %02x \n\r",addr1[0]);
			//xprintf("\n\raddr1[1] = %02x \n\r",addr1[1]);
			//xprintf("\n\rZDODstAddr.Val = %04X \n\r",ZDODstAddr.Val);
			SendAPPLRequest( ZDODstAddr, MSDCL_ClusterID.Val , asduData, i);
			
			SendModuleNetworkSetCommandResponse( (const unsigned char*) SetNetworkResetCommandResponse, sizeof(SetNetworkResetCommandResponse) );
			
		}*/
	}
	else
	{

		IeeAddress.v[7] = Uart2Buffer[10];
		IeeAddress.v[6] = Uart2Buffer[9];
		IeeAddress.v[5] = Uart2Buffer[8];
		IeeAddress.v[4] = Uart2Buffer[7];
		IeeAddress.v[3] = Uart2Buffer[6];
		IeeAddress.v[2] = Uart2Buffer[5];
		IeeAddress.v[1] = Uart2Buffer[4];
		IeeAddress.v[0] = Uart2Buffer[3];
	
		//xprintf("\n\r IeeAddress Received =");
		//PrintChar(IeeAddress.v[7]);
		//PrintChar(IeeAddress.v[6]);
		//PrintChar(IeeAddress.v[5]);
		//PrintChar(IeeAddress.v[4]);
		//PrintChar(IeeAddress.v[3]);
		//PrintChar(IeeAddress.v[2]);
		//PrintChar(IeeAddress.v[1]);
		//PrintChar(IeeAddress.v[0]);
	
		//xprintf("\n\rNow Going to send Data = ");
	
		{
			unsigned char cntr;
			
			for( cntr = 15; cntr <=Uart2Datalen; cntr++ )
			{
				asduData[i++] = Uart2Buffer[cntr];
				//PrintChar(Uart2Buffer[cntr]); 
				//ConsolePut(' '); 
			}
			MSDCL_ClusterID.byte.HB = Uart2Buffer[13];
			MSDCL_ClusterID.byte.LB = Uart2Buffer[14];
		}
		

		{
			LogAddrPos = -1;
			if(EndDeviceAnounceTable.Counter>0)
			{
				for(Tp=0;Tp<EndDeviceAnounceTable.Counter;Tp++)
				{
					//xprintf("\n\r IeeAddress Received =");
					//PrintChar(IeeAddress.v[7]);
					//PrintChar(IeeAddress.v[6]);
					//PrintChar(IeeAddress.v[5]);
					//PrintChar(IeeAddress.v[4]);
					//PrintChar(IeeAddress.v[3]);
					//PrintChar(IeeAddress.v[2]);
					//PrintChar(IeeAddress.v[1]);
					//PrintChar(IeeAddress.v[0]);
	
					//xprintf("\n\r Cur Device Log Addr =");
					//PrintChar(CurrentDeviceConnectedInfo[Tp].longAddr.v[7]);
					//PrintChar(CurrentDeviceConnectedInfo[Tp].longAddr.v[6]);
					//PrintChar(CurrentDeviceConnectedInfo[Tp].longAddr.v[5]);
					//PrintChar(CurrentDeviceConnectedInfo[Tp].longAddr.v[4]);
					//PrintChar(CurrentDeviceConnectedInfo[Tp].longAddr.v[3]);
					//PrintChar(CurrentDeviceConnectedInfo[Tp].longAddr.v[2]);
					//PrintChar(CurrentDeviceConnectedInfo[Tp].longAddr.v[1]);
					//PrintChar(CurrentDeviceConnectedInfo[Tp].longAddr.v[0]);
	
					if(	IeeAddress.v[7] == EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[7] &&
						IeeAddress.v[6] == EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[6] &&
						IeeAddress.v[5] == EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[5] &&
						IeeAddress.v[4] == EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[4] &&
						IeeAddress.v[3] == EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[3] &&
						IeeAddress.v[2] == EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[2] &&
						IeeAddress.v[1] == EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[1] &&
						IeeAddress.v[0] == EndDeviceAnounceTable.EndDevAddr[Tp].longaddress.v[0])
					{
						LogAddrPos = Tp;
						break;
		
					}
				}	
		
			}
	
			//xprintf("\n\r LogAddrPos = %u\n\r",LogAddrPos);
			if(LogAddrPos != -1)
			{
				addr1[0] =  EndDeviceAnounceTable.EndDevAddr[LogAddrPos].shortaddress.v[0];
				addr1[1] =  EndDeviceAnounceTable.EndDevAddr[LogAddrPos].shortaddress.v[1];  
				
				ZDODstAddr.Val =  ( (addr1[1] << 8) | addr1[0] ); 
				
				xprintf("\n\raddr1[0] = %02x \n\r",addr1[0]);
				xprintf("\n\raddr1[1] = %02x \n\r",addr1[1]);
				xprintf("\n\rZDODstAddr.Val = %04X \n\r",ZDODstAddr.Val);
				SendAPPLRequest( ZDODstAddr, MSDCL_ClusterID.Val , asduData, i);
			}
			else
			{
				MyAskForDeviceAddress(FALSE,IeeAddress);
			}
		}
	}	

}

void SendExtPANIdResponse(void)
{
	int Tp;
	int SenLen;
	
	unsigned char SenUart2Buffer[200];

	{

		SenLen = 0;
		SenUart2Buffer[SenLen] = 0x2D;	
		SenLen++;
		SenUart2Buffer[SenLen] = 0x14;	//update the packet length later
		SenLen++;	
		SenUart2Buffer[SenLen] = 0x82;	
		SenLen++;	
		SenUart2Buffer[SenLen] = 0x73;	
		SenLen++;	
		SenUart2Buffer[SenLen] = 0x70;	
		SenLen++;	
		SenUart2Buffer[SenLen] = 0x00;	
		SenLen++;	
		SenUart2Buffer[SenLen] = 0x01;	//update the number of PANId discovered later
		SenLen++;	
		{
			unsigned char validPanId = 0;
			for(Tp=0;Tp<MAX_NUMBER_OF_NETWORK_DETECT;Tp++)
			{
				if( ListOfExtendedPANIdinVicinity[Tp].ValidData == TRUE )
				{
					validPanId++;
					SenUart2Buffer[SenLen] = ListOfExtendedPANIdinVicinity[Tp].ExtPANId.v[0];	
					SenLen++;	
					SenUart2Buffer[SenLen] = ListOfExtendedPANIdinVicinity[Tp].ExtPANId.v[1];
					SenLen++;	
					SenUart2Buffer[SenLen] = ListOfExtendedPANIdinVicinity[Tp].ExtPANId.v[2];
					SenLen++;	
					SenUart2Buffer[SenLen] = ListOfExtendedPANIdinVicinity[Tp].ExtPANId.v[3];
					SenLen++;	
					SenUart2Buffer[SenLen] = ListOfExtendedPANIdinVicinity[Tp].ExtPANId.v[4];
					SenLen++;	
					SenUart2Buffer[SenLen] = ListOfExtendedPANIdinVicinity[Tp].ExtPANId.v[5];
					SenLen++;	
					SenUart2Buffer[SenLen] = ListOfExtendedPANIdinVicinity[Tp].ExtPANId.v[6];
					SenLen++;	
					SenUart2Buffer[SenLen] = ListOfExtendedPANIdinVicinity[Tp].ExtPANId.v[7];
					SenLen++;	
				}
			}
			SenUart2Buffer[6] = validPanId;
		}	
		
		SenUart2Buffer[1] = SenLen-1;
		
		//xprintf("\n\r I Send Data = ");
		for(Tp=0;Tp<SenLen;Tp++)
		{
			//PrintChar(SenUart2Buffer[Tp]); 
			//ConsolePut(' '); 
			x2putc(SenUart2Buffer[Tp]); 
		}
	}
}

void SendModuleStartData(void)
{
		int Tp;
		unsigned char SenUart2Buffer[20];

		memcpy( (void*)SenUart2Buffer, (void*)DeviceStartIndication, sizeof(DeviceStartIndication) );
		
		//xprintf("\n\r I Send Data = ");
		for(Tp=0;Tp<sizeof(DeviceStartIndication);Tp++)
		{
			//PrintChar(SenUart2Buffer[Tp]); 
			//ConsolePut(' '); 
			x2putc(SenUart2Buffer[Tp]); 
		}
}

void SendModuleNetworkSetCommandResponse( const unsigned char* responseCommand, unsigned char dataSize )
{
		unsigned char Tp;
		unsigned char SenUart2Buffer[20];

		memcpy( (void*)SenUart2Buffer, (void*)responseCommand, dataSize );
		
		//xprintf("\n\r I Send Data = ");
		for(Tp=0;Tp<dataSize;Tp++)
		{
			//PrintChar(SenUart2Buffer[Tp]); 
			//ConsolePut(' '); 
			x2putc(SenUart2Buffer[Tp]); 
		}
}

void SendAddMeterCommandResponce(unsigned char success)
{
	unsigned char Tp;
	unsigned char SenUart2Buffer[20];
	unsigned char dataSize;

	memcpy( (void*)SenUart2Buffer, (void*)AddMeterCommandResponce, sizeof(AddMeterCommandResponce));
	dataSize = sizeof(AddMeterCommandResponce);
	if(success == 1)
	{
		SenUart2Buffer[dataSize++] = 0x00;
		SenUart2Buffer[dataSize++] = MACAddMeter.NumberMeter;
	}
	else
		SenUart2Buffer[dataSize++] = 0x01;
	SenUart2Buffer[1] = dataSize-1;
	//xprintf("\n\r I Send Data = ");
		DelayMs(100);
	for(Tp=0;Tp<dataSize;Tp++)
	{
		PrintChar(SenUart2Buffer[Tp]); 
		ConsolePut(' '); 
		x2putc(SenUart2Buffer[Tp]); 
	}
}
void FindNumberofAddedMetersResponce(void)
{
	unsigned char Tp,Tp2;
	unsigned char SenUart2Buffer[260];
	unsigned char dataSize;

	memcpy( (void*)SenUart2Buffer, (void*)NumberofAddedMeterResponce, sizeof(NumberofAddedMeterResponce));
	dataSize = sizeof(NumberofAddedMeterResponce);
	SenUart2Buffer[dataSize++] = 0x00;
	SenUart2Buffer[dataSize++] = MACAddMeter.NumberMeter;
	for(Tp=0;Tp<MACAddMeter.NumberMeter;Tp++)
	{
		for(Tp2=0;Tp2<=7;Tp2++)
		{
			SenUart2Buffer[dataSize++] = MACAddMeter.AddMeter[Tp].v[Tp2];
		}
	}
	SenUart2Buffer[1] = dataSize-1;
	//xprintf("\n\r I Send Data = ");
		DelayMs(100);
	for(Tp=0;Tp<dataSize;Tp++)
	{
		//PrintChar(SenUart2Buffer[Tp]); 
		//ConsolePut(' '); 
		x2putc(SenUart2Buffer[Tp]); 
	}
	
}


void FindMyChannel(void)
{
	unsigned char Tp,Tp2;
	unsigned char SenUart2Buffer[260];
	unsigned char dataSize;

	memcpy( (void*)SenUart2Buffer, (void*)MyChannelNumberResponce, sizeof(MyChannelNumberResponce));
	dataSize = sizeof(MyChannelNumberResponce);
	SenUart2Buffer[dataSize++] = 0x00;
	SenUart2Buffer[dataSize++] = MyCurrentChannel;
	SenUart2Buffer[dataSize++] = 0xff;
	SenUart2Buffer[dataSize++] = 0xff;
	SenUart2Buffer[dataSize++] = AckReceived;
	SenUart2Buffer[dataSize++] = 0xff;
	SenUart2Buffer[dataSize++] = 0xff;
		
	SenUart2Buffer[1] = dataSize-1;
	//xprintf("\n\r I Send Data = ");
		DelayMs(100);
	for(Tp=0;Tp<dataSize;Tp++)
	{
		//PrintChar(SenUart2Buffer[Tp]); 
		//ConsolePut(' '); 
		x2putc(SenUart2Buffer[Tp]); 
	}
}


//this is call back function for NWK layer checing for Permission
BOOL CheckDeviceJoiningPermission(LONG_ADDR ieeeAddr)
{

	int Tp;
	printf("Checking For Device Permission\n\r");
	if(MACAddMeter.NumberMeter != 0)
	{
		for(Tp=0;Tp<MACAddMeter.NumberMeter;Tp++)
		{
			if(	ieeeAddr.v[0] == MACAddMeter.AddMeter[Tp].v[0] &&
				ieeeAddr.v[1] == MACAddMeter.AddMeter[Tp].v[1] &&
				ieeeAddr.v[2] == MACAddMeter.AddMeter[Tp].v[2] &&
				ieeeAddr.v[3] == MACAddMeter.AddMeter[Tp].v[3] &&
				ieeeAddr.v[4] == MACAddMeter.AddMeter[Tp].v[4] &&
				ieeeAddr.v[5] == MACAddMeter.AddMeter[Tp].v[5] &&
				ieeeAddr.v[6] == MACAddMeter.AddMeter[Tp].v[6] &&
				ieeeAddr.v[7] == MACAddMeter.AddMeter[Tp].v[7])
			{
				return TRUE;
				
			}
		}
	}
	else if(MACAddMeter.NumberMeter == 0)
	{
            int i;
            BOOL FoundDevice = FALSE;
            if(PerMissionEndDeviceAnounceTable.Counter>0)
            {

                for(i=0;i<PerMissionEndDeviceAnounceTable.Counter;i++)
                {
                    if( ieeeAddr.v[7] == PerMissionEndDeviceAnounceTable.EndDevAddr[i].longaddress.v[7] &&
                    ieeeAddr.v[6] == PerMissionEndDeviceAnounceTable.EndDevAddr[i].longaddress.v[6] &&
                    ieeeAddr.v[5] == PerMissionEndDeviceAnounceTable.EndDevAddr[i].longaddress.v[5] &&
                    ieeeAddr.v[4] == PerMissionEndDeviceAnounceTable.EndDevAddr[i].longaddress.v[4] &&
                    ieeeAddr.v[3] == PerMissionEndDeviceAnounceTable.EndDevAddr[i].longaddress.v[3] &&
                    ieeeAddr.v[2] == PerMissionEndDeviceAnounceTable.EndDevAddr[i].longaddress.v[2] &&
                    ieeeAddr.v[1] == PerMissionEndDeviceAnounceTable.EndDevAddr[i].longaddress.v[1] &&
                    ieeeAddr.v[0] == PerMissionEndDeviceAnounceTable.EndDevAddr[i].longaddress.v[0])
                    {
                        FoundDevice = TRUE;
                    }
                }

            }
            if(FoundDevice == FALSE)
            {
                for(Tp=0;Tp<8;Tp++)
                {
                    PerMissionEndDeviceAnounceTable.EndDevAddr[PerMissionEndDeviceAnounceTable.Counter].longaddress.v[Tp] = ieeeAddr.v[Tp];
                }
                PerMissionEndDeviceAnounceTable.Counter++;
            }

            if(PerMissionEndDeviceAnounceTable.Counter<=MAX_METER_TO_JOIN)
                return TRUE;
            else
                return FALSE;
	}
	return FALSE;
}


int EndDeviceLookupNodeByLongAddr( LONG_ADDR longAddr )
{
    BYTE i;

    for ( i=0; i<=EndDeviceAnounceTable.Counter; i++)
    {
        
        if( longAddr.v[7] == EndDeviceAnounceTable.EndDevAddr[i].longaddress.v[7] &&
            longAddr.v[6] == EndDeviceAnounceTable.EndDevAddr[i].longaddress.v[6] &&
            longAddr.v[5] == EndDeviceAnounceTable.EndDevAddr[i].longaddress.v[5] &&
            longAddr.v[4] == EndDeviceAnounceTable.EndDevAddr[i].longaddress.v[4] &&
            longAddr.v[3] == EndDeviceAnounceTable.EndDevAddr[i].longaddress.v[3] &&
            longAddr.v[2] == EndDeviceAnounceTable.EndDevAddr[i].longaddress.v[2] &&
            longAddr.v[1] == EndDeviceAnounceTable.EndDevAddr[i].longaddress.v[1] &&
            longAddr.v[0] == EndDeviceAnounceTable.EndDevAddr[i].longaddress.v[0])
        {
            return i;
        }
    }

    return -1;
}


void AddEndDeviceAnnceDevice(SHORT_ADDR shortaddr,LONG_ADDR longaddr)
{
	int Tp;
	int Pos=-1;
	
	if(EndDeviceAnounceTable.Counter>0)
	{
		Pos = EndDeviceLookupNodeByLongAddr(longaddr);
		if(Pos!=-1)
		{
			printf("\n\rPrivious Entery Found");
			PrintChar(Pos);
			EndDeviceAnounceTable.EndDevAddr[Pos].shortaddress.Val = shortaddr.Val;
			for(Tp=0;Tp<8;Tp++)
			{
				EndDeviceAnounceTable.EndDevAddr[Pos].longaddress.v[Tp] = longaddr.v[Tp];
			}
			//printf("\n\rrAddEndDeviceAnnceDevice:  Short Address = ");
			//PrintChar(EndDeviceAnounceTable.EndDevAddr[Pos].shortaddress.v[0]);
			//PrintChar(EndDeviceAnounceTable.EndDevAddr[Pos].shortaddress.v[1]);
			
			//{
				//printf("\n\rrAddEndDeviceAnnceDevice: Long Address = ");
				//for(Tp=7;Tp>=0;Tp--)
				//{
				//	PrintChar(EndDeviceAnounceTable.EndDevAddr[Pos].longaddress.v[Tp]);
				//}
				//printf("\n\r");
			//}
			return;
		}
	}
	
	EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].shortaddress.Val = shortaddr.Val;
	
	for(Tp=0;Tp<8;Tp++)
	{
		EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].longaddress.v[Tp] = longaddr.v[Tp];
	}

	printf("\n\rNew Device Added No:-");
	PrintChar(EndDeviceAnounceTable.Counter);
	
	
	//printf("\n\rAddEndDeviceAnnceDevice: Short Address = ");
	//PrintChar(EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].shortaddress.v[1]);
	//PrintChar(EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].shortaddress.v[0]);
		
	//{
	//	printf("\n\rAddEndDeviceAnnceDevice: Address = ");
	//	for(Tp=7;Tp>=0;Tp--)
	//	{
	//		PrintChar(EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].longaddress.v[Tp]);
	//	}
	//	printf("\n\r");
	//}
	
	
	EndDeviceAnounceTable.Counter++;
	if(EndDeviceAnounceTable.Counter>=MAX_METER_TO_JOIN)
		EndDeviceAnounceTable.Counter = MAX_METER_TO_JOIN;
}


void AddNWKAddrReq(SHORT_ADDR shortaddr,LONG_ADDR longaddr)
{
	int Pos;
	int Tp;
	
	
	
	Pos = EndDeviceLookupNodeByLongAddr(longaddr);
	if(Pos!=-1)
	{
		//printf("\n\rPrivious Entery Found");
		EndDeviceAnounceTable.EndDevAddr[Pos].shortaddress.Val = shortaddr.Val;
		for(Tp=0;Tp<8;Tp++)
		{
			EndDeviceAnounceTable.EndDevAddr[Pos].longaddress.v[Tp] = longaddr.v[Tp];
		}
		//printf("\n\rAddNWKAddrReq: Short Address = ");
		//PrintChar(EndDeviceAnounceTable.EndDevAddr[Pos].shortaddress.v[0]);
		//PrintChar(EndDeviceAnounceTable.EndDevAddr[Pos].shortaddress.v[1]);
		
		//{
		//	printf("\n\rAddNWKAddrReq: Long Address = ");
		//	for(Tp=7;Tp>=0;Tp--)
		//	{
		//		PrintChar(EndDeviceAnounceTable.EndDevAddr[Pos].longaddress.v[Tp]);
		//	}
		//	printf("\n\r");
		//}
	}
	else
	{
		//printf("\n\rNew Entery");
		EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].shortaddress.Val = shortaddr.Val;
		for(Tp=0;Tp<8;Tp++)
		{
			EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].longaddress.v[Tp] = longaddr.v[Tp];
		}
		//printf("\n\rAddNWKAddrReq: Short Address = ");
		//PrintChar(EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].shortaddress.v[0]);
		//PrintChar(EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].shortaddress.v[1]);
		
		//{
		//	printf("\n\rAddNWKAddrReq: Long Address = ");
		//	for(Tp=7;Tp>=0;Tp--)
		//	{
		//		PrintChar(EndDeviceAnounceTable.EndDevAddr[EndDeviceAnounceTable.Counter].longaddress.v[Tp]);
		//	}
		//	printf("\n\r");
		//}
		EndDeviceAnounceTable.Counter++;
		if(EndDeviceAnounceTable.Counter>=MAX_METER_TO_JOIN)
			EndDeviceAnounceTable.Counter = MAX_METER_TO_JOIN;
	}
}



