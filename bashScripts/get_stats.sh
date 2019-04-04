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
    fi

    if [[ ${sentence[0]} == "File" ]] && [[ ${sentence[1]} == "Read:" ]]; then
        filesRead=$((filesRead + 1))
        bytesReceived=$((bytesReceived + ${sentence[4]}))
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

for ((i = 0 ; i < clientsArrived ; i++)); do
    echo 'client id: '${clientArray[$i]}
done

echo 'min ID is: '${minID}
echo 'max ID is: '${maxID}
echo 'bytes Sent in total: '${bytesSent}
echo 'bytes Received in total: '${bytesReceived}
echo 'files Sent in total: '${filesWritten}
echo 'files Received in total: '${filesRead}
echo 'clients Left: '${clientsLeft}

echo '=============END================='
