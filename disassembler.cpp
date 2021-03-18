#include "disassembler.h"
#include <iostream>
#include <fstream>
#include <sstream>

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

