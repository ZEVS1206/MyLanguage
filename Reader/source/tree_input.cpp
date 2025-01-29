#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tree.h"
#include "tree_input.h"
#include "tree_tex_dump.h"

static void parse_information_from_file_by_staples(struct Node *root, char **buffer, char *end_pointer);
static void parse_information_from_file_by_recursive_descent(struct Node **root, char **buffer, char *end_pointer, int *index);
static void get_number_from_file(struct Node **root, char **buffer, char *end_pointer, int *index);
static void get_variable_from_file(struct Node **root, char **buffer, char *end_pointer, int *index);
static void get_function_from_file(struct Node **root, char **buffer, char *end_pointer, int *index);
static void get_variable_or_function_from_file(struct Node **root, char **buffer, char *end_pointer, int *index);
static void get_staples_expression_or_number_or_variable_or_function(struct Node **root, char **buffer, char *end_pointer, int *index);
static void get_expression_with_plus_or_minus(struct Node **root, char **buffer, char *end_pointer, int *index, struct Node *parent);
static void get_expression_with_mul_or_div(struct Node **root, char **buffer, char *end_pointer, int *index, struct Node *parent);
static void get_expression_with_pow(struct Node **root, char **buffer, char *end_pointer, int *index, struct Node *parent);
static void syntax_error(char **buffer, int *index);
static size_t get_size_of_file(FILE *file_pointer);
static char * get_value_from_file(char *str, size_t size_of_str, char *buffer, char *end_pointer);
static char * skip_spaces(char *buffer, char *end_pointer);
static void transform_to_variable(const char *str, struct Value *value);
static int  transform_to_function(const char *str, struct Value *value);
static void bypass_of_tree(struct Node *root);


static void bypass_of_tree(struct Node *root)
{
    if (root == NULL)
    {
        return;
    }
    bypass_of_tree(root->left);
    bypass_of_tree(root->right);
    //printf("root->value.type = %d\n", (root->value).type);
    if (root->left != NULL)
    {
        (root->left)->parent_node = root;
    }
    if (root->right != NULL)
    {
        (root->right)->parent_node = root;
    }
    return;
}

static char * skip_spaces(char *buffer, char *end_pointer)
{
    if (buffer == end_pointer)
    {
        return buffer;
    }
    while (buffer < end_pointer && isspace(*buffer))
    {
        buffer++;
    }
    return buffer;
}


static void transform_to_variable(const char *str, struct Value *value)
{
    if (value == NULL || str == NULL)
    {
        return;
    }

    if (strcasecmp("x", str) == 0)
    {
        value->variable = VAR_X;
        value->type     = VARIABLE;
    }
    else
    {
        value->variable = NOT_A_VAR;
    }
    return;
}

static int  transform_to_function(const char *str, struct Value *value)
{
    if (str == NULL || value == NULL)
    {
        return 0;
    }
    for (size_t index = 0; index < size_of_functions; index++)
    {
        if (strcasecmp(str, G_functions[index].function_str_name) == 0)
        {
            value->function = G_functions[index].function_name;
            value->type     = FUNCTION;
            break;
        }
    }
    if (value->type == FUNCTION)
    {
        return 1;
    }
    return 0;
}

void transform_to_operation(const char symbol, struct Value *value)
{
    if (value == NULL)
    {
        return;
    }
    switch (symbol)
    {
        case '+':value->operation = OP_ADD; break;
        case '-':value->operation = OP_SUB; break;
        case '*':value->operation = OP_MUL; break;
        case '/':value->operation = OP_DIV; break;
        case '^':value->operation = OP_DEG; break;
        default: value->operation = NOT_AN_OP; break;
    }
    return;
}

static size_t get_size_of_file(FILE *file_pointer)
{
    fseek(file_pointer, 0L, SEEK_END);
    size_t size_of_file = (size_t)ftell(file_pointer);
    fseek(file_pointer, 0L, SEEK_SET);
    return size_of_file;
}

Errors_of_tree get_tree_from_file(struct Tree *tree, const char *file_source_name)
{
    if (tree == NULL)
    {
        return ERROR_OF_READ_TREE;
    }

    FILE *file_pointer = fopen(file_source_name, "rb");
    if (file_pointer == NULL)
    {
        return ERROR_OF_READ_TREE;
    }

    size_t size_of_file = get_size_of_file(file_pointer);
    char *buffer = (char *)calloc(size_of_file, sizeof(char));
    if (buffer == NULL)
    {
        return ERROR_OF_READ_TREE;
    }

    char *end_pointer = &(buffer[size_of_file - 1]);
    char *old_buffer = &(buffer[0]);

    size_t result_of_reading = fread(buffer, sizeof(char), size_of_file, file_pointer);
    if (result_of_reading != size_of_file)
    {
        return ERROR_OF_READ_FROM_FILE;
    }
    fclose(file_pointer);

    tree->tmp_root = tree->root;
    if (buffer[0] == '$')
    {
        buffer++;
        parse_information_from_file_by_staples(tree->root, &buffer, end_pointer);
    }
    else
    {
        int index = 0;
        parse_information_from_file_by_recursive_descent(&tree->root, &buffer, end_pointer, &index);
        //printf("index = %d\n", index);
        bypass_of_tree(tree->root);
        //printf("tree->root = %p\n", tree->root);
    }

    free(old_buffer);
    return NO_ERRORS;
}

static char * get_value_from_file(char *str, size_t size_of_str, char *buffer, char *end_pointer)
{
    size_t index = 0;
    while (buffer < end_pointer && (isalnum(*buffer) || *buffer == '.') && index < size_of_str)
    {
        str[index] = *buffer;
        index++;
        buffer++;
    }
    if (index < size_of_str - 1)
    {
        str[index] = '\0';
    }
    return buffer;
}

static void parse_information_from_file_by_staples(struct Node *root, char **buffer, char *end_pointer)
{
    int verdict = 0;
    *buffer = skip_spaces(*buffer, end_pointer);
    if (*buffer >= end_pointer)
    {
        return;
    }

    if (*buffer[0] == '(')
    {
        (*buffer)++;
        root->left = (Node *) calloc(1, sizeof(Node));
        if (root->left == NULL)
        {
            printf("Error of creating node!\n");
            return;
        }
        (root->left)->parent_node = root;
        ON_DEBUG(printf("go to left\n");)
        ON_DEBUG(getchar();)
        parse_information_from_file_by_staples(root->left, buffer, end_pointer);
        ON_DEBUG(printf("leave left\n");)
        if (*buffer >= end_pointer)
        {
            return;
        }
        if (*buffer[0] == '\n')
        {
            return;
        }
        *buffer = skip_spaces(*buffer, end_pointer);
        ON_DEBUG(getchar();)
        ON_DEBUG(printf("processing center\n");)
        if (!isalnum(*buffer[0]) && *buffer[0] != ')' && *buffer[0] != '(')
        {
            transform_to_operation(*buffer[0], &(root->value));
            ON_DEBUG(printf("operation = %c\n", *buffer[0]);)
            (root->value).type = OPERATION;
            (*buffer)++;
        }
        else if (isalpha(*buffer[0]))
        {
            char str[100];
            *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
            verdict = transform_to_function(str, &(root->value));
            (*buffer)++;
        }

        *buffer = skip_spaces(*buffer, end_pointer);
        (*buffer)++;
        if (verdict == 0)
        {
            root->right = (Node *) calloc(1, sizeof(Node));
            if (root->right == NULL)
            {
                printf("Error of creating node!\n");
                return;
            }
            (root->right)->parent_node = root;
            ON_DEBUG(printf("go to right\n");)
            ON_DEBUG(getchar();)
            parse_information_from_file_by_staples(root->right, buffer, end_pointer);
            ON_DEBUG(printf("leave right\n");)
            ON_DEBUG(getchar();)
        }
        if (*buffer >= end_pointer)
        {
            return;
        }
        if (*buffer[0] == '\n')
        {
            return;
        }
    }

    if (isdigit(*buffer[0]))
    {
        char str[100];
        *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
        ON_DEBUG(printf("value = %s\n", str);)
        char *end = NULL;
        (root->value).number = strtod(str, &end);
        //(root->value).number = atoi(str);
        (root->value).type = NUMBER;
    }
    else if (isalpha(*buffer[0]))
    {
        char str[100];
        *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
        ON_DEBUG(printf("variable = %s\n", str);)
        //int verdict = transform_to_function(str, &(root->value));
        transform_to_variable(str, &(root->value));
    }

    if (*buffer[0] == ')')
    {
        (*buffer)++;
        return;
    }
}

static void syntax_error(char **buffer, int *index)
{
    printf("Syntax Error! At index %d wait %c\n", *index, *buffer[0]);
    abort();
}

static void get_number_from_file(struct Node **root, char **buffer, char *end_pointer, int *index)
{
    if (*buffer == end_pointer)
    {
        return;
    }
    char *old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    int old_index = *index;
    char str[100] = {0};
    old = *buffer;
    *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
    *index += *buffer - old;
    if (old_index == *index)
    {
        syntax_error(buffer, index);
    }
    ON_DEBUG(printf("index in get_number_from_file = %d\n", *index);)
    char *end = NULL;
    double value = strtod(str, &end);
    struct Value new_node_value = {.type = NUMBER, .number = value};
    Errors_of_tree error = create_new_node(root, &new_node_value, NULL, NULL);
    //printf("root->value = %d\n", ((*root)->value).number);
    //printf("root = %p\n", (*root));
    return;
}

static void get_variable_from_file(struct Node **root, char **buffer, char *end_pointer, int *index)
{
    if (*buffer == end_pointer)
    {
        return;
    }
    char *old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    int old_index = *index;
    char str[100] = {0};
    old = *buffer;
    *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
    *index += *buffer - old;
    if (old_index == *index)
    {
        syntax_error(buffer, index);
    }
    ON_DEBUG(printf("index in get_variable_from_file = %d\n", *index);)
    struct Value new_node_value = {.type = VARIABLE};
    transform_to_variable(str, &new_node_value);
    Errors_of_tree error = create_new_node(root, &new_node_value, NULL, NULL);
    return;
}

static void get_function_from_file(struct Node **root, char **buffer, char *end_pointer, int *index)
{
    if (*buffer == end_pointer)
    {
        return;
    }

    char *old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    int old_index = *index;
    char str[100] = {0};
    old = *buffer;
    *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
    *index += *buffer - old;
    old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    struct Node *left_node = NULL;
    if (old_index == *index)
    {
        syntax_error(buffer, index);
    }
    ON_DEBUG(printf("index in get_function_from_file = %d\n", *index);)
    struct Value new_node_value = {};
    int verdict = transform_to_function(str, &new_node_value);
    if (*buffer[0] != '(')
    {
        syntax_error(buffer, index);
    }
    (*buffer)++;
    while (*buffer[0] != ')')
    {
        // if (*buffer[0] == '$')
        // {
        //     break;
        // }
        //printf("Here!\n");
        // printf("*buffer[0] before = %c\n", *buffer[0]);
        // while (getchar() != '\n');
        // getchar();
        ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable_or_function\n");)
        ON_DEBUG(getchar();)
        get_staples_expression_or_number_or_variable_or_function(&left_node, buffer, end_pointer, index);
        // printf("*buffer[0] after = %d\n", *buffer[0]);
        // getchar();
        // old = *buffer;
        // *buffer = skip_spaces(*buffer, end_pointer);
        // *index += *buffer - old;
    }
    (*buffer)++;
    Errors_of_tree error = create_new_node(root, &new_node_value, left_node, NULL);
    return;
}


static void get_variable_or_function_from_file(struct Node **root, char **buffer, char *end_pointer, int *index)
{
    if (*buffer == end_pointer)
    {
        return;
    }
    char *old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    //*index += *buffer - old;
    char str[100] = {0};
    old = *buffer;
    *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
    struct Value value = {};
    int verdict = transform_to_function(str, &value);
    ON_DEBUG(printf("index in get_variable_or_function_from_file = %d\n", *index);)
    if (verdict == 0)
    {
        ON_DEBUG(printf("go to get_variable_from_file\n");)
        ON_DEBUG(getchar();)
        *buffer = old;
        get_variable_from_file(root, buffer, end_pointer, index);
        return;
    }
    else
    {
        ON_DEBUG(printf("go to get_function_from_file\n");)
        ON_DEBUG(getchar();)
        *buffer = old;
        get_function_from_file(root, buffer, end_pointer, index);
        return;
    }
}


static void get_staples_expression_or_number_or_variable_or_function(struct Node **root, char **buffer, char *end_pointer, int *index)
{
    if (*buffer == end_pointer)
    {
        return;
    }
    char *old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    if (*buffer[0] == '(')
    {
        (*index)++;
        (*buffer)++;
        ON_DEBUG(printf("go to get_expression_with_plus_or_minus\n");)
        ON_DEBUG(getchar();)
        get_expression_with_plus_or_minus(root, buffer, end_pointer, index, NULL);
        if (*buffer[0] != ')')
        {
            syntax_error(buffer, index);
        }
        (*index)++;
        (*buffer)++;
        return;
    }
    else if (isalpha(*buffer[0]))
    {
        ON_DEBUG(printf("go to get_variable_of_function_from_file\n");)
        ON_DEBUG(getchar();)
        get_variable_or_function_from_file(root, buffer, end_pointer, index);
        return;
    }
    else if (isdigit(*buffer[0]))
    {
        //printf("digit\n");
        //printf("*buffer[0] = %d\n", *buffer[0] - '0');
        ON_DEBUG(printf("go to get_number_from_file\n");)
        ON_DEBUG(getchar();)
        get_number_from_file(root, buffer, end_pointer, index);
        //printf("*buffer[0] = %c\n", *buffer[0]);
        //printf("root in staples = %p\n", (*root));
        return;
    }
    ON_DEBUG(printf("index in get_staples_expression_or_number_or_variable_or_function = %d\n", *index);)
    return;
}
static void get_expression_with_pow(struct Node **root, char **buffer, char *end_pointer, int *index, struct Node *parent)
{
    if (*buffer == end_pointer)
    {
        return;
    }
    char *old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    //printf("%c\n", *buffer[0]);
    *index += *buffer - old;
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable_or_function\n");)
    ON_DEBUG(getchar();)
    get_staples_expression_or_number_or_variable_or_function(&left_node, buffer, end_pointer, index);
    //ON_DEBUG(printf("left_node in pow = %f\n", (left_node->value).number);)
    //printf("left_node = %d\n", (left_node->value).number);
    old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    while (*buffer[0] == '^')
    {
        //printf("Here\n");
        char operation = *buffer[0];
        (*index)++;
        (*buffer)++;
        //printf("%c\n", *buffer[0]);
        ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable_or_function\n");)
        ON_DEBUG(getchar();)
        get_staples_expression_or_number_or_variable_or_function(&right_node, buffer, end_pointer, index);
        struct Value new_node_value = {.type = OPERATION};
        transform_to_operation(operation, &new_node_value);
        Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
        (*root)->parent_node = parent;
        left_node = copy_node(*root, parent);
        //printf("root->type = %d\n", ((*root)->value).type);
    }
    if (right_node == NULL)
    {
        //ON_DEBUG(printf("left_node in pow if right_node null = %p\n", left_node);)
        // if (*root != NULL)
        // {
        //     free(*root);
        //     *root = NULL;
        // }
        *root = left_node;
        //(*root)->parent_node = parent;
        //*root = copy_node(left_node, left_node->parent_node);
    }
    //ON_DEBUG(printf("Here is root has value = %f\n", ((*root)->value).number);)
    ON_DEBUG(printf("index in get_expression_with_pow = %d\n", *index);)
    return;
}

static void get_expression_with_mul_or_div(struct Node **root, char **buffer, char *end_pointer, int *index, struct Node *parent)
{
    if (*buffer == end_pointer)
    {
        return;
    }
    char *old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    //printf("%c\n", *buffer[0]);
    *index += *buffer - old;
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_expression_with_pow\n");)
    ON_DEBUG(getchar();)
    get_expression_with_pow(&left_node, buffer, end_pointer, index, *root);
    //ON_DEBUG(printf("left_node in mul or div = %p\n", left_node);)
    //printf("left_node = %d\n", (left_node->value).number);
    old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    while (*buffer[0] == '*' || *buffer[0] == '/')
    {
        //printf("root address in mul or div = %p\n", *root);
        //printf("left_node parent address in mul or div = %p\n", left_node->parent_node);
        ON_DEBUG(printf("get * or / operation\n");)
        ON_DEBUG(getchar();)
        //printf("Here\n");
        char operation = *buffer[0];
        (*index)++;
        (*buffer)++;
        //printf("%c\n", *buffer[0]);
        ON_DEBUG(printf("go to get_expression_with_pow\n");)
        ON_DEBUG(getchar();)
        get_expression_with_pow(&right_node, buffer, end_pointer, index, *root);
        //printf("right_node parent address in mul or div = %p\n", right_node->parent_node);
        struct Value new_node_value = {.type = OPERATION};
        transform_to_operation(operation, &new_node_value);
        Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
        //printf("root address = %p\n", *root);
        left_node = copy_node(*root, parent);
        //printf("root->type = %d\n", ((*root)->value).type);
    }
    if (right_node == NULL)
    {
        //ON_DEBUG(printf("left_node in mul or div if right_node null = %p\n", left_node);)
        // if (*root != NULL)
        // {
        //     free(*root);
        //     *root = NULL;
        // }
        *root = left_node;
        //(*root)->parent_node = parent;
        //*root = copy_node(left_node, NULL);
    }
    ON_DEBUG(printf("index in get_expression_with_mul_or_div = %d\n", *index);)
    return;
}

static void get_expression_with_plus_or_minus(struct Node **root, char **buffer, char *end_pointer, int *index, struct Node *parent)
{
    if (*buffer == end_pointer)
    {
        return;
    }
    //printf("root = %p\n", *root);
    char *old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_expression_with_mul_or_div\n");)
    ON_DEBUG(getchar();)
    get_expression_with_mul_or_div(&left_node, buffer, end_pointer, index, *root);
    //ON_DEBUG(printf("left_node in plus or minus = %f\n", (left_node->value).number);)
    old = *buffer;
    *buffer = skip_spaces(*buffer, end_pointer);
    *index += *buffer - old;
    while (*buffer[0] == '+' || *buffer[0] == '-')
    {
        ON_DEBUG(printf("get + or - operation\n");)
        ON_DEBUG(getchar();)
        //printf("Here\n");
        //printf("left_node address = %p\n", left_node);
        char operation = *buffer[0];
        //ON_DEBUG(printf("operation = %c\n", operation);)
        (*index)++;
        (*buffer)++;
        ON_DEBUG(printf("go to get_expression_with_mul_or_div\n");)
        ON_DEBUG(getchar();)
        get_expression_with_mul_or_div(&right_node, buffer, end_pointer, index, *root);
        //ON_DEBUG(printf("buffer[0] = %c\n", *buffer[0]);)
        //ON_DEBUG(printf("right_node address = %p\n", right_node);)
        struct Value new_node_value = {.type = OPERATION};
        transform_to_operation(operation, &new_node_value);
        Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
        left_node = copy_node(*root, parent);
        ON_DEBUG(printf("root address = %p\n", *root);)
    }
    if (right_node == NULL)
    {
        //ON_DEBUG(printf("left_node in plus or minus if right_node null = %p\n", left_node);)
        // if (*root != NULL)
        // {
        //     free(*root);
        //     *root = NULL;
        // }
        *root = left_node;
        //(*root)->parent_node = parent;
        //*root = copy_node(left_node, NULL);
    }
    ON_DEBUG(printf("index in get_expression_with_plus_or_minus = %d\n", *index);)
    return;
}


static void parse_information_from_file_by_recursive_descent(struct Node **root, char **buffer, char *end_pointer, int *index)
{
    if (*root == NULL || *buffer == end_pointer)
    {
        return;
    }
    ON_DEBUG(printf("go to get_expression_with_plus_or_minus\n");)
    ON_DEBUG(getchar();)
    get_expression_with_plus_or_minus(root, buffer, end_pointer, index, NULL);
    //printf("general root = %p\n", *root);
    if (*buffer[0] != '$')
    {
        syntax_error(buffer, index);
    }
    return;
}
