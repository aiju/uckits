#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>

enum {
	LED = PORTB(1)
};

void
main()
{
	initclk();
	gpiocfg(LED, GPIOOUT);

	for(;;){
		gpioset(LED, 1);
		delayus(500000);
		gpioset(LED, 0);
		delayus(500000);
	}
}
