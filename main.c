#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "opcodes.h"

typedef uint8_t val;
typedef uint16_t address;

struct stack {
    val value;
    struct stack* next;
};

void stack_push(struct stack *s, val v) {
    struct stack* tmp = malloc(sizeof(struct stack));
    tmp->next = s->next;
    tmp->value = s->value;
    s->value = v;
    s->next = tmp;
}

val stack_pop(struct stack *s) {
    val tmp = s->value;
    // TODO: clear up node?
    s = s->next;

    return tmp;
}

val stack_peek(struct stack *s) {
    return s->value;
}

struct stack* init_stack() {
    struct stack* s = malloc(sizeof(struct stack));
    return s;
}

val* init_memory() {
    val* memory = malloc(sizeof(val) * (1 << 16));
    // TODO: zero memory?
    return memory;
}

void free_memory(val* memory) {
    free(memory);
}

// TODO: better name and register resolve logic
uint16_t uint16_no_resolve_at(val* memory, address address) {
    if (0x8000 <= address && address < 0x8010)
        address = 0x8000 + 2 * (address - 0x8000);

    uint16_t val = (uint16_t) memory[address]
                   + (((uint16_t) memory[address + 1]) << 8);

    if (0x8000 <= val && val < 0x8010)
        val = 0x8000 + 2 * (val - 0x8000);

    return val;
}

uint16_t uint16_at(val* memory, address address) {
    if (0x8000 <= address && address < 0x8010)
        address = 0x8000 + 2 * (address - 0x8000);
    uint16_t val = (uint16_t) memory[address] + (((uint16_t) memory[address + 1]) << 8);
    if (val >= 0x8000 && val < 0x8010) {
        val = 0x8000 + 2 * (val - 0x8000);
        val = (uint16_t) memory[val]
              + (((uint16_t) memory[val + 1]) << 8);
    }

    return val;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        // TODO: exit codes
        return 1;
    }

    val* memory = init_memory();
    struct stack* stack = init_stack();

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
        v = uint16_at(memory, pc);

        uint16_t r0 = uint16_at(memory, 0x8000);
        uint16_t r1 = uint16_at(memory, 0x8001);

        switch (v) {
            case OPCODE_HALT: // 0
                free_memory(memory);

                return 0;
            case OPCODE_SET: // 1
                a = uint16_no_resolve_at(memory, pc + (address) 2);
                b = uint16_at(memory, pc + (address) 4);

                memory[a] = (val) (b & 0xFF);
                memory[a + 1] = (val) ((b >> 8) & 0xFF);

                pc = pc + (address) 6;
                break;
            case OPCODE_PUSH: // 2
                a = uint16_at(memory, pc + (address) 2);

                memory[sc] = (val) (a & 0xFF);
                memory[sc + 1] = (val) ((a >> 8) & 0xFF);

                sc = sc + (address) 2;
                pc = pc + (address) 4;
                break;
            case OPCODE_POP: // 3
                if (sc == 0x8010) {
                    printf("FATAL ERROR: empty stack\n");
                    return 1;
                }
                a = uint16_no_resolve_at(memory, pc + (address) 2);
                b = uint16_at(memory, sc - (address) 2);

                memory[a] = (val) (b & 0xFF);
                memory[a + 1] = (val) ((b >> 8) & 0xFF);

                sc = sc - (address) 2;
                pc = pc + (address) 4;
                break;
            case OPCODE_EQ: // 4
                a = uint16_no_resolve_at(memory, pc + (address) 2);
                b = uint16_at(memory, pc + (address) 4);
                c = uint16_at(memory, pc + (address) 6);
                
                if (b == c) {
                    memory[a] = (val) 1;
                    memory[a + 1] = (val) 0;
                } else {
                    memory[a] = (val) 0;
                    memory[a + 1] = (val) 0;
                }

                uint16_t foo = uint16_at(memory, a);

                pc = pc + (address) 8;
                break;
            case OPCODE_JMP: // 6
                a = uint16_at(memory, pc + (address) 2);
                pc = (address) 2 * a;
                break;
            case OPCODE_JT: // 7
                a = uint16_at(memory, pc + (address) 2);
                b = uint16_at(memory, pc + (address) 4);

                if (a != 0) {
                    pc = (address) 2 * b;
                } else {
                    pc = pc + (address) 6;
                }
                break;
            case OPCODE_JF: // 8
                a = uint16_at(memory, pc + (address) 2);
                b = uint16_at(memory, pc + (address) 4);

                if (a == 0) {
                    pc = (address) 2 * b;
                } else {
                    pc = pc + (address) 6;
                }
                break;
            case OPCODE_ADD: // 9
                a = uint16_no_resolve_at(memory, pc + (address) 2);
                b = uint16_at(memory, pc + (address) 4);
                c = uint16_at(memory, pc + (address) 6);

                uint16_t sum = (uint16_t) ((b + c) % 0x8000);

                memory[a] = (val) (sum & 0xFF);
                memory[a + 1] = (val) ((sum >> 8) & 0xFF);

                pc = pc + (address) 8;
                break;
            case OPCODE_OUT: // 19
                a = uint16_at(memory, pc + (address) 2);
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