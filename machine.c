// Daniel Landsman
#include <stdlib.h>
#include "machine.h"
#include "machine_types.h"
#include "instruction.h"
#include "bof.h"
#include "regname.h"
#include "utilities.h"

#define DEBUG 1

// Memory
static union mem_u
{
word_type words[MEMORY_SIZE_IN_WORDS];
uword_type uwords[MEMORY_SIZE_IN_WORDS];
bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} memory;

// General purpose registers
static word_type GPR[NUM_REGISTERS];

// HI and LO registers
static word_type HI;
static word_type LO;

// Program counter
static address_type PC;

static unsigned int num_instrs;

// Pre-Condition: bof represents a valid binary object file.
// Post-Condition: Loads the file's instructions and global data
// into memory and initializes registers.
void load_bof(BOFFILE bof) //
{
    // Open header for reading
    BOFHeader bHeader = bof_read_header(bof);

    // Initialize registers and memory
    init(bHeader);

    // Check to make sure everything was initialized properly.
    invariant_check(bHeader);

    // Load program instructions
    load_instrs(bof, bHeader);

    // Load program global data
    load_globals(bof, bHeader);
}

// Pre-Condition: header represents a valid BOF header.
// Post-Condition: Initializes memory to 0 and sets registers
// to their proper starting values according to the header.
void init(BOFHeader header) {

    // Set all registers to 0
    for (int i = 0; i < NUM_REGISTERS; i++)
    {
        GPR[i] = 0;
    }

    // Set all memory to 0
    for (int i = 0; i < MEMORY_SIZE_IN_WORDS; i++)
    {
        memory.words[i] = 0;
    }

    // Set GP, FP, and SP registers appropriately
    GPR[GP] = header.data_start_address;
    GPR[FP] = GPR[SP] = header.stack_bottom_addr;

    // Properly initialize special registers
    PC = header.text_start_address;
    HI = 0;
    LO = 0;
}

// Pre-Condition: Registers are properly initialized and updated.
// Post-Condition: Checks the registers to make sure all invariants hold.
void invariant_check()
{
    // Check if 0 is <= global pointer
    if (!(0 <= GPR[GP]))
    {
        bail_with_error("Global data starting address (%d) is less than 0!",
                        GPR[GP]);
    }

    // Check if global pointer < stack pointer
    if (!(GPR[GP] < GPR[SP]))
    {
        bail_with_error("Global data starting address (%d) is not less than the stack top address (%d)!",
                        GPR[GP], GPR[SP]);
    }

    // Check if stack pointer <= frame pointer
    if (!(GPR[SP] <= GPR[FP]))
    {
        bail_with_error("Stack bottom address (%d) is not less than the stack top address (%d)!",
                        GPR[FP], GPR[SP]);
    }

    // Check that framep pointer < memory size
    if (!(GPR[FP] < MEMORY_SIZE_IN_WORDS))
    {
        bail_with_error("Stack bottom address (%d) is not less than the memory size (%d)!",
                        GPR[FP], MEMORY_SIZE_IN_WORDS);
    }

    // Check that 0 <= program counter
    if (!(0 <= PC))
    {
        bail_with_error("Program counter (%u) is less than zero!",
                        PC);
    }

    // Check that program counter < memory size
    if (!(PC < MEMORY_SIZE_IN_WORDS))
    {
        bail_with_error("Program counter (%u) is not less than the memory size (%d)!",
                        PC, MEMORY_SIZE_IN_WORDS);
    }

    if (DEBUG) printf("Invariant check passed!\n");
}

// Pre-Condition: bof and header are a valid binary object file and header, respectively
// Post-Condition: Loads instructions from the BOF into program memory
void load_instrs(BOFFILE bof, BOFHeader header) 
{
    // Number of instructions is simply text length since word addressed.
    num_instrs = header.text_length;

    // Loop through number of instructions, adding to memory array
    for (int i = 0; i < num_instrs; i++) 
    {
        memory.instrs[i] = instruction_read(bof);
    }
}

// Pre-Condition: bof and header are a valid binary object file and header, respectively
// Post-Condition: Loads global data from the BOF into program memory
void load_globals(BOFFILE bof, BOFHeader header)
{
    // Length of global data is the number of global data values.
    int num_globals = header.data_length;

    // Use data start address to find where in the array to
    // start saving global data to.
    int offset = header.data_start_address;

    // Loop through number of global data values, adding to memory array using offset.
    for (int i = 0; i < num_globals; i++)
    {
        memory.words[i + offset] = bof_read_word(bof);
    }
}

void vm_print_program(FILE* out)
{
    instruction_print_table_heading(out);
    print_all_instrs(out);
    // need to figure out how to print global data, see disasm files for some guidance
    // and check .lst files for what we need to match
}

void print_all_instrs(FILE* out)
{
    for (int i = 0; i < num_instrs; i++)
    {
        instruction_print(out, i, memory.instrs[i]);
    }
}