#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "machine.h"
#include "bof.h"
#include "utilities.h"

// we can remove all this debug stuff when we're done
#define DEBUG 1

// we can remove this after we're done
void testPrint(int argcP, char* argvP[]);

int main(int argc, char* argv[])
{
    bool trace_program = true;
    bool print_assembly = false;

    if (DEBUG) 
    {
        printf("DEBUG ON\n");
        testPrint(argc, argv);
    }

    if (argc == 2 && strcmp(argv[1], "-p") == 0)
    {
        print_assembly = true;
        if (DEBUG) printf("DEBUG: Print mode activated\n");
    }

    if (DEBUG) printf("DEBUG: argv[1] is %s\n", argv[1]);
    BOFFILE bof = bof_read_open(argv[1]);
    load_bof(bof);

    // if (print_assembly)
        // print program out

    // machine run function, actual function body should be in machine.c
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