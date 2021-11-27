#ifndef CSM_COMMANDLINE_ARGUMENTS_H
#define CSM_COMMANDLINE_ARGUMENTS_H

#include <stdint.h>

#include "csm-defines.h"

class CommandlineArgs {
    public:
    int filename_received = false;
    char filename_string[MAX_FILENAME_STRING_SIZE];

    int index_navigation_received = false;
    char index_navigation_string[MAX_INDEX_NAVIGATION_STRING_SIZE];

    int color_received = false;
    CONSOLE_COLORS_ENUM color = CONSOLE_COLORS_DEFAULT;

    int one_based_received = false;

    int disable_verify_received = false;

    int help_received = false;

    int version_received = false;
};

bool get_options_received(CommandlineArgs *cmd_args, int argc, char *argv[]);

// CSM_COMMANDLINE_ARGUMENTS_H
#endif
