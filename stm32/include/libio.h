typedef struct Uart Uart;

extern int apb1hz, apb2hz, apb3hz, apb4hz, ahbhz;

enum {
	PERUSART1,
	PERUSART2,
	PERUSART3,
	PERUSART4,
	PERUSART5,
	PERUSART6,
	PERUSART7,
	PERUSART8,
	PERUSART9,
	PERUSART10,
	PERGPIOA,
	PERGPIOB,
	PERGPIOC,
	PERGPIOD,
	PERGPIOE,
	PERGPIOF,
	PERGPIOG,
	PERGPIOH,
	PERGPIOI,
	PERGPIOJ,
	PERGPIOK,
	PERAFIO,
	PERUSB,
	PERTIM1,
	PERTIM2,
	PERTIM3,
	PERTIM4,
	PERTIM5,
	PERTIM6,
	PERTIM7,
	PERTIM8,
	PERTIM9,
	PERTIM10,
	PERTIM11,
	PERTIM12,
	PERTIM13,
	PERTIM14,
	PERTIM15,
	PERTIM16,
	PERTIM17,
	PERTIM18,
	PERTIM19,
	PERTIM20,
	PERTIM21,
	PERTIM22,
	PERTIM23,
	PERI2C1,
	PERI2C2,
	PERI2C3,
	PERI2C4,
	PERSPI1,
	PERSPI2,
	PERSPI3,
	PERSPI4,
	PERSPI5,
	PERSPI6,
};

void*	peribase(int);
int	periclk(int, int);

void	initclk(void);

struct Uart {
	void *base;
};

int	uartopen(Uart *, int, int, int);

enum {
	STOPMsk = 0xf,
	STOP1 = 0,
	STOP05,
	STOP15,
	STOP2,
	
	PARMsk = 0xf0,
	PARNO = 0<<4,
	PARODD = 1<<4,
	PAREVEN = 2<<4,
};

int	uartwrite(Uart *, void *, size_t);
int	uartread(Uart *, void *, size_t);

void	gpiocfg(int, int);
void	gpioset(int, int);
int	gpioget(int);

#define PORTA(n) ((n)&15)
#define PORTB(n) (0x10|(n)&15)
#define PORTC(n) (0x20|(n)&15)
#define PORTD(n) (0x30|(n)&15)
#define PORTE(n) (0x40|(n)&15)
#define PORTF(n) (0x50|(n)&15)
#define PORTG(n) (0x60|(n)&15)
#define PORTH(n) (0x70|(n)&15)
#define PORTI(n) (0x80|(n)&15)
#define PORTJ(n) (0x90|(n)&15)
#define PORTK(n) (0xa0|(n)&15)

enum {
	GPIOIN = 0,
	GPIOOUT = 1,
	GPIOPULLUP = 2,
	GPIOPULLDN = 4,
	GPIOPUSHPULL = 0,
	GPIOOPDRAIN = 8,
	GPIOANALOG = 16,
	GPIOALT = 32,
	GPIOSPEEDMsk = 0xf00,
	GPIOSPEED0 = 0x000,
	GPIOSPEED1 = 0x100,
	GPIOSPEED2 = 0x200,
};

void	delayus(uint);

void	irqen(int, int, u8int);

enum {
	EXTIRQRISE = 1,
	EXTIRQFALL = 2,
};

int	extirqcfg(int, int, int);
int	extirqen(int, int);
void	extirqack(int);
