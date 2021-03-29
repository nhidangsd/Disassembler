#include <iostream>
#include "test.h"

using namespace std;


void print_symbolTab(map < unsigned int, pair<string, string> > symTab){

    cout << "PRINT SYMBOL TABLE MAP" << endl;
    cout << "----------------------------" << endl;
    // Create a map iterator and point to beginning of map
    map < unsigned int, pair<string, string> >::iterator it = symTab.begin();

    // Iterate over the map using Iterator till end.
    while (it != symTab.end())
    {
        // Accessing KEY from element pointed by it.
        unsigned int key = it->first;
        // Accessing VALUE from element pointed by it.
        pair<string, string> value = it->second;
        // Print out data
        cout << "{" << hex << uppercase << setfill('0') << setw(6)
            << key << " : { " 
            << '"' << value.first << '"' << ", " 
            << '"' << value.second << '"' << "}" << endl;
        // Increment the Iterator to point to next entry
        it++;
    }
    cout << "========================" << endl << endl;
}

void print_objLines(std::vector<std::string> objLines)
{
    cout << "PRINT OBJLINES VECTOR" << endl;
    cout << "----------------------------" << endl;
    for (auto& it : objLines) {
  
        // Print the values
        cout << it << endl;
    }
    cout << "========================" << endl << endl;
}

void compare2Files(string fileNameA, string fileNameB){
    ifstream fileA (fileNameA);
    ifstream fileB (fileNameB);
    string lineA, lineB;

    //Remember to check for failure (2 points)
    if (fileA.fail())
    {
        cout << "Couldn't open the file " << fileNameA <<endl;
        return ;
    }

    if (fileB.fail())
    {
        cout << "Couldn't open the file " << fileNameB << endl;
        return ;
    }

    if (fileNameA == fileNameB)
    {
        cout << "files are identical" << endl;
        return ;
    }


    while (getline(fileA, lineA) && getline(fileB, lineB))
    {
        const std::string WHITESPACE = " \n\r\t\f\v";
        size_t end = lineB.find_last_not_of(WHITESPACE);
        lineB = (end == std::string::npos) ? "" : lineB.substr(0, end + 1);

        if (lineA.compare(lineB) != 0 || ( lineA.length() != lineB.length() ))
        {
            cout << "TEST FAILED: compare2Files" << endl;
            cout << "line A length = " << lineA.length() <<  ": " << lineA << endl;
            cout << "line B length = " << lineB.length() <<  ": " << lineB << endl;
            return;
        }
        
    }

    cout << "TEST PASSED: compare2Files" << endl;
    
    // clear the state of File2 stream
    fileA.close();
    fileB.close();
}

    
