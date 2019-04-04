#!/bin/bash

# generates a random string of random len 1-8
function random_string()
{
    cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w ${1:-$((1 + RANDOM % 8))} | head -n 1
}

# generates a random string of random len 1-8
function random_data()
{
    VAR=$(shuf -i 1000-128000 -n 1)
    # echo 'my var is '${VAR}
    cat /dev/urandom | tr -dc "a-zA-Z0-9!@#$%^&*()_+?><~\`;'" | head -c ${VAR} > $1
}

# checks if to delete or not the given folder
function deleter () {
    echo "Press Y/y to delete it and proceed"
    echo "Press anything else to exit"
    read  -n 1 -p "Input Selection:" input
    echo ''
    if [[ "$input" == "y" ]] || [[ "$input" == "Y" ]]; then
        rm -rf $1
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
        # echo ${dir_array[$i]}
    done
    # create them in lvls
    declare -a rr_dir_arr
    rr_dir_arr[0]=$dir_name
    let dir_counter=1
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
            # put it in dir_arr
            rr_dir_arr[$dir_counter]=$tmp_dir
            # echo 'array elem'${dir_counter}'is '${rr_dir_arr[$dir_counter]}
            dir_counter=$((dir_counter + 1))
            counter=$((counter + 1))
        done
        # echo ${tmp_dir}
        mkdir -p $tmp_dir
    done
    # declare a filename array
    declare -a fil_array
    # create random file names
    # echo 'lets make our nice files'
    for ((i = 0 ; i < num_of_files ; i++)); do
        fil_array[$i]=$(random_string)
        # fil_array[$i]='f'${i}
        # echo ${fil_array[$i]}
    done
    # create them in RR order and place them in folders
    # i have already my array of dirs in rr order so just go through the array and add files
    let fil_counter=0
    while [[ fil_counter -lt num_of_files ]]; do
        for ((i = 0 ; i < dir_counter ; i++)); do
            if [[ fil_counter -eq num_of_files ]]; then
                break
            fi
            tmp_file=${rr_dir_arr[$i]}'/'${fil_array[$fil_counter]}
            echo 'file to be made '${tmp_file}
            touch $tmp_file
            random_data $tmp_file
            fil_counter=$((fil_counter + 1))
        done
    done
}

main $*
