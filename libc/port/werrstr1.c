#include <u.h>
#include <libc.h>

void
werrstr1(char *str)
{
	strecpy(errbuf, errbuf + ERRMAX, str);
}
