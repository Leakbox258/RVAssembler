.bss
.data
.text
.globl main
main: 
	# call foo	
	auipc t1, %pcrel_hi(foo)
	jalr ra, t1, %pcrel_lo(foo)
	beq a0, x0, main_0
	jal x1, func0
main_0:
	ret

.global func0
func0:
	li a0, 42
	ret