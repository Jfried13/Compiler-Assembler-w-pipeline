	.orig x3000
ADD	R2, R2, #2
ADD	R3, R3, #-4
Sti	R2, Address
Sti	R3, Address2
ADD	R2, R2, #1
ADD	R3, R3, #5
LdI	R2, Address 
Ldi	R3, Address2
Halt
Address 	.Fill x300B
Address2	.Fill x300C
.END