// Your First C++ Program
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

int main(int argc, char** argv) {

    // Check if we have enough args:
    // if (argv[1] == NULL)
	// {
	// 	cout << "There was either an empty file or nothing inputted\n";
	// 	return 0;
	// }
	// else
	// {
	// 	fstream file;
	// 	file.open(argv[1]);
	// 	cout << "File Sucessfully Read!\n";
	// }

    if(argc != 3){
        cout << "Not enough args. Please provide the name of the 2 files.";
        return 1;
    }

    // 
    for(int i = 1; i <= 2; i++){

        // Check if agrc[i] not null
        if(argv[i] == NULL ){
            cout << "name file" << argv[i] << "is NULL";
            return 1;
        }

        fstream file;
        file.open(argv[i]);
        string tp;
        while (getline(file, tp) ){
             cout << tp << "\n";
        }
	
    }

    




    std::cout << "Hello World!";
    return 0;
}