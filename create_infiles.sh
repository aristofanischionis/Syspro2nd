#!/bin/bash

# generates a random string of random len 1-8
function random_string()
{
    cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w ${1:-$((1 + RANDOM % 8))} | head -n 1
}

# checks if to delete or not the given folder
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

# main function
function main () {
    if [[ $# -ne 4 ]] ; then
        echo 'the number of args provided is' $#
        exit 1
    fi

    dir_name=$1
    let num_of_files=$2
    let num_of_dirs=$3
    let levels=$4

    # i want at least one

    if [[ num_of_dirs -lt 0 ]] || [[ num_of_files -lt 0 ]] || [[ levels -lt 0 ]]; then
        echo 'number given is < 0'
        exit 2
    fi

    if [ -d $dir_name ]; then 
        echo 'dir_name exists'
        deleter $dir_name
    fi

    mkdir -p $dir_name

    declare -a dir_array

    for ((i = 0 ; i < num_of_dirs ; i++)); do
        dir_array[$i]=$(random_string)
        echo ${dir_array[$i]}
    done

    let counter=0
    while true; do
        if [[ counter -eq num_of_dirs ]]; then
                break
        fi
        tmp_dir=$dir_name

        for ((i = 0 ; i < levels ; i++)); do
            if [[ counter -eq num_of_dirs ]]; then
                break
            fi
            tmp_dir=$tmp_dir'/'${dir_array[$counter]}
            counter=$counter+1
        done
        echo ${tmp_dir}
        mkdir -p $tmp_dir
    done
}

main $*


