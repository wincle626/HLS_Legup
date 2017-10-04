__stack_top = mem_start_addr + mem_size;
	SECTIONS
	{
		. = mem_start_addr;
		.text : { *(_mips_startup_section) *(_main_section) *(.text) }
		.data : { *(.data) }
		.bss : { *(.bss) }
	}

