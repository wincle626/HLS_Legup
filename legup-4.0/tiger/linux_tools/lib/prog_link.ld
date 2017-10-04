	SECTIONS
	{
		. = 0x800020;
		.text : { *(_main_section) *(.text) }
		.data : { *(.data) }
		.bss : { *(.bss) }
	}

