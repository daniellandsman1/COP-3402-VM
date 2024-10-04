// Daniel Landsman
// Michael John
// Christian Morton
// Ryan Rohan


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "machine.h"
#include "bof.h"
#include "instruction.h"
#include "utilities.h"


#define DEBUG 0

void testPrint(int argcP, char* argvP[]);

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

    if (DEBUG) printf("arg2 is %s", argv[2]);
    //BOFHeader headerTest = bof_read_header(bof);
    //printf("DEBUG: headerTest data length is %d\n", headerTest.data_length);

    if (DEBUG) printf("DEBUG: argv[1] is %s\n", argv[1]);

    load_bof(bof);

    if (print_assembly)
    {
        vm_print_program(stdout);
    }

    else
    {
        vm_run_program();
    }

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