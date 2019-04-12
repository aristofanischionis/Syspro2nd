#!/bin/bash
# decrypt using the password for the private key of the receiver
# the pinentry option let me skip the prompt messages and just take te passPhrase 
# from the next option passphrase
# -q and no verbose are there to mute the gpg's messages as much as possible
# correct usage: ./decrypt_file.sh passPhrase encrypted_file output_file

if [[ $# -ne 3 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

pass=$1
encrypted=$2
output=$3

gpg --pinentry-mode loopback --yes -q --no-verbose --passphrase=${pass} --decrypt --output ${output} ${encrypted}