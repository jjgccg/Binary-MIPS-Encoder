#include "BINParser.h"
#include <stack>
#include <string>
#include <sstream>
#include <iostream>
#include <bitset>
#include <math.h>

/*
  Joseph George
  CMSC 301 - Program 1
*/

/*
  Taking in a text file containing the Binary Encoding of MIPS instructions,
  checks syntactic correctness of file and creates a list of instructions.
*/
BINParser::BINParser(string filename)
{
  Instruction i;
  myFormatCorrect = true;

  ifstream in;
  in.open(filename.c_str());
  if(in.bad()){
    myFormatCorrect = false;
  }
  else{
    string line;

    while( getline(in, line)){
      bool isRightAmount = checkBits(line); //checks to make sure each line is 32 bits

      if(isRightAmount == false)
      {
        myFormatCorrect = false;
        cout << "ERROR: Incorrect length of binary encoding";
        break;
      }

      else
      {
        string opcode("");
        string operand[80];
        string funct("");
        InstType instructionType;

        getTokens(line, opcode, operand, funct, instructionType);
        
        Opcode o = opcodes.getOpcode(opcode, funct);   
        if(o == UNDEFINED){
          //invalid opcode specified
          myFormatCorrect = false;
          break;
        }

        bool success = getOperands(i, o, operand);
        if(!success){
          myFormatCorrect = false;
          break;
        }
      }
      string encoding = decode(i); //decode binary encoding
      i.setEncoding(encoding);

      myInstructions.push_back(i);

    }
  }

  myIndex = 0;
}

/*
  Iterator that returns the next instruction in the list of instructions
*/
Instruction BINParser::getNextInstruction()
{
  if(myIndex < (int)(myInstructions.size())){
    myIndex++;
    return myInstructions[myIndex-1];
  }
  
  Instruction i;
  return i;

}
	
/*
  Breaks apart and stores fields into instructions based on the binary encoding
  of the MIPS instructions, based on an Opcode, a string representing the operands,
  and the number of operands
*/
bool BINParser::getOperands(Instruction &i, Opcode o, string *operand)
{
  int rs = 0;
  int rd = 0;
  int rt = 0;
  int imm = 0;

  //Opcode object passed in provides position of these
  int rs_p = opcodes.RSposition(o);
  int rt_p = opcodes.RTposition(o);
  int rd_p = opcodes.RDposition(o);
  int imm_p = opcodes.IMMposition(o);
  bool hasAddress = opcodes.isAddress(o);
  InstType instType = opcodes.getInstType(o); //RTYPE, JTYPE, ITYPE

  //SET POSITIONS OF ENCODING

  //DEAL WITH RTYPE
  if(instType == RTYPE) //basic RTYPE format
  {
    rd_p = 0;
    rs_p = 1;
    rt_p = 2;
  }

  //ITYPE
  if(instType == ITYPE){ 
    if((hasAddress == true) || imm_p == 1){ //has an immediate or has an address - i.e. BEQ
      rs_p = 0;
      rt_p = 1;
      imm_p = 2; 
    }
    if(imm_p != -1) //just an immediate
    {
      imm = btod(operand[imm_p]);
    }
  }

  //assigns appropriate register types by using the parsed binary
  if(rs_p != -1){
    rs = btodForReg(operand[rs_p]);
  }
  if(rt_p != -1){
    rt = btodForReg(operand[rt_p]);
  }
  if(rd_p != -1){
    rd = btodForReg(operand[rd_p]);
  }

  //R,I,J types containing immediate values
  if(imm_p != -1 && instType == RTYPE) //RTYPE that has immediate field
  {
    imm_p = 3;
    imm = btodForReg(operand[imm_p]);
  }
  if(imm_p != -1 && instType == ITYPE)
  {
    imm = btodForReg(operand[imm_p]); //sets immediate to appropriate register
  }
  //DEAL WITH JYTPE
  if(instType == JTYPE)
  {
    imm_p = 0; //always zero in this type
    imm = btodForReg(operand[imm_p]); //sets immediate to appropriate register
  }
  
  i.setValues(o, rs, rt, rd, imm);

  return true;
}

/*
  This method breaks the binary encoding into chunks to use later when 
  the binary representation is being translated into MIPS instructions
*/
void BINParser::getTokens(string line, string &opcode, string *operand, string& funct, InstType& instructionType)
{
    opcode = line.substr(0,6);   //gives opcode as first 6 bits
    Opcode anOpcode = opcodes.getOpcode(opcode);
    instructionType = opcodes.getInstType(anOpcode);

    //check for incompatible types
    if(instructionType != RTYPE && instructionType != ITYPE && instructionType != JTYPE)
    {
      cerr << "ERROR: Type not known";
    }

    //Case where instruction is an RTYPE
    if(instructionType == RTYPE)    
    {
        operand[1] = line.substr(6,5); //rs section
        operand[2] = line.substr(11,5); //rt section
        operand[0] = line.substr(16,5); //rd section
        operand[3] = line.substr(21,5); //shammt section
        operand[4] = line.substr(26,6); //funct section

        funct = opcodes.getFunctField(opcodes.getOpcode(opcode, operand[4])); //function field
    }
    //Case where instruction is an ITYPE
    else if(instructionType == ITYPE)   
     {

      if(opcodes.isAddress(anOpcode) == true)//is immidiate field  an address 
      {
          operand[0] = line.substr(6,5); //rs section
          operand[1] = line.substr(11, 5); //rt section
          operand[2] = (line.substr(16, 16)+ "00"); //shift the immediate
      }
      //Case where immediate field is not actually an address
      else
      {      
          operand[0] = line.substr(6,5); //rs section
          operand[1] = line.substr(11, 5); //rt section
          operand[2] = line.substr(16, 16); //immediate section
      }    

    }
    //Case where instruction is a JTYPE
    else
    { 
          operand[0] = "0000" + line.substr(6, 26) + "00"; //shift immediate field for addressing
    }

  }

/*
  Returns an integer representing the decimal equivalent of a
  two's complement binary string passed in.
*/
int BINParser::btod(string strBin)
{
  int binPos = strBin.length() - 1;
  int dec;
  //check the case where the first is 1 - this means negative
  if(strBin[0] == '1')
  {
    dec = -1*pow(2, binPos); //two to power of length - 1 and mult by -1 to get first val
  }

  //iterate through the rest of the bin number to add values and get the decimal
  for(int i = 1; i < strBin.length(); i++)
  {
    binPos--; //decrement length of binary number
    if(strBin[i] == '1')
    {
      dec = dec + pow(2, binPos);
    }

  }

  return dec;

}

/*
  Converts an integer decimal taken in its hexademical representation,
  which is returned as a string.
*/
string BINParser::hexFormat(int dec)
{
  stringstream ss;
  ss << "0x" << hex << dec;
  string hex(ss.str());
  return hex;
}

/*
  A boolean method which, taking in a string that will be the line of a 
  file, checks to make sure this line contains 32 bits.  If it does, the
  method returns true, and false otherwise.
*/
bool BINParser::checkBits(string line)
{
    if(line.length()-1 != 32)
    {
      if(line.length()-1 != 31) //account for how C++ parses the file
      {
        return false;
      }
    }


    return true;
}


/*
  Determines the MIPS assembly code that is equivalent to its encoded
  binary representation given an Instruction.
*/
string BINParser::decode(Instruction i)
{
    Opcode op = i.getOpcode(); //get type of instruction
    string parsedBin = ""; //will hold decoded binary value

    if(opcodes.getInstType(op) == RTYPE) //RTYPE
    {
      parsedBin = rDecode(op, i);
    }
    else if(opcodes.getInstType(op) == ITYPE) //ITYPE
    {
      parsedBin = iDecode(op, i);
    }
    else //jtype
    {
      parsedBin = jDecode(op, i);
    }

    return parsedBin;
}


/*
  Subroutine used to decode RTYPE instructions.
*/
string BINParser::rDecode(Opcode op, Instruction i)
{
  string rtype = opcodes.getName(op) + " "; //string holds instruction

  //first get the name of opcode
  int rd_p = opcodes.RDposition(op);
  int rs_p = opcodes.RSposition(op);
  int rt_p = opcodes.RTposition(op);
  int imm_p = opcodes.IMMposition(op);

  for(int position = 0; position < opcodes.numOperands(op); position++)
  {
    if(rd_p == position)
    {
      rtype = rtype + "$" + this->to_string(i.getRD()) + ", ";
    }
    else if(imm_p == position)
    {
      rtype = rtype + to_string(i.getImmediate());
    }
    else if(rs_p == position)
    {
      rtype = rtype + "$" + to_string(i.getRS()) + ", ";
    }
    else if(rt_p == position)
    {
      rtype = rtype + "$" + to_string(i.getRT()) + ", ";
    }
  }

  return rtype;
}

/*
  Subroutine used to decode ITYPE instructions.
*/
string BINParser::iDecode(Opcode op, Instruction i)
{
    string itype = opcodes.getName(op) + " ";

    //if immediate position is 1, this is a load/store instruction!
    if(opcodes.IMMposition(op) == 1)
    {
      if(opcodes.RTposition(op) != -1)
      {
        itype = itype + "$" + to_string(i.getRT()) + ", "; //RT
      }

      itype = itype + to_string(i.getImmediate());//IMMEDIATE
      if(opcodes.RSposition(op) != -1)
      {
        itype = itype + "(" + "$" + to_string(i.getRS()) + ")"; //(RS)
      }
    }

    else //all other i-type instructions
    {
      //check if immedaite field is an address - i.e. instructions like "beq"
      if(opcodes.isAddress(op))
      {
        itype = itype + "$" + to_string(i.getRT()) + ", ";
        itype = itype + "$" + to_string(i.getRS()) + ", ";
        itype = itype + hexFormat(i.getImmediate());
      }
      //immediate field is not an address - format another way
      else
      {
        itype = itype + "$" + to_string(i.getRT()) + ", ";
        itype = itype + "$" + to_string(i.getRS()) + ", ";
        itype = itype + to_string(i.getImmediate()); 
      }

    }

    return itype;

}

/*
  Subroutine used to decode JTYPE instructions.
*/
string BINParser::jDecode(Opcode op, Instruction i)
{
    string jtype = opcodes.getName(op) + " "; 
    if(opcodes.isIMMLabel(op) == true)
    {
      cout << i.getImmediate() << endl;
      jtype = jtype + hexFormat(i.getImmediate());
    }

    return jtype;
}

/*
  General method that converts a binary string into its decimal equivalent.
*/
int BINParser::btodForReg(string binStr)
{
  int binInt = stoi(binStr);

  int decimal = 0;
  int i = 0;
  int rem;

  while(binInt != 0)
  {
    rem = binInt % 10;
    binInt = binInt/10;
    decimal = decimal + rem*pow(2, i);
    i++;
  }

  return decimal;
}

/*
  Method for converting an integer to a string
*/
string BINParser::to_string(int num)
{
    stringstream ss;
    ss << num;
    return ss.str();
}

/*
  Method for converting a string to integer
*/
int BINParser::stoi(const string& s)
{
  //cout << "string is: " << s;
  stringstream str(s);
  int i;
  str >> i;
  return i;
}
