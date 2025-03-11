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

void modifyFlags(VM *vm, uint16_t result) {
  vm->flags[ZERO_FLAG] = result == 0 ? 1 : 0;
  vm->flags[SIGN_FLAG] = (result & 0x8000) < 0 ? 1 : 0;
  vm->flags[OVERFLOW_FLAG] = result < -32768 || result > 32767 ? 1 : 0;
  vm->flags[CARRY_FLAG] = (result & 0x10000) ? 1 : 0;
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
      default:
          vm_exception(vm, ERR_INVALID_ALU, EXC_WARNING, 0);
          break;
  }
}

void handleSET(VM *vm, uint8_t op, uint8_t DSb) {
  DSbyte DS = decodeDS(DSb);
  uint16_t* dest = GetDestination(vm, DSb);
  if (!dest) {
      vm_exception(vm, ERR_NULL_PTR, EXC_WARNING, "Null ptr passed to SET\n");
      return;
  }
  switch (op) {
      case 0x32: // SETZ
          *dest = (vm->flags[ZERO_FLAG]) ? 1 : 0;
          break;
      case 0x33: // SETNZ
          *dest = (!vm->flags[ZERO_FLAG]) ? 1 : 0;
          break;
      case 0x34: // SETL
          *dest = (vm->flags[SIGN_FLAG] != vm->flags[OVERFLOW_FLAG]) ? 1 : 0;
          break;
      case 0x35: // SETLE
          *dest = (vm->flags[ZERO_FLAG] || vm->flags[SIGN_FLAG] != vm->flags[OVERFLOW_FLAG]) ? 1 : 0;
          break;
      case 0x36: // SETG
          *dest = (vm->flags[ZERO_FLAG] && vm->flags[SIGN_FLAG] == vm->flags[OVERFLOW_FLAG]) ? 1 : 0;
          break;
      case 0x37: // SETGE
          *dest = (vm->flags[SIGN_FLAG] == vm->flags[OVERFLOW_FLAG]) ? 1 : 0;
          break;
      case 0x38: // SETB
          *dest = (vm->flags[CARRY_FLAG]) ? 1 : 0;
          break;
      case 0x39: // SETBE
          *dest = (vm->flags[CARRY_FLAG] || vm->flags[ZERO_FLAG]) ? 1 : 0;
          break;
      case 0x3a: // SETA
          *dest = (!vm->flags[CARRY_FLAG] && !vm->flags[ZERO_FLAG]) ? 1 : 0;
          break;
      case 0x3b: // SETAE
          *dest = (!vm->flags[CARRY_FLAG]) ? 1 : 0;
          break;
      default:
          vm_exception(vm, ERR_INVALID_ALU, EXC_WARNING, "Invalid SET opcode\n");
          break;

  }
}