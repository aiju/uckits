#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>
#include <usb.h>

USBEp usbep[4];
int ROM usbmaxep = nelem(usbep);

enum {
	DISCON = PORTB(9),
	USBDP = PORTA(12),
	USBDM = PORTA(11),
	LED = PORTB(1)
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
	gpiocfg(LED, GPIOOUT);
	usbacminit();
	irqen(USB_LP_IRQn, 1, 0);
	
	for(;;){
		if(cqureadnb(&usbacmrxqu, &c, 1) > 0)
			cquwritenb(&usbacmtxqu, &c, 1);
	}
}

void
trap(Ureg *ur)
{
	switch(ur->irq){
	case USB_LP_IRQn: gpioset(LED, 1); usbirq(); gpioset(LED, 0); break;
	default:
		for(;;);
	}
}
