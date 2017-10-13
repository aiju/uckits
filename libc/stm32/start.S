.syntax unified
.code 16

.section .vectors
.global _vectors
.extern _estack
_vectors:
	.word _estack
	.word _start
	.word _nmi
	.word _hard
	.word _pagefault
	.word _busfault
	.word _usagefault
	.word 0
	.word 0
	.word 0
	.word 0
	.word _svcall
	.word 0
	.word 0
	.word _pendsv
	.word _systick
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef
	.word _irqdef

.section .text
.thumb_func
.global _start
_start:
	BL _init
	BL main
_loop:	B _loop

.global _nmi, _hard, _pagefault, _busfault, _usagefault, _svcall, _pendsv, _systick, _irqdef
.weak _nmi, _hard, _pagefault, _busfault, _usagefault, _svcall, _pendsv, _systick, _irqdef
.thumb_func
_nmi:
.thumb_func
_hard:
.thumb_func
_pagefault:
.thumb_func
_busfault:
.thumb_func
_usagefault:
.thumb_func
_svcall:
.thumb_func
_pendsv:
.thumb_func
_systick:
.thumb_func
_irqdef:
	push {r4-r11}
	mrs r0, ipsr
	sub r0, #16
	add r1, sp, #64
	push {r0,r1,r14}
	mov r0, r13
	sub sp, #4 /* restore 8-byte alignment */
	bl trap
	add sp, #4
	pop {r0,r1,r14}
	pop {r4-r11}
	bx lr

.global trap
.weak trap
trap:
	B trap
