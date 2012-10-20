#include <stdarg.h>
#include "monitor.h"



int x2atoi (char **str, long *res)
{
	DWORD val;
	BYTE c, radix, s = 0;


	while ((c = **str) == ' ') (*str)++;
	if (c == '-') {
		s = 1;
		c = *(++(*str));
	}
	if (c == '0') {
		c = *(++(*str));
		if (c <= ' ') {
			*res = 0; return 1;
		}
		if (c == 'x') {
			radix = 16;
			c = *(++(*str));
		} else {
			if (c == 'b') {
				radix = 2;
				c = *(++(*str));
			} else {
				if ((c >= '0')&&(c <= '9'))
					radix = 8;
				else
					return 0;
			}
		}
	} else {
		if ((c < '1')||(c > '9'))
			return 0;
		radix = 10;
	}
	val = 0;
	while (c > ' ') {
		if (c >= 'a') c -= 0x20;
		c -= '0';
		if (c >= 17) {
			c -= 7;
			if (c <= 9) return 0;
		}
		if (c >= radix) return 0;
		val = val * radix + c;
		c = *(++(*str));
	}
	if (s) val = -val;
	*res = val;
	return 1;
}




void x2putc (char c)
{
	//if (c == '\n') uart_put('\r');
	uart_put(c);
}




void x2puts (const char* str)
{
	while (*str) x2putc(*str++);
}




void x2itoa (long val, char radix, char len)
{
	BYTE c, r, sgn = 0, pad = ' ';
	BYTE s[20], i = 0;
	DWORD v;


	if (radix < 0) {
		radix = -radix;
		if (val < 0) {
			val = -val;
			sgn = '-';
		}
	}
	v = val;
	r = radix;
	if (len < 0) {
		len = -len;
		pad = '0';
	}
	if (len > 20) return;
	do {
		c = (BYTE)(v % r);
		if (c >= 10) c += 7;
		c += '0';
		s[i++] = c;
		v /= r;
	} while (v);
	if (sgn) s[i++] = sgn;
	while (i < len)
		s[i++] = pad;
	do
		x2putc(s[--i]);
	while (i);
}




void x2printf (const char* str, ...)
{
	va_list arp;
	char d, r, w, s, l;


	va_start(arp, str);

	while ((d = *str++) != 0) {
		if (d != '%') {
			x2putc(d); continue;
		}
		d = *str++; w = r = s = l = 0;
		if (d == '0') {
			d = *str++; s = 1;
		}
		while ((d >= '0')&&(d <= '9')) {
			w += w * 10 + (d - '0');
			d = *str++;
		}
		if (s) w = -w;
		if (d == 'l') {
			l = 1;
			d = *str++;
		}
		if (!d) break;
		if (d == 's') {
			x2puts(va_arg(arp, char*));
			continue;
		}
		if (d == 'c') {
			x2putc(va_arg(arp, char));
			continue;
		}
		if (d == 'u') r = 10;
		if (d == 'd') r = -10;
		if (d == 'X') r = 16;
		if (d == 'b') r = 2;
		if (!r) break;
		if (l) {
			x2itoa(va_arg(arp, long), r, w);
		} else {
			if (r > 0)
				x2itoa((long)va_arg(arp, unsigned int), r, w);
			else
				x2itoa((long)va_arg(arp, int), r, w);
		}
	}

	va_end(arp);
}




void put2_dump (const BYTE *buff, DWORD ofs, int cnt)
{
	BYTE n;


	x2printf("%08lX ", ofs);
	for(n = 0; n < cnt; n++)
		x2printf(" %02X", buff[n]);
	x2putc(' ');
	for(n = 0; n < cnt; n++) {
		if ((buff[n] < 0x20)||(buff[n] >= 0x7F))
			x2putc('.');
		else
			x2putc(buff[n]);
	}
	x2putc('\n');
}







