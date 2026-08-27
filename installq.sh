#!/bin/bash

set -e

EXTRA_CFLAGS=""

while getopts "a:" opt; do
	case "$opt" in
		a) EXTRA_CFLAGS="$OPTARG" ;;
		\?) echo "[Error] Usage: $0 [-a \"compiler_flags\"] <tool_name>"
			exit 1;;
	esac
done

shift $((OPTIND - 1))
TOOL_NAME=$1

if [ -z "$TOOL_NAME" ]; then
	echo "[Error] Usage: $0 [-a \"compiler_flags\"] <tool_name>"
	exit 1
fi

WIN_BIN_DIR="/mnt/c/sne/bin"
LINUX_BIN_DIR="$HOME/bin"

mkdir -p "$WIN_BIN_DIR"
mkdir -p "$LINUX_BIN_DIR"
mkdir -p "./src"

gcc quick.c sne.c -o quick $EXTRA_CFLAGS

x86_64-w64-mingw32-gcc quick.c sne.c -o quick.exe $EXTRA_CFLAGS

mv quick.exe "$WIN_BIN_DIR/${TOOL_NAME}.exe"

mv quick "$LINUX_BIN_DIR/${TOOL_NAME}"

chmod +x "$LINUX_BIN_DIR/${TOOL_NAME}"

cp quick.c "./src/${TOOL_NAME}.c"

echo Install ${TOOL_NAME} complete.
