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

uint16_t uint16_at(val* memory, address address) {
    uint16_t val = (uint16_t) memory[address] + (((uint16_t) memory[address + 1]) << 8);
    if (val >= 0x8000)
        val = (uint16_t) memory[val] + (((uint16_t) memory[val + 1]) << 8);

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
    fread(memory, sizeof(val), 1 << 16, fp);
    fclose(fp);

    printf("%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\n",
           uint16_at(memory, 0x8000),
           uint16_at(memory, 0x8001),
           uint16_at(memory, 0x8002),
           uint16_at(memory, 0x8003),
           uint16_at(memory, 0x8004),
           uint16_at(memory, 0x8005),
           uint16_at(memory, 0x8006),
           uint16_at(memory, 0x8007));

    address pc = 0;
    // TODO: these should be pushed onto a stack.
    uint16_t a;
    uint16_t b;
    while (true) {
        uint16_t v = uint16_at(memory, pc);

        switch (v) {
            case OPCODE_HALT: // 0
                free_memory(memory);

                return 0;
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
            case OPCODE_OUT: // 19
                a = uint16_at(memory, pc + (address) 2);
                printf("%c", a);
                pc = pc + (address) 4;
                break;
            case OPCODE_NOOP: // 21
                pc = pc + (address) 2;
                break;
            default:
                printf("NOT IMPLEMENTED: %d\n", v);
                return 1;
        }
    }
}