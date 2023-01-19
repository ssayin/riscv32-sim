OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv",
	      "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_entry)
SECTIONS
{
. = 0x1000;
.text.init : { *(.text.init) }
. = ALIGN(0x1000);
.tohost : { *(.tohost) }
.mtime : { *(.mtime) }
.mtimecmp : { *(.mtimecmp) }
. = ALIGN(0x1000);
.text : { *(.text) }
. = ALIGN(0x1000);
.data : { *(.data) PROVIDE( __global_pointer$ = . + 0x200 );}
.bss : { *(.bss) }
_end = .;
}
