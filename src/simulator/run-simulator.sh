#!/bin/bash

runtime=$(NS_LOG="MPINodeApplication" ./simulator --number="$1" --file="$2" --logs="$3" 2>&1 | grep "StopApplication")
count=$(echo "$runtime" | wc -l)

if [ "$count" == "$1" ]
then
    final=$(echo "$runtime" | tail -n1 | cut -f1 -d' ')
    echo $final
else
    echo "Mismatch. An error occurred"
fi
