/**
    CS-530 Assignment 1: Disassembler
    @file disassembler.cpp
    @authors Luka Jozic & Nhi Dang
    @version 2.1 3/28/21
*/

#include "disassembler.h"
using namespace std;


// Constructor
DisAssembler::DisAssembler()
{
    baseRegister = 0x0;
    PCRegister = 0x0;
    XRegister = 0x0;
    ARegister = 0x0;
}

// Destructor
DisAssembler::~DisAssembler()
{
}


/**
    Reads the object code file and stores it in a vector called "objLines" for processing
    @param fileName A character ptr containing the name of the file to open
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
    Reads the symbol table file iand stores it in a map called "symbolTable" for lookup purpose
    @param filName A character ptr containing the name of the file to open
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
    Perform the translation from Object Code program into Assembly Launguage program
    @return void
*/
void DisAssembler::ObjectCode2AssemblyCode()
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
                HeaderParser (outFile, line.substr(1));
                break;
            case 'T':
                TextParser (outFile, line.substr(1));
                break;
            case 'E':
                EndParser (outFile, line.substr(1));
                break;
            default:
                break;
        }
        counter++;
    }

    outFile.close();
}


/**
    Translates a Header Record into corresponding an Assembly Code instruction
    @param outFile The output file containing the result Assembly Code after the translation
    @param line A string containing the Header Record 
    @return void
*/
void DisAssembler::HeaderParser  ( ofstream &outFile, string line )
{
    // Find the index of the first number, that is where the header name ends
    int idx = std::distance(line.begin(), std::find_if(line.begin(), line.end(), [](const char c) { return std::isdigit(c); }));
    string headerName = line.substr(0, idx);
    int startingAddress = stoi(line.substr(6, 6), nullptr, 16);
    fullRecordLength = stoi(line.substr(12, 6), nullptr, 16);
    GenerateAssemblyInstruction(outFile, startingAddress, headerName, "", "START   0", "");
}


/**
    Translates a Text Record into corresponding an Assembly Code instruction
    @param outFile The output file containing the result Assembly Code after the translation 
    @param line A string containing the Text Record
    @return void
*/
void DisAssembler::TextParser  ( ofstream &outFile, string line )
{

    int format; // store the format type of the current machine instruction

    // Col 0-6 is the STARTING ADDRESS for object code (hexidecimal)
    // we only need the last 4 digits in the 6digits starting addr
    unsigned int currentMemoryAddress = stoi(line.substr(2, 4), nullptr, 16);

    // Col 6-8 is the length of object code in bytes (hexidecimal)
    long recordLength = HexString2Decimal(line.substr(6, 2));

    // Start processing the object code
    for (int i = 8; i < recordLength * 2 + 8; )
    {
        if (currentMemoryAddress - mostRecentMemoryAddress > 4)
        {
            MemoryAssignment(outFile, mostRecentMemoryAddress, currentMemoryAddress);
        }

        string firstBits = HexString2BinaryString(line.substr(i, 3)); 
        string opcode = firstBits.substr(0, 6);
        string mnemonic = GetMnemonic(opcode).first;
        string subroutineName = (symbolTable.find(currentMemoryAddress) != symbolTable.end()) ? symbolTable.at(currentMemoryAddress).first : "";

        // Check if format is 2, if it is grab the remaining two bits and write to file
        if (GetMnemonic(opcode).second == 2)
        {
            format = 2;
            string objectCode = line.substr(i, format * 2);
            int regIdx = (int)objectCode.at(2) - 48;
            GenerateAssemblyInstruction(outFile, currentMemoryAddress, subroutineName, mnemonic, registerTable.substr(regIdx, 1), objectCode);
        }
        else // Its format 3 or 4, continue accordingly
        {
            const int nixbpe = stoi(firstBits.substr(6, 6));
        
            string instructionFormat = CalculateTargetAddress(nixbpe, 0).first;

            // If format contains a '+' sign, set format to 4. Otherwise grab from opcodeTable via function
            format = instructionFormat.find('+') != string::npos ? 4 : GetMnemonic(firstBits.substr(0, 6)).second;
            PCRegister = currentMemoryAddress + format;

            // If the format is 4, grab 5 characters for the address, otherwise grab 3 for displacement
            int dispOrAddr = format == 4 ? stoi(line.substr(i - 1 + format, format + 1), nullptr, 16)
                : stoi(line.substr(i + format, format), nullptr, 16);
  
            // If displacement is format 3 and in 2's complement, convert to its signed value (for TA calculation)
            dispOrAddr = ((format == 3 && 0x800 & dispOrAddr) ? (int)(0x7FF & dispOrAddr) - 0x800 : dispOrAddr);

            unsigned int targetAddress = CalculateTargetAddress(nixbpe, dispOrAddr).second;
            string operand = (symbolTable.find(targetAddress)) != symbolTable.end() ? symbolTable.at(targetAddress).first : "";
            string objectCode = line.substr(i, format * 2);

            if (subroutineName.find(objectCode) != string::npos)
            {
                GenerateAssemblyInstruction(outFile, "LTORG", "");
                mnemonic = "*";
                GenerateAssemblyInstruction(outFile, currentMemoryAddress, "", "*", subroutineName, objectCode);
            } else if (instructionFormat.length() == 0) {
                GenerateAssemblyInstruction(outFile, "LTORG", "");
                GenerateAssemblyInstruction(outFile, currentMemoryAddress, "", "*", subroutineName, objectCode.substr(0,2));
                format = 1;
            }

            else
            {
                string srcStatement = instructionFormat.replace(instructionFormat.find("op"), 2, mnemonic);
                if (instructionFormat.find('m') != string::npos)
                    srcStatement = instructionFormat.replace(instructionFormat.find("m"), 1, operand);
 
                else if ( (instructionFormat.find('c') != string::npos) ){
                    srcStatement = instructionFormat.replace(instructionFormat.find("c"), 1, objectCode.substr(5));
                }
                string forwardRef = srcStatement.substr(srcStatement.find(" "));
                if ( !mnemonic.compare("RSUB")){
                    forwardRef = "";
                }
                GenerateAssemblyInstruction(outFile, currentMemoryAddress,
                    subroutineName, srcStatement.substr(0, srcStatement.find(" ")), forwardRef, objectCode);
            }
            
            UpdateRegisters(outFile, mnemonic, dispOrAddr);
        }
        currentMemoryAddress += format;
        mostRecentMemoryAddress = currentMemoryAddress;
        i += format * 2;


        if (i >= recordLength * 2 + 8 && objLines.at(counter + 1).at(0) != 'T')
        {
            if (fullRecordLength - currentMemoryAddress > format)
            {
                MemoryAssignment(outFile, mostRecentMemoryAddress - format, fullRecordLength);
            }

        }
    }
}


/**
    Translates an End Record into corresponding an Assembly Code instruction
    @param outFile The output file containing the result Assembly Code after the translation
    @param line A string containing the End Record 
    @return void
*/
void DisAssembler::EndParser ( ofstream &outFile, string line )
{
    int endingAddress = stoi(line, nullptr, 6);
    GenerateAssemblyInstruction(outFile, "END", symbolTable.at(endingAddress).first);
}


/**
    Generates Assembly Code instruction(s) if there is any needed memory assignment & 
    stores result(s) into the output file
    @param outFile The output file containing the result Assembly Code after the translation 
    @param lower An integer containing the lower bound of memory address
    @param upper An integer containing the upper bound of memory address
    @return void
*/
void DisAssembler::MemoryAssignment(ofstream &outFile, int rangeLower, int rangeUpper)
{
    vector<unsigned int> addressRanges;
    for (const auto& key : symbolTable)
    {
        if (key.first >= rangeLower && key.first <= rangeUpper)
        {
            addressRanges.push_back(key.first);     
        }
    }

    for (int i = 0; i < addressRanges.size() - 1; i++)
    {
        int currentMemoryAddress = addressRanges[i];
        int currentRange = addressRanges[i + 1] - addressRanges[i];        
        string subroutineName = symbolTable.at(addressRanges[i]).first;
        GenerateAssemblyInstruction(outFile, currentMemoryAddress, subroutineName, "RESW", to_string(currentRange / 3), "");
    }
    int currentMemoryAddress = addressRanges[addressRanges.size() - 1];
    int lastRange = rangeUpper - addressRanges[addressRanges.size()-1];
    string subroutineName = symbolTable.at(addressRanges[addressRanges.size() - 1]).first;
    if(lastRange != 0)
        GenerateAssemblyInstruction(outFile, currentMemoryAddress, subroutineName, "RESW", to_string(lastRange / 3), "");

}


/**
    Updates the base and registers & stores result(s) into the output file
    @param outFile The output file containing the result Assembly Code after the translation 
    @param mnemonic A string containing the mnemonic name
    @param value An integer containing the upper bound of memory address
    @return void
*/
void DisAssembler::UpdateRegisters(ofstream &outFile, string mnemonic, unsigned int value)
{
    if (mnemonic == "LDB" && !baseRegisterActive)
    {
        baseRegisterActive = true;
        baseRegister = value;
        GenerateAssemblyInstruction(outFile, "BASE", symbolTable.at(baseRegister).first);
    }
    else if (mnemonic == "LDX")
    {
        XRegister = value;
    }
}


/**
    Looks up mnemonic information using the opcode as the key
    @param binary A string containing the opcode in a binary number format. Its length should be 6
    @return A pair<string, int> containing the mnemonic name and the format type
*/
pair<string, int> DisAssembler::GetMnemonic(string binary)
{
    binary.append("00"); // Appends two 0's to the binary opcode string
    string opCode = BinaryToHex(binary);
    return { opcodeTable.at(opCode).first, opcodeTable.at(opCode).second };
}


/**
    Looks up the instruction format based the nixbpe flagBits
    @param flagBits A const integer containing nixbpe flagBits
    @param disOrAddr An unsigned integer containing the displacement or address depending on format
    @return a pair<string, unsigned int> containing the instruction format and the target address, respectively
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
            return { "op c,X", dispOrAddr + XRegister };
        case 111001:
            return { "+op m,X", dispOrAddr + XRegister };
        case 111010:
            return { "op m,X", PCRegister + dispOrAddr + XRegister };
        case 111100:
            return { "op m,X", baseRegister + dispOrAddr + XRegister };
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
    Converts a string representing a hexidecimal number (Base 16) to a decimal number (Base 10)
    @param hex A string representing a hexidecimal number (Base 16)
    @return A long integer in decimal number (Base 10)
*/
long DisAssembler::HexString2Decimal(string hex)
{
    return stol(hex, nullptr, 16);
}


/**
    Converts a string representing a hexidecimal number (Base 16) to a string representing a binary number (Base 2)
    @param hex A string representing a hexidecimal number (Base 16)
    @return A string representing a binary number (Base 2)
*/
string DisAssembler::HexString2BinaryString(string hex)
{
    string binary = "";
    for (const auto& character : hex)
    {
        int index = hexDigits.find(character);
        binary.append(binaryNums[index]);
    }
    return binary;
}


/**
    Converts a string representing a binary number (Base 2) to a string representing a hexidecimal number (Base 16)
    @param binary A string representing a binary number (Base 2)
    @return A string representing a hexidecimal number (Base 16)
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
void DisAssembler::GenerateAssemblyInstruction(ofstream &outFile, int address, string subroutineName, string mnemonic, string forwardRef, string objectCode)
{
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = forwardRef.find_first_not_of(WHITESPACE);
    forwardRef =  (start == std::string::npos) ? "" : forwardRef.substr(start);

    int subroutineNameLen = 10;
    int mnemonicLen = 8;
    int forwardRefLen = 16;
    
    if(subroutineName.length() == 0){
        subroutineName = " ";
    }
    if(mnemonic.length() == 0){
        mnemonic = " ";
    }
    if(forwardRef.length() == 0){
        forwardRef = " ";
    }

    if(mnemonic.at(0) == '+'){
        subroutineNameLen --;
        mnemonicLen++;
    }
   
    if (forwardRef.at(0) == '#' || forwardRef.at(0) == '@'){
        forwardRefLen++;
        mnemonicLen--;
    }

    outFile << uppercase << hex << right << setfill('0') << setw(4) << address << left
         << setfill(' ') << setw(4) << " " 
         << setfill(' ') << setw(subroutineNameLen) << left << subroutineName
         << setfill(' ') << setw(mnemonicLen) << mnemonic
         << setfill(' ') << setw(forwardRefLen) << forwardRef
         << objectCode
         << endl;
}


/**
    Produces Assembly Code instruction to the output file in an appropriate format
    @param outFile The output file containing the result Assembly Code after the translation
    @param mnemonic A string containing a mnemonic name
    @param forwardRef A string representing a forward reference 
    @return void
*/
void DisAssembler::GenerateAssemblyInstruction(ofstream &outFile, string mnemonic, string forwardRef)
{
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = forwardRef.find_first_not_of(WHITESPACE);
    forwardRef =  (start == std::string::npos) ? "" : forwardRef.substr(start);

    outFile << setfill(' ') << setw(18) << " "  
            << left << setw(8) << mnemonic 
            << setw(16) << forwardRef << endl;
}

