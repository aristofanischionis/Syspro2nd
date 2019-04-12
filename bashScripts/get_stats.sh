#!/bin/bash

let id=0
let filesWritten=0
let filesRead=0
let minID=0
let maxID=0
let bytesSent=0
let bytesReceived=0
let clientsLeft=0
let clientsArrived=0
let metaWrite=0
let metaRead=0
# array of clients to be created
declare -a clientArray

while read -a sentence; do

    if [[ ${sentence[0]} == "id:" ]]; then
        id=${sentence[1]}
        # echo 'I read id is '${sentence[1]}
        clientArray[$clientsArrived]=${id}
        clientsArrived=$((clientsArrived + 1))
    fi

    if [[ ${sentence[0]} == "File" ]] && [[ ${sentence[1]} == "Written:" ]]; then
        filesWritten=$((filesWritten + 1))
        bytesSent=$((bytesSent + ${sentence[4]}))
        metaWrite=$((metaWrite + ${sentence[7]}))
    fi

    if [[ ${sentence[0]} == "File" ]] && [[ ${sentence[1]} == "Read:" ]]; then
        filesRead=$((filesRead + 1))
        bytesReceived=$((bytesReceived + ${sentence[4]}))
        metaRead=$((metaRead + ${sentence[7]}))
    fi

    if [[ ${sentence[0]} == "Leaving" ]]; then
        clientsLeft=$((clientsLeft + 1))
        # echo 'Client left the system '${sentence[3]}
    fi

done
# find the max and min from the client array
maxID=${clientArray[0]}
for n in "${clientArray[@]}" ; do
    ((n > maxID)) && maxID=$n
done
# echo 'max id is: '$maxID
# 
minID=${clientArray[0]}
for n in "${clientArray[@]}" ; do
    ((n < minID)) && minID=$n
done
# echo 'min id is: '$minID

echo '=============RESULTS============='
# printing the results
echo 'clients Arrived: '${clientsArrived}
echo '                                 '
echo 'Clients list:'
for ((i = 0 ; i < clientsArrived ; i++)); do
    echo 'client id: '${clientArray[$i]}
done

echo '                                 '
echo 'min ID is: '${minID}
echo 'max ID is: '${maxID}
echo '                                 '
echo 'bytes Sent in total: '${bytesSent}
echo 'MetaData Sent in total: '${metaWrite}
echo '                                 '
echo 'bytes Received in total: '${bytesReceived}
echo 'MetaData Received in total: '${metaRead}
echo '                                 '
echo 'files Sent in total: '${filesWritten}
echo 'files Received in total: '${filesRead}
echo '                                 '
echo 'clients Left: '${clientsLeft}
echo '=============END================='
