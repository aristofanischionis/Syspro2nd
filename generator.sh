#!/bin/bash

if [[ "$#" -ne 2 ]]; then
    echo "Illegal number of parameters";  exit 1
fi

clientsNum=$1
delay=$2

pkill -f mirror_client
rm -rf mirror*

make clean && make

sleep 2

for ((i=1; i<=$clientsNum; i++));
do
    inputDirNum=$(($i%2 + 1))
    gnome-terminal --working-directory=/home/aristofanis/Desktop/ergasiaFakelidi/Syspro2nd -e "./mirror_client -n $i -c common -i hello$inputDirNum -m mirror$i -b 1000 -l log_file$i"
    sleep $delay
done

