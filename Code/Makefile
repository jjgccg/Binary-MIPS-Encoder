# makefile

BIN: BIN.o Instruction.o Opcode.o RegisterTable.o BINParser.o
	g++ -o BIN BIN.o Opcode.o BINParser.o RegisterTable.o Instruction.o

BINParser.o: BINParser.h Opcode.h RegisterTable.h Instruction.h 

Instruction.o: Opcode.h RegisterTable.h Instruction.h 

Opcode.o: Opcode.h 

RegisterTable.o: RegisterTable.h 

clean:
	/bin/rm -f ASM *.o core