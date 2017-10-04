     SECTIONS
     {
       . = 0x80030000;
       .text : { *(.text) }
       .data : { *(.data) }
       .bss : { *(.bss) }
     }
