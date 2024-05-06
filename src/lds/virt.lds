OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{
  ram  (wxa) : ORIGIN = 0x84000000, LENGTH = 10M
}

/*
	The low memory part of the kernel will be reside into l* sections/headers.
	The higher part will reside into normal sections/headers.
*/

PHDRS
{
	ltext PT_LOAD;
	ldata PT_LOAD;
  	lbss PT_LOAD;
	text PT_LOAD;
  	data PT_LOAD;
  	bss PT_LOAD;	
}

SECTIONS
{
	.ltext : {
		. = ALIGN(4096);
		PROVIDE(_lowmem_kernel_begin = .);
		PROVIDE(_ltext_start = .);
		*(.ltext) *(.ltext .ltext.*)
		. = ALIGN(4096);
		PROVIDE(_ltext_end = .);
	} >ram AT>ram : ltext

	PROVIDE(_lglobal_pointer = .);

	.lrodata : {
		. = ALIGN(4096);
		PROVIDE(_lrodata_start = .);
    	*(.lrodata .lrodata.*)
    	. = ALIGN(4096);
		PROVIDE(_lrodata_end = .);
	}>ram AT> ram : ltext

	.ldata : {
  		. = ALIGN(4096);
  	  	PROVIDE(_ldata_start = .);
  	  	*(.lsdata .lsdata.*) *(.ldata .ldata.*)
  	  	. = ALIGN(4096);
		PROVIDE(_ldata_end = .);
  	} >ram AT>ram : ldata

	.lbss : {
		. = ALIGN(4096);
  	  	PROVIDE(_lbss_start = .);
  	  	*(.lsbss .lsbss.*) *(.lbss .lbss.*)
  	  	. = ALIGN(4096);
		PROVIDE(_lbss_end = .);
  	} >ram AT>ram : lbss

	PROVIDE(_lstack_start = _lbss_end);
  	PROVIDE(_lstack_end = _stack_start + 0x80000);
  	. = ALIGN(4096);
  	PROVIDE(_lheap_start = _lstack_end);

  	.text : {
		. = ALIGN(4096);
		PROVIDE(_kernel_begin = .);
  	  	PROVIDE(_text_start = .);
  	  	*(.text.init) *(.text .text.*)
		. = ALIGN(4096);
  	  	PROVIDE(_text_end = .);
  	} >ram AT>ram :text
	
  	PROVIDE(_global_pointer = .);
  	
	.rodata : {
		. = ALIGN(4096);
  	  	PROVIDE(_driverinfo_start = .);
		*(.driverinfo .driverinfo.*)
		PROVIDE(_driverinfo_end = .);
		PROVIDE(_rodata_start = .);
  	  	*(.rodata .rodata.*)
  	  	. = ALIGN(4096);
		PROVIDE(_rodata_end = .);
  	} >ram AT>ram :text

  	.data : {
  		. = ALIGN(4096);
  		PROVIDE(_data_start = .);
  		*(.sdata .sdata.*) *(.data .data.*)
  		. = ALIGN(4096);
		PROVIDE(_data_end = .);
  	} >ram AT>ram :data

  	.bss : {
		. = ALIGN(4096);
  	  	PROVIDE(_bss_start = .);
  	  	*(.sbss .sbss.*) *(.bss .bss.*)
  	  	. = ALIGN(4096);
		PROVIDE(_bss_end = .);
  	} >ram AT>ram :bss

  	PROVIDE(_memory_start = ORIGIN(ram));
  	PROVIDE(_stack_start = _bss_end);
  	PROVIDE(_stack_end = _stack_start + 0x80000);
  	. = ALIGN(4096);
  	PROVIDE(_kernel_end = .);
	PROVIDE(_heap_start = .);

}
