#ifndef _MERGE_INFO_H_
#define _MERGE_INFO_H_

#include "generic.h"

#define MERGED_STACK					1
//----------------
#define A_CORDINATOR				0x0001
#define A_ROUTER					0x0002
#define A_FFD						0x0004
#define A_RFD						0x0008
//---------------
#define NOW_I_AM_A_CORDINATOR()		(NowIam & A_CORDINATOR)
#define NOW_I_AM_A_ROUTER()			(NowIam & A_ROUTER)
#define NOW_I_AM_A_FFD()			(NowIam & A_FFD)
#define NOW_I_AM_A_RFD()			(NowIam & A_RFD)

#define NOW_I_AM_NOT_A_CORDINATOR()		(!(NowIam & A_CORDINATOR))
#define NOW_I_AM_NOT_A_ROUTER()			(!(NowIam & A_ROUTER))
#define NOW_I_AM_NOT_A_FFD()			(!(NowIam & A_FFD))
#define NOW_I_AM_NOT_A_RFD()			(!(NowIam & A_RFD))

//--------------

#define SOURCE_ENDPOINT                 (NOW_I_AM_A_ROUTER()?METER_ENDPOINT_ID:METER_ENDPOINT_ID)
#define appNextSeqNum 					(*appNextSeqNum_PTR)

extern unsigned int MAX_ENERGY_THRESHOLD;
extern unsigned char DEFAULT_STARTUP_CONTROL;
extern unsigned int NowIam;
extern unsigned char I_AM_TRUST_CENTER;
extern unsigned char USE_COMMON_TC_LINK_KEY;

extern BYTE appNextSeqNum_MTR;
extern BYTE appNextSeqNum_ESP;
extern BYTE *appNextSeqNum_PTR;


int main_MTR();
int main_ESP();
#endif //_MERGE_INFO_H_
