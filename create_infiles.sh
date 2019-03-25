#!/bin/bash

function deleter () {
    echo "Press Y/y to delete it and proceed"
    echo "Press anything else to exit"
    read  -n 1 -p "Input Selection:" input
    echo ''
    if [[ "$input" == "y" ]] || [[ "$input" == "Y" ]]; then
        rm -rf $dir_name
        echo 'Deleted'
    else
        echo 'Exiting...'
        exit 3
    fi
}

if [[ $# -ne 4 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

dir_name=$1
let num_of_files=$2
let num_of_dirs=$3
let levels=$4

# i want at least one

if [[ num_of_dirs -lt 1 ]] || [[ num_of_files -lt 1 ]] || [[ levels -lt 1 ]]; then
    echo 'number given is < 0'
    exit 2
fi

if [ -d $dir_name ]; then 
    echo 'dir_name exists'
    deleter $dir_name
fi

mkdir -p $dir_name

# random-string()
# {
#     cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w ${1:-32} | head -n 1
# }
# random-string 16