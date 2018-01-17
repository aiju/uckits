#include <u.h>
#include <libc.h>
#include <libio.h>
#include <usb.h>

static void (*poststatus)(USBReq *);
static int curconfig;

enum {
	Get_Status = 0,
	Clear_Feature = 1,
	Set_Feature = 3,
	Set_Address = 5,
	Get_Descriptor = 6,
	Set_Descriptor = 7,
	Get_Configuration = 8,
	Set_Configuration = 9,
	Get_Interface = 10,
	Set_Interface = 11,
	Synch_Frame = 12,
};

void
usbep0reset(void)
{
	poststatus = nil;
	curconfig = 0;
}

static void
usb_ep0_Set_Address_Complete(USBReq *r)
{
	usbphysetaddr(r->wValue);
}

int WEAK
usb_ep0_Set_Address(USBReq *r)
{
	if(r->bmRequestType != 0 || r->wIndex != 0 || r->wLength != 0 || r->wValue > 127)
		return -1;
	usbdebug("Set_Address(%d)\n", r->wValue);
	poststatus = usb_ep0_Set_Address_Complete;
	return 0;
}

int WEAK
usb_ep0_Get_Descriptor(USBReq *r)
{
	int dlen;
	USBDesc *p;
	extern USBDesc *usbdesctab;
	
	if(r->bmRequestType != 0x80)
		return -1;
	usbdebug("Get_Descriptor(%#.4ux)\n", r->wValue);
	for(p = usbdesctab; p->data != nil; p++)
		if(p->idx == r->wValue)
			break;
	if(p->data == nil)
		return -1;
	dlen = p->len;
	if(r->wLength < dlen) dlen = r->wLength;
	memcpy(r->data, p->data, dlen);
	return dlen;
}

int WEAK
usb_ep0_Get_Configuration(USBReq *r)
{
	if(r->bmRequestType != 0x80 || r->wValue != 0 || r->wIndex != 0 || r->wLength != 1)
		return -1;
	usbdebug("Get_Configuration()\n");
	r->data[0] = curconfig;
	return 1;
}

int WEAK
usb_ep0_Set_Configuration(USBReq *r)
{
	if(r->bmRequestType != 0x00 || r->wIndex != 0 || r->wLength != 0)
		return -1;
	usbdebug("Set_Configuration(%d)\n", r->wValue);
	if(curconfig == r->wValue)
		return 0;
	if(usbconfig(r->wValue) < 0)
		return -1;
	curconfig = r->wValue;
	return 0;
}

int WEAK
usb_ep0_Get_Interface(USBReq *r)
{
	if(r->bmRequestType != 0x81 || r->wValue != 0 || r->wLength != 1)
		return -1;
	usbdebug("Get_Interface(%d)\n", r->wIndex);
	r->data[0] = 0;
	return 1;
}

int WEAK
usb_ep0_Set_Interface(USBReq *r)
{
	if(r->bmRequestType != 0x01 || r->wLength != 0)
		return -1;
	usbdebug("Set_Interface(%d, %d)\n", r->wIndex, r->wValue);
	if(r->wValue != 0)
		return -1;
	return 0;
}

int WEAK
usb_ep0_Get_Status(USBReq *r)
{
	if(r->wValue != 0 || r->wLength != 2)
		return -1;
	usbdebug("Get_Status(%d, %d)\n", r->bmRequestType & 3, r->wIndex);
	r->data[0] = 0;
	r->data[1] = 0;
	switch(r->bmRequestType){
	case 0x80: break;
	case 0x81: break;
	case 0x82: break;
	default: return -1;
	}
	return 2;
}

int WEAK
usb_ep0_Clear_Feature(USBReq *r)
{
	return -1;
}

int WEAK
usb_ep0_Set_Feature(USBReq *r)
{
	return -1;
}

int WEAK
usb_ep0_Synch_Frame(USBReq *r)
{
	return -1;
}

static ROM int (*ep0stdhandler[])(USBReq*) = {
	[Get_Status] usb_ep0_Get_Status,
	[Clear_Feature] usb_ep0_Clear_Feature,
	[Set_Feature] usb_ep0_Set_Feature,
	[Set_Address] usb_ep0_Set_Address,
	[Get_Descriptor] usb_ep0_Get_Descriptor,
	[Get_Configuration] usb_ep0_Get_Configuration,
	[Set_Configuration] usb_ep0_Set_Configuration,
	[Get_Interface] usb_ep0_Get_Interface,
	[Set_Interface] usb_ep0_Set_Interface,
	[Synch_Frame] usb_ep0_Synch_Frame,
};

int WEAK
usbvendreq(USBReq *r)
{
	return -1;
}

int WEAK
usbep0req(USBReq *r)
{
	switch(r->bmRequestType & RTTYPE){
	case RTSTANDARD:
		if(r->bRequest >= nelem(ep0stdhandler) || ep0stdhandler[r->bRequest] == nil)
			return -1;
		return ep0stdhandler[r->bRequest](r);
	case RTVENDOR:
		return usbvendreq(r);
	default:
		return -1;
	}
}

void
usbep0begin(USBReq *req)
{
	poststatus = nil;
}

void
usbep0complete(USBReq *req)
{
	if(poststatus != nil){
		poststatus(req);
		poststatus = nil;
	}
}
