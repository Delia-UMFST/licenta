#!/bin/bash

FILE="test-all.c"
OUT="test-bench"
PLUGINS="-fplugin=./plugin1.so -fplugin=./plugin2.so -fplugin=./plugin3.so -fplugin=./plugin4.so -fplugin=./plugin5.so"

# Functie pentru rulare curata
measure_time() {
    { /usr/bin/time -f "%e" "$@" > /dev/null; } 2>&1 | tail -n 1
}

benchmark() {
    desc=$1
    cmd=$2
    echo "$desc"
    echo "------------------------------"
    printf "%-10s | %-10s\n" "Run" "Time (s)"
    echo "------------------------------"

    total=0
    for i in {1..5}; do
        t=$(measure_time $cmd -o $OUT)
        printf "%-10s | %-10s\n" "$i" "$t"
        total=$(echo "$total + $t" | bc)
    done
    avg=$(echo "scale=3; $total / 5" | bc)
    echo "------------------------------"
    printf "%-10s | %-10s\n\n" "Average" "$avg"
}

echo "Benchmarking compilation times for $FILE"
echo

benchmark "Fără pluginuri" "g++-13 -O0 $FILE"
benchmark "Cu pluginuri"   "g++-13 -O0 $PLUGINS $FILE"
