#ifndef ASM_H
#define ASM_H

#include "../../Processor/include/processor.h"
#include "../../Reader/include/tree.h"

#define SIZE_OF_ALL_VARIABLES 50



enum Errors_of_ASM
{
    NO_ASM_ERRORS              = 0,
    ERROR_OF_UNKNOWN_TEXT_CMD  = 1,
    ERROR_OF_READING_FROM_FILE = 2,
    ERROR_OF_NO_COMMANDS       = 3,
    ERROR_OF_CREATING_OUT_FILE = 4,
    ERROR_OF_DESTRUCTOR_ASM    = 5,
    ERROR_OF_CREATE_ARRAY      = 6,
    ERROR_OF_UNKNOWN_REGISTER  = 7,
    ERROR_OF_CONSTRUCTOR_ASM   = 8,
    ERROR_OF_PARSE_WORD        = 9,
    ERROR_OF_CREATE_ASM_FILE   = 10,
    ERROR_OF_OPERATE_TREE      = 11
};

struct Command
{
    char command[50];
    Commands transformed_command;
    double element;
    Registers reg;
    int ram_address;
    char label_name[50];
};

struct CMD
{
    char name[50];
    Commands transformed_name;
};

struct Label
{
    char name[50];
    int address;
};

struct Labels
{
    char name[50];
};

struct Table_labels
{
    struct Label *labels;
    size_t size_of_labels;
};

struct ASM
{
    struct Command *commands;
    size_t count_of_rows;
    struct Table_labels *table;
    FILE *file_pointer;
};

struct Function_type
{
    char function_name[100];
    struct Label *all_local_variables;
    size_t start_local_memory_address;
    size_t end_local_memory_address;
    bool is_parametres_processed;
};

struct Special_elements_for_processing
{
    struct MyStack current_function;
    size_t counter_of_if;
    size_t counter_of_while;
    size_t counter_of_else;
    size_t start_local_memory_address;
    struct MyStack stack_if;
    struct MyStack stack_while;
    struct MyStack stack_else;
    struct Label *all_variables;
    struct Labels *all_labels;
    struct Function_type *all_functions;
    bool is_body_of_functions;
    bool is_assignment;
};


Errors_of_ASM get_commands(struct ASM *Asm, struct Labels *all_labels, size_t size_of_all_labels);
Errors_of_ASM transform_commands(struct ASM *Asm, struct CMD *all_commands, size_t size_of_all_commands);
Errors_of_ASM create_file_with_commands(struct ASM *Asm);
Errors_of_ASM transform_programm_to_assembler(struct Tree *tree, struct Labels **all_labels);

#endif

