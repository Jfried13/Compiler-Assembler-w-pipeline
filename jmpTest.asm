
		.ORIG 	x3000
		AND	R1, R1, #0
		LEA 	R3, NEXT
		JMP 	R3		;Should jump here
		ADD	R1, R1, #1	;Should not reach any of these
		ADD	R1, R1, #1
		ADD	R1, R1, #1
		ADD	R1, R1, #1
NEXT		HALT
		.END