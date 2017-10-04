/* Linker script for Altera SoCFPGA
 * Derived from Linker Linker script for ARMulator (RDI)
 *  Derived from file "armulator-ram-hosted.ld"
 *
 * Version: Sourcery CodeBench Lite 2012.03-56
 * Support: https://support.codesourcery.com/GNUToolchain/
 *
 * Copyright (c) 2007, 2008, 2009, 2010 CodeSourcery, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
/*ENTRY(__cs3_reset)*/
/*SEARCH_DIR(./build/)*/
/*GROUP(-lgcc -lc -lcs3 -lcs3unhosted -lcs3arm)*/
/* These force the linker to search for particular symbols from
 * the start of the link process and thus ensure the user's
 * overrides are picked up
 */
/*EXTERN(__cs3_reset __cs3_reset_generic)*/
EXTERN(__cs3_start_asm _start)
/* Bring in the interrupt routines & vector */
EXTERN (__cs3_reset)
EXTERN (__cs3_isr_undef)
EXTERN (__cs3_isr_swi)
EXTERN (__cs3_isr_pabort)
EXTERN (__cs3_isr_dabort)
EXTERN (__cs3_isr_reserved)
EXTERN (__cs3_isr_irq)
EXTERN (__cs3_isr_fiq)
EXTERN(__cs3_interrupt_vector_arm)

EXTERN(__stack_top)

/* These symbols are defined through linker options in Monitor Program.[ArmA9AsmProgram.java]
EXTERN(arm_program_mem arm_available_mem_size)

EXTERN(__cs3_start_c main __cs3_stack __cs3_heap_end)

/* Provide fall-back values */
PROVIDE(__cs3_heap_start = _end);
PROVIDE(__cs3_heap_end = __cs3_region_start_ram + __cs3_region_size_ram);
PROVIDE(__cs3_region_num = (__cs3_regions_end - __cs3_regions) / 20);
PROVIDE(__cs3_stack = __cs3_region_start_ram + __cs3_region_size_ram);

PROVIDE(__startup_lma = 0x00100000);
PROVIDE(__text_lma    = 0x00200000);
PROVIDE(__vma_offset  = 0x00000000);
__text_vma = __text_lma + __vma_offset;

PROVIDE(__stack_top   = __vma_offset + 0x40000000);

INPUT(arm_startup.o)

/* __legup_init_arm is the entry point the preloader should branch to */
ENTRY(__legup_init_arm)

SECTIONS
{
  /* The startup section should be first
     This section has the same LMA and VMA, and will reside in the 1GB or DDR on
     the DE1-SoC.  */
  .startup __startup_lma :
  {
    /* LegUp ARM startup code - setup MMU, Page Tables, Caches, etc */
    arm_startup.o(.startup)
    *(.startup)

    /* Put the translation table here */
    . = ALIGN(0x4000);
    arm_startup.o(.ttb_data)
    *(.ttb_data)
    . = . + 0x400;
    __startup_stack_top = .;
  }

  /* The .text section has an LMA in the first 1GB of the address space, and a
     VMA in the second GB of the address space, eg. starting at 0x40000000. */
  .text __text_vma :
  AT(__text_lma)
  {
    __text_start = .;
    /* the .text section of arm_startup should be the first thing to run after our
       startup code */
    arm_startup.o(.text)

    . = ALIGN(0x8);

    CREATE_OBJECT_SYMBOLS
    __cs3_region_start_ram = .;
    _ftext = .;
    *(.cs3.region-head.ram)
    ASSERT (. == __cs3_region_start_ram, ".cs3.region-head.ram not permitted");
    __cs3_interrupt_vector = __cs3_interrupt_vector_arm;
    *(.cs3.interrupt_vector)
    /* Make sure we pulled in an interrupt vector.  */
    ASSERT (. != __cs3_interrupt_vector_arm, "No interrupt vector");

    PROVIDE(__cs3_reset = __cs3_reset_generic);
    *(.cs3.reset)

    /*_start = DEFINED(__cs3_start_asm) ? __cs3_start_asm : _start;*/
    *(.text.cs3.init)
    *(.text .text.* .gnu.linkonce.t.*)
    *(.plt)
    *(.gnu.warning)
    *(.glue_7t) *(.glue_7) *(.vfp11_veneer)

    *(.ARM.extab* .gnu.linkonce.armextab.*)
    *(.gcc_except_table)
  }
  .eh_frame_hdr : ALIGN (4)
  {
    KEEP (*(.eh_frame_hdr))
    *(.eh_frame_entry .eh_frame_entry.*)
  }
  .eh_frame : ALIGN (4)
  {
    KEEP (*(.eh_frame)) *(.eh_frame.*)
  }
  /* .ARM.exidx is sorted, so has to go in its own output section.  */
  PROVIDE_HIDDEN (__exidx_start = .);
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  }
  PROVIDE_HIDDEN (__exidx_end = .);

  .rodata : ALIGN (8)
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)

    . = ALIGN(4);
    KEEP(*(.init))

    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP (*(.preinit_array))
    __preinit_array_end = .;

    . = ALIGN(4);
    __init_array_start = .;
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    __init_array_end = .;

    . = ALIGN(4);
    KEEP(*(.fini))

    . = ALIGN(4);
    __fini_array_start = .;
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    __fini_array_end = .;

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*crtend.o(.ctors))

    . = ALIGN(0x4);
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*crtend.o(.dtors))

    . = ALIGN(4);
    __cs3_regions = .;
    LONG (0)
    LONG (__cs3_region_init_ram)
    LONG (__cs3_region_start_ram)
    LONG (__cs3_region_init_size_ram)
    LONG (__cs3_region_zero_size_ram)
    __cs3_regions_end = .;
    . = ALIGN (8);
    _etext = .;
  }


  ASSERT (!(__cs3_region_init_ram & 7), "__cs3_region_init_ram not aligned")
  ASSERT (!(__cs3_region_start_ram & 7), "__cs3_region_start_ram not aligned")
  ASSERT (!(__cs3_region_init_size_ram & 7), "__cs3_region_init_size_ram not aligned")
  ASSERT (!(__cs3_region_zero_size_ram & 7), "__cs3_region_zero_size_ram not aligned")

  /* If the .data and .text section address are the same, append the .data
   * section after .text section, the feature is disabled now
   *     _arm_data_source = .;
   *     . = (arm_program_mem == arm_data_mem) ? _arm_data_source : arm_data_mem;
   */

  .data : ALIGN (0x10)
  {
    KEEP(*(.jcr))
    *(.got.plt) *(.got)
    *(.shdata)
    *(.data .data.* .gnu.linkonce.d.*)
    . = ALIGN (8);
    *(.ram)
    . = ALIGN (8);
    _edata = .;
  }
  .bss : ALIGN (0x10)
  {
    *(.shbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN (8);
    *(.ram.b .bss.ram)
    . = ALIGN (8);
    _end = .;
    __end = .;
  }

  /* __cs3_region_end_ram is deprecated */
  __cs3_region_end_ram = __cs3_region_start_ram + arm_available_mem_size;
  /* Default to 1M of heap */
  __cs3_region_size_ram = arm_available_mem_size;
  __cs3_region_init_ram = LOADADDR (.text);
  __cs3_region_init_size_ram = _edata - ADDR (.text);
  __cs3_region_zero_size_ram = _end - _edata;

  .stab 0 (NOLOAD) : { *(.stab) }
  .stabstr 0 (NOLOAD) : { *(.stabstr) }
  /* DWARF debug sections.
   * Symbols in the DWARF debugging sections are relative to
   * the beginning of the section so we begin them at 0.
   */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* DWARF 2.1 */
  .debug_ranges   0 : { *(.debug_ranges) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }

  .note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
  .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) }
  /DISCARD/ : { *(.note.GNU-stack) }
}
/* checksum: 0c047b73e70684920c31d9 */
