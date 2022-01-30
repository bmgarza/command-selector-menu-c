#ifndef JSON_PROCESS_UTILS_H
#define JSON_PROCESS_UTILS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NOTE: BMG (Dec. 28, 2021) The main objective of this set of functions is to make it more managable to process a json
 * object of arbitrary size in an environment where ram could potentially be constrained, but where the json object
 * could have an array of arbitrary size.
 *
 * The basic idea is that, when we are handing an array, we are going to use the json_get_sub_json_object to get the
 * object at a specific index and place it in a buffer that could then be processed by something like microjson.
 * Additionally, it should take some VA arguments afterwards that would contain the set of parameters in the json object
 * that should be ignored. The idea with this is that, if there is a recursive json object that we are dealing with, we
 * don't want to bulk up the subobject that is going to be provided to the json object deserializer and subsequently
 * take up more memory than is necessary.
 *
 * Say that we have some object, jotest, that potentially contains an array of jotest objects too.
 *  jotest:
 *    name: string
 *    arr: jotest[]
 *
 * ─ │ ┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼
 *  ┌─obj a
 *  │ └─obj b
 *  ├─obj c
 *  ├─obj d
 *  │ ├─obj e
 *  │ └─obj f
 *  │   └─obj g
 *  └─obj h
 *
 * In this case we would make sure that we go through the first layer, ignoring any recursive objects by name, we we
 * would be able to access the following objects in the first pass:
 *  - obj a
 *  - obj c
 *  - obj d
 *  - obj h
 *
 * If at this point we would want to continue by looking within the array in "obj d", we would then be able to access
 * the following objects:
 *  - obj e
 *  - obj f
 *
 * The idea would then be that the json object is stored in either a file or eeprom or something like that.
 */

/**
 * @brief A simple function that reads the next character to a pointer and returns a boolean indicating success.
 *
 * This is basically just going to be a simple function that reads the next character in the json object, regardless of
 * the platform that this code is being run under. The template for this function exists in order to make sure that the
 * functions here are able to be used in either an embedded or desktop environment.
 *
 * @param uint8_t A pointer to where the next character is going to be written to.
 * @return Returns a boolean indicating whether the function was successful or not.
 */
typedef bool (*json_get_next_character)(int *);

/**
 * @brief A simple function that writes the next character to a buffer and returns a boolean indicating success.
 *
 * This is basically just going to be a simple function that writes the next character into the json buffer, regardless
 * of the platform that this code is being run under. The template for this function exists in order to make sure that
 * the functions here are able to be used in either an embedded or desktop environment.
 *
 * @param uint8_t The next character is going to be written to the buffer.
 * @return Returns a boolean indicating whether the function was successful or not.
 */
typedef bool (*json_set_next_character)(int);

typedef enum {
    JSON_ERR_NO_ERR,
    JSON_ERR_OBJECT_INDEX_ARRAY_EMPTY,
    JSON_ERR_INCORRECT_JSON_FORMAT,
    JSON_ERR_PARAMETER_MISSING_FROM_OBJECT,
    JSON_ERR_WRONG_RECURSIVE_PARAMETER_VALUE_TYPE,
    JSON_ERR_INDEX_NOT_FOUND,
    JSON_ERR_INVALID_JSON_TYPE_PROVIDED,
    JSON_ERR_TOO_MANY_NESTED_OBJECTS,
    JSON_ERR_PARAMETER_TOO_LARGE,
    JSON_ERR_GET_FUNCTION_FAILURE,
    JSON_ERR_SET_FUNCTION_FAILURE,
    JSON_ERR_NOT_FINISHED,
    JSON_ERR_COUNT,
} JSON_ERR_ENUM;

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
                                        char *recursive_parameter_name);

#ifdef __cplusplus
}
#endif

// JSON_PROCESS_UTILS_H
#endif
