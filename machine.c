// Daniel Landsman
#include <stdlib.h>
#include "machine.h"
#include "machine_types.h"
#include "instruction.h"
#include "bof.h"
#include "regname.h"
#include "utilities.h"

#define MAX_PRINT_WIDTH 59
#define DEBUG 1

word_type GPR[NUM_REGISTERS];
address_type PC = 0;
word_type HI = 0;
word_type LO = 0;
unsigned int num_instrs = 0;
unsigned int num_globals = 0;

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
    num_globals = header.data_length;

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
	int global_end = global_start + num_globals;
	
	int num_chars = 0;
	bool no_dots_yet = true;
	
	for (int i = global_start; i < global_end; i++)
	{
		if (memory.words[i] != 0)
		{
			num_chars += fprintf(out, "%8d: %d\t", i, memory.words[i]);
			no_dots_yet = true;
		} else
		{
			if (no_dots_yet)
			{
				num_chars += fprintf(out, "%8d: %d\t", i, memory.words[i]);
				num_chars += fprintf(out, "...");
				no_dots_yet = false;
			}
		}
		if (num_chars > MAX_PRINT_WIDTH)
		{
			newline(out);
			num_chars = 0;
		}
	}
}

void trace_instruction(bin_instr_t instr)
{
    //Print PC
    printf("PC: %d\n", PC);

    //Print GPRs
    printf("GPR[$gp]: %d GPR[$sp]: %d GPR[$fp]: %d GPR[$r3]: %d GPR[$r4]: %d\n", GPR[0], GPR[SP], GPR[FP], GPR[3], GPR[4]);
    printf("GPR[$r5]: %d GPR[$r6]: %d GPR[$ra]: %d\n", GPR[5], GPR[6], GPR[RA]);

    //Print Memory
    printf("%d: %d ...\n", GPR[GP], memory.words[GPR[GP]]);
    printf("%d: %d\n", GPR[SP], memory.words[GPR[SP]]);

    //Print Current Instruction
    printf("==> %d: %s\n", PC - 1, instruction_assembly_form(PC - 1, instr));
}

bin_instr_t fetch_instruction()
{
    bin_instr_t instr = memory.instrs[PC];
    PC++;
    return instr;
}

// Fetch-execute cycle
void execute_instruction(bin_instr_t instr, bool trace_flag)
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
                    memory.words[GPR[SP]] + (memory.words[GPR[s]] + machine_types_formOffset(os));
                    break;

                case SUB_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] = 
                    memory.words[GPR[SP]] - (memory.words[GPR[s]] + machine_types_formOffset(os));
                    break;

                case CPW_F:
                    memory.words[GPR[t] + machine_types_formOffset(ot)] = 
                    memory.words[GPR[s]] + machine_types_formOffset(os);
                    break;

                case AND_F:
                    memory.uwords[GPR[t] + machine_types_formOffset(ot)] =
                    memory.uwords[GPR[SP]] & (memory.uwords[GPR[s]] + machine_types_formOffset(os));
                    break;

                case BOR_F:
                    memory.uwords[GPR[t] + machine_types_formOffset(ot)] =
                    memory.uwords[GPR[SP]] | (memory.uwords[GPR[s]] + machine_types_formOffset(os));
                    break;

                case NOR_F:
                    memory.uwords[GPR[t] + machine_types_formOffset(ot)] =
                    ~(memory.uwords[GPR[SP]] | (memory.uwords[GPR[s]] + machine_types_formOffset(os)));
                    break;

                case XOR_F:
                    memory.uwords[GPR[t] + machine_types_formOffset(ot)] =
                    memory.uwords[GPR[SP]] ^ (memory.uwords[GPR[s]] + machine_types_formOffset(os));
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
                    GPR[instr.immed.reg] += machine_types_sgnExt(instr.immed.immed);
                    break;

                case ANDI_O:
                    GPR[instr.immed.reg] &= machine_types_zeroExt(instr.immed.immed);
                    break;

                case BNE_O:
                    if (GPR[instr.immed.reg] != GPR[SP]) 
                    {
                        PC += machine_types_formOffset(instr.immed.immed);
                    }
                    break;

                case BEQ_O:
                    if (GPR[instr.immed.reg] == GPR[SP]) 
                    {
                        PC += machine_types_formOffset(instr.immed.immed);
                    }
                    break;

                // Other immediate instructions (BORI, XORI, etc.)
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
                    trace_flag = true;
                    break;

                case stop_tracing_sc:
                    trace_flag = false;
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