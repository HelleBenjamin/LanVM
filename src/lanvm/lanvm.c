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

bool DEBUG = false;
bool GRAPHICS = false;

int vm_init(VM *vm, uint8_t *program) {
    for (int i = 0; i < 8; i++) vm->flags[i] = 0;
    for (int i = 0; i < 5; i++) vm->r[i] = 0;
    vm->sp = DEFAULT_MEMORY_SIZE; // Top of the stack
    vm->bp = 0;
    vm->pc = 0;
    vm->iv = 0;
    vm->memSize = DEFAULT_MEMORY_SIZE;
    vm->memory = calloc(vm->memSize, sizeof(uint8_t));
    vm->program = calloc(vm->progSize, sizeof(uint8_t));
    if (!vm->memory || !vm->program) {
        vm_exit(vm, ERR_MALLOC);
    }
    return 0;
}


int vm_restart(VM *vm) {
    for (int i = 0; i < 8; i++) vm->flags[i] = 0;
    for (int i = 0; i < 5; i++) vm->r[i] = 0;
    vm->sp = DEFAULT_MEMORY_SIZE; // Top of the stack
    vm->bp = 0;
    vm->pc = 0;
    vm->iv = 0;
    if (vm->memSize != DEFAULT_MEMORY_SIZE) { // Set stack size back to default
        free(vm->memory);
        vm->memSize = DEFAULT_MEMORY_SIZE;
        vm->memory = calloc(vm->memSize, sizeof(uint8_t));
        if (!vm->memory) {
            vm_exit(vm, ERR_MALLOC);
            return -5;
        }
    }
    return 0;
}

int vm_exit(VM *vm, int8_t code) {
    free(vm->memory);
    free(vm->program);
    printf("VM exited with code %d\n", code);
    langlExit(vm);
    exit(code);
}

int vm_load(VM *vm, uint8_t *program) {
    if (!program) {
        return -1;
    }
    memcpy(vm->memory, program, vm->progSize);
    return 0;
}

int vm_malloc(VM *vm, uint16_t size) { // Allocate memory, current memory size += size
    
    if ((vm->memSize + size) > 0xFFFF) {
        vm_exception(vm, ERR_MALLOC, EXC_WARNING, "Stack allocation exceeds maximum size\n");
        return 1;
    }

    uint16_t new_mem_size = vm->memSize + size;
    uint8_t *new_memory = (uint8_t *)realloc(vm->memory, new_mem_size);

    if (!new_memory) {
        vm_exception(vm, ERR_MALLOC, EXC_SEVERE, 0);
        return 1;
    }
    vm->memory = new_memory;
    vm->memSize = new_mem_size;
    return 0;
}

int vm_free(VM *vm, uint16_t size) { // Free memory, current memory size -= size

    if (size >= vm->memSize) {
        vm_exception(vm, ERR_FREE, EXC_WARNING, "Cannot free more memory than allocated\n");
        return 1;
    }

    uint16_t new_mem_size = vm->memSize - size;
    uint8_t *new_memory = (uint8_t *)realloc(vm->memory, new_mem_size);

    if (!new_memory) {
        vm_exception(vm, ERR_MALLOC, EXC_SEVERE, 0);
        return 1;
    }

    vm->memory = new_memory;
    vm->memSize = new_mem_size;

    if (vm->sp > vm->memSize) { // Reset stack pointer if out of bounds
        vm->sp = vm->memSize;
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
        case 0x02: // VMGETMEMSIZE
            vm->r[0] = vm->memSize; // Current stack size in r0
            return 0;
        case 0x05: // VMMALLOC
            return vm_malloc(vm, operand);
        case 0x06: // VMFREE
            return vm_free(vm, operand);
        default:
            return 0;
    }
}


void printState(VM *vm) {
    printf("Current state: \n"
        "r0=0x%04x r1=0x%04x r2=0x%04x r3=0x%04x r4=0x%04x\nSP=0x%04x BP=0x%04x PC=0x%04x F=0x%d%d%d%d%d%d%d%d\n",
        vm->r[0], vm->r[1], vm->r[2], vm->r[3], vm->r[4], vm->sp, vm->bp, vm->pc, vm->flags[7], vm->flags[6], vm->flags[5], vm->flags[4], vm->flags[3], vm->flags[2], vm->flags[1], vm->flags[0]
    );
}

void loadProgram(VM *vm,FILE *file, uint8_t *program) {
    uint8_t line[3];
    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n') break;
        program[i++] = (uint8_t)strtol(line, NULL, 16);
    }
    vm->progSize = i;
    memcpy(vm->memory, program, vm->progSize);
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
    vm_init(&vm, program);
    loadProgram(&vm, file, program);
    fclose(file);

    free(program);

    while (vm.pc < DEFAULT_PROGRAM_SIZE && !vm.flags[HALT_FLAG]) {
        //printf("Instruction: 0x%02x PC: 0x%04x SP: 0x%04x R0: 0x%04x\n", vm.memory[vm.pc], vm.pc, vm.sp, vm.r[0]); // Debug
        execute(&vm);
        if (GRAPHICS){ // TODO: Implement graphics rendering on a separate thread for performance
            if (glfwWindowShouldClose(vm.window) || vm.flags[HALT_FLAG]) break;
            langlRender(&vm);
            glfwPollEvents();
        }
    }

    // If program didn't stop correctly, print state and exit with code 1
    printState(&vm);
    vm_exit(&vm, 1);

    return 0;
}