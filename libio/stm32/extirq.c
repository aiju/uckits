#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>

#ifdef AFIO
#define exticr AFIO->EXTICR
#else
#define exticr SYSCFG->EXTICR
#endif
#if defined(EXTI_RTSR1_TR0_Pos) || defined(EXTI_RTSR1_RT0_Pos)
#define rtsr EXTI->RTSR1
#define ftsr EXTI->FTSR1
#else
#define rtsr EXTI->RTSR
#define ftsr EXTI->FTSR
#endif
#ifdef EXTI_D1
#define imr EXTI_D1->IMR1
#define pr EXTI_D1->PR1
#elif defined(EXTI_IMR1_IM0_Pos)
#define imr EXTI->IMR1
#define pr EXTI->PR1
#else
#define imr EXTI->IMR
#define pr EXTI->PR
#endif

int
extirqcfg(int line, int pin, int trig)
{
	int r, sh, s;

	if(line < 0){
		if(pin < 0)
			return -1;
		line = pin & 15;
		if(((rtsr | ftsr) & 1<<line) != 0)
			return -1;
	}
	if(pin < 0){
		s = splhi();
		rtsr &= ~(1<<line);
		ftsr &= ~(1<<line);
		pr = 1<<line;
		splx(s);
		return line;
	}
	if(line != (pin & 15))
		return -1;
	r = pin >> 2 & 3;
	sh = (pin & 3) << 2;
	s = splhi();
	exticr[r] = exticr[r] & ~(0xf << sh) | (pin >> 4 & 0xf) << sh;
	if((trig & EXTIRQRISE) != 0)
		rtsr |= 1<<line;
	else
		rtsr &= ~(1<<line);
	if((trig & EXTIRQFALL) != 0)
		ftsr |= 1<<line;
	else
		ftsr &= ~(1<<line);
	pr = 1<<line;
	splx(s);
	return line;
}

int
extirqen(int line, int en)
{
	int old;

	old = imr >> line & 1;
	if(en > 0)
		imr |= 1<<line;
	else
		imr &= ~(1<<line);
	return old;
}

void
extirqack(int line)
{
	if(line < 0)
		pr = -1;
	else
		pr = 1<<line;
}
