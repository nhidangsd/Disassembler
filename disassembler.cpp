#include "disassembler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <typeinfo>
#include <iomanip>


using namespace std;

DisAssembler::DisAssembler()
{
}

DisAssembler::~DisAssembler()
{
    
}

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


void DisAssembler::PrintText()
{
    for (auto& line : objLines)
    {
        cout << line << endl;
    }
    cout << "\n";
    for (auto& line : symLines)
    {
        cout << line << endl;
    }
    
    
}

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

void DisAssembler::HeaderParser(string line)
{
    string headerName = line.substr(0, 6);
    int startingAddress = stoi(line.substr(9, 4), nullptr, 16);
    WriteToLst(startingAddress, headerName, "", "START  0", "");
}

void DisAssembler::TextParser(string line)
{
    //cout << line << endl;
    // 6910083E174000024000

    int currentMemoryAddress = stoi(line.substr(2, 4), nullptr, 16);
    long recordLength = HexToDecimal(line.substr(6, 2));
    int format;
    
    //cout << "OPCODES: " << line.substr(8) << endl;

    
    for (int i = 8; i < recordLength * 2 + 8; )
    {
        //cout << "\nSTARTING ADDR: " << currentMemoryAddress << endl;
        //cout << "MOST RECENT ADDR: " << mostRecentMemoryAddress << endl;

        if (currentMemoryAddress - mostRecentMemoryAddress > 4)
        {
            //cout << "RESW IS NEEDED HERE =====================" << endl;
            CheckMemoryGap(mostRecentMemoryAddress, currentMemoryAddress);
        }

        

        string targetName = (symbolTable.find(currentMemoryAddress) != symbolTable.end()) ? symbolTable.at(currentMemoryAddress).first : "";
        //cout << "TARGET NAME " << targetName << endl;

        string firstBits = HexToBinary(line.substr(i, 3)); 
        //cout << "FIRST BITS: " << firstBits << endl;


        string mnemonic = GetMnemonic(firstBits.substr(0, 6)).first;
        //cout << "MNEMONIC: " << mnemonic << endl;

        if (GetMnemonic(firstBits.substr(0, 6)).second == 2)
        {
            //cout << "CUT HERE ==========================================" << endl;
            format = 2;
            string opCode = line.substr(i, format * 2);
            int regIdx = (int)opCode.at(2) - 48;
            //cout << "OPCODE: " << opCode << endl;
            WriteToLst(currentMemoryAddress, targetName, mnemonic, registerTable.substr(regIdx, 1), opCode);
        }
        else
        {
            int nixbpe = stoi(firstBits.substr(6, 6));
            //cout << "NIXBPE: " << std::dec << nixbpe << endl;


            string asl = CalculateTargetAddress(nixbpe, 0).first;
            //cout << "ASL: " << asl << endl;


            format = asl.find('+') != string::npos ? 4 : GetMnemonic(firstBits.substr(0, 6)).second;
            //cout << "FORMAT: " << format << endl;

            PCRegister = currentMemoryAddress + format;
            //cout << "PCRegister: " << PCRegister << endl;


            unsigned int dispOrAddr = format == 4 ? stoi(line.substr(i - 1 + format, format + 1), nullptr, 16)
                : stoi(line.substr(i + format, format), nullptr, 16);
            //cout << "DISP OR ADDR: " << std::hex << dispOrAddr << endl;

            unsigned int targetAddress = CalculateTargetAddress(nixbpe, dispOrAddr).second;
            if (format == 3 && targetAddress > 0x1000)
                targetAddress -= 0x1000;
            //cout << "TARGET ADDRESS: " << std::hex << targetAddress << endl;

            

            string srcVarName = (symbolTable.find(targetAddress)) != symbolTable.end() ? symbolTable.at(targetAddress).first : "";
            //cout << "SRC SYMBOL NAME: " << srcVarName << endl;
            
            string opCode = line.substr(i, format * 2);
            //cout << "OPCODE: " << opCode << endl;

            if (targetName.find(opCode) != string::npos)
            {
                WriteToLst("", "LTORG");
                mnemonic = "*";
                WriteToLst(currentMemoryAddress, "", "*", targetName, opCode);
            } 
            else
            {
                string srcStatement = asl.replace(asl.find("op"), 2, mnemonic);
                if (asl.find('m') != string::npos)
                {
                    srcStatement = asl.replace(asl.find("m"), 1, srcVarName);
                    //cout << "SRC STATEMENT: " << srcStatement << endl;
                }
                else if (asl.find('c') != string::npos)
                {
                    srcStatement = asl.replace(asl.find("c"), 1, opCode.substr(5));
                    //cout << "SRC STATEMENT: " << srcStatement << endl;
                }

                WriteToLst(currentMemoryAddress,
                    targetName, srcStatement.substr(0, srcStatement.find(" ")),
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


void DisAssembler::EndParser(string line)
{
    int endingAddress = stoi(line, nullptr, 16);
    WriteToLst("END", symbolTable.at(endingAddress).first);
}



pair<string, unsigned int> DisAssembler::CalculateTargetAddress(int flagBits, unsigned int dispOrAddr)
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






pair<string, int> DisAssembler::GetMnemonic(string binary)
{
    binary += "00";
    string opCode = BinaryToHex(binary);
    return { opCodeTable.at(opCode).first, opCodeTable.at(opCode).second };
}


void DisAssembler::CheckMemoryGap(int rangeLower, int rangeUpper)
{
    string mnemonic = "RESW";
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
        string targetName = symbolTable.at(addressRanges[i]).first;
        WriteToLst(currentMemoryAddress,targetName, mnemonic, to_string(currentRange / 3), "");
    }
    int currentMemoryAddress = addressRanges[addressRanges.size() - 1];
    int lastRange = rangeUpper - addressRanges[addressRanges.size()-1];
    string targetName = symbolTable.at(addressRanges[addressRanges.size() - 1]).first;
    if(lastRange != 0)
        WriteToLst(currentMemoryAddress, targetName, mnemonic, to_string(lastRange / 3), "");

}


void DisAssembler::UpdateRegisters(string mnemonic, unsigned int value)
{
    if (mnemonic == "LDB" && !baseRegisterActive)
    {
        baseRegisterActive = true;
        baseRegister = value;
        WriteToLst("BASE", symbolTable.at(baseRegister).first);
    }
}

pair<string, string> DisAssembler::GetAddressingMode(string flagBits)
{
    return { addressingModeTable.at(flagBits).first, addressingModeTable.at(flagBits).second };
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

void DisAssembler::WriteToLst(int address, string name, string mnemonic, string srcName, string opcode)
{
    printf("%04X", address);
    cout << setw(10) << name << setw(10) << mnemonic << setw(10) << srcName << setw(10) << opcode << endl;
}

void DisAssembler::WriteToLst(string mnemonic, string srcName)
{
    cout << setw(20) << mnemonic << setw(10) << srcName << endl;
}




