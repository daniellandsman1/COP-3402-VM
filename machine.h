// Daniel Landsman
#ifndef _MACHINE_H
#define _MACHINE_H
#include "bof.h"
#include "instruction.h"
#include "regname.h"

#define MEMORY_SIZE_IN_WORDS 32768

// Memory
extern union mem_u
{
word_type words[MEMORY_SIZE_IN_WORDS];
uword_type uwords[MEMORY_SIZE_IN_WORDS];
bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} memory;

// General purpose registers
extern word_type GPR[NUM_REGISTERS];

// HI and LO registers
extern word_type HI;
extern word_type LO;

// Program counter
extern address_type PC;

extern unsigned int num_instrs;
extern unsigned int num_globals;

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
extern void load_instrs(BOFFILE bof, BOFHeader header);

// Pre-Condition: bof and header are a valid binary object file and header, respectively
// Post-Condition: Loads global data from the BOF into program memory
extern void load_globals(BOFFILE bof, BOFHeader header);

// Pre-Condition: Instructions and global data have been properly loaded
// into program memory.
// Post-Condition: Prints table heading, assembly instructions, and global
// data in program without executing instructions (-p option).
extern void vm_print_program(FILE* out);

// Pre-Condition: Instructions have been properly loaded into program memory.
// Post-Condition: Prints the address and assembly form of all instructions in memory.
// to the file stream out.
extern void print_all_instrs(FILE* out);

extern void print_global_data(FILE* out);

extern void trace_instruction(bin_instr_t instr);

extern bin_instr_t fetch_instruction();

extern void execute_instruction(bin_instr_t instr, bool trace_flag);

extern void print_state();

extern void vm_run_program(bool trace_flag);

#endif