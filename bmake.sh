#!/bin/bash
COUNT_FILE="build_number.txt"
if [[ ! -f "$COUNT_FILE" ]]; then
    echo 0 > "$COUNT_FILE"
fi
current_number=$(< "$COUNT_FILE")
((current_number++))
echo "$current_number" > "$COUNT_FILE"
echo "***************************************************"
echo "******  Build Number: $current_number ******"
echo "***************************************************"
nmake

