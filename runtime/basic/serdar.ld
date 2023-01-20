OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv",
	      "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_entry)
SECTIONS
{
PROVIDE(__stack_size = 0x100);
. = 0x1000;
.text.init : { *(.text.init) }
. = ALIGN(0x1000);
.tohost : { *(.tohost) }
. = ALIGN(0x1000);
.text : { *(.text) }
. = ALIGN(0x1000);
.data : { 
  *(.data);
  PROVIDE( __global_pointer$ = . + 0x200 );
}
.bss : { *(.bss) }
.stack (NOLOAD) : ALIGN(16) { 
  PROVIDE(stack_begin = . );
  . += __stack_size;
  PROVIDE(_sp = . ); 
  PROVIDE(stack_end = . );
}
_end = .;
}
