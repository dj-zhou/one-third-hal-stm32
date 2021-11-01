#!/bin/bash
set -eu -o pipefail

GRN='\033[0;32m'
RED='\033[0;31m'
NOC='\033[0m'

# available targets: all, hal, clean, hal-clean

# if $1 does not exit, assign "all" to target
target="${1:-"all"}"
directories="$(ls)"

function _verify_target() {
    target="$1"
    if [ "$target" = "all" ]; then
        return
    fi
    if [ "$target" = "hal" ]; then
        return
    fi
    if [ "$target" = "clean" ]; then
        return
    fi
    if [ "$target" = "hal-clean" ]; then
        return
    fi
    echo -e "${RED}supported targets: all, hal, clean, hal-clean${NOC}"
    exit 1
}

function _make() { # target, index, dir
    target="$1"
    index="$2"
    dir="$3"
    echo -e "\n------------------------------------------------------"
    echo -e "(below) #$index: ${GRN}$dir${NOC}"
    cd $dir
    if [ "$target" = "all" ]; then
        make $target -j$(nproc)
    else
        make $target
    fi
    echo -e "(above) #$index: ${GRN}$dir${NOC}"
    echo "------------------------------------------------------"
    cd ..
}

index=0
_verify_target $target
for i in $directories; do
    # if that is a project, build it
    if [[ -d $i ]] && [[ -f $i/Makefile ]]; then
        index=$((index + 1))
        sleep 1
        _make $target $index $i
    elif [[ -d $i ]] && [[ ! -f $i/Makefile ]]; then
        cd $i
        subdirectories="$(ls)"
        for j in $subdirectories; do
            if [[ -d $j ]] && [[ -f $j/Makefile ]]; then
                index=$((index + 1))
                sleep 1
                _make $target $index $j
            fi
        done
        cd ..
    fi
done
