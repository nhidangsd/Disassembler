#include <vector>
#include <string>

using namespace std;

class DisAssembler 
{
public:
    DisAssembler();
    ~DisAssembler();

    
    void ReadFiles(char* objFile);
    void PrintText();
    

private:
    vector<string> objLines;
    vector<string> symLines;

};


