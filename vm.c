#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bof.h"
#include "instruction.h"
#include "utilities.h"
#include "regname.h"

#define MEMORY_SIZE 32768

// this should go in the machine_main.c file
bool trace_program = true;

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
            func_type func = instr.comp.func;

            switch(func) {

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
            func_type func = instr.othc.func;

            switch(func) {

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

                    reg_num_type reg = instr.syscall.reg;
                    offset_type o = instr.syscall.offset;
                    syscall_type code = instr.syscall.code;

                    switch(code) {

                        case exit_sc:
                            exit(machine_types_sgnExt(o));
                            break;

                        case print_str_sc:
                            memory.words[GPR[SP]] = 
                            printf("%s", (char*)&memory.words[GPR[reg] + machine_types_formOffset(o)]);
                            break;

                        case print_char_sc:
                            memory.words[GPR[SP]] = 
                            fputc(memory.words[GPR[reg] + machine_types_formOffset(o)], stdout);
                            break;

                        case read_char_sc:
                            memory.words[GPR[reg] + machine_types_formOffset(o)] =
                            getc(stdin);
                            break;

                        case start_tracing_sc:
                            trace_program = true;
                            break;

                        case stop_tracing_sc:
                            trace_program = false;
                            break;
                    }
                    break;

                default:
                    bail_with_error("Other computational function code (%hu) is invalid!", instr.othc.func);
                    break;
            }

            break;

        case immed_instr_type:
            switch (instr.immed.opcode) {
                case OP_ADDI:
                    GPR[instr.immed.reg] += machine_types_sgnExt(instr.immed.immed);
                    break;
                case OP_ANDI:
                    GPR[instr.immed.reg] &= machine_types_zeroExt(instr.immed.immed);
                    break;
                case OP_BNE:
                    if (GPR[instr.immed.reg] != GPR[SP]) {
                        PC += machine_types_formOffset(instr.immed.immed);
                    }
                    break;
                case OP_BEQ:
                    if (GPR[instr.immed.reg] == GPR[SP]) {
                        PC += machine_types_formOffset(instr.immed.immed);
                    }
                    break;
                // Other immediate instructions (BORI, XORI, etc.)
                default:
                    bail_with_error("Immediate instruction opcode (%d) is invalid!", instr.immed.opcode);
            }
            break;

        case jump_instr_type:
            switch(instr.jump.opcode) {
                case JMPA:
                    PC = formAddress(PC - 1, instr.jump.addr);
                    break;
                case CALL:
                    GPR[RA] = PC;
                    PC = formAddress(PC - 1, instr.jump.addr);
                    break;
                case RTN:
                    PC = GPR[RA];
                    break;
                default:
                    bail_with_error("Jump instruction opcode (%d) is invalid!", instr.jump.opcode);
            }
            break;

        case error_instr_type:
            bail_with_error("Opcode (%hu) is invalid!", instr.comp.op);
            break;
    }
}

// VM execution
void vm_execute_program(const char* bof_file) {
    BOFFILE bof = bof_read_open(bof_file);
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

void trace_instruction(bin_instr_t instr){

    //Print PC
    printf("PC: %d\n", PC);

    //Print GPRs
    printf("GPR[$gp]: %d GPR[$sp]: %d GPR[$fp]: %d GPR[$r3]: %d GPR[$r4]: %d\n", GPR[0], GPR[SP], GPR[FP], GPR[3], GPR[4]);
    printf("GPR[$r5]: %d GPR[$r6]: %d GPR[$ra]: %d\n", GPR[5], GPR[6], GPR[RA]);

    //Print Memory
    printf("%d: %d ...\n", GPR[GP], memory.words[GPR[GP]]);
    printf("%d: %d\n", GPR[SP], memory.words[GPR[SP]]);

    //Print Current Instruction
    printf("==> %d: ", PC - 1);
    instruction_print_assembly(stdout, &instr);
    printf("\n");
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
