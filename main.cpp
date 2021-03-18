// Your First C++ Program
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "disassembler.h"


using namespace std;

int main(int argc, char** argv) {

    

    if(argc != 3){
        cout << "Not enough args. Please provide the name of the 2 files.";
        return 1;
    }


    DisAssembler* dasm = new DisAssembler;
    dasm->ReadFiles(*(argv + 1));
    dasm->PrintText();
    

    return 0;
}