	.ORIG x3000
	LD	R6, STACK
	LD	R0, LEG1
	JSR	Sqr		; R0 <- R0^2
	ADD	R6, R6, #-1	; Push value of R0 onto stack 
	STR	R0, R6, #0
	LDR	R1, R6, #0	; R1 <- R0
	ADD	R6, R6, #1	; Pop R0's val off stack
	LD	R0, LEG2
	JSR	Sqr		; R0 <- R0^2
	ADD	R6, R6, #-1	; Push value of R0 onto stack 
	STR	R0, R6, #0
	LDR	R2, R6, #0	; R2 <- R0
	ADD	R6, R6, #1	; Pop R0's val off stack
	JSR	FndHyp
	HALT
;
STACK	.FILL	x4000
LEG1	.FILL	#5
LEG2	.FILL	#5
;
; Set R0 to the largest number such that R0^2 <= leg1^2 + leg2^2
;
FndHyp	ADD	R6, R6, #-3 
	STR	R3, R6, #0	; Push R3, R4 and R7 onto stack
	STR	R4, R6, #1
	STR	R7, R6, #2
	ADD	R3, R1, R2	; R3 <- leg1^2 + leg2^2
	NOT	R3, R3
	ADD	R3, R3, #1
	AND	R4, R4, 0	; R4 <- 1
check	ADD	R4, R4, #1
	ADD	R6, R6, #-1	; Push value of R4 onto stack 
	STR	R4, R6, #0
	LDR	R0, R6, #0	; R0 <- R4
	ADD	R6, R6, #1	; Pop R4's val off stack
	JSR	Sqr		; R0 <- R0^2
	ADD	R0, R0, R3	
	BRn	check		; Neg = still not found
	BRz	found		; Zero = found exact match
	ADD	R4, R4, #-1	; Pos = went too far, solution is one integer smaller
	BRnzp	found
;
found	ADD	R6, R6, #-1	; Push value of R4 onto stack 
	STR	R4, R6, #0
	LDR	R0, R6, #0	; R0 <- R4
	ADD	R6, R6, #1	; Pop R4's val off stack
	LDR	R3, R6, #0	; Pop R3, R4 and R7 off of stack
	LDR	R4, R6, #1	; and restore registers
	LDR	R7, R6, #2
	ADD	R6, R6, #3
	RET
	
;
; Takes value in R0 and squares it
;
Sqr	ADD	R6, R6, #-2 
	STR	R1, R6, #0	; Push R1 and R2 onto stack
	STR	R2, R6, #1
	ADD	R6, R6, #-1	; Push value of R0 onto stack 
	STR	R0, R6, #0
	LDR	R1, R6, #0	; R1 <- R0
	LDR	R2, R6, #0	; R2 <- R0
	ADD	R6, R6, #1	; Pop R0's val off stack
	ADD	R2, R2, #-1
loop	ADD	R0, R0, R1	; loop to square value in R0
	ADD	R2, R2, #-1
	BRp	loop
	LDR	R1, R6, #0	; Pop R1 and R2 off of stack
	LDR	R2, R6, #1	; and restore registers
	ADD	R6, R6, #2
	RET

;
	.END