/**
    CS-530 Assignment 1: Disassembler
    @file main.cpp
    @authors Luka Jozic & Nhi Dang
    @version 2.1 3/28/21
*/

#include <iostream>
#include "disassembler.h"
using namespace std;

int main(int argc, char** argv) 
{

    // Check if there are enough arguments provided from terminal
    if(argc != 3)
    {
        cout << "Not enough args. Please provide the name of the 2 files." << endl;
        return 1;
    }
    
    // Declare an instance of DisAssembler
    DisAssembler* dasm = new DisAssembler;

    // Load object code file to DisAssembler for processing
    dasm->ReadinObjectCode(argv[1]);

    // Init value for DisAssembler's symbol table
    dasm->ReadinSymbolTable(argv[2]);

    // Translate Object Code program to Assembly Code program
    dasm->ObjectCode2AssemblyCode();

    return 0;
}