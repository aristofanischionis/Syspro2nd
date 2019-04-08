#!/bin/bash
# outputs any differencies between mirrors and inputfolders
# give number of clients that have ran
# give input folder name without any number
# give mirror folder name without any number
# run it like:
# ./bashScripts/differ.sh 4 hello _mirror
# checks arguments
if [[ $# -ne 3 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

let num=$1
input=$2
mirror=$3

for ((i = 1 ; i <= num ; i++)); do
    for ((j = 1 ; j <= num ; j++)); do
        if [[ i -ne j ]];then
            echo "I am doing : diff -r -q ${input}${i}/ ${j}${mirror}/${i}/"
            diff -r -q ${input}${i}/ ${j}${mirror}/${i}/ 
        fi
    done
done

