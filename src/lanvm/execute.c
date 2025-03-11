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
#include "../include/lanvm.h"

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

    case SETZ_dest:
    case SETNZ_dest:
    case SETL_dest:
    case SETLE_dest:
    case SETG_dest:
    case SETGE_dest:
    case SETB_dest:
    case SETBE_dest:
    case SETA_dest:
    case SETAE_dest:
        handleSET(vm, opcode, DSb);
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
    case GETS_r4:
        {
            char c;
            dest = GetDestination(vm, 0xA0);
            if (!dest) {
                vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null ptr passed to GETS\n");
                break;
            }
            *dest = '\0';
            while ((c = getchar()) != '\n') {
                *dest = c;
                dest++;
                vm->r[r3]++;
            }
        }
        break;
    case PRINTS_r3:
        {
            char c = '\0';
            uint16_t* src = GetDestination(vm, 0x90);
            if (!src) {
                vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null ptr passed to PRINTS\n");
                break;
            }
            while ((c = *src++) != '\0') {
                printf("%c", c);
                vm->r[r3]++;
            }
        }
        break;

    // Hypervisor calls
    case VMEXIT:
        hypervisorCall(vm, 0x00, (uint16_t)fByte(vm));
        break;
    case VMRESTART:
        hypervisorCall(vm, 0x01, 0);
        break;
    case VMGETMEMSIZE:
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

    // Graphics
    case GLINIT:
        GRAPHICS = true;
        vm->screenWidth = vm->r[1];
        vm->screenHeight = vm->r[2];
        langlInit(vm);
        break;
    case GLCLEAR:
        langlClear(vm);
        break;
    case GLSETCOLOR:
        langlSetColor(vm,vm->r[1]);
        break;
    case GLPLOT:
        langlPlot(vm, vm->r[1], vm->r[2]);
        break;
    case GLLINE:
        langlLine(vm, vm->r[1], vm->r[2], vm->r[3], vm->r[4]);
        break;
    case GLRECT:
        langlRect(vm, vm->r[1], vm->r[2], vm->r[3], vm->r[4]);
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