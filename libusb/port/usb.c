#include <u.h>
#include <libc.h>
#include <libio.h>
#include <usb.h>

USBDesc *usbdesctab;
USBEp *usbep;
int (*usbconfig)(u8int);
int usbmaxep;

void
usbepclear(void)
{
	int i;

	for(i = 1; i < usbmaxep; i++)
		usbepdecfg(&usbep[i]);
}

USBEp *
usbepcfg(u8int addr, u8int type, int maxpkt, void (*act)(USBEp *), void *aux)
{
	USBEp *ep;
	int i, s;
	
	s = splhi();
	for(i = 0; i < usbmaxep; i++)
		if((usbep[i].type & EPACTIVE) == 0)
			break;
	if(i == usbmaxep){
		splx(s);
		return nil;
	}
	ep = &usbep[i];
	ep->addr = addr;
	ep->type = type | EPACTIVE;
	ep->maxpkt = maxpkt;
	ep->act = act;
	ep->aux = aux;
	if(usbphyepcfg(ep) < 0){
		ep->type &= ~EPACTIVE;
		splx(s);
		return nil;
	}
	if((ep->type & EPTYPE) != EPCTRL && (ep->addr & EPDIR) == EPIN && act != nil)
		act(ep);
	splx(s);
	return ep;
}

void
usbepdecfg(USBEp *ep)
{
	int s;
	
	s = splhi();
	if((ep->type & EPACTIVE) != 0)
		usbphyepdecfg(ep);
	ep->type &= ~EPACTIVE;
	splx(s);
}

void
usbinit(USBDesc *desc, USBEp *ep, int nep, int (*cfg)(u8int))
{
	usbdesctab = desc;
	usbconfig = cfg;
	usbep = ep;
	usbmaxep = nep;
	memset(usbep, 0, usbmaxep * sizeof(USBEp));
	usbep0reset();
	usbphyinit();
}

void
usbreset(void)
{
	usbphyreset();
	if((usbep[0].type & EPACTIVE) != 0)
		usbepdecfg(&usbep[0]);
	usbepclear();
	usbepcfg(0, EPCTRL, 64, nil, nil);
	usbep0reset();
}
