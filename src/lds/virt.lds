
OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{
  	ram  (wxa) : ORIGIN = 0x84000000, LENGTH = 10M
}

PHDRS
{
  	text PT_LOAD;
  	data PT_LOAD;
  	bss PT_LOAD;
}

SECTIONS
{
	.boot.text : {
		*(.boot.text.init) *(.boot.text .boot.text.*)
	}
	
	PROVIDE(_bglobal_pointer = .);
	
	.boot.rodata : {
		*(.boot.rodata .boot.rodata.*)
	}

	.boot.data : {
    	*(.boot.sdata .boot.sdata.*) *(.boot.data .boot.data.*)
	}

	.boot.bss : {
		. = ALIGN(8);
		PROVIDE(_bbss_begin = .);
    	*(.boot.sbss .boot.sbss.*) *(.boot.bss .boot.bss.*)
		. = ALIGN(16);
		PROVIDE(_bbss_end = .);
  	}
	
	PROVIDE(_bstack_start = _bbss_end);
	PROVIDE(_bstack_end = _bstack_start + 0x80000);

	. = 0xFFFFFFFFC0000000;

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
  	PROVIDE(_memory_end = ORIGIN(ram) + LENGTH(ram));

  	PROVIDE(_heap_start = _stack_end);
  	PROVIDE(_heap_size = _memory_end - _heap_start);
}
