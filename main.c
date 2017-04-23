#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "operations.h"

typedef uint8_t val;
typedef uint32_t address;

val* init_memory(size_t size) {
    val* memory = malloc(size);
    return memory;
}

void free_memory(val* memory) {
    free(memory);
}

uint16_t read_uint16(val *memory, address addr) {
    uint16_t val = (uint16_t) memory[addr] + (((uint16_t) memory[addr + 1]) << 8);

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

    val* memory = init_memory(sizeof(val) * (1 << 17));

    // Load program into memory.
    FILE *fp;
    fp = fopen(argv[1], "r");
    fread(memory, sizeof(val), 1 << 16, fp);
    fclose(fp);

    // Init addresses of program counter, r0 and stack counter.
    address pc = 0;
    address r0 = 0x10000;
    address sc = 0x10010;

    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t v;
    while (true) {
        v = read_uint16(memory, pc);

        switch (v) {
            case OPCODE_HALT: // 0
                free_memory(memory);

                return 0;
            case OPCODE_SET: // 1
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));

                write_uint16(memory, r0 + 2 * (a - 0x8000), b);

                pc = pc + (address) 6;
                break;
            case OPCODE_PUSH: // 2
                a = read_uint16(memory, pc + (address) 2);
                if (a >= 0x8000 && a < 0x8008)
                    a = read_uint16(memory, r0 + 2 * (a - 0x8000));

                write_uint16(memory, sc, a);

                sc = sc + (address) 2;
                pc = pc + (address) 4;
                break;
            case OPCODE_POP: // 3
                if (sc == 0x8010) {
                    printf("FATAL ERROR: empty stack\n");
                    return 1;
                }
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, sc - (address) 2);

                write_uint16(memory, r0 + 2 * (a - 0x8000), b);

                sc = sc - (address) 2;
                pc = pc + (address) 4;
                break;
            case OPCODE_EQ: // 4
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));
                c = read_uint16(memory, pc + (address) 6);
                if (c >= 0x8000 && c < 0x8008)
                    c = read_uint16(memory, r0 + 2 * (c - 0x8000));

                if (b == c) {
                    write_uint16(memory, r0 + 2 * (a - 0x8000), 1);
                } else {
                    write_uint16(memory, r0 + 2 * (a - 0x8000), 0);
                }

                pc = pc + (address) 8;
                break;
            case OPCODE_GT: // 5
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));
                c = read_uint16(memory, pc + (address) 6);
                if (c >= 0x8000 && c < 0x8008)
                    c = read_uint16(memory, r0 + 2 * (c - 0x8000));

                if (b > c) {
                    write_uint16(memory, r0 + 2 * (a - 0x8000), 1);
                } else {
                    write_uint16(memory, r0 + 2 * (a - 0x8000), 0);
                }

                pc = pc + (address) 8;
                break;
            case OPCODE_JMP: // 6
                a = read_uint16(memory, pc + (address) 2);
                if (a >= 0x8000 && a < 0x8008)
                    a = read_uint16(memory, r0 + 2 * (a - 0x8000));
                pc = (address) 2 * a;
                break;
            case OPCODE_JT: // 7
                a = read_uint16(memory, pc + (address) 2);
                if (a >= 0x8000 && a < 0x8008)
                    a = read_uint16(memory, r0 + 2 * (a - 0x8000));
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (a - 0x8000));

                if (a != 0) {
                    pc = (address) 2 * b;
                } else {
                    pc = pc + (address) 6;
                }
                break;
            case OPCODE_JF: // 8
                a = read_uint16(memory, pc + (address) 2);
                if (a >= 0x8000 && a < 0x8008)
                    a = read_uint16(memory, r0 + 2 * (a - 0x8000));
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (a - 0x8000));

                if (a == 0) {
                    pc = (address) 2 * b;
                } else {
                    pc = pc + (address) 6;
                }
                break;
            case OPCODE_ADD: // 9
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));
                c = read_uint16(memory, pc + (address) 6);
                if (c >= 0x8000 && c < 0x8008)
                    c = read_uint16(memory, r0 + 2 * (c - 0x8000));

                uint16_t sum = (uint16_t) ((b + c) & 0x7FFF);

                write_uint16(memory, r0 + 2 * (a - 0x8000), sum);

                pc = pc + (address) 8;
                break;
            case OPCODE_MULT: // 10
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));
                c = read_uint16(memory, pc + (address) 6);
                if (c >= 0x8000 && c < 0x8008)
                    c = read_uint16(memory, r0 + 2 * (c - 0x8000));

                uint16_t product = (uint16_t) ((b * c) & 0x7FFF);

                write_uint16(memory, r0 + 2 * (a - 0x8000), product);

                pc = pc + (address) 8;
                break;
            case OPCODE_MOD: // 11
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));
                c = read_uint16(memory, pc + (address) 6);
                if (c >= 0x8000 && c < 0x8008)
                    c = read_uint16(memory, r0 + 2 * (c - 0x8000));

                uint16_t remainder = (uint16_t) ((b % c) & 0x7FFF);

                write_uint16(memory, r0 + 2 * (a - 0x8000), remainder);

                pc = pc + (address) 8;
                break;
            case OPCODE_AND: // 12
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));
                c = read_uint16(memory, pc + (address) 6);
                if (c >= 0x8000 && c < 0x8008)
                    c = read_uint16(memory, r0 + 2 * (c - 0x8000));

                uint16_t and = b & c;

                write_uint16(memory, r0 + 2 * (a - 0x8000), and);

                pc = pc + (address) 8;
                break;
            case OPCODE_OR: // 13
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));
                c = read_uint16(memory, pc + (address) 6);
                if (c >= 0x8000 && c < 0x8008)
                    c = read_uint16(memory, r0 + 2 * (c - 0x8000));

                uint16_t or = b | c;

                write_uint16(memory, r0 + 2 * (a - 0x8000), or);

                pc = pc + (address) 8;
                break;
            case OPCODE_NOT: // 14
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));

                uint16_t not = (uint16_t) ((~b) & 0x7FFF);

                write_uint16(memory, r0 + 2 * (a - 0x8000), not);

                pc = pc + (address) 6;
                break;
            case OPCODE_RMEM: // 15
                a = read_uint16(memory, pc + (address) 2);
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));

                uint16_t rmem_val = read_uint16(memory, (address) 2 * (address) b);

                write_uint16(memory, r0 + 2 * (a - 0x8000), rmem_val);

                pc = pc + (address) 6;
                break;
            case OPCODE_WMEM: // 16
                a = read_uint16(memory, pc + (address) 2);
                if (a >= 0x8000 && a < 0x8008)
                    a = read_uint16(memory, r0 + 2 * (a - 0x8000));
                b = read_uint16(memory, pc + (address) 4);
                if (b >= 0x8000 && b < 0x8008)
                    b = read_uint16(memory, r0 + 2 * (b - 0x8000));

                address wmem_addr;

                wmem_addr = (address) 2 * (address) a;

                write_uint16(memory, wmem_addr, b);

                pc = pc + (address) 6;
                break;
            case OPCODE_CALL: // 17
                a = read_uint16(memory, pc + (address) 2);
                if (a >= 0x8000 && a < 0x8008)
                    a = read_uint16(memory, r0 + 2 * (a - 0x8000));

                uint16_t next_address = (uint16_t) ((pc + 4) / 2);

                write_uint16(memory, sc, next_address);

                sc = sc + (address) 2;
                pc = (address) 2 * a;
                break;
            case OPCODE_RET: // 18
                if (sc == 0x8010) {
                    printf("FATAL ERROR: empty stack\n");
                    return 1;
                }
                a = read_uint16(memory, sc - (address) 2);
                pc = (address) (2 * a);

                sc = sc - (address) 2;
                break;
            case OPCODE_OUT: // 19
                a = read_uint16(memory, pc + (address) 2);
                if (a >= 0x8000 && a < 0x8008)
                    a = read_uint16(memory, r0 + 2 * (a - 0x8000));

                printf("%c", a);
                pc = pc + (address) 4;
                break;
            case OPCODE_IN: // 20
                a = read_uint16(memory, pc + (address) 2);
                int char_in = getchar();

                address in_addr = 2 * (address) a;

                write_uint16(memory, in_addr, (uint16_t) char_in);

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