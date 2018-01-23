#include <windows.h>
#include <winusb.h>
#include <setupapi.h>
#include <stdio.h>
#include "../usbif.h"
#include "dll.h"
#include "dat.h"
#include "fns.h"
#include "md5.h"

static DWORD tlsidx;

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

Tls *
tls(void)
{
	Tls *t;
	ULONG err;
	
	err = GetLastError();
	t = TlsGetValue(tlsidx);
	if(t == NULL){
		t = LocalAlloc(LPTR, sizeof(Tls));
		TlsSetValue(tlsidx, t);
	}
	SetLastError(err);
	return t;
}

int gibsta, giberr, gibcnt;

long
error(int n)
{
	giberr = n;
	tls()->iberr = n;
	return gibsta = tls()->ibsta = tls()->ibsta & ~TIMO | ERR | (n == EABO ? TIMO : 0);
}

long
setibsta(void)
{
	return gibsta = tls()->ibsta &= ATN|END;
}

long EXPORT __stdcall ThreadIbsta (void) { return tls()->ibsta; }
long EXPORT __stdcall ThreadIberr (void) { return tls()->iberr; }
long EXPORT __stdcall ThreadIbcnt (void) { return tls()->ibcnt; }

long EXPORT __stdcall Ibsta (void) { return gibsta; }
long EXPORT __stdcall Iberr (void) { return giberr; }
long EXPORT __stdcall Ibcnt (void) { return gibcnt; }

long EXPORT __stdcall
ibexpert(int ud, int option, void * Input, void * Output)
{
	switch(option){
	case 14:
	{
		/* NI, go home, you're drunk */

		DWORD fplist[] = {(DWORD)ibwrtfW, (DWORD)ibwrta, (DWORD)ibwrt, (DWORD)ibwait, (DWORD)ibtrg, (DWORD)ibstop, (DWORD)ibsic, (DWORD)ibrsp, (DWORD)ibrpp, (DWORD)ibrdfW, (DWORD)ibrda, (DWORD)ibrd, (DWORD)ibppc, (DWORD)ibpct, (DWORD)ibonl, (DWORD)ibnotify, (DWORD)ibloc, (DWORD)ibln, (DWORD)iblines, (DWORD)iblck, (DWORD)ibgts, (DWORD)ibfindW, (DWORD)ibdev, (DWORD)ibconfig, (DWORD)ibcmda, (DWORD)ibcmd, (DWORD)ibclr, (DWORD)ibcac, (DWORD)ibask};
		md5_state_t state;
		UCHAR dig[16];

		md5_init(&state);
		md5_append(&state, (void *) fplist, sizeof(fplist));
		md5_finish(&state, dig);
		md5_init(&state);
		md5_append(&state, dig, 16);
		md5_append(&state, Input, 4);
		md5_finish(&state, Output);
		return setibsta();
	}
	default:
		printf("ibexpert called with option=%d, Input=%p, Output=%p\n", option, Input, Output);
		return error(EDVR);
	}
}
