		.ORIG x3000
		LEA R1, NAME
		LD R3, NEGENTER
AGAIN2 		GETC
		OUT
		ADD R2, R0, R3
		BRz CONT
		STR R0, R1, #0
		ADD R1, R1, #1
		BR AGAIN2
CONT		AND R2, R2, #0	
		LEA R0, NAME
		PUTS
		AND R3, R3, #0		
		AND R0, R0, #0
		ADD R0, R0, xD		; Carriage return char			
		OUT
		AND R0, R0, #0
		ADD R0, R0, xA		; New Line char			
		OUT
		ADD R7, R7, #1
		LEA R0, NAME
		PUTS
		HALT
NEGENTER	.FILL xFFF6	
NAME		.BLKW #20 	
		.END			