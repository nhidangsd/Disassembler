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

    - Program name <- the first 6 characters
    - Starting address of object program <- the next 6 characters
    - The length of object program <- the next 6 characters
    - Pass these variable into a function GenerateAssemblyInstruction() 
    to generate Assembly Code instruction


 * Algorithm for Text Parser: (Exclude format 1)

    - Declare a var named "format" to store the format type of the current machine instruction
    - currentMemoryAddress <- the first 6 characters
    - recordLength <- the next 2 characters
    - Loop through each character in the given string record, starting from index 8
    For i=8 to n = recordLength*2 + 8:

        + Check if there is a memory gap, if so:
            call MemoryAssignment() function
        + Extract the first 3 characters representing a hexidecimal number
            & convert into binary number form 
            & store it into a variable called "firstBits"
        + The first 6 digits of "firstBits" will be the opcode
        + mnemonic <- lookup mnemonic name using the opcode
        + subroutineName <- lookup subroutineName using the currentMemoryAddress

        + If the instruction is format 2:
            - format <- 2
            - Extract the objectCode base on the format type
            - Look up the register name
            - Pass these variable into a function GenerateAssemblyInstruction() 
            to generate Assembly Code instruction

        + Else:
            - Extract nixbpe bits <- the last half of "firstBits" (6 digits)
            - instructionFormat <- lookup instruction format by using CalculateTargetAddress with 
            nixbpe as the key
            - format <- 4 if instructionFormat containing '+', else 3
            - Assign correct data to operand, set base or (X) value as necessary depending on the format type

        + i += format * 2
        + currentMemoryAddress += format
        + mostRecentMemoryAddress <- currentMemoryAddress;
        + Check if there is memory gap at the end of the text record,
        if so: call MemoryAssignment() function



 * Algorithm for End Parser:

    - Set the address of first executable instruction of object file program <- the first 6 characters  
    - Pass the variable into a function GenerateAssemblyInstruction() 
    to generate Assembly Code instruction


