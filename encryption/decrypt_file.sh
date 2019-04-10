#!/bin/bash
# checks arguments
# correct usage: ./decrypt_file.sh pass encrypted_file output_file

if [[ $# -ne 3 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

pass=$1
encrypted=$2
output=$3

gpg --pinentry-mode loopback --yes --no-verbose --passphrase=${pass} --decrypt --output ${output} ${encrypted}