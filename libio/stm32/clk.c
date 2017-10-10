#include <u.h>
#include <uio.h>
#include <libio.h>

int apb1hz, apb2hz, ahbhz;

#if defined(STM32F103x6) || defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG)

void
initclk(void)
{
	RCC->CR |= RCC_CR_HSEON;
	while((RCC->CR & RCC_CR_HSERDY) == 0)
		;
	FLASH->ACR |= FLASH_ACR_PRFTBE | 2;
	SETFIELD(RCC, CFGR, PLLMULL, 7);
	SETFIELD(RCC, CFGR, PPRE1, 4);
	SETFIELD(RCC, CFGR, PPRE2, 0);
	SETFIELD(RCC, CFGR, HPRE, 0);
	RCC->CFGR = RCC->CFGR & ~(RCC_CFGR_PLLXTPRE) | RCC_CFGR_PLLSRC;
	RCC->CR |= RCC_CR_PLLON;
	while((RCC->CR & RCC_CR_PLLRDY) == 0)
		;
	SETFIELDE(RCC, CFGR, SW, PLL);
	while((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL)
		;
	
	apb1hz = 36000000;
	apb2hz = 72000000;
	ahbhz = 72000000;
}

#endif
