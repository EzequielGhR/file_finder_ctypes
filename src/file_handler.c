#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <dirent.h>
#include "file_handler.h"


// Forward declarations
Bool                    concat_path(char* dest_path, char* to_concat);
struct file_data*       allocate_new_file_data(char* file_path, char* filename, char* file_content_slice);
struct file_matches*    allocate_or_return_file_matches(struct file_matches* fmatches);
void                    free_pointer(void* ptr);
Bool                    is_substring(char* src, char* sub_str, char* buffer, unsigned int buffer_size);
Bool                    is_in_file_lines(char* filepath, char* sub_str, char* buffer, unsigned int buffer_size);
char*                   case_strchr(char* s, char c);


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

    fmatches = allocate_or_return_file_matches(fmatches);
    if (fmatches == NULL) {
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
            find_by_name(filename, (const char*)path_buffer, fmatches);
        }

        if (!is_substring(dir_entry->d_name, (char*)filename, name_buffer, MAX_FILENAME_LENGTH)) {
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

struct file_matches* find_by_content(const char* data, const char* start_path, struct file_matches* fmatches){
    if (strlen(start_path) > MAX_PATH_LENGTH) {
        fprintf(stderr, "EORROR: Start path is too big\n");
        return NULL;
    }

    if (strlen(data) > MAX_CONTENT_SLICE_LENGTH) {
        fprintf(stderr, "ERROR: Content slice is too big\n");
        return NULL;
    }

    char                name_buffer[MAX_FILENAME_LENGTH];
    char                path_buffer[MAX_PATH_LENGTH];
    char                content_buffer[MAX_CONTENT_SLICE_LENGTH];
    DIR*                current_dir;
    struct dirent*      dir_entry;
    struct file_data*   fdata;

    current_dir = opendir(start_path);
    if (current_dir == NULL) {
        fprintf(stderr, "ERROR: Failed to open directory: %s\n", start_path);
        return NULL;
    }

    fmatches = allocate_or_return_file_matches(fmatches);
    if (fmatches == NULL) {
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
            find_by_content(data, (const char*)path_buffer, fmatches);
        }

        if (dir_entry->d_type != DT_REG) {
            printf("INFO: Skipping non-regular file: %s\n", dir_entry->d_name);
            continue;
        }

        printf("INFO: Found regular file: %s\n", dir_entry->d_name);
        strncpy(path_buffer, start_path, MAX_PATH_LENGTH);
        if (!concat_path(path_buffer, dir_entry->d_name)) {
            break;
        }

        if (!is_in_file_lines(path_buffer, data, content_buffer, MAX_CONTENT_SLICE_LENGTH)) {
            continue;
        }
        
        printf("INFO: Found substring '%s' in file '%s' contents\n", data, dir_entry->d_name);
        strncpy(name_buffer, dir_entry->d_name, MAX_FILENAME_LENGTH);

        if (fmatches->size >= MAX_MATCHES) {
            fprintf(stderr, "WARNING: Maximum matches reached\n");
            break;
        }

        fdata = allocate_new_file_data(path_buffer, name_buffer, content_buffer);
        fmatches->files[fmatches->size] = fdata;
        fmatches->size++;
    }

    closedir(current_dir);
    return fmatches;
};


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

struct file_matches* allocate_or_return_file_matches(struct file_matches* fmatches) {
    if (fmatches) {
        printf("INFO: Found allocated matches\n");
        return fmatches;
    }

    fmatches = malloc(sizeof(struct file_matches));
    if (fmatches == NULL) {
        fprintf(stderr, "Failed to allocate memory for file matches\n");
        return NULL;
    }

    fmatches->size = 0;
    return fmatches;
}

void free_pointer(void* ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Cannot free null pointer\n");
        return;
    }

    free(ptr);
    ptr = NULL;
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
    char*   alternate_substring;
    char    alternate_char;
    int     substring_length = 0;
    int     alternate_substring_length = 0;

    substring = strchr(string, character);
    substring_length = substring ? strlen(substring) : 0;

    if (character > 122 || character < 65 || (character > 90 && character < 97)) {
        alternate_substring = (char*)0;
    } else {
        alternate_char = character < 91 ? character + 32 : character - 32;
        alternate_substring = strchr(string, alternate_char);
    }

    alternate_substring_length = alternate_substring ? strlen(alternate_substring) : 0;

    return substring_length > alternate_substring_length ? substring : alternate_substring;
}

Bool is_in_file_lines(char* filepath, char* sub_str, char* buffer, unsigned int buffer_size) {
    FILE*   fs;
    char    line_buffer[MAX_CONTENT_SLICE_LENGTH];
    Bool    match_found = False;

    fs = fopen(filepath, "r");
    if (fs == NULL) {
        fprintf(stderr, "Failed to open file at: %s\n", filepath);
        return False;
    }
    while (fgets(line_buffer, MAX_CONTENT_SLICE_LENGTH, fs)) {
        if (match_found = is_substring(line_buffer, sub_str, buffer, buffer_size)) {
            break;
        }
    }
    
    fclose(fs);
    return match_found;
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
