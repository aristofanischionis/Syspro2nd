#!/bin/bash

if [[ "$#" -ne 2 ]]; then
    echo "Illegal number of parameters";  exit 1
fi

clientsNum=$1
delay=$2

pkill -f mirror_client
rm -rf mirror\*
rm -rf common/
rm log_file\*

make clean && make

sleep 2

for ((i=1; i<=$clientsNum; i++));
do
    inputDirNum=$(($i%4 + 1))
    echo "Client $i has input: hello$inputDirNum"
    gnome-terminal --working-directory=/home/aristofanis/Desktop/Syspro2nd -e "./mirror_client -n $i -c common -i hello$inputDirNum -m mirror$i -b 1 -l log_file$i"
    sleep $delay
done

