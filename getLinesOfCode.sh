#!/bin/bash

files_that_count=$(find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.glsl.vert" -o -name "*.glsl.frag" -o -name "*.v.pica" \) \( -not -path "./include/*" -a -not -path "./simdjson/*" -a -not -path "./SDL/*" -a -not -path "./SDL_mixer/*" \))

lines=0

while IFS= read -r file; do
    lines=$(($(wc -l < "$file") + lines))
done <<< "$files_that_count"

echo $lines