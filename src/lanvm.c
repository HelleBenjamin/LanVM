/*  
 * Lanskern ByteCode - A Virtual Machine & Assembler 
 * Copyright (c) 2025 Benjamin Helle
 *  
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or  
 * (at your option) any later version.  
 *  
 * This program is distributed in the hope that it will be useful,  
 * but WITHOUT ANY WARRANTY; without even the implied warranty of  
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the  
 * GNU General Public License for more details.  
 *  
 * You should have received a copy of the GNU General Public License  
 * along with this program. If not, see <https://www.gnu.org/licenses/>.  
 */
#include "../include/lanvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

bool DEBUG = false;

int vm_exception(VM *vm, int code, int severity, char *fmt, ...) {
    if (code == ERR_NO_ERROR) return 0; // No error
    va_list args;
    va_start(args, fmt);
    printf("======================================================\nVM Runtime Exception: code %d severity %d at PC 0x%04x:\n", code, severity, vm->pc);
    switch (code) {
        case ERR_NO_ERROR:
            return 0;
        case ERR_OOB_OFF:
            printf("Offset out of bounds\n");
            printf("BP: %d\n", vm->bp);
            printf("Offset: %d\n", vm->program[vm->pc - 1]);
            printf("BP+Offset: %d\n", vm->bp+vm->program[vm->pc - 1]);
            printf("Valid address range: 0x0000 - 0x%04x\n", vm->stackSize);
            break;
        case ERR_OOB_REG:
            printf("Register indirect address out of bounds\nValid address range: 0x0000 - 0x%04x\n", vm->stackSize);
            break;
        case ERR_STACK_OVERFLOW:
            printf("Stack overflow\n");
            break;
        case ERR_STACK_UNDERFLOW:
            printf("Stack underflow\n");
            break;
        case ERR_INVALID_OPCODE:
            printf("Unknown opcode\n");
            break;
        case ERR_PC_OOB:
            printf("Program counter out of bounds\n");
            break;
        case ERR_MALLOC:
            printf("Memory allocation failed\n");
            printf("Requested size: %d\n", vm->program[vm->pc - 1]);
            break;
        case ERR_FREE:
            printf("Memory free failed\n");
            printf("Requested size: %d\n", vm->program[vm->pc - 1]);
            break;
        case ERR_DBZ:
            printf("Division by zero\n");
            break;
        case ERR_NULL_PTR:
            printf("Null pointer\n");
            break;
        default:
            printf("Unknown error\n");
            break;

    }
    printf("Additional information:\n");
    vprintf(fmt, args);
    printState(vm);
    printf("======================================================\n");
    va_end(args);
    if (severity == EXC_SEVERE) { // Exit if severity is severe
        vm_exit(vm, code);
    }
    return code;
}

uint8_t fByte(VM *vm) {
    return vm->program[vm->pc++];
}

uint16_t fWord(VM *vm) {
    uint16_t temp = vm->program[vm->pc++];
    temp |= vm->program[vm->pc++] << 8;
    return temp;
}

void push8(VM *vm, uint8_t value) {
    if (vm->sp == 0) {
        vm_exception(vm, ERR_STACK_OVERFLOW, EXC_SEVERE, 0);
        return;
    }
    vm->stack[--vm->sp] = value;
}

uint8_t pop8(VM *vm) {
    if (vm->sp > vm->stackSize) {
        return vm_exception(vm, ERR_STACK_UNDERFLOW, EXC_SEVERE, 0);
    }
    return vm->stack[vm->sp++];
}

void push16(VM *vm, uint16_t value) {
    if (vm->sp > vm->stackSize) {
        vm_exception(vm, ERR_STACK_OVERFLOW, EXC_SEVERE, 0);
        return;
    }
    vm->stack[--vm->sp] = value & 0xff;
    vm->stack[--vm->sp] = value >> 8;
}

uint16_t pop16(VM *vm) {
    if (vm->sp > vm->stackSize) {
        return vm_exception(vm, ERR_STACK_UNDERFLOW, EXC_SEVERE, 0);
    }
    uint16_t temp = vm->stack[vm->sp++] << 8;
    temp |= vm->stack[vm->sp++];
    return temp;
}

void modifyFlags(VM *vm, uint16_t result) {
    vm->flags[ZERO_FLAG] = result == 0 ? 1 : 0;
    vm->flags[SIGN_FLAG] = result < 0 ? 1 : 0;
    vm->flags[OVERFLOW_FLAG] = result < -32768 || result > 32767 ? 1 : 0;
    vm->flags[CARRY_FLAG] = (result & 0x10000) ? 1 : 0;
}

DSbyte decodeDS(uint8_t DSb) {
    DSbyte DS;
    DS.destReg = (DSb & 0xF0) >> 4;
    DS.srcReg = DSb & 0x0F;
    return DS;
}

uint8_t* RegOffset(VM *vm, bool reg, int8_t offset) {
    if (reg) { // SP
        if (vm->sp + offset > vm->stackSize || vm->sp + offset < 0) { // Prevent accessing memory out of bounds
            vm_exception(vm, ERR_OOB_OFF, EXC_WARNING, 0);
            return NULL;
        }
        return &vm->stack[vm->sp + offset];
    } else { // BP
        if (vm->bp + offset > vm->stackSize || vm->bp + offset < 0) { // Prevent accessing memory out of bounds
            vm_exception(vm, ERR_OOB_OFF, EXC_WARNING, 0);
            return NULL;
        }
        return &vm->stack[vm->bp + offset];
    }
}

void* GetDestination(VM *vm, uint8_t DSb) {
    uint16_t *dest[16] = {&vm->r[0], &vm->r[1], &vm->r[2], &vm->r[3], &vm->r[4], &vm->bp, &vm->sp, &vm->r[3], &vm->r[4], NULL, NULL, NULL, NULL, NULL, NULL};
    DSbyte DS = decodeDS(DSb);

    if (DS.destReg >= 7) { // Memory destination
        uint16_t addr = 0;
        if (DS.destReg == 7) { // [bp+offset8]
            return RegOffset(vm, 0, fByte(vm));
        } else if (DS.destReg == 8) { // [sp+offset8]
            return RegOffset(vm, 1, fByte(vm));
        }
        else if (DS.destReg == 9) addr = vm->r[3]; // [r3]
        else if (DS.destReg == 10) addr = vm->r[4]; // [r4]
        if (addr < 0 || addr > vm->stackSize) { // Prevent accessing memory out of bounds
            vm_exception(vm, ERR_OOB_REG, EXC_WARNING, "Indirect address: 0x%04x\n", addr);
            return NULL;
        }
        printf("ADDR: %d\n", addr);
        return &vm->stack[addr];
    } else { // Register destination
        return (uint16_t*)dest[DS.destReg];
    }

    return NULL;
}

uint16_t GetSource(VM *vm, uint8_t DSb) {
    uint16_t *src[16] = {&vm->r[0], &vm->r[1], &vm->r[2], &vm->r[3], &vm->r[4], &vm->bp, &vm->sp, &vm->r[3], &vm->r[4], NULL, NULL, NULL, NULL, NULL, NULL};
    DSbyte DS = decodeDS(DSb);

    if (DS.srcReg >= 7) { // Memory source
        uint16_t addr = 0;
        if (DS.srcReg == 7) { // [bp+offset8]
            return *RegOffset(vm, 0, fByte(vm));
        } else if (DS.srcReg == 8) { // [sp+offset8]
            return *RegOffset(vm, 1, fByte(vm));
        }
        else if (DS.srcReg == 9) addr = vm->r[3]; // [r3]
        else if (DS.srcReg == 10) addr = vm->r[4]; // [r4]
        if (addr < 0 || addr > vm->stackSize) { // Prevent accessing memory out of bounds
            vm_exception(vm, ERR_OOB_REG, EXC_WARNING, "Indirect address: 0x%04x\n", addr);
            return 0;
        }
        return vm->stack[addr];
    } else { // Register source
        return *src[DS.srcReg];
    }

    return 0;
}

uint16_t* GetReg(VM *vm, uint8_t reg) {
    uint16_t *regs[16] = {&vm->r[0], &vm->r[1], &vm->r[2], &vm->r[3], &vm->r[4], &vm->bp, &vm->sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    return regs[reg];
}


int vm_init(VM *vm, uint8_t *program) {
    for (int i = 0; i < 8; i++) vm->flags[i] = 0;
    for (int i = 0; i < 5; i++) vm->r[i] = 0;
    vm->sp = DEFAULT_STACK_SIZE; // Top of the stack
    vm->bp = 0;
    vm->pc = 0;
    vm->iv = 0;
    vm->stackSize = DEFAULT_STACK_SIZE;
    vm->stack = calloc(vm->stackSize, sizeof(uint8_t));
    vm->program = calloc(DEFAULT_PROGRAM_SIZE, sizeof(uint8_t));
    if (!vm->stack || !vm->program) {
        vm_exit(vm, ERR_MALLOC);
    }
    vm_load(vm, program);
    return 0;
}


int vm_restart(VM *vm) {
    for (int i = 0; i < 8; i++) vm->flags[i] = 0;
    for (int i = 0; i < 5; i++) vm->r[i] = 0;
    vm->sp = DEFAULT_STACK_SIZE; // Top of the stack
    vm->bp = 0;
    vm->pc = 0;
    vm->iv = 0;
    if (vm->stackSize != DEFAULT_STACK_SIZE) { // Set stack size back to default
        free(vm->stack);
        vm->stackSize = DEFAULT_STACK_SIZE;
        vm->stack = calloc(vm->stackSize, sizeof(uint8_t));
        if (!vm->stack) {
            vm_exit(vm, ERR_MALLOC);
            return -5;
        }
    }
    return 0;
}

int vm_exit(VM *vm, int8_t code) {
    free(vm->stack);
    free(vm->program);
    printf("VM exited with code %d\n", code);
    exit(code);
}

int vm_load(VM *vm, uint8_t *program) {
    if (program == NULL) {
        return -1;
    }
    size_t size = sizeof(uint8_t) * DEFAULT_PROGRAM_SIZE;
    memcpy(vm->program, program, size);
    return 0;
}

int vm_malloc(VM *vm, uint16_t size) { // Allocate stack memory, current stack size += size
    
    if ((vm->stackSize + size) > 0xFFFF) {
        vm_exception(vm, ERR_MALLOC, EXC_WARNING, "Stack allocation exceeds maximum size\n");
        return 1;
    }

    uint16_t new_stack_size = vm->stackSize + size;
    uint8_t *new_stack = (uint8_t *)realloc(vm->stack, new_stack_size);

    if (!new_stack) {
        vm_exception(vm, ERR_MALLOC, EXC_SEVERE, 0);
        return 1;
    }
    vm->stack = new_stack;
    vm->stackSize = new_stack_size;
    return 0;
}

int vm_free(VM *vm, uint16_t size) { // Free stack memory, current stack size -= size

    if (size >= vm->stackSize) {
        vm_exception(vm, ERR_FREE, EXC_WARNING, "Cannot free more memory than allocated\n");
        return 1;
    }

    uint16_t new_stack_size = vm->stackSize - size;
    uint8_t *new_stack = (uint8_t *)realloc(vm->stack, new_stack_size);

    if (!new_stack) {
        vm_exception(vm, ERR_MALLOC, EXC_SEVERE, 0);
        return 1;
    }

    vm->stack = new_stack;
    vm->stackSize = new_stack_size;

    if (vm->sp > vm->stackSize) { // Reset stack pointer if out of bounds
        vm->sp = vm->stackSize;
    }

    return 0;
}

int hypervisorCall(VM *vm, uint8_t operation, uint16_t operand) {
    switch (operation) {
        case 0x00: // VMEXIT
            vm_exit(vm, operand);
        case 0x01: // VMRESTART
            vm_restart(vm);
            return 0;
        case 0x02: // VMGETSTACKSIZE
            vm->r[0] = vm->stackSize; // Current stack size in r0
            return 0;
        case 0x05: // VMMALLOC
            return vm_malloc(vm, operand);
        case 0x06: // VMFREE
            return vm_free(vm, operand);
        default:
            return 0;
    }
}

void alu(VM *vm, ALU_OP op, uint16_t *dest, uint16_t src) {
    if (!dest) {
        vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null pointer passed to ALU\n");
        return;
    }
    switch(op) {
        case ALU_ADD:
            *dest += src;
            modifyFlags(vm, *dest);
            break;
        case ALU_SUB:
            *dest -= src;
            modifyFlags(vm, *dest);
            break;
        case ALU_AND:
            *dest &= src;
            modifyFlags(vm, *dest);
            break;
        case ALU_OR:
            *dest |= src;
            modifyFlags(vm, *dest);
            break;
        case ALU_XOR:
            *dest ^= src;
            modifyFlags(vm, *dest);
            break;
        case ALU_CMP:
            modifyFlags(vm, (int16_t)(*dest - src));
            break;
        case ALU_MUL:
            *dest *= src;
            modifyFlags(vm, *dest);
            break;
        case ALU_DIV:
            if (src == 0) {
                vm_exception(vm, ERR_DBZ, EXC_WARNING, 0);
                break;
            }
            *dest /= src;
            modifyFlags(vm, *dest);
            break;
        case ALU_NOT:
            *dest = ~*dest;
            modifyFlags(vm, *dest);
            break;
        case ALU_INC:
            *dest += 1;
            modifyFlags(vm, *dest);
            break;
        case ALU_DEC:
            *dest -= 1;
            modifyFlags(vm, *dest);
            break;
    }
}

int execute(VM *vm) {
    uint8_t opcode = fByte(vm);
    uint8_t DSb = 0;
    uint16_t* dest;
    switch(opcode) {

        // Load/Store
        case LD_dest_src:
            DSb = fByte(vm);
            dest = GetDestination(vm, DSb);
            if (!dest) {
                vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null ptr passed to LD\n");
                break;
            }
            *dest = GetSource(vm, DSb);
            break;
        case LD_dest_imm16:
            DSb = fByte(vm);
            dest = GetDestination(vm, DSb);
            if (!dest) {
                vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null ptr passed to LD\n");
                break;
            }
            *dest = fWord(vm);
            break;

        // Stack
        case PUSH_src:
            DSb = fByte(vm);
            push16(vm, GetSource(vm, DSb));
            break;
        case PUSH_imm16:
            push16(vm, fWord(vm));
            break;
        case POP_dest:
            DSb = fByte(vm);
            dest = GetDestination(vm, DSb);
            if (!dest) {
                vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null ptr passed to POP\n");
                break;
            }
            *dest = pop16(vm);
            break;
            
        // ALU
        case ADD_dest_src:
        case ADD_dest_imm16:
        case SUB_dest_src:
        case SUB_dest_imm16:
        case AND_dest_src:
        case AND_dest_imm16:
        case OR_dest_src:
        case OR_dest_imm16:
        case XOR_dest_src:
        case XOR_dest_imm16:
        case CMP_dest_src:
        case CMP_dest_imm16:
        case MUL_dest_src:
        case MUL_dest_imm16:
        case DIV_dest_src:
        case DIV_dest_imm16:
            {
                DSb = fByte(vm);
                dest = GetDestination(vm, DSb);
                if (opcode % 2 == 0) { // Even operations are dest, src
                    alu(vm, opcode, dest, GetSource(vm, DSb));
                } else { // Odd operations are dest, imm16
                    alu(vm, opcode-1, dest, fWord(vm));
                }
            }
            break;
        case NOT_dest:
            DSb = fByte(vm);
            dest = GetDestination(vm, DSb);
            alu(vm, ALU_NOT, dest, 0);
            break;
        case INC_dest:
            DSb = fByte(vm);
            dest = GetDestination(vm, DSb);
            alu(vm, ALU_INC, dest, 0);
            break;
        case DEC_dest:
            DSb = fByte(vm);
            dest = GetDestination(vm, DSb);
            alu(vm, ALU_DEC, dest, 0);
            break;

        // Control Flow
        case JMP_addr16:
            vm->pc = fWord(vm);
            break;
        case JZ_addr16:
            if (vm->flags[ZERO_FLAG]) {
                vm->pc = fWord(vm);
            } else vm->pc += 2;
            break;
        case JNZ_addr16:
            if (!vm->flags[ZERO_FLAG]) {
                vm->pc = fWord(vm);
            } else vm->pc += 2;
            break;
        case JC_addr16:
            if (vm->flags[CARRY_FLAG]) {
                vm->pc = fWord(vm);
            } else vm->pc += 2;
            break;
        case JNC_addr16:
            if (!vm->flags[CARRY_FLAG]) {
                vm->pc = fWord(vm);
            } else vm->pc += 2;
            break;
        case JLE_addr16:
            if (vm->flags[OVERFLOW_FLAG] || (vm->flags[SIGN_FLAG] != vm->flags[ZERO_FLAG])) {
                vm->pc = fWord(vm);
            } else vm->pc += 2;
            break;
        case JGE_addr16:
            if (vm->flags[OVERFLOW_FLAG] == vm->flags[OVERFLOW_FLAG]) {
                vm->pc = fWord(vm);
            } else vm->pc += 2;
            break;
        case JL_addr16:
            if (vm->flags[SIGN_FLAG] != vm->flags[OVERFLOW_FLAG]) {
                vm->pc = fWord(vm);
            } else vm->pc += 2;
            break;
        case JG_addr16:
            if (vm->flags[ZERO_FLAG] && (vm->flags[SIGN_FLAG] == vm->flags[OVERFLOW_FLAG])) {
                vm->pc = fWord(vm);
            } else vm->pc += 2;
            break;
        case CALL_addr16:
            push16(vm, vm->pc + 2);
            vm->pc = fWord(vm);
            break;
        case RET:
            vm->pc = pop16(vm);
            break;
        case RETI:
            vm->pc = pop16(vm);
            vm->flags[IA_FLAG] = false;
            break;
        case INT:
            if (vm->flags[IE_FLAG]) {
                push16(vm, vm->pc);
                vm->pc = vm->iv;
                vm->flags[IA_FLAG] = true;   
            }
            break;
        case EI:
            vm->flags[IE_FLAG] = true;
            break;
        case DI:
            vm->flags[IE_FLAG] = false;
            break;
        case CHK_INT:
            vm->flags[ZERO_FLAG] = vm->flags[IA_FLAG] ? 0 : 1;
            break;
        case PUSHF:
            {
                uint8_t temp = 0;
                temp |= vm->flags[ZERO_FLAG] << ZERO_FLAG;
                temp |= vm->flags[CARRY_FLAG] << CARRY_FLAG;
                temp |= vm->flags[OVERFLOW_FLAG] << OVERFLOW_FLAG;
                temp |= vm->flags[SIGN_FLAG] << SIGN_FLAG;
                temp |= vm->flags[HALT_FLAG] << HALT_FLAG;
                temp |= vm->flags[IA_FLAG] << IA_FLAG;
                temp |= vm->flags[IE_FLAG] << IE_FLAG;
                push8(vm, temp);
            }
            break;
        case POPF:
            {
                uint8_t temp = pop8(vm);
                vm->flags[ZERO_FLAG] = temp & (1 << ZERO_FLAG);
                vm->flags[CARRY_FLAG] = temp & (1 << CARRY_FLAG);
                vm->flags[OVERFLOW_FLAG] = temp & (1 << OVERFLOW_FLAG);
                vm->flags[SIGN_FLAG] = temp & (1 << SIGN_FLAG);
                vm->flags[HALT_FLAG] = temp & (1 << HALT_FLAG);
                vm->flags[IA_FLAG] = temp & (1 << IA_FLAG);
                vm->flags[IE_FLAG] = temp & (1 << IE_FLAG);
                vm->flags[7] = temp & (1 << 7);
            }
            break;

        // I/O
        case IN_dest:
            DSb = fByte(vm);
            dest = GetDestination(vm, DSb);
            if (!dest) {
                vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null ptr passed to IN\n");
                break;
            }
            *dest = getchar();
            break;
        case OUT_src:
            DSb = fByte(vm);
            printf("%c", GetSource(vm, DSb));
            break;

        // Hypervisor calls
        case VMEXIT:
            hypervisorCall(vm, 0x00, (uint16_t)fByte(vm));
            break;
        case VMRESTART:
            hypervisorCall(vm, 0x01, 0);
            break;
        case VMGETSTACKSIZE:
            hypervisorCall(vm, 0x02, 0);
            break;
        case VMSTATE: // Print current state, use for debugging
            printState(vm);
            break;
        case VMMALLOC:
            vm->flags[ZERO_FLAG] = hypervisorCall(vm, 0x05, fWord(vm)); // 0 = success, 1 = failure
            break;
        case VMFREE:
            vm->flags[ZERO_FLAG] = hypervisorCall(vm, 0x06, fWord(vm)); // 0 = success, 1 = failure
            break;

        case LEA_dest_bpoff:
            DSb = fByte(vm);
            dest = GetDestination(vm, DSb);
            if (!dest) {
                vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null ptr passed to LEA\n");
                break;
            }
            *dest = vm->bp + fByte(vm);
            break;

        case LIV_addr16:
            vm->iv = fWord(vm);
            break;

        case NOP:
            break;
        
        case HALT:
            vm->flags[HALT_FLAG] = true;
            break;
        default:
            vm_exception(vm, ERR_INVALID_OPCODE, EXC_WARNING, "Opcode: 0x%02x\n", opcode);
            return -1;
    }
}


void printState(VM *vm) {
    printf("Current state: \n"
        "r0=0x%04x r1=0x%04x r2=0x%04x r3=0x%04x r4=0x%04x\nSP=0x%04x BP=0x%04x PC=0x%04x F=0x%d%d%d%d%d%d%d%d\n",
        vm->r[0], vm->r[1], vm->r[2], vm->r[3], vm->r[4], vm->sp, vm->bp, vm->pc, vm->flags[7], vm->flags[6], vm->flags[5], vm->flags[4], vm->flags[3], vm->flags[2], vm->flags[1], vm->flags[0]
    );
}

void loadProgram(FILE *file, uint8_t *program) {
    uint8_t line[3];
    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n') break;
        program[i++] = (uint8_t)strtol(line, NULL, 16);
    }
    /*for (int j = 0; j < i; j++) { // Debug
        printf("%02x ", program[j]);
    }
    printf("\n");*/
}

int main(int argc, char **argv) {
    printf("LanVM v%s\n", VM_VERSION_STR);

    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    VM vm;

    FILE *file = fopen(argv[1], "rb"); // Open the program file
    if (!file) {
        printf("Error opening file\n");
        return 1;
    }
    uint8_t *program = calloc(DEFAULT_PROGRAM_SIZE, sizeof(uint8_t));
    if (!program) {
        printf("Memory allocation failed\n");
        fclose(file);
        return 1;
    }
    loadProgram(file, program);
    fclose(file);

    vm_init(&vm, program);

    free(program);

    while (vm.pc < DEFAULT_PROGRAM_SIZE && !vm.flags[HALT_FLAG]) {
        //printf("Instruction: 0x%02x PC: 0x%04x SP: 0x%04x R0: 0x%04x\n", vm.program[vm.pc], vm.pc, vm.sp, vm.r[0]); // Debug
        execute(&vm);
    }

    // If program didn't stop correctly, print state and exit with code 1
    printState(&vm);
    vm_exit(&vm, 1);

    return 0;
}