	SECTIONS
	{
		. = 0x80030000;
		.text : { *(_main_section) *(.text) }
		.data : { *(.data) }
		.bss : { *(.bss) }
	}
