	SECTIONS
	{
		. = 0x800000;
		.text : { *(_main_section) *(.text) }
		.data : { *(.data) }
		.bss : { *(.bss) }
	}

