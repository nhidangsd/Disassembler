#include "disassembler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>


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
    Reads the input files into their proper data structures for later use
    @param  char ptr representing the commanline arguments
    @return 
*/
void DisAssembler::ReadFiles(char* objFile)
{
    fstream objStream;
    fstream symStream;
    string line;

    objStream.open(objFile);
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

    string symFile = objFile;
    symFile.erase(symFile.find_last_of('.'));
    symFile.append(".sym");
    symStream.open(symFile.c_str());
    if (!symStream.is_open()) {
        cout << "'.sym' file not found." << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        while (getline(symStream, line)) {
            symLines.push_back(line);
        }
    }
}


/**
    Loops over the object lines and uses helper functions to parse each line
    @param 
    @return 
*/
void DisAssembler::Parser()
{
    for (const string& line : objLines)
    {
        char firstChar = *line.c_str();
        switch (firstChar)
        {
            case 'H':
                HeaderParser(line.substr(1));
                break;
            case 'T':
                TextParser(line.substr(1));
                break;
            case 'E':
                EndParser(line.substr(1));
                break;
            default:
                break;
        }
    }
}


/**
    Parses a single header record line and writes it to output file
    @param  string representing the header record line
    @return
*/
void DisAssembler::HeaderParser(string line)
{
    // Find the index of the first number, that is where the header name ends
    int idx = std::distance(line.begin(), std::find_if(line.begin(), line.end(), [](const char c) { return std::isdigit(c); }));
    string headerName = line.substr(0, idx);
    int startingAddress = stoi(line.substr(9, 4), nullptr, 16);
    WriteToLst(startingAddress, headerName, "", "START  0", "");
}


/**
    Parses a single text record line. Initializes for loop to partwise parse the string and with helper functions
    extract the wanted information and store it in relevant variables. Finally write the result to output file
    @param  string representing the text record line
    @return
*/
void DisAssembler::TextParser(string line)
{
    unsigned int currentMemoryAddress = stoi(line.substr(2, 4), nullptr, 16);
    long recordLength = HexToDecimal(line.substr(6, 2));
    int format;
    
    for (int i = 8; i < recordLength * 2 + 8; )
    {
        if (currentMemoryAddress - mostRecentMemoryAddress > 4)
            MemoryAssignment(mostRecentMemoryAddress, currentMemoryAddress);
       
        string subroutineName = (symbolTable.find(currentMemoryAddress) != symbolTable.end()) ? symbolTable.at(currentMemoryAddress).first : "";
        string firstBits = HexToBinary(line.substr(i, 3)); 
        string mnemonic = GetMnemonic(firstBits.substr(0, 6)).first;

        // Check if format is 2, if it is grab the remaining two bits and write to file
        if (GetMnemonic(firstBits.substr(0, 6)).second == 2)
        {
            format = 2;
            string opCode = line.substr(i, format * 2);
            int regIdx = (int)opCode.at(2) - 48;
            WriteToLst(currentMemoryAddress, subroutineName, mnemonic, registerTable.substr(regIdx, 1), opCode);
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
                WriteToLst("LTORG", "");
                mnemonic = "*";
                WriteToLst(currentMemoryAddress, "", "*", subroutineName, opCode);
            } 
            else
            {
                string srcStatement = instructionFormat.replace(instructionFormat.find("op"), 2, mnemonic);
                if (instructionFormat.find('m') != string::npos)
                    srcStatement = instructionFormat.replace(instructionFormat.find("m"), 1, operand);
 
                else if (instructionFormat.find('c') != string::npos)
                    srcStatement = instructionFormat.replace(instructionFormat.find("c"), 1, opCode.substr(5));
 
                WriteToLst(currentMemoryAddress,
                    subroutineName, srcStatement.substr(0, srcStatement.find(" ")),
                    srcStatement.substr(srcStatement.find(" ")),
                    opCode);
            }
            UpdateRegisters(mnemonic, dispOrAddr);
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
void DisAssembler::EndParser(string line)
{
    int endingAddress = stoi(line, nullptr, 16);
    WriteToLst("END", symbolTable.at(endingAddress).first);
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
void DisAssembler::MemoryAssignment(int rangeLower, int rangeUpper)
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
        WriteToLst(currentMemoryAddress, subroutineName, "RESW", to_string(currentRange / 3), "");
    }
    int currentMemoryAddress = addressRanges[addressRanges.size() - 1];
    int lastRange = rangeUpper - addressRanges[addressRanges.size()-1];
    string subroutineName = symbolTable.at(addressRanges[addressRanges.size() - 1]).first;
    if(lastRange != 0)
        WriteToLst(currentMemoryAddress, subroutineName, "RESW", to_string(lastRange / 3), "");

}


void DisAssembler::UpdateRegisters(string mnemonic, unsigned int value)
{
    if (mnemonic == "LDB" && !baseRegisterActive)
    {
        baseRegisterActive = true;
        baseRegister = value;
        WriteToLst("BASE", symbolTable.at(baseRegister).first);
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
long DisAssembler::HexToDecimal(string hex)
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


void DisAssembler::WriteToLst(int address, string subroutineName, string mnemonic, string forwardRef, string opcode)
{
    cout << uppercase << hex << setfill('0') << setw(4) << address << setfill(' ') << setw(10) << subroutineName << setw(10) << mnemonic << setw(13) << forwardRef << setw(10) << opcode << endl;
}


void DisAssembler::WriteToLst(string mnemonic, string forwardRef)
{
    cout << setw(24) << mnemonic << setw(13) << forwardRef << endl;
}

