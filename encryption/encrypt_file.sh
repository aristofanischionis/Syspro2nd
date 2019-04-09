#!/bin/bash
# checks arguments
# correct usage: ./encrypt_file.sh email target_file
if [[ $# -ne 2 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

email=$1
target_file=$2 
# file to be encrypted

gpg --armor --trust-model always --yes --recipient ${email} --encrypt ${target_file}