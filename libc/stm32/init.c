#include <u.h>
#include <libc.h>

void
_init(void)
{
	u32int *p, *q;
	
	p = (u32int *) _etext;
	q = (u32int *) _data;
	while(q < (u32int *) _edata)
		*q++ = *p++;
	while(q < (u32int *) _end)
		*q++ = 0;
}
