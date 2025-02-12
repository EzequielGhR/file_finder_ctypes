#ifndef FILE_HANDLER_H
    #define FILE_HANDLER_H

    #define MAX_PATH_LENGTH 10000
    #define MAX_FILENAME_LENGTH 1000
    #define MAX_CONTENT_SLICE_LENGTH 1000
    #define MAX_MATCHES 10


    typedef enum {
        False,
        True
    } Bool;

    struct file_data {
        char file_path[MAX_PATH_LENGTH];
        char file_name[MAX_FILENAME_LENGTH];
        char file_content_slice[MAX_CONTENT_SLICE_LENGTH];
    };

    struct file_matches {
        struct file_data* files[MAX_MATCHES];
        unsigned int size;
    };

    unsigned int max_path_length();
    unsigned int max_filename_length();
    unsigned int max_content_slice_length();
    unsigned int max_matches();

    struct file_matches* find_by_name(const char* filename, const char* start_path,
                                      struct file_matches* fmatches, Bool recursive);
    struct file_matches* find_by_content(const char* data, const char* start_path,
                                         struct file_matches* fmatches, Bool recursive);
    void free_pointer(void* ptr);

#endif