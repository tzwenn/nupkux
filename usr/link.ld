ENTRY (_start)

SECTIONS {
	. = 0x10000;
	.text : {
		code = .; _code = .; __code = .;
		*(.text)
	}
	.rodata : {
		*(.rodata)
	}
	.data : {
		data = .; _data = .; __data = .;
		*(.data)
	}
	.bss : {
		bss = .; _bss = .; __bss = .;
		_sbss = .;
		*(COMMON)
		*(.bss)
		_ebss = .;
	}
    	end = .;
}

