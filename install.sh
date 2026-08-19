#!/bin/bash

TOOL_NAME=$1

if [ -z "$TOOL_NAME" ]; then
    echo "[Error]Usage: ./install.sh xxx"
    exit 1
fi

WIN_BIN_DIR="/mnt/c/mtb/bin"

x86_64-w64-mingw32-gcc quick.c mtb.c -o quick.exe

mv quick.exe "$WIN_BIN_DIR/${TOOL_NAME}.exe"

cp quick.c "./src/${TOOL_NAME}.c"

echo Install ${TOOL_NAME} complete.
