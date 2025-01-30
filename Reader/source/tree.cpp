#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "tree_dump.h"
#include "tree_input.h"



int main()
{
    struct Tree tree = {0};
    Errors_of_tree error = NO_ERRORS;
    error = tree_constructor(&tree);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error = %d\n", error);
        return error;
    }
    error = get_tree_from_file(&tree, "source/input.txt"); //"../../test_file.rt");
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error = %d\n", error);
        return 1;
    }
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error = %d\n", error);
        return 1;
    }
    tree.tmp_root = tree.root;
    tree.error = NO_ERRORS;
    graphic_dump(&tree, "example");
    if (tree.error != NO_ERRORS)
    {
        fprintf(stderr, "error = %d\n", tree.error);
        return 1;
    }
    if (tree.error != NO_ERRORS)
    {
        fprintf(stderr, "error = %d\n", tree.error);
        return 1;
    }
    error = tree_destructor(&tree);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error = %d\n", error);
        return 1;
    }
    return 0;
}
