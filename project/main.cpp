#include <iostream>
#include <fstream>
#include <iomanip>

#include <thread>
#include <cstring>
#include <mpi.h>

#include "seqsolver.h"
#include "ompsolver.h"

#include "Graph.h"
#include "mpisolver.h"


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
    // ./solver input/graph seq
    // ./solver input/graph par 10


    if (argc < 3) {
        cout << "Enter input file name. Run as './solver input/graph [seq|par] [num_threads]'" << endl;
        return 1;
    }

    char *fn = argv[1];
    Graph graph = loadProblem(fn);


//    unsigned threadCount = 1;
//    #if defined(_OPENMP)
//    threadCount = std::thread::hardware_concurrency();
//    #endif

    if (strcmp(argv[2], "seq") == 0) {
        printInit(graph.nodes, 1, fn);
        seqsolver::search(graph);
    } else if (strcmp(argv[2], "mpi") == 0) {
        int threadCount = stoi(argv[3]);
        printInit(graph.nodes, threadCount, fn);
        mpisolver::runMPI(argc, argv, graph, 40, threadCount);
    } else if (strcmp(argv[2], "omp") == 0) {
        int threadCount = stoi(argv[3]);
        printInit(graph.nodes, threadCount, fn);
        cout << "  Only OpenMP" << endl;

        double wtimeStart = MPI_Wtime();
        Graph * bgraph = ompsolver::doSearchOpenMP(graph, threadCount);
        mpisolver::printBest(bgraph);
        double wtimeEnd = MPI_Wtime();

        cout << endl << endl << "Computation time: " << endl << (wtimeEnd - wtimeStart) << endl;
    } else {
        cout << "Unknown parameter \""<<argv[2]<<"\". Expected seq or par." << endl;
        exit(2);
    }
    return 0;
}
