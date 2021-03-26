/**
    CS-530 Assignment 1: Disassembler
    @file checks.cpp
    @authors Luka Jozic & Nhi Dang
    @version 1.1 3/10/21
*/

#include <iostream>
#include "disassembler.h"


using namespace std;

int main(int argc, char** argv) 
{
    if(argc != 3)
    {
        cout << "Not enough args. Please provide the name of the 2 files.";
        return 1;
    }

    DisAssembler* dasm = new DisAssembler;
    dasm->ReadFiles(*(argv + 1));
    dasm->Parser();
 
    return 0;
}