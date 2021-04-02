CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Requirements
 * Compilation
 * Algorithm Explaination


INTRODUCTION
------------

The Disassembler module translates the entire Machine Language (Object Code) program file
into an Assembly Language program file.



REQUIREMENTS
------------

This module requires the following:

 * Compiler: g++
 * C++ version: C++11 and above
 * Operating System: Linux/Unix



COMPILATION
-----------

    In your terminal, navigate into the folder, and: 

 * Step 1:  Compile all the source code files by typing:
    $   make
 * Step 2:  Run the executable file
    $   ./dissem <objectCodeFileName> <symbolTableFileName>
    


ALGORITHM EXPLAINATION
----------------------

 * The main Algorithm:

    - Read in the Object Code program file and load into a vector called "objLines" to process line by line
    - Read in the Symnol Table file and store it in a map for lookup purpose
    - Create an output file to store the end result of Assembly Code after the translation
    - Loop through the "objLines" and process each item at a time:

        + Extracted the first character on that line and check if it is equal to:
            - 'H':  call the HeaderParser() function &
                + pass the rest of the string as 1 of the argument (EXCLUDING the 1 character)
                    Exp: "HAssign000000001093"
                        We only pass "Assign000000001093" into the HeaderParser()
                + pass the output file reference to append the result into that file

            - 'T':  call the TextParser() function &
                + do similar to the 'H' case above.

            - 'E':  call the EndParser() function &
                + do similar to the 'H' case above.
        
        + End the loop

    - Close the output file


 * Algorithm for Header Parser:

    - Set program name <- the first 6 characters
    - Set starting address of object program <- the next 6 characters
    - Set the length of object program <- the next 6 characters
    - Pass these variable into a function GenerateAssemblyInstruction() 
    to generate Assembly Code instruction


 * Algorithm for Text Parser:


 * Algorithm for End Parser:

    - Set the address of first executable instruction of object file program <- the first 6 characters  
    - Pass the variable into a function GenerateAssemblyInstruction() 
    to generate Assembly Code instruction


        


 