#include <u.h>
#include <libc.h>

void
cquclear(CQueue *q)
{
	int x;
	
	x = splhi();
	q->rd = q->wr = 0;
	splx(x);
}

int
cquwritenb(CQueue *q, void *d, uint n)
{
	int x;
	u16int s, w;
	int wasempty;
	
	x = splhi();
	s = q->rd + q->sz - q->wr;
	if(n > s) n = s;
	if(n == 0){
		splx(x);
		return 0;
	}
	wasempty = q->rd == q->wr;
	w = q->wr & q->sz - 1;
	if(w + n > q->sz){
		s = q->sz - w;
		memmove(q->d + w, d, s);
		memmove(q->d, (uchar*)d + s, n - s);
	}else
		memmove(q->d + w, d, n);
	q->wr += n;
	if(wasempty && q->fnrxkick != nil)
		q->fnrxkick(q);
	splx(x);
	return n;
}

int
cqureadnb(CQueue *q, void *d, uint n)
{
	u16int s, r;
	int wasfull;
	int x;
	
	x = splhi();
	s = q->wr - q->rd;
	if(n > s) n = s;
	if(n == 0){
		splx(x);
		return 0;
	}
	wasfull = (u16int)(q->rd + q->sz) == q->wr;
	r = q->rd & q->sz - 1;
	if(r + n > q->sz){
		s = q->sz - r;
		memmove(d, q->d + r, s);
		memmove((uchar*)d + s, q->d, n - s);
	}else
		memmove(d, q->d + r, n);
	q->rd += n;
	if(wasfull && q->fntxkick != nil)
		q->fntxkick(q);
	splx(x);
	return n;
}

int
cquwrite(CQueue *q, void *d, uint n)
{
	int rc;
	
	if(n == 0)
		return 0;
	while(rc = cquwritenb(q, d, n), rc == 0)
		wfi();
	return rc;
}

int
cquread(CQueue *q, void *d, uint n)
{
	int rc;
	
	if(n == 0)
		return 0;
	while(rc = cqureadnb(q, d, n), rc == 0)
		wfi();
	return rc;
}

int
cqugetcnb(CQueue *q)
{
	int x, rc, wasfull;
	
	x = splhi();
	if(q->wr == q->rd){
		splx(x);
		return -1;
	}
	wasfull = (u16int)(q->rd + q->sz) == q->wr;
	rc = q->d[q->rd++ & q->sz - 1];
	if(wasfull && q->fntxkick != nil)
		q->fntxkick(q);
	splx(x);
	return rc;
}

int
cqugetc(CQueue *q)
{
	int rc;
	
	while(rc = cqugetcnb(q), rc < 0)
		;
	return rc;
}

int
cquputcnb(CQueue *q, uchar c)
{
	int x;
	int wasempty;
	
	x = splhi();
	if((u16int)(q->rd + q->sz) == q->wr){
		splx(x);
		return -1;
	}
	wasempty = q->rd == q->wr;
	q->d[q->wr++ & q->sz - 1] = c;
	if(wasempty && q->fnrxkick != nil)
		q->fnrxkick(q);
	splx(x);
	return 1;
}

void
cquputc(CQueue *q, uchar c)
{
	while(cquputcnb(q, c) < 0)
		;
}

int
cqucanread(CQueue *q)
{
	int x;
	int rc;
	
	x = splhi();
	rc = (u16int)(q->wr - q->rd);
	splx(x);
	return rc;
}
