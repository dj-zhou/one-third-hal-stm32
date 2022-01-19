#!/bin/bash
set -eu -o pipefail

target="${1:-"all"}"

GRN='\033[0;32m'
RED='\033[0;31m'
NOC='\033[0m'

script_dir="$(dirname "$0")"
pushd "$script_dir" &>/dev/null

# =====================================================
# build stm32 project
for dir in "f407zg" "f767zi" ; do
    pushd $dir &>/dev/null
    echo -e "\n------------------------------------------------------"
    echo -e "(below): ${GRN}$(pwd)${NOC}"
    make $target -j$(nproc)
    echo -e "(above): ${GRN}$(pwd)${NOC}"
    echo "------------------------------------------------------"
    popd &>/dev/null
done
# =====================================================
# build PC project
# this script is specific for this directory
if [[ "$target" = 'hal'* ]]; then
    exit
fi

pushd pc-program &>/dev/null
echo -e "\n------------------------------------------------------"
echo -e "(below): ${GRN}$(pwd)${NOC}"

if [[ "$target" = 'all'* ]]; then
    echo "mkdir -p build && cd build && cmake .. && make -j$(nproc)"
    # just always build, since docker build and native build may have conflict
    rm -rf build && mkdir -p build && cd build
    cmake .. && make -j$(nproc) && cd ..
elif [[ "$target" = 'clean'* ]]; then
    echo "rm -rf build/"
    rm -rf build/
fi
echo -e "(above): ${GRN}$(pwd)${NOC}"
echo "------------------------------------------------------"
popd &>/dev/null

popd &>/dev/null
