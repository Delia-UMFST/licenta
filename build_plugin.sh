#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <x> <y>"
  echo "Example: $0 3 2"
  exit 1
fi

x=$1
y=$2
source_file="plugin${x}.${y}.cc"
output_file="plugin${x}.so"

if [ ! -f "$source_file" ]; then
  echo "Error: Source file '$source_file' not found."
  exit 2
fi

g++-13 -fPIC -shared -o "$output_file" "$source_file" \
  -fno-rtti -fno-exceptions \
  -I/usr/lib/gcc/x86_64-linux-gnu/13/plugin/include \
  -I/usr/lib/gcc/x86_64-linux-gnu/13/include

if [ $? -eq 0 ]; then
  echo "Successfully built '$output_file'."
else
  echo "Compilation failed."
  exit 3
fi

