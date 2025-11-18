.bss
.data
.text
.globl main
main: 
main_0: # rv32/64 I
	and x1, x2, x3
	or x1, x2, x3
	andi x1, x2, 255 # 0xff
	ori x1, x2, 1
	xori x1, x2, -1
	sll x1, x2, x3
	srl x1, x2, x3
	sra x1, x2, x3
	slli x1, x2, 3
	srli x1, x2, 4
	slt x1, x2, x3
	lw x1, 0(sp)
	lbu x1, 0(x2)
	ecall
	ebreak
	ret