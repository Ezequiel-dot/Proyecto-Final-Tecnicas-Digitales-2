.text
.arm

.global calculoLectura

calculoLectura:
	MOV R0, R0, LSL #1
	ADD R0, R0, #10
	
	MOV PC,LR
