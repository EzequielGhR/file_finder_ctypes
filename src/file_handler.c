#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <dirent.h>
#include "file_handler.h"


// <<< Forward declarations >>>
char*                   case_strchr(char* s, char c);
Bool                    concat_path(char* dest_path, char* to_concat);
Bool                    is_substring(char* src, char* sub_str, char* buffer, unsigned int buffer_size);
Bool                    is_in_file_lines(char* filepath, char* sub_str, char* buffer, unsigned int buffer_size);
struct file_data*       allocate_new_file_data(char* file_path, char* filename, char* file_content_slice);
void                    free_pointer(void* ptr);


// <<< Main logic >>>
/**
 @brief Find a file in start_path and child paths by filename
 @param filename: A substring of the name of the file to find
 @param start_path: The starting path for searching the file
 @param fmatches: structure for holding file matches previously allocated
 */
struct file_matches* find_by_name(const char* filename, const char* start_path,
                                  struct file_matches* fmatches, Bool recursive) {
    printf("INFO: Searching by name with params: %s, %s\n", filename, start_path);
    
    // Pre checks
    if (strlen(start_path) > MAX_PATH_LENGTH) {
        printf("ERROR: Start path is too big\n");
        return fmatches;
    }

    if (strlen(filename) > MAX_FILENAME_LENGTH) {
        printf("ERROR: Filename is too big\n");
        return fmatches;
    }

    if (fmatches == NULL) {
        printf("ERROR: File matches pointer cannot be null");
        return NULL;
    }

    if (fmatches->size >= MAX_MATCHES) {
        printf("WARNING: Maximum matches reached\n");
        return fmatches;
    }

    // Variable declarations
    char              name_buffer[MAX_FILENAME_LENGTH];
    char              path_buffer[MAX_PATH_LENGTH];
    DIR*              current_dir;
    struct dirent*    dir_entry;
    struct file_data* fdata;

    // Open directory
    current_dir = opendir(start_path);
    if (current_dir == NULL) {
        printf("ERROR: Failed to open directory: %s\n", start_path);
        return fmatches;
    }

    while (dir_entry = readdir(current_dir)) {
        // Ignore current and previous directories
        if (strncmp(dir_entry->d_name, ".", MAX_PATH_LENGTH) == 0) {
            continue;
        }

        if (strncmp(dir_entry->d_name, "..", MAX_PATH_LENGTH) == 0) {
            continue;
        }

        // Search directories recursively
        if (dir_entry->d_type == DT_DIR && recursive) {
            strncpy(path_buffer, start_path, MAX_PATH_LENGTH);
            if (!concat_path(path_buffer, dir_entry->d_name)) {
                break;
            }
            find_by_name(filename, (const char*)path_buffer, fmatches, recursive);
        }

        if (!is_substring(dir_entry->d_name, (char*)filename, name_buffer, MAX_FILENAME_LENGTH)) {
            continue;
        }

        strncpy(path_buffer, start_path, MAX_PATH_LENGTH);
        if (!concat_path(path_buffer, dir_entry->d_name)) {
            break;
        }

        if (fmatches->size >= MAX_MATCHES) {
            printf("WARNING: Maximum matches reached\n");
            break;
        }

        fdata = allocate_new_file_data(path_buffer, name_buffer, "");
        fmatches->files[fmatches->size] = fdata;
        fmatches->size++;
    }

    closedir(current_dir);
    return fmatches;
}

/**
 @brief Find a file in start_path and its child path by checking its content lines
 @param data: The data to search for on the file lines
 @param start_path: The starting path when searching for the file.
 @param fmatches: structure for holding file matches previously allocated
 */
struct file_matches* find_by_content(const char* data, const char* start_path,
                                     struct file_matches* fmatches, Bool recursive){
    printf("INFO: Searching by content with params: %s, %s\n", data, start_path);
    // Pre checks
    if (strlen(start_path) > MAX_PATH_LENGTH) {
        printf("ERROR: Start path is too big\n");
        return fmatches;
    }

    if (strlen(data) > MAX_CONTENT_SLICE_LENGTH) {
        printf("ERROR: Content slice is too big\n");
        return fmatches;
    }

    if (fmatches == NULL) {
        printf("ERROR: File matches pointer cannot be null");
        return NULL;
    }

    if (fmatches->size >= MAX_MATCHES) {
        printf("WARNING: Maximum matches reached\n");
        return fmatches;
    }

    // Variable declarations
    char                name_buffer[MAX_FILENAME_LENGTH];
    char                path_buffer[MAX_PATH_LENGTH];
    char                content_buffer[MAX_CONTENT_SLICE_LENGTH];
    DIR*                current_dir;
    struct dirent*      dir_entry;
    struct file_data*   fdata;

    // Open the directory
    current_dir = opendir(start_path);
    if (current_dir == NULL) {
        printf("ERROR: Failed to open directory: %s\n", start_path);
        return fmatches;
    }

    while (dir_entry = readdir(current_dir)) {
        // Skip current and previous directory
        if (strncmp(dir_entry->d_name, ".", MAX_PATH_LENGTH) == 0) {
            continue;
        }

        if (strncmp(dir_entry->d_name, "..", MAX_PATH_LENGTH) == 0) {
            continue;
        }

        if (dir_entry->d_type == DT_DIR && recursive) {
            strncpy(path_buffer, start_path, MAX_PATH_LENGTH);
            if (!concat_path(path_buffer, dir_entry->d_name)) {
                break;
            }
            find_by_content(data, (const char*)path_buffer, fmatches, recursive);
        }

        if (dir_entry->d_type != DT_REG) {
            continue;
        }

        strncpy(path_buffer, start_path, MAX_PATH_LENGTH);
        if (!concat_path(path_buffer, dir_entry->d_name)) {
            break;
        }

        if (!is_in_file_lines(path_buffer, (char*)data, content_buffer, MAX_CONTENT_SLICE_LENGTH)) {
            continue;
        }
        
        strncpy(name_buffer, dir_entry->d_name, MAX_FILENAME_LENGTH);

        if (fmatches->size >= MAX_MATCHES) {
            printf("WARNING: Maximum matches reached\n");
            break;
        }

        fdata = allocate_new_file_data(path_buffer, name_buffer, content_buffer);
        fmatches->files[fmatches->size] = fdata;
        fmatches->size++;
    }

    closedir(current_dir);
    return fmatches;
};


// <<< Aux functions >>>
/**
 @brief Concat to_concat to dest_path.
 @param dest_path: The source path where to_concat should be appended. Must have space for it.
 @param to_concat: The sub path to concat.
 @return Boolean stating if the operation was successful or not.
 */
Bool concat_path(char* dest_path, char* to_concat) {
    unsigned int dest_path_len = strlen(dest_path);
    unsigned int to_concat_len = strlen(to_concat);

    if (dest_path_len + to_concat_len + 2 > MAX_PATH_LENGTH) {
        printf("ERROR: Cannot concat %d length sub path to %d path. Max length exceeded\n",
                to_concat_len, dest_path_len);
        return False;
    }

    if (dest_path[dest_path_len - 1] != '/') {
        strcat(dest_path, "/");
    }

    strcat(dest_path, to_concat);
    return True;
}

/**
 @brief Allocate file data
 @param file_path: The path of the file matched.
 @param filename: The name of the file matched.
 @param file_content_slice: The contents matched on the file if any.
 @return The pointer to the file data structure allocated.
 */
struct file_data* allocate_new_file_data(char* file_path, char* filename, char* file_content_slice) {
    if (strlen(file_path) > MAX_PATH_LENGTH) {
        printf("ERROR: Filepath is too long\n");
        return NULL;
    }

    if (strlen(filename) > MAX_FILENAME_LENGTH) {
        printf("ERROR: Filename is too long\n");
        return NULL;
    }

    if (strlen(file_content_slice) > MAX_CONTENT_SLICE_LENGTH) {
        printf("ERROR: File content slice is too long\n");
        return NULL;
    }

    struct file_data* fdata = malloc(sizeof(struct file_data));
    if (fdata == NULL) {
        printf("ERROR: Failed to allocate memory for file data\n");
        return NULL;
    }

    strncpy(fdata->file_path, file_path, MAX_PATH_LENGTH);
    strncpy(fdata->file_name, filename, MAX_FILENAME_LENGTH);
    strncpy(fdata->file_content_slice, file_content_slice, MAX_CONTENT_SLICE_LENGTH);
    
    return fdata;
}

/**
 @brief Free a pointer and set it to null
 @param ptr: The pointer to be freed.
 */
void free_pointer(void* ptr) {
    if (ptr == NULL) {
        printf("ERROR: Cannot free null pointer\n");
        return;
    }

    free(ptr);
    ptr = NULL;
}

/**
 @brief Check if sub_str is a substring of src. On success store src on buffer.
 @param src: The source string
 @param sub_str: The string to check if its a substring of src
 @param buffer: The buffer where to store src on success.
 @param buffer_size: The size of buffer.
 @return A boolean stating if sub_str is sub string of src.
 */
Bool is_substring(char* src, char* sub_str, char* buffer, unsigned int buffer_size) {
    char*           str         = src;
    unsigned int    sub_str_len = strlen(sub_str);

    while(str = case_strchr(str, *sub_str)) {
        if (strncasecmp(str, sub_str, sub_str_len) == 0) {
            strncpy(buffer, src, buffer_size);
            return True;
        }
        str++;
    }

    return False;
}

/**
 @brief An adaptation of strchr that isn't case sensitive.
 @param string: The string where to find the occurrence of the character.
 @param character: The char to find its first occurrence on the string.
 @return A pointer to the first occurrence of character in string.
 */
char* case_strchr(char* string, char character) {
    char*   substring;
    char*   alternate_substring;
    char    alternate_char;
    int     substring_length = 0;
    int     alternate_substring_length = 0;

    // Find the substring case sensitive
    substring = strchr(string, character);
    substring_length = substring ? strlen(substring) : 0;

    if (character > 122 || character < 65 || (character > 90 && character < 97)) {
        // There isn't an alternate substring if the character is not in the alphabet.
        alternate_substring = (char*)0;
    } else {
        // De phase the character by 32 to get their opposite case. Find the substring
        alternate_char = character < 91 ? character + 32 : character - 32;
        alternate_substring = strchr(string, alternate_char);
    }

    alternate_substring_length = alternate_substring ? strlen(alternate_substring) : 0;

    // Return the longest substring to make sure first iteration of the character is found.
    return substring_length > alternate_substring_length ? substring : alternate_substring;
}

/**
 @brief Check whether sub_str is in any of the lines of the file at filepath.
 @param filepath: The path to the file to check;
 @param sub_str: The substring to find in the file lines.
 @param buffer: The buffer where to store the line on success.
 @param buffer_size: The size of the buffer.
 @return A boolean stating if the operation was successful.
 */
Bool is_in_file_lines(char* filepath, char* sub_str, char* buffer, unsigned int buffer_size) {
    FILE*   fs;
    char    line_buffer[MAX_CONTENT_SLICE_LENGTH];
    Bool    match_found = False;

    // Open the file
    fs = fopen(filepath, "r");
    if (fs == NULL) {
        printf("ERROR: Failed to open file at: %s\n", filepath);
        return False;
    }
    // Read the file line by line and store on buffer if a match is found.
    while (fgets(line_buffer, MAX_CONTENT_SLICE_LENGTH, fs)) {
        if (match_found = is_substring(line_buffer, sub_str, buffer, buffer_size)) {
            break;
        }
    }
    
    fclose(fs);
    return match_found;
} 

// <<< Retrieve constants >>>
unsigned int max_path_length() {
    return MAX_PATH_LENGTH;
}

unsigned int max_filename_length() {
    return MAX_FILENAME_LENGTH;
}

unsigned int max_content_slice_length() {
    return MAX_CONTENT_SLICE_LENGTH;
}

unsigned int max_matches() {
    return MAX_MATCHES;
}
