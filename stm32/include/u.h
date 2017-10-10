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
