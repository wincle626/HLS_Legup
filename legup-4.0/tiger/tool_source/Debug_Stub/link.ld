     SECTIONS
     {
       . = 0x00A00;
       .exception : {exception.o(.text)}
       . = 0x00000;
       .text : {start.o(.text) *(EXCLUDE_FILE (start.o) .text)}
       .data : { *(.data) }
       .bss : { *(.bss) }
     }