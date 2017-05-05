; Daniel and Connor 
; 372 Problem 4

; Part 1
; The assembly program will prompt a user to enter their first name. It will allow the user to type in their name. The string
; should be stored in a memory location (BLKW #20). 

; Part 2a
; The program should then call a subroutine that "encrypts" the name by 
; subtracting a small constant from each character and replacing the original string with the new one. The subroutine will
; need the starting address of the string location passed to it in R1. (Get location of subroutine from R1)

; Part 3a (Function of Subroutine)
; It will loop through the string subtracting the constant from each character and replacing it in the array.

; Part 3b
; The subroutine return the starting address of the array in R0, (to indicate success) the program will print prompt
; "Press any key to continue: ".  

; Part 4
; When user presses key (without echo) the program prints the array (using trap x22). 


		.ORIG x3000
			LEA R1, PRESSKEY
	AGAIN	 	LDR R2, R1, #0
			BRz NEXT
			ADD R1, R1, #1
			BR AGAIN
	NEXT		LEA R0, PROMPT
			PUTS
			ADD R1, R1, #1
			LD R3, NEGENTER
		
	AGAIN2 		GETC
			OUT
			ADD R2, R0, R3
			BRz CONT
			STR R0, R1, #0
			ADD R1, R1, #1

			BR AGAIN2
	CONT		AND R2, R2, #0
			STR R2, R1, #0
	
			LEA R1, NAME
			LD R2, KEY
			AND R3, R3, #0
			
			AND R0, R0, #0
			ADD R0, R0, xD		; Carriage return char			
			OUT
			AND R0, R0, #0
			ADD R0, R0, xA		; New Line char			
			OUT
			ADD R7, R7, #1
			
JSRR R7	
	LOOP		
			LDR R3, R1, #0		; Load contents of [index R1] into R3		

			ADD R3, R3, R2		; Change contents of R3 to R3 + key
			

			AND R6, R6, #0		; Loop Check for #-3
			ADD R6, R3, #3
			BRz DONE
			
			STR R3, R1, #0		; Store contents of R3 back into Array
			ADD R1, R1, #1		; Increment to next index [R1 + 1]
			BR LOOP
			RET

	DONE
			

			
			LEA R0, PRESSKEY   	 ; 
			PUTS		 	 ; Printing Hello
			GETC
			LEA R0, NAME		 ;
			PUTS			 ; Printing Encrypted Name

		HALT
KEY		.FILL #-3
NEGENTER	.FILL xFFF6	
PROMPT		.STRINGZ "Please enter your name: "
PRESSKEY	.STRINGZ "Press any key to continue\n"
NAME		.BLKW #20 
		.END