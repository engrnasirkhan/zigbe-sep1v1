/*********************************************************************
 
 ********************************************************************/

/****************************************************************************
   Includes
 *****************************************************************************/
/* Configuration Files */
#include "zigbee.def"
#include "ZigbeeTasks.h"
#include "SE_Profile.h"
#include "zZDO.h"
#include "ZCL_Interface.h"
#include "zNVM.h"
#include "SE_MTR.h"
#include "Console.h"
#include "SymbolTime.h"
#include "MSDCL_Commissioning.h"
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

/******************************************************************************
        Variable Definations
******************************************************************************/
//unsigned int NowIam = A_ROUTER | A_FFD; 	// These variables are exported in Zigbee.def
unsigned int NowIam = A_CORDINATOR | A_FFD; // These variables are exported in Zigbee.def
unsigned char I_AM_TRUST_CENTER = 1; 		//Trust center enabled Enabled
unsigned char USE_COMMON_TC_LINK_KEY = 1;

/*****************************************************************************
   Function Prototypes
 *****************************************************************************/

BYTE *appNextSeqNum_PTR;
App_AttributeStorage  *App_AttributeStorageTable;
ZCL_DeviceInfo *pAppListOfDeviceServerInfo[2];
ZCL_DeviceInfo *pAppListOfDeviceClientInfo[2];
ROM NODE_SIMPLE_DESCRIPTOR   *Config_Simple_Descriptors;
unsigned int MAX_ENERGY_THRESHOLD;
unsigned char DEFAULT_STARTUP_CONTROL = DEFAULT_STARTUP_CONTROL_ESP;

WORD MSDCL_Commission_Locations = 1;	//The location of MSDLC_Commission shoudl always be zero in EEPROM
MSDCL_Commissioning_Cluster MSDCL_Commission;

unsigned long ALLOWED_CHANNELS;
DWORD_VAL ChannelsToBeScanned;
BYTE MAXDISCOVERYYRETRIES = 3;
 /*****************************************************************************
  Private Functions
 *****************************************************************************/
extern BOOL StartNetworkFlag;
extern App_AttributeStorage  App_AttributeStorageTable_ESP[];
extern App_AttributeStorage  App_AttributeStorageTable_MTR[];
extern NODE_DESCRIPTOR          Config_Node_Descriptor;
extern ROM NODE_SIMPLE_DESCRIPTOR   Config_Simple_Descriptors_MTR[];
extern ROM NODE_SIMPLE_DESCRIPTOR   Config_Simple_Descriptors_ESP[];

extern ZCL_DeviceInfo Meter_DeviceServerinfo;
extern ZCL_DeviceInfo ESP_DeviceServerinfo;
extern ZCL_DeviceInfo Meter_DeviceClientinfo;
extern ZCL_DeviceInfo ESP_DeviceClientinfo;
void HardwareInit( void );
void uart_init( void );
void SendModuleStartData( void );
void HanddleUART2( void );
/*****************************************************************************
  Public Functions
 *****************************************************************************/

// Meter_FormattingAttributes meterFormatAttributes;



int main ()
{
	

	unsigned char zigbee_mode = 0;

	HardwareInit();
	ConsoleInit();
	InitSymbolTimer();
	uart_init();
	initMSDCLTimers();
	IFS1bits.U2RXIF = 0;
	

	ConsolePutROMString( (ROM char *)"\r\nW to exit");

	SendModuleStartData();
	while( StartNetworkFlag == FALSE )
	{
		HanddleUART2();	
		if(IFS1bits.U2RXIF)
		{
			zigbee_mode = U2RXREG;
			if(zigbee_mode == 'W')
				break;
		}
	}
	StartNetworkFlag = FALSE;
	zigbee_mode = 0;
	
    ConsolePutROMString( (ROM char *)"\r\n*********************************" );
	ConsolePutROMString( (ROM char *)"\r\nMicrochip SE Profile 1.0.version.0.5.3" );
	ConsolePutROMString( (ROM char *)"\r\n*********************************" );
	ConsolePutROMString( (ROM char *)"\r\nE:Comission device as ESP\r\n" );
	ConsolePutROMString( (ROM char *)"\r\nM:Comission device as MTR\r\n" );
	
	{
		
		TICK startTime = TickGet();
		
		do
		{
			IFS1bits.U2RXIF = 0;
			do
			{
				if( TickGetDiff(TickGet(),startTime) > (2 * ONE_SECOND))
				{
					break;
				}
			}
			while( !IFS1bits.U2RXIF );
			if( TickGetDiff(TickGet(),startTime) > (2 * ONE_SECOND))
			{
				break;
			}
			zigbee_mode = U2RXREG;
			ConsolePut(zigbee_mode);
		}while( (zigbee_mode != 'M') && (zigbee_mode != 'm') && (zigbee_mode != 'E') && (zigbee_mode != 'e') );
		
		NVMRead ( (BYTE*)&MSDCL_Commission, MSDCL_Commission_Locations, sizeof(MSDCL_Commission));

		if( ( MSDCL_Commission.ValidCleanStartUp != MSDCL_COMMISSION_DATA_VALID )
			 && (MSDCL_Commission.ValidCleanStartUp != MSDCL_DEFAULT_MTR) && 
			 (MSDCL_Commission.ValidCleanStartUp != MSDCL_DEFAULT_ESP) &&  (zigbee_mode != 'E') && (zigbee_mode != 'e') )
		{
			zigbee_mode = 'M';
	 	}   
	 			
		if( ((zigbee_mode == 'M') || (zigbee_mode == 'm') || (MSDCL_Commission.ValidCleanStartUp == MSDCL_DEFAULT_MTR))
			&& (zigbee_mode != 'E') && (zigbee_mode != 'e') )
		{
			NowIam = 0;
 			NowIam = A_ROUTER | A_FFD; // These variables are exported in Zigbee.def
			I_AM_TRUST_CENTER = 0; 		//Trust center enabled Enabled
			USE_COMMON_TC_LINK_KEY = 1;
			MAX_ENERGY_THRESHOLD = 112;
			DEFAULT_STARTUP_CONTROL = DEFAULT_STARTUP_CONTROL_MTR;
			MSDCL_Commission.StartupStatus = STARTUP_CONTROL_JOIN_NEW_NETWORK;
			ALLOWED_CHANNELS = ALLOWED_CHANNELS_PRE_CONFIG;
			if(MSDCL_Commission.ValidCleanStartUp != MSDCL_DEFAULT_MTR)
			{
				MSDCL_Commission.ValidCleanStartUp = MSDCL_DEFAULT_MTR;
				NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
			}
		}
		else if( (zigbee_mode == 'E') || (zigbee_mode == 'e') || (MSDCL_Commission.ValidCleanStartUp == MSDCL_DEFAULT_ESP)  )
		{
			NowIam = 0;
 			NowIam = A_CORDINATOR | A_FFD; // These variables are exported in Zigbee.def
			I_AM_TRUST_CENTER = 1; 		//Trust center enabled Enabled
			USE_COMMON_TC_LINK_KEY = 1;
			MAX_ENERGY_THRESHOLD = 241;
			DEFAULT_STARTUP_CONTROL = DEFAULT_STARTUP_CONTROL_ESP;
			MSDCL_Commission.StartupStatus = STARTUP_CONTROL_FORM_NEW_NETWORK;
			ALLOWED_CHANNELS = ALLOWED_CHANNELS_PRE_CONFIG;
			if(MSDCL_Commission.ValidCleanStartUp != MSDCL_DEFAULT_ESP)
			{
				MSDCL_Commission.ValidCleanStartUp = MSDCL_DEFAULT_ESP;
				NVMWrite( MSDCL_Commission_Locations, (BYTE*)&MSDCL_Commission, sizeof(MSDCL_Commission) );
			}
		}


	if((MSDCL_Commission.ValidCleanStartUp == MSDCL_COMMISSION_DATA_VALID) )
	{
	 	switch( MSDCL_Commission.StartupStatus )
	 	{
		 	case STARTUP_CONTROL_FORM_NEW_NETWORK:
		 		ConsolePutROMString( (ROM char *)"\r\nStarting as ESP\r\n" );
				NowIam = 0;
	 			NowIam = A_CORDINATOR | A_FFD; // These variables are exported in Zigbee.def
				I_AM_TRUST_CENTER = 1; 		//Trust center enabled Enabled
				USE_COMMON_TC_LINK_KEY = 1;
				MAX_ENERGY_THRESHOLD = 241;
				ALLOWED_CHANNELS = MSDCL_Commission.ChannelMask.Val ;
				DEFAULT_STARTUP_CONTROL = DEFAULT_STARTUP_CONTROL_ESP;
		 	break;
		 	
		 	case STARTUP_CONTROL_PART_OF_NETWORK_NO_EXPLICIT_ACTION:
		 	case STARTUP_CONTROL_JOIN_NEW_NETWORK:
		 	case STARTUP_CONTROL_START_FROM_SCRATCH_AS_ROUTER:
		 	default:
		 		ConsolePutROMString( (ROM char *)"\r\nStarting as MTR\r\n" );
				NowIam = 0;
	 			NowIam = A_ROUTER | A_FFD; // These variables are exported in Zigbee.def
				I_AM_TRUST_CENTER = 1; 		//Trust center enabled Enabled
				USE_COMMON_TC_LINK_KEY = 0;
				MAX_ENERGY_THRESHOLD = 112;
				ALLOWED_CHANNELS = MSDCL_Commission.ChannelMask.Val ;
				DEFAULT_STARTUP_CONTROL = DEFAULT_STARTUP_CONTROL_MTR;
		 	break;
		}
	}
	
	}
	
//	if (NOW_I_AM_A_CORDINATOR())
//		USE_COMMON_TC_LINK_KEY = 1;
//	else
//		USE_COMMON_TC_LINK_KEY = 0;
		
	if(NOW_I_AM_A_ROUTER())
		I_AM_TRUST_CENTER = 0;

	
	if(NOW_I_AM_A_ROUTER())
		appNextSeqNum_PTR = &appNextSeqNum_MTR;
	else if (NOW_I_AM_A_CORDINATOR())
		appNextSeqNum_PTR = &appNextSeqNum_ESP;
	
	if(NOW_I_AM_A_ROUTER())
		App_AttributeStorageTable = App_AttributeStorageTable_MTR;
	else if (NOW_I_AM_A_CORDINATOR())
		App_AttributeStorageTable = App_AttributeStorageTable_ESP;
		
	if(NOW_I_AM_A_ROUTER())
		Config_Node_Descriptor.NodeLogicalType = 0x01;
	else if (NOW_I_AM_A_CORDINATOR())
		Config_Node_Descriptor.NodeLogicalType = 0x00;
		
	if( NOW_I_AM_A_ROUTER() )
		pAppListOfDeviceServerInfo[0] = &Meter_DeviceServerinfo;
	else if( NOW_I_AM_A_CORDINATOR() )
		pAppListOfDeviceServerInfo[0] = &ESP_DeviceServerinfo;
		
	if( NOW_I_AM_A_ROUTER() )
		pAppListOfDeviceClientInfo[0] = &Meter_DeviceClientinfo;
	else if( NOW_I_AM_A_CORDINATOR() )
		pAppListOfDeviceClientInfo[0] = &ESP_DeviceClientinfo;
		
	if( NOW_I_AM_A_ROUTER() )
		Config_Simple_Descriptors = Config_Simple_Descriptors_MTR;
	else if( NOW_I_AM_A_CORDINATOR() )
		Config_Simple_Descriptors = Config_Simple_Descriptors_ESP;

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
		
		if( ( ChannelsToBeScanned.Val & 0x03FFF800UL ) == 0 )
		{
			ChannelsToBeScanned.Val = ALLOWED_CHANNELS_PRE_CONFIG & 0x03FFF800UL;
		}
		
		ChannelsToBeScanned.Val &= 0x03FFF800UL;
	
		while( !(ChannelsToBeScanned.Val & channelMaskToScan) )
		{
			channelMaskToScan <<= 1;	
		}
		ALLOWED_CHANNELS = channelMaskToScan;
		ChannelsToBeScanned.Val &= channelMaskToScan ^ 0xFFFFFFFFUL;
		
		//ALLOWED_CHANNELS = 0x3FFFC00UL;
	}
	

	//ALLOWED_CHANNELS = 0b000000000111111111101111100000000000;
	ALLOWED_CHANNELS = 0b0000000000010000000000000000000000;

	while(1)
	{	
		if (NOW_I_AM_A_CORDINATOR())
			main_ESP(); 
		else
		{
			
			main_MTR();
		}
	}
}
