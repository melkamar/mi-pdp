function testSolver {
    fn="$1"
    expectedEdges="$2"
    executable="$3"
    echo "Testing: $executable $fn"

    output=$("./$executable" "input/official/$fn")

    regex='==\s*([[:digit:]]*)\s+edges\s*=='
    if [[ $output =~ $regex ]]; then
	edgesFound="${BASH_REMATCH[1]}"
        if [[ $edgesFound -eq $expectedEdges ]]; then
            echo "OK: $fn - $executable"
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

#testSolver "graph10_5.txt" 19 "solver-seq"
#testSolver "graph17_3.txt" 22 "solver-seq"
#testSolver "graph20_3.txt" 26 "solver-seq"
#testSolver "graph25_3.txt" 34 "solver-seq"
#testSolver "graph14_4.txt" 22 "solver-seq"

testSolver "graph10_5.txt" 19 "solver"
testSolver "graph17_3.txt" 22 "solver"
testSolver "graph20_3.txt" 26 "solver"
testSolver "graph25_3.txt" 34 "solver"
testSolver "graph14_4.txt" 22 "solver"

