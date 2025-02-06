#!/bin/bash

cpp_hpp_files=$(find . -type f \( -name "*.cpp" -o -name "*.hpp" \) \( -not -path "./include/*" -a -not -path "./simdjson/*" \))

lines=0

while IFS= read -r file; do
    lines=$(($(wc -l < "$file") + lines))
done <<< "$cpp_hpp_files"

echo $lines