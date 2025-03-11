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

DSbyte decodeDS(uint8_t DSb) {
  DSbyte DS;
  DS.destReg = (DSb & 0xF0) >> 4;
  DS.srcReg = DSb & 0x0F;
  return DS;
}

uint8_t* RegOffset(VM *vm, bool reg, int8_t offset) {
  if (reg) { // SP
      if (vm->sp + offset > vm->memSize || vm->sp + offset < 0) { // Prevent accessing memory out of bounds
          vm_exception(vm, ERR_OOB_OFF, EXC_WARNING, 0);
          return NULL;
      }
      return &vm->memory[vm->sp + offset];
  } else { // BP
      if (vm->bp + offset > vm->memSize || vm->bp + offset < 0) { // Prevent accessing memory out of bounds
          vm_exception(vm, ERR_OOB_OFF, EXC_WARNING, 0);
          return NULL;
      }
      return &vm->memory[vm->bp + offset];
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
      if (addr < 0 || addr > vm->memSize) { // Prevent accessing memory out of bounds
          vm_exception(vm, ERR_OOB_REG, EXC_WARNING, "Indirect address: 0x%04x\n", addr);
          return NULL;
      }
      return &vm->memory[addr];
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
      if (addr < 0 || addr > vm->memSize) { // Prevent accessing memory out of bounds
          vm_exception(vm, ERR_OOB_REG, EXC_WARNING, "Indirect address: 0x%04x\n", addr);
          return 0;
      }
      return vm->memory[addr];
  } else { // Register source
      return *src[DS.srcReg];
  }

  return 0;
}

uint16_t* GetReg(VM *vm, uint8_t reg) {
  uint16_t *regs[16] = {&vm->r[0], &vm->r[1], &vm->r[2], &vm->r[3], &vm->r[4], &vm->bp, &vm->sp, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  return regs[reg];
}