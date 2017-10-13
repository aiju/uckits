#!/bin/sh
H=$UCKITS/stm32/include/$(echo -n $1 | tr A-Z a-z).h
cat <<END
#include <u.h>
#include <uio.h>
#include <libc.h>
#include <libio.h>

#define apb1lhz apb1hz
#define apb1hhz apb1hz
#define ahb1hz ahbhz
#define ahb2hz ahbhz
#define ahb3hz ahbhz
#define ahb4hz ahbhz
#define iophz ahbhz

void *
peribase(int p)
{
	switch(p){
END
sed -n 's/^#define USART\([0-9]\+\)_BASE.*$/\1/gp' $H | awk ' { printf "\tcase PERUSART%d: return USART%d;\n", $1, $1 } '
sed -n 's/^#define GPIO\([A-Z]\)_BASE.*$/\1/gp' $H | awk ' { printf "\tcase PERGPIO%s: return GPIO%s;\n", $1, $1 } '
cat <<END
	default: werrstr1(Enodev); return nil;
	}
}

int
periclk(int p, int st)
{
	switch(p){
END
sed -n 's/^#define RCC_\([A-Z0-9]\+\)ENR\([0-9]*\)_USART\([0-9]\+\)EN .*$/\1;\2;\3/gp' $H | awk -F';' ' { printf "\tcase PERUSART%d:\n\t\tif(st > 0) RCC->%sENR%s |= RCC_%sENR%s_USART%sEN;\n\t\telse if(st == 0) RCC->%sENR%s &= ~RCC_%sENR%s_USART%sEN;\n\t\treturn %shz;\n", $3, $1, $2, $1, $2, $3, $1, $2, $1, $2, $3, tolower($1) } '
sed -n 's/^#define RCC_\([A-Z0-9]\+\)ENR\([0-9]*\)_GPIO\([A-Z]\)EN .*$/\1;\2;\3/gp' $H | awk -F';' ' { printf "\tcase PERGPIO%s:\n\t\tif(st > 0) RCC->%sENR%s |= RCC_%sENR%s_GPIO%sEN;\n\t\telse if(st == 0) RCC->%sENR%s &= ~RCC_%sENR%s_GPIO%sEN;\n\t\treturn %shz;\n", $3, $1, $2, $1, $2, $3, $1, $2, $1, $2, $3, tolower($1) } '
sed -n 's/^#define RCC_\([A-Z0-9]\+\)ENR\([0-9]*\)_USB\(FS\)\?EN .*$/\1;\2;\3/gp' $H | awk -F';' ' { printf "\tcase PERUSB:\n\t\tif(st > 0) RCC->%sENR%s |= RCC_%sENR%s_USB%sEN;\n\t\telse if(st == 0) RCC->%sENR%s &= ~RCC_%sENR%s_USB%sEN;\n\t\treturn %shz;\n", $1, $2, $1, $2, $3, $1, $2, $1, $2, $3, tolower($1) } '
cat <<END
	default:
		werrstr1(Enodev);
		return -1;
	}
}
END
