/*  
 * Lanskern ByteCode - A Virtual Machine & Assembler  
 * Copyright (c) 2025 Benjamin Helle  
 *  
 * This file is part of Lanskern ByteCode.  
 *  
 * Lanskern ByteCode is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or  
 * (at your option) any later version.  
 *  
 * Lanskern ByteCode is distributed in the hope that it will be useful,  
 * but WITHOUT ANY WARRANTY; without even the implied warranty of  
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the  
 * GNU General Public License for more details.  
 *  
 * You should have received a copy of the GNU General Public License  
 * along with this program. If not, see <https://www.gnu.org/licenses/>.  
 */
#ifndef LANVM_H
#define LANVM_H

#define VM_VERSION 101
#define VM_VERSION_STR "1.0.1"

#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_MEMORY_SIZE 1024 // Sets the maximum memory size. Maximum memory size is 0xFFFF due to 16-bit registers
#define DEFAULT_PROGRAM_SIZE 2048

extern bool DEBUG; // Debug mode

typedef struct{
    uint8_t *memory; // RAM
    uint16_t memSize;
    uint8_t *program; // program
    uint16_t progSize;
    uint16_t pc;
    uint16_t iv; // interrupt vector
    uint16_t r[5]; // Accumulator, Data, Base, Destination, Source
    bool flags[8];
    uint16_t sp, bp;
} VM;

int vm_exception(VM *vm, int code, int severity, char *fmt, ...);

int vm_init(VM *vm, uint8_t *program);
int vm_restart(VM *vm);
int vm_load(VM *vm, uint8_t *program);
int vm_malloc(VM *vm, uint16_t size);
int hypervisorCall(VM *vm, uint8_t operation, uint16_t operand);
int vm_exit(VM *vm, int8_t code);

#define EXC_SEVERE 0
#define EXC_WARNING 1

#define ERR_NO_ERROR 0
#define ERR_OOB_OFF -1
#define ERR_OOB_REG -2
#define ERR_STACK_OVERFLOW -3
#define ERR_STACK_UNDERFLOW -4
#define ERR_INVALID_OPCODE -5
#define ERR_PC_OOB -6
#define ERR_MALLOC -7
#define ERR_FREE -8
#define ERR_DBZ -9
#define ERR_NULL_PTR -10
#define ERR_INVALID_ALU -11


/* Error & warning codes
    0 - No error
    -1 - Tried accessing memory out of bounds by offset
    -2 - Tried accessing memory out of bounds by register
    -3 - Stack overflow
    -4 - Stack underflow
    -5 - Invalid opcode
    -6 - Program counter out of bounds
    -7 - Failed to allocate memory
    -8 - Failed to free memory
    -9 - Division by zero
    -10 - Null pointer
    -11 - Invalid ALU operation
*/

/*  FLAGS
    0x01 - Carry
    0x02 - Zero
    0x04 - Overflow
    0x08 - Sign
    0x10 - Halt
    0x20 - Interrupt Enable
    0x40 - Interrupt Active

*/

typedef enum {
    r0, // Accumulator
    r1, // B
    r2, // C
    r3, // Destination
    r4  // Source
} GP_Registers;

typedef struct {
    uint8_t destReg;
    uint8_t srcReg;
} DSbyte;

void* GetDestination(VM *vm, uint8_t DSb);
uint16_t GetSource(VM *vm, uint8_t DSb);

#define CARRY_FLAG 0x00
#define ZERO_FLAG 0x01
#define OVERFLOW_FLAG 0x02
#define SIGN_FLAG 0x03
#define HALT_FLAG 0x04
#define IE_FLAG 0x05
#define IA_FLAG 0x06

typedef enum {
    // NOP
    NOP,

    // Load/Store
    LD_dest_src,        // ld dest, src
    LD_dest_imm16,      // ld dest, imm16

    // Stack
    PUSH_src,           // push src
    PUSH_imm16,         // push imm16
    POP_dest,           // pop dest

    // Arithmetic/Logical
    ADD_dest_src,       // add dest, src
    ADD_dest_imm16,     // add dest, imm16
    SUB_dest_src,       // sub dest, src
    SUB_dest_imm16,     // sub dest, imm16
    AND_dest_src,       // and dest, src
    AND_dest_imm16,     // and dest, imm16
    OR_dest_src,        // or  dest, src
    OR_dest_imm16,      // or  dest, imm16
    XOR_dest_src,       // xor dest, src
    XOR_dest_imm16,     // xor dest, imm16
    CMP_dest_src,       // cmp dest, src
    CMP_dest_imm16,     // cmp dest, imm16
    MUL_dest_src,       // mul dest, src
    MUL_dest_imm16,     // mul dest, imm16
    DIV_dest_src,       // div dest, src
    DIV_dest_imm16,     // div dest, imm16
    NOT_dest,           // not dest
    INC_dest,           // inc dest
    DEC_dest,           // dec dest

    // Control Flow
    JMP_addr16 = 0x20,  // jmp addr16
    JZ_addr16,          // jz addr16
    JNZ_addr16,         // jnz addr16
    JC_addr16,          // jc addr16
    JNC_addr16,         // jnc addr16
    JLE_addr16,         // jle addr16
    JGE_addr16,         // jge addr16
    JL_addr16,          // jl addr16
    JG_addr16,          // jg addr16
    CALL_addr16,        // call addr16
    RET,                // ret
    RETI,               // reti
    INT,                // int
    EI,                 // ei
    DI,                 // di
    CHK_INT,            // chk int ; check if executing isr, if executing zf = 1
    PUSHF,              // pushf
    POPF,               // popf
    SETZ_dest,          // setz dest
    SETNZ_dest,         // setnz dest
    SETL_dest,          // setl dest
    SETLE_dest,         // setle dest
    SETG_dest,          // setg dest
    SETGE_dest,         // setge dest
    SETB_dest,          // setb dest
    SETBE_dest,         // setbe dest
    SETA_dest,          // seta dest
    SETAE_dest,         // setae dest

    // I/O
    IN_dest = 0xf0,     // in dest
    OUT_src = 0xf1,      // out src

    // Hypervisor Calls
    VMEXIT = 0xd0,      // vmexit code
    VMRESTART,          // vmrestart
    VMGETMEMSIZE,       // vmgetmemsize
    VMSTATE,            // vmstate
    VMMALLOC = 0xd5,    // vmmalloc size
    VMFREE = 0xd6,      // vmfree size

    LEA_dest_bpoff = 0xfb,  // lea dest, boff
    LIV_addr16 = 0xfd,  // liv, imm16
    HALT = 0xfe         // hlt

} ISA;

typedef enum {
    ALU_ADD = 6,
    ALU_SUB = 8,
    ALU_AND = 10,
    ALU_OR = 12,
    ALU_XOR = 14,
    ALU_CMP = 16,
    ALU_MUL = 18,
    ALU_DIV = 20,
    ALU_NOT = 21,
    ALU_INC = 22,
    ALU_DEC = 23
} ALU_OP;

void printState(VM *vm);
int execute(VM *vm);
void alu(VM *vm, ALU_OP op, uint16_t *dest, uint16_t src);

void push8(VM *vm, uint8_t value);
void push16(VM *vm, uint16_t value);
uint8_t pop8(VM *vm);
uint16_t pop16(VM *vm);

#endif