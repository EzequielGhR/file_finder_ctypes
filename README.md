# Description

Linux File Finder using ctypes.

# Structure

- `src/file_handler.c` : Main source code file for the file finder logic.
- `src/file_handler.h` : Header file for file handler logic.
- `wrapper.py` : Main python wrapper for C functions.
- `compile_shared_library.sh` : Bash script to compile the shared library to the expected directory.

# How to (WIP)

- Compile shared library : Execute `./compile_shared_library.sh`
- Use (WIP) : Main logic not yet finished. You can test and use the python wrapper by calling its functions "find_by_name" and "find_by_content". 