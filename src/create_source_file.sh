#!/bin/bash

touch $1.h
touch $1.c
cat begin_file_h > $1.h
cat begin_file_c > $1.c

IFS='/' read -ra file <<< "$1"
file=${file[-1]}
file=$file.h
echo "\"${file}\"" >> $1.c