#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>

void
_libioinit(void)
{
	SCB->CCR |= SCB_CCR_STKALIGN_Msk;
}
