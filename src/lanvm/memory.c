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

uint8_t fByte(VM *vm) {
  return vm->memory[vm->pc++];
}

uint16_t fWord(VM *vm) {
  uint16_t temp = vm->memory[vm->pc++];
  temp |= vm->memory[vm->pc++] << 8;
  return temp;
}

void push8(VM *vm, uint8_t value) {
  if (vm->sp == 0) {
      vm_exception(vm, ERR_STACK_OVERFLOW, EXC_SEVERE, 0);
      return;
  }
  vm->memory[--vm->sp] = value;
}

uint8_t pop8(VM *vm) {
  if (vm->sp > vm->memSize) {
      return vm_exception(vm, ERR_STACK_UNDERFLOW, EXC_SEVERE, 0);
  }
  return vm->memory[vm->sp++];
}

void push16(VM *vm, uint16_t value) {
  if (vm->sp > vm->memSize) {
      vm_exception(vm, ERR_STACK_OVERFLOW, EXC_SEVERE, 0);
      return;
  }
  vm->memory[--vm->sp] = value & 0xff;
  vm->memory[--vm->sp] = value >> 8;
}

uint16_t pop16(VM *vm) {
  if (vm->sp > vm->memSize) {
      return vm_exception(vm, ERR_STACK_UNDERFLOW, EXC_SEVERE, 0);
  }
  uint16_t temp = vm->memory[vm->sp++] << 8;
  temp |= vm->memory[vm->sp++];
  return temp;
}