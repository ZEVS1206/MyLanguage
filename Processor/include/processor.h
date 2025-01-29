#ifndef CPU_H
#define CPU_H

#include "stack.h"

const int TOXIC = 1985;
const int VERSION = 1;
const int ACCURANCY = 2;
const long long int RAM_SIZE  = 100;

enum Registers
{
    NOT_A_REGISTER = 0,
    AX             = 1,
    BX             = 2,
    CX             = 3,
    DX             = 4
};

enum Commands
{
    CMD_PUSH       = 1,
    CMD_ADD        = 2,
    CMD_SUB        = 3,
    CMD_MUL        = 4,
    CMD_DIV        = 5,
    CMD_OUT        = 6,
    CMD_IN         = 7,
    CMD_SQRT       = 8,
    CMD_SIN        = 9,
    CMD_COS        = 10,
    CMD_DUMP       = 11,
    CMD_HLT        = 12,
    CMD_UNKNOWN    = 13,
    CMD_JMP        = 14, // jump
    CMD_JA         = 15, // >
    CMD_JAE        = 16, // >=
    CMD_JB         = 17, // <
    CMD_JBE        = 18, // <=
    CMD_JE         = 19, // ==
    CMD_JNE        = 20, // !=
    CMD_POP        = 21,
    CMD_LABEL      = 22,
    CMD_PRINT      = 23,
    CMD_PRINT_INF  = 24,
    CMD_PRINT_NONE = 25,
    CMD_HERE       = 26
};

enum Errors_of_CPU
{
    ERROR_OF_GET_POP_CMD                   = 6,
    ERROR_OF_GET_PUSH_CMD                  = 5,
    ERROR_OF_UNKNOWN_TYPE                  = 4,
    ERROR_OF_GETTING_INFORMATION_ABOUT_CMD = 3,
    ERROR_OF_UNKNOWN_CMD                   = 2,
    ERROR_OF_NOT_ENOUGH_ELEMENTS_IN_STACK  = 1,
    NO_CPU_ERRORS                          = 0
};

struct MySPU
{
    int *commands;
    size_t size_of_commands;
    size_t ip;
    MyStack *stack;
    int *registers;
    size_t size_of_registers;
    Stack_Elem_t *ram;
};

Errors_of_CPU create_commands();
Errors_of_CPU do_cmd(struct MySPU *spu);


#endif
