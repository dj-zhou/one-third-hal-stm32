#!/bin/bash
set -eu -o pipefail

GRN='\033[0;32m'
RED='\033[0;31m'
NOC='\033[0m'

# available targets: all, hal, clean, hal-clean

script_dir="$(dirname "$0")"
pushd "$script_dir" &>/dev/null

# if $1 does not exit, assign "all" to target
target="${1:-"all"}"

function _verify_target() {
    target="$1"
    if [[ "$target" = "all" || "$target" = "hal" || "$target" = "clean" || "$target" = "hal-clean" ]]; then
        return
    fi
    echo -e "${RED}supported targets: all, hal, clean, hal-clean${NOC}"
    exit 1
}

# target, index, dir
function _make() {
    target="$1"
    index="$2"
    dir="$3"

    pushd "$dir" &>/dev/null
    echo -e "\n------------------------------------------------------"
    echo -e "(below) #$index: ${GRN}$(pwd)${NOC}"
    make $target -j$(nproc)
    echo -e "(above) #$index: ${GRN}$(pwd)${NOC}"
    echo "------------------------------------------------------"

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
