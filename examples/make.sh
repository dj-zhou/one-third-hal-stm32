#!/bin/bash
set -eu -o pipefail

# target can be: all, hal, clean, hal-clean

target="${1:-"all"}" # if $1 does not exit, assign "all" to target

directories=$(ls)

index=0
for i in $directories; do
    if [[ -d $i ]] && [[ -f $i/Makefile ]]; then
        index=$((index + 1))
        echo -e "\n---------------------------------------"
        echo -e "(below) directory #$index:\033[0;32m $i\033[0m\n"
        sleep 1
        cd $i
        if [ "$target" = "clean" ]; then
            make clean
        else
            make $target -j$(nproc)
        fi
        echo -e "(above) directory #$index:\033[0;32m $i\033[0m\n"
        cd ..
    fi
done
