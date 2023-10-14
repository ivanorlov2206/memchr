.equ REP_01, 0x0101010101010101
.equ REGSIZE, 8
.equ REGP, 3

.text
.global memchrasm

memchrasm:

# use byte-wide iteration for small numbers
add t1, x0, a2
sltiu t2, a2, 128
bnez t2, iter_reminder

# get the number of bytes we should iterate before alignment
andi t0, a0, REGSIZE - 1
xor t0, t0, REGSIZE - 1

sub a2, a2, t0
# iterate before alignment
pre_align:
beq t0, x0, continue
lb t2, 0(a0)
beq t2, a1, fw2
addi t0, t0, -1
addi a0, a0, 1
j pre_align

found_word:
ctz t2, t2
srli t2, t2, REGP
add a0, a0, t2
fw2:
ret

continue:

# get the count reminder
andi t1, a2, REGSIZE - 1

# align the count
sub a2, a2, t1

beqz a2, iter_reminder

# get 0xTBTBTBTBTBTBTBTB from 0xTB
li t3, REP_01
mul t3, t3, a1

add a2, a2, a0

iter_words:
ld t2, 0(a0)
xor t2, t2, t3
orc.b t2, t2
not t2, t2
bne t2, x0, found_word
addi a0, a0, REGSIZE
bne a0, a2, iter_words

iter_reminder:
beq t1, x0, exit
lb t4, 0(a0)
beq t4, a1, fw2
addi a0, a0, 1
addi t1, t1, -1
j iter_reminder

exit:
addi a0, x0, 0
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



