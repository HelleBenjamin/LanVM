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
          printf("Valid address range: 0x0000 - 0x%04x\n", vm->memSize);
          break;
      case ERR_OOB_REG:
          printf("Register indirect address out of bounds\nValid address range: 0x0000 - 0x%04x\n", vm->memSize);
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
          break;
      case ERR_FREE:
          printf("Memory free failed\n");
          break;
      case ERR_DBZ:
          printf("Division by zero\n");
          break;
      case ERR_NULL_PTR:
          printf("Null pointer\n");
          break;
      case ERR_INVALID_ALU:
          printf("Invalid ALU operation\n");
          break;
      case ERR_GRAPHICS:
          printf("Graphics error\n");
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