CC=g++

CFLAGS= -std=c++11

all: dissem

dissem: main.cpp disassembler.cpp test.cpp
	$(CC) $(CFLAGS) main.cpp disassembler.cpp test.cpp -o dissem

clean:
	$(RM) *.o *.lst dissem