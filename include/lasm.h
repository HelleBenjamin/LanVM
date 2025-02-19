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
#ifndef LASM_H
#define LASM_H

#define MAX_LABELS 256

#define LASM_VERSION 100
#define LASM_VERSION_STR "1.0.0"

// Instruction table
typedef struct {
    const char *mnemonic;
    uint8_t opcode;
    uint8_t size; // How many bytes the instruction takes
} Instruction;

typedef struct {
    char name[32];
    uint16_t address;
} Label;

#endif