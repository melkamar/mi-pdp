#include <iostream>
#include <fstream>
#include <iomanip>

#include <thread>

#include "Graph.h"
#include "mpisolver.h"


using namespace std;


void printInit(int nodes, unsigned threadCount, string fn) {
    cout << "=========================================" << endl;
    cout << "Graph file:  " << fn << endl;
    cout << "Graph nodes: " << nodes << endl;
    #if defined(_OPENMP)
    cout << "Running MULTITHREADED with " << threadCount << " threads." << endl;
    #else
    cout << "Running SINGLETHREADED." << endl;
    #endif
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
        cout << "Enter input file name." << endl;
        return 1;
    }

    char *fn = argv[1];

    Graph graph = loadProblem(fn);


    unsigned threadCount = 1;
    #if defined(_OPENMP)
    threadCount = std::thread::hardware_concurrency();
    #endif

    printInit(graph.nodes, threadCount, fn);

    mpisolver::runMPI(argc, argv, graph, 10);
    return 0;
}
