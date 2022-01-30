#include <stdio.h>
#include <string.h>

#include "csm-defines.h"
#include "color-print-lib.h"
#include "json-process-utils.h"

#include "log.h"

// The max parameter string size is 20, which means that the array to contain that would be 1 more because we need to
// make sure that we store the null terminating character at the end.
#define MAX_PARAMETER_STRING_SIZE 20
#define MAX_PARAMETER_ARRAY_SIZE MAX_PARAMETER_STRING_SIZE + 1

#define JSON_CHARACTER_OBJ_BRACKET          '{'
#define JSON_CHARACTER_OBJ_CLOSE_BRACKET    '}'
#define JSON_CHARACTER_ARR_BRACKET          '['
#define JSON_CHARACTER_ARR_CLOSE_BRACKET    ']'
#define JSON_CHARACTER_QUOTE                '"'
#define JSON_CHARACTER_SINGLE_QUOTE         '\''
#define JSON_CHARACTER_COLON                ':'
#define JSON_CHARACTER_COMMA                ','
#define JSON_CHARACTER_ESCAPE               '\\'
#define JSON_CHARACTER_COMMENT              '/'

// This set of characters encapsulates the set of characters that could be at the beginning of a number declaration
#define JSON_CHARACTER_NUMBER_NEG           '-'
#define JSON_CHARACTER_NUMBER_ZERO          '0'
#define JSON_CHARACTER_NUMBER_ONE           '1'
#define JSON_CHARACTER_NUMBER_TWO           '2'
#define JSON_CHARACTER_NUMBER_THREE         '3'
#define JSON_CHARACTER_NUMBER_FOUR          '4'
#define JSON_CHARACTER_NUMBER_FIVE          '5'
#define JSON_CHARACTER_NUMBER_SIX           '6'
#define JSON_CHARACTER_NUMBER_SEVEN         '7'
#define JSON_CHARACTER_NUMBER_EIGHT         '8'
#define JSON_CHARACTER_NUMBER_NINE          '9'
// These characters are just the extra set of characters that could be used in the definition of a number
#define JSON_CHARACTER_NUMBER_PLUS          '+'
#define JSON_CHARACTER_NUMBER_EXP_CAP       'E'
#define JSON_CHARACTER_NUMBER_EXP_LOW       'e'
#define JSON_CHARACTER_NUMBER_DECIMAL       '.'

#define MAX_JSON_CONSTANT_STRING_LEN        5
#define JSON_CHARACTER_TRUE_BEGINNING       't'
#define JSON_TRUE_STRING                    "true"
#define JSON_CHARACTER_FALSE_BEGINNING      'f'
#define JSON_FALSE_STRING                   "false"
#define JSON_CHARACTER_NULL_BEGINNING       'n'
#define JSON_NULL_STRING                    "null"

#define JSON_CHARACTER_SPACE                ' '
#define JSON_CHARACTER_NEW_LINE             '\n'
#define JSON_CHARACTER_CARRIAGE_RETURN      '\r'
#define JSON_CHARACTER_HORIZONTAL_TAB       '\t'

#define JSON_ESCAPE_CHAR_QUOTE              '"'
#define JSON_ESCAPE_CHAR_BACKSLASH          '\\'
#define JSON_ESCAPE_CHAR_FORWARDSLASH       '/'
#define JSON_ESCAPE_CHAR_BACKSPACE          'b'
#define JSON_ESCAPE_CHAR_FORMFEED           'f'
#define JSON_ESCAPE_CHAR_NEW_LINE           'n'
#define JSON_ESCAPE_CHAR_CARRIAGE_RETURN    'r'
#define JSON_ESCAPE_CHAR_TAB                't'
#define JSON_ESCAPE_CHAR_UNICODE            'u'
#define JSON_ESCAPE_CHAR_UNICODE_HEX_CHARS  "0123456789ABCDEFabcdef"
#define JSON_ESCAPE_CHAR_UNICODE_COUNT      4

typedef enum {
    JSON_TYPES_STRING,
    JSON_TYPES_NUMBER,
    JSON_TYPES_OBJECT,
    JSON_TYPES_ARRAY,
    JSON_TYPES_TRUE,
    JSON_TYPES_FALSE,
    JSON_TYPES_NULL,
    JSON_TYPES_NOT_VALID_TYPE,
} JSON_TYPES_ENUM;
#define JSON_STRING_ARRAY_SIZE              7
#define JSON_STRING_STRING                  "string"
#define JSON_STRING_NUMBER                  "number"
#define JSON_STRING_OBJECT                  "object"
#define JSON_STRING_ARRAY                   "array"
#define JSON_STRING_TRUE                    "true"
#define JSON_STRING_FALSE                   "false"
#define JSON_STRING_NULL                    "null"

#define MAX_COORDINATE_LINE_LENGTH 100
// When we print the coordinates, we are going to print the lower coordinates in a comma separated string with the last
//  coordinate being 0 because we are trying to look for the first object in an array that is supposed to have at least
//  one object. We only want to print this stuff if we are in the correct debugging mode
#define PRINT_COULDNT_FIND_COORDINATE_ERROR(index_arr, index_cnt, last_index)   \
    char output_string[MAX_COORDINATE_LINE_LENGTH];                             \
    int output_string_runner = 0;                                               \
    output_string_runner += sprintf(                                            \
        &output_string[output_string_runner],                                   \
        "Couldn't find an the object at the following index coordinates: "      \
    );                                                                          \
    output_string_runner += json_sprint_comma_separated_coordinates(            \
        &output_string[output_string_runner],                                   \
        index_arr,                                                              \
        index_cnt                                                               \
    );                                                                          \
    output_string_runner += sprintf(                                            \
        &output_string[output_string_runner],                                   \
        ",%d\n",                                                                \
        last_index                                                              \
    );                                                                          \
    log_error(output_string);

#define PULL_NEXT_CHAR(char_ptr)                                            \
    if (!get_char_func(char_ptr)) {                                         \
        log_error("The get Function failed to get the next character.");    \
        return JSON_ERR_GET_FUNCTION_FAILURE;                               \
    }

#define PUSH_NEXT_CHAR(char)                                                \
    if (!set_char_func(char)) {                                             \
        log_error("The set Function failed to set the next character.");    \
        return JSON_ERR_SET_FUNCTION_FAILURE;                               \
    }

static JSON_ERR_ENUM json_traverse_json_object(json_get_next_character get_char_func,
                                               uint8_t *object_index_arr, int object_index_cnt,
                                               char *recursive_parameter_name, int *current_char);
static JSON_ERR_ENUM json_record_walk_to_end_of_object( json_get_next_character get_char_func,
                                                        json_set_next_character set_char_func,
                                                        char **ignore_obj_str_arr, int ignore_obj_str_cnt,
                                                        char *recursive_parameter_name, int *current_char);
static JSON_ERR_ENUM json_walk_through_object_or_array_to_next_value(   json_get_next_character get_char_func,
                                                                        JSON_TYPES_ENUM search_type,
                                                                        int *current_char);
static JSON_ERR_ENUM json_go_to_start_of_array_of_recursive_parameter_name( json_get_next_character get_char_func,
                                                                            char *recursive_parameter_name,
                                                                            int *current_char);

static JSON_ERR_ENUM json_traverse_array_to_coordinate( json_get_next_character get_char_func, int object_index,
                                                        int *current_char, int *err_index);
static JSON_TYPES_ENUM json_get_type_from_character(int current_char);
static JSON_ERR_ENUM json_walk_to_start_of_parameter_value(json_get_next_character get_char_func, int *current_char);
static JSON_ERR_ENUM json_walk_to_end_of_string(json_get_next_character get_char_func, int *current_char);
static JSON_ERR_ENUM json_walk_to_end_of_number(json_get_next_character get_char_func, int *current_char);
static JSON_ERR_ENUM json_walk_to_end_of_object(json_get_next_character get_char_func, int *current_char);
static JSON_ERR_ENUM json_walk_to_end_of_array(json_get_next_character get_char_func, int *current_char);
static JSON_ERR_ENUM json_walk_to_end_of_true(json_get_next_character get_char_func, int *current_char);
static JSON_ERR_ENUM json_walk_to_end_of_false(json_get_next_character get_char_func, int *current_char);
static JSON_ERR_ENUM json_walk_to_end_of_null(json_get_next_character get_char_func, int *current_char);
static int json_sprint_comma_separated_coordinates(char *starting_ptr, uint8_t *object_index_arr, int object_index_cnt);

/**
 * @brief This function traverses a recursive json object to find the object at the coordinates provided in the
 * "object_index_arr" array.
 *
 * This function is meant to help with handling large recursive json objects, allowing for a c program to process the
 * recursive file without having to take a large memory footprint. This does come with a bit of a downside, however, it
 * reduces memory footprint necessary for processing a recursive json object by fully recursing through the json file
 * once every time that this function is called. Given the fact that this function is meant to help processing recursive
 * json objects with recursive arrays, that means that we would traverse through the json file multiple times. Assume
 * that this function is slow to execute due to the need to constantly access storage, so keep that in mind when working
 * with this function.
 *
 * When this function is run, the result should be that the array that is being filled by the set_char_func callback
 * would be filled with a json object containing all the parameters that are NOT in the ignore_obj_str_arr. This string
 * could then be parsed by a json parser like microjson or something similar.
 *
 * If the function doesn't successfully find an object at the coordinates indicated, then a corresponding error will be
 * returned by the function.
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param set_char_func callback to set the next character in the json buffer.
 * @param object_index_arr An array of indexes that indicates the location of the object that we're interested in
 *                         getting.
 * @param object_index_cnt The size of the index array.
 * @param ignore_obj_str_arr An array of parameter names that we are going to ignore writing to the buffer.
 * @param ignore_obj_str_cnt The size of the array of ignored parameter names.
 * @param recursive_parameter_name The name of the recursive parameter.
 * @return JSON_ERR_ENUM 
 */
JSON_ERR_ENUM json_get_sub_json_object( json_get_next_character get_char_func, json_set_next_character set_char_func,
                                        uint8_t *object_index_arr, int object_index_cnt,
                                        char **ignore_obj_str_arr, int ignore_obj_str_cnt,
                                        char *recursive_parameter_name) {
    int current_char;
    JSON_ERR_ENUM err_holder = JSON_ERR_NO_ERR;

    // TODO: BMG (Jan. 18, 2022) We need to review this function and make sure that the way that we are handling walking
    //  through the file is consistent across this function and the other functions that we have written.

    // Make sure that the object index count is more than 0, because we need at least one coordinate
    if (object_index_cnt == 0) {
        log_error("There are not enough coordinates provided in the object_index_arr. Expected more than 0.");
        return JSON_ERR_OBJECT_INDEX_ARRAY_EMPTY;
    }

    // We need to run this first to make sure that when we start processing the json file we start within the main
    //  object of the json file, not outside of it to keep it consistent with what would happen at other parts of the
    //  file processing. By the end of this function call we should be at the beginning of a JSON object.
    err_holder = json_walk_through_object_or_array_to_next_value(get_char_func, JSON_TYPES_OBJECT, &current_char);
    if (err_holder != JSON_ERR_NO_ERR) {
        // NOTE: BMG (Jan. 05, 2022) Assume that the error message was already printed out by the function.
        return err_holder;
    }

    // NOTE: BMG (Jan. 02, 2022) In this case, we are at the beginning of the primary object of the json file, but that
    //  means that we aren't necessarily at the beginning of the array that contains the objects that we are trying to
    //  traverse. Because of this we need to make sure that we first go through what we need to get through to get to
    //  the opening of the top level array we are going to work with.
    err_holder = json_go_to_start_of_array_of_recursive_parameter_name(get_char_func, recursive_parameter_name, &current_char);
    if (err_holder != JSON_ERR_NO_ERR) {
        // NOTE: BMG (Jan. 05, 2022) Assume that the error message was already printed out by the function.
        return err_holder;
    }

    if (object_index_cnt > 1) {
        // If there is more than one coordinate provided to us, make sure that we pass those lower coordinates to the
        //  json_traverse_json_object function.
        err_holder = json_traverse_json_object(get_char_func,
                                               object_index_arr, object_index_cnt - 1,
                                               recursive_parameter_name, &current_char);

        if (err_holder != JSON_ERR_NO_ERR) {
            // NOTE: BMG (Jan. 05, 2022) Assume that the error message was already printed out by the function.
            return err_holder;
        }
    }

    // Pull the top coordinate from the index array
    int top_coordinate = object_index_arr[object_index_cnt - 1];

    // Make sure that we walk through the array of objects until we get to the top coordinate that we are interested in.
    int err_index = 0;
    err_holder = json_traverse_array_to_coordinate(get_char_func, top_coordinate, &current_char, &err_index);
    if (err_holder != JSON_ERR_NO_ERR) {
        PRINT_COULDNT_FIND_COORDINATE_ERROR(object_index_arr, object_index_cnt - 1, err_index);
        return err_holder;
    }

    // NOTE: BMG (Jan. 02, 2022) At this point we are at the beginning of the object defined by the top coordinate, so
    //  the next thing that we need to do is make sure that the object that we are interested in is pushed onto the
    //  output buffer.
    err_holder = json_record_walk_to_end_of_object(get_char_func, set_char_func,
                                                   ignore_obj_str_arr, ignore_obj_str_cnt,
                                                   recursive_parameter_name, &current_char);
    if (err_holder != JSON_ERR_NO_ERR) {
        PRINT_COULDNT_FIND_COORDINATE_ERROR(object_index_arr, object_index_cnt - 1, top_coordinate);
        return err_holder;
    }

    // NOTE: BMG (Jan. 02, 2022) At this point all the characters necessary for the json object have been passed through
    //  to the set_char_func, so we have done all we need to do successfully.
    return JSON_ERR_NO_ERR;
}

/**
 * @brief This function traverses a recursive object to the level it needs to get to.
 *
 * This function would basically traverse the json object given the set of indexes in the array like coordinates. Unlike
 * the coordinates of the json_get_sub_json_object, the coordinates for this function aren't meant to take it to the
 * beginning of the object definition at the coordinate, but instead to the beginning of the object array in the
 * "recursive_parameter_name" parameter name.
 * 
 * In other words, if you pass through the coordinates of [5, 4, 1] to this function, it would:
 *  1.  Go to the start of the array at the "recursive_parameter_name" parameter of the 5th object of the first array.
 *  2.  Go to the start of the array at the "recursive_parameter_name" parameter of the 4th object of the second array.
 *  3.  Go to the start of the array at the "recursive_parameter_name" parameter of the 1st object of the third array.
 * 
 * Generally speaking, this function should receive an array with a size 1 less than the array that was recieved by the
 * "json_get_sub_json_object".
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param set_char_func callback to set the next character in the json buffer.
 * @param object_index_arr An array of indexes that indicates the location of the object that we're interested in
 *                         getting.
 * @param object_index_cnt The size of the index array.
 * @param recursive_parameter_name The name of the recursive parameter.
 * @return true 
 * @return false 
 */
static JSON_ERR_ENUM json_traverse_json_object(json_get_next_character get_char_func,
                                               uint8_t *object_index_arr, int object_index_cnt,
                                               char *recursive_parameter_name, int *current_char) {
    int err_index = 0;
    JSON_ERR_ENUM err_holder = JSON_ERR_NO_ERR;

    for (int i = 0; i < object_index_cnt; i++) {
        err_holder = json_traverse_array_to_coordinate(get_char_func, i, current_char, &err_index);
        if (err_holder != JSON_ERR_NO_ERR) {
            PRINT_COULDNT_FIND_COORDINATE_ERROR(object_index_arr, object_index_cnt - 1, err_index);
            return err_holder;
        }

        err_holder = json_go_to_start_of_array_of_recursive_parameter_name(get_char_func, recursive_parameter_name, current_char);
        if (err_holder != JSON_ERR_NO_ERR) {
            // NOTE: BMG (Jan. 05, 2022) Assume that the error message was already printed out by the function.
            return err_holder;
        }
    }

    return JSON_ERR_NO_ERR;
}

/**
 * @brief 
 * 
 * @param get_char_func 
 * @param object_index 
 * @param current_char 
 * @param err_index 
 * @return JSON_ERR_ENUM 
 */
static JSON_ERR_ENUM json_traverse_array_to_coordinate(json_get_next_character get_char_func, int object_index,
                                                       int *current_char, int *err_index) {
    JSON_ERR_ENUM err_holder = JSON_ERR_NO_ERR;

    // NOTE: BMG (Jan. 02, 2022) At this point we are sure that we are at the beginning of the array that contains the
    //  object defined by the top coordinate in the coordinates that were provided to the function.
    err_holder = json_walk_through_object_or_array_to_next_value(get_char_func, JSON_TYPES_OBJECT, current_char);
    if (err_holder != JSON_ERR_NO_ERR) {
        // If the first object in the array doesn't exist, return an error.
        *err_index = 0;
        return err_holder;
    }

    // NOTE: BMG (Jan. 02, 2022) At this point we are sure that we are at the beginning of the first object in the array
    //  that contains the object defined by the top coordinate in the coordinates that were provided to the function. We 
    //  need to recurse through each of the objects in the array until we get to the object that we are interested in.
    for (int i = 0; i < object_index; i++) {
        err_holder = json_walk_to_end_of_object(get_char_func, current_char);
        if (err_holder != JSON_ERR_NO_ERR) {
            *err_index = i;
            return err_holder;
        }

        err_holder = json_walk_through_object_or_array_to_next_value(get_char_func, JSON_TYPES_OBJECT, current_char);
        if (err_holder != JSON_ERR_NO_ERR) {
            *err_index = i + 1;
            return err_holder;
        }
    }

    return JSON_ERR_NO_ERR;
}

/**
 * @brief Traverse the json string to the end of the current object that we're in.
 *
 * This function basically keeps track of the brackets found in the json string to make sure that we find the
 * appropriate end bracket for the object that we're currently in. This is of course assuming that we're actually in an
 * object. If we can't find the end of the object, return an error to indicate that.
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param set_char_func callback to set the next character in the json buffer.
 * @param ignore_obj_str_arr An array of parameter names that we are going to ignore writing to the buffer.
 * @param ignore_obj_str_cnt The size of the array of ignored parameter names.
 * @param recursive_parameter_name The name of the recursive parameter.
 * @param write_object A boolean to indicate whether we should write the object that we are traversing or not.
 * @return TODO: BMG (Dec. 29, 2021) The return for this function needs to be changed from being an integer return to an
 * enum return.
 */
static JSON_ERR_ENUM json_record_walk_to_end_of_object(json_get_next_character get_char_func,
                                                       json_set_next_character set_char_func,
                                                       char **ignore_obj_str_arr, int ignore_obj_str_cnt,
                                                       char *recursive_parameter_name, int *current_char) {
    // I can't imagine that that we will run into more than 255 nested objects, or at least I hope we don't.
    int escape_char = 0;
    uint8_t nesting_obj_count = 0; 
    bool in_string = false;
    char hex_chars[] = JSON_ESCAPE_CHAR_UNICODE_HEX_CHARS;

    while (true) {
        PULL_NEXT_CHAR(current_char);

        if (*current_char == EOF) {
            // We found an EOF character when we didn't expect it. Return a failure for this function, the JSON format
            //  for the file provided is wrong.
            log_error("Reached end of file before we found the end of the object.");
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }

        if (in_string) {
            // We are going to process the characters in a string
            if (*current_char == JSON_CHARACTER_NEW_LINE) {
                // JSON strings cannot contain a new line character, they can only span a single line.
                log_error("Reached a new line character, strings are supposed to be a single line only.");
                return JSON_ERR_INCORRECT_JSON_FORMAT;
            }

            if (*current_char == JSON_CHARACTER_ESCAPE) {
                // If we run into an escape character, we will pull a second character to see what it is.
                PULL_NEXT_CHAR(&escape_char);

                switch (escape_char) {
                case JSON_ESCAPE_CHAR_QUOTE:
                case JSON_ESCAPE_CHAR_BACKSLASH:
                case JSON_ESCAPE_CHAR_FORWARDSLASH:
                case JSON_ESCAPE_CHAR_BACKSPACE:
                case JSON_ESCAPE_CHAR_FORMFEED:
                case JSON_ESCAPE_CHAR_NEW_LINE:
                case JSON_ESCAPE_CHAR_CARRIAGE_RETURN:
                case JSON_ESCAPE_CHAR_TAB:
                    break;
                case JSON_ESCAPE_CHAR_UNICODE:
                    for (uint8_t i = 0; i < JSON_ESCAPE_CHAR_UNICODE_COUNT - 1; i++) {
                        // Push the escape character as we go through pulling the rest of the escape characters in the
                        //  sequence
                        PUSH_NEXT_CHAR(escape_char);

                        // Pull the rest of the 3 characters that would correspond to the unicode escape sequence
                        PULL_NEXT_CHAR(&escape_char);

                        if (strchr(hex_chars, escape_char) == NULL) {
                            // The hex character that was passed through to the strchr wasn't found in the set of hex
                            //  characters that we have defined, so it isn't a valid hex character.
                            log_error("An invalid hex character was found in a unicode escape sequence.");
                            return JSON_ERR_INCORRECT_JSON_FORMAT;
                        }
                    }
                    break;
                default:
                    log_error("An invalid escape sequence was found when walking through the string.");
                    return JSON_ERR_INCORRECT_JSON_FORMAT;
                }

                PUSH_NEXT_CHAR(escape_char);
            }

            if (*current_char == JSON_CHARACTER_QUOTE) {
                log_debug("Reached the end of the string we were walking through. Disabling string mode.");

                // Since we have reached the end of the string that we were running through, set the in_string variable
                //  to be false. This way the next iteration won't be processed as a string.
                in_string = false;
            }

            PUSH_NEXT_CHAR(*current_char);
        }
        else {
            if (
                *current_char == JSON_CHARACTER_SPACE           || *current_char == JSON_CHARACTER_NEW_LINE       ||
                *current_char == JSON_CHARACTER_CARRIAGE_RETURN || *current_char == JSON_CHARACTER_HORIZONTAL_TAB
            ) {
                // We found a space, ignore and continue. This essentially minifies the JSON object that we are
                //  traversing in order to be more efficient with the space we need to operate.
                continue;
            }

            if (*current_char == JSON_CHARACTER_OBJ_BRACKET) {
                if (nesting_obj_count == 255) {
                    log_error("Are you serious... You have more than 255 nested objects in your JSON? What are you "
                              "doing?");
                    return JSON_ERR_TOO_MANY_NESTED_OBJECTS;
                }
                nesting_obj_count++;
                log_trace("Incrementing nested object count by 1, now at \"%d\"", nesting_obj_count);
            }

            if (*current_char == JSON_CHARACTER_OBJ_CLOSE_BRACKET) {
                if (nesting_obj_count == 0) {
                    log_debug("Reached the end of the object we were traversing.");

                    break;
                }
                else {
                    nesting_obj_count--;
                    log_trace("Decrementing nested object count by 1, now at \"%d\"", nesting_obj_count);
                }
            }

            // At this point we have properly accounted for spaces in the JSON object and properly counted the nested
            //  objects that are present.
            if (*current_char == JSON_CHARACTER_QUOTE) {
                log_debug("Reached the beginning of a string. Enabling string mode.");

                // Since we have reached the end of the string that we were running through, set the in_string variable
                //  to be false. This way the next iteration won't be processed as a string.
                in_string = true;
            }

            PUSH_NEXT_CHAR(*current_char);
        }
    }

    // We push the last character in the object that we ran through then return that we were successful
    PUSH_NEXT_CHAR(*current_char);
    return JSON_ERR_NO_ERR;
}


/**
 * @brief This function is meant to be called that at a point in the JSON file where the last character that was pulled
 * before this function was called was the character denoting the end of the value for the previous parameter. For
 * example, if the last parameter was a string, the last character pulled was a ", if the last parameter was an object,
 * the last character pulled was a }. This is a straight forward function, it just goes through processing the next
 * characters coming in until we get to the start of the value type that was passed through to the function.
 *
 * A few things that this function is specifically looking out for and when it will return an error:
 *  - The EOF Character: We need to error out if we reach the end of the file prematurely.
 *  - Object Close Bracket: If we reach the end of the object then we can be sure that the value type we are looking for
 *    isn't going to be found.
 *  - Array Close Bracket: If we reach the end of the array, then we can be sure that the value type we are looking for
 *    isn't going to be found.
 *  - Commas: We need to make sure that the first comma is considered okay because it could be denoting the separation
 *    between parameters or array values, but any commas after the first one would indicate incorrect formatting in the
 *    file.
 *  - Non-space characters: Any character that isn't an opening quotation is problematic because it wouln't be expected,
 *    so we need to error out in this case.
 *
 * NOTE: BMG (Jan. 25, 2022) It is VERY IMPORTANT to note that this function is going to expect the current_char pointer
 * variable to contain the character that was last pulled from the previous function that worked with it. There should
 * be no character pulls between the last function that used the current_char variable in order for this function to
 * work properly.
 *
 * @param get_char_func 
 * @param search_type 
 * @return JSON_ERR_ENUM 
 */
static JSON_ERR_ENUM json_walk_through_object_or_array_to_next_value(json_get_next_character get_char_func,
                                                                     JSON_TYPES_ENUM search_type,
                                                                     int *current_char) {
    // We need to store the string with the json type somewhere
    char search_type_string[JSON_STRING_ARRAY_SIZE];
    switch (search_type) {
    case JSON_TYPES_ARRAY:
        strcpy(search_type_string, JSON_STRING_ARRAY);
        break;
    case JSON_TYPES_OBJECT:
        strcpy(search_type_string, JSON_STRING_OBJECT);
        break;
    case JSON_TYPES_STRING:
        strcpy(search_type_string, JSON_STRING_STRING);
        break;
    case JSON_TYPES_NUMBER:
        strcpy(search_type_string, JSON_STRING_NUMBER);
        break;
    case JSON_TYPES_TRUE:
        strcpy(search_type_string, JSON_STRING_TRUE);
        break;
    case JSON_TYPES_FALSE:
        strcpy(search_type_string, JSON_STRING_FALSE);
        break;
    case JSON_TYPES_NULL:
        strcpy(search_type_string, JSON_STRING_NULL);
        break;
    default:
        log_error("An invalid type was provided to the json_walk_through_object_or_array_to_next_value function.");
        return JSON_ERR_INVALID_JSON_TYPE_PROVIDED;
    }

    bool expect_comma = true;
    if (*current_char == JSON_CHARACTER_COMMA) {
        // We want to check to see if the last character that was pulled was a comma, to see if we should be expecting a
        //  comma.
        expect_comma = false;
    }

    int comma_count = 0;
    while (true) {
        PULL_NEXT_CHAR(current_char);

        if (*current_char == EOF) {
            // We found an EOF character when we didn't expect it. Return a failure for this function.
            // At this point we have either:
            //  - Gone through all the parameters in the object and not found the one we are interested in, or
            //  - Reached the EOF prematurely when we were expecting it.
            // Either way, the JSON format for the file provided is wrong.
            log_error("Reached end of file before the desired JSON type (%s) was found.", search_type_string);
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }

        if (expect_comma && comma_count == 0 && *current_char == JSON_CHARACTER_COMMA) {
            // We only want to make sure we ignore a comma if there is 1, if there is more than 1 there is a problem
            //  with the format of the file.
            comma_count++;
            log_trace("Found a comma denoting the end of the previous parameter.");
            continue;
        }

        if (*current_char == JSON_CHARACTER_OBJ_CLOSE_BRACKET) {
            // We have found the end of the object that we are looking through, meaning that the paramter we are
            //  interested doesn't exist in this object.
            log_error("Reached end of object before the desired JSON type (%s) was found.", search_type_string);
            return JSON_ERR_PARAMETER_MISSING_FROM_OBJECT;
        }

        if (*current_char == JSON_CHARACTER_ARR_CLOSE_BRACKET) {
            // We have found the end of the object that we are looking through, meaning that the paramter we are
            //  interested doesn't exist in this object.
            log_error("Reached end of array before the desired JSON type (%s) was found.", search_type_string);
            return JSON_ERR_PARAMETER_MISSING_FROM_OBJECT;
        }

        if (comma_count != 0 && json_get_type_from_character(*current_char) == search_type) {
            // We need to make sure that we have actually seen the comma before we get to the json type.
            // We have successfully found a character indicating the beginning of the json type that we are expecting.
            log_trace("Successfully found the beginning of the requested search type.");
            return JSON_ERR_NO_ERR;
        }

        if (
            *current_char != JSON_CHARACTER_SPACE           && *current_char != JSON_CHARACTER_NEW_LINE &&
            *current_char != JSON_CHARACTER_CARRIAGE_RETURN && *current_char != JSON_CHARACTER_HORIZONTAL_TAB
        ) {
            // If the current character doesn't match any of the valid space characters as defined in the JSON
            //  specification, throw an error.
            log_error("Found an unexpected non-space character '%c' before the desired JSON type (%s) was found.",
                      *current_char,
                      search_type_string);
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }
    }
}

/**
 * @brief Straight forward, this just keeps calling the the get_char_func until it gets to the beginning of the array
 * inside the parameter named.
 *
 * NOTE: This function should always be called when the current position is at the start of an object. i.e. The last
 * character to be gotten from the get_char_func should be a '{' character.
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param recursive_parameter_name The name of the recursive parameter.
 * @return JSON_ERR_ENUM
 */
static JSON_ERR_ENUM json_go_to_start_of_array_of_recursive_parameter_name(json_get_next_character get_char_func,
                                                                           char *recursive_parameter_name,
                                                                           int *current_char) {
    uint8_t i;
    JSON_ERR_ENUM err_holder = JSON_ERR_NO_ERR;

    // Create an array for the current parameter that we are reading and make sure that we zero it out
    char current_parameter[MAX_PARAMETER_ARRAY_SIZE];
    for (i = 0; i < MAX_PARAMETER_ARRAY_SIZE; i++) current_parameter[i] = 0;

    PULL_NEXT_CHAR(current_char);

    // We don't know how many parameters are going to be in the object so this must be a while loop
    while (true) {
        /**
         * At this point in this loop, we are at a position in the JSON file where the next character we expect is the
         * start of the opening quote for the parameter name.
         *
         * Maybe add a TODO: for checking what the last character pulled before we got here is to make sure it isn't an
         * unexpected character. Additionally, we can use this information to inform us on whether or not we should be
         * expecting a comma in the characters that we are going to go through.
         */
        err_holder = json_walk_through_object_or_array_to_next_value(get_char_func, JSON_TYPES_STRING, current_char);
        if (err_holder != JSON_ERR_NO_ERR) {
            log_error("Unable to find parameter \"%s\"", recursive_parameter_name);
            return err_holder;
        }

        // At this point, the next characters that we pull will be the parameter string until we get to the next
        //  quotation. We subtract 1 from the parameter string size because the last character will always be a null
        //  terminating character
        for (i = 0; i < MAX_PARAMETER_ARRAY_SIZE; i++) {
            PULL_NEXT_CHAR(current_char);

            if (*current_char == JSON_CHARACTER_QUOTE) {
                // Set the null terminating character when we get to the end of the quote
                current_parameter[i] = 0;
                break;
            }
            else if (*current_char != EOF) {
                current_parameter[i] = *current_char;
            }
            else {
                // At this point, we have prematurely reached an EOF character in the parameter name, incorrect JSON
                //  format for the file.
                log_error("Reached EOF prematurely. Incrrect JSON format.");
                return JSON_ERR_INCORRECT_JSON_FORMAT;
            }
        }
        if (i == MAX_PARAMETER_ARRAY_SIZE) {
            // We ran through the size of the array and never ran into end quotation, therefore we know that the
            //  parameter name was too large.
            current_parameter[MAX_PARAMETER_ARRAY_SIZE - 1] = 0;
            log_error("Ran into a parameter that was larger than the maximum %d characters for us to process (%s). "
                      "Parameter too large.",
                      MAX_PARAMETER_STRING_SIZE,
                      current_parameter);
            return JSON_ERR_PARAMETER_TOO_LARGE;
        }

        // At this point, we have a parameter name in the current_parameter character array. So now we will check if we
        //  have found the correct parameter in the json file.
        if (strcmp(current_parameter, recursive_parameter_name) == 0) {
            log_debug("Successfully found parameter: %s", current_parameter);
            break;
        }
        else {
            log_trace("Found non-matching parameter: %s", current_parameter);

            // self-explanatory
            json_walk_to_start_of_parameter_value(get_char_func, current_char);

            switch (json_get_type_from_character(*current_char)) {
            case JSON_TYPES_ARRAY:
                json_walk_to_end_of_array(get_char_func, current_char);
                break;;
            case JSON_TYPES_STRING:
                json_walk_to_end_of_string(get_char_func, current_char);
                break;
            case JSON_TYPES_OBJECT:
                json_walk_to_end_of_object(get_char_func, current_char);
                break;
            case JSON_TYPES_TRUE:
                json_walk_to_end_of_true(get_char_func, current_char);
                break;
            case JSON_TYPES_FALSE:
                json_walk_to_end_of_false(get_char_func, current_char);
                break;
            case JSON_TYPES_NULL:
                json_walk_to_end_of_null(get_char_func, current_char);
                break;
            case JSON_TYPES_NUMBER:
                json_walk_to_end_of_number(get_char_func, current_char);
                break;
            default:
                break;
            }

            continue;
        }
    }

    // self-explanatory
    json_walk_to_start_of_parameter_value(get_char_func, current_char);

    // At this point we know that we are at the beginning of the value of the parameter we are interested in, so we need
    //  to make sure that we return corresponding errors for wrong value types in that parameter.
    switch (json_get_type_from_character(*current_char)) {
    case JSON_TYPES_ARRAY:
        return JSON_ERR_NO_ERR;

    case JSON_TYPES_STRING:
        log_error("The type of the value for parameter \"%s\" isn't the array that we were expecting. Found a String "
                  "instead.",
                  recursive_parameter_name);
        return JSON_ERR_WRONG_RECURSIVE_PARAMETER_VALUE_TYPE;
    case JSON_TYPES_OBJECT:
        log_error("The type of the value for parameter \"%s\" isn't the array that we were expecting. Found an Object "
                  "instead.",
                  recursive_parameter_name);
        return JSON_ERR_WRONG_RECURSIVE_PARAMETER_VALUE_TYPE;
    case JSON_TYPES_TRUE:
        log_error("The type of the value for parameter \"%s\" isn't the array that we were expecting. Found a true "
                  "constant instead.",
                  recursive_parameter_name);
        return JSON_ERR_WRONG_RECURSIVE_PARAMETER_VALUE_TYPE;
    case JSON_TYPES_FALSE:
        log_error("The type of the value for parameter \"%s\" isn't the array that we were expecting. Found a false "
                  "constant instead.",
                  recursive_parameter_name);
        return JSON_ERR_WRONG_RECURSIVE_PARAMETER_VALUE_TYPE;
    case JSON_TYPES_NULL:
        log_error("The type of the value for parameter \"%s\" isn't the array that we were expecting. Found a null "
                  "constant instead.",
                  recursive_parameter_name);
        return JSON_ERR_WRONG_RECURSIVE_PARAMETER_VALUE_TYPE;
    case JSON_TYPES_NUMBER:
        log_error("The type of the value for parameter \"%s\" isn't the array that we were expecting. Found a Number "
                  "instead.",
                  recursive_parameter_name);
        return JSON_ERR_WRONG_RECURSIVE_PARAMETER_VALUE_TYPE;
    default:
        log_error("The type of the value for parameter \"%s\" didn't match any type defined in the JSON specification.",
                  recursive_parameter_name);
        return JSON_ERR_INCORRECT_JSON_FORMAT;
    }
}

static JSON_TYPES_ENUM json_get_type_from_character(int current_char) {
    switch (current_char) {
    case JSON_CHARACTER_ARR_BRACKET:
        return JSON_TYPES_ARRAY;

    case JSON_CHARACTER_QUOTE:
        return JSON_TYPES_STRING;

    case JSON_CHARACTER_OBJ_BRACKET:
        return JSON_TYPES_OBJECT;

    case JSON_CHARACTER_TRUE_BEGINNING:
        return JSON_TYPES_TRUE;

    case JSON_CHARACTER_FALSE_BEGINNING:
        return JSON_TYPES_FALSE;

    case JSON_CHARACTER_NULL_BEGINNING:
        return JSON_TYPES_NULL;

    case JSON_CHARACTER_NUMBER_NEG:
    case JSON_CHARACTER_NUMBER_ZERO:
    case JSON_CHARACTER_NUMBER_ONE:
    case JSON_CHARACTER_NUMBER_TWO:
    case JSON_CHARACTER_NUMBER_THREE:
    case JSON_CHARACTER_NUMBER_FOUR:
    case JSON_CHARACTER_NUMBER_FIVE:
    case JSON_CHARACTER_NUMBER_SIX:
    case JSON_CHARACTER_NUMBER_SEVEN:
    case JSON_CHARACTER_NUMBER_EIGHT:
    case JSON_CHARACTER_NUMBER_NINE:
        return JSON_TYPES_NUMBER;

    default:
        return JSON_TYPES_NOT_VALID_TYPE;
    }
}

/**
 * @brief At this point, we have successfully traversed the JSON file to the point where the last character that we
 * pulled from the json was the ending quotation for the parameter name. So we are going to continue traversing through
 * the JSON file while ignoring a number of characters until we get to the beginning of the value that corresponds to
 * the parameter that we are currently working with. The following values are the ones we are going to be ignoring:
 *  - Any space characters as defined by the JSON file spec
 *  - The single colon that we are expecting between the parameter name and the value
 *
 * NOTE: BMG (Jan. 14, 2022) The last character that is pulled will remain in the current_char pointer that was passed 
 * through to this function.
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param current_char A pointer to the current_char variable that is being used in the function that is calling this
 * @return JSON_TYPES_ENUM 
 */
static JSON_ERR_ENUM json_walk_to_start_of_parameter_value(json_get_next_character get_char_func, int *current_char) {
    uint8_t colon_count = 0;

    while (true) {
        PULL_NEXT_CHAR(current_char);

        if (*current_char == EOF) {
            // We found an EOF character when we didn't expect it. Return a failure for this function.
            // At this point we have either:
            //  - Gone through all the parameters in the object and not found the one we are interested in, or
            //  - Reached the EOF prematurely when we were expecting it.
            // Either way, the JSON format for the file provided is wrong.
            log_error("Unable to find the value for the parameter, reached the end of the file before the value was "
                      "found.");
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }

        if (
            *current_char == JSON_CHARACTER_SPACE           || *current_char == JSON_CHARACTER_NEW_LINE       ||
            *current_char == JSON_CHARACTER_CARRIAGE_RETURN || *current_char == JSON_CHARACTER_HORIZONTAL_TAB
        ) {
            // We found a space, ignore and continue
            continue;
        }
        if (colon_count == 0) {
            // We need to make sure that we only go through this case once when we find the colon we are expecting here
            if (*current_char == JSON_CHARACTER_COLON) {
                // We found the colon that we were talking about!
                colon_count++;
                continue;
            }
        }

        // If we have gotten to this point it's because we have found a disallowed character
        if (colon_count == 0) {
            // We got to a character that is not a space or a colon before we were expecting
            log_error("A non-space, non-colon character was found before it was expected.");
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }
        else {
            // At this point we have gotten to what we presume is the beginning of the parameter value
            break;
        }
    }

    return JSON_ERR_NO_ERR;
}

/**
 * @brief Self explanatory, this function is assuming that the last character that was pulled before this function is
 * called was the quotation that denotes the start of the string that is being defined. It will walk through the
 * function until it pulls the quotation character that denotes the end of the defined string. Then, it will pull one
 * more character to keep the behavior of all the "walk" functions consistent.
 *
 * NOTE: BMG (Jan. 14, 2022) The last character that is pulled will remain in the current_char pointer that was passed 
 * through to this function.
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param current_char A pointer to the current_char variable that is being used in the function that is calling this
 * @return JSON_TYPES_ENUM 
 */
static JSON_ERR_ENUM json_walk_to_end_of_string(json_get_next_character get_char_func, int *current_char) {
    int escape_char = 0;
    char hex_chars[] = JSON_ESCAPE_CHAR_UNICODE_HEX_CHARS;

    while (true) {
        PULL_NEXT_CHAR(current_char);

        if (*current_char == EOF) {
            // We found an EOF character when we didn't expect it. Return a failure for this function, the JSON format
            //  for the file provided is wrong.
            log_error("Reached end of file before we found the end of the string.");
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }

        if (*current_char == JSON_CHARACTER_NEW_LINE) {
            // JSON strings cannot contain a new line character, they can only span a single line.
            log_error("Reached a new line character, strings are supposed to be a single line only.");
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }

        if (*current_char == JSON_CHARACTER_ESCAPE) {
            // If we run into an escape character, we will pull a second character to see what it is.
            PULL_NEXT_CHAR(&escape_char);

            switch (escape_char) {
            case JSON_ESCAPE_CHAR_QUOTE:
            case JSON_ESCAPE_CHAR_BACKSLASH:
            case JSON_ESCAPE_CHAR_FORWARDSLASH:
            case JSON_ESCAPE_CHAR_BACKSPACE:
            case JSON_ESCAPE_CHAR_FORMFEED:
            case JSON_ESCAPE_CHAR_NEW_LINE:
            case JSON_ESCAPE_CHAR_CARRIAGE_RETURN:
            case JSON_ESCAPE_CHAR_TAB:
                break;
            case JSON_ESCAPE_CHAR_UNICODE:
                for (uint8_t i = 0; i < JSON_ESCAPE_CHAR_UNICODE_COUNT - 1; i++) {
                    // Pull the rest of the 3 characters that would correspond to the unicode escape sequence
                    PULL_NEXT_CHAR(&escape_char);

                    if (strchr(hex_chars, escape_char) == NULL) {
                        // The hex character that was passed through to the strchr wasn't found in the set of hex
                        //  characters that we have defined, so it isn't a valid hex character.
                        log_error("An invalid hex character was found in a unicode escape sequence.");
                        return JSON_ERR_INCORRECT_JSON_FORMAT;
                    }
                }
                break;
            default:
                log_error("An invalid escape sequence was found when walking through the string.");
                return JSON_ERR_INCORRECT_JSON_FORMAT;
            }
        }

        if (*current_char == JSON_CHARACTER_QUOTE) {
            log_debug("Reached the end of the string we were walking through.");

            log_trace("Pulling an extra character for consistency in walk functions.");
            PULL_NEXT_CHAR(current_char);

            return JSON_ERR_NO_ERR;
        }
    }
}

/**
 * @brief Self explanatory, this function is assuming that the last character that was pulled before this function is
 * called was the first character of the defined number that we are trying to walk through. It will walk through the
 * value until it pulls a character that couldn't be a part of the number value. There is no way around this for this
 * function.
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param current_char A pointer to the current_char variable that is being used in the function that is calling this
 * @return JSON_TYPES_ENUM 
 */
static JSON_ERR_ENUM json_walk_to_end_of_number(json_get_next_character get_char_func, int *current_char) {
    // Because we know that the start of the number value has been read into current char (how would we know that we are
    //  walking a number otherwise?) so we don't have a pull a new character at the beginning of this function.
    if (*current_char == JSON_CHARACTER_NUMBER_NEG) {
        // If the first character we pull is a negative sign then we take note of that and then pull the next character
        //  to be processed.
        log_trace("We know that this is a negative number.");
        PULL_NEXT_CHAR(current_char);
    }

    #define PULL_DIGITS_WHILE_LOOP \
        while (true) {                                                                                          \
            if (                                                                                                \
                *current_char == JSON_CHARACTER_NUMBER_ONE   || *current_char == JSON_CHARACTER_NUMBER_TWO   || \
                *current_char == JSON_CHARACTER_NUMBER_THREE || *current_char == JSON_CHARACTER_NUMBER_FOUR  || \
                *current_char == JSON_CHARACTER_NUMBER_FIVE  || *current_char == JSON_CHARACTER_NUMBER_SIX   || \
                *current_char == JSON_CHARACTER_NUMBER_SEVEN || *current_char == JSON_CHARACTER_NUMBER_EIGHT || \
                *current_char == JSON_CHARACTER_NUMBER_NINE  || *current_char == JSON_CHARACTER_NUMBER_ZERO     \
            ) {                                                                                                 \
                PULL_NEXT_CHAR(current_char);                                                                   \
                continue;                                                                                       \
            }                                                                                                   \
            else {                                                                                              \
                log_trace("Ran into a non-digit character when trying to read through the number type, continuing the " \
                          "walk.");                                                                             \
                break;                                                                                          \
            }                                                                                                   \
        }

    // Parsing the whole umber portion of the number defined
    if (*current_char == JSON_CHARACTER_NUMBER_ZERO) {
        log_trace("We know that this json number is a either a fraction or a zero.");
        PULL_NEXT_CHAR(current_char);
    }
    else if (
        *current_char == JSON_CHARACTER_NUMBER_ONE   || *current_char == JSON_CHARACTER_NUMBER_TWO   ||
        *current_char == JSON_CHARACTER_NUMBER_THREE || *current_char == JSON_CHARACTER_NUMBER_FOUR  ||
        *current_char == JSON_CHARACTER_NUMBER_FIVE  || *current_char == JSON_CHARACTER_NUMBER_SIX   ||
        *current_char == JSON_CHARACTER_NUMBER_SEVEN || *current_char == JSON_CHARACTER_NUMBER_EIGHT ||
        *current_char == JSON_CHARACTER_NUMBER_NINE
    ) {
        // Once we know that the first character of the whole number is not a zero, we can continue to look through the
        //  rest of the numbers.
        PULL_NEXT_CHAR(current_char);

        PULL_DIGITS_WHILE_LOOP;
    }
    else {
        log_error("An invalid number was read.");
        return JSON_ERR_INCORRECT_JSON_FORMAT;
    }

    // Parsing the decimal portion of the number defined.
    if (*current_char == JSON_CHARACTER_NUMBER_DECIMAL) {
        log_trace("We have confirmed that the number has a decimal portion that we are going to walk through");
        PULL_NEXT_CHAR(current_char);

        PULL_DIGITS_WHILE_LOOP;
    }

    // Parsing the exponent portion of the number defined.
    if (*current_char == JSON_CHARACTER_NUMBER_EXP_CAP || *current_char == JSON_CHARACTER_NUMBER_EXP_LOW) {
        log_trace("We have confirmed that the number has an exponent portion that we are going to walk through");
        PULL_NEXT_CHAR(current_char);

        if (*current_char == JSON_CHARACTER_NUMBER_NEG || *current_char == JSON_CHARACTER_NUMBER_PLUS) {
            log_trace("The number in the exponent has a positive or a negative sign preceeding it.");
            PULL_NEXT_CHAR(current_char);
        }

        PULL_DIGITS_WHILE_LOOP;
    }

    // There is no way for us to know whether or not there is an error in the format of the number because there are no
    //  clear borders to the defined value, and it can be of basically any type.
    return JSON_ERR_NO_ERR;
}

/**
 * @brief Self explanatory, this function is assuming that the last character that was pulled before this function is
 * called is called was the open object character that denotes the start of the object that is being defind. It will
 * walk through the function until it pulls the close object character that denotes the end of the defined object. Then,
 * it will pull one more character to keep the behavior of all the "walk" functions consistent.
 *
 * NOTE: BMG (Jan. 14, 2022) The last character that is pulled will remain in the current_char pointer that was passed 
 * through to this function.
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param current_char A pointer to the current_char variable that is being used in the function that is calling this
 * @return JSON_TYPES_ENUM 
 */
static JSON_ERR_ENUM json_walk_to_end_of_object(json_get_next_character get_char_func, int *current_char) {
    // I can't imagine that that we will run into more than 255 nested objects, or at least I hope we don't.
    uint8_t nesting_obj_count = 0; 

    while (true) {
        PULL_NEXT_CHAR(current_char);

        if (*current_char == EOF) {
            // We found an EOF character when we didn't expect it. Return a failure for this function, the JSON format
            //  for the file provided is wrong.
            log_error("Reached end of file before we found the end of the object.");
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }

        if (*current_char == JSON_CHARACTER_OBJ_BRACKET) {
            if (nesting_obj_count == 255) {
                log_error("Are you serious... You have more than 255 nested objects in your JSON? What are you doing?");
                return JSON_ERR_TOO_MANY_NESTED_OBJECTS;
            }
            nesting_obj_count++;
            log_trace("Incrementing nested object count by 1, now at \"%d\"", nesting_obj_count);
            continue;
        }

        if (*current_char == JSON_CHARACTER_OBJ_CLOSE_BRACKET) {
            if (nesting_obj_count == 0) {
                log_debug("Reached the end of the object we were traversing.");

                log_trace("Pulling an extra character for consistency in walk functions.");
                PULL_NEXT_CHAR(current_char);

                return JSON_ERR_NO_ERR;
            }
            else {
                nesting_obj_count--;
                log_trace("Decrementing nested object count by 1, now at \"%d\"", nesting_obj_count);
                continue;
            }
        }
    }
}

/**
 * @brief Self explanatory, this function is assuming that the last character that was pulled before this function is
 * called is called was the open array character that denotes the start of the array that is being defind. It will walk
 * through the function until it pulls the close array character that denotes the end of the defined array. Then, it
 * will pull one more character to keep the behavior of all the "walk" functions consistent.
 *
 * NOTE: BMG (Jan. 14, 2022) The last character that is pulled will remain in the current_char pointer that was passed 
 * through to this function.
 *
 * @param get_char_func callback to get the next character in the json object.
 * @param current_char A pointer to the current_char variable that is being used in the function that is calling this
 * @return JSON_TYPES_ENUM 
 */
static JSON_ERR_ENUM json_walk_to_end_of_array(json_get_next_character get_char_func, int *current_char) {
    // I can't imagine that that we will run into more than 255 nested objects, or at least I hope we don't.
    uint8_t nesting_arr_count = 0; 

    while (true) {
        PULL_NEXT_CHAR(current_char);

        if (*current_char == EOF) {
            // We found an EOF character when we didn't expect it. Return a failure for this function, the JSON format
            //  for the file provided is wrong.
            log_error("Reached end of file before we found the end of the array.");
            return JSON_ERR_INCORRECT_JSON_FORMAT;
        }

        if (*current_char == JSON_CHARACTER_OBJ_BRACKET) {
            if (nesting_arr_count == 255) {
                log_error("Are you serious... You have more than 255 nested arrays in your JSON? What are you doing?");
                return JSON_ERR_TOO_MANY_NESTED_OBJECTS;
            }
            nesting_arr_count++;
            log_trace("Incrementing nested arrays count by 1, now at \"%d\"", nesting_arr_count);
            continue;
        }

        if (*current_char == JSON_CHARACTER_OBJ_CLOSE_BRACKET) {
            if (nesting_arr_count == 0) {
                log_debug("Reached the end of the array we were traversing.");

                log_trace("Pulling an extra character for consistency in walk functions.");
                PULL_NEXT_CHAR(current_char);

                return JSON_ERR_NO_ERR;
            }
            else {
                nesting_arr_count--;
                log_trace("Decrementing nested array count by 1, now at \"%d\"", nesting_arr_count);
                continue;
            }
        }
    }
}

static JSON_ERR_ENUM json_walk_to_end_of_true(json_get_next_character get_char_func, int *current_char) {
    int i;
    const char true_string_arr[] = JSON_TRUE_STRING;
    const int true_string_cnt = strlen(true_string_arr);

    for (i = 0; i < true_string_cnt; i++) {
        if (*current_char == true_string_arr[i]) {
            PULL_NEXT_CHAR(current_char);
        }
        else {
            break;
        }
    }

    if (i == true_string_cnt) {
        log_debug("Successfully walked through the true value.");

        log_trace("Pulling an extra character for consistency in walk functions.");
        PULL_NEXT_CHAR(current_char);

        return JSON_ERR_NO_ERR;
    }
    else {
        log_error("Wasn't successfully able to walk through the true value.");
        return JSON_ERR_INCORRECT_JSON_FORMAT;
    }
}

static JSON_ERR_ENUM json_walk_to_end_of_false(json_get_next_character get_char_func, int *current_char) {
    int i;
    const char false_string_arr[] = JSON_FALSE_STRING;
    const int false_string_cnt = strlen(false_string_arr);

    for (i = 0; i < false_string_cnt; i++) {
        if (*current_char == false_string_arr[i]) {
            PULL_NEXT_CHAR(current_char);
        }
        else {
            break;
        }
    }

    if (i == false_string_cnt) {
        log_debug("Successfully walked through the false value.");

        log_trace("Pulling an extra character for consistency in walk functions.");
        PULL_NEXT_CHAR(current_char);

        return JSON_ERR_NO_ERR;
    }
    else {
        log_error("Wasn't successfully able to walk through the false value.");
        return JSON_ERR_INCORRECT_JSON_FORMAT;
    }
}

static JSON_ERR_ENUM json_walk_to_end_of_null(json_get_next_character get_char_func, int *current_char) {
    int i;
    const char null_string_arr[] = JSON_NULL_STRING;
    const int null_string_cnt = strlen(null_string_arr);

    for (i = 0; i < null_string_cnt; i++) {
        if (*current_char == null_string_arr[i]) {
            PULL_NEXT_CHAR(current_char);
        }
        else {
            break;
        }
    }

    if (i == null_string_cnt) {
        log_debug("Successfully walked through the null value.");

        log_trace("Pulling an extra character for consistency in walk functions.");
        PULL_NEXT_CHAR(current_char);

        return JSON_ERR_NO_ERR;
    }
    else {
        log_error("Wasn't successfully able to walk through the null value.");
        return JSON_ERR_INCORRECT_JSON_FORMAT;
    }
}

/**
 * @brief This is a super straight forward function that prints the coordinates that were given to it in a comma
 * separated format without any spaces.
 *
 * For example, if the arrray [1, 2, 3, 4] is provided to this function, the following string would be printed.
 *  - "1,2,3,4"
 *
 * @param object_index_arr An array of indexes that indicates the location of the object that we're interested in
 *                         getting.
 * @param object_index_cnt The size of the index array.
 */
static int json_sprint_comma_separated_coordinates(char *starting_ptr, uint8_t *object_index_arr, int object_index_cnt) {
    int i;
    int pointer_adder = 0;

    // Recurse through every digit in the array except for the last one. Print each digit with a comma afterwards.
    for (i = 0; i < object_index_cnt - 1; i++) {
        pointer_adder += sprintf(starting_ptr + pointer_adder, "%d,", object_index_arr[i]);
    }
    // At this point i would point to the last digit in the array, so we print the last digit in the array.
    pointer_adder += sprintf(starting_ptr + pointer_adder, "%d", object_index_arr[i]);

    return pointer_adder;
}
