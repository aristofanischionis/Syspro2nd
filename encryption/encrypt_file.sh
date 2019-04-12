#!/bin/bash
# encrypting files using the recipients public key alias, the email
# --armor let me encrypt as ascii file not as binary which is the default
# so I don't have to do major changes in my program when using encryption
# correct usage: ./encrypt_file.sh email target_file
if [[ $# -ne 2 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

email=$1
target_file=$2 
# file to be encrypted

gpg --armor --trust-model always --yes --no-verbose --recipient ${email} --encrypt ${target_file}