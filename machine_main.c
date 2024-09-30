// Daniel Landsman
/*

so far we got: 
machine_main.c and h
machine.c and h



*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "machine.h"
#include "bof.h"
#include "utilities.h"

#define DEBUG 1

// BOF loading module?
// memory module? (maybe registers as well)?

void testPrint(int argcP, char* argvP[]);

int main(int argc, char* argv[])
{

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
}

void testPrint(int argcP, char* argvP[])
{
    printf("There are %d args. They are:\n", argcP);
    for (int i = 0; i < argcP; i++)
    {
        printf("%s\n", argvP[i]);
    }
}