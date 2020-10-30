#!/bin/bash
directories=$(ls)

index=0
for i in $directories ; do
    if [[ -d $i ]] && [[ -f $i/Makefile ]] ; then
        index=$((index+1))
		echo -e "\n---------------------------------------"
		echo -e " directory #$index:\033[0;32m $i\033[0m\n"
        if [ -z $1 ] ; then
		    sleep 1
        fi
        cd $i
        make -j$(cat /proc/cpuinfo | grep processor | wc -l) $1
        cd ..
    fi
done

