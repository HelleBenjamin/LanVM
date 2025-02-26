start:
    vmgetmemsize
    ld bp, r0
    push 72
    push 101
    push 108
    push 108
    push 111
    push 32
    push 87
    push 111
    push 114
    push 108
    push 100
    push 33
    push 0
loop:
    cmp [bp-1], 0
    jz end
    out [bp-1]
    dec bp
    jmp loop
end:
    vmexit 0