#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>
#include <usb.h>
#include "usbif.h"

static USBEp myusbep[3];
#define rxep (&myusbep[2])
#define txep (&myusbep[1])
extern USBDesc ROM usbdesc[];
Uart uart;

CQueue txqu, rxqu;
static uchar rxbuf[512], txbuf[512];
static uchar rpkt[MAXPKT], tpkt[MAXPKT];
static int rpktp, rpktl, tpktp, tpktl;

uchar readeoi;
u16int readeos;
uchar wasreset;

enum {
	USBDP = PORTA(12),
	USBDM = PORTA(11),
	LED = PORTC(13),
	
	DIO1 = PORTA(9),
	DIO2 = PORTB(10),
	DIO3 = PORTB(12),
	DIO4 = PORTB(15),
	EOI = PORTB(7),
	DAV = PORTA(8),
	NRFD = PORTB(3),
	NDAC = PORTB(14),
	IFC = PORTB(4),
	SRQ = PORTA(15),
	ATN = PORTB(9),
	DIO5 = PORTB(13),
	DIO6 = PORTB(8),
	DIO7 = PORTB(11),
	DIO8 = PORTB(6),
	REN = PORTA(10),
};

void
usbdisconnect(int n)
{
	if(n){
		gpioset(USBDP, 0);
		gpiocfg(USBDP, GPIOOUT);
	}else
		gpiocfg(USBDP, GPIOOUT | GPIOALT | GPIOSPEED2);
}

void
sendb(u16int c)
{
	gpioset(DAV, 1);
	gpiocfg(DAV, GPIOOUT);
	c ^= 0x1ff;
	gpioset(DIO1, c & 1);
	gpioset(DIO2, c >> 1 & 1);
	gpioset(DIO3, c >> 2 & 1);
	gpioset(DIO4, c >> 3 & 1);
	gpioset(DIO5, c >> 4 & 1);
	gpioset(DIO6, c >> 5 & 1);
	gpioset(DIO7, c >> 6 & 1);
	gpioset(DIO8, c >> 7 & 1);
	gpioset(EOI, c >> 8 & 1);
	gpiocfg(DIO1, GPIOOUT);
	gpiocfg(DIO2, GPIOOUT);
	gpiocfg(DIO3, GPIOOUT);
	gpiocfg(DIO4, GPIOOUT);
	gpiocfg(DIO5, GPIOOUT);
	gpiocfg(DIO6, GPIOOUT);
	gpiocfg(DIO7, GPIOOUT);
	gpiocfg(DIO8, GPIOOUT);
	gpiocfg(EOI, GPIOOUT);
	while(!gpioget(NRFD))
		;
	gpioset(DAV, 0);
	while(!gpioget(NDAC))
		;
	gpioset(DAV, 1);
	gpiocfg(DIO1, GPIOIN);
	gpiocfg(DIO2, GPIOIN);
	gpiocfg(DIO3, GPIOIN);
	gpiocfg(DIO4, GPIOIN);
	gpiocfg(DIO5, GPIOIN);
	gpiocfg(DIO6, GPIOIN);
	gpiocfg(DIO7, GPIOIN);
	gpiocfg(DIO8, GPIOIN);
	gpiocfg(DAV, GPIOIN);
	gpiocfg(EOI, GPIOIN);
}

int
recvb(void)
{
	u16int c;

	gpiocfg(DAV, GPIOIN);
	gpiocfg(NRFD, GPIOIN);
	gpiocfg(NDAC, GPIOOUT);
	while(gpioget(DAV))
		if(wasreset || cqucanread(&rxqu)){
			print("read aborted\n");
			gpiocfg(NRFD, GPIOOUT);
			return -1;
		}
	gpiocfg(NRFD, GPIOOUT);
	c = gpioget(DIO1);
	c |= gpioget(DIO2) << 1;
	c |= gpioget(DIO3) << 2;
	c |= gpioget(DIO4) << 3;
	c |= gpioget(DIO5) << 4;
	c |= gpioget(DIO6) << 5;
	c |= gpioget(DIO7) << 6;
	c |= gpioget(DIO8) << 7;
	c |= gpioget(EOI) << 8;
	c ^= 0x1ff;
	gpiocfg(NDAC, GPIOIN);
	while(!gpioget(DAV))
		;
	gpiocfg(NDAC, GPIOOUT);
	return c;
}

static void
eprecv(USBEp *ep)
{
	int n;

	if(rpktp < rpktl){
		n = cquwritenb(&rxqu, rpkt + rpktp, rpktl - rpktp);
		if(n > 0)
			rpktp += n;
	}
	if(rpktp < rpktl)
		return;
	n = usbeprecvnb(ep, rpkt, MAXPKT);
	if(n > 0){
		rpktl = n;
		n = cquwritenb(&rxqu, rpkt, n);
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
	n = cqureadnb(&txqu, tpkt, MAXPKT);
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

int
myusbconfig(u8int c)
{
	switch(c){
	case 0:
		usbepclear();
		return 0;
	case 1:
		wasreset = 1;
		cquclear(&txqu);
		cquclear(&rxqu);
		usbepclear();
		usbepcfg(EPIN | 1, EPBULK, MAXPKT, epsend, nil);
		usbepcfg(EPOUT | 1, EPBULK, MAXPKT, eprecv, nil);
		return 0;
	default:
		return -1;
	}
}

ssize_t
write(int fd, void *d, size_t n)
{
	char *c;
	int i;
	
	c = d;
	for(i = 0; i < n; i++){
		if(c[i] == '\n')
			uartwrite(&uart, "\r", 1);
		uartwrite(&uart, &c[i], 1);
	}
	return n;
}

static void
sendbytes(void *d, int n)
{
	uchar *c;

	for(c = d; c < (uchar*)d + n; c++){
		if(*c == ChESC || *c == ChEOI)
			cquputc(&txqu, ChESC);
		cquputc(&txqu, *c);
	}
	epsend(txep);
}

void
readbytes(u32int l)
{
	int c;

	splhi();
	wasreset = 0;
	while(l--){
		spllo();
		c = recvb();
		splhi();
		if(wasreset || c < 0)
			break;
		if(c >= 0x100)
			cquputc(&txqu, ChEOI);
		cquputc(&txqu, c);
		epsend(txep);
		if(readeoi && c >= 0x100)
			break;
		if((readeos & 0x1400) == 0x400 && (c & 0x7f) == (readeos & 0x7f))
			break;
		if((readeos & 0x1400) == 0x1400 && (c & 0xff) == (readeos & 0xff))
			break;
	}
	spllo();
}

void
cmd(uchar c)
{
	uchar x;
	u32int l;

	switch(c){
	case CmdATN0: 
		gpioset(ATN, 0);
		gpioset(DAV, 1);
		gpiocfg(DAV, GPIOOUT);
		gpiocfg(NRFD, GPIOIN);
		gpiocfg(NDAC, GPIOIN);
		break;
	case CmdATN1T:
		gpioset(ATN, 1);
		gpioset(DAV, 1);
		gpiocfg(DAV, GPIOOUT);
		gpiocfg(NRFD, GPIOIN);
		gpiocfg(NDAC, GPIOIN);
		break;
	case CmdATN1L:
		gpioset(ATN, 1);
		gpiocfg(DAV, GPIOIN);
		gpiocfg(NRFD, GPIOOUT);
		gpiocfg(NDAC, GPIOOUT);
		break;
	case CmdIFC: gpioset(IFC, 0); delayus(100); gpioset(IFC, 1); break;
	case CmdStatus:
		x = ~(gpioget(DAV) | gpioget(NDAC) << 1 | gpioget(NRFD) << 2 | gpioget(IFC) << 3 | gpioget(REN) << 4 | gpioget(SRQ) << 5 | gpioget(ATN) << 6 | gpioget(EOI) << 7);
		sendbytes(&x, 1);
		break;
	case CmdReadEOI0: readeoi = 0; break;
	case CmdReadEOI1: readeoi = 1; break;
	case CmdReadEOS:
		readeos = cqugetc(&rxqu);
		readeos |= cqugetc(&rxqu) << 8;
		break;
	case CmdReadN:
		l = cqugetc(&rxqu);
		l |= cqugetc(&rxqu) << 8;
		l |= cqugetc(&rxqu) << 16;
		l |= cqugetc(&rxqu) << 24;
		readbytes(l);
		break;
	case CmdNOP: break;
	case ChESC: case ChEOI: sendb(c); break;
	default:
		print("unknown command %#.2ux\n", c);
	}
}

void
cmdloop(void)
{
	uchar c;

	for(;;){
		c = cqugetc(&rxqu);
		switch(c){
		case ChESC:
			cmd(cqugetc(&rxqu));
			break;
		case ChEOI:
			sendb(cqugetc(&rxqu) | 0x100);
			break;
		default:
			sendb(c);
			break;
		}
	}
}

void
main()
{
	initclk();
	splhi();
	uartopen(&uart, 2, 115200, 0);
	gpiocfg(PORTA(2), GPIOOUT | GPIOALT);
	gpiocfg(USBDP, GPIOOUT | GPIOALT | GPIOSPEED2);
	gpiocfg(USBDM, GPIOOUT | GPIOALT | GPIOSPEED2);
	gpiocfg(LED, GPIOOUT);
	
	txqu.d = txbuf;
	txqu.sz = sizeof(txbuf);
	txqu.fnrxkick = qusend;
	rxqu.d = rxbuf;
	rxqu.sz = sizeof(rxbuf);
	rxqu.fntxkick = qurecv;
	
	usbinit(usbdesc, myusbep, nelem(myusbep), myusbconfig);
	irqen(USB_LP_IRQn, 1, 0);
	spllo();
	
	AFIO->MAPR = AFIO->MAPR & 0xfffff | 1<<25;
	gpioset(ATN, 0);
	gpiocfg(ATN, GPIOOUT);
	gpioset(IFC, 1);
	gpiocfg(IFC, GPIOOUT);
	gpioset(REN, 0);
	gpiocfg(REN, GPIOOUT);
	gpioset(NRFD, 0);
	gpioset(NDAC, 0);
	
	print("reset.\n");
	
	cmdloop();
}

void
trap(Ureg *ur)
{
	static int led;

	switch(ur->irq){
	case USB_LP_IRQn: gpioset(LED, led ^= 1); usbirq(); break;
	default:
		for(;;);
	}
}
