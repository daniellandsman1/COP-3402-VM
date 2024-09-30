#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bof.h"
#include "instruction.h"
#include "utilities.h"
#include "regname.h"

#define MEMORY_SIZE 32768

// Memory and registers
union memory_t {
    int words[MEMORY_SIZE];
    unsigned int uwords[MEMORY_SIZE];
    bin_instr_t instructions[MEMORY_SIZE];
} memory;

int PC = 0, HI = 0, LO = 0;
int GPR[8]; // General-purpose registers

// VM print function (-p flag)
void vm_print_program(const char* bof_file) {
    BOFFILE bof = bof_read_open(bof_file);
    if (bof.fileptr == NULL) { // Already checked in bof_read_open.
        fprintf(stderr, "Error opening BOF file: %s\n", bof_file);
        exit(1);
    }
    BOFHeader header = bof_read_header(bof);

    // Load and print instructions
    for (int i = 0; i < header.text_length; i++) {
        bin_instr_t instr;
        instruction_read(bof);
        printf("%d: ", i);
        instruction_print_assembly(stdout, &instr);
    }

    bof_close(bof);
}

// Fetch-execute cycle
void execute_instruction(bin_instr_t instr) {

    instr_type type = instruction_type(instr);

    switch (type) { // Need to identify type of instruction first to get opcode.
        case comp_instr_type:

            reg_num_type t = instr.comp.rt;
            offset_type ot = instr.comp.ot;
            reg_num_type s = instr.comp.rs;
            offset_type os = instr.comp.os;

            switch(instr.comp.func) {

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
            offset_type o = instr.othc.offset;
            arg_type i = instr.othc.arg;

            switch(instr.othc.func) {

                case LIT_F:
                    memory.words[GPR[reg] + machine_types_formOffset(o)] =
                    machine_types_sgnExt(i);
                    break;
                
                case ARI_F:
                    GPR[reg] = (GPR[reg] + machine_types_sgnExt(i));
                    break;

                case SRI_F:
                    GPR[reg] = (GPR[reg] - machine_types_sgnExt(i));
                    break;

                case MUL_F:
                    long long int res = memory.words[GPR[SP]] * 
                    (memory.words[GPR[reg] + machine_types_formOffset(o)]);

                    LO = (res & 0xFFFFFFFF);
                    HI = (res >> 32);
                    break;

                case DIV_F:

                    if (memory.words[GPR[reg] + machine_types_formOffset(o)] == 0) {
                        bail_with_error("Division by 0 encountered!");
                    }

                    LO = memory.words[GPR[SP]] / 
                    (memory.words[GPR[reg] + machine_types_formOffset(o)]);
                    HI = memory.words[GPR[SP]] % 
                    (memory.words[GPR[reg] + machine_types_formOffset(o)]);
                    break;

                case CFHI_F:
                    
                    break;

                case CFLO_F:

                    break;

                case SLL_F:

                    break;

                case SRL_F:

                    break;

                case JMP_F:

                    break;

                case CSI_F:

                    break;

                case JREL_F:

                    break;

                case SYS_F:

                    break;

                default:
                    bail_with_error("Other computational function code (%d) is invalid!", instr.othc.func);
                    break;
            }

            break;
        case immed_instr_type:
            break;
        case jump_instr_type:
            break;
        case syscall_instr_type:
            break;
        case error_instr_type:
            break;

        case OP_ADDI:
            GPR[instr.reg] += instr.immediate;
            break;
        case OP_EXIT:
            exit(instr.offset);
        default:
            fprintf(stderr, "Unknown opcode: %d\n", instr.opcode);
            exit(1);
    }
}

// VM execution
void vm_execute_program(const char* bof_file) {
    BOFFILE* bof = bof_read_open(bof_file);
    if (!bof) {
        fprintf(stderr, "Error opening BOF file: %s\n", bof_file);
        exit(1);
    }

    BOFHeader header;
    bof_read_header(bof, &header);

    // Load instructions into memory
    for (int i = 0; i < header.text_length; i++) {
        instruction_read(bof, &memory.instructions[i]);
    }
    bof_close(bof);

    // Execute program
    while (1) {
        instruction_t instr = memory.instructions[PC];
        PC++;
        execute_instruction(instr);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-p] <bof_file>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-p") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s -p <bof_file>\n", argv[0]);
            return 1;
        }
        vm_print_program(argv[2]);
    } else {
        vm_execute_program(argv[1]);
    }

    return 0;
}
