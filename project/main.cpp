#include <iostream>
#include <fstream>
#include <iomanip>

#include <omp.h>
#include <queue>
#include <thread>

#include "Graph.h"
#include <mpi.h>

using namespace std;

int MIN_EDGES_SOLUTION = 0;
int PROCESS_RANK = -1;

bool betterGraphFound = false;

/**
 * Increment the j-index with regard to the edges matrix. If j overflows, increment i and start from the diagonal.
 *
 * For nodes=4, the valid positions marked with O:
 *
 *     0 1 2 3
 *   ---------
 * 0 | X O O O
 * 1 | X X O O
 * 2 | X X X O
 * 3 | X X X X
 *
 *
 * @param i
 * @param j
 * @param nodes
 * @returns True if the returned position is valid, false if there is not another valid position.
 */
bool incrementEdgeIndex(int &i, int &j, int nodes) {
    j++;

    if (j > nodes - 1) { // j is outside of the matrix range (overflown to the right)
        i++;
        if (i > nodes - 2) // i is outside of the matrix range (overflown to the bottom), no more positions to try
            return false;

        j = i + 1;
    }

    return true;
}

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

void printBest(Graph *bestGraph) {
    if (bestGraph) {
        cout << "Best bipartite graph edge count: " << endl;
        cout << "================" << endl;
        cout << "== " << setw(4) << bestGraph->getEdgesCount() << " edges ==" << endl;
        cout << "================" << endl;
        bestGraph->printBipartiteSets();
        cout << endl;
        bestGraph->printEdges();
    } else {
        cout << "No best graph found. This should not happen!" << endl;
    }
}

Graph *bestGraph = NULL;


/**
 * Sequential search of graph state space using DFS.
 * @param graph
 */
void doSearchDFS(Graph *graph) {
    // Only begin loop if removing an edge from this graph will make sense
    if (
            (graph->getEdgesCount() < graph->nodes) ||
            (bestGraph && graph->getEdgesCount() <= bestGraph->getEdgesCount())
            ) { // Graph is already as sparse as possible, no reason to process it
        delete graph;
        return;
    }

    // Generate neighboring graphs by removing one edge from this one.
    // Start with the [startI, startJ] edge in the adjacency matrix.
    int i = graph->startI;
    int j = graph->startJ;

    bool valid; // If true, the obtained [i, j] indices point at a valid ID of edge to remove
    do {
        valid = incrementEdgeIndex(i, j, graph->nodes);
        if (valid && graph->isAdjacent(i, j)) {
            // if [i,j] are valid indices and the edge they point at is present -> create a new graph by removing the edge

            // Instead of cloning the Graph straíght away, test and see if it is valid using this graph first. (Revert this value at the end of loop)
            graph->setAdjacency(i, j, false);

            if (graph->getEdgesCount() < graph->nodes - 1) {
                graph->setAdjacency(i, j, true);
                continue; // this graph must be disjoint -> skip searching
            }

            if (bestGraph && graph->getEdgesCount() < bestGraph->getEdgesCount()) {
                // this graph is already worse than the found maximum -> skip searching
                graph->setAdjacency(i, j, true);
                continue;
            }

            short bip = graph->isBipartiteOrConnected();
            switch (bip) {
                case 1: // Is bipartite -> check if better than current best.
                    // Do not process it further in any case, because all subsequent graphs will only be worse.
                    if (!bestGraph || (graph->getEdgesCount() > bestGraph->getEdgesCount())) {
                        #pragma omp critical
                        {
                            if (!bestGraph || (graph->getEdgesCount() > bestGraph->getEdgesCount())) {
                                if (bestGraph) delete bestGraph;
                                bestGraph = new Graph(*graph);

                                #if defined(_OPENMP)
                                int threadNum = omp_get_thread_num();
                                #else
                                int threadNum = 0;
                                #endif

                                betterGraphFound = true;

                                cout << "New best graph edges count: " << bestGraph->getEdgesCount()
                                     << ". Found by thread " << threadNum << endl;
                            }
                        }
                    }
                    break;

                case -1: // Is disjoint -> dont bother.
                    break;

                case 0: // Is connected but not bipartite -> check if it makes sense to search further
                    if (graph->getEdgesCount() < graph->nodes)
                        break; // this graph already has the minimum possible edges and is not a solution -> skip searching
                    if (bestGraph && graph->getEdgesCount() <= bestGraph->getEdgesCount())
                        break; // this graph is already worse than the found maximum -> skip searching


                    Graph *newGraph = new Graph(*graph);
                    newGraph->startI = i;
                    newGraph->startJ = j;

                    doSearchDFS(newGraph);
                    break;
            }

            graph->setAdjacency(i, j, true);
        }
    } while (valid); // While there are edges to remove


    delete graph;
}

/**
 * Generate a queue of initial states using BFS.
 *
 * @param startGraph Initial search state.
 * @param requestedSize Number of graphs to generate.
 */
deque<Graph *> *generateInitialStates(Graph &startGraph, int requestedSize) {
    deque<Graph *> *resultQ = new deque<Graph *>();
    resultQ->push_back(new Graph(startGraph)); // make a copy of the graph so that it can be safely deleted during dfs

    short bip = startGraph.isBipartiteOrConnected();
    switch (bip) {
        case 1:
            return resultQ;
        case -1:
            cout << "Given source graph is disjoint! I refuse to process it." << endl;
            return NULL;
        default:
            break;
    }

    while (!resultQ->empty() && resultQ->size() < requestedSize) {
        Graph *graph = resultQ->front();
        resultQ->pop_front();

        // Only begin loop if removing an edge from this graph will make sense
        if (
                (graph->getEdgesCount() < graph->nodes) ||
                (bestGraph && graph->getEdgesCount() <= bestGraph->getEdgesCount())
                ) { // Graph is already as sparse as possible, no reason to process it
            delete graph;
            continue;
        }

        // Generate neighboring graphs by removing one edge from this one.
        // Start with the [startI, startJ] edge in the adjacency matrix.
        int i = graph->startI;
        int j = graph->startJ;

        bool valid; // If true, the obtained [i, j] indices point at a valid ID of edge to remove
        do {
            valid = incrementEdgeIndex(i, j, graph->nodes);
            if (valid && graph->isAdjacent(i, j)) {
                // if [i,j] are valid indices and the edge they point at is present -> create a new graph by removing the edge

                // Instead of cloning the Graph straíght away, test and see if it is valid using this graph first. (Revert this value at the end of loop)
                graph->setAdjacency(i, j, false);

                if (graph->getEdgesCount() < graph->nodes - 1) {
                    graph->setAdjacency(i, j, true);
                    continue; // this graph must be disjoint -> skip searching
                }

                if (bestGraph && graph->getEdgesCount() < bestGraph->getEdgesCount()) {
                    // this graph is already worse than the found maximum -> skip searching
                    graph->setAdjacency(i, j, true);
                    continue;
                }

                short bip = graph->isBipartiteOrConnected();
                bool breakloop = false;
                switch (bip) {
                    case 1: // Is bipartite -> check if better than current best.
                        // Do not process it further in any case, because all subsequent graphs will only be worse.
                        if (!bestGraph || (graph->getEdgesCount() > bestGraph->getEdgesCount())) {
                            if (bestGraph) delete bestGraph;
                            bestGraph = new Graph(*graph);
                        }
                        break;

                    case -1: // Is disjoint -> dont bother.
                        break;

                    case 0: // Is connected but not bipartite -> check if it makes sense to search further
                        if (graph->getEdgesCount() < graph->nodes)
                            break; // this graph already has the minimum possible edges and is not a solution -> skip searching
                        if (bestGraph && graph->getEdgesCount() <= bestGraph->getEdgesCount())
                            break; // this graph is already worse than the found maximum -> skip searching


                        Graph *newGraph = new Graph(*graph);
                        newGraph->startI = i;
                        newGraph->startJ = j;
                        resultQ->push_back(newGraph);

                        if (resultQ->size() + 1 >= requestedSize) {
                            // already have enough initial states, stop generating more and add currently processed
                            // graph to it too (there may be remaining graphs to generate from it
                            graph->setAdjacency(i, j, true);
                            graph->startI = i;
                            graph->startJ = j;
                            resultQ->push_back(new Graph(*graph));
                            breakloop = true; // Need to set a flag, because break only breaks the switch statement
                            break;
                        }
                }

                if (breakloop) break;

                graph->setAdjacency(i, j, true);
            }
        } while (valid); // While there are edges to remove

        delete graph;
    } // No graphs left in stack

    return resultQ;
}


Graph* doSearchOpenMP(Graph &startGraph, unsigned threadCount) {
    short bip = startGraph.isBipartiteOrConnected();
    switch (bip) {
        case 1:
            printBest(&startGraph);
            return &startGraph;
        case -1:
            cout << "Given source graph is disjoint! I refuse to process it." << endl;
            return NULL;
        default:
            break;
    }

    MIN_EDGES_SOLUTION = startGraph.nodes - 1;

    // BFS to obtain a certain number of tasks
    deque<Graph *> *initialGraphs = generateInitialStates(startGraph, threadCount * 7);

    int i;
    #pragma omp parallel for private(i) num_threads(threadCount)
    for (i = 0; i < initialGraphs->size(); i++) {
        Graph *graph = (*initialGraphs)[i];
        doSearchDFS(graph);
    }

    printBest(bestGraph);
    return bestGraph;
//    delete bestGraph;
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
                if (line[j] == '0') {
                    graph.setAdjacency(i, j, false);
                } else {
                    graph.setAdjacency(i, j, true);
                }
            }
        }

        f.close();
        return graph;

    } else {
        cout << "Error opening the file." << endl;
        exit(1);
    }
}

#define MESSAGE_TAG_NEED_WORK 1 // the slave is requesting new work
#define MESSAGE_TAG_NEW_BEST 2 // the message payload contains a newly found best result's edges count
#define MESSAGE_TAG_WORK 3 // the message payload contains a graph to be solved by a slave
#define MESSAGE_TAG_NO_MORE_WORK 4 // there is no more work for a slave to be done

void myMPI_SendGraph(Graph *graph, int targetProcessId, int tag) {
    int intBuffer[4];
    if (bestGraph) intBuffer[0] = bestGraph->edgesCount;
    else intBuffer[0] = -1;

    intBuffer[1] = graph->nodes;
    intBuffer[2] = graph->startI;
    intBuffer[3] = graph->startJ;

    MPI_Send(&intBuffer, 4, MPI_INT, targetProcessId, tag, MPI_COMM_WORLD);

    for (int i = 0; i < graph->nodes; i++) {
        MPI_Send(graph->adjacency[i], graph->nodes, MPI_CXX_BOOL, targetProcessId, tag, MPI_COMM_WORLD);
    }
}

Graph *myMPI_ReceiveGraph(int source) {
    int intBuffer[4];

    // Receive ints - edges of the best graph etc.
    MPI_Status status;
    MPI_Recv(&intBuffer, 4, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    int wantedTag = status.MPI_TAG;
    switch (status.MPI_TAG){
        case MESSAGE_TAG_WORK:
            // ok. got new work as requested.
            break;

        case MESSAGE_TAG_NO_MORE_WORK:
            // No more work is to be done. Stop working.
            return NULL;

        case MESSAGE_TAG_NEW_BEST:
            // A new best graph is being sent.
            break;
        default:
            throw new invalid_argument("Unknown tag for Slave#ReceiveWork: " + status.MPI_TAG);
    }


    int best_edges = intBuffer[0]; // Number of edges of the best solution yet
    int graph_nodes = intBuffer[1]; // Number of nodes of the graph being sent
    int graph_startI = intBuffer[2]; // startI coordinate of the graph being sent
    int graph_startJ = intBuffer[3]; // startJ coordinate of the graph being sent

    bool **adjacency = new bool *[graph_nodes];
    for (int i = 0; i < graph_nodes; ++i) {
        adjacency[i] = new bool[graph_nodes];
    }

    for (int i = 0; i < graph_nodes; i++) { // receive as many lines as there are nodes
        MPI_Recv(adjacency[i], graph_nodes, MPI_CXX_BOOL, source, wantedTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    Graph *graph = new Graph(graph_nodes, adjacency);

    cout << "SLAVE " << PROCESS_RANK << " Got work. Best: " << best_edges << " | nodes: " << graph_nodes
         << " | start: [" << graph_startI
         << "," << graph_startJ << "]" << endl;
    graph->print("SLAVE " + to_string(PROCESS_RANK) + ": ");

    return graph;
}

/*
 * Message format:
 * WORK:
 *  - int*
 *    - [0] - best edges count so far
 *    - [1] - number of nodes
 *    - [2] - startI
 *    - [3] - startJ
 *  - bool* - 1st row of matrix (size=number of nodes)
 *  - bool* - 2nd row of matrix
 *  - ....
 */
void MPI_ProcessMaster(Graph &startGraph, int processCount, int initialGraphsCount) {
    cout << "I am MASTER" << endl;

    short bip = startGraph.isBipartiteOrConnected();
    switch (bip) {
        case 1:
            printBest(&startGraph);
            return;
        case -1:
            cout << "Given source graph is disjoint! I refuse to process it." << endl;
            return;
        default:
            break;
    }

    MIN_EDGES_SOLUTION = startGraph.nodes - 1;

    deque<Graph *> *initialGraphs = generateInitialStates(startGraph, initialGraphsCount);

    int buffer;
    MPI_Status status;

    // In a loop listen for slaves' messages
    while (!initialGraphs->empty()) {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        switch (status.MPI_TAG) {
            case MESSAGE_TAG_NEED_WORK: { // Slave needs a new thing to work on
                int source = status.MPI_SOURCE;
                cout << source << " wants work" << endl;
                MPI_Recv(&buffer, 1, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                Graph *graph = initialGraphs->front();
                initialGraphs->pop_front();
                myMPI_SendGraph(graph, source, MESSAGE_TAG_WORK);
                break;
            }

            case MESSAGE_TAG_NEW_BEST: { // Slave found a new best solution, update it
                int source = status.MPI_SOURCE;
                MPI_Recv(&buffer, 1, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                cout << source << " found new best: " << buffer << endl;

                if (bestGraph->getEdgesCount() < buffer){
                    // Graph being received really is better than what I currently have
                    Graph * graph = myMPI_ReceiveGraph(source);
                    delete bestGraph;
                    bestGraph = graph;
                }
                break;
            }

            default:
                throw new invalid_argument("Unknown tag for Master: " + status.MPI_TAG);
        }
    }

//    for (int i = 0; i < processCount - 1; i++) {
//        MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
//        cout << "MASTER: Received " << buffer << endl;
//    }
}


void MPI_ProcessSlave() {
    cout << "I am SLAVE" << endl;
    int buffer = PROCESS_RANK;

    while (true) {
        MPI_Send(&buffer, 1, MPI_INT, 0, MESSAGE_TAG_NEED_WORK, MPI_COMM_WORLD);
        Graph *startGraph = myMPI_ReceiveGraph(0);

        if (!startGraph){
            // No more work to be done, stop the loop
            break;
        }

        // generate some initial graphs
        unsigned threadCount = 1;
        #if defined(_OPENMP)
        threadCount = std::thread::hardware_concurrency();
        #endif


        betterGraphFound = false; // flag will be true if doSearchOpenMP finds a better result
        Graph * bestFound = doSearchOpenMP(*startGraph, threadCount);

        if (betterGraphFound){
            buffer = bestFound->edgesCount;
            MPI_Send(&buffer, 1, MPI_INT, 0, MESSAGE_TAG_NEW_BEST, MPI_COMM_WORLD);
        }
    }
}

void runMPI(int argc, char **argv, Graph &graph) {
    int processCount;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &PROCESS_RANK);

    /* Number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    if (PROCESS_RANK == 0) {
        MPI_ProcessMaster(graph, processCount, processCount * 20);
    } else {
        MPI_ProcessSlave();
    }

    MPI_Finalize();
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

    runMPI(argc, argv, graph);
    return 0;
}
