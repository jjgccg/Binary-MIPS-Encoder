#ifndef __BINPARSER_H__
#define __BINPARSER_H__

#include <iostream>
#include <fstream>
#include "Instruction.h"
#include "RegisterTable.h"
#include "Opcode.h"
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <bitset>
using namespace std;

/*
  Joseph George
  CMSC 301 - Program 1
*/

/* This class reads in a MIPS assembly file and checks its syntax.  If
 * the file is syntactically correct, this class will retain a list 
 * of Instructions (one for each instruction from the file).  This
 * list of Instructions can be iterated through.
 */
   

class BINParser{
 public:
  // Specify a text file containing MIPS assembly instructions. Function
  // checks syntactic correctness of file and creates a list of Instructions.
  BINParser(string filename);

  // Returns true if the file specified was syntactically correct.  Otherwise,
  // returns false.
  bool isFormatCorrect() { return myFormatCorrect; };

  // Iterator that returns the next Instruction in the list of Instructions.
  Instruction getNextInstruction();

  // Checks that the format of the input file is correct - i.e. contains 32 bits
  bool checkBits(string Line);

 private:
  vector<Instruction> myInstructions;      // list of Instructions
  int myIndex;                             // iterator index
  bool myFormatCorrect;

  RegisterTable registers;                 // encodings for registers
  OpcodeTable opcodes;                     // encodings of opcodes
  int myLabelAddress;   // Used to assign labels addresses

  // Given an Opcode, a string representing the operands, and the number of operands, 
  // breaks operands apart and stores fields into Instruction.
  bool getOperands(Instruction &i, Opcode o, string *operand);

  // Decomposes a line of assembly code into strings for the opcode field and operands, 
  // checking for syntax errors and counting the number of operands.
  void getTokens(string line, string &opcode, string *operand, string& funct, InstType& instructionType);

  // Helper functions
  bool isWhitespace(char c)    { return (c == ' '|| c == '\t'); };
  bool isDigit(char c)         { return (c >= '0' && c <= '9'); };
  bool isAlphaUpper(char c)    { return (c >= 'A' && c <= 'Z'); };
  bool isAlphaLower(char c)    { return (c >= 'a' && c <= 'z'); };
  bool isSign(char c)          { return (c == '-' || c == '+'); };
  bool isAlpha(char c)         {return (isAlphaUpper(c) || isAlphaLower(c)); };
  

  // Converts a string to an integer.  Assumes s is something like "-231" and produces -231
  int  cvtNumString2Number(string s);

  // Converts an interger into string hexademical representation
  string hexFormat(int dec);

  //returns the string in decimal form of a binary number
  int btod(string strBin);

  // Given a valid instruction, returns a string representing the 32 bit MIPS binary encoding
  // of that instruction.
  string decode(Instruction i);
  
  // Respsonsible for returning the string representation of r-type instruction 
  // encoding given an Instruction
  string rDecode(Opcode op, Instruction i);

  // Respsonsible for returning the string representation of r-type instruction 
  // encoding given an Instruction
  string iDecode(Opcode op, Instruction i);

  // Respsonsible for returning the string representation of r-type instruction 
  // encoding given an Instruction
  string jDecode(Opcode op, Instruction i);

  // General algorithm that converts a binary string to a number for registers
  int btodForReg(string binStr);

  // Integer to string method
  string to_string(int num);

  // String to integer method
  int stoi(const string& s);

};

#endif
