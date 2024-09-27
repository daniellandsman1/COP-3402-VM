
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bof.h"
#include "instruction.h"

#define MEMORY_SIZE 32768

// Memory and registers
union memory_t {
    int words[MEMORY_SIZE];
    unsigned int uwords[MEMORY_SIZE];
    instruction_t instructions[MEMORY_SIZE];
} memory;

int PC = 0, HI = 0, LO = 0;
int GPR[8]; // General-purpose registers

// VM print function (-p flag)
void vm_print_program(const char* bof_file) {
    BOFFILE* bof = bof_read_open(bof_file);
    if (!bof) {
        fprintf(stderr, "Error opening BOF file: %s\n", bof_file);
        exit(1);
    }
    BOFHeader header;
    bof_read_header(bof, &header);

    // Load and print instructions
    for (int i = 0; i < header.text_length; i++) {
        instruction_t instr;
        instruction_read(bof, &instr);
        printf("%d: ", i);
        instruction_print_assembly(stdout, &instr);
    }

    bof_close(bof);
}

// Fetch-execute cycle
void execute_instruction(instruction_t instr) {
    switch (instr.opcode) {
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
