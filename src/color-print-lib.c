#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "color-print-lib.h"

void printf_debug(const char *string,...) {
    va_list arg_list;
    va_start(arg_list, string);
    // Prepend a yellow DEBUG message before printing the message that was originally passed through
    printf(COLOR_YELLOW "DEBUG: " COLOR_NO_COLOR);
    vprintf(string, arg_list);
}

void printf_error(const char *string,...) {
    va_list arg_list;
    va_start(arg_list, string);
    // Prepend a yellow DEBUG message before printing the message that was originally passed through
    printf(COLOR_RED "ERROR: " COLOR_NO_COLOR);
    vprintf(string, arg_list);
}

void printf_color(const char *color_code, const char *string,...) {
    va_list arg_list;
    va_start(arg_list, string);
    // Set the color before we print the string and then reset the color after we are done printing the string that was
    //  passed through
    printf("%s", color_code);
    vprintf(string, arg_list);
    printf(COLOR_NO_COLOR);
}
