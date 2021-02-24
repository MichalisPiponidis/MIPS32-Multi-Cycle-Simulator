.data
.text
#print: cycles 160 & 465, final state
main:
	and $zero, $zero, $zero
	and $at, $zero, $zero
	and $v0, $zero, $zero
	and $v1, $zero, $zero
	and $a0, $zero, $zero
	and $a1, $zero, $zero
	and $a2, $zero, $zero
	and $a3, $zero, $zero
	and $t0, $zero, $zero
	and $t1, $zero, $zero
	and $t2, $zero, $zero
	and $t3, $zero, $zero
	and $t4, $zero, $zero
	and $t5, $zero, $zero
	and $t6, $zero, $zero
	and $t7, $zero, $zero
	and $s0, $zero, $zero
	and $s1, $zero, $zero
	and $s2, $zero, $zero
	and $s3, $zero, $zero
	and $s4, $zero, $zero
	and $s5, $zero, $zero
	and $s6, $zero, $zero
	and $s7, $zero, $zero
	and $t8, $zero, $zero
	and $t9, $zero, $zero
	and $k0, $zero, $zero
	and $k1, $zero, $zero
	and $fp, $zero, $zero
	and $ra, $zero, $zero
	
	#initialize matrix1
	addi $t1, $zero, 1
	sw $t1, 0($gp)
	addi $t1, $zero, 2
	sw $t1, 4($gp)
	addi $t1, $zero, 3
	sw $t1, 8($gp)
	addi $t1, $zero, 4
	sw $t1, 12($gp)
	addi $t1, $zero, 5
	sw $t1, 16($gp)
	addi $t1, $zero, 6
	sw $t1, 20($gp)
	addi $t1, $zero, 7
	sw $t1, 24($gp)
	addi $t1, $zero, 8
	sw $t1, 28($gp)
	addi $t1, $zero, 9
	sw $t1, 32($gp)
	addi $t1, $zero, 10
	sw $t1, 36($gp)
	addi $t1, $zero, 11
	sw $t1, 40($gp)
	addi $t1, $zero, 12
	sw $t1, 44($gp)
	addi $t1, $zero, 13
	sw $t1, 48($gp)
	addi $t1, $zero, 14
	sw $t1, 52($gp)
	addi $t1, $zero, 15
	sw $t1, 56($gp)
	addi $t1, $zero, 16
	sw $t1, 60($gp)
	
	#initialize matrix2
	addi $t1, $zero, 2
	sw $t1, 64($gp)
	sw $t1, 68($gp)
	sw $t1, 72($gp)
	sw $t1, 76($gp)
	
	addi $t0, $gp, 0		#pointer to matrix1
	addi $t1, $gp, 64		#pointer to matrix2
	addi $t6, $t6, 4		#itterations counter
	addi $t7, $t7, 16		#itterations counter
	addi $t8, $gp, 16		#itterations counter
	addi $t9, $gp, 80		#address to save result
row1:
	lw $s0, 0($t0)
	lw $s1, 0($t1)
	sll $s2, $s0, 1
	add $s3, $s3, $s2
	add $t0, $t0, $t6
	add $t1, $t1, $t6
	bne $t0, $t8, row1
	sw $s3, 0($t9)		#store 1st element

	addi $t1, $gp, 64
	#addi $t1, $zero, 16
	addi $s3, $zero, 0
	add $t8, $t8, $t7
	addi $t9, $t9, 4
row2:
	lw $s0, 0($t0)
	lw $s1, 0($t1)
	sll $s2, $s0, 1
	add $s3, $s3, $s2
	add $t0, $t0, $t6
	add $t1, $t1, $t6
	bne $t0, $t8, row2	
	sw $s3, 0($t9)		#store 2nd element
	
	addi $t1, $gp, 64
	#addi $t1, $zero, 16
	addi $s3, $zero, 0
	add $t8, $t8, $t7
	addi $t9, $t9, 4
row3:
	lw $s0, 0($t0)
	lw $s1, 0($t1)
	sll $s2, $s0, 1
	add $s3, $s3, $s2
	add $t0, $t0, $t6
	add $t1, $t1, $t6
	bne $t0, $t8, row3
	sw $s3, 0($t9)		#store 3rd element
	
	addi $t1, $gp, 64
	#addi $t1, $zero, 16
	addi $s3, $zero, 0
	add $t8, $t8, $t7
	addi $t9, $t9, 4

row4:
	lw $s0, 0($t0)
	lw $s1, 0($t1)
	sll $s2, $s0, 1
	add $s3, $s3, $s2
	add $t0, $t0, $t6
	add $t1, $t1, $t6
	bne $t0, $t8, row4
	sw $s3, 0($t9)		#store 4th element

end:	
	sll $zero, $zero, 0			#THIS IS MANDATORY

