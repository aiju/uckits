#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>

void
irqen(int no, int state, u8int prio)
{
	NVIC->IP[no] = prio;
	if(state)
		NVIC->ISER[no >> 5] = 1<<(no & 31);
	else
		NVIC->ICER[no >> 5] = 1<<(no & 31);
}
