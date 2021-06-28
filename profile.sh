#!/bin/bash
path=$(readlink -f "$1")
if [[ -z "$path" ]]; then
    echo "Executable not found: $path."
    exit 1
fi
valgrind --tool=callgrind "$path" "${@:2}"
kcachegrind "$(ls callgrind.out.* | tail -1)"
