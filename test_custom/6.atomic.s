.text
.global main
main:
# -----------------------
# AMOADD.W
# -----------------------
li t2, 5
amoadd.w t3, t2, (t0) # var1 += 5


# -----------------------
# AMOXOR.W
# -----------------------
li t2, 0xFF
amoxor.w t3, t2, (t0)


# -----------------------
# AMOOR.W
# -----------------------
li t2, 0x0F0F
amoor.w t3, t2, (t0)


# -----------------------
# AMOAND.W
# -----------------------
li t2, 0x00FF
amoand.w t3, t2, (t0)


# -----------------------
# AMOMIN.W
# -----------------------
li t2, 10
amomin.w t3, t2, (t0)


# -----------------------
# AMOMAX.W
# -----------------------
li t2, 1000
amomax.w t3, t2, (t0)


# -----------------------
# AMOMINU.W (unsigned)
# -----------------------
li t2, 0xFF00
amominu.w t3, t2, (t0)


# -----------------------
# AMOMAXU.W (unsigned)
# -----------------------
li t2, 0x7
amomaxu.w t3, t2, (t0)


# -----------------------
# LR/SC test loop
# -----------------------
LR_SC_TEST:
lr.w t4, (t1) # load reserved
addi t4, t4, 1 # modify
sc.w t5, t4, (t1) # store conditional
bnez t5, LR_SC_TEST # retry if SC failed


# End
li a0, 0
ret


# -------------------------
# Data section
# -------------------------


.data
var1:
.word 0x12345678
var2:
.word 0x10