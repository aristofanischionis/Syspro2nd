#!/bin/bash
# checks arguments
if [[ $# -ne 1 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

gen_key_script=$1

gpg --batch --gen-key ${gen_key_script}
