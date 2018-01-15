CXX = g++

default: runfile

runfile.o: mips_assembler.cpp
	g++ -g -c mips_assembler.cpp -o runfile.o

runfile: runfile.o
	$(CXX) runfile.o -o runfile

clean:
	rm -f *.o
	rm -f runfile
