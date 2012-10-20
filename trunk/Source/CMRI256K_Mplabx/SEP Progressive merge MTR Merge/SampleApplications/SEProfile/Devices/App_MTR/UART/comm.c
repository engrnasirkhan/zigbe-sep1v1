#include <string.h>
#if defined (__PIC24FJ256GB110__)
	#include <p24FJ256GB110.h>
#elif defined (__PIC24FJ256GB106__)		//yoda
	#include <p24FJ256GB106.h>
#else
	#error "define corrent processor"
#endif

#include "comm.h"

#define BUFFER_SIZE 100
#define BPS 		9600UL


static int TxRun;
static volatile struct
{
	int		rptr;
	int		wptr;
	int		count;
	BYTE	buff[BUFFER_SIZE];
} TxFifo, RxFifo;




void __attribute__((interrupt, auto_psv)) _U1RXInterrupt (void)
{
	BYTE d;
	int n;
	
	while(U1STA & 0x0001)
	{
		d = (BYTE)U1RXREG;
		_U1RXIF = 0;
		n = RxFifo.count;
		if (n < BUFFER_SIZE) {
			n++;
			RxFifo.count = n;
			n = RxFifo.wptr;
			RxFifo.buff[n] = d;
			RxFifo.wptr = (n + 1) % BUFFER_SIZE;
		}
	}
	if(U1STA & 0x000E)	//If overrun error occured, reset the buffer overrun
	{
		U1STA &= 0xFFF1;
	}
}




void __attribute__((interrupt, auto_psv)) _U1TXInterrupt (void)
{
	int n;


	_U1TXIF = 0;
	n = TxFifo.count;
	if (n) {
		n--;
		TxFifo.count = n;
		n = TxFifo.rptr;
		U1TXREG = TxFifo.buff[n];
		TxFifo.rptr = (n + 1) % BUFFER_SIZE;
	} else {
		TxRun = 0;
	}
}


int uart_test (void)
{
	return RxFifo.count;
}


BYTE uart_get (void)
{
	BYTE d;
	int n;


	while (!RxFifo.count);

	n = RxFifo.rptr;
	d = RxFifo.buff[n];
	RxFifo.rptr = (n + 1) % BUFFER_SIZE;
	//_DI();
	RxFifo.count--;
	//_EI();

	return d;
}




void uart_put (BYTE d)
{
	int n;


	while (TxFifo.count >= BUFFER_SIZE);

	n = TxFifo.wptr;
	TxFifo.buff[n] = d;
	TxFifo.wptr = (n + 1) % BUFFER_SIZE;
	//_DI();
	TxFifo.count++;
	if (!TxRun) {
		TxRun = 1;
		_U1TXIF = 1;
	}
	//_EI();
}




void uart_init (void)
{
	/* Disable Tx/Rx interruptrs */
	_U1RXIE = 0;
	_U1TXIE = 0;

	/* Initialize UART1 */
	U1BRG = FCY / 16 / BPS - 1;
	U1MODEbits.UARTEN = 1;
	U1STAbits.UTXEN = 1;

	/* Clear Tx/Rx FIFOs */
	TxFifo.rptr = TxFifo.wptr = TxFifo.count = 0;
	RxFifo.rptr = RxFifo.wptr = RxFifo.count = 0;
	TxRun = 0;

	/* Enable Tx/Rx interruptrs */
	_U1RXIE = 1;
	_U1TXIE = 1;
}

unsigned char uart_kbhit(void)
{
	if(RxFifo.count !=0 )
		return 1;
	return 0;
}

