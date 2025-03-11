CC = gcc

SRC = src
VM_SRC = $(SRC)/lanvm
# Note: if building system binaries use /bin
BIN = bin

all:
	rm -rf $(BIN)
	mkdir $(BIN)
	$(CC) $(SRC)/lanvm.c -o $(BIN)/lanvm
	$(CC) $(SRC)/lasm.c -o $(BIN)/lasm

vm:
	$(CC) $(SRC)/lanvm.c -o $(BIN)/lanvm

asm:
	$(CC) $(SRC)/lasm.c -o $(BIN)/lasm