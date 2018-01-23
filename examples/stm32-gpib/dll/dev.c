#include <windows.h>
#include <winusb.h>
#include <setupapi.h>
#include <stdio.h>
#include "../usbif.h"
#include "dll.h"
#include "dat.h"
#include "fns.h"

Desc fdtab[NDESC] = {
	[0] = { .flags = DESCVALID|DESCBOARD|DESCSENDEOI|DESCNORMEND }
};

Desc *
getdesc(int ud, int brd)
{
	if(ud < 0 || ud >= NDESC || (fdtab[ud].flags & DESCVALID) == 0)
		return NULL;
	if((fdtab[ud].flags & DESCBOARD) == 0 && (brd & DEVOK) == 0)
		return NULL;
	if((fdtab[ud].flags & DESCBOARD) != 0 && (brd & BRDOK) == 0)
		return NULL;
	return &fdtab[ud];
}

int EXPORT __stdcall
ibdev(int boardID, int pad, int sad, int tmo, int eot, int eos)
{
	Desc *p;

	if(boardID != 0){
	edvr:
		error(EHDL);
		return -1;
	}
	if(pad < 0 || pad >= 31 || sad != 0 && (sad < 96 || sad >= 127)){
	earg:
		error(EARG);
		return -1;
	}
	if(tmo > T1000s) goto earg;
	for(p = fdtab; p < fdtab + NDESC; p++)
		if((p->flags & DESCVALID) == 0)
			break;
	if(p == fdtab + NDESC) goto edvr;
	p->flags = DESCVALID;
	if(eot) p->flags |= DESCSENDEOI;
	p->pad = pad;   
	p->sad = sad;
	p->tmo = tmo;
	p->eos = eos;
	setibsta();
	return p - fdtab;
}

long EXPORT __stdcall
ibask(int ud, int option, int* v)
{
	Desc *d;
	
	d = getdesc(ud, DEVOK|BRDOK);
	if(d == NULL)
		return error(EHDL);
	if((d->flags & DESCBOARD) != 0)
		switch(option){
		case IbaAUTOPOLL: *v = 0; break;
		case IbaCICPROT: *v = 0; break;
		case IbaDMA: *v = 1; break;
		case IbaEndBitIsNormal: *v = (d->flags & DESCNORMEND) != 0; break;
		case IbaEOSchar: *v = d->eos & 0xff; break;
		case IbaEOScmp: *v = (d->eos & EOS8) != 0; break;
		case IbaEOSrd: *v = (d->eos & TermEOS) != 0; break;
		case IbaEOSwrt: *v = (d->eos & SetEOI) != 0; break;
		case IbaEOT: *v = (d->flags & DESCSENDEOI) != 0; break;
		case IbaHSCableLength: *v = 0; break;
		case IbaIst: *v = (d->flags & DESCIST) != 0; break;
		case IbaPAD: *v = d->pad; break;
		case IbaPP2: *v = (d->flags & DESCPP2) != 0; break;
		case IbaPPC: *v = d->ppc; break;
		case IbaPPollTime: *v = 0; break;
		case IbaReadAdjust: *v = 0; break;
		case IbaRsv: *v = d->rsv; break;
		case IbaSAD: *v = d->sad; break;
		case IbaSC: *v = 1; break;
		case IbaSendLLO: *v = (d->flags & DESCLLO) != 0; break;
		case IbaSRE: *v = (d->flags & DESCSRE) != 0; break;
		case IbaTIMING: *v = 1; break;
		case IbaTMO: *v = d->tmo; break;
		case IbaWriteAdjust: *v = 0; break;
		default: return error(ECAP);
		}
	else
		switch(option){
		case IbaBNA: *v = 0; break;
		case IbaEOSchar: *v = d->eos & 0xff; break;
		case IbaEOScmp: *v = (d->eos & EOS8) != 0; break;
		case IbaEOSrd: *v = (d->eos & TermEOS) != 0; break;
		case IbaEOSwrt: *v = (d->eos & SetEOI) != 0; break;
		case IbaEOT: *v = (d->flags & DESCSENDEOI) != 0; break;
		case IbaPAD: *v = d->pad; break;
		case IbaReadAdjust: *v = 0; break;
		case IbaREADDR: *v = (d->flags & DESCREADDR) != 0; break;
		case IbaSAD: *v = d->sad; break;
		case IbaSPollTime: *v = 0; break;
		case IbaTMO: *v = d->tmo; break;
		case IbaUnAddr: *v = (d->flags & DESCUNADDR) != 0; break;
		case IbaWriteAdjust: *v = 0; break;
		default: return error(ECAP);
		}
	return setibsta();
}

long EXPORT __stdcall
ibconfig (int ud, int option, int v)
{
	Desc *d;
	int prev;
	#define FLAG(x) prev = (d->flags & x) != 0; if(v) d->flags |= x; else d->flags &= ~x
	#define EOSFLAG(x) prev = (d->eos & x) != 0; if(v) d->eos |= x; else d->eos &= ~x
	
	d = getdesc(ud, DEVOK|BRDOK);
	if(d == NULL)
		return error(EHDL);
	if((d->flags & DESCBOARD) != 0)
		switch(option){
		case IbcAUTOPOLL: prev = 0; if(v != 0); return error(ECAP); break;
		case IbcCICPROT: prev = 0; if(v != 0); return error(ECAP); break;
		case IbcDMA: prev = 1; break;
		case IbcEndBitIsNormal: FLAG(DESCNORMEND); break;
		case IbcEOSchar: prev = d->eos & 0xff; d->eos = d->eos & 0xff00 | v & 0xff; break;
		case IbcEOScmp: EOSFLAG(EOS8); break;
		case IbcEOSrd: EOSFLAG(TermEOS); break;
		case IbcEOSwrt: EOSFLAG(SetEOI); break;
		case IbcEOT: FLAG(DESCSENDEOI); break;
		case IbcHSCableLength: prev = 0; if(v != 0) return error(ECAP); break;
		case IbcIst: FLAG(DESCIST); break;
		case IbcPAD: prev = d->pad; if(v < 0 || v > 31) return error(EARG); d->pad = v; break;
		case IbcPP2: FLAG(DESCPP2); break;
		case IbcPPC: prev = d->ppc; d->ppc = v; break;
		case IbcPPollTime: prev = 0; break;
		case IbcReadAdjust: prev = 0; if(v != 0) return error(ECAP); break;
		case IbcRsv: prev = d->rsv; d->rsv = v; break;
		case IbcSAD: prev = d->sad; if(v != 0 && (v < 96 || v >= 127)) return error(EARG); d->sad = v; break;
		case IbcSC: prev = 1; if(!v) return error(ECAP); break;
		case IbcSendLLO: FLAG(DESCLLO); break;
		case IbcSRE: FLAG(DESCSRE); break;
		case IbcTIMING: prev = 1; break;
		case IbcTMO: prev = d->tmo; if(v < 0 || v > T1000s) return error(EARG); d->tmo = v; break;
		case IbcWriteAdjust: prev = 0; if(v != 0) return error(ECAP); break;
		default: return error(EARG);
		}
	else
		switch(option){
		case IbcEOSchar: prev = d->eos & 0xff; d->eos = d->eos & 0xff00 | v & 0xff; break;
		case IbcEOScmp: EOSFLAG(EOS8); break;
		case IbcEOSrd: EOSFLAG(TermEOS); break;
		case IbcEOSwrt: EOSFLAG(SetEOI); break;
		case IbcEOT: FLAG(DESCSENDEOI); break;
		case IbcPAD: prev = d->pad; if(v < 0 || v > 31) return error(EARG); d->pad = v; break;
		case IbcReadAdjust: prev = 0; if(v != 0) return error(ECAP); break;
		case IbcREADDR: FLAG(DESCREADDR); break;
		case IbcSAD: prev = d->sad; if(v != 0 && (v < 96 || v >= 127)) return error(EARG); d->sad = v; break;
		case IbcSPollTime: prev = 0; break;
		case IbcTMO: prev = d->tmo; if(v < 0 || v > T1000s) return error(EARG); d->tmo = v; break;
		case IbcUnAddr: FLAG(DESCUNADDR); break;
		case IbcWriteAdjust: prev = 0; if(v != 0) return error(ECAP); break;
		default: return error(EARG);
		}
	
	giberr = tls()->iberr = prev;
	return setibsta();
	#undef FLAG
	#undef EOSFLAG
}

long EXPORT __stdcall
ibonl(int ud, int v)
{
	Desc *d;
	
	d = getdesc(ud, DEVOK|BRDOK);
	if(d == NULL)
		return error(EHDL);
	if(v){
		d->flags = d->flags & (DESCVALID|DESCBOARD) | DESCSENDEOI | DESCNORMEND;
		d->tmo = 0;
		d->eos = 0;
	}else if(ud != 0)
		d->flags = 0;
	else
		ibsic(0);
	return setibsta();
}


long EXPORT __stdcall
ibbnaA(int ud, char* udname) 
{
	if(getdesc(ud, DEVOK) == NULL || strcmp(udname, "GPIB0") != 0)
		return error(EARG);
	return setibsta();
}

long EXPORT __stdcall
ibbnaW(int ud, wchar_t* udname) 
{
	if(getdesc(ud, DEVOK) == NULL || wcscmp(udname, L"GPIB0") != 0)
		return error(EARG);
	return setibsta();
}

int EXPORT __stdcall
ibfindA(char* udname)
{
	if(stricmp(udname, "GPIB0") != 0){
		error(EDVR);
		return -1;
	}
	if((ibonl(0, 1) & ERR) != 0)
		return -1;
	return 0;
}

int EXPORT __stdcall
ibfindW(wchar_t* udname)
{
	if(wcsicmp(udname, L"GPIB0") != 0){
		error(EDVR);
		return -1;
	}
	if((ibonl(0, 1) & ERR) != 0)
		return -1;
	return 0;
}

long EXPORT __stdcall
ibclr(int ud)
{
	Desc *d;
	
	d = getdesc(ud, DEVOK);
	if(d == NULL) return error(EHDL);
	if(sendcmd(CMD(CmdATN0), UNL, UNT, LISTEN|d->pad, SAD(d->sad), SDC, UNL, -1) < 0) return error(EDVR);
	return setibsta();
}

long EXPORT __stdcall
ibloc(int ud)
{
	Desc *d;

	d = getdesc(ud, BRDOK|DEVOK);
	if(d == NULL) return error(EHDL);
	if((d->flags & DESCBOARD) == 0)
		if(sendcmd(CMD(CmdATN0), UNL, UNT, LISTEN|d->pad, SAD(d->sad), GTL, UNL, -1) < 0)
			return error(EDVR);
	return setibsta();
}

long EXPORT __stdcall
ibtrg(int ud)
{

	Desc *d;

	d = getdesc(ud, DEVOK);
	if(d == NULL) return error(EHDL);
	if(sendcmd(CMD(CmdATN0), UNL, UNT, LISTEN|d->pad, SAD(d->sad), GET, UNL, -1) < 0)
		return error(EDVR);
	return setibsta();
}

long EXPORT __stdcall
ibrd(int ud, void* v, long cnt)
{
	Desc *d;
	int rc, reason;

	d = getdesc(ud, BRDOK|DEVOK);
	if(d == NULL) return error(EHDL);
	if((d->flags & DESCBOARD) == 0)
		if(sendcmd(CMD(CmdATN0), UNL, LISTEN|mypad, SAD(mysad), TALK|d->pad, SAD(d->sad), -1) < 0)
			return error(EDVR);	
	if(sendcmd(CMD(CmdATN1L), CMD(CmdReadEOI1), CMD(CmdReadEOS), (UCHAR)(d->eos), (UCHAR)(d->eos >> 8), CMD(CmdReadN), (UCHAR)cnt, (UCHAR)(cnt >> 8), (UCHAR)(cnt >> 16), (UCHAR)(cnt >> 24), -1) < 0) return error(EDVR);
	rc = recvesc(v, cnt, 1, d->eos, &reason, d->tmo);
	gibcnt = tls()->ibcnt = rc;	
	if((reason & 4) != 0)
		if(GetLastError() == ERROR_SEM_TIMEOUT)
			return error(EABO);
		else
			return error(EDVR);
	if((reason & 1) != 0 || (reason & 2) != 0 && (fdtab[0].flags & DESCNORMEND) != 0)
		gibsta = tls()->ibsta |= END;
	else
		gibsta = tls()->ibsta &= ~END;
	return setibsta();	
}

long EXPORT __stdcall
ibwrt(int ud, void* buf, long cnt)
{
	Desc *d;

	d = getdesc(ud, BRDOK|DEVOK);
	if(d == NULL) return error(EHDL);
	if((d->flags & DESCBOARD) == 0)
		if(sendcmd(CMD(CmdATN0), UNL, LISTEN|d->pad, SAD(d->sad), TALK|mypad, SAD(mysad), -1) < 0)
			return error(EDVR);	
	if(sendcmd(CMD(CmdATN1T), -1) < 0) return error(EDVR);
	if(sendesc(buf, cnt, (d->flags & DESCSENDEOI) != 0, d->eos) < 0) return error(EDVR);
	gibcnt = tls()->ibcnt = cnt;
	return setibsta();	
}
