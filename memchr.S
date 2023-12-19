.equ REP_01, 0x0101010101010101
.equ REGSIZE, 8
.equ REGP, 3

.text
.global memchrasm

memchrasm:
	zext.b a1, a1
	beqz a2, ret0
	add a2, a0, a2
1:
	lbu t0, 0(a0)
	beq t0, a1, found
	addi a0, a0, 1
	blt a0, a2, 1b
ret0:
	li a0, 0
found:
	ret


##################################
.global memchr2
memchr2:
	addi	sp,sp,-16
	sd	s0,8(sp)
	addi	s0,sp,16
	add	a2,a2,a0
	zext.b	a1, a1
	beq a0, a2, m2
iter:
	lbu a5, 0(a0)
	addi a4,a0, 1
	beq a5,a1, m3
	mv a0,a4
	bne a0,a2, iter
m2:
	li a0,0
m3:
	ld s0,8(sp)
	addi sp,sp,16
	ret


.global memchr_zbb
memchr_zbb:
	# use byte-wide iteration for small numbers
	zext.b a1, a1

	add t4, a0, a2

	# get the number of bytes we should iterate before alignment
	andi t0, a0, REGSIZE - 1
	
	sub a0, a0, t0
	add a2, a2, t0
	
	# get 0xTBTBTBTBTBTBTBTB from 0xTB
	li t3, REP_01
	mul t3, t3, a1

	slli t0, t0, 3

	ld t2, 0(a0)
	xor t2, t2, t3
	orc.b t2, t2
	not t2, t2
	
	srl t2, t2, t0
	sll t2, t2, t0

	beqz t2, continue

	ctz t2, t2
	srli t2, t2, REGP
	add a0, a0, t2
	bge a0, t4, not_found
	ret

continue:
	addi a0, a0, REGSIZE
	addi a2, a2, -REGSIZE

	# get the count reminder
	andi t1, a2, REGSIZE - 1

	# align the count
	sub a2, a2, t1

	beqz a2, iter_reminder

	add a2, a2, a0

iter_words:
	ld t2, 0(a0)
	xor t2, t2, t3
	orc.b t2, t2
	not t2, t2
	bnez t2, found_word
	addi a0, a0, REGSIZE
	bne a0, a2, iter_words

iter_reminder:
	add t1, a0, t1
iter_bytes:
	beq a0, t1, not_found
	lb t4, 0(a0)
	beq t4, a1, fw2
	addi a0, a0, 1
	j iter_bytes
not_found:
	li a0, 0
	ret
found_word:
	ctz t2, t2
	srli t2, t2, 3
	add a0, a0, t2
fw2:
	ret
