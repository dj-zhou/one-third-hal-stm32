#!/bin/bash
set -eu -o pipefail

GRN='\033[0;32m'
RED='\033[0;31m'
NOC='\033[0m'

# if $1 does not exist, assign "all" to target
target="${1:-"all"}"

function _verify_target() {
    target="$1"
    if [[ "$target" = "all" || "$target" = "hal" || "$target" = "clean" || "$target" = "hal-clean" ]]; then
        return
    fi
    # target can also be a directory name
    if [ -d "$target" ]; then
        return
    fi
    echo -e "${RED}supported targets: all, hal, clean, hal-clean, or any subdirectory${NOC}"
    exit 1
}

_verify_target $target

# if the target is a directory, enter it and use Makefile or make.sh file
if [ -d "$target" ]; then
    pushd "$target" &>/dev/null
    if [ -f Makefile ]; then
        make hal -j$(nproc)
        make all -j$(nproc)
    elif [ -f make.sh ]; then
        ./make.sh hal
        ./make.sh all
    fi
    popd &>/dev/null
    exit
fi

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

# if the target is either all, hal, clean, hal-clean, iterate over the dirctories
directories="$(ls)"
index=0
for i in $directories; do
    # if that is a project, build it
    if [[ -d $i ]] && [[ -f $i/Makefile ]]; then
        index=$((index + 1))
        _make $target $index $i
    # if it has a make.sh, use it
    elif [[ -d $i ]] && [[ -f $i/make.sh ]]; then
        pushd "$i" &>/dev/null
        ./make.sh $target
        popd &>/dev/null
    fi
done
