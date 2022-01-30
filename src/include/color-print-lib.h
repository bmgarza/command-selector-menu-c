#ifndef PRETTY_PRINT_LIB_H
#define PRETTY_PRINT_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

// ANSI Color Escape Values
// Black        0;30     Dark Gray     1;30
// Red          0;31     Light Red     1;31
// Green        0;32     Light Green   1;32
// Brown/Orange 0;33     Yellow        1;33
// Blue         0;34     Light Blue    1;34
// Purple       0;35     Light Purple  1;35
// Cyan         0;36     Light Cyan    1;36
// Light Gray   0;37     White         1;37
#define COLOR_NO_COLOR      "\033[0m"

#define COLOR_BLACK         "\033[0;30m"
#define COLOR_RED           "\033[0;31m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_BROWN_ORANGE  "\033[0;33m"
#define COLOR_BLUE          "\033[0;34m"
#define COLOR_PURPLE        "\033[0;35m"
#define COLOR_CYAN          "\033[0;36m"
#define COLOR_LIGHT_GRAY    "\033[0;37m"

#define COLOR_DARK_GRAY     "\033[1;30m"
#define COLOR_LIGHT_RED     "\033[1;31m"
#define COLOR_LIGHT_GREEN   "\033[1;32m"
#define COLOR_YELLOW        "\033[1;33m"
#define COLOR_LIGHT_BLUE    "\033[1;34m"
#define COLOR_LIGHT_PURPLE  "\033[1;35m"
#define COLOR_LIGHT_CYAN    "\033[1;36m"
#define COLOR_WHITE         "\033[1;37m"

/**
 * @brief Works like printf, but appends a Yellow DEBUG: at the beginning of what is printed.
 */
void printf_debug(const char *string,...);

/**
 * @brief Works like printf, but appends a Red ERROR: at the beginning of what is printed.
 */
void printf_error(const char *string,...);

/**
 * @brief Works like printf, but allows for a custom color to be used. The whole string is the same color.
 * 
 * @param color_code 
 * @param string 
 * @param ... 
 */
void printf_color(const char *color_code, const char *string,...);

#ifdef __cplusplus
}
#endif

// PRETTY_PRINT_LIB_H
#endif
