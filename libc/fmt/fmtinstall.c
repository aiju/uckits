#include <u.h>
#include <libc.h>
#include "fmtdef.h"

enum
{
	Maxfmt = 64
};

struct
{
	/* lock by calling _fmtlock, _fmtunlock */
	int	nfmt;
	Convfmt	fmt[Maxfmt];
} fmtalloc;

extern Convfmt knownfmt[];

/*
 * _fmtlock() must be set
 */
static int
_fmtinstall(int c, Fmts f)
{
	Convfmt *p, *ep;

	if(c<=0 || c>Runemax)
		return -1;
	if(!f)
		f = _badfmt;

	ep = &fmtalloc.fmt[fmtalloc.nfmt];
	for(p=fmtalloc.fmt; p<ep; p++)
		if(p->c == c)
			break;

	if(p == &fmtalloc.fmt[Maxfmt])
		return -1;

	p->fmt = f;
	if(p == ep){	/* installing a new format character */
		fmtalloc.nfmt++;
		p->c = c;
	}

	return 0;
}

int
fmtinstall(int c, Fmts f)
{
	int ret;

	_fmtlock();
	ret = _fmtinstall(c, f);
	_fmtunlock();
	return ret;
}

Fmts
fmtfmt(int c)
{
	Convfmt *p, *ep;

	ep = &fmtalloc.fmt[fmtalloc.nfmt];
	for(p=fmtalloc.fmt; p<ep; p++)
		if(p->c == c){
			while(p->fmt == nil)	/* loop until value is updated */
				;
			return p->fmt;
		}

	/* is this a predefined format char? */
	_fmtlock();
	for(p=knownfmt; p->c; p++)
		if(p->c == c){
			_fmtinstall(p->c, p->fmt);
			_fmtunlock();
			return p->fmt;
		}
	_fmtunlock();

	return _badfmt;
}
