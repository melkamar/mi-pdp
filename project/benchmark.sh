#!/usr/bin/env bash

set -e -E

#EXECUTABLE_SEQUENTIAL="solver-seq"
#EXECUTABLE_PARALLEL="solver"
EXECUTABLE_PARALLEL="mpirun"
EXECUTABLE_PARALLEL_ARGS="-n 4 ./solver"

function runCommand {
    fn="$1"
    expectedEdges="$2"
    executable="$3"

    output=$(/usr/bin/time -o "out_time_$executable" -f %E "$executable" $EXECUTABLE_PARALLEL_ARGS "input/official/$fn")
    elapsed_time=$(cat "out_time_$executable")

    # Check that solution is correct
    regex='==\s*([[:digit:]]*)\s+edges\s*=='
    if [[ $output =~ $regex ]]; then
	edgesFound="${BASH_REMATCH[1]}"
        if [[ $edgesFound -ne $expectedEdges ]]; then
            echo "FAIL: $fn"
            echo "Expected: $expectedEdges"
            echo "Received: $edgesFound"
            echo "Full output:\n$output"
            exit 2
        fi
    else
	echo "Output does not match regex!"
        echo "$output"
        exit 1
    fi

    time_millis=$(echo "$elapsed_time" | awk -F '[:.]' '{print ($1*60*100+$2*100+$3)*10}')
}

function benchmark {
    fn="$1"
    expectedEdges="$2"

    runCommand "$fn" "$expectedEdges" "$EXECUTABLE_SEQUENTIAL"
    first_millis=$time_millis

    runCommand "$fn" "$expectedEdges" "$EXECUTABLE_PARALLEL"
    second_millis=$time_millis

    if [[ $second_millis -eq 0 ]]; then
        speedup="N/A"
    else
        speedup=$(echo "scale=2;($first_millis/$second_millis-1)*100" | bc)
    fi

    echo "OK ${fn} [ms]:"
    echo "     - sequential: $first_millis"
    echo "     - parallel:   $second_millis"
    echo "     - speedup:    $speedup %"
    echo ""
}

function reportParallel {
    fn="$1"
    expectedEdges="$2"

    runCommand "$fn" "$expectedEdges" "$EXECUTABLE_PARALLEL"
    echo "OK ${fn}: $time_millis ms   (multithreaded)"
}

reportParallel "graph10_5.txt" 19
reportParallel "graph17_3.txt" 22
reportParallel "graph20_3.txt" 26
reportParallel "graph25_3.txt" 34
reportParallel "graph14_4.txt" 22

reportParallel "graph10_6.txt" 20
reportParallel "graph12_5.txt" 22
#reportParallel "graph14_4.txt" 22
reportParallel "graph18_4.txt" 29

reportParallel "graph14_5.txt" 25
