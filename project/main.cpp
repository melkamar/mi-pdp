#include <iostream>
#include <fstream>
#include <iomanip>

#include <thread>
#include <cstring>

#include "Graph.h"
#include "mpisolver.h"
#include "seqsolver.h"


using namespace std;


void printInit(int nodes, unsigned threadCount, string fn) {
    cout << "=========================================" << endl;
    cout << "Graph file:  " << fn << endl;
    cout << "Graph nodes: " << nodes << endl;
    if (threadCount == 1){
        cout << "Running SINGLETHREADED." << endl;
    } else {
        cout << "Running MULTITHREADED with " << threadCount << " threads." << endl;
    }
    cout << "=========================================" << endl;
}


Graph loadProblem(string filename) {
    ifstream f;
    f.open(filename);

    if (f.is_open()) {
        int nodesCount;
        f >> nodesCount;

        Graph graph(nodesCount);

        for (int i = 0; i < nodesCount; i++) {
            string line;
            f >> line;

            for (int j = 0; j < nodesCount; ++j) {
                graph.setAdjacency(i, j, line[j] != '0');
            }
        }

        f.close();
        return graph;

    } else {
        cout << "Error opening the file." << endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Enter input file name. Run as './solver input/graph seq' to run sequentially." << endl;
        return 1;
    }

    char *fn = argv[1];
    Graph graph = loadProblem(fn);


    unsigned threadCount = 1;
    #if defined(_OPENMP)
    threadCount = std::thread::hardware_concurrency();
    #endif

    if (argc == 3 && strcmp(argv[2], "seq") == 0) {
        printInit(graph.nodes, 1, fn);
        seqsolver::search(graph);
    } else {
        printInit(graph.nodes, threadCount, fn);
        mpisolver::runMPI(argc, argv, graph, 40);
    }
    return 0;
}
