#include <stdio.h>
#include <string.h>

#ifdef __linux
    #include <unistd.h>
#endif

#ifdef __APPLE__
    #include <mach-o/dyld.h>
#endif

#include "csm-defines.h"
#include "csm-utilities.h"

/**
 * @brief Get the path of the binary
 *
 * This function is meant to write the path of the binary that is currently being run in the path_string variable. This
 * is likely a function that is going to have multiple implementations in Linux, MacOS, and Windows.
 *
 * @param path_string The pointer to the string where the path is going to be written to
 * @param path_string_length 
 */
bool csm_get_binary_working_dir(char *path_string, uint16_t path_string_length) {
    #ifdef __linux
        readlink("/proc/self/exe", path, MAX_FILENAME_STRING_SIZE);
    #endif

    #ifdef __APPLE__
        uint32_t macos_path_size = MAX_FILENAME_STRING_SIZE;
        int macos_path_return = 0;
        macos_path_return = _NSGetExecutablePath(path_string, &macos_path_size);
        if (macos_path_return == -1) {
            printf("Unable to copy the path of the binary to the path variable. Buffer size necessary: %u\n", macos_path_size);
            return false;
        }
        char *last_backslash = strrchr(path_string, '/');
        if (last_backslash != NULL) {
            *last_backslash = '\0';
        }
    #endif

    return true;
}
