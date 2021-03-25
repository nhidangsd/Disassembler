#include <vector>
#include <string>
#include <map>
#include <algorithm>

using namespace std;

#define BIN_LIST_SIZE 16

class DisAssembler 
{
public:
    DisAssembler();
    ~DisAssembler();

    
    void ReadFiles(char* objFile);
    void PrintText();
    void Parser();
    void TextParser(string line);
    
    pair<string, string> GetAddressingMode(string flagBits);
    pair<string, unsigned int> CalculateTargetAddress(int flagBits, unsigned int dispOrAddr);

private:
    void HeaderParser(string line);
    void EndParser(string line);
    pair<string, int> GetMnemonic(string binary);

    void CheckMemoryGap(int rangeLower, int rangeUpper);

    void UpdateRegisters(string mnemonic, unsigned int value);
    long HexToDecimal(string hex);
    string HexToBinary(string hex);
    string BinaryToHex(string binary);
    void WriteToLst(int address, string name, string mnemonic, string srcName, string opcode);
    void WriteToLst(string mnemonic, string srcName);

    vector<string> objLines;
    vector<string> symLines;


    bool baseRegisterActive = false;
    bool XRegisterFlag = false;
    unsigned int baseRegister = 0;
    unsigned int PCRegister = 0;
    unsigned int XRegister = 0;
    unsigned int ARegister = 0;

    int mostRecentMemoryAddress;
    
    const string registerTable = "AXLBSTF";

    map<string, pair<string, int>> opCodeTable = {
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


    map<string, pair<string, string>> addressingModeTable = {
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
        {"100100", { "op @m", "(B) + disp" }},
        {"010000", { "op #c", "disp" }},
        {"010001", { "+op #m", "addr" }},
        {"010010", { "op #m", "(PC) + disp" }},
        {"010100", { "op #m", "(B) + disp" }}
    };
    

    map<unsigned int, pair<string, string>> symbolTable = {
        {0x0, { "FIRST", "R" }},
        {0xA, { "BADR", "R" }},
        {0x83E, { "RETADR", "R" }},
        {0x849, { "WLOOP", "R" }},
        {0x85C, { "EADR", "R" }},

        {0x855, { "=X'000001'", "6" }},
        {0x1090, { "=X'000007'", "6" }}
    };



    const string binaryNums[BIN_LIST_SIZE] = { "0000", "0001", "0010", "0011", "0100", "0101","0110", "0111", "1000", "1001", "1010",
                               "1011", "1100", "1101", "1110","1111" };
    const string hexDigits = "0123456789ABCDEF";


};



