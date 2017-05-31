		.ORIG x3000
		LD	R6, STACKPTR
		LD	R4, NUM2
		JSR	SUM_3_4_2
		STI 	R4, STIADDR
		LDI 	R5, STIADDR
		HALT	

		
		;this is where we would add our push/pop 

SUM_3_4_2	ADD	R4, R4, #1
		ADD 	R5, R4, #3
		;this is where we would pop off the stack
		RET	
NUM1		.FILL	x0003
NUM2		.FILL	x0004
STACKPTR	.FILL	x3030
STIADDR		.FILL	x301A
		.END
	






STACKPTR	.FILL x3050
NUM1		.FILL x0004
NUM2 		.FILL x0003
STIADDR 	.FILL x3005
EXIT 		TRAP  x25
		.END	