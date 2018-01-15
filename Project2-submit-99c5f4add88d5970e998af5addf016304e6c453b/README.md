# Project 2. MIPS Simulator
Skeleton developed by CMU,
modified for KAIST CS311 purpose by THKIM.

## Instructions
There are three functions you need to implement, located in the following two files: `parse.c`, `run.c`

### 1. parse.c

**Implement** the following parsing functions.

    instruction parsing_instr(const char* buffer, int index)
    void parsing_data(const char* buffer, int index)

The `parsing_instr()` function is called for every instruction in the input file, and converts them into the `instruction` type.
The `instruction` type is defined in util.h

The `parsing_data()` function is called for every data field in the input file, and you need to fill the data into the `simulated memory`.
Use the `mem_read_32` and `mem_write_32` as mentioned in the Hints section below.

There is a helper function you can use to convert binary strings to int (32bit) variables:

    int fromBinary(char * s)


### 2. run.c

**Implement** the following function:

    void process_instruction()

The `process_instruction()` function is used by the `cycle()` instruction to execute the instruction at the current PC.
Your internal register/memory state should be changed according to the instruction that is pointed to by the current PC.

## Hints

* Always use the `mem_read_32()`, `mem_write32()` functions when trying to read or write from the `simulated memory`.
This includes your implementation of the load/store functions, but also when you are loading `data` region to the `simulated memory`.
* You may generate input files of very simple instructions to check the functionality of your simulator. For example, `add $1, $1, 5`.
Then you can check if the R1 of your simulator has been incremented by 5, etc..
* To compare your results to the reference simulator of the TAs that will be used to grade your simulator, execute `make test`.
