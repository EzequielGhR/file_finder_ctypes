#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <dirent.h>
#include "file_handler.h"


// Forward declarations
Bool                concat_path(char* dest_path, char* to_concat);
struct file_data*   allocate_new_file_data(char* file_path, char* filename, char* file_content_slice);
void                free_pointer(void* ptr);
Bool                is_substring(char* src, char* sub_str, char* buffer, unsigned int buffer_size);
char*               case_strchr(char* s, char c);


// Main logic
struct file_matches* find_by_name(const char* filename, const char* start_path,
                                  struct file_matches* fmatches) {
    if (strlen(start_path) > MAX_PATH_LENGTH) {
        fprintf(stderr, "EORROR: Start path is too big\n");
        return NULL;
    }

    if (strlen(filename) > MAX_FILENAME_LENGTH) {
        fprintf(stderr, "ERROR: Filename is too big\n");
        return NULL;
    }

    char              name_buffer[MAX_FILENAME_LENGTH];
    char              path_buffer[MAX_PATH_LENGTH];
    DIR*              current_dir;
    struct dirent*    dir_entry;
    struct file_data* fdata;

    current_dir = opendir(start_path);
    if (current_dir == NULL) {
        fprintf(stderr, "ERROR: Failed to open directory: %s\n", start_path);
        return NULL;
    }

    fmatches = fmatches ? fmatches : malloc(sizeof(struct file_matches));
    fmatches->size = fmatches->size ? fmatches->size : 0;
    if (fmatches == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory for file matches\n");
        closedir(current_dir);
        return NULL;
    }

    while (dir_entry = readdir(current_dir)) {
        if (strncmp(dir_entry->d_name, ".", MAX_PATH_LENGTH) == 0) {
            continue;
        }

        if (strncmp(dir_entry->d_name, "..", MAX_PATH_LENGTH) == 0) {
            continue;
        }

        if (dir_entry->d_type == DT_DIR) {
            printf("INFO: Found directory: %s\n", dir_entry->d_name);
            strncpy(path_buffer, start_path, MAX_PATH_LENGTH);
            if (!concat_path(path_buffer, dir_entry->d_name)) {
                break;
            }
            find_by_name(filename, path_buffer, fmatches);
        }

        if (!is_substring(dir_entry->d_name, filename, name_buffer, MAX_FILENAME_LENGTH)) {
            continue;
        }
        
        printf("INFO: Found substring '%s' in '%s'\n", filename, dir_entry->d_name);

        strncpy(path_buffer, start_path, MAX_PATH_LENGTH);
        if (!concat_path(path_buffer, dir_entry->d_name)) {
            break;
        }

        if (fmatches->size >= MAX_MATCHES) {
            fprintf(stderr, "WARNING: Maximum matches reached\n");
            break;
        }

        fdata = allocate_new_file_data(path_buffer, name_buffer, "");
        fmatches->files[fmatches->size] = fdata;
        fmatches->size++;
    }

    closedir(current_dir);
    return fmatches;
}


// Aux functions
Bool concat_path(char* dest_path, char* to_concat) {
    unsigned int dest_path_len = strlen(dest_path);
    unsigned int to_concat_len = strlen(to_concat);

    if (dest_path_len + to_concat_len + 2 > MAX_PATH_LENGTH) {
        fprintf(stderr, "ERROR: Cannot concat %d length sub path to %d path. Max length exceeded\n",
                to_concat_len, dest_path_len);
        return False;
    }

    if (dest_path[dest_path_len - 1] != '/') {
        strcat(dest_path, "/");
    }

    strcat(dest_path, to_concat);
    return True;
}

struct file_data* allocate_new_file_data(char* file_path, char* filename, char* file_content_slice) {
    printf("INFO: Allocating file data: %s\n", file_path);
    if (strlen(file_path) > MAX_PATH_LENGTH) {
        fprintf(stderr, "ERROR: Filepath is too long\n");
        return NULL;
    }

    if (strlen(filename) > MAX_FILENAME_LENGTH) {
        fprintf(stderr, "ERROR: Filename is too long\n");
        return NULL;
    }

    if (strlen(file_content_slice) > MAX_CONTENT_SLICE_LENGTH) {
        fprintf(stderr, "ERROR: File content slice is too long\n");
        return NULL;
    }

    struct file_data* fdata = malloc(sizeof(struct file_data));
    if (fdata == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory for file data\n");
        return NULL;
    }

    strncpy(fdata->file_path, file_path, MAX_PATH_LENGTH);
    strncpy(fdata->file_name, filename, MAX_FILENAME_LENGTH);
    strncpy(fdata->file_content_slice, file_content_slice, MAX_CONTENT_SLICE_LENGTH);
    
    return fdata;
}

void free_pointer(void* ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Cannot free null pointer\n");
        return;
    }

    free(ptr);
}

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

char* case_strchr(char* string, char character) {
    char*   substring;
    char    alternate_char;

    if (substring = strchr(string, character)) {
        return substring;
    }

    if (character > 122 || character < 65 || (character > 90 && character < 97)) {
        return NULL;
    }

    alternate_char = character < 91 ? character + 32 : character - 32;

    if (substring = strchr(string, alternate_char)) {
        return substring;
    } 

    return NULL;
}

// Retrieve constants
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
