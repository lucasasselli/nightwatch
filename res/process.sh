#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

PATH=$SCRIPT_DIR/../build-sim/libs/minigl/:$PATH

input_dir=$SCRIPT_DIR/textures
output_dir=$SCRIPT_DIR/../game/res/textures

if [[ ! -d "$input_dir" ]]; then
    echo "Input directory '$input_dir' does not exist."
    exit 1
fi

# Check if output directory exists and is empty
if [[ -d "$output_dir" && -n "$(ls -A "$output_dir")" ]]; then
    echo "Output directory '$output_dir' is not empty. Do you want to proceed and overwrite the contents? (y/n)"
    read -r answer
    if [[ "$answer" != "y" ]]; then
        echo "Aborting..."
        exit 1
    fi
fi

# Copy source folder
rm -rf $output_dir
cp -rf $input_dir $output_dir


for file in $(find $output_dir/. -type f); do
    echo "Processing $file ..."

    ext="${file##*.}"
    case "$ext" in
        png)
            png2tex "$file" "${file%.png}.tex"
            rm -f "$file"
            ;;
        *)
            echo "Unsupported file type: $file"
            ;;
    esac
done
