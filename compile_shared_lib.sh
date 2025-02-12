#!/bin/bash

SCRIPT_DIR=$(dirname "${0}")
LIB_DIR="${SCRIPT_DIR}/lib"
LIB_PATH="${LIB_DIR}/filehandler.so"

echo "Creating directory: ${LIB_DIR}"
mkdir -p $LIB_DIR

echo "Creating shared library at ${LIB_PATH}"
gcc -shared -fPIC ${SCRIPT_DIR}/src/file_handler.* -o $LIB_PATH
