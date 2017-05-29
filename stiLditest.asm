	.orig x3000
ADD	R2, R2, #2
Sti	R2, Address
LdI	R2, Address 
Halt
Address .Fill x3007
.END