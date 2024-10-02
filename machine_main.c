#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "machine.h"
#include "bof.h"
#include "instruction.h"
#include "utilities.h"

// we can remove all this debug stuff when we're done
#define DEBUG 1

// we can remove this after we're done
void testPrint(int argcP, char* argvP[]);

    bool trace_program = true;
    bool print_assembly = false;

int main(int argc, char* argv[])
{
    if (DEBUG) 
    {
        printf("DEBUG ON\n");
        testPrint(argc, argv);
    }

    if (argc == 3 && strcmp(argv[1], "-p") == 0)
    {
        print_assembly = true;
        if (DEBUG) printf("DEBUG: Print mode activated\n");
    }

    BOFFILE bof;

    if (print_assembly) bof = bof_read_open(argv[2]);
    else bof = bof_read_open(argv[1]);

    if (DEBUG) printf("DEBUG: argv[1] is %s\n", argv[1]);

    load_bof(bof);

    if (print_assembly)
    {
        vm_print_program(stdout);
    }

    else
    {
        // Execute program
        while (1) 
        {
            bin_instr_t instr = memory.instrs[PC];
        PC++;


        //If tracing is enabled, print trace before executing instruction
        if(trace_program){
            trace_instruction(instr);
        }
        
        execute_instruction(instr, trace_program);

        //If tracing is enabled, print trace after executing instruction
        if(trace_program){
            trace_instruction(instr);
        }
    }
    }

    // machine run function, actual function body should be in machine.c
    return EXIT_SUCCESS;
}

// we can remove this after we're done
void testPrint(int argcP, char* argvP[])
{
    printf("There are %d args. They are:\n", argcP);
    for (int i = 0; i < argcP; i++)
    {
        printf("%s\n", argvP[i]);
    }
}