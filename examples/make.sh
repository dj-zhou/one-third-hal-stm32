#!/bin/bash
set -eu -o pipefail

GRN='\033[0;32m'
RED='\033[0;31m'
NOC='\033[0m'

# available targets: all, hal, clean, hal-clean

script_dir="$(dirname "$0")"
pushd "$script_dir" &>/dev/null

# if $1 does not exist, assign "all" to target
target="${1:-"all"}"
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

    pushd "$dir" &>/dev/null
    echo -e "\n------------------------------------------------------"
    echo -e "(below) #$index: ${GRN}$(pwd)${NOC}"
    if [ "$target" = "all" ]; then
        make $target -j$(nproc)
    else
        make $target
    fi
    echo -e "(above) #$index: ${GRN}$(pwd)${NOC}"
    echo "------------------------------------------------------"
    if [[ "$target" = "all" || "$target" = "hal" ]]; then
        sleep 1
    fi
    popd &>/dev/null
}

_verify_target $target
directories="$(ls)"
index=0
for i in $directories; do
    # if that is a project, build it
    if [[ -d $i ]] && [[ -f $i/Makefile ]]; then
        index=$((index + 1))
        _make $target $index $i
    elif [[ -d $i ]] && [[ -f $i/make.sh ]]; then
        pushd "$i" &>/dev/null
        ./make.sh $target
        popd &>/dev/null
    fi
done
popd &>/dev/null
