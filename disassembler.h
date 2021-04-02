#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <algorithm>

#define BIN_LIST_SIZE 16

class DisAssembler 
{

public:

    DisAssembler();     // Contructor
    ~DisAssembler();    // Destructor

/**
    Reads the object code file and stores it in a vector called "objLines" for processing
    @param fileName A character ptr containing the name of the file to open
    @return void
*/
    void ReadinObjectCode(char* fileName);


/**
    Reads the symbol table file iand stores it in a map called "symbolTable" for lookup purpose
    @param filName A character ptr containing the name of the file to open
    @return void
*/
    void ReadinSymbolTable(char* fileName);


/**
    Perform the translation from Object Code program into Assembly Launguage program
    @param none
    @return void
*/
    void ObjectCode2AssemblyCode();    



private:

    bool baseRegisterActive = false;
    bool XRegisterFlag = false;
    unsigned int baseRegister, PCRegister, XRegister, ARegister, fullRecordLength, mostRecentMemoryAddress, counter = 0;
    

    const std::string registerTable = "AXLBSTF";
    const std::string hexDigits = "0123456789ABCDEF";
    const std::string binaryNums[BIN_LIST_SIZE] = { "0000", "0001", "0010", "0011", "0100", 
                                                    "0101", "0110", "0111", "1000", "1001", 
                                                    "1010", "1011", "1100", "1101", "1110","1111" };
    
    std::vector<std::string> objLines;
    std::map < unsigned int, std::pair<std::string, std::string> > symbolTable;

    std::map <std::string, std::pair<std::string, int> > opcodeTable = {
        {"18", {"ADD", 3}},  {"58", {"ADDF", 3}},   {"90", {"ADDR", 2}},   {"40", {"AND", 3}},  {"B4", {"CLEAR", 2}},
        {"28", {"COMP", 3}}, {"88", {"COMPF", 3}},  {"A0", {"COMPR", 2}},  {"24", {"DIV", 3}},  {"64", {"DIVF", 3}},
        {"9C", {"DIVR", 2}}, {"C4", {"FIX", 1}},    {"C0", {"FLOAT", 1}},  {"F4", {"HIO", 1}},  {"3C", {"J", 3}},
        {"30", {"JEQ", 3}},  {"34", {"JGT", 3}},    {"38", {"JLT", 3}},    {"48", {"JSUB", 3}}, {"00", {"LDA", 3}},
        {"68", {"LDB", 3}},  {"50", {"LDCH", 3}},   {"70", {"LDF", 3}},    {"08", {"LDL", 3}},  {"6C", {"LDS", 3}},
        {"74", {"LDT", 3}},  {"04", {"LDX", 3}},    {"D0", {"LPS", 3}},    {"20", {"MUL", 3}},  {"60", {"MULF", 3}},
        {"98", {"MULR", 2}}, {"C8", {"NORM", 1}},   {"44", {"OR", 3}},     {"D8", {"RD", 3}},   {"AC", {"RMO", 2}},
        {"4C", {"RSUB", 3}}, {"A4", {"SHIFTL", 2}}, {"A8", {"SHIFTR", 2}}, {"F0", {"SIO", 1}},  {"EC", {"SSK", 3}},
        {"0C", {"STA", 3}},  {"78", {"STB", 3}},    {"54", {"STCH", 3}},   {"80", {"STF", 3}},  {"D4", {"STI", 3}},
        {"14", {"STL", 3}},  {"7C", {"STS", 3}},    {"E8", {"STSW", 3}},   {"84", {"SIT", 3}},  {"10", {"STX", 3}},
        {"1C", {"SUB", 3}},  {"5C", {"SUBF", 3}},   {"94", {"SUBR", 2}},   {"B0", {"SVC", 2}},  {"E0", {"TD", 3}},
        {"F8", {"TIO", 1}},  {"2C", {"TIX", 3}},    {"B8", {"TIXR", 2}},   {"DC", {"WD", 3}},
    };


    std::map <std::string, std::pair<std::string, std::string> > addressingModeTable = {
        {"110000", { "op c", "disp" }},
        {"110001", { "+op m", "addr" }},
        {"110010", { "op m", "(PC) + disp" }},
        {"110100", { "op m", "(B) + disp" }},
        {"111000", { "op c, X", "disp + (X)" }},
        {"111001", { "+op m, X", "addr + (X)" }},
        {"110010", { "op m, X", "(PC) + disp + (X)" }},
        {"100000", { "op @c", "disp" }},
        {"100001", { "+op @m", "addr" }},
        {"100010", { "op @m", "(PC) + disp" }},
        {"010000", { "op #c", "disp" }},
        {"010001", { "+op #m", "addr" }},
        {"010010", { "op #m", "(PC) + disp" }},
        {"010100", { "op #m", "(B) + disp" }},
    };


/**
    Translates a Header Record into corresponding an Assembly Code instruction & 
    stores result into output file
    @param outFile The output file containing the result Assembly Code after the translation
    @param line A string containing the Header Record 
    @return void
*/
    void HeaderParser ( std::ofstream &outFile, std::string line );


/**
    Translates a Text Record into corresponding an Assembly Code instruction &
    stores result into the output file
    @param outFile The output file containing the result Assembly Code after the translation 
    @param line A string containing the Text Record
    @return void
*/
    void TextParser ( std::ofstream &outFile, std::string line );


/**
    Translates an End Record into corresponding an Assembly Code instruction &
    stores result into the output file
    @param outFile The output file containing the result Assembly Code after the translation
    @param line A string containing the End Record 
    @return void
*/
    void EndParser ( std::ofstream &outFile, std::string line );


/**
    Generates Assembly Code instruction(s) if there is any needed memory assignment & 
    stores result(s) into the output file
    @param outFile The output file containing the result Assembly Code after the translation 
    @param lower An integer containing the lower bound of memory address
    @param upper An integer containing the upper bound of memory address
    @return void
*/
    void MemoryAssignment(std::ofstream &outFile, int rangeLower, int rangeUpper);


/**
    Updates the base and registers & stores result(s) into the output file
    @param outFile The output file containing the result Assembly Code after the translation 
    @param mnemonic A string containing the mnemonic name
    @param value An integer containing the upper bound of memory address
    @return void
*/
    void UpdateRegisters(std::ofstream &outFile, std::string mnemonic, unsigned int value);

/**
    Looks up mnemonic information using the opcode as the key
    @param binary A string containing the opcode in a binary number format. Its length should be 6
    @return A pair<string, int> containing the mnemonic name and the format type
*/
    std::pair<std::string, int> GetMnemonic(std::string binary);


/**
    Looks up the instruction format based the nixbpe flagBits
    @param flagBits A const integer containing nixbpe flagBits
    @param disOrAddr An unsigned integer containing the displacement or address depending on format
    @return a pair<string, unsigned int> containing the instruction format and the target address, respectively
*/
    std::pair<std::string, unsigned int> CalculateTargetAddress(const int flagBits, unsigned int dispOrAddr);


/**
    Converts a string representing a hexidecimal number (Base 16) to a decimal number (Base 10)
    @param hex A string representing a hexidecimal number (Base 16)
    @return A long integer in decimal number (Base 10)
*/
    long HexString2Decimal(std::string hex);


/**
    Converts a string representing a hexidecimal number (Base 16) to a string representing a binary number (Base 2)
    @param hex A string representing a hexidecimal number (Base 16)
    @return A string representing a binary number (Base 2)
*/
    std::string HexString2BinaryString(std::string hex);
    

/**
    Converts a string representing a binary number (Base 2) to a string representing a hexidecimal number (Base 16)
    @param binary A string representing a binary number (Base 2)
    @return A string representing a hexidecimal number (Base 16)
*/
    std::string BinaryToHex(std::string binary);
    

/**
    Produces Assembly Code instruction to the output file in an appropriate format
    @param outFile The output file containing the result Assembly Code after the translation
    @param address An integer representing a memory address 
    @param subroutineName A string containing a subroutine name
    @param mnemonic A string containing a mnemonic name
    @param forwardRef A string representing a forward reference 
    @param objectCode A string representing an object code
    @return void
*/
    void GenerateAssemblyInstruction(std::ofstream &outFile, int address, std::string subroutineName, std::string mnemonic, std::string forwardRef, std::string objectCode);


/**
    Produces Assembly Code instruction to the output file in an appropriate format
    @param outFile The output file containing the result Assembly Code after the translation
    @param mnemonic A string containing a mnemonic name
    @param forwardRef A string representing a forward reference 
    @return void
*/
    void GenerateAssemblyInstruction(std::ofstream &outFile, std::string mnemonic, std::string forwardRef);


};

