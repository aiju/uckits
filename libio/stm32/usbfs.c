#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>
#include <usb.h>

#ifdef USB_PMAADDR

#define PMA ((volatile u16int*) USB_PMAADDR)

#define ep0 (usbep[0])

enum {
	EP0RVAL = USB_EP_CTR_TX | USB_EP_CTR_RX | USB_EP_CONTROL,	
};

static enum {
	EP0SETUP,
	EP0TXDATA,
	EP0TXDATAL,
	EP0RXDATA,
	EP0STATUS,
	EP0STALL,
} ep0state;

static u8int eprx[8], eptx[8];

static u64int pmaallocbits;

static int
pmaalloc(uint n)
{
	uint i;
	u64int m;
	
	n = n + 7 >> 3;
	if(n >= 64)
		return -1;
	m = ((u64int)1<<n) - 1;
	for(i = 0; i < 64; i++)
		if((pmaallocbits & m<<i) == 0){
			pmaallocbits |= m<<i;
			return i * 8;
		}
	return -1;
}

static void
pmafree(uint a, uint n)
{
	a >>= 3;
	n = n + 7 >> 3;
	pmaallocbits &= ~(((u64int)1<<n) - 1 << a);
}

static uint
rxround(uint n)
{
	if(n > 64)
		n = -(-n & -32);
	return n;
}

int
usbphyepcfg(USBEp *ep)
{
	int i, n, t;
	int txbuf, rxbuf;

	for(i = 0; i < usbmaxep; i++)
		if(usbep[i].type == ep->type && (usbep[i].addr & 0xf) == ep->addr){
			i = usbep[i].hwep;
			goto found;
		}
	for(i = 0; i < 8; i++)
		if(eprx[i] == 0xff && eptx[i] == 0xff)
			break;
	if(i == 8)
		return -1;
found:
	if((ep->addr & EPDIR) == EPIN || (ep->type & EPTYPE) == EPCTRL){
		txbuf = pmaalloc(ep->maxpkt);
		if(txbuf < 0)
			return -1;
	}else
		txbuf = -1;
	if((ep->addr & EPDIR) == EPOUT || (ep->type & EPTYPE) == EPCTRL){
		rxbuf = pmaalloc(rxround(ep->maxpkt));
		if(rxbuf < 0){
			if(txbuf >= 0)
				pmafree(txbuf, ep->maxpkt);
			return -1;
		}
	}else
		rxbuf = -1;
	if((ep->addr & EPDIR) == EPIN || (ep->type & EPTYPE) == EPCTRL)
		eptx[i] = ep - usbep;
	if((ep->addr & EPDIR) == EPOUT || (ep->type & EPTYPE) == EPCTRL)
		eprx[i] = ep - usbep;
	
	ep->hwep = i;
	ep->reg = &USB->EP0R + 2 * i;
	ep->pmatab = PMA + ep->hwep * 8;
	if(txbuf >= 0){
		ep->pmatx = PMA + txbuf;
		ep->pmatab[0] = txbuf;
		ep->pmatab[2] = 0;
	}else
		ep->pmatx = nil;
	if(rxbuf >= 0){
		ep->pmarx = PMA + rxbuf;
		ep->pmatab[4] = rxbuf;
		n = ep->maxpkt;
		if(n > 62) n = 32 | (n >> 5) - 1;
		else n >>= 1;
		ep->pmatab[6] = n << 10;
	}else
		ep->pmarx = nil;
	t = 0xc9 >> ((ep->type & EPTYPE) << 1) & 3;
	if((ep->type & EPTYPE) == EPCTRL)
		*ep->reg = *ep->reg & (USB_EPRX_STAT | USB_EPTX_STAT) ^ (USB_EP_RX_VALID | USB_EP_TX_NAK) | USB_EP_CONTROL | ep->addr & 0xf;
	else if((ep->addr & EPDIR) == EPIN)
		*ep->reg = *ep->reg & (USB_EPTX_STAT | USB_EP_DTOG_TX) ^ USB_EP_TX_NAK | t << 9 | ep->addr & 0xf;
	else
		*ep->reg = *ep->reg & (USB_EPRX_STAT | USB_EP_DTOG_RX) ^ USB_EP_RX_VALID | t << 9 | ep->addr & 0xf;
	return 0;
}

void
usbphyepdecfg(USBEp *ep)
{
	u16int m;
	
	if((ep->type & EPTYPE) == EPCTRL)
		m = USB_EPRX_STAT | USB_EPTX_STAT;
	else if((ep->addr & EPDIR) == EPIN)
		m = USB_EPTX_STAT;
	else
		m = USB_EPRX_STAT;
	while((*ep->reg & m) != 0)
		*ep->reg = *ep->reg & (USB_EPREG_MASK | m) | (USB_EP_CTR_TX|USB_EP_CTR_RX);
	if((ep->addr & EPDIR) == EPIN || (ep->type & EPTYPE) == EPCTRL){
		*ep->reg = *ep->reg & (USB_EPREG_MASK & ~USB_EP_CTR_TX) | USB_EP_CTR_RX;
		eptx[ep->hwep] = 0xff;
	}
	if((ep->addr & EPDIR) == EPOUT || (ep->type & EPTYPE) == EPCTRL){
		*ep->reg = *ep->reg & (USB_EPREG_MASK & ~USB_EP_CTR_RX) | USB_EP_CTR_TX;
		eprx[ep->hwep] = 0xff;
	}
	if(ep->pmatx != nil){
		pmafree(ep->pmatx - PMA, ep->maxpkt);
		ep->pmatab[2] = 0;
		ep->pmatx = nil;
	}
	if(ep->pmarx != nil){
		pmafree(ep->pmarx - PMA, rxround(ep->maxpkt));
		ep->pmatab[6] = 0;
		ep->pmarx = nil;
	}
}

void WEAK
usbdisconnect(int v)
{
}

void
usbphyinit(void)
{
	usbdisconnect(1);
	delayus(100);
	
	memset(eprx, 0xff, sizeof(eprx));
	memset(eptx, 0xff, sizeof(eptx));
	pmaallocbits = 0xff;
	
	periclk(PERUSB, 1);
	USB->CNTR = USB_CNTR_FRES;
	delayus(1);
	USB->CNTR = 0;
	USB->ISTR = 0;
	usbdisconnect(0);
	USB->CNTR = USB_CNTR_CTRM | USB_CNTR_PMAOVRM | USB_CNTR_ERRM | USB_CNTR_RESETM;
}

void
usbphyreset(void)
{
	usbdebug("USB reset\n");
	USB->ISTR = 0;
	USB->BTABLE = 0;
	USB->DADDR = 0x80;
	ep0state = EP0SETUP;
}

void
usbphysetaddr(u8int addr)
{
	USB->DADDR = 0x80 | addr;
}

static void
ep0stall(USBReq *req)
{
	usbdebug("USB stall\n");
	USB->EP0R = EP0RVAL| USB->EP0R & (USB_EPRX_STAT | USB_EPTX_STAT) ^ (USB_EP_RX_STALL | USB_EP_TX_STALL);
	ep0state = EP0STALL;
}

static void
ep0txkick(USBReq *req)
{
	int i, n;

	if((USB->EP0R & USB_EPTX_STAT) == USB_EP_TX_VALID){
		usbdebug("USB: tried to write to full FIFO\n");
		return;
	}
	n = req->len - req->ptr;
	if(n >= ep0.maxpkt) n = ep0.maxpkt;
	for(i = 0; i < n; i += 2)
		ep0.pmatx[i] = *(u16int*)(req->data + req->ptr + i);
	req->ptr += n;
	ep0.pmatab[2] = n;
	USB->EP0R = EP0RVAL | USB->EP0R & USB_EPTX_STAT ^ USB_EP_TX_VALID;
	if(n == ep0.maxpkt && req->ptr != req->wLength || req->ptr != req->len)
		ep0state = EP0TXDATA;
	else
		ep0state = EP0TXDATAL;
}

static void
ep0rxdata(USBReq *req)
{
	int i, n;

	n = ep0.pmatab[6] & 0x3ff;
	for(i = 0; i < n; i += 2)
		*(u16int*)(req->data + req->ptr + i) = ep0.pmarx[i];
	req->ptr += n;
	ep0.pmatab[6] &= ~0x3ff;
	if(n == ep0.maxpkt && req->ptr != req->wLength){
		USB->EP0R = EP0RVAL & ~USB_EP_CTR_RX | USB->EP0R & USB_EPRX_STAT ^ USB_EP_RX_VALID;
		ep0state = EP0RXDATA;
		return;
	}
	req->len = req->ptr;
	if(usbep0req(req) >= 0){
		ep0.pmatab[2] = 0;
		USB->EP0R = EP0RVAL & ~USB_EP_CTR_RX | USB->EP0R & USB_EPTX_STAT ^ USB_EP_TX_VALID;
		ep0state = EP0STATUS;
	}else{
		USB->EP0R = EP0RVAL & ~USB_EP_CTR_RX;
		ep0stall(req);
	}
}

static void
usbep0(void)
{
	static USBReq req;
	static uchar buf[64];
	int rc;

	if((USB->EP0R & USB_EP_CTR_TX) != 0){
		USB->EP0R = EP0RVAL & ~USB_EP_CTR_TX;
		switch(ep0state){
		case EP0TXDATA:
			ep0txkick(&req);
			break;
		case EP0TXDATAL:
			USB->EP0R = EP0RVAL | USB->EP0R & USB_EPRX_STAT ^ USB_EP_RX_VALID;
			ep0state = EP0STATUS;
			break;
		case EP0STATUS:
			ep0state = EP0SETUP;
			usbep0complete(&req);
			USB->EP0R = EP0RVAL | USB->EP0R & USB_EPRX_STAT ^ USB_EP_RX_VALID;
			break;
		default:
			usbdebug("USB: unexpected TX complete in state %d\n", ep0state);
		}
	}
	if((USB->EP0R & USB_EP_CTR_RX) == 0)
		return;
	if((USB->EP0R & USB_EP_SETUP) != 0){
		*(u16int*)&req.bmRequestType = ep0.pmarx[0];
		req.wValue = ep0.pmarx[2];
		req.wIndex = ep0.pmarx[4];
		req.wLength = ep0.pmarx[6];
		req.ptr = 0;
		req.len = 0;
		req.data = buf;
		USB->EP0R = EP0RVAL & ~USB_EP_CTR_RX;
		usbdebug("bmRequestType=%.2ux, bRequest=%.2ux, wValue=%.4ux, wIndex=%.4ux, wLength=%.4ux\n", req.bmRequestType, req.bRequest, req.wValue, req.wIndex, req.wLength);
		
		usbep0begin(&req);
		
		if(req.wLength == 0){
			if(usbep0req(&req) >= 0){
				ep0state = EP0STATUS;
				ep0.pmatab[2] = 0;
				USB->EP0R = EP0RVAL | USB->EP0R & USB_EPTX_STAT ^ USB_EP_TX_VALID;
			}else
				ep0stall(&req);
		}else if((req.bmRequestType & RTTOHOST) == 0){
			ep0state = EP0RXDATA;
			USB->EP0R = EP0RVAL & ~USB_EP_CTR_RX | USB->EP0R & USB_EPRX_STAT ^ USB_EP_RX_VALID;
		}else{
			rc = usbep0req(&req);
			if(rc >= 0){
				req.len = rc;
				ep0txkick(&req);
			}else
				ep0stall(&req);
		}	
	}else{
		switch(ep0state){
		case EP0STATUS:
			ep0state = EP0SETUP;
			usbep0complete(&req);
			USB->EP0R = EP0RVAL & ~USB_EP_CTR_RX | USB->EP0R & USB_EPRX_STAT ^ USB_EP_RX_VALID;
			break;
		case EP0RXDATA:
			ep0rxdata(&req);
			break;
		default:
			usbdebug("USB: unexpected RX complete in state %d\n", ep0state);
			USB->EP0R = EP0RVAL & ~USB_EP_CTR_RX;
		}
	}
}

void
usbirq(void)
{
	int i;
	volatile u16int *r;
	USBEp *ep;

	if((USB->ISTR & USB_ISTR_RESET) != 0){
		usbreset();
		return;
	}
	if((USB->ISTR & USB_ISTR_ERR) != 0){
		USB->ISTR = ~USB_ISTR_ERR;
		usbdebug("USB ERR interrupt\n");
	}
	if((USB->ISTR & USB_ISTR_PMAOVR) != 0){
		USB->ISTR = ~USB_ISTR_PMAOVR;
		usbdebug("USB PMA overflow\n");
	}
	if((USB->ISTR & USB_ISTR_CTR) != 0){
		if((USB->EP0R & (USB_EP_CTR_RX|USB_EP_CTR_TX)) != 0)
			usbep0();
		r = &USB->EP1R;
		for(i = 1; (USB->ISTR & USB_ISTR_CTR) != 0 && i < 8; i++, r += 2){
			if((*r & USB_EP_CTR_TX) != 0){
				*r = *r & (USB_EPREG_MASK & ~USB_EP_CTR_TX) | USB_EP_CTR_RX;
				if(eptx[i] == 0xff)
					usbdebug("USB: TX complete on unconfigured endpoint\n");
				else{
					ep = usbep + eptx[i];
					if(ep->act != nil)
						ep->act(ep);
				}
			}
			if((*r & USB_EP_CTR_RX) != 0){
				if(eprx[i] == 0xff){
					usbdebug("USB: RX complete on unconfigured endpoint\n");
					*r = *r & (USB_EPREG_MASK & ~USB_EP_CTR_RX) | USB_EP_CTR_TX;
				}else{
					ep = usbep + eprx[i];
					if(ep->act != nil)
						ep->act(ep);
					else
						*r = *r & (USB_EPREG_MASK & ~USB_EP_CTR_RX) | USB_EP_CTR_TX;
				}
			}
		}
	}
}

int
usbepsendnb(USBEp *ep, void *d, uint n)
{
	int s, i;
	
	s = splhi();
	if((ep->addr & EPDIR) != EPIN || (*ep->reg & USB_EPTX_STAT) == USB_EP_TX_VALID){
		splx(s);
		return -1;
	}
	if(n > ep->maxpkt)
		n = ep->maxpkt;
	for(i = 0; i < n; i += 2)
		ep->pmatx[i] = *(u16int*)((u8int*)d + i);
	ep->pmatab[2] = n;
	*ep->reg = *ep->reg & (USB_EPREG_MASK & ~USB_EP_CTR_TX | USB_EPTX_STAT) ^ USB_EP_TX_VALID | USB_EP_CTR_RX;
	splx(s);
	return n;
}

int
usbeprecvnb(USBEp *ep, void *d, uint n)
{
	int s, i;
	
	s = splhi();
	if((ep->addr & EPDIR) != EPOUT || (*ep->reg & USB_EP_CTR_RX) == 0){
		splx(s);
		return -1;
	}
	if(n > (ep->pmatab[6] & 0x3ff))
		n = ep->pmatab[6] & 0x3ff;
	for(i = 0; i + 1 < n; i += 2)
		*(u16int*)((u8int*)d + i) = ep->pmarx[i];
	if(i != n)
		*(u8int*)((u8int*)d + i) = ep->pmarx[i];
	ep->pmatab[6] &= ~0x3ff;
	*ep->reg = *ep->reg & (USB_EPREG_MASK & ~USB_EP_CTR_RX | USB_EPRX_STAT) ^ USB_EP_RX_VALID | USB_EP_CTR_TX;
	splx(s);
	return n;
}

#endif
