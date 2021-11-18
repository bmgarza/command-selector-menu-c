#include <stdint.h>
#include <stdio.h>

#include "csm-defines.h"
#include "simple-class.h"

int main(int argc, char *argv[]) {
    printf("This is a test\n");

    SimpleClass sc;

    sc.print_something();

    return MAIN_CSM_RETURN_NO_ERROR;
}
