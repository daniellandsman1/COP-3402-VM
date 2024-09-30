// Daniel Landsman
#ifndef _MACHINE_H
#define _MACHINE_H
#include "bof.h"

#define MEMORY_SIZE_IN_WORDS 32768

extern void load_bof(BOFFILE bof);

extern void init(BOFHeader header);

extern void invariant_check(BOFHeader header);

void load_instrs(BOFFILE bof, BOFHeader header);

void load_globals(BOFFILE bof, BOFHeader header);

#endif
