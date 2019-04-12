#!/bin/bash
# generates keys for a client with the details specified by a file like the gen_key_script provided
# as demo
# but a bit different in my program
# i'm not using the defaults
# i'm using the ELG-E function and 1024 bytes so I try to keep it as small as possible to 
# make the process as fast as possible, but maybe a bit more insecure
if [[ $# -ne 1 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

gen_key_script=$1

gpg --batch --gen-key ${gen_key_script}
