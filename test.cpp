#include <iostream>
#include "test.h"

using namespace std;


void print_symbolTab(map < unsigned int, pair<string, string> > symTab){
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
}

    
