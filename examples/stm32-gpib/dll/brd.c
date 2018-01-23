#include <windows.h>
#include <winusb.h>
#include <setupapi.h>
#include <stdio.h>
#include "../usbif.h"
#include "dll.h"
#include "dat.h"
#include "fns.h"


long EXPORT __stdcall ibrdfA    (int ud, char* filename) NOPE 
long EXPORT __stdcall ibwrtfA   (int ud, char* filename) NOPE 

long EXPORT __stdcall ibrdfW    (int ud, wchar_t* filename) NOPE 
long EXPORT __stdcall ibwrtfW   (int ud, wchar_t* filename) NOPE 

long EXPORT __stdcall
ibcac(int ud, int v)
{
	if(getdesc(ud, BRDOK) == NULL) return error(EHDL);
	if(sendcmd(CMD(CmdATN0), -1) < 0) return error(EDVR);
	return setibsta();
}

long EXPORT __stdcall
ibcmd(int ud, void* buf, long cnt)
{
	if(getdesc(ud, BRDOK) == NULL) return error(EHDL);
	if(sendcmd(CMD(CmdATN0), -1) < 0) return error(EDVR);
	if(sendesc(buf, cnt, 0, 0) < 0) return error(EDVR);
	gibcnt = tls()->ibcnt = cnt;
	return setibsta();	
}

long EXPORT __stdcall ibcmda   (int ud, void* buf, long cnt) NOPE 
long EXPORT __stdcall ibdiag   (int ud, void* buf, long cnt) NOPE 

long EXPORT __stdcall iblck    (int ud, int v, unsigned int LockWaitTime, void * Reserved) NOPE 

long EXPORT __stdcall
ibgts(int ud, int v)
{
	if(getdesc(ud, BRDOK) == NULL) return error(EHDL);
	if(v != 0) return error(ECAP);
	if(sendcmd(CMD(CmdATN1T), -1) < 0) return error(EDVR);
	return setibsta();
}

long EXPORT __stdcall
iblines(int ud, short* result)
{
	if(getdesc(ud, BRDOK) == NULL) return error(EHDL);
	if(getstatus(result) < 0) return error(EDVR);
	return setibsta();	
}

long EXPORT __stdcall
ibln(int ud, int pad, int sad, short* listen)
{
	int rc;

	if(getdesc(ud, BRDOK) == NULL) return error(EHDL);
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
	if((unsigned)pad >= 31 || sad != 0 && (sad < 96 || sad >= 127)) return error(EARG);
	rc = checkaddr(pad, sad);
	if(rc < 0) return error(EDVR);
	*listen = rc;
	return setibsta();
}

long EXPORT __stdcall ibnotify (int ud, int mask, GpibNotifyCallback_t Callback, void* RefData) NOPE 
long EXPORT __stdcall ibpct    (int ud) NOPE 
long EXPORT __stdcall ibppc    (int ud, int v) NOPE 

long EXPORT __stdcall ibrda    (int ud, void* buf, long cnt) NOPE 
long EXPORT __stdcall ibrpp    (int ud, char* ppr) NOPE 
long EXPORT __stdcall ibrsp    (int ud, char* spr) NOPE  

long EXPORT __stdcall
ibsic(int ud)
{
	if(getdesc(ud, BRDOK) == NULL) return error(EHDL);
	if(sendcmd(CMD(CmdATN0), CMD(CmdIFC), -1) < 0) return error(EDVR);
	return setibsta();
}

long EXPORT __stdcall ibsre    (int ud, int v) NOPE 
long EXPORT __stdcall ibstop   (int ud) NOPE 
long EXPORT __stdcall ibwait   (int ud, int mask) NOPE

long EXPORT __stdcall ibwrta   (int ud, void* buf, long cnt) NOPE 
