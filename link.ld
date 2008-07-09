ENTRY (_loader)

SECTIONS{
    . = 0x00100000;
    .text :{
	code = .; _code = .; __code = .;
        *(.text)
    }
    .rodata ALIGN (0x1000) : {
        *(.rodata)
    }
    .data ALIGN (0x1000) : {
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
    kernel_end = .; _kernel_end = .; __kernel_end = .;
}
