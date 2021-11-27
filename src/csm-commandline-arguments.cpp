#include <stdio.h>
#include <string.h>

#include <getopt.h>

#include "csm-commandline-arguments.h"
#include "color-print-lib.h"

#define ARG_NAME_FILE                   "file"
#define ARG_NAME_FILE_SHORT             "f"
#define ARG_NAME_INDEXNAV               "index-navigation"
#define ARG_NAME_INDEXNAV_SHORT         "i"
#define ARG_NAME_COLOR                  "color"
#define ARG_NAME_ONEBASE                "one-based"
#define ARG_NAME_ONEBASE_SHORT          "o"
#define ARG_NAME_DISABLE_VERIFY         "disable-verify"
#define ARG_NAME_HELP                   "help"
#define ARG_NAME_HELP_SHORT             "h"
#define ARG_NAME_VERSION                "version"
#define ARG_NAME_VERSION_SHORT          "v"

// NOTE: BMG (Nov. 24, 2021) MAKE sure that you add a ":" string after the flag letters that take arguments
#define ARG_NAME_ALLOWED_SHORT_ARGS \
    ARG_NAME_FILE_SHORT ":" \
    ARG_NAME_INDEXNAV_SHORT ":" \
    ARG_NAME_ONEBASE_SHORT \
    ARG_NAME_HELP_SHORT \
    ARG_NAME_VERSION_SHORT

static bool get_options_validate(CommandlineArgs *cmd_args) {
    // TODO: BMG (Nov. 24, 2021) Make sure that you implement this function
    return true;
}

bool get_options_received(CommandlineArgs *cmd_args, int argc, char *argv[]) {
    struct option longopts[] = {
    //   Long option name           Has argument        Flag pointer                                Short option name
        {ARG_NAME_FILE,             required_argument,  NULL,                                       ARG_NAME_FILE_SHORT[0]},
        {ARG_NAME_INDEXNAV,         required_argument,  NULL,                                       ARG_NAME_INDEXNAV_SHORT[0]},
        {ARG_NAME_COLOR,            required_argument,  &cmd_args->color_received,          true},
        {ARG_NAME_ONEBASE,          no_argument,        NULL,                                       ARG_NAME_ONEBASE_SHORT[0]},
        {ARG_NAME_DISABLE_VERIFY,   no_argument,        &cmd_args->disable_verify_received, true},
        {ARG_NAME_HELP,             no_argument,        NULL,                                       ARG_NAME_HELP_SHORT[0]},
        {ARG_NAME_VERSION,          no_argument,        NULL,                                       ARG_NAME_VERSION_SHORT[0]},
        {0, 0, 0, 0},
    };
    int getopt_return;
    int option_index;

    while((getopt_return = getopt_long(argc, argv, ARG_NAME_ALLOWED_SHORT_ARGS, longopts, &option_index)) != -1) {
        switch (getopt_return) {
        case ARG_NAME_FILE_SHORT[0]:
            if (optarg != NULL) {
                strncpy(cmd_args->filename_string, optarg, MAX_FILENAME_STRING_SIZE);
                DEBUG_CMD printf_debug("Filename flag argument received: %s\n", cmd_args->filename_string);
            }
            else {
                printf_error("File flag received a bad argument\n");
                return false;
            }
            break;
        case ARG_NAME_INDEXNAV_SHORT[0]:
            if (optarg != NULL) {
                strncpy(cmd_args->index_navigation_string, optarg, MAX_INDEX_NAVIGATION_STRING_SIZE);
                DEBUG_CMD printf_debug("Index navigation flag argument received: %s\n", cmd_args->index_navigation_string);
            }
            else {
                printf_error("Indexnav flag received a bad argument\n");
                return false;
            }
            break;
        case ARG_NAME_ONEBASE_SHORT[0]:
            cmd_args->one_based_received = true;
            break;
        case ARG_NAME_HELP_SHORT[0]:
            cmd_args->help_received = true;
            break;
        case ARG_NAME_VERSION_SHORT[0]:
            cmd_args->version_received = true;
            break;
        default:
            // TODO: BMG (Nov. 24, 2021) Implement some sort of logging library
            printf_error("We ran into the default with argument: %d\n", getopt_return);
            return false;
        }
    }

    return get_options_validate(cmd_args);
}
