#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>

#ifdef GPIO_CRL_MODE

void
gpiocfg(int n, int f)
{
	GPIO_TypeDef *gpio;
	int ctl, g;
	
	gpio = peribase(PERGPIOA + (n >> 4));
	if(gpio == nil)
		return;
	n &= 15;
	g = 0;
	if((f & GPIOOUT) != 0){
		switch(f & GPIOSPEEDMsk){
		case GPIOSPEED0: ctl = 2; break;
		case GPIOSPEED1: ctl = 1; break;
		case GPIOSPEED2: ctl = 3; break;
		}
		if((f & GPIOOPDRAIN) != 0)
			ctl |= 4;
		if((f & GPIOALT) != 0)
			ctl |= 8;
	}else if((f & GPIOANALOG) != 0)
		ctl = 0;
	else if((f & (GPIOPULLUP|GPIOPULLDN)) != 0){
		ctl = 8;
		g = f & (GPIOPULLUP|GPIOPULLDN);
	}else
		ctl = 4;
	if(n >= 8)
		gpio->CRH = gpio->CRH & ~(0xf << (n & 7) * 4) | ctl << (n & 7) * 4;
	else
		gpio->CRL = gpio->CRL & ~(0xf << (n & 7) * 4) | ctl << (n & 7) * 4;
	if((g & GPIOPULLUP) != 0)
		gpio->ODR |= 1<<n;
	else if((g & GPIOPULLDN) != 0)
		gpio->ODR &= ~(1<<n);
}

#endif

#if defined(STM32H743xx) || defined(STM32H753xx)
#else
void
gpioset(int n, int v)
{
	v = (v & 1 | 0x10000) << (n & 15);
	switch(n >> 4){
#ifdef GPIOA
	case 0: GPIOA->BSRR = v; break;
#endif
#ifdef GPIOB
	case 1: GPIOB->BSRR = v; break;
#endif
#ifdef GPIOC
	case 2: GPIOC->BSRR = v; break;
#endif
#ifdef GPIOD
	case 3: GPIOD->BSRR = v; break;
#endif
#ifdef GPIOE
	case 4: GPIOE->BSRR = v; break;
#endif
#ifdef GPIOF
	case 5: GPIOF->BSRR = v; break;
#endif
#ifdef GPIOG
	case 6: GPIOG->BSRR = v; break;
#endif
#ifdef GPIOH
	case 7: GPIOH->BSRR = v; break;
#endif
#ifdef GPIOI
	case 8: GPIOI->BSRR = v; break;
#endif
#ifdef GPIOJ
	case 9: GPIOJ->BSRR = v; break;
#endif
#ifdef GPIOK
	case 10: GPIOK->BSRR = v; break;
#endif
	default: break;
	}
}
#endif

int
gpioget(int n)
{
	switch(n >> 4){
#ifdef GPIOA
	case 0: return GPIOA->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOB
	case 1: return GPIOB->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOC
	case 2: return GPIOC->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOD
	case 3: return GPIOD->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOE
	case 4: return GPIOE->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOF
	case 5: return GPIOF->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOG
	case 6: return GPIOG->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOH
	case 7: return GPIOH->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOI
	case 8: return GPIOI->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOJ
	case 9: return GPIOJ->IDR >> (n & 15) & 1;
#endif
#ifdef GPIOK
	case 10: return GPIOK->IDR >> (n & 15) & 1;
#endif
	default: return 0;
	}
}
