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

void load_bof(BOFFILE bof) // work on loading
{
    // load header
    BOFHeader bHeader = bof_read_header(bof);

    // do some initialize stuff (reg/mem)
    init(bHeader);

    // invariant check
    invariant_check(bHeader);

    // load instructions
    load_instrs(bof, bHeader);

    // load global data
    load_globals(bof, bHeader);
}

void init(BOFHeader header) {

    // Set all registers to 0.
    for (int i = 0; i < NUM_REGISTERS; i++)
    {
        GPR[i] = 0;
    }

    // Set all memory to 0.
    for (int i = 0; i < MEMORY_SIZE_IN_WORDS; i++)
    {
        memory.words[i] = 0;
    }

    // Set GP, FP, SP, and PC registers appropriately.
    GPR[GP] = header.data_start_address;
    GPR[FP] = GPR[SP] = header.stack_bottom_addr;
    PC = header.text_start_address;
    HI = 0;
    LO = 0;
}

void invariant_check(BOFHeader header)
{
    //check if 0 is <= gp
    if (!(0 <= GPR[GP]))
    {
        bail_with_error("Global data starting address (%u) is less than 0!",
                        GPR[GP]);
    }

    //check if gp < sp
    if (!(GPR[GP] < GPR[SP]))
    {
        bail_with_error("Global data starting address (%u) is not less than the stack top address (%u)!",
                        GPR[GP], GPR[SP]);
    }

    //check if sp <= fp
    if (!(GPR[SP] <= GPR[FP]))
    {
        bail_with_error("Stack bottom address (%u) is not less than the stack top address (%u)!",
                        GPR[FP], GPR[SP]);
    }

    //check that fp < memory_size
    if (!(GPR[FP] < MEMORY_SIZE_IN_WORDS))
    {
        bail_with_error("Stack bottom address (%u) is not less than the memory size (%u)!",
                        GPR[FP], MEMORY_SIZE_IN_WORDS);
    }

    //check that 0 <= pc
    if (!(0 <= PC))
    {
        bail_with_error("Program counter (%u) is less than zero!",
                        PC);
    }

    //check that pc < memory_size
    if (!(PC < MEMORY_SIZE_IN_WORDS))
    {
        bail_with_error("Program counter (%u) is not less than the memory size (%u)!",
                        PC, MEMORY_SIZE_IN_WORDS);
    }

    if (DEBUG) printf("Invariant check passed!\n");
}

void load_instrs(BOFFILE bof, BOFHeader header) 
{
    int num_instrs = (header.text_length / BYTES_PER_WORD);
    for (int i = 0; i < num_instrs; i++) 
    {
        memory.instrs[i] = instruction_read(bof);
    }
}

void load_globals(BOFFILE bof, BOFHeader header)
{
    int num_globals = (header.data_length / BYTES_PER_WORD);
    int offset = (header.data_start_address / BYTES_PER_WORD);

    for (int i = 0; i < num_globals; i++)
    {
        memory.words[i + offset] = bof_read_word(bof);
    }
}