# The full instruction set of Lanskern Bytecode

## DS Byte
The DS byte is used to specify the destination and source registers/immediate for an instruction.

| D/S  | DEST      | SRC       |
|------|-----------|-----------|
| 0000 | r0        | r0        |
| 0001 | r1        | r1        |
| 0010 | r2        | r2        |
| 0011 | r3        | r3        |
| 0100 | r4        | r4        |
| 0101 | bp        | bp        |
| 0110 | sp        | sp        |
| 0111 | [bp+off8] | [bp+off8] |
| 1000 | [sp+off8] | [sp+off8] |
| 1001 | [r3]      | [r3]      |
| 1010 | [r4]      | [r4]      |

### 1. Load/Store Instructions
    0x01 LD dest, src
    0x02 LD dest, imm16
    
### 2. Stack Instructions
    0x03 PUSH src
    0x04 PUSH imm16
    0x05 POP dest

### 3. Arithmetic/Logical Operations
    0x06    ADD dest, src
    0x07    ADD dest, imm16
    0x08    SUB dest, src
    0x09    SUB dest, imm16
    0x0A    AND dest, src
    0x0B    AND dest, imm16
    0x0C    OR  dest, src
    0x0D    OR  dest, imm16
    0x0E    XOR dest, src
    0x0F    XOR dest, imm16
    0x10    CMP dest, src
    0x11    CMP dest, imm16
    0x12    MUL dest, src
    0x13    MUL dest, imm16
    0x14    DIV dest, src
    0x15    DIV dest, imm16
    0x16    NOT dest
    0x17    INC dest
    0x18    DEC dest


### 4. Control Flow Instructions
    0x00    NOP
    0x20    JMP addr16
    0x21    JZ addr16
    0x22    JNZ addr16
    0x23    JC addr16
    0x24    JNC addr16
    0x25    JLE addr16
    0x26    JGE addr16
    0x27    JL addr16
    0x28    JG addr16
    0x29    CALL addr16
    0x2A    RET
    0x2B    RETI
    0x2C    INT
    0x2D    EI
    0x2E    DI
    0x2F    CHK INT
    0x30    PUSHF
    0x31    POPF
    0xFB    LEA dest, [bp+offset]
    0xFD    LIV addr16
    0xFE    HLT

### 5. I/O Instructions
    0xF0    IN dest
    0xF1    OUT src

### 6. Hypervisor Calls
    0xd0    VMEXIT code
    0xd1    VMRESTART
    0xd2    VMGETSTACKSIZE
    0xd3    VMSTATE
    0xd5    VMMALLOC size16
    0xd6    VMFREE size16
