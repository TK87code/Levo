#!/bin/bash

set -eu

USE_SDL=false

while getopts "s" opt; do
        case "$opt" in
                s) USE_SDL=true ;;
                \?) echo "[Error] Invalid arguments."
                        exit 1
        esac
done

shift $((OPTIND - 1))

if [ "$USE_SDL" = true ]; then 
	gcc -Wall -Wextra -Werror -std=c99 -pedantic -O2 sdlquick.c levo.c -o a -lSDL2
else
	gcc -Wall -Wextra -Werror -std=c99 -pedantic -O2 quick.c levo.c -o a 
fi
