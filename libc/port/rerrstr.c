#include <u.h>
#include <libc.h>

void
rerrstr(char *buf, uint nbuf)
{
	utfecpy(buf, buf+nbuf, errbuf);
}
