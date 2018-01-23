#include <windows.h>
#include <winusb.h>
#include <setupapi.h>
#include <stdio.h>
#include "../usbif.h"
#include "dll.h"
#include "dat.h"
#include "fns.h"

static void
flushinput(WINUSB_INTERFACE_HANDLE wh)
{
	UCHAR buf[MAXPKT];
	ULONG tmo, len;
	
	printf("flushinput in\n");
	tmo = 100;
	sendcmd(CMD(CmdNOP), -1);
	WinUsb_SetPipePolicy(wh, 0x81, PIPE_TRANSFER_TIMEOUT, sizeof(tmo), &tmo);
	while(WinUsb_ReadPipe(wh, 0x81, buf, sizeof(buf), &len, NULL))
		;
	tmo = 0;
	WinUsb_SetPipePolicy(wh, 0x81, PIPE_TRANSFER_TIMEOUT, sizeof(tmo), &tmo);
	printf("flushinput out\n");
}

WINUSB_INTERFACE_HANDLE
usbhandle(void)
{
	static GUID guid = { 0x5118703a, 0xace0, 0x4545, 0xa0, 0xf0, 0x31, 0xf5, 0x7d, 0x27, 0x64, 0xe1};
	HDEVINFO di;
	SP_DEVINFO_DATA devinfo;
	SP_DEVICE_INTERFACE_DATA devif;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detail;
	LPTSTR path;
	ULONG len;
	HANDLE h;
	static WINUSB_INTERFACE_HANDLE wh;
	int i, rc;

	if(wh != NULL)
		return wh;
	di = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if(di == INVALID_HANDLE_VALUE)
		return NULL;
	devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
	path = NULL;
	detail = NULL;
	for(i = 0; SetupDiEnumDeviceInfo(di, i, &devinfo); i++){
		LocalFree(path);
		LocalFree(detail);
		devif.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
		rc = SetupDiEnumDeviceInterfaces(di, &devinfo, &guid, i, &devif);
		if(GetLastError() == ERROR_NO_MORE_ITEMS) break;
		if(!rc) goto done;
		rc = SetupDiGetDeviceInterfaceDetail(di, &devif, NULL, 0, &len, NULL);
		if(!rc){
			if(GetLastError() == ERROR_INSUFFICIENT_BUFFER && len > 0){
				detail = LocalAlloc(LPTR, len);
				if(!detail) goto done;
			}else
				goto done;
		}
		detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		rc = SetupDiGetDeviceInterfaceDetail(di, &devif, detail, len, NULL, &devinfo);
		if(!rc)
			goto done;
		
		path = LocalAlloc(LPTR, _tcslen(detail->DevicePath) + 1);
		_tcscpy(path, detail->DevicePath);
	}
	if(path == NULL)
		goto done;
	h = CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if(h == INVALID_HANDLE_VALUE)
		goto done;
	if(!WinUsb_Initialize(h, &wh))
		goto done;
	flushinput(wh);
done:
	LocalFree(path);
	LocalFree(detail);
	SetupDiDestroyDeviceInfoList(&devinfo);
	return wh;
}

int
sendraw(void *d, int len)
{
	WINUSB_INTERFACE_HANDLE wh;
	ULONG txd;

	wh = usbhandle();
	if(wh == NULL) return -1;
	if(!WinUsb_WritePipe(wh, 0x01, d, len, &txd, NULL)){
		printf("WinUSB_WritePipe: %d\n", (int)GetLastError());
		return -1;
	}
	return txd;
}

int
recvraw(void *d, int len, int tmo)
{
	WINUSB_INTERFACE_HANDLE wh;
	int n, m, err, rc;
	static UCHAR buf[MAXPKT];
	static int bufp;
	static ULONG bufl;
	ULONG tmout;
	int tmotab[18] = {0, 1, 1, 1, 1, 1, 3, 10, 30, 100, 300, 1000, 3000, 10000, 30000, 100000, 300000, 1000000};

	wh = usbhandle();
	if(wh == NULL) return -1;
	n = 0;
	while(n < len){
		if(bufp == bufl){
			tmout = tmotab[(unsigned)tmo <= 17 ? tmo : 0];
			if(tmout != 0)
				WinUsb_SetPipePolicy(wh, 0x81, PIPE_TRANSFER_TIMEOUT, sizeof(tmout), &tmout);
			rc = WinUsb_ReadPipe(wh, 0x81, buf, sizeof(buf), &bufl, NULL);
			err = GetLastError();
			if(tmout != 0){
				tmout = 0;
				WinUsb_SetPipePolicy(wh, 0x81, PIPE_TRANSFER_TIMEOUT, sizeof(tmout), &tmout);
			}
			if(!rc){
				if(err == ERROR_SEM_TIMEOUT)
					flushinput(wh);
				SetLastError(err);
				bufl = bufp = 0;
				break;
			}
			bufp = 0;
			if(bufl == 0) break;
		}
		m = len - n;
		if(m > bufl - bufp) m = bufl - bufp;
		memmove((UCHAR*)d + n, buf + bufp, m);
		bufp += m;
		n += m;
	}
	return n;
}

int
sendesc(void *d, int len, int eot, int eos)
{
	UCHAR buf[MAXPKT];
	UCHAR *c;
	int bp;
	
	bp = 0;
	for(c = d; c < (UCHAR*)d + len; c++){
		if(bp + (*c == ChESC || *c == ChEOI || eot && c+1 == (UCHAR*)d+len) == MAXPKT){
			if(sendraw(buf, bp) < 0)
				return -1;
			bp = 0;
		}
		if(eot && c+1 == (UCHAR*)d+len || (eos & 0x1800) == 0x800 && (*c & 0x7f) == (eos & 0x7f) || (eos & 0x1800) == 0x1800 && (*c & 0xff) == (eos & 0xff))
			buf[bp++] = ChEOI;
		else if(*c == ChESC || *c == ChEOI)
			buf[bp++] = ChESC;
		buf[bp++] = *c;
	}
	if(bp > 0 && sendraw(buf, bp) < 0)
		return -1;
	return 0;
}

int
recvesc(void *d, int n, int eot, int eos, int *reason, int tmo)
{
	int i, rc, goteot, goteos, goterr;
	UCHAR c;
	
	goteot = 0;
	goteos = 0;
	goterr = 0;
	for(i = 0; i < n && (!eot || !goteot) && !goteos; i++){
		rc = recvraw(&c, 1, tmo);
		if(rc == 0) {goterr = 1; break;}
		goteot = c == ChEOI;
		if(c == ChESC || c == ChEOI){
			rc = recvraw(&c, 1, tmo);
			if(rc == 0) {goterr = 1; break;}
		}
		goteos = (eos & 0x1400) == 0x400 && (c & 0x7f) == (eos & 0x7f) || (eos & 0x1400) == 0x1400 && (c & 0xff) == (eos & 0xff);
		((UCHAR*)d)[i] = c;
	}
	if(reason != NULL)
		*reason = goteot | goteos << 1 | goterr << 2;
	return i;
}

int
sendcmd(int c0, ...)
{
	va_list va;
	char buf[256];
	int i, bp, fi, x;

	if(c0 < 0) return 0;
	bp = 0;
	for(i = 0, fi = 0;;){
		if(fi == 0){
			x = c0;
			va_start(va, c0);
			fi = 1;
		}else
			x = va_arg(va, int);
		if(x < 0) break;
		if((x & 0x100) != 0) continue;
		if(bp + 1 >= sizeof(buf)){
			if(sendraw(buf, bp) < 0)
				return -1;
			bp = 0;
		}
		switch(x){
		case CmdATN1L: case CmdATN1T: gibsta = tls()->ibsta &= ~ATN; break;
		case CmdATN0: gibsta = tls()->ibsta |= ATN; break;
		}
		if(x == ChESC || x == ChEOI || (x & 0x200) != 0)
			buf[bp++] = ChESC;
		buf[bp++] = x;
	}
	if(bp > 0 && sendraw(buf, bp) < 0)
		return -1;
	if(fi)
		va_end(va);
	return i;
}

int
getstatus(short *r)
{
	UCHAR x;

	if(sendcmd(CMD(CmdStatus), -1) < 0) return error(EDVR);
	if(recvesc(&x, 1, 0, 0, NULL, 0) != 1) return error(EDVR);
	*r = x << 8 | 0xff;
	return 0;
}

int
checkaddr(int pad, int sad)
{
	short r;

	printf("checkaddr(%d, %d)\n", pad, sad);
	if(sendcmd(CMD(CmdATN0), UNL, TALK | mypad, SAD(mysad), LISTEN | pad, SAD(sad), CMD(CmdATN1T), -1) < 0)
		return -1;
	if(getstatus(&r) < 0)
		return -1;
	return r >> 9 & 1;
}
