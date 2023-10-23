.equ REP_01, 0x0101010101010101
.equ REGSIZE, 8
.equ mask, 0xFF

.text
.global memchrasm

memchrasm:

# use byte-wide iteration for small numbers
add t1, x0, a2
sltiu t2, a2, 128
bnez t2, iter_reminder

# get the number of bytes we should iterate before alignment
andi t0, a0, REGSIZE - 1
beqz t0, continue
xor t0, t0, 0x07
addi t0, t0, 1

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
li t1, REGSIZE
j iter_reminder
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

li t4, REP_01
li t5, 0x8080808080808080

iter_words:
ld t2, 0(a0)

xor t2, t2, t3

sub t0, t2, t4

not t2, t2

and t0, t0, t2
and t0, t0, t5

bnez t0, found_word
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



.global memcmp_asm

memcmp_asm:
# a0 - data1
# a1 - data2
# a2 - count of bytes

# jump to byte iteration for small count
sltiu t0, a2, 256
bnez t0, byte_iter

# if addresses are unaligned and have different remainder of division by 8,
# jump to the byte iteration, as we can't compare data by words in this case
andi t0, a1, 0x07
andi t1, a0, 0x07
bne t0, t1, byte_iter

# get 8 - (count % 8)
xori t0, t0, 0x07
addi t0, t0, 1

# iterate over bytes until the both addresses are aligned
align_cycle:
beqz t0, continue2
lb t1, 0(a0)
lb t2, 0(a1)
addi a0, a0, 1
addi a1, a1, 1
addi t0, t0, -1
beq t1, t2, align_cycle

# we have found the unequal bytes, so return
blt t1, t2, retn
bgt t1, t2, retp

continue2:
# get the count remainder of division by 8. We will iterate over it at the end
srli t0, a2, 3
andi a2, a2, 0x07

iter_words2:
beqz t0, byte_iter
ld t1, 0(a0)
ld t2, 0(a1)
addi t0, t0, -1
addi a0, a0, 8
addi a1, a1, 8
beq t1, t2, iter_words2

# we have found the unequal words, iterate over them byte by byte
addi a0, a0, -8
addi a1, a1, -8
li a2, 8
j byte_iter

li a0, 0x1337
ret

ret0:
li a0, 0
ret

retn:
li a0, -1
ret

retp:
li a0, 1
ret

byte_iter:
beqz a2, ret0
lbu t1, 0(a0)
lbu t2, 0(a1)
addi a2, a2, -1
addi a0, a0, 1
addi a1, a1, 1
beq t1, t2, byte_iter

blt t1, t2, retn
bgt t1, t2, retp

li a0, 0x1337
ret


.global memcmp_orig
memcmp_orig:
addi	sp,sp,-16
sd	s0,8(sp)
addi	s0,sp,16
beqz	a2, l2
mv	a5,a0
add	a2,a2,a0
j	l2
l4:
beq	a5,a2, l3
l2:
lbu	a0,0(a5)
lbu	a4,0(a1)
addi	a5,a5,1
addi	a1,a1,1
subw	a0,a0,a4
beqz	a0, l4
l3:
ld	s0,8(sp)
addi	sp,sp,16
ret

l1:
li	a0,0
j	l3

