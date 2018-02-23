#include <u.h>
#include <libc.h>
#include <libio.h>

extern void _libioinit();

void
_init(void)
{
	u32int *p, *q;
	int i;
	
	p = (u32int *) _etext;
	q = (u32int *) _data;
	while(q < (u32int *) _edata)
		*q++ = *p++;
	while(q < (u32int *) _end)
		*q++ = 0;

	for(i = PERGPIOA; i <= PERAFIO; i++)
		periclk(i, 1);

	_libioinit();
}
