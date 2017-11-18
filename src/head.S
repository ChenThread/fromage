	/*.org 0x10800*/
	.section ".boot"

	.ascii "PS-X EXE"
	.word 0, 0
	.word aaa_very_start, 0
	.extern _ftext
	.extern _LOADABLE_SIZE
	.word _ftext, _LOADABLE_SIZE
	.word 0, 0
	.word 0, 0
	.word 0x801FF000, 0
	.word 0, 0

	.word 0, 0
	.word 0
	/*.ascii "Sony Computer Entertainment Inc. for North America area"*/
	.ascii "Sony Computer Entertainment Inc. for Europe area"
	.word 0

	.section ".text"
load_base:
	.global aaa_nop_sled_cache_clearer
aaa_nop_sled_cache_clearer:
	.rept 4096/4
	nop
	.endr
	
	jr $ra
	nop

isr_handler_asm:
	.set noreorder
	.set noat

	/* Save registers */
	mfc0 $k0, $14
	addiu $sp, $sp, -4*19
	nop
	sw $at, 4*0($sp)
	sw $k0, 4*1($sp)
	sw $v0, 4*2($sp)
	sw $v1, 4*3($sp)
	sw $a0, 4*4($sp)
	sw $a1, 4*5($sp)
	sw $a2, 4*6($sp)
	sw $a3, 4*7($sp)
	sw $t0, 4*8($sp)
	sw $t1, 4*9($sp)
	sw $t2, 4*10($sp)
	sw $t3, 4*11($sp)
	sw $t4, 4*12($sp)
	sw $t5, 4*13($sp)
	sw $t6, 4*14($sp)
	sw $t7, 4*15($sp)
	sw $t8, 4*16($sp)
	sw $t9, 4*17($sp)
	sw $ra, 4*18($sp)

	/* Call interrupt handler */
	move $a0, $k0
	jal isr_handler_c
	addiu $sp, $sp, -4*4
	addiu $sp, $sp, 4*4
	move $k0, $a0
	nop

	/* Restore registers */
	lw $at, 4*0($sp)
	lw $k0, 4*1($sp)
	lw $v0, 4*2($sp)
	lw $v1, 4*3($sp)
	lw $a0, 4*4($sp)
	lw $a1, 4*5($sp)
	lw $a2, 4*6($sp)
	lw $a3, 4*7($sp)
	lw $t0, 4*8($sp)
	lw $t1, 4*9($sp)
	lw $t2, 4*10($sp)
	lw $t3, 4*11($sp)
	lw $t4, 4*12($sp)
	lw $t5, 4*13($sp)
	lw $t6, 4*14($sp)
	lw $t7, 4*15($sp)
	lw $t8, 4*16($sp)
	lw $t9, 4*17($sp)
	lw $ra, 4*18($sp)
	nop
	addiu $sp, $sp, 4*19

	/* Return to code */
	jr $k0
	rfe

	.set at
	.set reorder

	.global int_handler_stub
	.global int_handler_stub_end
	.extern isr_handler_c
int_handler_stub:
	la $k0, isr_handler_asm
	jr $k0
int_handler_stub_end:

aaa_very_start:
	/* Load stack pointer */
	la $sp, 0x801FFF00
	/*la $sp, 0x1F800400*/

	/* Load global pointer */
	la $gp, _gp

	/* Clear scratch */
	la $t0, 0x1F800000
	la $t1, 0x1F800400
	_lp_clear_scratch:
		sw $zero, ($t0)
		nop
		addiu $t0, $t0, 4
		nop
		bne $t0, $t1, _lp_clear_scratch

	/* Clear BSS + remaining heap */
	.extern _fbss
	.extern _end
	la $t0, _fbss
	/*la $t1, _end*/
	la $t1, 0x801FF000
	move $t3, $sp
	blt $t1, $t3, _skip_clamping_sp
	nop
	move $t1, $t3
	_skip_clamping_sp:
	_lp_clear_bss:
		sw $zero, ($t0)
		nop
		addiu $t0, $t0, 4
		nop
		bne $t0, $t1, _lp_clear_bss

	/* Call .init */
	.extern _INIT_SECTION
	la $t0, _INIT_SECTION
	jalr $t0

	/* Jump to start */
	la $t0, aaa_start
	addiu $sp, $sp, -32
	jalr $t0
	nop
	addiu $sp, $sp, 32
	j aaa_very_start
	nop

	.section ".rodata"

	.align 5
	.global fsys_mctex
fsys_mctex:
	.incbin "dat/atlas.raw"

	.align 5
	.global fsys_rawcga
fsys_rawcga:
	/*.incbin "rawcga.bin"*/

	.align 5
	.global fsys_s3m_test
fsys_s3m_test:
	/*.incbin "dat/s2-cez.s3m"*/
	.align 5

