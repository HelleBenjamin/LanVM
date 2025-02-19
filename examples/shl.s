shl_loop:
	or r2, r2
	jz end_shl
	add r1, r1
	dec r2
	jnz shl_loop
end_shl:
	ret