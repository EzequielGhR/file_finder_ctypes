#!/bin/bash

SCRIPT_DIR=$(dirname "${0}")
EXEC_DIR=$pwd

if [ "${1}" == "-help" ]
    then echo "Usage: ${0} [path] [name | -null] [content]"
    echo "Only by name: ${0} [path] [name]"
    echo "Only by content: ${0} [path] -null [content]"
    exit 0
fi

if [ -z "${1}" ] || [ -z "${2}" ]
    then echo "Missing arguments. Usage: ${0} [path] [name | -null] [content]"
    echo "Help message: ${0} -help"
    exit 1
fi

if [ "${2}" == "-null" ] && [ -z "${3}" ]
    then echo "If name is \"-null\", content must be provided. Usage: ${0} [path] [name | -null ] [content]"
    echo "Help message: ${0} -help"
    exit 1
fi

if [ "${2}" == "-null" ]
    then cd $SCRIPT_DIR
    python main.py --path "${1}" --content "${3}"
    cd $EXEC_DIR
    exit 0
fi

cd $SCRIPT_DIR
python main.py --path "${1}" --name "${2}" --content "${3}"
cd $EXEC_DIR
exit 0
