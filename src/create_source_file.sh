#!/bin/bash

touch $1
cat begin_file > $1

IFS='/' read -ra file <<< "$1"
file=${file[-1]}
file="${file%.c}.h"
echo "\"${file}\"" >> "$1"