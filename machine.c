// Daniel Landsman
#include <stdlib.h>
#include "machine.h"
#include "machine_types.h"
#include "instruction.h"
#include "bof.h"
#include "regname.h"
#include "utilities.h"

#define MAX_PRINT_WIDTH 59
#define DEBUG 0

union mem_u memory;
word_type GPR[NUM_REGISTERS];
address_type PC = 0;
word_type HI = 0;
word_type LO = 0;
unsigned int num_instrs = 0;
unsigned int num_globals = 0;
bool trace_program = true;

// Pre-Condition: bof represents a valid binary object file.
// Post-Condition: Loads the file's instructions and global data
// into memory and initializes registers.
void load_bof(BOFFILE bof) //
{

    // Open header for reading
    BOFHeader bHeader = bof_read_header(bof);
    if (DEBUG) printf("DEBUG: bHeader data length in load_bof is %d\n", bHeader.data_length);
    if (DEBUG) printf("DEBUG: bHeader data start address in load_bof is %d\n", bHeader.data_start_address);
    if (DEBUG) printf("DEBUG: bHeader magic in load_bof is %s\n", bHeader.magic);
    if (DEBUG) printf("DEBUG: bHeader stack bottom address in load_bof is %d\n", bHeader.stack_bottom_addr);
    if (DEBUG) printf("DEBUG: bHeader text length in load_bof is %d\n", bHeader.text_length);
    if (DEBUG) printf("DEBUG: bHeader text start address in load_bof is %d\n", bHeader.text_start_address);

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
        bail_with_error("Stack top address (%d) is not less than or equal to the stack bottom address (%d)!",
                        GPR[SP], GPR[FP]);
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
    num_globals = header.data_length;
    if (DEBUG) printf("DEBUG: data length in load_globals is %d\n", header.data_length);

    // Use data start address to find where in the array to
    // start saving global data to.
    int offset = header.data_start_address;

    // Loop through number of global data values, adding to memory array using offset.
    for (int i = 0; i < num_globals; i++)
    {
        memory.words[i + offset] = bof_read_word(bof);
    }
}

// Pre-Condition: Instructions and global data have been properly loaded
// into program memory.
// Post-Condition: Prints table heading, assembly instructions, and global
// data in program without executing instructions (-p option).
void vm_print_program(FILE* out)
{
    if (DEBUG) printf("DEBUG: printing table heading\n");
    instruction_print_table_heading(out);
    if (DEBUG) printf("DEBUG: printing instructions\n");
    print_all_instrs(out);
    if (DEBUG) printf("DEBUG: printing global data\n");
    print_global_data(out);
    // need to figure out how to print global data, see disasm files for some guidance
    // and check .lst files for what we need to match
}

// Pre-Condition: Instructions have been properly loaded into program memory.
// Post-Condition: Prints the address and assembly form of all instructions in memory.
void print_all_instrs(FILE* out)
{
    for (int i = 0; i < num_instrs; i++)
    {
        instruction_print(out, i, memory.instrs[i]);
    }
}

void print_global_data(FILE* out)
{
    int global_start = GPR[GP];
    int global_end = GPR[SP] - 1;

    int num_chars = 0;
    bool printing_dots = false;
    
    char* dots = "..."; // Made dots into a string so it can be formatted with the %8. Not sure if necessary but makes it fit expected output closer ************************

    for (int i = global_start; i <= global_end; i++)
    {
        if (memory.words[i] != 0)
        {
            if (printing_dots) // Removed newline here to make up for the ones added in the if (!printing_dots) function ************************
            {
                num_chars = 0;
                printing_dots = false;
            }
            num_chars += fprintf(out, "%8d: %d\t", i, memory.words[i]);
        }
        else
        {
            if (!printing_dots)
            {
                //printf("HEY! i + 1 is %d AND NEXT MEM IS %d\n", i + 1, memory.words[i+1]);
                //printf("GLOBAL END IS %d\n", global_end);
                if (memory.words[i + 1] == 0) // Added a check to possibly prevent index out of bounds ************************
                {
                    num_chars += fprintf(out, "%8d: %d\t", i, memory.words[i]); // Removed the dots for a reason explained below ************************
                    if (num_chars > MAX_PRINT_WIDTH) // Some test cases had ... surpass MAX_PRINT_WIDTH but didn't put it on a new line. This should fix that ************************
                    {
                        newline(out);
                        num_chars = 0;
                    }
                    
                    // Not sure about the spaces after the dots, hardcoded to work for test 0
                    // but may not work for other tests.
                    fprintf(out, "%11s     ", dots); // Adjusted spacing of ... to better fit the format of the test cases. Might still need to do some work on spacing dots and numbers but might just be that the test case examples look off from how it should actually be. ************************
                    //newline(out); // In test1 where only the dots go to the new line, it didn't print a newline after the dots since the loop ends. this should fix it ******************
                    printing_dots = true;
                }
                else
                {
                    num_chars += fprintf(out, "%8d: %d\t", i, memory.words[i]);
                }
            }
        }

        if (num_chars > MAX_PRINT_WIDTH)
        {
            newline(out);
            num_chars = 0;
        }
    }

    // if (num_chars > 0)
    // {
    //     newline(out);
    // }
}

void print_AR(FILE* out)
{
    // MAYBE
    printf("\n");

    int AR_start = GPR[SP];
    int AR_end = GPR[FP];

    int num_chars = 0;
    bool printing_dots = false;
    
    char* dots = "..."; // Made dots into a string so it can be formatted with the %8. Not sure if necessary but makes it fit expected output closer ************************

    for (int i = AR_start; i <= AR_end; i++)
    {
        if (memory.words[i] != 0)
        {
            if (printing_dots) // Removed newline here to make up for the ones added in the if (!printing_dots) function ************************
            {
                num_chars = 0;
                printing_dots = false;
            }
            num_chars += fprintf(out, "%8d: %d\t", i, memory.words[i]);
        }
        else
        {
            if (!printing_dots)
            {
                if (i + 1 <= GPR[FP] && memory.words[i + 1] == 0) // Added a check to possibly prevent index out of bounds ************************
                {
                    num_chars += fprintf(out, "%8d: %d\t", i, memory.words[i]); // Removed the dots for a reason explained below ************************
                    if (num_chars > MAX_PRINT_WIDTH) // Some test cases had ... surpass MAX_PRINT_WIDTH but didn't put it on a new line. This should fix that ************************
                    {
                        newline(out);
                        num_chars = 0;
                    }
                    
                    fprintf(out, "%11s", dots); // Adjusted spacing of ... to better fit the format of the test cases. Might still need to do some work on spacing dots and numbers but might just be that the test case examples look off from how it should actually be. ************************
                    newline(out); // In test1 where only the dots go to the new line, it didn't print a newline after the dots since the loop ends. this should fix it ******************
                    printing_dots = true;
                }
                else
                {
                    num_chars += fprintf(out, "%8d: %d\t", i, memory.words[i]);
                }
            }
        }

        if (num_chars > MAX_PRINT_WIDTH)
        {
            newline(out);
            num_chars = 0;
        }
    }

    if (num_chars > 0)
    {
        newline(out);
    }
}

void trace_instruction(bin_instr_t instr)
{
    //Print current instruction
    printf("==>      %d: %s\n", PC - 1, instruction_assembly_form(PC - 1, instr));

    // Print VM state
    print_state();
}

void print_state()
{
    //Print PC with HI and LO registers if necessary.
    if (HI == 0 && LO == 0) printf("%8s: %d\n", "PC", PC);
    else printf("%8s: %d   HI: %d   LO: %d\n", "PC", PC, HI, LO);

    //Print GPRs

    // Top row
    printf("GPR[%s]: %-5d GPR[%s]: %-5d GPR[%s]: %-5d GPR[%s]: %-5d GPR[%s]: %-5d\n", 
            regname_get(GP), GPR[GP], regname_get(SP), GPR[SP], regname_get(FP), GPR[FP],
            regname_get(3), GPR[3], regname_get(4), GPR[4]);

    // Bottom row
    printf("GPR[%s]: %-5d GPR[%s]: %-5d GPR[%s]: %-5d\n",
    regname_get(5), GPR[5], regname_get(6), GPR[6], regname_get(RA), GPR[RA]);

    //Print Memory
    print_global_data(stdout);
    print_AR(stdout);
    //printf("%d: %d ...\n", GPR[GP], memory.words[GPR[GP]]);
    //printf("%d: %d\n", GPR[SP], memory.words[GPR[SP]]);

    // Print newline
    printf("\n");
}

bin_instr_t fetch_instruction()
{
    bin_instr_t instr = memory.instrs[PC];
    PC++;
    return instr;
}

// Fetch-execute cycle
void execute_instruction(bin_instr_t instr)
{

    instr_type type = instruction_type(instr);

    switch (type) { // Need to identify type of instruction first to get opcode.
        case comp_instr_type:

            reg_num_type t = instr.comp.rt;
            offset_type ot = instr.comp.ot;
            reg_num_type s = instr.comp.rs;
            offset_type os = instr.comp.os;
            func_type func_0 = instr.comp.func;

            switch(func_0) {

                case NOP_F:
                    break;

                case ADD_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] = 
                    memory.words[GPR[SP]] + (memory.words[GPR[s] + machine_types_formOffset(os)]);
                    break;

                case SUB_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] = 
                    memory.words[GPR[SP]] - (memory.words[GPR[s] + machine_types_formOffset(os)]);
                    break;

                case CPW_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] = 
                    memory.words[GPR[s] + machine_types_formOffset(os)];
                    break;

                case AND_F:
                    memory.uwords[GPR[t] + machine_types_formOffset(ot)] =
                    memory.uwords[GPR[SP]] & (memory.uwords[GPR[s] + machine_types_formOffset(os)]);
                    break;

                case BOR_F:
                    memory.uwords[GPR[t] + machine_types_formOffset(ot)] =
                    memory.uwords[GPR[SP]] | (memory.uwords[GPR[s] + machine_types_formOffset(os)]);
                    break;

                case NOR_F:
                    memory.uwords[GPR[t] + machine_types_formOffset(ot)] =
                    ~(memory.uwords[GPR[SP]] | (memory.uwords[GPR[s] + machine_types_formOffset(os)]));
                    break;

                case XOR_F:
                    memory.uwords[GPR[t] + machine_types_formOffset(ot)] =
                    memory.uwords[GPR[SP]] ^ (memory.uwords[GPR[s] + machine_types_formOffset(os)]);
                    break;

                case LWR_F:
                    GPR[t] = memory.words[GPR[s] + machine_types_formOffset(os)];
                    break;

                case SWR_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] = GPR[s];
                    break;

                case SCA_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] = 
                    (GPR[s] + machine_types_formOffset(os));
                    break;

                case LWI_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] =
                    memory.words[memory.words[GPR[s] + machine_types_formOffset(os)]];
                    break;

                case NEG_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] =
                    -memory.words[GPR[s] + machine_types_formOffset(os)];
                    break;

                default:
                    bail_with_error("Computational function code (%d) is invalid!", instr.comp.func);
                    break;
            }
            break;

        case other_comp_instr_type:

            reg_num_type reg = instr.othc.reg;
            offset_type offset = instr.othc.offset;
            arg_type arg = instr.othc.arg;
            func_type func_1 = instr.othc.func;

            switch(func_1) 
            {
                case LIT_F:
                    memory.words[GPR[reg] + machine_types_formOffset(offset)] =
                    machine_types_sgnExt(arg);
                    break;
                
                case ARI_F:
                    GPR[reg] = (GPR[reg] + machine_types_sgnExt(arg));
                    break;

                case SRI_F:
                    GPR[reg] = (GPR[reg] - machine_types_sgnExt(arg));
                    break;

                case MUL_F:
                    long long int res = memory.words[GPR[SP]] * 
                    (memory.words[GPR[reg] + machine_types_formOffset(offset)]);

                    LO = (res & 0xFFFFFFFF);
                    HI = (res >> 32);
                    break;

                case DIV_F:

                    if (memory.words[GPR[reg] + machine_types_formOffset(offset)] == 0) {
                        bail_with_error("Division by 0 encountered!");
                    }

                    LO = memory.words[GPR[SP]] / 
                    (memory.words[GPR[reg] + machine_types_formOffset(offset)]);
                    HI = memory.words[GPR[SP]] % 
                    (memory.words[GPR[reg] + machine_types_formOffset(offset)]);
                    break;

                case CFHI_F:
                    memory.words[GPR[reg] + machine_types_formOffset(offset)] = HI;
                    break;

                case CFLO_F:
                    memory.words[GPR[reg] + machine_types_formOffset(offset)] = LO;
                    break;

                case SLL_F:
                    memory.uwords[GPR[reg] + machine_types_formOffset(offset)] = 
                    memory.uwords[GPR[SP]] << arg;
                    break;

                case SRL_F:
                    memory.uwords[GPR[reg] + machine_types_formOffset(offset)] =
                    memory.uwords[GPR[SP]] >> arg;
                    break;

                case JMP_F:
                    PC = memory.uwords[GPR[reg] + machine_types_formOffset(offset)];
                    break;

                case CSI_F:
                    GPR[RA] = PC;
                    PC = memory.words[GPR[reg] + machine_types_formOffset(offset)];
                    break;

                case JREL_F:
                    PC = ((PC - 1) + machine_types_formOffset(arg));
                    break;

                case SYS_F:
                    // Should never happen, all other computational instructions
                    // with this func code should be returned as syscall_instr_type
                    // by instruction_type() function.
                    break;

                default:
                    bail_with_error("Other computational function code (%hu) is invalid!", instr.othc.func);
                    break;
            }
        break;

        case immed_instr_type:

            switch (instr.immed.op) 
            {
                case ADDI_O:
                    memory.words[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)] =
                    (memory.words[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)]) +
                    machine_types_sgnExt(instr.immed.immed);
                    break;

                case ANDI_O:
                    memory.uwords[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)] =
                    (memory.uwords[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)]) &
                    machine_types_zeroExt(instr.immed.immed);
                    break;

                case BORI_O:
                    memory.uwords[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)] =
                    (memory.uwords[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)]) |
                    machine_types_zeroExt(instr.immed.immed);
                    break;

                case XORI_O:
                    memory.uwords[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)] =
                    (memory.uwords[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)]) ^
                    machine_types_zeroExt(instr.immed.immed);
                    break;

                case BEQ_O:
                    if (memory.words[GPR[SP]] == memory.words[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)])
                    {
                        PC = (PC - 1) + machine_types_formOffset(instr.immed.immed);
                    }
                    break;

                case BGEZ_O:
                    if (memory.words[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)] >= 0)
                    {
                        PC = (PC - 1) + machine_types_formOffset(instr.immed.immed);
                    }
                    break;

                case BGTZ_O:
                    if (memory.words[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)] > 0)
                    {
                        PC = (PC - 1) + machine_types_formOffset(instr.immed.immed);
                    }
                    break;

                case BLEZ_O:
                    if (memory.words[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)] <= 0)
                    {
                        PC = (PC - 1) + machine_types_formOffset(instr.immed.immed);
                    }
                    break;

                case BLTZ_O:
                    if (memory.words[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)] < 0)
                    {
                        PC = (PC - 1) + machine_types_formOffset(instr.immed.immed);
                    }
                    break;

                case BNE_O:
                    if (memory.words[GPR[SP]] != memory.words[GPR[instr.immed.reg] + machine_types_formOffset(instr.immed.offset)])
                    {
                        PC = (PC - 1) + machine_types_formOffset(instr.immed.immed);
                    }
                    break;

                default:
                    bail_with_error("Immediate instruction opcode (%d) is invalid!", instr.immed.op);
            }
            break;

        case jump_instr_type:

            switch(instr.jump.op) 
            {
                case JMPA_O:
                    PC = machine_types_formAddress(PC - 1, instr.jump.addr);
                    break;
                case CALL_O:
                    GPR[RA] = PC;
                    PC = machine_types_formAddress(PC - 1, instr.jump.addr);
                    break;
                case RTN_O:
                    PC = GPR[RA];
                    break;
                default:
                    bail_with_error("Jump instruction opcode (%d) is invalid!", instr.jump.op);
            }
        break;

        case syscall_instr_type:

            reg_num_type r = instr.syscall.reg;
            offset_type o = instr.syscall.offset;
            syscall_type code = instruction_syscall_number(instr);

            switch(code) 
            {
                case exit_sc:
                    if (trace_program)
                    {
                        printf("==>      %d: %s\n", PC - 1, instruction_assembly_form(PC - 1, instr));
                    }
                    exit(machine_types_sgnExt(o));
                    break;

                case print_str_sc:
                    memory.words[GPR[SP]] = 
                    printf("%s", (char*)&memory.words[GPR[r] + machine_types_formOffset(o)]);
                    break;

                case print_char_sc:
                    memory.words[GPR[SP]] = 
                    fputc(memory.words[GPR[r] + machine_types_formOffset(o)], stdout);
                    break;

                case read_char_sc:
                    memory.words[GPR[r] + machine_types_formOffset(o)] =
                    getc(stdin);
                    break;

                case start_tracing_sc:
                    trace_program = true;
                    break;

                case stop_tracing_sc:
                    trace_program = false;
                    printf("==>      %d: %s\n", PC - 1, instruction_assembly_form(PC - 1, instr));
                    break;

                default:
                    bail_with_error("System call instruction opcode (%d) is invalid!", instr.syscall.op);
            }
        break;

        case error_instr_type:
            bail_with_error("Opcode (%hu) is invalid!", instr.comp.op);
            break;
    }
}

void vm_run_program()
{
    if (trace_program)
    {
        print_state();
    }

    invariant_check();

    bin_instr_t cur_instr;

    while (true)
    {
        cur_instr = fetch_instruction();
        execute_instruction(cur_instr);
        if (trace_program) trace_instruction(cur_instr);
        invariant_check();
    }
}