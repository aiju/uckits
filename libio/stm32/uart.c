#include <u.h>
#include <uio.h>
#include <libio.h>
#include <libc.h>

int
uartopen(Uart *p, int no, int baud, int flags)
{
	USART_TypeDef *usart;
	int hz;

	if(baud <= 0){
		werrstr1(Ebadctl);
		return -1;
	}
	memset(p, 0, sizeof(*p));
	if(no >= 10 || no <= 0 || (p->base = peribase(PERUSART1 + no - 1), p->base == nil)){
		werrstr1(Enodev);
		return -1;
	}
	usart = p->base;
	hz = periclk(PERUSART1 + no - 1, 1);
	if(hz < 0)
		return -1;
	usart->CR1 = 0;
	usart->CR3 = 0;
	switch(flags & STOPMsk){
	case STOP1: usart->CR2 = 0 << USART_CR2_STOP_Pos; break;
	case STOP05: usart->CR2 = 1 << USART_CR2_STOP_Pos; break;
	case STOP2: usart->CR2 = 2 << USART_CR2_STOP_Pos; break;
	case STOP15: usart->CR2 = 3 << USART_CR2_STOP_Pos; break;
	default:
		werrstr1(Ebadctl);
		return -1;
	}
	switch(flags & PARMsk){
	case PARNO: break;
	case PAREVEN: usart->CR1 |= USART_CR1_M | USART_CR1_PCE; break;
	case PARODD: usart->CR1 |= USART_CR1_M | USART_CR1_PCE | USART_CR1_PS; break;
	default:
		werrstr1(Ebadctl);
		return -1;
	}

	usart->BRR = hz / baud;
	usart->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
	return 0;
}

#if defined(USART_ISR_TXE) && !defined(USART_SR_TXE)
#define SR ISR
#define USART_SR_TXE USART_ISR_TXE
#define USART_SR_PE USART_ISR_PE
#define USART_SR_FE USART_ISR_FE
#define USART_SR_RXNE USART_ISR_RXNE
#define DR TDR
#endif

int
uartwrite(Uart *p, void *v, size_t n)
{
	uchar *pv;
	size_t m;

	USART_TypeDef *usart;
	if(p->base == nil){
		werrstr1(Enodev);
		return -1;
	}
	usart = p->base;
	pv = v;
	for(m = 0; m < n; m++){
		while((usart->SR & USART_SR_TXE) == 0)
			;
		usart->DR = *pv++;
	}
	return n;
}

#if defined(DR)
#undef DR
#define DR RDR
#endif

int
uartread(Uart *p, void *v, size_t n)
{
	uchar *pv;
	size_t m;

	USART_TypeDef *usart;
	if(p->base == nil){
		werrstr1(Enodev);
		return -1;
	}
	usart = p->base;
	pv = v;
	for(m = 0; m < n; m++){
		while((usart->SR & (USART_SR_RXNE | USART_SR_PE | USART_SR_FE)) == 0)
			;
		if((usart->SR & (USART_SR_PE|USART_SR_FE)) != 0){
			#ifdef USART_ICR_PE
			usart->ICR = USART_ICR_PE | USART_ICR_FE;
			#else
			USED(usart->DR);
			#endif
			werrstr1(Eio);
			return m;
		}
		*pv++ = usart->DR;
	}
	return m;
}
