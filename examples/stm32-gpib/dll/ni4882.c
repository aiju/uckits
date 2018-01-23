#include <windows.h>
#include <winusb.h>
#include <setupapi.h>
#include <stdio.h>
#include "usbif.h"

#define EXPORT __declspec(dllexport)

#define UNL  0x3f  /* GPIB unlisten command                 */
#define UNT  0x5f  /* GPIB untalk command                   */
#define GTL  0x01  /* GPIB go to local                      */
#define SDC  0x04  /* GPIB selected device clear            */
#define PPC  0x05  /* GPIB parallel poll configure          */
#define GET  0x08  /* GPIB group execute trigger            */
#define TCT  0x09  /* GPIB take control                     */
#define LLO  0x11  /* GPIB local lock out                   */
#define DCL  0x14  /* GPIB device clear                     */
#define PPU  0x15  /* GPIB parallel poll unconfigure        */
#define SPE  0x18  /* GPIB serial poll enable               */
#define SPD  0x19  /* GPIB serial poll disable              */
#define PPE  0x60  /* GPIB parallel poll enable             */
#define PPD  0x70  /* GPIB parallel poll disable            */

#define LISTEN 0x20
#define TALK 0x40
#define SAD(x) ((((UCHAR)(x))-1 & 0xff)+1)
#define CMD(x) (0x200|(x))

/* GPIB status bit vector :                                 */
/*       global variable ibsta and wait mask                */

#define ERR     (1<<15) /* Error detected                   */
#define TIMO    (1<<14) /* Timeout                          */
#define END     (1<<13) /* EOI or EOS detected              */
#define SRQI    (1<<12) /* SRQ detected by CIC              */
#define RQS     (1<<11) /* Device needs service             */
#define CMPL    (1<<8)  /* I/O completed                    */
#define LOK     (1<<7)  /* Local lockout state              */
#define REM     (1<<6)  /* Remote state                     */
#define CIC     (1<<5)  /* Controller-in-Charge             */
#define ATN     (1<<4)  /* Attention asserted               */
#define TACS    (1<<3)  /* Talker active                    */
#define LACS    (1<<2)  /* Listener active                  */
#define DTAS    (1<<1)  /* Device trigger state             */
#define DCAS    (1<<0)  /* Device clear state               */

/* Error messages returned in global variable iberr         */

#define EDVR  0  /* System error                            */
#define ECIC  1  /* Function requires GPIB board to be CIC  */
#define ENOL  2  /* Write function detected no Listeners    */
#define EADR  3  /* Interface board not addressed correctly */
#define EARG  4  /* Invalid argument to function call       */
#define ESAC  5  /* Function requires GPIB board to be SAC  */
#define EABO  6  /* I/O operation aborted                   */
#define ENEB  7  /* Non-existent interface board            */
#define EDMA  8  /* Error performing DMA                    */
#define EOIP 10  /* I/O operation started before previous   */
                 /* operation completed                     */
#define ECAP 11  /* No capability for intended operation    */
#define EFSO 12  /* File system operation error             */
#define EBUS 14  /* Command error during device call        */
#define ESTB 15  /* Serial poll status byte lost            */
#define ESRQ 16  /* SRQ remains asserted                    */
#define ETAB 20  /* The return buffer is full.              */
#define ELCK 21  /* Address or board is locked.             */
#define EARM 22  /* The ibnotify Callback failed to rearm   */
#define EHDL 23  /* The input handle is invalid             */
#define EWIP 26  /* Wait already in progress on input ud    */
#define ERST 27  /* The event notification was cancelled    */
                 /* due to a reset of the interface         */
#define EPWR 28  /* The system or board has lost power or   */
                 /* gone to standby                         */

/* Warning messages returned in global variable iberr       */

#define WCFG 24  /* Configuration warning                   */
#define ECFG WCFG

/* EOS mode bits                                            */

#define BIN  (1<<12) /* Eight bit compare                   */
#define XEOS (1<<11) /* Send END with EOS byte              */
#define REOS (1<<10) /* Terminate read on EOS               */

/* Timeout values and meanings                              */

#define TNONE    0   /* Infinite timeout (disabled)         */
#define T10us    1   /* Timeout of 10 us (ideal)            */
#define T30us    2   /* Timeout of 30 us (ideal)            */
#define T100us   3   /* Timeout of 100 us (ideal)           */
#define T300us   4   /* Timeout of 300 us (ideal)           */
#define T1ms     5   /* Timeout of 1 ms (ideal)             */
#define T3ms     6   /* Timeout of 3 ms (ideal)             */
#define T10ms    7   /* Timeout of 10 ms (ideal)            */
#define T30ms    8   /* Timeout of 30 ms (ideal)            */
#define T100ms   9   /* Timeout of 100 ms (ideal)           */
#define T300ms  10   /* Timeout of 300 ms (ideal)           */
#define T1s     11   /* Timeout of 1 s (ideal)              */
#define T3s     12   /* Timeout of 3 s (ideal)              */
#define T10s    13   /* Timeout of 10 s (ideal)             */
#define T30s    14   /* Timeout of 30 s (ideal)             */
#define T100s   15   /* Timeout of 100 s (ideal)            */
#define T300s   16   /* Timeout of 300 s (ideal)            */
#define T1000s  17   /* Timeout of 1000 s (ideal)           */

/*  IBLN Constants                                          */
#define NO_SAD   0
#define ALL_SAD -1

/*  The following constants are used for the second parameter of the
 *  ibconfig function.  They are the "option" selection codes.
 */
#define  IbcPAD        0x0001      /* Primary Address                      */
#define  IbcSAD        0x0002      /* Secondary Address                    */
#define  IbcTMO        0x0003      /* Timeout Value                        */
#define  IbcEOT        0x0004      /* Send EOI with last data byte?        */
#define  IbcPPC        0x0005      /* Parallel Poll Configure              */
#define  IbcREADDR     0x0006      /* Repeat Addressing                    */
#define  IbcAUTOPOLL   0x0007      /* Disable Auto Serial Polling          */
#define  IbcCICPROT    0x0008      /* Use the CIC Protocol?                */
#define  IbcIRQ        0x0009      /* Use PIO for I/O                      */
#define  IbcSC         0x000A      /* Board is System Controller?          */
#define  IbcSRE        0x000B      /* Assert SRE on device calls?          */
#define  IbcEOSrd      0x000C      /* Terminate reads on EOS               */
#define  IbcEOSwrt     0x000D      /* Send EOI with EOS character          */
#define  IbcEOScmp     0x000E      /* Use 7 or 8-bit EOS compare           */
#define  IbcEOSchar    0x000F      /* The EOS character.                   */
#define  IbcPP2        0x0010      /* Use Parallel Poll Mode 2.            */
#define  IbcTIMING     0x0011      /* NORMAL, HIGH, or VERY_HIGH timing.   */
#define  IbcDMA        0x0012      /* Use DMA for I/O                      */
#define  IbcReadAdjust 0x0013      /* Swap bytes during an ibrd.           */
#define  IbcWriteAdjust 0x014      /* Swap bytes during an ibwrt.          */
#define  IbcSendLLO    0x0017      /* Enable/disable the sending of LLO.      */
#define  IbcSPollTime  0x0018      /* Set the timeout value for serial polls. */
#define  IbcPPollTime  0x0019      /* Set the parallel poll length period.    */
#define  IbcEndBitIsNormal 0x001A  /* Remove EOS from END bit of IBSTA.       */
#define  IbcUnAddr         0x001B  /* Enable/disable device unaddressing.     */
#define  IbcSignalNumber   0x001C  /* Set UNIX signal number - unsupported */
#define  IbcBlockIfLocked  0x001D  /* Enable/disable blocking for locked boards/devices */
#define  IbcHSCableLength  0x001F  /* Length of cable specified for high speed timing.*/
#define  IbcIst        0x0020      /* Set the IST bit.                     */
#define  IbcRsv        0x0021      /* Set the RSV byte.                    */
#define  IbcLON        0x0022      /* Enter listen only mode               */

/*
 *    Constants that can be used (in addition to the ibconfig constants)
 *    when calling the ibask() function.
 */

#define  IbaPAD            IbcPAD
#define  IbaSAD            IbcSAD
#define  IbaTMO            IbcTMO
#define  IbaEOT            IbcEOT
#define  IbaPPC            IbcPPC
#define  IbaREADDR         IbcREADDR
#define  IbaAUTOPOLL       IbcAUTOPOLL
#define  IbaCICPROT        IbcCICPROT
#define  IbaIRQ            IbcIRQ
#define  IbaSC             IbcSC
#define  IbaSRE            IbcSRE
#define  IbaEOSrd          IbcEOSrd
#define  IbaEOSwrt         IbcEOSwrt
#define  IbaEOScmp         IbcEOScmp
#define  IbaEOSchar        IbcEOSchar
#define  IbaPP2            IbcPP2
#define  IbaTIMING         IbcTIMING
#define  IbaDMA            IbcDMA
#define  IbaReadAdjust     IbcReadAdjust
#define  IbaWriteAdjust    IbcWriteAdjust
#define  IbaSendLLO        IbcSendLLO
#define  IbaSPollTime      IbcSPollTime
#define  IbaPPollTime      IbcPPollTime
#define  IbaEndBitIsNormal IbcEndBitIsNormal
#define  IbaUnAddr         IbcUnAddr
#define  IbaSignalNumber   IbcSignalNumber
#define  IbaBlockIfLocked  IbcBlockIfLocked
#define  IbaHSCableLength  IbcHSCableLength
#define  IbaIst            IbcIst
#define  IbaRsv            IbcRsv
#define  IbaLON            IbcLON
#define  IbaSerialNumber   0x0023

#define  IbaBNA            0x0200   /* A device's access board. */


/* Values used by the Send 488.2 command. */
#define  NULLend 0x00  /* Do nothing at the end of a transfer.*/
#define  NLend   0x01  /* Send NL with EOI after a transfer.  */
#define  DABend  0x02  /* Send EOI with the last DAB.         */

/* Value used by the 488.2 Receive command.
 */
#define  STOPend     0x0100


/* Address type (for 488.2 calls) */

//typedef short Addr4882_t; /* System dependent: must be 16 bits */

/*
 *  This macro can be used to easily create an entry in address list
 *  that is required by many of the 488.2 functions. The primary address goes in the
 *  lower 8-bits and the secondary address goes in the upper 8-bits.
 */
#define  MakeAddr(pad, sad)   ((short)(((pad)&0xFF) | ((sad)<<8)))

/*
 *  This value is used to terminate an address list.  It should be
 *  assigned to the last entry.
 */
#ifndef NOADDR
#define NOADDR    (short)((unsigned short)0xFFFF)
#endif

/*
 *  The following two macros are used to "break apart" an address list
 *  entry.  They take an unsigned integer and return either the primary
 *  or secondary address stored in the integer.
 */
#define  GetPAD(val)    ((val) & 0xFF)
#define  GetSAD(val)    (((val) >> 8) & 0xFF)

/* iblines constants */

#define  ValidEOI   (short)0x0080
#define  ValidATN   (short)0x0040
#define  ValidSRQ   (short)0x0020
#define  ValidREN   (short)0x0010
#define  ValidIFC   (short)0x0008
#define  ValidNRFD  (short)0x0004
#define  ValidNDAC  (short)0x0002
#define  ValidDAV   (short)0x0001
#define  BusEOI     (short)0x8000
#define  BusATN     (short)0x4000
#define  BusSRQ     (short)0x2000
#define  BusREN     (short)0x1000
#define  BusIFC     (short)0x0800
#define  BusNRFD    (short)0x0400
#define  BusNDAC    (short)0x0200
#define  BusDAV     (short)0x0100

static DWORD tlsidx;

typedef struct Tls {
	long ibsta;
	long iberr;
	long ibcnt;
} Tls;

enum {
	DESCVALID = 1,
	DESCBOARD = 1,
};
typedef struct Desc {
	int flags;
	UCHAR pad, sad;
	
} Desc;

enum { NDESC = 64 };

static Desc fdtab[NDESC] = {
	[0] = { .flags = DESCVALID|DESCBOARD }
};

static int mypad, mysad;

static void
flushinput(WINUSB_INTERFACE_HANDLE wh)
{
	UCHAR buf[MAXPKT];
	ULONG len;
	OVERLAPPED ov;
	HANDLE ev;
	
	ev = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(ev == NULL) return;
	for(;;){
		memset(&ov, 0, sizeof(ov));
		ov.hEvent = ev;
		if(!WinUsb_ReadPipe(wh, 0x81, buf, sizeof(buf), &len, &ov))
			break;
		WaitForSingleObject(ev, 100);
		if(!WinUsb_GetOverlappedResult(wh, &ov, &len, FALSE))
			break;
	}
	WinUsb_AbortPipe(wh, 0x81);
	CloseHandle(ev);
}

static WINUSB_INTERFACE_HANDLE
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

BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	void *v;

	switch(fdwReason){
	case DLL_PROCESS_ATTACH:
		if((tlsidx = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			return FALSE;
		break;
	case DLL_THREAD_DETACH:
		v = TlsGetValue(tlsidx);
		if(v != NULL)
			LocalFree(v);
		break;
	case DLL_PROCESS_DETACH:
		v = TlsGetValue(tlsidx);
		if(v != NULL)
			LocalFree(v);
		TlsFree(tlsidx);
		break;
	}
	return TRUE;
}

static Tls *
tls(void)
{
	Tls *t;
	
	t = TlsGetValue(tlsidx);
	if(t == NULL){
		t = LocalAlloc(LPTR, sizeof(Tls));
		TlsSetValue(tlsidx, t);
	}
	return t;
}

static int gibsta, giberr, gibcnt;

static long
error(int n)
{
	giberr = n;
	tls()->iberr = n;
	return gibsta = tls()->ibsta |= ERR;
}

static long
setibsta(void)
{
	return gibsta = tls()->ibsta = 0;
}

static int
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

static int
recvraw(void *d, int len)
{
	WINUSB_INTERFACE_HANDLE wh;
	int n, m;
	static UCHAR buf[MAXPKT];
	static int bufp;
	static ULONG bufl;

	wh = usbhandle();
	if(wh == NULL) return -1;
	n = 0;
	while(n < len){
		if(bufp == bufl){
			if(!WinUsb_ReadPipe(wh, 0x81, buf, sizeof(buf), &bufl, NULL)){
				printf("WinUSB_ReadPipe: %d\n", (int)GetLastError());
				return -1;
			}
			if(bufl == 0) break;
			bufp = 0;
		}
		m = len - n;
		if(m > bufl - bufp) m = bufl - bufp;
		memmove((UCHAR*)d + n, buf + bufp, m);
		bufp += m;
		n += m;
	}
	return n;
}

static int
sendesc(void *d, int len, int eot)
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
		if(eot && c+1 == (UCHAR*)d+len)
			buf[bp++] = ChEOI;
		else if(*c == ChESC || *c == ChEOI)
			buf[bp++] = ChESC;
		buf[bp++] = *c;
	}
	if(bp > 0 && sendraw(buf, bp) < 0)
		return -1;
	return 0;
}

static int
recvesc(void *d, int n, int eot)
{
	int i, rc, goteot;
	UCHAR c;
	
	goteot = 0;
	for(i = 0; i < n && (!eot || !goteot); i++){
		rc = recvraw(&c, 1);
		if(rc < 0) return -1;
		if(rc == 0) break;
		goteot = c == ChEOI;
		if(c == ChESC || c == ChEOI){
			rc = recvraw(&c, 1);
			if(rc < 0) return -1;
			if(rc == 0) break;
		}
		((UCHAR*)d)[i] = c;
	}
	return n;
}

static int
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

static int
getstatus(short *r)
{
	UCHAR x;

	if(sendcmd(CMD(CmdStatus), -1) < 0) return error(EDVR);
	if(recvesc(&x, 1, 0) != 1) return error(EDVR);
	*r = x << 8 | 0xff;
	return 0;
}

static int
checkaddr(int pad, int sad)
{
	short r;

	if(sendcmd(CMD(CmdATN0), UNL, TALK | mypad, SAD(mysad), LISTEN | pad, SAD(sad), CMD(CmdATN1T), -1) < 0)
		return -1;
	if(getstatus(&r) < 0)
		return -1;
	return r >> 9 & 1;
}

/****
 **** typedef for ibnotify callback ****
 ****/
typedef int (__stdcall * GpibNotifyCallback_t)(int, int, int, long, void*);

#define NOPE { error(ECAP); return tls()->ibsta; }
#define NOPEV { error(ECAP); }

int EXPORT __stdcall ibfindA   (char* udname) NOPE 
long EXPORT __stdcall ibbnaA    (int ud, char* udname) NOPE 
long EXPORT __stdcall ibrdfA    (int ud, char* filename) NOPE 
long EXPORT __stdcall ibwrtfA   (int ud, char* filename) NOPE 

int EXPORT __stdcall ibfindW   (wchar_t* udname) NOPE 
long EXPORT __stdcall ibbnaW    (int ud, wchar_t* udname) NOPE 
long EXPORT __stdcall ibrdfW    (int ud, wchar_t* filename) NOPE 
long EXPORT __stdcall ibwrtfW   (int ud, wchar_t* filename) NOPE 

long EXPORT __stdcall ibask    (int ud, int option, int* v) NOPE 
long EXPORT __stdcall ibcac    (int ud, int v) NOPE 
long EXPORT __stdcall ibclr    (int ud) NOPE 

long EXPORT __stdcall
ibcmd(int ud, void* buf, long cnt)
{
	if(ud != 0) return error(EDVR);
	if(sendcmd(CMD(CmdATN0), -1) < 0) return error(EDVR);
	if(sendesc(buf, cnt, 0) < 0) return error(EDVR);
	gibcnt = tls()->ibcnt = cnt;
	return setibsta();	
}

long EXPORT __stdcall ibcmda   (int ud, void* buf, long cnt) NOPE 
long EXPORT __stdcall ibconfig (int ud, int option, int v) NOPE 
int EXPORT __stdcall ibdev    (int boardID, int pad, int sad, int tmo, int eot, int eos) NOPE 
long EXPORT __stdcall ibdiag   (int ud, void* buf, long cnt) NOPE 
long EXPORT __stdcall ibexpert (int ud, int option, void * Input, void * Output) NOPE 
long EXPORT __stdcall ibgts    (int ud, int v) NOPE 
long EXPORT __stdcall iblck    (int ud, int v, unsigned int LockWaitTime, void * Reserved) NOPE 

long EXPORT __stdcall
iblines(int ud, short* result)
{
	if(ud != 0) return error(EDVR);
	if(getstatus(result) < 0) return error(EDVR);
	return setibsta();	
}

long EXPORT __stdcall
ibln(int ud, int pad, int sad, short* listen)
{
	int rc;

	if(sad == ALL_SAD){
		for(sad = 96; sad < 127; sad++){
			rc = checkaddr(pad, sad);
			if(rc < 0) return error(EDVR);
			if(rc > 0){
				*listen = 1;
				return setibsta();
			}
		}
		*listen = 0;
		return setibsta();
	}
	if((unsigned)pad >= 31 || sad != 0 && ((sad & ~31) != 96 || sad == 127)) return error(EARG);
	rc = checkaddr(pad, sad);
	if(rc < 0) return error(EDVR);
	*listen = rc;
	return setibsta();
}

long EXPORT __stdcall ibloc    (int ud) NOPE 
long EXPORT __stdcall ibnotify (int ud, int mask, GpibNotifyCallback_t Callback, void* RefData) NOPE 
long EXPORT __stdcall ibonl    (int ud, int v) NOPE 
long EXPORT __stdcall ibpct    (int ud) NOPE 
long EXPORT __stdcall ibppc    (int ud, int v) NOPE 

long EXPORT __stdcall
ibrd(int ud, void* v, long cnt)
{
	int rc;
	
	if(sendcmd(CMD(CmdATN1L), CMD(CmdReadEOI1), CMD(CmdReadN), (UCHAR)cnt, (UCHAR)(cnt >> 8), (UCHAR)(cnt >> 16), (UCHAR)(cnt >> 24), -1) < 0) return error(EDVR);
	rc = recvesc(v, cnt, 1);
	if(rc < 0) return error(EDVR);
	gibcnt = tls()->ibcnt = rc;
	return setibsta();	
}

long EXPORT __stdcall ibrda    (int ud, void* buf, long cnt) NOPE 
long EXPORT __stdcall ibrpp    (int ud, char* ppr) NOPE 
long EXPORT __stdcall ibrsp    (int ud, char* spr) NOPE  
long EXPORT __stdcall ibsic    (int ud) NOPE 
long EXPORT __stdcall ibsre    (int ud, int v) NOPE 
long EXPORT __stdcall ibstop   (int ud) NOPE 
long EXPORT __stdcall ibtrg    (int ud) NOPE 
long EXPORT __stdcall ibwait   (int ud, int mask) NOPE

long EXPORT __stdcall
ibwrt(int ud, void* buf, long cnt)
{
	if(ud != 0) return error(EDVR);
	if(sendcmd(CMD(CmdATN1T), -1) < 0) return error(EDVR);
	if(sendesc(buf, cnt, 1) < 0) return error(EDVR);
	gibcnt = tls()->ibcnt = cnt;
	return setibsta();	
}

long EXPORT __stdcall ibwrta   (int ud, void* buf, long cnt) NOPE 

/**************************************************************************/
/*  Functions to access Thread-Specific copies of the GPIB global vars */

long EXPORT __stdcall ThreadIbsta (void) { return tls()->ibsta; }
long EXPORT __stdcall ThreadIberr (void) { return tls()->iberr; }
long EXPORT __stdcall ThreadIbcnt (void) { return tls()->ibcnt; }

/**************************************************************************/
/*  NI-488.2 Function Prototypes  */

void EXPORT __stdcall AllSpoll      (int boardID, short * addrlist, short* results) NOPEV

void EXPORT __stdcall
DevClear(int boardID, short addr)
{
	if(addr == NOADDR){
		if(sendcmd(CMD(CmdATN0), DCL, -1) < 0) {error(EDVR); return;}
		setibsta();
		return;
	}
	if(sendcmd(CMD(CmdATN0), UNL, TALK|mypad, SAD(mysad), LISTEN|GetPAD(addr), SAD(GetSAD(addr)), SDC, -1) < 0){
		error(EDVR);
		return;
	}
	setibsta();
}

void EXPORT __stdcall
DevClearList(int boardID, short *al)
{
	if(*al == NOADDR){
		DevClear(boardID, NOADDR);
		return;
	}
	for(; *al != NOADDR; al++){
		DevClear(boardID, *al);
		if((tls()->ibsta & ERR) != 0)
			return;
	}
}

void EXPORT __stdcall EnableLocal   (int boardID, short * addrlist) NOPEV
void EXPORT __stdcall EnableRemote  (int boardID, short * addrlist) NOPEV

void EXPORT __stdcall
FindLstn(int boardID, short * addrlist, short * results, int limit)
{
	short *p, *q;
	int i;
	int rc;
	
	if(limit <= 0){
		gibcnt = tls()->ibcnt = 0;
		setibsta();
		return;
	}
	q = results;
	for(p = addrlist; *p != NOADDR; p++){
		rc = checkaddr(GetPAD(*p), NO_SAD);
		if(rc < 0){
			error(EDVR);
			return;
		}
		if(rc > 0){
			*q++ = GetPAD(*p);
			if(q - results == limit)
				break;
			continue;
		}
		for(i = 96; i < 126; i++){
			rc = checkaddr(GetPAD(*p), i);
			if(rc < 0){
				error(EDVR);
				return;
			}
			if(rc > 0){
				*q++ = GetPAD(*p) | i << 8;
				if(q - results == limit)
					goto out;
				continue;
			}
		}
	}
out:
	gibcnt = tls()->ibcnt = q - results;
	setibsta();
}

void EXPORT __stdcall FindRQS       (int boardID, short * addrlist, short* dev_stat) NOPEV
void EXPORT __stdcall PPoll         (int boardID, short* result) NOPEV
void EXPORT __stdcall PPollConfig   (int boardID, short addr, int dataLine, int lineSense) NOPEV
void EXPORT __stdcall PPollUnconfig (int boardID, short * addrlist) NOPEV
void EXPORT __stdcall PassControl   (int boardID, short addr) NOPEV
void EXPORT __stdcall RcvRespMsg    (int boardID, void* buffer, long cnt, int Termination) NOPEV
void EXPORT __stdcall ReadStatusByte(int boardID, short addr, short* result) NOPEV

void EXPORT __stdcall
Receive(int boardID, short addr, void* buffer, long cnt, int Termination)
{
	if(sendcmd(CMD(CmdATN0), UNL, LISTEN|mypad, SAD(mysad), TALK|GetPAD(addr), SAD(GetSAD(addr)), CMD(CmdATN1L), -1) < 0){
		error(EDVR);
		return;
	}
	ibrd(0, buffer, cnt);
}

void EXPORT __stdcall ReceiveSetup  (int boardID, short addr) NOPEV
void EXPORT __stdcall ResetSys      (int boardID, short * addrlist) NOPEV
void EXPORT __stdcall Send          (int boardID, short addr, void* databuf, long datacnt, int eotMode) NOPEV
void EXPORT __stdcall SendCmds      (int boardID, void* buffer, long cnt) NOPEV
void EXPORT __stdcall SendDataBytes (int boardID, void* buffer, long cnt, int eot_mode) NOPEV

void EXPORT __stdcall
SendIFC(int boardID)
{	
	if(boardID != 0){ error(EDVR); return; }
	if(sendcmd(CMD(CmdATN0), CMD(CmdIFC), -1) < 0){
		error(EDVR);
		return;
	}
	setibsta();
}

void EXPORT __stdcall SendLLO       (int boardID) NOPEV

void EXPORT __stdcall
SendList(int boardID, short * addrlist, void* databuf, long datacnt, int eotMode)
{
	char buf[512];
	char *p;
	short *q;
	
	p = buf;
	*p++ = 0x3f;
	*p++ = 0x40;
	for(q = addrlist; *q != NOADDR; q++){
		if(p + 2 > buf + sizeof(buf)){
			error(EARG);
			return;
		}
		*p++ = 0x20 | GetPAD(*q);
		if(GetSAD(*q) != 0)
			*p++ = GetSAD(*q);
	}
	ibcmd(0, buf, p - buf);
	if((tls()->ibsta & ERR) != 0){
		gibcnt = tls()->ibcnt = 0;
		return;
	}
	ibwrt(0, databuf, datacnt);
}

void EXPORT __stdcall SendSetup     (int boardID, short * addrlist) NOPEV
void EXPORT __stdcall SetRWLS       (int boardID, short * addrlist) NOPEV
void EXPORT __stdcall TestSRQ       (int boardID, short* result) NOPEV
void EXPORT __stdcall TestSys       (int boardID, short * addrlist, short* results) NOPEV
void EXPORT __stdcall Trigger       (int boardID, short addr) NOPEV
void EXPORT __stdcall TriggerList   (int boardID, short * addrlist) NOPEV
void EXPORT __stdcall WaitSRQ       (int boardID, short* result) NOPEV
