start:
	ld r0, 48
loop:
	cmp r0, 126
	jz end
    out r0
	inc r0
	jnz loop
end:
	vmexit 0