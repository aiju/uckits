#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>
#include <usb.h>

enum {
	DISCON = PORTB(9),
	USBDP = PORTA(12),
	USBDM = PORTA(11)
};

void
usbdisconnect(int n)
{
	gpioset(DISCON, n);
}

void
main()
{
	char c;

	initclk();
	gpiocfg(DISCON, GPIOOUT);
	gpiocfg(USBDP, GPIOOUT | GPIOALT | GPIOSPEED2);
	gpiocfg(USBDM, GPIOOUT | GPIOALT | GPIOSPEED2);
	usbacminit();
	irqen(USB_LP_IRQn, 1, 0);
	
	for(;;)
		if(cquread(&usbacmrxqu, &c, 1) > 0)
			cquwrite(&usbacmtxqu, &c, 1);
}

void
trap(Ureg *ur)
{
	switch(ur->irq){
	case USB_LP_IRQn: usbirq(); break;
	default:
		for(;;);
	}
}
