/**
    CS-530 Assignment 1: Disassembler
    @file checks.cpp
    @authors Luka Jozic & Nhi Dang
    @version 1.2 3/28/21
*/

#include <iostream>
#include <iomanip>
#include "disassembler.h"

// #include "test.h"
using namespace std;

int main(int argc, char** argv) 
{
    if(argc != 3)
    {
        cout << "Not enough args. Please provide the name of the 2 files." << endl;
        return 1;
    }
    // Declare an instance of DisAssembler
    DisAssembler* dasm = new DisAssembler;

    // Load object code to DisAssembler for processing
    dasm->ReadinObjectCode(argv[1]);

    // Init value for DisAssembler 's symbol table
    dasm->ReadinSymbolTable(argv[2]);

    // print_symbolTab(dasm->symbolTable);

    dasm->Parser();

    return 0;
}