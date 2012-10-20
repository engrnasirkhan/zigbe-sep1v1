#include "generic.h"
#include "comm.h"
#define xgetc()		(char)uart_get()

int x2atoi (char**, long*);
void x2putc (char);
void x2puts (const char*);
void x2itoa (long, char, char);
void x2printf (const char*, ...);
void put2_dump (const BYTE*, DWORD ofs, int cnt);
void get2_line (char*, int len);

