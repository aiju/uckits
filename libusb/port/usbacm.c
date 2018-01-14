#include <u.h>
#include <libc.h>
#include <libio.h>
#include <usb.h>

static USBEp usbacmep[4];

extern USBDesc usbacmdesc[];
#define txep (&usbep[2])
#define rxep (&usbep[3])
CQueue usbacmrxqu, usbacmtxqu;

enum { MAXPKT = 8 };
static uchar rxbuf[512], txbuf[512];
static uchar rpkt[MAXPKT], tpkt[MAXPKT];
static int rpktp, rpktl, tpktp, tpktl;

static void
eprecv(USBEp *ep)
{
	int n;

	if(rpktp < rpktl){
		n = cquwritenb(&usbacmrxqu, rpkt + rpktp, rpktl - rpktp);
		if(n > 0)
			rpktp += n;
	}
	if(rpktp < rpktl)
		return;
	n = usbeprecvnb(ep, rpkt, MAXPKT);
	if(n > 0){
		rpktl = n;
		n = cquwritenb(&usbacmrxqu, rpkt, n);
		rpktp = n > 0 ? n : 0;
	}
}

static void
qurecv(CQueue *qu)
{
	eprecv(rxep);
}

static void
epsend(USBEp *ep)
{
	int n;

	if(tpktp < tpktl){
		n = usbepsendnb(ep, tpkt + tpktp, tpktl - tpktp);
		if(n > 0)
			tpktp += n;
	}
	if(tpktp < tpktl)
		return;
	n = cqureadnb(&usbacmtxqu, tpkt, MAXPKT);
	if(n > 0){
		tpktl = n;
		n = usbepsendnb(ep, tpkt, n);
		tpktp = n > 0 ? n : 0;
	}
}

static void
qusend(CQueue *qu)
{
	epsend(txep);
}

static int
usbacmconfig(u8int n)
{
	switch(n){
	case 0:
		usbepclear();
		return 0;
	case 1:
		usbepclear();
		usbepcfg(EPIN | 1, EPINT, 8, nil, nil);
		usbepcfg(EPIN | 2, EPBULK, MAXPKT, epsend, nil);
		usbepcfg(EPOUT | 2, EPBULK, MAXPKT, eprecv, nil);
		return 0;
	default:
		return -1;
	}
}

void
usbacminit(void)
{
	memset(&usbacmrxqu, 0, sizeof(CQueue));
	memset(&usbacmtxqu, 0, sizeof(CQueue));
	usbacmrxqu.d = rxbuf;
	usbacmrxqu.sz = sizeof(rxbuf);
	usbacmrxqu.fntxkick = qurecv;
	usbacmtxqu.d = txbuf;
	usbacmtxqu.sz = sizeof(txbuf);
	usbacmtxqu.fnrxkick = qusend;
	usbinit(usbacmdesc, usbacmep, nelem(usbacmep), usbacmconfig);
}
