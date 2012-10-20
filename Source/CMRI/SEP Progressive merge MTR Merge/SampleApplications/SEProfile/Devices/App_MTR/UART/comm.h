#ifndef _COMMFUNC
#define _COMMFUNC

#include "generic.h"


#define FCY		4000000UL

#define _DI()		__asm__ volatile("disi #0x3FFF")
#define _EI()		__asm__ volatile("disi #0")


void uart_init (void);
int uart_test (void);
void uart_put (BYTE);
BYTE uart_get (void);
unsigned char uart_kbhit(void);


#endif

