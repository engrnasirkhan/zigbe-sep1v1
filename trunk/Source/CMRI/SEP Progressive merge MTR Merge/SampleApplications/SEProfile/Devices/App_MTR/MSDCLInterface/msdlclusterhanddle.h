

//msdlclusterhanddle.h

#include "zigbee.def"

/*Generic Files*/
#include "generic.h"
#include "sralloc.h"
#include "zNVM.h"
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


#ifndef __MSDCL_CLUSTER_HANDLE
#define __MSDCL_CLUSTER_HANDLE

#define MAX_METER_TO_JOIN   50

struct __EndDeviceAnounceAddress
{	
	SHORT_ADDR 	shortaddress;
	LONG_ADDR	longaddress;
};

struct __EndDeviceAnounceTable
{	
	struct __EndDeviceAnounceAddress  EndDevAddr[MAX_METER_TO_JOIN+1];
	int Counter;
};

#define ACK_PENDING 	0x00
#define ACK_NOTRECEIVED 0x01
#define ACK_RECEIVED	0x02

struct __MyCommandFlag
{
	unsigned char ACK;
	unsigned char DataSent;
};

struct _MACAddMeter
{
	LONG_ADDR AddMeter[MAX_METER_TO_JOIN+1];
	int NumberMeter;
};

extern struct _MACAddMeter MACAddMeter;
extern struct __MyCommandFlag MyCommandFlag;
//extern struct MSDLValue MTRActualValues;
extern struct __EndDeviceAnounceTable	EndDeviceAnounceTable;
extern struct __EndDeviceAnounceTable	PerMissionEndDeviceAnounceTable;

void HanddleMSDLClusterParameters(APP_DATA_indication* p_dataInd);
void HanddleUART2Request(int Seq,int Cmd,unsigned char RW,APP_DATA_indication* p_dataInd);
void HanddleUART2(void);
SHORT_ADDR FindShortAddressFromMtrSerial(LONG_ADDR MtrSerial);
LONG_ADDR FindMeterSerialNumberFromShortAddr(SHORT_ADDR  ShortAddr);
void PutMtrSerialNumberAtShortAddr(SHORT_ADDR ShortAddr,LONG_ADDR MtrSerial);
void CheckDataAndSend(void);

void SendExtPANIdResponse(void);
void SendModuleStartData(void);
void SendModuleNetworkSetCommandResponse( const unsigned char* responseCommand, unsigned char dataSize );
void SendAddMeterCommandResponce(unsigned char success);
void FindNumberofAddedMetersResponce(void);
BOOL CheckDeviceJoiningPermission(LONG_ADDR ieeeAddr);
void InitAddMeterCommand(void);
void AddEndDeviceAnnceDevice(SHORT_ADDR shortaddr,LONG_ADDR longaddr);
void AddNWKAddrReq(SHORT_ADDR shortaddr,LONG_ADDR longaddr);
int EndDeviceLookupNodeByLongAddr( LONG_ADDR longAddr );
void FindMyChannel(void);
#endif




