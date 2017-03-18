#!/usr/bin/env bash

function testSolver {
    fn="$1"
    expectedEdges="$2"
    output=$(./solver.exe "input/official/$fn")

    regex='==\s*([[:digit:]]*)\s+edges\s*=='
    if [[ $output =~ $regex ]]; then
	edgesFound="${BASH_REMATCH[1]}"
        if [[ $edgesFound -eq $expectedEdges ]]; then
            echo "OK: $fn"
        else
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
}

testSolver "graph10_5.txt" 19
testSolver "graph17_3.txt" 22
testSolver "graph20_3.txt" 26
testSolver "graph25_3.txt" 34
testSolver "graph14_4.txt" 22

