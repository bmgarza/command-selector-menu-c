#ifndef CSM_DEFINES_H
#define CSM_DEFINES_H

// NOTE: BMG (Nov. 23, 2021) This isn't a very interesting version indicator, it's a single number and it should be
//  incremented by one any time the version of the application is updated.
#define CSM_APPLICATION_VERSION 1

// Define the maximum string size for important variables
#define MAX_FILENAME_STRING_SIZE 255
#define MAX_INDEX_NAVIGATION_STRING_SIZE 50

#define DEFAULT_CSM_JSON_RELATIVE_PATH "./csm.json"

typedef enum {
    MAIN_CSM_RETURN_NO_ERROR,
    MAIN_CSM_RETURN_COMMANDLINE_OPTION_ERROR,
    MAIN_CSM_RETURN_BINARY_PATH_ERROR,
    MAIN_CSM_RETURN_CSM_JSON_FILE_OPEN_ERROR,
    MAIN_CSM_RETURN_NOT_FINISHED,
} MAIN_CSM_RETURN_ENUM;

typedef enum {
    CONSOLE_COLORS_DEFAULT,
    CONSOLE_COLORS_DIM,
    CONSOLE_COLORS_BRIGHT,
    CONSOLE_COLORS_COUNT,
} CONSOLE_COLORS_ENUM;

// CSM_DEFINES_H
#endif

