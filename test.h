#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <fstream>
#include <iostream>


void print_symbolTab(std::map < unsigned int, std::pair<std::string, std::string> > symTab);
void print_objLines(std::vector<std::string> objLines);
void compare2Files(std::string fileA,std::string fileB);