typedef unsigned char u8int;
typedef unsigned short u16int;
typedef unsigned int u32int;
typedef unsigned long long u64int;

typedef char s8int;
typedef short s16int;
typedef int s32int;
typedef long long s64int;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long vlong;
typedef unsigned long long uvlong;

typedef u32int uintptr;
typedef u32int Rune;
typedef u32int size_t;
typedef s32int ssize_t;

typedef long	jmp_buf[2];
#define	JMPBUFSP	0
#define	JMPBUFPC	1
#define	JMPBUFDPC	0

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_arg __builtin_va_arg
#define va_end __builtin_va_end

#define nil ((void*)0)

typedef union FPdbleword FPdbleword;

union FPdbleword
{
	double	x;
	struct {	/* little endian */
		ulong lo;
		ulong hi;
	};
};

#define USED(x) ((void)(x))

#define ROM __attribute__((section(".rodata,\"a\",%progbits //")))
#define WEAK __attribute__((weak))

typedef struct Ureg Ureg;
struct Ureg {
	u32int irq, sp, ret;
	u32int r4, r5, r6, r7, r8, r9, r10, r11;
	u32int r0, r1, r2, r3, r12, lr, pc, psr;
};

static inline int spllo(void)
{
	int rv;
	
	__asm__("mrs %0, primask" : "=r"(rv));
	__asm__ volatile("cpsie i");
	__asm__ volatile("isb");
	return rv;
}

static inline int splhi(void)
{
	int rv;
	
	__asm__("mrs %0, primask" : "=r"(rv));
	__asm__ volatile("cpsid i");
	return rv;
}

static inline void splx(int s)
{
	__asm__ volatile("msr primask, %0" :: "r"(s));
}

static inline void wfi(void)
{
	__asm__ volatile("wfi");
}
