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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "../include/lasm.h"
#include "../include/lanvm.h"

Instruction instructions[] = {
    {"NOP", NOP, 1}, {"LD", LD_dest_src, 2}, {"PUSH", PUSH_src, 2}, {"POP", POP_dest, 2},
    {"ADD", ADD_dest_src, 2}, {"SUB", SUB_dest_src, 2}, {"AND", AND_dest_src, 2}, {"OR", OR_dest_src, 2},
    {"XOR", XOR_dest_src, 2}, {"CMP", CMP_dest_src, 2}, {"MUL", MUL_dest_src, 2}, {"DIV", DIV_dest_src, 2}, 
    {"NOT", NOT_dest, 2}, {"INC", INC_dest, 2}, {"DEC", DEC_dest, 2},
    {"JMP", JMP_addr16, 3}, {"JZ", JZ_addr16, 3}, {"JNZ", JNZ_addr16, 3}, {"JC", JC_addr16, 3},
    {"JNC", JNC_addr16, 3}, {"JLE", JLE_addr16, 3}, {"JGE", JGE_addr16, 3}, {"JL", JL_addr16, 3},
    {"JG", JG_addr16, 3}, {"CALL", CALL_addr16, 3}, {"RET", RET, 1}, {"RETI", RETI, 1}, {"INT", INT, 1},
    {"EI", EI, 1}, {"DI", DI, 1}, {"CHK", CHK_INT, 1}, {"HLT", HALT, 1}, {"PUSHF", PUSHF, 1}, {"POPF", POPF, 1},
    {"IN", IN_dest, 2}, {"OUT", OUT_src, 2},
    {"SETZ", SETZ_dest, 2}, {"SETNZ", SETNZ_dest, 2}, {"SETL", SETL_dest, 2}, {"SETLE", SETLE_dest, 2},
    {"SETG", SETG_dest, 2}, {"SETGE", SETGE_dest, 2}, {"SETB", SETB_dest, 2}, {"SETBE", SETBE_dest, 2}, {"SETA", SETA_dest, 2}, {"SETAE", SETAE_dest, 2},
    {"VMEXIT", VMEXIT, 2}, {"VMRESTART", VMRESTART, 1}, {"VMGETMEMSIZE", VMGETMEMSIZE, 1}, {"VMSTATE", VMSTATE, 1}, {"VMMALLOC", VMMALLOC, 3}, {"VMFREE", VMFREE, 3},
    {"LIV", LIV_addr16, 3}, {"LEA", LEA_dest_bpoff, 4}
};

#define INSTRUCTION_COUNT (sizeof(instructions) / sizeof(Instruction))

Label labels[MAX_LABELS];
int label_count = 0;
uint16_t current_address = 0;


// Custom functions for the assembler
char *trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str) || *str == '\t') str++;
    if (*str == '\0') return str;
    end = str + strlen(str) - 1;
    while (end > str && (isspace((unsigned char)*end) || *end == '\t')) end--;
    *(end + 1) = '\0';
    return str;
}

void removeNL(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
        }
    }
}

void remove_leading_tabs_and_spaces(char *str) {
    char *start = str;
    while (*start == '\t' || *start == ' ') {
        start++;
    }
    memmove(str, start, strlen(start) + 1);
}

void LowerToCap(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') continue;
        str[i] = str[i] - 0x20;
    }
}

void store_label(const char *name, uint16_t address) {
    strcpy(labels[label_count].name, name);
    labels[label_count].address = address;
    label_count++;
}

uint16_t resolve_label(const char *operand) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, operand) == 0) {
            return labels[i].address;
        }
    }
    fprintf(stderr, "Error: Undefined label '%s'\n", operand);
    exit(1);
}


uint8_t get_opcode(const char *mnemonic) {
    for (size_t i = 0; i < INSTRUCTION_COUNT; i++) {
        if (strcmp(mnemonic, instructions[i].mnemonic) == 0) {
            return instructions[i].opcode;
        }
    }
    return 0xFF; // Invalid opcode
}

uint8_t get_opcode_size(const char *mnemonic) {
    for (size_t i = 0; i < INSTRUCTION_COUNT; i++) {
        if (strcmp(mnemonic, instructions[i].mnemonic) == 0) {
            return instructions[i].size;
        }
    }
    return 0x0;
}

uint8_t encode_operand(const char *operand, int8_t *offset) {

    if (operand == NULL) return 0;

    if (strcmp(operand, "r0") == 0) return 0x00;
    if (strcmp(operand, "r1") == 0) return 0x01;
    if (strcmp(operand, "r2") == 0) return 0x02;
    if (strcmp(operand, "r3") == 0) return 0x03;
    if (strcmp(operand, "r4") == 0) return 0x04;
    if (strcmp(operand, "bp") == 0) return 0x05;
    if (strcmp(operand, "sp") == 0) return 0x06;
    
    if (operand[0] == '[') {
        char base[4];
        int off = 0;
        if (sscanf(operand, "[%3[a-z]+%d]", base, &off) == 2 ||
            sscanf(operand, "[%3[a-z]-%d]", base, &off) == 2) {
            *offset = (operand[strlen(base) + 1] == '-') ? -off : off;
            if (strcmp(base, "bp") == 0) return 0x07;
            if (strcmp(base, "sp") == 0) return 0x08;
        } else if (sscanf(operand, "[%[^]]", base) == 1) {
            if (strcmp(base, "r3") == 0) return 0x09;
            if (strcmp(base, "r4") == 0) return 0x0A;
        }
    }
    return 0; // Return 0x0 if no match, defaults to r0
}

void genInsOffs(FILE *output, uint8_t opcode, char *operand1, char *operand2, int pass) { // Generate the instruction and offset bytes
    int8_t offset = 0;
    uint8_t ds_byte = (encode_operand(operand1, &offset) << 4) | encode_operand(operand2, &offset);
    if (pass == 2) {
        fprintf(output, "%02x%02x", opcode, ds_byte);
    }
    if (offset != 0 && pass == 2) {
        fprintf(output, "%02x", (offset & 0xFF));
    } if (offset != 0) current_address += 1;
}

void assemble_line(char *line, FILE *output, int pass) {
    if (line[strlen(line) - 2] == ':') {
        line[strlen(line) - 2] = '\0';  // Remove ':'
        if (pass == 1) store_label(line, current_address);
        return;
    } else if (line[0] == '\n') return;
    remove_leading_tabs_and_spaces(line); // Remove leading tabs and spaces
    char mnemonic[16], operand1[16], operand2[16] = "";

    int count = sscanf(line, "%s %[^,], %s", mnemonic, operand1, operand2);

    // Remove '\n' from end of strings. The newline char causes some problems.
    removeNL(mnemonic);
    removeNL(operand1);
    removeNL(operand2);
    LowerToCap(mnemonic);

    uint8_t opcode = get_opcode(mnemonic); // Get the opcode based on mnemonic
    if (opcode == 0xFF) {
        fprintf(stderr, "Error: Unknown instruction '%s'\n", mnemonic);
        return;
    }

    if (pass == 2 ) printf("%02x %s %s, %s\n", current_address, mnemonic, operand1, operand2);

    // Pass == 2 must be present when writing to the file otherwise the program will crash on pass 1

    if (count >= 3) { // ld, arith/logic
        if (opcode == LD_dest_src || opcode == LD_dest_imm16) { // ld
            if (isdigit(operand2[0])) { // ld dest, imm16
                current_address += 2;
                genInsOffs(output, LD_dest_imm16, operand1, NULL, pass);
                if (pass == 2) {
                    fprintf(output, "%02x%02x", (atoi(operand2) & 0xFF), (atoi(operand2) >> 8) & 0xFF);
                }

            } else {
                genInsOffs(output, opcode, operand1, operand2, pass);
            }
        } else if (opcode >= ADD_dest_src && opcode <= DIV_dest_imm16) {
            if (isdigit(operand2[0])) {// op dest, imm16
                current_address += 2;
                genInsOffs(output, opcode+1, operand1, NULL, pass); // opcode +1 = opcode with imm16
                if (pass == 2) {
                    fprintf(output, "%02x%02x", (atoi(operand2) & 0xFF), (atoi(operand2) >> 8) & 0xFF);
                }
            } else { // op dest, src
                genInsOffs(output, opcode, operand1, operand2, pass);
            }

        } else if (opcode == LEA_dest_bpoff) {
            genInsOffs(output, opcode, operand1, operand2, pass);
        }
    }

    else if (count == 2) { // push, pop, jmp, etc..
        if (opcode >= JMP_addr16 && opcode <= CALL_addr16) { // Jump and call
            uint16_t addr = (pass == 2) ? resolve_label(operand1) : 0;
            if (pass == 2) fprintf(output, "%02x%02x%02x", opcode, addr & 0xFF, (addr >> 8) & 0xFF);
        } else if (opcode == PUSH_imm16 || opcode == PUSH_src) { // Push
            if (isdigit(operand1[0])) { // Immediate
                current_address += 1;
                if (pass == 2) fprintf(output, "%02x%02x%02x", PUSH_imm16, atoi(operand1) & 0xFF, (atoi(operand1) >> 8) & 0xFF);
            } else { // Reg/mem
                genInsOffs(output, opcode, operand1, NULL, pass);
            }
        } else if (opcode == POP_dest || opcode >= NOT_dest && opcode <= DEC_dest || opcode == IN_dest) { // Instructions that have a destination 
            genInsOffs(output, opcode, operand1, NULL, pass);
        }else if (opcode == OUT_src) {
            genInsOffs(output, opcode, NULL, operand1, pass);
        } else if (opcode == VMEXIT || opcode >= VMMALLOC && opcode <= VMFREE) { // Hypervisor calls
            if (opcode == VMEXIT) {
                if (pass == 2) fprintf(output, "%02x%02x", opcode, atoi(operand1));
            } else {
                if (pass == 2) fprintf(output, "%02x%02x%02x", opcode, atoi(operand1) & 0xFF, (atoi(operand1) >> 8) & 0xFF);
            }
        } else if (opcode == LIV_addr16) {
            uint16_t addr = (pass == 2) ? resolve_label(operand1) : 0;
            if (pass == 2) fprintf(output, "%02x%02x%02x", opcode, addr & 0xFF, (addr >> 8) & 0xFF);
        } else if (opcode >= SETZ_dest && opcode <= SETA_dest) {
            genInsOffs(output, opcode, operand1, NULL, pass);
        }

    }

    else if (count == 1) { // ei, di, hlt..
        if (opcode >= RET && opcode <= POPF || opcode >= VMRESTART && opcode <= VMSTATE) {
            if (pass == 2) {
                fprintf(output, "%02x", opcode);
            }
        }
    }

    current_address += get_opcode_size(mnemonic);
}

int main(int argc, char **argv) {
    printf("LASM v%s\n", LASM_VERSION_STR);

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "wb");
    if (!input || !output) {
        fprintf(stderr, "Error opening files\n");
        return 1;
    }

    char line[64];

    // First pass: Collect labels
    while (fgets(line, sizeof(line), input)) assemble_line(line, NULL, 1);
    
    rewind(input);
    current_address = 0;

    printf("Pass 1 complete\n");
    printf("Labels (%d found):\n", label_count);
    for (int i = 0; i < label_count; i++) {
        if (label_count == 0) {
            printf("No labels found\n");
            break;
        }
        printf("%s: 0x%04x\n", labels[i].name, labels[i].address);
    }

    // Second pass: Code generation
    while (fgets(line, sizeof(line), input)) assemble_line(line, output, 2);

    fclose(input);
    fclose(output);
    printf("Assembled successfully!\n");
    return 0;
}
