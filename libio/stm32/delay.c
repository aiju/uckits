#include <u.h>
#include <uio.h>
#include <libio.h>

void
delayus(uint us)
{
	/* BUG: ahbhz must be a multiple of 1 million */
	SysTick->CTRL = SysTick->CTRL & ~0x10003 | 4;
	SysTick->LOAD = (ahbhz / 1000000) - 1;
	SysTick->CTRL |= 1;
	for(; us > 0; us--)
		while((SysTick->CTRL & 0x10000) == 0)
			;
}
