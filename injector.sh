#!/bin/bash
str=$(pgrep -nx "$1" -r S | head -n 1)
pid=$((str))
if [ $pid -le 0 ]; then
    echo "$1 - No such process" >&2
    exit 1
fi
lib=$(readlink -f "$2")
if [ ! -f "$lib" ]; then
    echo "$lib - No such file" >&2
    exit 1
fi

"$(dirname "$0")"/linux_injector 1 $pid "$lib"
