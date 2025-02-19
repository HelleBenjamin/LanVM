# Lanskern Bytecode & Virtual Machine
Lanskern Bytecode & Virtual Machine(refered as LanCode and LanVM) is a bytecode interpreter designed for higher level programming languages. The bytecode is generated by a compiler and executed by the virtual machine. It's main goal is to make compiled programming languages portable across different platforms.

## VM Architecture
The VM uses a custom 16-bit ISA, similar to the x86 and RISC-V. LanVM supports hypervisor calls, for example, VMEXIT and VMMALLOC. The hypervisor is responsible for managing memory and handling exceptions.

### Registers
- R0-R4: 16-bit General Purpose Registers(Accumulator, B, C, Destination, Source)
- F: 8-bit Flags register
- SP: 16-bit Stack Pointer
- BP: 16-bit Base Pointer
- IP: 16-bit Instruction Pointer
- IV: 16-bit Interrupt Vector

### Memory
The VM has a maximum of 64KB of stack memory(RAM) and program memory. The memory is dynamically allocated. Initially, the VM allocates 1KB of stack memory. A program can allocate more memory using the `VMMALLOC` instruction. The program should check the zero flag to see if the operation was successful after executing `VMMALLOC`. To free memory, use the `VMFREE` instruction and check the zero flag.

## Instruction Set
The LanCode instruction set consists of different types of instructions, for example arithmetic operations, memory operations, control flow instructions, etc. The full instruction set is defined in the [ISA.md](ISA.md) file.

## Assembler
LASM is a simple and small assembler for the LanCode language. Currently it supports labels and comments in the assembly code.

## Speed
The VM is generally faster than interpreted languages such as Python, with a typical 2-3 times performance boost.

## Building
Prerequisites:
- C compiler, such as gcc
- Make
- Linux

Steps:
1. Clone the repository
2. Run `make` to build the VM. You can use `make vm` and `make asm` to build the VM and LASM respectively.

## Usage

### VM
Run `./bin/lanvm <program_file>` to run a program.

### LASM
Run `./bin/lasm <input_file> <output_file>` to assemble a program.

## License
This project is licensed under the **GNU General Public License v3.0 (GPLv3)**.  
You are free to use, modify, and distribute it under the same license terms.

See the **LICENSE** file for details.