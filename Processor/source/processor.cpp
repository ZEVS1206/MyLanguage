#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>

#include "stack.h"
#include "processor.h"

static int compare(double a, double b);

Errors_of_CPU create_commands()
{
    FILE *fp = fopen("Assembler/source/cpu_commands.txt", "r");
    struct MySPU spu = {0};
    struct stat statistics = {0};
    int res = stat("Assembler/source/cpu_commands.txt", &statistics);
    if (res != 0)
    {
        return ERROR_OF_GETTING_INFORMATION_ABOUT_CMD;
    }

    size_t size_of_file = (uint64_t)statistics.st_size;
    size_t count = 0;
    int rows = 0;
    int c = 0;
    while (rows < 7)
    {
        c = getc(fp);
        if (c == '\n')
        {
            rows++;
        }
        count++;
    }
    size_of_file -= count;
    char *buffer = (char *) calloc(size_of_file, sizeof(char));
    if (buffer == NULL)
    {
        return ERROR_OF_GETTING_INFORMATION_ABOUT_CMD;
    }
    size_t result_of_reading = fread(buffer, sizeof(char), size_of_file, fp);
    if (result_of_reading != size_of_file)
    {
        return ERROR_OF_GETTING_INFORMATION_ABOUT_CMD;
    }
    size_t size = 0;
    for (size_t i = 0; i < size_of_file; i++)
    {
        if (isspace(buffer[i]))
        {
            size++;
        }
    }
    spu.size_of_commands = size;
    spu.commands = (int *) calloc(size, sizeof(int));
    size_t j = 0;
    size_t i = 0;
    while (i < size_of_file)
    {
        bool is_cmd = true;
        char tmp[10] = {0};
        int k = 0;
        while (!isspace(buffer[i]))
        {
            tmp[k++] = buffer[i];
            if (buffer[i] == '.')
            {
                is_cmd = false;
            }
            i++;
        }
        tmp[k] = '\0';
        if (is_cmd)
        {
            (spu.commands)[j] = atoi(tmp);
        }
        else
        {
            char *end = NULL;
            double element = strtod(tmp, &end);
            (spu.commands)[j] = (int)(element * pow(10, ACCURANCY));
        }
        j++;
        i++;
    }
    Errors_of_CPU error = NO_CPU_ERRORS;
    error = do_cmd(&spu);
    free(buffer);
    return error;
}


static int compare(double a, double b)
{
    double eps = 1e-4;
    if (b > a + eps)
    {
        return -1;
    }
    else if (b < a - eps)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


Errors_of_CPU do_cmd(struct MySPU *spu)
{
    spu->stack = (struct MyStack *) calloc(1, sizeof(struct MyStack));
    STACK_CTOR(spu->stack, 10);
    spu->registers = (int *) calloc(4, sizeof(int));
    spu->size_of_registers = 4;
    spu->ram = (Stack_Elem_t *) calloc(RAM_SIZE, sizeof(int));
    Errors error = NO_ERRORS;
    size_t i = 0;
    bool flag = false;
    //printf("ax bx cx dx\n");
    while (i < spu->size_of_commands)
    {
        // for (size_t k = 0; k < (spu->size_of_registers); k++)
        // {
        //     printf("%d ", (spu->registers)[k]);
        // }
        // printf("\n");
        if (flag)
        {
            break;
        }
        Commands cmd = (Commands)(spu->commands)[i];
        switch (cmd)
        {
            case CMD_PUSH:
            {
                i++;
                Stack_Elem_t elem = ((spu->commands)[i] / pow(10, ACCURANCY));
                i++;
                Registers reg = (Registers)(spu->commands)[i];
                i++;
                int ram_address = (spu->commands)[i];
                if (reg == NOT_A_REGISTER)
                {
                    if (compare(elem, TOXIC) == 0)
                    {
                        if (ram_address == -1)
                        {
                            return ERROR_OF_GET_PUSH_CMD;
                        }
                        Stack_Elem_t operand = 0;
                        error = stack_pop(spu->stack, &operand);
                        (spu->ram)[ram_address] = operand;
                    }
                    else
                    {
                        error = stack_push(spu->stack, elem);

                    }
                }
                else
                {
                    if (compare(elem, TOXIC) != 0 || ram_address != -1)
                    {
                        return ERROR_OF_GET_PUSH_CMD;
                    }
                    Stack_Elem_t operand = (spu->registers)[reg - 1];
                    error = stack_push(spu->stack, operand);
                }
                break;
            }
            case CMD_POP:
            {
                i++;
                Stack_Elem_t elem = ((spu->commands)[i] / pow(10, ACCURANCY));
                i++;
                Registers reg = (Registers)(spu->commands)[i];
                i++;
                int ram_address = (spu->commands)[i];
                if (reg == NOT_A_REGISTER)
                {
                    if (compare(elem, TOXIC) == 0)
                    {
                        if (ram_address == -1)
                        {
                            return ERROR_OF_GET_POP_CMD;
                        }
                        Stack_Elem_t operand = (spu->ram)[ram_address];
                        error = stack_push(spu->stack, operand);
                    }
                    error = stack_pop(spu->stack, &elem);
                }
                else
                {
                    if (compare(elem, TOXIC) != 0 || ram_address != -1)
                    {
                        return ERROR_OF_GET_POP_CMD;
                    }
                    Stack_Elem_t operand = 0;
                    error = stack_pop(spu->stack, &operand);
                    (spu->registers)[reg - 1] = (int)operand;
                    //printf("reg = %d\n", (spu->registers)[reg - 1]);
                }
                break;
            }
            case CMD_ADD:
            {
                if ((spu->stack)->size < 1)
                {
                    return ERROR_OF_NOT_ENOUGH_ELEMENTS_IN_STACK;
                }
                Stack_Elem_t first_operand = 0;
                Stack_Elem_t second_operand = 0;
                error = stack_pop(spu->stack, &second_operand);
                error = stack_pop(spu->stack, &first_operand);
                error = stack_push(spu->stack, (first_operand + second_operand));
                //special_dump(&stack);
                break;
            }
            case CMD_SUB:
            {
                if ((spu->stack)->size < 1)
                {
                    return ERROR_OF_NOT_ENOUGH_ELEMENTS_IN_STACK;
                }
                Stack_Elem_t first_operand = 0;
                Stack_Elem_t second_operand = 0;
                error = stack_pop(spu->stack, &second_operand);
                error = stack_pop(spu->stack, &first_operand);
                error = stack_push(spu->stack, (first_operand - second_operand));
                //special_dump(&stack);
                break;
            }
            case CMD_MUL:
            {
                if ((spu->stack)->size < 1)
                {
                    return ERROR_OF_NOT_ENOUGH_ELEMENTS_IN_STACK;
                }
                Stack_Elem_t first_operand = 0;
                Stack_Elem_t second_operand = 0;
                error = stack_pop(spu->stack, &second_operand);
                error = stack_pop(spu->stack, &first_operand);
                error = stack_push(spu->stack, (first_operand * second_operand));
                //special_dump(&stack);
                break;
            }
            case CMD_DIV:
            {
                Stack_Elem_t first_operand = 0;
                Stack_Elem_t second_operand = 0;
                error = stack_pop(spu->stack, &second_operand);
                error = stack_pop(spu->stack, &first_operand);
                error = stack_push(spu->stack, (first_operand / second_operand));
                //special_dump(&stack);
                break;
            }
            case CMD_OUT:
            {
                if ((spu->stack)->size - 1 < 0)
                {
                    return ERROR_OF_NOT_ENOUGH_ELEMENTS_IN_STACK;
                }
                Stack_Elem_t element = 0;
                error = stack_element(spu->stack, &element);
                break;
            }
            case CMD_DUMP:
            {
                special_dump(spu->stack);
                break;
            }
            case CMD_PRINT:
            {
                printf("RAM:\n");
                for (long long int j = 0; j < RAM_SIZE; j++)
                {
                    printf("%lf ", (spu->ram)[j]);
                }
                printf("\n");
                break;
            }
            case CMD_IN:
            {
                Stack_Elem_t element = 0;
                printf("Enter the element: ");
                scanf("%lf", &element);
                error = stack_push(spu->stack, element);
                break;
            }
            case CMD_SIN:
            {
                if ((spu->stack)->size < 1)
                {
                    return ERROR_OF_NOT_ENOUGH_ELEMENTS_IN_STACK;
                }
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                error = stack_push(spu->stack, (sin(operand)));
                break;
            }
            case CMD_COS:
            {
                if ((spu->stack)->size < 1)
                {
                    return ERROR_OF_NOT_ENOUGH_ELEMENTS_IN_STACK;
                }
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                error = stack_push(spu->stack, (cos(operand)));
                break;
            }
            case CMD_SQRT:
            {
                if ((spu->stack)->size < 1)
                {
                    return ERROR_OF_NOT_ENOUGH_ELEMENTS_IN_STACK;
                }
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                error = stack_push(spu->stack, (sqrt(operand)));
                break;
            }
            case CMD_JMP:
            {
                i++;
                i++;
                int position = (spu->commands[i]);
                i = (size_t)position;
                break;
            }
            case CMD_JA:
            {
                i++;
                Registers reg = (Registers)(spu->commands)[i];
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                i++;
                //printf("register = %d\n", (spu->registers)[reg - 1]);
                if (compare((spu->registers)[reg - 1], operand) > 0)
                {
                    int position = (spu->commands)[i];
                    i = (size_t)position;
                    break;
                }
                else
                {
                    break;
                }
            }
            case CMD_JB:
            {
                i++;
                Registers reg = (Registers)(spu->commands)[i];
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                i++;
                //printf("register = %d\n", (spu->registers)[reg - 1]);
                if (compare((spu->registers)[reg - 1], operand) < 0)
                {
                    int position = (spu->commands)[i];
                    i = (size_t)position;
                    break;
                }
                else
                {
                    break;
                }
            }
            case CMD_JAE:
            {
                i++;
                Registers reg = (Registers)(spu->commands)[i];
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                i++;
                //printf("register = %d\n", (spu->registers)[reg - 1]);
                if (compare((spu->registers)[reg - 1], operand) >= 0)
                {
                    int position = (spu->commands)[i];
                    i = (size_t)position;
                    break;
                }
                else
                {
                    break;
                }
            }
            case CMD_JBE:
            {
                i++;
                Registers reg = (Registers)(spu->commands)[i];
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                i++;
                //printf("register = %d\n", (spu->registers)[reg - 1]);
                if (compare((spu->registers)[reg - 1], operand) <= 0)
                {
                    int position = (spu->commands)[i];
                    i = (size_t)position;
                    break;
                }
                else
                {
                    break;
                }
            }
            case CMD_JE:
            {
                i++;
                Registers reg = (Registers)(spu->commands)[i];
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                i++;
                //printf("register = %d\n", (spu->registers)[reg - 1]);
                if (compare((spu->registers)[reg - 1], operand) == 0)
                {
                    int position = (spu->commands)[i];
                    i = (size_t)position;
                    break;
                }
                else
                {
                    break;
                }
            }
            case CMD_JNE:
            {
                i++;
                Registers reg = (Registers)(spu->commands)[i];
                Stack_Elem_t operand = 0;
                error = stack_pop(spu->stack, &operand);
                i++;
                //printf("register = %d\n", (spu->registers)[reg - 1]);
                if (compare((spu->registers)[reg - 1], operand) != 0)
                {
                    int position = (spu->commands)[i];
                    i = (size_t)position;
                    break;
                }
                else
                {
                    break;
                }
            }
            case CMD_PRINT_INF:
            {
                printf("INF\n");
                break;
            }
            case CMD_PRINT_NONE:
            {
                printf("NONE\n");
                break;
            }
            case CMD_HERE:
            {
                printf("Here\n");
                break;
            }
            case CMD_LABEL:
                break;
            case CMD_HLT:
            {
                flag = true;
                break;
            }
            case CMD_UNKNOWN:
                return ERROR_OF_UNKNOWN_CMD;
                break;
            default:
                printf("cmd=%d\n", cmd);
                return ERROR_OF_UNKNOWN_TYPE;
                break;
        }
        i++;
    }
    printf("error_of_stack = %s\n", get_error(error));
    free(spu->commands);
    free(spu->registers);
    error = stack_destructor(spu->stack);
    free(spu->stack);
    free(spu->ram);
    return NO_CPU_ERRORS;
}

int main()
{
    printf("Processor started!\n");
    Errors_of_CPU error = create_commands();
    fprintf(stderr, "error_of_processor = %d\n", error);
    printf("Processor finished!\n");
    return 0;
}

