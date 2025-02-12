# Description

Linux File Finder using ctypes.
It uses a shared library compiled from the contents of src, and python to wrap the C-functions using ctypes.
You can search for a file on a given path and all its subpaths by providing name and/or content.
The output will be a collection of markdown tables on the terminal:
    - Results found only by name
    - Results found only by content
    - Combined results
    - Intersection of results

Rows are truncated to avoid the tables getting too big. But filepaths are clickable hyperlinks

# Structure

- `src/file_handler.c` : Main source code file for the file finder logic.
- `src/file_handler.h` : Header file for file handler logic.
- `wrapper.py` : Main python wrapper for C functions.
- `compile_shared_library.sh` : Bash script to compile the shared library to the expected directory.
- `main.py` : Main python script to run the searches.
- `run_search.sh` : Run a search providing a path, file name and/or content.

# How to (WIP)

- Compile shared library : Execute `./compile_shared_library.sh`
- You can use `run_search.sh` to perform a file search:
    - `./run_search.sh -help` will give you general script usage examples.
    - `./run_search.sh some/starting/path some_filename` will search for all files with "some_filename" in their names, on the provided path and subpaths.
    - `./run_search.sh some/starting/path -null some_content` will search for all files with "some_content" in their contents, on the provided path and subpaths.
    - `./run_search.sh some/starting/path some_filename some_content` will search for all files with "some_filename" as part of their names, and/or "some_content" in their contents, on the provided path and subpaths.
- Alternatively you can use the python script `main.py` directly, using the arguments "--path" for the starting path, "--name" for the filename, and "--content" for the file content.
