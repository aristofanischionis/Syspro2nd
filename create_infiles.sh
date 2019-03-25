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
    # checks arguments
    if [[ $# -ne 4 ]] ; then
        echo 'the number of args provided is' $#
        exit 1
    fi

    dir_name=$1
    let num_of_files=$2
    let num_of_dirs=$3
    let levels=$4

    # checks numbers in args
    if [[ num_of_dirs -lt 0 ]] || [[ num_of_files -lt 0 ]] || [[ levels -lt 0 ]]; then
        echo 'number given is < 0'
        exit 2
    fi
    # checks dir name given
    if [ -d $dir_name ]; then 
        echo 'dir_name exists'
        deleter $dir_name
    fi
    # making the dir
    mkdir -p $dir_name
    # array of dir names to be created
    declare -a dir_array
    # get their random names
    for ((i = 0 ; i < num_of_dirs ; i++)); do
        dir_array[$i]=$(random_string)
        echo ${dir_array[$i]}
    done
    # create them in lvls
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
    # declare a filename array
    declare -a fil_array
    # create random file names
    echo 'lets make our nice files'
    for ((i = 0 ; i < num_of_files ; i++)); do
        fil_array[$i]=$(random_string)'.file'
        echo ${fil_array[$i]}
    done
    # create them in RR order and place them in folders
    counter=0
    let dir_counter=0
    # not correct yet, right now i put the files in the same pattern as i create the folders
    # have to check it again
    while true; do
        if [[ counter -eq num_of_files ]]; then
            break
        fi
        tmp_dir=$dir_name
        echo $tmp_dir'/'${fil_array[$counter]}
        touch $tmp_dir'/'${fil_array[$counter]}
        counter=$counter+1

        for ((i = 0 ; i < levels ; i++)); do
            if [[ dir_counter -eq num_of_dirs ]]; then
                break
            fi
            
            tmp_dir=$tmp_dir'/'${dir_array[$dir_counter]}
            dir_counter=$dir_counter+1
            echo $tmp_dir'/'${fil_array[$counter]}
            touch $tmp_dir'/'${fil_array[$counter]}
            counter=$counter+1
        done
        # echo ${tmp_dir}
        # mkdir -p $tmp_dir
    done
}

main $*


