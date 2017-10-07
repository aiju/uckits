.thumb
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

.thumb_func
.global _nmi
_nmi:
	b _nmi

.thumb_func
.global _hard
_hard:
	b _hard

.thumb_func
.global _pagefault
_pagefault:
	b _pagefault

.thumb_func
.global _busfault
_busfault:
	b _busfault

.thumb_func
.global _usagefault
_usagefault:
	b _usagefault

.thumb_func
.global _svcall
_svcall:
	b _svcall

.thumb_func
.global _pendsv
_pendsv:
	b _pendsv

.thumb_func
.global _systick
_systick:
	b _systick

.thumb_func
.global _irqdef
_irqdef:
	b _irqdef
