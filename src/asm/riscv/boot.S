# Boot for Risc-V 
.option norvc

.section .boot.data

.section .boot.text

.global _start
_start:

.option push
.option norelax
	la		gp, _bglobal_pointer
.option pop
	# No address translation for now
	csrw		satp, x0
		
	# Zeroes the BSS section
	la 			t0, _bbss_start
	la			t1, _bbss_end
	bgeu		t0, t1, 2f
1:
	sd			x0, (t0)
	addi		t0, t0, 8
	bltu		t0, t1, 1b
2:	
	# Store the stack frame, so we can unwind whenever the kernel panics!
	add			fp, x0, x0
	la			sp, _bstack_end

	addi		sp, sp, -16
	sd			x0, 0(sp)
	la			t4, 3f
	sd			t4, 8(sp)
3:
	# Calls bootmain, so we start using C code. a0 and a1 should contain argc and argv
	#call		bootmain



.section .data

.section .text.init

.global _high_start
_high_start:
	wfi
	j _high_start

