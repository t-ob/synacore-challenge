#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "opcodes.h"

typedef struct _number _register;
typedef uint8_t val;

struct _number {
    uint8_t low;
    uint8_t high;
};

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


int main(int argc, char *argv[]) {
    if (argc != 2) {
        // TODO: exit codes
        return 1;
    }
    _register r0, r1, r2, r3, r4, r5, r6, r7;

    val* memory = init_memory();
    struct stack* stack = init_stack();

    // Load program into memory.
    printf("%s\n", argv[1]);
    FILE *fp;
    fp = fopen(argv[1], "r");

    fread(memory, sizeof(val), 1 << 16, fp);
    fclose(fp);

    int pc = 0;
    uint16_t foo;
    while (memory[pc] != 0) {
        val low_byte = memory[pc];
        val high_byte = memory[pc + 1];

        uint16_t v = low_byte + (((uint16_t)  high_byte) << 8);

        switch (v) {
            case OPCODE_HALT:
                return 0;
            case OPCODE_OUT:
                foo = (uint16_t) memory[pc + 2] + (((uint16_t) memory[pc + 2]) << 8);
                printf("%c", foo);
                pc = pc + 4;
                break;
            case OPCODE_NOOP:
                pc = pc + 2;
                break;
            default:
                return 1;
        }
    }


    // TODO: free_stack ?
    free_memory(memory);

    return 0;
}