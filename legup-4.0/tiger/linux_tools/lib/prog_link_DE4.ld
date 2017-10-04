	SECTIONS
	{
		. = 0x40000020;
		.text : { *(_main_section) *(.text) }
		.data : { *(.data) }
		.bss : { *(.bss) }
	}

