#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#include "../../Processor/include/processor.h"
#include "assembler.h"
#include "../../Reader/include/tree.h"
// #include "../../Reader/include/tree_input.h"
// #include "../../Reader/include/tree_dump.h"


//static const int num_of_labels = 50;

static Errors_of_ASM constructor(struct ASM *Asm, size_t size_of_all_labels);
static Errors_of_ASM destructor(struct ASM *Asm);
static Errors_of_ASM get_count_of_rows(struct ASM *Asm);
static Errors_of_ASM parse_word(struct ASM *Asm, int *step, size_t len, size_t index, struct Labels *all_labels, size_t size_of_all_labels);
static Errors_of_ASM parse_label(struct ASM *Asm, int *step, size_t index, struct Labels *all_labels, size_t size_of_all_labels);
static Errors_of_ASM parse_push_pop_cmd(struct ASM *Asm, size_t index);
static Errors_of_ASM new_parse_jump_cmds(struct ASM *Asm, size_t index);
static Errors_of_ASM parse_jump_cmds(struct ASM *Asm, size_t index, char *command);
static Errors_of_ASM repeat_get_commands(struct ASM *Asm);
static Errors_of_ASM new_parse_word(struct ASM *Asm, size_t index);


static Errors_of_ASM new_parse_jump_cmds(struct ASM *Asm, size_t index)
{
    if (Asm == NULL)
    {
        return ERROR_OF_PARSE_WORD;
    }
    for (size_t k = 0; k < (Asm->table)->size_of_labels; k++)
    {
        if (strcasecmp((Asm->commands)[index].label_name, ((Asm->table)->labels)[k].name) == 0)
        {
            //printf("label = %s\n", ((Asm->table)->labels)[k].name);
            (Asm->commands)[index].element = (double)((Asm->table)->labels)[k].address;
            //printf("address = %lf\n", (Asm->commands)[index].element);
            break;
        }
    }
    return NO_ASM_ERRORS;
}

static Errors_of_ASM repeat_get_commands(struct ASM *Asm)
{
    if (Asm == NULL)
    {
        return ERROR_OF_READING_FROM_FILE;
    }
    size_t i = 0;
    Errors_of_ASM error = NO_ASM_ERRORS;
    // for (size_t j = 0; j < (Asm->count_of_rows); j++)
    // {
    //     printf("j = %d command = %s\n", j, (Asm->commands)[j].command);
    // }
    while (i < (Asm->count_of_rows))
    {
        error = new_parse_word(Asm, i);
        i++;
    }
    return error;
}

static Errors_of_ASM new_parse_word(struct ASM *Asm, size_t index)
{
    Errors_of_ASM error = NO_ASM_ERRORS;
    if (Asm == NULL)
    {
        return ERROR_OF_PARSE_WORD;
    }
    if (strcasecmp((Asm->commands)[index].command, "jmp")    == 0
            || strcasecmp((Asm->commands)[index].command, "ja")   == 0
            || strcasecmp((Asm->commands)[index].command, "jae")  == 0
            || strcasecmp((Asm->commands)[index].command, "jb")   == 0
            || strcasecmp((Asm->commands)[index].command, "jbe")  == 0
            || strcasecmp((Asm->commands)[index].command, "je")   == 0
            || strcasecmp((Asm->commands)[index].command, "jne")  == 0)
    {
        if ((Asm->commands)[index].element == -1)
        {
            error = new_parse_jump_cmds(Asm, index);
        }
        if (error != NO_ASM_ERRORS)
        {
            return error;
        }
    }
    return error;
}

static Errors_of_ASM parse_word(struct ASM *Asm, int *step, size_t len, size_t index, struct Labels *all_labels, size_t size_of_all_labels)
{
    Errors_of_ASM error = NO_ASM_ERRORS;
    if (Asm == NULL)
    {
        return ERROR_OF_PARSE_WORD;
    }
    if ((Asm->commands)[index].command[len - 1] == ':')
    {
        //printf("label_name = %s\n", (Asm->commands)[index].command);
        error = parse_label(Asm, step, index, all_labels, size_of_all_labels);

        // printf("\n\n");
        // for (size_t i = 0; i < (Asm->table)->size_of_labels; i++)
        // {
        //     printf("label = %s address = %d\n", ((Asm->table)->labels)[i].name, ((Asm->table)->labels)[i].address);
        // }
        //*step += 1;
        if (error != NO_ASM_ERRORS)
        {
            return error;
        }
    }
    else if (strcasecmp((Asm->commands)[index].command, "push") == 0
        || strcasecmp((Asm->commands)[index].command, "pop")  == 0)
    {
        error = parse_push_pop_cmd(Asm, index);
        *step += 4;
        if (error != NO_ASM_ERRORS)
        {
            return error;
        }
    }
    else if (strcasecmp((Asm->commands)[index].command,   "jmp")  == 0
            || strcasecmp((Asm->commands)[index].command, "ja")   == 0
            || strcasecmp((Asm->commands)[index].command, "jae")  == 0
            || strcasecmp((Asm->commands)[index].command, "jb")   == 0
            || strcasecmp((Asm->commands)[index].command, "jbe")  == 0
            || strcasecmp((Asm->commands)[index].command, "je")   == 0
            || strcasecmp((Asm->commands)[index].command, "jne")  == 0)
    {
        error = parse_jump_cmds(Asm, index, (Asm->commands)[index].command);
        //printf("command = %s register = %d label_name = %s\n", (Asm->commands)[index].command, (Asm->commands)[index].reg, (Asm->commands)[index].label_name);
        *step += 3;
        if (error != NO_ASM_ERRORS)
        {
            return error;
        }
    }
    else
    {
        //printf("command = %s\n", (Asm->commands)[index].command);
        *step += 1;
    }
    return error;
}

static Errors_of_ASM parse_label(struct ASM *Asm, int *step, size_t index, struct Labels *all_labels, size_t size_of_all_labels)
{
    if (Asm == NULL)
    {
        return ERROR_OF_PARSE_WORD;
    }
    //printf("label_name = %s\n", (Asm->commands)[index].command);
    int fl = 0;
    //printf("\n\n");
    for (size_t j = 0; j < (Asm->table)->size_of_labels; j++)
    {
        //printf("name = %s\n", ((Asm->table)->labels)[j].name);
        if (strcasecmp(((Asm->table)->labels)[j].name, "") == 0)
        {
            // size_t k = 0;
            // if (j != 0)
            // {
            //     k = j - 1;
            // }
            // printf("LABEL = %s\n", ((Asm->table)->labels)[k].name);
            for (size_t k = 0; k < size_of_all_labels; k++)
            {
                //printf("label_name = %s\n", (Asm->commands)[index].command);
                if (strcasecmp((Asm->commands)[index].command, (all_labels[k]).name) == 0)
                {
                    memcpy(((Asm->table)->labels)[j].name, (all_labels[k]).name, strlen((all_labels[k]).name));
                    if (*step != 0)
                    {
                        ((Asm->table)->labels)[j].address = *step - 1;
                    }
                    else
                    {
                        ((Asm->table)->labels)[j].address = 0;
                    }
                    //printf("label_name = %s\naddress = %d\n", ((Asm->table)->labels)[j].name, ((Asm->table)->labels)[j].address);
                    fl = 1;
                    break;
                }
            }
        }
        if (fl)
        {
            break;
        }
    }
    char str[6] = "label";
    memcpy((Asm->commands)[index].command, str, 6);
    return NO_ASM_ERRORS;
}

static Errors_of_ASM parse_push_pop_cmd(struct ASM *Asm, size_t index)
{
    if (Asm == NULL || (Asm->file_pointer) == NULL)
    {
        return ERROR_OF_PARSE_WORD;
    }
    char s[50] = {0};
    fscanf(Asm->file_pointer, "%s", s);
    if (isalpha(s[0]))
    {
        (Asm->commands)[index].element = TOXIC;
        if (strcasecmp(s, "ax") == 0)
        {
            (Asm->commands)[index].reg = AX;
        }
        else if (strcasecmp(s, "bx") == 0)
        {
            (Asm->commands)[index].reg = BX;
        }
        else if (strcasecmp(s, "cx") == 0)
        {
            (Asm->commands)[index].reg = CX;
        }
        else if (strcasecmp(s, "dx") == 0)
        {
            (Asm->commands)[index].reg = DX;
        }
        else
        {
            return ERROR_OF_UNKNOWN_REGISTER;
        }
    }
    else if (s[0] == '[')
    {
        (Asm->commands)[index].element = TOXIC;
        (Asm->commands)[index].reg = NOT_A_REGISTER;
        char address[50] = {0};
        memcpy(address, s + 1, strlen(s));
        (Asm->commands)[index].ram_address = atoi(address);
    }
    else
    {
        char *end = NULL;
        (Asm->commands)[index].element = strtod(s, &end);
        //(Asm->commands)[index].element = atoi(s);
        (Asm->commands)[index].reg = NOT_A_REGISTER;
    }
    return NO_ASM_ERRORS;
}

static Errors_of_ASM parse_jump_cmds(struct ASM *Asm, size_t index, char *command)
{
    if (Asm == NULL || (Asm->file_pointer) == NULL)
    {
        return ERROR_OF_PARSE_WORD;
    }
    char str[50] = {0};
    if (strcasecmp(command, "jmp") != 0)
    {
        fscanf(Asm->file_pointer, "%s", str);
        if (strcasecmp(str, "ax") == 0)
        {
            (Asm->commands)[index].reg = AX;
        }
        else if (strcasecmp(str, "bx") == 0)
        {
            (Asm->commands)[index].reg = BX;
        }
        else if (strcasecmp(str, "cx") == 0)
        {
            (Asm->commands)[index].reg = CX;
        }
        else if (strcasecmp(str, "dx") == 0)
        {
            (Asm->commands)[index].reg = DX;
        }
        else
        {
            return ERROR_OF_UNKNOWN_REGISTER;
        }
    }
    fscanf(Asm->file_pointer, "%s", str);
    //printf("str = %s\n", str);
    //printf("\n\n");
    for (size_t k = 0; k < (Asm->table)->size_of_labels; k++)
    {
        //printf("label name = %s\n", (((Asm->table)->labels)[k]).name);
        if (strcasecmp(str, (((Asm->table)->labels)[k]).name) == 0)
        {
            //printf("label_name in parse jump cmds = %s\n", ((Asm->table)->labels)[k].name);
            (Asm->commands)[index].element = (double)((Asm->table)->labels)[k].address;
            break;
        }
    }
    memcpy((Asm->commands)[index].label_name, str, strlen(str));
    return NO_ASM_ERRORS;
}

Errors_of_ASM get_commands(struct ASM *Asm, struct Labels *all_labels, size_t size_of_all_labels)
{
    if (Asm == NULL)
    {
        return ERROR_OF_READING_FROM_FILE;
    }
    size_t i = 0;
    int step = 0;
    Errors_of_ASM error = NO_ASM_ERRORS;
    while (i < (Asm->count_of_rows) && fscanf(Asm->file_pointer, "%s", (Asm->commands)[i].command))
    {
        size_t len = strlen((Asm->commands)[i].command);
        error = parse_word(Asm, &step, len, i, all_labels, size_of_all_labels);
        if (error != NO_ASM_ERRORS)
        {
            return error;
        }
        i++;
    }
    return error;
}


Errors_of_ASM transform_commands(struct ASM *Asm, struct CMD *all_commands, size_t size_of_all_commands)
{
    if (Asm == NULL || all_commands == NULL)
    {
        return ERROR_OF_NO_COMMANDS;
    }
    for (size_t i = 0; i < (Asm->count_of_rows); i++)
    {
        bool flag = false;
        for (size_t j = 0; j < size_of_all_commands; j++)
        {
            if (strcasecmp((Asm->commands)[i].command, (all_commands[j]).name) == 0)
            {
                (Asm->commands)[i].transformed_command = (all_commands[j]).transformed_name;
                flag = true;
                break;
            }
        }
        if (!flag)
        {
            printf("command = %s\n", (Asm->commands)[i].command);
            (Asm->commands)[i].transformed_command = CMD_UNKNOWN;
        }
    }
    return NO_ASM_ERRORS;
}

Errors_of_ASM create_file_with_commands(struct ASM *Asm)
{
    FILE *fp = fopen("Assembler/source/cpu_commands.txt", "w");
    if (Asm == NULL || fp == NULL)
    {
        return ERROR_OF_CREATING_OUT_FILE;
    }
    fprintf(fp, "ZEVS_GROM\n");
    fprintf(fp, "Vesrion %d\n", VERSION);
    time_t my_time = time(NULL);
    struct tm *now = localtime(&my_time);
    char current_time[20] = {};
    char date[20] = {};
    char day[20] = {};
    char month[20] = {};
    strftime(month, sizeof(month), "%B", now);
    strftime(day, sizeof(day), "%A", now);
    strftime(current_time, sizeof(current_time), "%T", now);
    strftime(date, sizeof(date), "%D", now);
    fprintf(fp, "Date: %s\n", date);
    fprintf(fp, "Day: %s\n", day);
    fprintf(fp, "Month: %s\n", month);
    fprintf(fp, "Time: %s\n", current_time);
    fprintf(fp, "________________________________________________________________\n");
    for (size_t i = 0; i < (Asm->count_of_rows); i++)
    {
        if ((Asm->commands)[i].transformed_command == CMD_LABEL)
        {
            continue;
        }

        if ((Asm->commands)[i].transformed_command == CMD_PUSH
         || (Asm->commands)[i].transformed_command == CMD_POP)
        {
            fprintf(fp, "%d %.*lf %d %d\n", (Asm->commands)[i].transformed_command, ACCURANCY, (Asm->commands)[i].element, (Asm->commands)[i].reg, (Asm->commands)[i].ram_address);
        }
        else if ((Asm->commands)[i].transformed_command == CMD_JMP
              || (Asm->commands)[i].transformed_command == CMD_JA
              || (Asm->commands)[i].transformed_command == CMD_JAE
              || (Asm->commands)[i].transformed_command == CMD_JB
              || (Asm->commands)[i].transformed_command == CMD_JBE
              || (Asm->commands)[i].transformed_command == CMD_JE
              || (Asm->commands)[i].transformed_command == CMD_JNE)
        {
            fprintf(fp, "%d %d %d\n", (Asm->commands)[i].transformed_command, (Asm->commands)[i].reg, (int)(Asm->commands)[i].element);
        }
        else
        {
            fprintf(fp, "%d\n", (Asm->commands)[i].transformed_command);
        }

    }
    return NO_ASM_ERRORS;
}


static Errors_of_ASM constructor(struct ASM *Asm, size_t size_of_all_labels)
{
    if (Asm == NULL)
    {
        return ERROR_OF_CONSTRUCTOR_ASM;
    }
    Asm->commands = (Command *) calloc(Asm->count_of_rows, sizeof(Command));
    if (Asm->commands == NULL)
    {
        return ERROR_OF_CONSTRUCTOR_ASM;
    }
    for (size_t i = 0; i < (Asm->count_of_rows); i++)
    {
        (Asm->commands)[i].ram_address = -1;
        (Asm->commands)[i].element = -1;
    }
    Asm->table = (Table_labels *) calloc(1, sizeof(Table_labels));
    if (Asm->table == NULL)
    {
        return ERROR_OF_CONSTRUCTOR_ASM;
    }
    (Asm->table)->size_of_labels = size_of_all_labels;
    (Asm->table)->labels = (Label *) calloc((Asm->table)->size_of_labels, sizeof(Label));
    if ((Asm->table)->labels == NULL)
    {
        return ERROR_OF_CONSTRUCTOR_ASM;
    }
    for (size_t i = 0; i < (Asm->table)->size_of_labels; i++)
    {
        ((Asm->table)->labels)[i].address = -1;
    }
    return NO_ASM_ERRORS;
}


static Errors_of_ASM get_count_of_rows(struct ASM *Asm)
{
    if (Asm == NULL)
    {
        return ERROR_OF_READING_FROM_FILE;
    }
    int c = 0;
    while ((c = getc(Asm->file_pointer)) != EOF)
    {

        if (c == '\n')
        {
            (Asm->count_of_rows)++;
        }
    }
    rewind(Asm->file_pointer);
    int cnt = 0;
    c = getc(Asm->file_pointer);
    int c1 = c;
    while ((c = getc(Asm->file_pointer)) != EOF)
    {
        if (c1 == '\n' && c == '\n')
        {
            cnt++;
        }
        c1 = c;
    }
    Asm->count_of_rows -= (size_t)cnt;
    rewind(Asm->file_pointer);
    return NO_ASM_ERRORS;
}


int main(int argc, char *argv[])
{
    struct Tree tree = {0};
    const char *file_name = argv[1];
    Errors_of_tree error = tree_reader(&tree, file_name);
    if (error != NO_ERRORS_TREE)
    {  
        fprintf(stderr, "error of tree = %d\n", error);
        abort();
    }
    struct Labels *all_labels = (struct Labels *) calloc(SIZE_OF_ALL_VARIABLES, sizeof(struct Labels));
    if (all_labels == NULL)
    {
        fprintf(stderr, "ERROR OF CREATE ARRAY OF LABELS\n");
        abort();
    }
    Errors_of_ASM error_asm = transform_programm_to_assembler(&tree, &all_labels);
    if (error_asm != NO_ASM_ERRORS)
    {
        fprintf(stderr, "error_asm = %d\n", error_asm);
        abort();
    }
    // for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    // {
    //     printf("Label name = %s\n", (all_labels[index]).name);
    // }
    
    printf("Assembler started!\n");
    //Errors_of_ASM error_asm = NO_ASM_ERRORS;
    struct CMD all_commands[] = {{"push",  CMD_PUSH},
                                 {"pop",   CMD_POP},
                                 {"add",   CMD_ADD},
                                 {"sub",   CMD_SUB},
                                 {"mul",   CMD_MUL},
                                 {"div",   CMD_DIV},
                                 {"out",   CMD_OUT},
                                 {"in",    CMD_IN},
                                 {"sqrt",  CMD_SQRT},
                                 {"deg",   CMD_DEG},
                                 {"sin",   CMD_SIN},
                                 {"cos",   CMD_COS},
                                 {"dump",  CMD_DUMP},
                                 {"jmp",   CMD_JMP},
                                 {"ja",    CMD_JA},
                                 {"jae",   CMD_JAE},
                                 {"jb",    CMD_JB},
                                 {"jbe",   CMD_JBE},
                                 {"je",    CMD_JE},
                                 {"jne",   CMD_JNE},
                                 {"label", CMD_LABEL},
                                 {"print", CMD_PRINT},
                                 {"inf",   CMD_PRINT_INF},
                                 {"none",  CMD_PRINT_NONE},
                                 {"here",  CMD_HERE},
                                 {"hlt",   CMD_HLT}};
    size_t size_of_all_commands = sizeof(all_commands) / sizeof(CMD);

    // struct Labels all_labels[] = {{"NEXT:"},
    //                               {"SKIP:"}};

    // struct Labels all_labels_new[] = {{"A=0:"},
    //                               {"B=0:"},
    //                               {"C=0:"},
    //                               {"D>0:"},
    //                               {"D=0:"},
    //                               {"D<0:"}};
    size_t size_of_all_labels = SIZE_OF_ALL_VARIABLES;
    // size_t size_of_all_labels_new = sizeof(all_labels_new) / sizeof(all_labels[0]);


    struct ASM Asm = {0};
    (Asm.file_pointer) = fopen("Assembler/source/asm_programm.txt", "rb");
    if (Asm.file_pointer == NULL)
    {
        fprintf(stderr, "ERROR_OF_OPEN_FILE\n");
        return 1;
    }
    error_asm = get_count_of_rows(&Asm);
    //printf("count_of_rows = %lu\n", Asm.count_of_rows);
    if (error_asm != NO_ASM_ERRORS)
    {
        fprintf(stderr, "error_asm=%d\n", error_asm);
        return 1;
    }

    error_asm = constructor(&Asm, size_of_all_labels);
    if (error_asm != NO_ASM_ERRORS)
    {
        
        fprintf(stderr, "error_asm=%d\n", error_asm);
        return 1;
    }

    error_asm = get_commands(&Asm, all_labels, size_of_all_labels);
    if (error_asm != NO_ASM_ERRORS)
    {
        fprintf(stderr, "error=%d\n", error_asm);
        return 1;
    }
    // for (size_t index = 0; index < (Asm.table)->size_of_labels; index++)
    // {
    //     printf("label = %s\n", (((Asm.table)->labels)[index]).name);
    // }
    error_asm = repeat_get_commands(&Asm);
    if (error_asm != NO_ASM_ERRORS)
    {
        fprintf(stderr, "error_asm=%d\n", error_asm);
        return 1;
    }

    error_asm = transform_commands(&Asm, all_commands, size_of_all_commands);
    if (error_asm != NO_ASM_ERRORS)
    {
        fprintf(stderr, "error_asm=%d\n", error_asm);
        return 1;
    }

    error_asm = create_file_with_commands(&Asm);
    if (error_asm != NO_ASM_ERRORS)
    {
        printf("Here\n");
        fprintf(stderr, "error_asm=%d\n", error_asm);
        return 1;
    }

    error_asm = destructor(&Asm);
    if (error_asm != NO_ASM_ERRORS)
    {
        fprintf(stderr, "error_asm=%d\n", error_asm);
        return 1;
    }

    error = tree_destructor(&tree);
    free(all_labels);

    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error of tree = %d\n", error);
        return 1;
    }

    printf("Assembler finished!\n");
    return 0;
}



static Errors_of_ASM destructor(struct ASM *Asm)
{
    if (Asm == NULL)
    {
        return ERROR_OF_DESTRUCTOR_ASM;
    }
    fclose(Asm->file_pointer);
    Asm->count_of_rows = 0;
    free(Asm->commands);
    (Asm->table)->size_of_labels = 0;
    free((Asm->table)->labels);
    free(Asm->table);
    return NO_ASM_ERRORS;
}
