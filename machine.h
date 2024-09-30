// Daniel Landsman
#ifndef _MACHINE_H
#define _MACHINE_H
#include "bof.h"

#define MEMORY_SIZE_IN_WORDS 32768

// Pre-Condition: bof represents a valid binary object file.
// Post-Condition: Loads the file's instructions and global data
// into memory and initializes registers.
extern void load_bof(BOFFILE bof);

// Pre-Condition: header represents a valid BOF header.
// Post-Condition: Initializes memory to 0 and sets registers
// to their proper starting values according to the header.
extern void init(BOFHeader header);

// Pre-Condition: Registers are properly initialized and updated.
// Post-Condition: Checks the registers to make sure all invariants hold.
extern void invariant_check();

// Pre-Condition: bof and header are a valid binary object file and header, respectively
// Post-Condition: Loads instructions from the BOF into program memory
void load_instrs(BOFFILE bof, BOFHeader header);

// Pre-Condition: bof and header are a valid binary object file and header, respectively
// Post-Condition: Loads global data from the BOF into program memory
void load_globals(BOFFILE bof, BOFHeader header);

#endif
