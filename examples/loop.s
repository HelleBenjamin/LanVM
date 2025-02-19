start:
	ld r0, 65535
loop:
	dec r0
	or r0, r0
	jnz loop
end:
	vmexit 0