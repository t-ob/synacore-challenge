#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "opcodes.h"

typedef uint8_t val;
typedef uint16_t address;

val* init_memory() {
    val* memory = malloc(sizeof(val) * (1 << 16));
    // TODO: zero memory?
    return memory;
}

void free_memory(val* memory) {
    free(memory);
}

// TODO: better name and register resolve logic
uint16_t read_uint16_no_resolve(val *memory, address addr) {
    if (0x8000 <= addr && addr < 0x8010)
        addr = (address) (0x8000 + 2 * (addr - 0x8000));

    uint16_t val = (uint16_t) memory[addr]
                   + (((uint16_t) memory[addr + 1]) << 8);

    if (0x8000 <= val && val < 0x8010)
        val = (uint16_t) (0x8000 + 2 * (val - 0x8000));

    return val;
}

uint16_t read_uint16(val *memory, address addr) {
    if (0x8000 <= addr && addr < 0x8010)
        addr = (address) (0x8000 + 2 * (addr - 0x8000));
    uint16_t val = (uint16_t) memory[addr] + (((uint16_t) memory[addr + 1]) << 8);
    if (val >= 0x8000 && val < 0x8010) {
        val = (address) (0x8000 + 2 * (val - 0x8000));
        val = (uint16_t) memory[val]
              + (((uint16_t) memory[val + 1]) << 8);
    }

    return val;
}

void write_uint16(val *memory, address address, uint16_t value) {
    memory[address] = (val) (value & 0xFF);
    memory[address + 1] = (val) ((value >> 8) & 0xFF);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        // TODO: exit codes
        return 1;
    }

    val* memory = init_memory();

    // Load program into memory.
    FILE *fp;
    fp = fopen(argv[1], "r");
    fread(memory, sizeof(val), 1 << 15, fp); // TODO: What about the rest?
    fclose(fp);

    // Zero registers?
    for (int i = 0x8000; i < 0x8010; ++i)
        memory[i] = 0;

    address pc = 0;
    address sc = 0x8010;
    // TODO: these should be pushed onto a stack.
    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t v;
    uint16_t last_v;
    while (true) {
        if (pc != 0)
            last_v = v;
        v = read_uint16(memory, pc);

        switch (v) {
            case OPCODE_HALT: // 0
                free_memory(memory);

                return 0;
            case OPCODE_SET: // 1
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);

                write_uint16(memory, a, b);

                pc = pc + (address) 6;
                break;
            case OPCODE_PUSH: // 2
                a = read_uint16(memory, pc + (address) 2);

                write_uint16(memory, sc, a);

                sc = sc + (address) 2;
                pc = pc + (address) 4;
                break;
            case OPCODE_POP: // 3
                if (sc == 0x8010) {
                    printf("FATAL ERROR: empty stack\n");
                    return 1;
                }
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, sc - (address) 2);

                write_uint16(memory, a, b);

                sc = sc - (address) 2;
                pc = pc + (address) 4;
                break;
            case OPCODE_EQ: // 4
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                c = read_uint16(memory, pc + (address) 6);

                if (b == c) {
                    write_uint16(memory, a, 1);
                } else {
                    write_uint16(memory, a, 0);
                }

                pc = pc + (address) 8;
                break;
            case OPCODE_GT: // 5
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                c = read_uint16(memory, pc + (address) 6);

                if (b > c) {
                    write_uint16(memory, a, 1);
                } else {
                    write_uint16(memory, a, 0);
                }

                pc = pc + (address) 8;
                break;
            case OPCODE_JMP: // 6
                a = read_uint16(memory, pc + (address) 2);
                pc = (address) 2 * a;
                break;
            case OPCODE_JT: // 7
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);

                if (a != 0) {
                    pc = (address) 2 * b;
                } else {
                    pc = pc + (address) 6;
                }
                break;
            case OPCODE_JF: // 8
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);

                if (a == 0) {
                    pc = (address) 2 * b;
                } else {
                    pc = pc + (address) 6;
                }
                break;
            case OPCODE_ADD: // 9
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                c = read_uint16(memory, pc + (address) 6);

                uint16_t sum = (uint16_t) ((b + c) & 0x7FFF);

                write_uint16(memory, a, sum);

                pc = pc + (address) 8;
                break;
            case OPCODE_MULT: // 10
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                c = read_uint16(memory, pc + (address) 6);

                uint16_t product = (uint16_t) ((b * c) & 0x7FFF);

                write_uint16(memory, a, product);

                pc = pc + (address) 8;
                break;
            case OPCODE_MOD: // 11
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                c = read_uint16(memory, pc + (address) 6);

                uint16_t remainder = (uint16_t) ((b % c) & 0x7FFF);

                write_uint16(memory, a, remainder);

                pc = pc + (address) 8;
                break;
            case OPCODE_AND: // 12
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                c = read_uint16(memory, pc + (address) 6);

                uint16_t and = b & c;

                write_uint16(memory, a, and);

                pc = pc + (address) 8;
                break;
            case OPCODE_OR: // 13
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                c = read_uint16(memory, pc + (address) 6);

                uint16_t or = b | c;

                write_uint16(memory, a, or);

                pc = pc + (address) 8;
                break;
            case OPCODE_NOT: // 14
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);

                uint16_t not = (uint16_t) ((~b) & 0x7FFF);

                write_uint16(memory, a, not);

                pc = pc + (address) 6;
                break;
            case OPCODE_RMEM: // 15
                a = read_uint16_no_resolve(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);

                uint16_t rmem_val = read_uint16(memory, (address) 2 * b);

                write_uint16(memory, a, rmem_val);

                pc = pc + (address) 6;
                break;
            case OPCODE_WMEM: // 16
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16_no_resolve(memory, pc + (address) 4);

                address rmem_addr = (address) 2 * a;

                write_uint16(memory, rmem_addr, b);

                pc = pc + (address) 6;
                break;
            case OPCODE_CALL: // 17
                a = read_uint16(memory, pc + (address) 2);

                address next_address = (address) ((pc + 4) / 2);

                write_uint16(memory, sc, next_address);

                sc = sc + (address) 2;
                pc = (address) 2 * a;
                break;
            case OPCODE_RET: // 18
                if (sc == 0x8010) {
                    printf("FATAL ERROR: empty stack\n");
                    return 1;
                }
                a = read_uint16_no_resolve(memory, sc - (address) 2);
                pc = (address) (2 * a);

                sc = sc - (address) 2;
                break;
            case OPCODE_OUT: // 19
                a = read_uint16(memory, pc + (address) 2);
                printf("%c", a);
                pc = pc + (address) 4;
                break;
            case OPCODE_NOOP: // 21
                pc = pc + (address) 2;
                break;
            default:
                printf("NOT IMPLEMENTED: %d\t(%x)\n", v, pc);
                return 1;
        }
    }
}