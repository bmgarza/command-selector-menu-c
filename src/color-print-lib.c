#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

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
    if (
        0 == strcmp(color_code, COLOR_BLACK)        || 0 == strcmp(color_code, COLOR_RED)           ||
        0 == strcmp(color_code, COLOR_GREEN)        || 0 == strcmp(color_code, COLOR_BROWN_ORANGE)  ||
        0 == strcmp(color_code, COLOR_BLUE)         || 0 == strcmp(color_code, COLOR_PURPLE)        ||
        0 == strcmp(color_code, COLOR_CYAN)         || 0 == strcmp(color_code, COLOR_LIGHT_GRAY)    ||
        0 == strcmp(color_code, COLOR_DARK_GRAY)    || 0 == strcmp(color_code, COLOR_LIGHT_RED)     ||
        0 == strcmp(color_code, COLOR_LIGHT_GREEN)  || 0 == strcmp(color_code, COLOR_YELLOW)        ||
        0 == strcmp(color_code, COLOR_LIGHT_BLUE)   || 0 == strcmp(color_code, COLOR_LIGHT_PURPLE)  ||
        0 == strcmp(color_code, COLOR_LIGHT_CYAN)   || 0 == strcmp(color_code, COLOR_WHITE)
    ) {
        // Make sure that the color that was passed through is an actual color before we decide to prepend it to the
        //  string that we are printing.
        printf("%s", color_code);
    }
    vprintf(string, arg_list);
    printf(COLOR_NO_COLOR);
}
