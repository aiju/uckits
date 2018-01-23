#include <windows.h>
#include <winusb.h>
#include <setupapi.h>
#include <stdio.h>
#include "../usbif.h"
#include "dll.h"
#include "dat.h"
#include "fns.h"

void EXPORT __stdcall AllSpoll      (int boardID, short * addrlist, short* results) NOPEV

void EXPORT __stdcall
DevClear(int boardID, short addr)
{
	if(boardID != 0){
		error(EHDL);
		return;
	}
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
	if(boardID != 0){
		error(EHDL);
		return;
	}
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

void EXPORT __stdcall
EnableLocal(int boardID, short *al)
{
	if(boardID != 0){
		error(EHDL);
		return;
	}
	if(*al == NOADDR){
		if(sendcmd(CMD(CmdREN1), -1) < 0){
			error(EDVR);
			return;
		}
		setibsta();
		return;
	}
	for(; *al != NOADDR; al++){
		if(sendcmd(CMD(CmdATN0), UNL, LISTEN|GetPAD(*al), SAD(GetSAD(*al)), GTL, UNL, -1) < 0){
			error(EDVR);
			return;
		}
	}
	setibsta();
}

void EXPORT __stdcall
EnableRemote(int boardID, short *al)
{
	if(boardID != 0){
		error(EHDL);
		return;
	}
	if(sendcmd(CMD(CmdATN0), CMD(UNL), CMD(CmdREN0), -1) < 0) { error(EDVR); return; }
	for(; *al != NOADDR; al++)
		if(sendcmd(LISTEN|GetPAD(*al), SAD(GetSAD(*al)), -1) < 0){
			error(EDVR);
			return;
		}
	setibsta();
}

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

void EXPORT __stdcall
RcvRespMsg(int boardID, void* buffer, long cnt, int Termination)
{
	int rc, eos, reason;

	if(boardID != 0) {error(EHDL); return;}
	if(Termination == STOPend)
		eos = 0;
	else
		eos = TermEOS | EOS8 | (UCHAR)Termination;
	if(sendcmd(CMD(CmdATN1L), CMD(CmdReadEOS), (UCHAR)eos, (UCHAR)(eos >> 8), CMD(CmdReadEOI1), CMD(CmdReadN), (UCHAR)cnt, (UCHAR)(cnt >> 8), (UCHAR)(cnt >> 16), (UCHAR)(cnt >> 24), -1) < 0) {error(EDVR); return;}
	rc = recvesc(buffer, cnt, 1, eos, &reason, fdtab[0].tmo);
	gibcnt = tls()->ibcnt = rc;
	if((reason & 4) != 0){
		if(GetLastError() == ERROR_SEM_TIMEOUT)
			error(EABO);
		else
			error(EDVR);
		return;
	}
	setibsta();	
}

void EXPORT __stdcall
ReadStatusByte(int boardID, short addr, short* result)
{
	UCHAR x;

	if(boardID != 0) {error(EHDL); return;}
	if(sendcmd(CMD(CmdATN0), UNL, LISTEN|mypad, SAD(mysad), SPE, TALK|GetPAD(addr), SAD(GetSAD(addr)), CMD(CmdATN1L), CMD(CmdReadEOS), 0, 0, CMD(CmdReadEOI0), CMD(CmdReadN), 1, 0, 0, 0, -1) < 0){
		error(EDVR);
		return;
	}
	if(recvesc(&x, 1, 0, 0, NULL, 0) < 0){
		error(EDVR);
		return;
	}
	sendcmd(CMD(CmdATN0), SPD, UNL, UNT, -1);
	if(result != NULL)
		*result = x;
	setibsta();
}

void EXPORT __stdcall
ReceiveSetup(int boardID, short addr)
{
	if(boardID != 0) {error(EHDL); return;}
	if(sendcmd(CMD(CmdATN0), UNL, LISTEN|mypad, SAD(mysad), TALK|GetPAD(addr), SAD(GetSAD(addr)), CMD(CmdATN1L), -1) < 0){
		error(EDVR);
		return;
	}
	setibsta();	
}

void EXPORT __stdcall
Receive(int boardID, short addr, void* buffer, long cnt, int Termination)
{
	ReceiveSetup(boardID, addr);
	if((tls()->ibsta & ERR) == 0)
		RcvRespMsg(boardID, buffer, cnt, Termination);
}

void EXPORT __stdcall
SendCmds(int boardID, void* buffer, long cnt)
{
	if(boardID != 0){
		error(EHDL);
		return;
	}
	if(sendcmd(CMD(CmdATN0), -1) < 0){
		error(EDVR);
		return;
	}
	if(sendesc(buffer, cnt, 0, 0) < 0){
		error(EDVR);
		return;
	}
	gibcnt = tls()->ibcnt = cnt;
	setibsta();
}

void EXPORT __stdcall
SendIFC(int boardID)
{	
	if(boardID != 0){ error(EHDL); return; }
	if(sendcmd(CMD(CmdATN0), CMD(CmdIFC), -1) < 0){
		error(EDVR);
		return;
	}
	setibsta();
}

void EXPORT __stdcall
SendLLO(int boardID)
{
	if(boardID != 0){
		error(EHDL);
		return;
	}
	if(sendcmd(CMD(CmdATN0), LLO, -1) < 0){
		error(EDVR);
		return;
	}
	setibsta();
}

void EXPORT __stdcall
SendSetup(int boardID, short * addrlist)
{
	char buf[512];
	char *p;
	short *q;
	
	if(boardID != 0){ error(EHDL); return; }
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
	if(sendcmd(CMD(CmdATN0), -1) < 0) goto err;
	if(sendesc(buf, p - buf, 0, 0) < 0) goto err;
	if(sendcmd(CMD(CmdATN1T), -1) < 0) goto err;
	setibsta();
	return;
err:
	gibcnt = tls()->ibcnt = 0;
	error(EDVR);
}

void EXPORT __stdcall
SendDataBytes(int boardID, void* buffer, long cnt, int eot_mode)
{
	if(boardID != 0){
		error(EHDL);
		return;
	}
	switch(eot_mode){
	case NULLend: case NLend: break;
	case DABend:
		if(cnt != 0)
			break;
	default:
		error(EARG);
		return;
	}
	if(sendesc(buffer, cnt, eot_mode == DABend, 0) < 0){
		error(EDVR);
		return;
	}
	if(eot_mode == NLend && sendesc("\n", 1, 1, 0) < 0){
		error(EDVR);
		return;
	}
	gibcnt = tls()->ibcnt = cnt;
	setibsta();
	
}

void EXPORT __stdcall
SendList(int boardID, short * addrlist, void* databuf, long datacnt, int eotMode)
{
	SendSetup(boardID, addrlist);
	if((tls()->ibsta & ERR) == 0)
		SendDataBytes(boardID, databuf, datacnt, eotMode);
}

void EXPORT __stdcall
Send(int boardID, short addr, void* databuf, long datacnt, int eotMode)
{
	short al[2] = {addr, NOADDR};

	SendSetup(boardID, al);
	if((tls()->ibsta & ERR) == 0)
		SendDataBytes(boardID, databuf, datacnt, eotMode);
}

void EXPORT __stdcall SetRWLS       (int boardID, short * addrlist) NOPEV

void EXPORT __stdcall
TestSRQ(int boardID, short* result)
{
	short x;

	if(boardID != 0){
		error(EHDL);
		return;
	}
	if(getstatus(&x) < 0){
		error(EDVR);
		return;
	}
	if(result != NULL)
		*result = (x & BusSRQ) != 0;
	setibsta();
}

void EXPORT __stdcall
TestSys(int boardID, short * addrlist, short* results)
{
	int i, fail;
	char res[32];

	SendList(boardID, addrlist, "*TST?\n", 6, DABend);
	if((tls()->ibsta & ERR) != 0)
		return;
	fail = 0;
	for(i = 0; addrlist[i] != NOADDR; i++){
		results[i] = -1;
		Receive(boardID, addrlist[i], res, sizeof(res)-1, '\n');
		if((tls()->ibsta & ERR) != 0){
			gibcnt = tls()->ibcnt = i;
			return;
		}
		res[tls()->ibcnt] = 0;
		results[i] = atoi(res);
		if(strcmp(res, "0\n") != 0)
			fail++;
	}
	gibcnt = tls()->ibcnt = fail;
	setibsta();
}

void EXPORT __stdcall
TriggerList(int boardID, short *al)
{
	if(boardID != 0){
		error(EHDL);
		return;
	}
	if(*al != NOADDR){
		if(sendcmd(CMD(CmdATN0), CMD(UNL), -1) < 0) { error(EDVR); return; }
		for(; *al != NOADDR; al++)
			if(sendcmd(LISTEN|GetPAD(*al), SAD(GetSAD(*al)), -1) < 0){
				error(EDVR);
				return;
			}
	}else
		if(sendcmd(CMD(CmdATN0), -1) < 0) { error(EDVR); return; }
	if(sendcmd(GET, -1) < 0) { error(EDVR); return; }
	setibsta();
}

void EXPORT __stdcall
Trigger(int boardID, short addr)
{
	short al[2] = {addr, NOADDR};
	
	TriggerList(boardID, al);
}

void EXPORT __stdcall WaitSRQ       (int boardID, short* result) NOPEV

void EXPORT __stdcall
ResetSys(int boardID, short * addrlist)
{
	if(boardID != 0){
		error(EHDL);
		return;
	}
	if(sendcmd(CMD(CmdATN0), CMD(CmdREN0), CMD(CmdIFC), -1) < 0){
		error(EDVR);
		return;
	}
	if(*addrlist == NOADDR){
		setibsta();
		return;
	}
	DevClearList(boardID, addrlist);
	if((tls()->ibsta & ERR) != 0)
		return;
	SendList(boardID, addrlist, "*RST\n", 5, DABend);
}
