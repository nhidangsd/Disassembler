#include "disassembler.h"
#include <algorithm>

using namespace std;

DisAssembler::DisAssembler()
{
    baseRegister = 0x0;
    PCRegister = 0x0;
    XRegister = 0x0;
    ARegister = 0x0;
}

DisAssembler::~DisAssembler()
{
}


/**
    Reads the object code file into vector objLines for processing
    @param  char ptr representing the commanline arguments
    @return void
*/
void DisAssembler::ReadinObjectCode(char* fileName)
{
    fstream objStream;
    string line;

    objStream.open(fileName);
    if (!objStream.is_open()) {
        cout << "'.obj' file not found." << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        while (getline(objStream, line)) {
            objLines.push_back(line);
        }
    }

}


/**
    Reads the symbol table file into a map symbolTable for look up data
    @param  char ptr representing the commanline arguments
    @return void
*/
void DisAssembler::ReadinSymbolTable(char* fileName)
{
    string line;
    ifstream inFile (fileName); // Open the file for output operations (reading)
    
    if(inFile.is_open())
    {
        
        // Read the first table in Symbol Table
        // ignore first 2 lines
        for(int i = 0; i < 2; i++){
            getline(inFile, line);
        }
        while ( getline(inFile, line) && line.length() != 0)
        {
            // Split a line into words by space
            vector<string> tokens;
            istringstream iss(line);

            for(std::string s; iss >> s; ) {
                tokens.push_back(s);
            }
  
            pair<string, string> value (tokens.at(0), tokens.at(2)); 
            unsigned int key = stoi(tokens.at(1), nullptr, 16);
 
            symbolTable.insert( make_pair(key, value));
        }
       


        // Read the second table in Symbol Table

        // ignore first 2 lines
        for(int i = 0; i < 2; i++){
            getline(inFile, line);
        }
        while ( getline(inFile, line))
        {
            // Split a line into words by space
            vector<string> tokens;
            istringstream iss(line);

            for(std::string s; iss >> s; ) {
                tokens.push_back(s); 
            }

            pair<string, string> value (tokens.at(0), tokens.at(1)); 
            unsigned int key = stoi(tokens.at(2), nullptr, 16);
 
            symbolTable.insert( make_pair(key, value));
        }


        inFile.close();
    }
    else cout << "Unable to open file";

}


/**
    Loops over the object lines and uses helper functions to parse each line
    @param 
    @return 
*/
void DisAssembler::Parser()
{
    ofstream outFile ("out.lst");

    if(!outFile.is_open())
    {
        cout << "Unable to open file" << endl;
        return;
    }

    for (const string& line : objLines)
    {
        char firstChar = *line.c_str();
        switch (firstChar)
        {
            case 'H':
                HeaderParser(line.substr(1), outFile);
                break;
            case 'T':
                TextParser(line.substr(1), outFile);
                break;
            case 'E':
                EndParser(line.substr(1), outFile);
                break;
            default:
                break;
        }
    }

    outFile.close();
}


/**
    Parses a single header record line and writes it to output file
    @param  string representing the header record line
    @return
*/
void DisAssembler::HeaderParser(string line, ofstream &outFile)
{
    // Find the index of the first number, that is where the header name ends
    int idx = std::distance(line.begin(), std::find_if(line.begin(), line.end(), [](const char c) { return std::isdigit(c); }));
    string headerName = line.substr(0, idx);
    int startingAddress = stoi(line.substr(6, 6), nullptr, 16);
    WriteToLst(outFile, startingAddress, headerName, "", "START   0", "");
}


/**
    Parses a single text record line. Initializes for loop to partwise parse the string and with helper functions
    extract the wanted information and store it in relevant variables. Finally write the result to output file
    @param  string representing the text record line
    @return
*/
void DisAssembler::TextParser(string line, ofstream &outFile)
{

    // Col 0-6 is the STARTING ADDRESS for object code (hexidecimal)
    // we only need the last 4 digits in the 6digits starting addr
    unsigned int currentMemoryAddress = stoi(line.substr(2, 4), nullptr, 16);

    // Col 6-8 is the length of object code in bytes (hexidecimal)
    long recordLength = HexString2Decimal(line.substr(6, 2));
    int format;
    

    // Start processing the object code
    for (int i = 8; i < recordLength * 2 + 8; )
    {
        if (currentMemoryAddress - mostRecentMemoryAddress > 4)
            MemoryAssignment(outFile, mostRecentMemoryAddress, currentMemoryAddress);
       
        string subroutineName = (symbolTable.find(currentMemoryAddress) != symbolTable.end()) ? symbolTable.at(currentMemoryAddress).first : "";
        string firstBits = HexToBinary(line.substr(i, 3)); 
        string mnemonic = GetMnemonic(firstBits.substr(0, 6)).first;

        // Check if format is 2, if it is grab the remaining two bits and write to file
        if (GetMnemonic(firstBits.substr(0, 6)).second == 2)
        {
            format = 2;
            string opCode = line.substr(i, format * 2);
            int regIdx = (int)opCode.at(2) - 48;
            WriteToLst(outFile, currentMemoryAddress, subroutineName, mnemonic, registerTable.substr(regIdx, 1), opCode);
        }
        else // Its format 3 or 4, continue accordingly
        {
            const int nixbpe = stoi(firstBits.substr(6, 6));
            string instructionFormat = CalculateTargetAddress(nixbpe, 0).first;
            
            // If format contains a '+' sign, set format to 4. Otherwise grab from opCodeTable via function
            format = instructionFormat.find('+') != string::npos ? 4 : GetMnemonic(firstBits.substr(0, 6)).second;
            PCRegister = currentMemoryAddress + format;

            // If the format is 4, grab 5 characters for the address, otherwise grab 3 for displacement
            int dispOrAddr = format == 4 ? stoi(line.substr(i - 1 + format, format + 1), nullptr, 16)
                : stoi(line.substr(i + format, format), nullptr, 16);
  
            // If displacement is format 3 and in 2's complement, convert to its signed value (for TA calculation)
            dispOrAddr = ((format == 3 && 0x800 & dispOrAddr) ? (int)(0x7FF & dispOrAddr) - 0x800 : dispOrAddr);
            unsigned int targetAddress = CalculateTargetAddress(nixbpe, dispOrAddr).second;

            string operand = (symbolTable.find(targetAddress)) != symbolTable.end() ? symbolTable.at(targetAddress).first : "";
            string opCode = line.substr(i, format * 2);

            if (subroutineName.find(opCode) != string::npos)
            {
                WriteToLst(outFile, "LTORG", "");
                mnemonic = "*";
                WriteToLst(outFile, currentMemoryAddress, "", "*", subroutineName, opCode);
            } 
            else
            {
                string srcStatement = instructionFormat.replace(instructionFormat.find("op"), 2, mnemonic);
                if (instructionFormat.find('m') != string::npos)
                    srcStatement = instructionFormat.replace(instructionFormat.find("m"), 1, operand);
 
                else if (instructionFormat.find('c') != string::npos)
                    srcStatement = instructionFormat.replace(instructionFormat.find("c"), 1, opCode.substr(5));
 
                WriteToLst(outFile, currentMemoryAddress,
                    subroutineName, srcStatement.substr(0, srcStatement.find(" ")),
                    srcStatement.substr(srcStatement.find(" ")),
                    opCode);
            }
            UpdateRegisters(outFile, mnemonic, dispOrAddr);
        }
        currentMemoryAddress += format;
        mostRecentMemoryAddress = currentMemoryAddress;
        i += format * 2;

    }
}


/**
    Parses a single end record line and writes it to output file
    @param  string representing the end record line
    @return 
*/
void DisAssembler::EndParser(string line, ofstream &outFile)
{

    int endingAddress = stoi(line, nullptr, 16);
    WriteToLst(outFile, "END", symbolTable.at(endingAddress).first);
}


/**
    Appends two 0's to the binary opcode and returns the mnemonic and its corresponding format
    @param  string representing the first 6 bits of the opcode
    @return a pair, the menmonic as a string and the format as int, respectively
*/
pair<string, int> DisAssembler::GetMnemonic(string binary)
{
    binary += "00";
    string opCode = BinaryToHex(binary);
    return { opCodeTable.at(opCode).first, opCodeTable.at(opCode).second };
}


/**
    Calculates the current target address
    @param takes in the nixbpe flagBits and a unsigned int representing the displacement or address depending on format
    @return a pair, assembly language notation as string and TA as unsigned int, respectively
*/
pair<string, unsigned int> DisAssembler::CalculateTargetAddress(const int flagBits, unsigned int dispOrAddr)
{
    switch (flagBits)
    {
        case 110000:
            return { "op c", dispOrAddr };
        case 110001:
            return { "+op m", dispOrAddr };
        case 110010:
            return { "op m", PCRegister + dispOrAddr };
        case 110100:
            return { "op m", baseRegister + dispOrAddr };
        case 111000:
            return { "op c, X", dispOrAddr + XRegister };
        case 111001:
            return { "+op m, X", dispOrAddr + XRegister };
        case 111010:
            return { "op m, X", PCRegister + dispOrAddr + XRegister };
        case 100000:
            return { "op @c", dispOrAddr };
        case 100001:
            return { "+op @m", dispOrAddr };
        case 100010:
            return { "op @m", PCRegister + dispOrAddr };
        case 100100:
            return { "op @m", baseRegister + dispOrAddr };
        case 10000:
            return { "op #c", dispOrAddr };
        case 10001:
            return { "+op #m", dispOrAddr };
        case 10010:
            return { "op #m", PCRegister + dispOrAddr };
        case 10100:
            return { "op #m", baseRegister + dispOrAddr };
        default:
            return { string(), 0 };
    }
}


/**
    Assigns memory within the gap range by iterating over the symbolTable and seeing which values are in the given range
    then calculates the proper memory assignment and the corresponding subroutine Name
    @param two ints representing the lower and upper memory range respectively
    @return 
*/
void DisAssembler::MemoryAssignment(ofstream &outFile, int rangeLower, int rangeUpper)
{
    vector<unsigned int> addressRanges;
    for (const auto& key : symbolTable)
    {
        if (key.first >= rangeLower && key.first <= rangeUpper)
            addressRanges.push_back(key.first);     
    }

    for (int i = 0; i < addressRanges.size() - 1; i++)
    {
        int currentMemoryAddress = addressRanges[i];
        int currentRange = addressRanges[i + 1] - addressRanges[i];        
        string subroutineName = symbolTable.at(addressRanges[i]).first;
        WriteToLst(outFile, currentMemoryAddress, subroutineName, "RESW", to_string(currentRange / 3), "");
    }
    int currentMemoryAddress = addressRanges[addressRanges.size() - 1];
    int lastRange = rangeUpper - addressRanges[addressRanges.size()-1];
    string subroutineName = symbolTable.at(addressRanges[addressRanges.size() - 1]).first;
    if(lastRange != 0)
        WriteToLst(outFile, currentMemoryAddress, subroutineName, "RESW", to_string(lastRange / 3), "");

}


void DisAssembler::UpdateRegisters(ofstream &outFile, string mnemonic, unsigned int value)
{
    if (mnemonic == "LDB" && !baseRegisterActive)
    {
        baseRegisterActive = true;
        baseRegister = value;
        WriteToLst(outFile, "BASE", symbolTable.at(baseRegister).first);
    }
    else if (mnemonic == "LDX")
    {
        XRegister = value;
    }
}


/**
    Converts a hexadecimal number to decimal number.
    @param hexadecimal number to convert.
    @return the hexadecimal number converted to decimal
*/
long DisAssembler::HexString2Decimal(string hex)
{
    return stol(hex, nullptr, 16);
}


/**
    Converts a hexadecimal number to binary number.
    @param hexadecimal number to convert.
    @return the hexadecimal number converted to binary
*/
string DisAssembler::HexToBinary(string hex)
{
    string binary = "";
    for (const auto& character : hex)
    {
        int index = hexDigits.find(character);
        binary += binaryNums[index];
    }
    return binary;
}


/**
    Converts a binary number to hexadecimal number.
    @param binary number to convert.
    @return the binary number converted to hexadecimal
*/
string DisAssembler::BinaryToHex(string binary)
{
    string hex = "";
    for (int i = 0; i < binary.size(); i +=4)
    {
        int index = find(binaryNums, binaryNums + BIN_LIST_SIZE, binary.substr(i, 4)) - binaryNums;
        hex += hexDigits[index];  
    }
    return hex;
}


void DisAssembler::WriteToLst(ofstream &outFile, int address, string subroutineName, string mnemonic, string forwardRef, string opcode)
{
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = forwardRef.find_first_not_of(WHITESPACE);
    forwardRef =  (start == std::string::npos) ? "" : forwardRef.substr(start);
    // regex r("^\\s+");
    // forwardRef = regex_replace(forwardRef, r, "");

    int subroutineNameLen = 10;
    int mnemonicLen = 8;
    int forwardRefLen = 16;
    
    if(subroutineName.length() == 0){
        subroutineName = " ";
    }
    if(mnemonic.length() == 0){
        mnemonic = " ";
    }

    if(mnemonic.at(0) == '+'){
        subroutineNameLen --;
        mnemonicLen++;
    }
   
    if (forwardRef.at(0) == '#' || forwardRef.at(0) == '@'){
        forwardRefLen++;
        mnemonicLen--;
    }
    // cout << "subroutineNameLen: " << subroutineNameLen << endl;
    // cout << "mnemonicLen: " << mnemonicLen << endl;
    outFile << uppercase << hex << right << setfill('0') << setw(4) << address << left
         << setfill(' ') << setw(4) << " " 
         << setfill(' ') << setw(subroutineNameLen) << left << subroutineName
         << setfill(' ') << setw(mnemonicLen) << mnemonic
         << setfill(' ') << setw(forwardRefLen) << forwardRef
         << opcode
         << endl;
}


void DisAssembler::WriteToLst(ofstream &outFile, string mnemonic, string forwardRef)
{
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = forwardRef.find_first_not_of(WHITESPACE);
    forwardRef =  (start == std::string::npos) ? "" : forwardRef.substr(start);
    // regex r("^\\s+");
    // forwardRef = regex_replace(forwardRef, r, "");
    outFile << setfill(' ') << setw(18) << " "  
            << left << setw(8) << mnemonic 
            << setw(16) << forwardRef << endl;
}

