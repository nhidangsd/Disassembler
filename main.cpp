/**
    CS-530 Assignment 1: Disassembler
    @file checks.cpp
    @authors Luka Jozic & Nhi Dang
    @version 1.1 3/10/21
*/



#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>


#include "disassembler.h"


using namespace std;




int main(int argc, char** argv) 
{

    

    if(argc != 3)
    {
        cout << "Not enough args. Please provide the name of the 2 files.";
        return 1;
    }
  
   //0000000A6910083E174000024000
    

    DisAssembler* dasm = new DisAssembler;
    //dasm->CalculateTargetAddress(100001, 0);
    dasm->ReadFiles(*(argv + 1));
    dasm->Parser();
    
    //dasm->TextParser("0000000A6910083E174000024000");
    //dasm->TextParser("0008411BB400050000010005E32009332FFA53AFECDF200000000103101090");
    //dasm->TextParser("00109003000007");


    /*string nixbpe = "110100";
    int nex = stoi(nixbpe);
    cout << "NIXBPE: " << nex << endl;*/

    // bit shifting to double
    /*int a = 10;
    a = a << 1;
    cout << a << endl;*/

    /*const char* registerTable = "AXLBSTF";
    
    string opCode = "B450";
    int regIdx = (int)opCode.at(2) - 48;
    cout << registerTable[regIdx];*/
    

    /*string a = "00AA";
    string b = "0003";
    int c = std::stoi(a, nullptr, 16) + std::stoi(b, nullptr, 16);
    printf("%04X", c);*/

   

   /* string src = "=X'000001'";
    string opCode = "000001";
    if (src.find(opCode) != string::npos) {
        std::cout << "found!" << '\n';
    }*/


    //Padding
    //string symbolValue = string(6 - dispOrAddr.length(), '0') + dispOrAddr;

    
    

    return 0;
}