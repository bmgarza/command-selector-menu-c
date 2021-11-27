#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "csm-defines.h"
#include "csm-commandline-arguments.h"
#include "csm-print-dialogs.h"
#include "csm-utilities.h"
#include "color-print-lib.h"

int main(int argc, char *argv[]) {
    DEBUG_CMD printf_debug("This is the beginning of the application\n");

    bool ret_bool = true;

    CommandlineArgs cmd_args;
    ret_bool = get_options_received(&cmd_args, argc, argv);

    if (!ret_bool) {
        csm_print_option_error_dialog();
        return MAIN_CSM_RETURN_COMMANDLINE_OPTION_ERROR;
    }

    if (cmd_args.help_received) {
        csm_print_help_dialog();
        return MAIN_CSM_RETURN_NO_ERROR;
    }

    if (cmd_args.version_received) {
        csm_print_version_dialog();
        return MAIN_CSM_RETURN_NO_ERROR;
    }

    if (!cmd_args.filename_received) {
        // If the filename flag wasn't set, make sure we set it to the csm.json file that is in the same directory as
        //  the csm binary
        ret_bool = csm_get_binary_working_dir(cmd_args.filename_string, MAX_FILENAME_STRING_SIZE);
        if (!ret_bool) {
            printf_error("Couldn't get the working directory of the binary.");
            return MAIN_CSM_RETURN_BINARY_PATH_ERROR;
        }
        strcat(cmd_args.filename_string, "/" DEFAULT_CSM_JSON_RELATIVE_PATH);
        DEBUG_CMD printf_debug("Calculated path to csm.json: %s\n", cmd_args.filename_string);
    }

    // TODO: BMG (Nov. 24, 2021) Work on opening the file and checking to make sure that there were no errors. The next
    //  step after that will be to verify the json format and put everything into a buffer that can then be parsed by
    //  microjson to fill out the structures that we're working with.
    // FILE *csm_json_file = NULL;
    // csm_json_file = fopen(cmd_args.filename_string, "r");
    // if (csm_json_file == NULL) {
    //     // The file does not exist, or at least the pointer was not set
    //     printf("ERROR: Couldn't open the csm.json file (%s)\n", cmd_args.filename_string);
    //     return MAIN_CSM_RETURN_CSM_JSON_FILE_OPEN_ERROR;
    // }

    printf_error("This program is still not in a finished state.\n");

    return MAIN_CSM_RETURN_NOT_FINISHED;
    // return MAIN_CSM_RETURN_NO_ERROR;
}
