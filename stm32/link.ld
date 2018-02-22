ENTRY(_start)

MEMORY {
	rom (rx)	: ORIGIN = 0x08000000, LENGTH = 128K
	ram (!rx)	: ORIGIN = 0x20000000, LENGTH = 20K
}

PROVIDE(_estack = ORIGIN(ram) + LENGTH(ram));

SECTIONS {
	.text : {
		_text = .;
		*(.vectors)
		*(.text)
	} >rom
	.rodata : {
		*(.rodata)
	} >rom
        .ARM.exidx : {
                *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        } >rom
	_etext = .;
	.data : {
		_data = .;
		*(.data)
		. = ALIGN(4);
		_edata = .;
	} >ram AT>rom
	.bss : {
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_end = .;
	} >ram
}
