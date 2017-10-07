ENTRY(_start)

MEMORY {
	rom (rx)	: ORIGIN = 0x08005000, LENGTH = 128K
	ram (!rx)	: ORIGIN = 0x20001000, LENGTH = 16K
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
	_etext = .;
	.data : {
		_data = .;
		*(.data)
		. = ALIGN(4);
		_edata = .;
	} >ram AT>rom
	.bss : {
		*(.bss)
		. = ALIGN(4);
		_end = .;
	} >ram
}
