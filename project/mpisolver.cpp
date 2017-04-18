//
// Created by Martin on 18.04.2017.
//

#include <iostream>
#include <iomanip>

#include <queue>
#include <thread>

#include "Graph.h"
#include "ompsolver.h"
#include <mpi.h>
#include "mpisolver.h"

using namespace std;

namespace mpisolver {

    #define MESSAGE_TAG_NEED_WORK 1 // the slave is requesting new work
    #define MESSAGE_TAG_SLAVE_RESULT 2 // the message payload contains a newly found best result's edges count
    #define MESSAGE_TAG_WORK 3 // the message payload contains a graph to be solved by a slave
    #define MESSAGE_TAG_NO_MORE_WORK 4 // there is no more work for a slave to be done
    #define MPI_GRAPH_INT_PARAMS_COUNT 5

    Graph *bestGraph = NULL;
    int PROCESS_RANK = -1;
    int MIN_EDGES_SOLUTION = 0;

    /**
     * Entrypoint for MPI Master and Slave processes.
     * @param argc
     * @param argv
     * @param graph Starting Graph.
     * @param graphsPerProcess Number of initial graphs to generate for each process.
     */
    void runMPI(int argc, char **argv, Graph &graph, int graphsPerProcess) {
        int processCount;

        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &PROCESS_RANK);

        /* Number of processes */
        MPI_Comm_size(MPI_COMM_WORLD, &processCount);

        if (PROCESS_RANK == 0) {
            mpisolver::processMaster(graph, processCount, processCount * graphsPerProcess);
            printBest(bestGraph);
        } else {
            mpisolver::processSlave();
        }

        MPI_Finalize();
    }

    void logMPI(string text) {
        if (PROCESS_RANK == 0) {
            cout << "MASTER  | " << text << endl;
        } else {
            cout << "SLAVE " << PROCESS_RANK << " | " << text << endl;
        }
    }

    void sendGraph(Graph *graph, int targetProcessId, int tag) {
        logMPI("   ... sending graph to " + to_string(targetProcessId) + " with tag " + to_string(tag) + " and hash " +
               to_string(graph->hash()));

        int intBuffer[MPI_GRAPH_INT_PARAMS_COUNT];
        if (bestGraph) intBuffer[0] = bestGraph->edgesCount;
        else intBuffer[0] = -1;

        intBuffer[1] = graph->nodes;
        intBuffer[2] = graph->startI;
        intBuffer[3] = graph->startJ;
        intBuffer[4] = graph->edgesCount;

        MPI_Send(&intBuffer, MPI_GRAPH_INT_PARAMS_COUNT, MPI_INT, targetProcessId, tag, MPI_COMM_WORLD);

        for (int i = 0; i < graph->nodes; i++) {
            MPI_Send(graph->adjacency[i], graph->nodes, MPI_CXX_BOOL, targetProcessId, tag, MPI_COMM_WORLD);
        }
        logMPI("   ... finished sending graph.");
    }

    Graph *recvGraph(int source) {
        int intBuffer[MPI_GRAPH_INT_PARAMS_COUNT];

        // Receive ints - edges of the best graph etc.
        MPI_Status status;
        MPI_Recv(&intBuffer, MPI_GRAPH_INT_PARAMS_COUNT, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int wantedTag = status.MPI_TAG;
        switch (status.MPI_TAG) {
            case MESSAGE_TAG_WORK:
                logMPI("ReceiveGraph: New work.");
                // ok. got new work as requested.
                break;

            case MESSAGE_TAG_NO_MORE_WORK:
                // No more work is to be done. Stop working.
                logMPI("ReceiveGraph: No more work!");
                return NULL;

            case MESSAGE_TAG_SLAVE_RESULT:
                // A new best graph is being sent.
                logMPI("ReceiveGraph: New best result");
                break;
            default:
                cout << "Unknown tag ReceiveGraph:: " << status.MPI_TAG << endl;
                throw new invalid_argument("Unknown tag for ReceiveGraph: " + status.MPI_TAG);
        }


        int best_edges = intBuffer[0]; // Number of edges of the best solution yet
        int graph_nodes = intBuffer[1]; // Number of nodes of the graph being sent
        int graph_startI = intBuffer[2]; // startI coordinate of the graph being sent
        int graph_startJ = intBuffer[3]; // startJ coordinate of the graph being sent
        int graph_edgesCount = intBuffer[4]; // edges count of the graph being sent (to save processing power on calculating it)

        bool **adjacency = new bool *[graph_nodes];
        for (int i = 0; i < graph_nodes; ++i) {
            adjacency[i] = new bool[graph_nodes];
        }

        for (int i = 0; i < graph_nodes; i++) { // receive as many lines as there are nodes
            MPI_Recv(adjacency[i], graph_nodes, MPI_CXX_BOOL, source, wantedTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        Graph *graph = new Graph(graph_nodes, adjacency, graph_startI, graph_startJ, graph_edgesCount);
        logMPI("Received graph, hash: " + to_string(graph->hash()));

//    cout << "SLAVE  |" << PROCESS_RANK << " Received graph. Best: " << best_edges << " | nodes: " << graph_nodes
//         << " | start: [" << graph_startI
//         << "," << graph_startJ << "]" << endl;
//    graph->print("SLAVE " + to_string(PROCESS_RANK) + ": ");

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
    void processMaster(Graph &startGraph, int processCount, int initialGraphsCount) {
        logMPI("Master started.");
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

        deque<Graph *> *initialGraphs = ompsolver::generateInitialStates(startGraph, initialGraphsCount);

        int buffer;
        MPI_Status status;

        int noMoreWorksSent = 0; // how many slaves have stopped working?

        // In a loop listen for slaves' messages
        while (true) {
            bool breakloop = false;

            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            switch (status.MPI_TAG) {
                case MESSAGE_TAG_NEED_WORK: { // Slave needs a new thing to work on
                    int source = status.MPI_SOURCE;
                    logMPI(to_string(source) + " NEED_WORK");
                    MPI_Recv(&buffer, 1, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                    if (initialGraphs->size() > 0) {
                        Graph *graph = initialGraphs->front();
                        initialGraphs->pop_front();
                        logMPI("Have work for " + to_string(source) + " -> sending WORK: " + to_string(graph->hash()) +
                               ". Graphs left to process: " + to_string(initialGraphs->size()));
                        sendGraph(graph, source, MESSAGE_TAG_WORK);
                    } else {
                        logMPI("No more work for " + to_string(source) + " -> sending NO_MORE_WORK");
                        MPI_Send(&buffer, 1, MPI_INT, source, MESSAGE_TAG_NO_MORE_WORK, MPI_COMM_WORLD);

                        noMoreWorksSent++; // Number of "NO_MORE_WORK" messages sent to slaves. TODO this should be a broadcast
                        if (noMoreWorksSent == processCount - 1) {
                            breakloop = true;
                        }
                    }
                    break;
                }

                case MESSAGE_TAG_SLAVE_RESULT: { // Slave found a new solution, check and update current best
                    int source = status.MPI_SOURCE;
                    MPI_Recv(&buffer, 1, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    logMPI(to_string(source) + " found new best. Edges: " + to_string(buffer));

                    logMPI("   - receiving");
                    Graph *graph = recvGraph(source);
                    logMPI("   - done receiving");

                    // If slave's solution is better than what I have
                    if (!bestGraph || bestGraph->getEdgesCount() < graph->getEdgesCount()) {
                        delete bestGraph;
                        bestGraph = graph;
                    }
                    break;
                }

                default:
                    logMPI("Unknown tag for Master: " + status.MPI_TAG);
                    throw new invalid_argument("Unknown tag for Master: " + status.MPI_TAG);

            }

            if (breakloop) break;
        }

        logMPI("== DONE == ProcessMaster");
    }


    void processSlave() {
        int buffer = PROCESS_RANK;

        while (true) {
            logMPI("Request work");
            MPI_Send(&buffer, 1, MPI_INT, 0, MESSAGE_TAG_NEED_WORK, MPI_COMM_WORLD);
            Graph *startGraph = recvGraph(0);

            if (!startGraph) {
                // No more work to be done, stop the loop
                logMPI("!startGraph -> break");
                break;
            }

            // generate some initial graphs
            unsigned threadCount = 1;
            #if defined(_OPENMP)
            threadCount = std::thread::hardware_concurrency();
            #endif

//            betterGraphFound = false; // flag will be true if doSearchOpenMP finds a better result
            Graph *bestFound = ompsolver::doSearchOpenMP(*startGraph, threadCount);

//        if (betterGraphFound) {
            logMPI("Found local best: " + to_string(bestFound->edgesCount));
            buffer = bestFound->edgesCount;
            logMPI("  ... sending flag");
            MPI_Send(&buffer, 1, MPI_INT, 0, MESSAGE_TAG_SLAVE_RESULT, MPI_COMM_WORLD);
            logMPI("  ... sending graph");
            sendGraph(bestFound, 0, MESSAGE_TAG_SLAVE_RESULT);
            logMPI("  ... done.");
//        }
        }

        logMPI("== DONE == ProcessSlave");
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
}