#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "assembler.h"
#include "../../Reader/include/tree.h"
#include "../../Processor/include/processor.h"
#include "stack.h"
static void bypass_of_tree(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);
static void choose_way_of_operating(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);
static void process_operator_assignment(struct Node *root, FILE *file_pointer, struct Label **all_variables);
static void process_expression_after_assignment(struct Node *root, FILE *file_pointer, struct Label **all_variables);
static void process_variable(struct Value *value, FILE *file_pointer, struct Label **all_variables);
static void process_operation(struct Value *value, FILE *file_pointer);
static void process_operator_if(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);
static void process_comparison_expression(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, Programm_operators operator_);
static void process_expression_after_comparison(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels);
static void process_comparison_operation(struct Value *value, FILE *file_pointer);
static void process_operator_while(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);


Errors_of_ASM transform_programm_to_assembler(struct Tree *tree, struct Labels **all_labels)
{
    if (tree == NULL)
    {
        return ERROR_OF_OPERATE_TREE;
    }
    FILE *file_pointer = fopen("source/asm_programm.txt", "w");
    if (file_pointer == NULL)
    {
        return ERROR_OF_CREATE_ASM_FILE;
    }
    struct Label *all_variables = (struct Label *) calloc (SIZE_OF_ALL_VARIABLES, sizeof(struct Label));
    if (all_variables == NULL)
    {
        return ERROR_OF_OPERATE_TREE;
    }
    for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    {
        all_variables[index].address = -1;
    }
    struct MyStack stack_if = {0};
    struct MyStack stack_while = {0};
    Errors error = NO_ERRORS;
    error = STACK_CTOR(&stack_if, 10);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = STACK_CTOR(&stack_while, 10);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    size_t counter_of_if = 0;
    size_t counter_of_while = 0;
    bypass_of_tree(tree->root, file_pointer, &all_variables, all_labels, &counter_of_if, &counter_of_while, &stack_if, &stack_while);
    fprintf(file_pointer, "hlt\n");
    free(all_variables);
    fclose(file_pointer);
    error = stack_destructor(&stack_if);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = stack_destructor(&stack_while);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    return NO_ASM_ERRORS;
}

static void bypass_of_tree(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while)
{
    if (root == NULL)
    {
        return;
    }
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF WRITING TO FILE\n");
        abort();
    }
    bypass_of_tree(root->left, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    choose_way_of_operating(root, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    if ((root->value).type != OPERATOR ||
        ((root->value).operator_ != OPERATOR_IF &&
         (root->value).operator_ != OPERATOR_WHILE))
    {
        bypass_of_tree(root->right, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    return;
}

static void choose_way_of_operating(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while)
{
    if (root == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF OPERATING TREE\n");
        abort();
    }
    if ((root->value).type != OPERATOR)
    {
        return;
    }
    switch ((root->value).operator_)
    {
        case OPERATOR_ASSIGNMENT:
        {
            process_operator_assignment(root, file_pointer, all_variables);
            return;
        }
        case OPERATOR_IF:
        {
            process_operator_if(root, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
            return;
        }
        case OPERATOR_WHILE:
        {
            process_operator_while(root, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
            return;
        }
        default: return;
    }
}

static void process_operator_while(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while)
{
    if (root == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATOR WHILE\n");
        abort();
    }
    (*counter_of_while)++;
    Errors error = stack_push(stack_while, (double)(*counter_of_while));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    process_comparison_expression(root->left, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, OPERATOR_WHILE);
    fprintf(file_pointer, "begin_while%lu:\n", *counter_of_while);
    bypass_of_tree(root->right, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    if (stack_while->size == 0)
    {
        bypass_of_tree(root->node_after_operator, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    while (stack_while->size != 0)
    {
        Stack_Elem_t element = 0;
        error = stack_pop(stack_while, &element);
        if (error != NO_ERRORS)
        {
            fprintf(stderr, "error of stack = %d\n", error);
            abort();
        }
        size_t counter = (size_t)element;
        process_comparison_expression(root->left, file_pointer, all_variables, all_labels, counter_of_if, &counter, OPERATOR_WHILE);
        fprintf(file_pointer, "end_while%d:\n", (int)element);
        size_t index = 0;
        while (strlen(((*all_labels)[index]).name) != 0 && index < SIZE_OF_ALL_VARIABLES)
        {
            index++;
        }
        if (index < SIZE_OF_ALL_VARIABLES)
        {
            char str[50] = "end_while";
            const size_t current_len = strlen(str);
            snprintf(str + current_len, sizeof(str) - current_len, "%d:", (int)element);
            strncpy(((*all_labels)[index]).name, str, strlen(str));
        }
        if (element > 1)
        {
            bypass_of_tree(root->node_after_operator, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
        }
    }
    if (*counter_of_while == 1)
    {
        bypass_of_tree(root->node_after_operator, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    //bypass_of_tree(root->right, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    return;
}

static void process_operator_if(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while)
{
    if (root == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATOR IF\n");
        abort();
    }
    (*counter_of_if)++;
    Errors error = stack_push(stack_if, (double)(*counter_of_if));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    process_comparison_expression(root->left, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, OPERATOR_IF);
    fprintf(file_pointer, "begin_if%lu:\n", *counter_of_if);
    bypass_of_tree(root->right, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    if (stack_if->size == 0)
    {
        bypass_of_tree(root->node_after_operator, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    while (stack_if->size != 0)
    {
        Stack_Elem_t element = 0;
        error = stack_pop(stack_if, &element);
        if (error != NO_ERRORS)
        {
            fprintf(stderr, "error of stack = %d\n", error);
            abort();
        }
        fprintf(file_pointer, "end_if%d:\n", (int)element);
        size_t index = 0;
        while (strlen(((*all_labels)[index]).name) != 0 && index < SIZE_OF_ALL_VARIABLES)
        {
            index++;
        }
        if (index < SIZE_OF_ALL_VARIABLES)
        {
            char str[50] = "end_if";
            const size_t current_len = strlen(str);
            snprintf(str + current_len, sizeof(str) - current_len, "%d:", (int)element);
            strncpy(((*all_labels)[index]).name, str, strlen(str));
        }
        if (element > 1)
        {
            bypass_of_tree(root->node_after_operator, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
        }
    }
    if (*counter_of_if == 1)
    {
        bypass_of_tree(root->node_after_operator, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    //bypass_of_tree(root->node_after_operator, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    return;
}

static void process_operator_assignment(struct Node *root, FILE *file_pointer, struct Label **all_variables)
{
    if (root == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATOR ASSIGNMENT\n");
        abort();
    }
    int ram_address = -1;
    bool is_exits = false;
    int empty_index = 0;
    for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    {
        if (strcasecmp(((root->left)->value).variable_name, ((*all_variables)[index]).name) == 0)
        {
            ram_address = ((*all_variables)[index]).address;
            is_exits = true;
            break;
        }
        if (((*all_variables)[index]).address == -1)
        {
            empty_index = index;
            break;
        }
    }
    process_expression_after_assignment(root->right, file_pointer, all_variables);
    if (!is_exits)
    {
        ((*all_variables)[empty_index]).address = empty_index;
        strncpy(((*all_variables)[empty_index]).name, ((root->left)->value).variable_name, strlen(((root->left)->value).variable_name));
        ram_address = empty_index;
    }
    fprintf(file_pointer, "push [");
    fprintf(file_pointer, "%d]\n", ram_address);
    return;
}

static void process_expression_after_assignment(struct Node *root, FILE *file_pointer, struct Label **all_variables)
{
    if (root == NULL)
    {
        return;
    }
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF CREATING ASM FILE\n");
        abort();
    }

    process_expression_after_assignment((root->left), file_pointer, all_variables);
    process_expression_after_assignment((root->right), file_pointer, all_variables);
    switch ((root->value).type)
    {
        case NUMBER:
        {
            fprintf(file_pointer, "push %f\n", (root->value).number);
            return;
        }
        case VARIABLE:
        {
            process_variable(&(root->value), file_pointer, all_variables);
            return;
        }
        case OPERATION:
        {
            process_operation(&(root->value), file_pointer);
            return;
        }
        default:
        {
            fprintf(stderr, "ERROR OF UNKNOWN TYPE\n");
            abort();
        }
    }
    return;
}

static void process_variable(struct Value *value, FILE *file_pointer, struct Label **all_variables)
{
    if (value == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING VARIABLE\n");
        abort();
    }
    bool is_exists = false;
    size_t address = -1;
    for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    {
        //printf("name of variable = %s\n", ((*all_variables)[index]).name);
        if (strcasecmp(value->variable_name, ((*all_variables)[index]).name) == 0)
        {
            is_exists = true;
            address = ((*all_variables)[index]).address;
            break;
        }
    }

    if (!is_exists)
    {
        fprintf(stderr, "ERROR OF USING UNKNOWN VARIABLE\n");
        abort();
    }
    fprintf(file_pointer, "pop [%d]\n", address);
    return;
}

static void process_operation(struct Value *value, FILE *file_pointer)
{
    if (value == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATION\n");
        abort();
    }
    switch(value->operation)
    {
        case OP_ADD:
        {
            fprintf(file_pointer, "add\n");
            return;
        }
        case OP_SUB:
        {
            fprintf(file_pointer, "sub\n");
            return;
        }
        case OP_MUL:
        {
            fprintf(file_pointer, "mul\n");
            return;
        }
        case OP_DIV:
        {
            fprintf(file_pointer, "div\n");
            return;
        }
        default: return;
    }
    return;
}

static void process_comparison_operation(struct Value *value, FILE *file_pointer)
{
    if (value == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING COMPARISON OPERATION\n");
        abort();
    }
    switch(value->comp_operation)
    {
        case OP_MORE:
        {
            fprintf(file_pointer, "ja ");
            return;
        }
        case OP_MORE_OR_EQUAL:
        {
            fprintf(file_pointer, "jae ");
            return;
        }
        case OP_LESS:
        {
            fprintf(file_pointer, "jb ");
            return;
        }
        case OP_LESS_OR_EQUAL:
        {
            fprintf(file_pointer, "jbe ");
            return;
        }
        case OP_EQUAL:
        {
            fprintf(file_pointer, "je ");
            return;
        }
        case OP_NOT_EQUAL:
        {
            fprintf(file_pointer, "jne ");
            return;
        }
        default: return;
    }
}

static void process_comparison_expression(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, Programm_operators operator_)
{
    if (root == NULL)
    {
        return;
    }
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF CREATING ASM FILE\n");
        abort();
    }
    process_expression_after_comparison(root->right, file_pointer, all_variables, all_labels);
    fprintf(file_pointer, "push ax\n");
    process_expression_after_comparison(root->left, file_pointer, all_variables, all_labels);
    process_comparison_operation(&(root->value), file_pointer);
    fprintf(file_pointer, "ax ");
    char str_begin[50] = "";
    char str_end[50] = "";
    size_t counter = 0;
    if (operator_ == OPERATOR_IF)
    {
        strncpy(str_begin, "begin_if", 8);
        strncpy(str_end, "end_if", 6);
        counter = *counter_of_if;
    }
    else if (operator_ == OPERATOR_WHILE)
    {
        strncpy(str_begin, "begin_while", 11);
        strncpy(str_end, "end_while", 9);
        counter = *counter_of_while;
    }
    fprintf(file_pointer, "%s%lu:\n", str_begin, counter);
    size_t index = 0;
    const size_t current_len = strlen(str_begin);
    snprintf(str_begin + current_len, sizeof(str_begin) - current_len, "%lu:", counter);
    while (strlen(((*all_labels)[index]).name) != 0 && index < SIZE_OF_ALL_VARIABLES)
    {
        if (strcasecmp(str_begin, ((*all_labels)[index]).name) == 0)
        {
            index = SIZE_OF_ALL_VARIABLES;
        }
        index++;
    }
    if (index < SIZE_OF_ALL_VARIABLES)
    {
        strncpy(((*all_labels)[index]).name, str_begin, strlen(str_begin));
        index++;
    }
    fprintf(file_pointer, "jmp %s%lu:\n", str_end, counter);

    return;
}


static void process_expression_after_comparison(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels)
{
    if (root == NULL)
    {
        return;
    }
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF CREATING ASM FILE\n");
        abort();
    }
    process_expression_after_comparison(root->left, file_pointer, all_variables, all_labels);
    process_expression_after_comparison(root->right, file_pointer, all_variables, all_labels);
    switch ((root->value).type)
    {
        case NUMBER:
        {
            fprintf(file_pointer, "push %f\n", (root->value).number);
            return;
        }
        case VARIABLE:
        {
            process_variable(&(root->value), file_pointer, all_variables);
            return;
        }
        case OPERATION:
        {
            process_operation(&(root->value), file_pointer);
            return;
        }
        default:
        {
            fprintf(stderr, "ERROR OF UNKNOWN TYPE\n");
            abort();
        }
    }
    return;

}