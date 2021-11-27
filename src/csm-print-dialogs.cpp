#include <stdio.h>

#include "csm-defines.h"

void csm_print_help_dialog() {
    printf(
   "Usage: csm [options]\n"
   "\n"
   "Options:\n"
   "    -f --file               <file>      The location of the configuration file you'd like to use (If no value is\n"
   "                                        provided the default value will be ./csm.json relative to the location of\n"
   "                                        the executable)\n"
   "    -i --index-navigation   <string>    A comma separated list of numerical indexes to navigate to the desired\n"
   "                                        command\n"
   "    --color                 <color>     Sets the color displayed by the tool, it doesn't affect the color\n"
   "                                        displayed by\n"
   "                                        the commands run (0 = Default, 1 = Dim, 2 = Bright)\n"
   "    -o --one-based                      Uses 1-based indexes for the selection manu instead of 0-based\n"
   "    --disable-verify                    Disable the verification of execution environments that could potentially\n"
   "                                        not\n"
   "                                        exist, like bash on windows.\n"
   "    -h --help                           Print the command-selector-menu help dialog (currently set)\n"
   "    -v --version                        Print the command-selector-menu version dialog\n"
    );
}

void csm_print_version_dialog() {
    // TODO: BMG (Nov. 23, 2021) Eventually it would be a good idea to also include the git hash into the application
    printf("csm version: %d\n", CSM_APPLICATION_VERSION);
}

void csm_print_option_error_dialog() {
    printf("You ran into an error");

    csm_print_help_dialog();
}
