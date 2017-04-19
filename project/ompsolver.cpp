//
// Created by Martin on 18.04.2017.
//

#include "ompsolver.h"
#include <iostream>
#include <fstream>
#include <iomanip>

#include <omp.h>
#include <queue>
#include <thread>

#include "Graph.h"
#include "mpisolver.h"
#include <mpi.h>

using namespace std;
namespace ompsolver {
    Graph *bestGraph = NULL;
    int MIN_EDGES_SOLUTION = 0;

    /**
     * Generate a queue of initial states using BFS.
     *
     * @param startGraph Initial search state.
     * @param requestedSize Number of graphs to generate.
     */
    deque<Graph *> *generateInitialStates(Graph &startGraph, int requestedSize) {
        deque<Graph *> *resultQ = new deque<Graph *>();
        resultQ->push_back(
                new Graph(startGraph)); // make a copy of the graph so that it can be safely deleted during dfs

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

    Graph *doSearchOpenMP(Graph &startGraph, unsigned threadCount) {
        short bip = startGraph.isBipartiteOrConnected();
        switch (bip) {
            case 1:
//            printBest(&startGraph);
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

//    printBest(bestGraph);
        return new Graph(*bestGraph);
//    delete bestGraph;
    }

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

//                                betterGraphFound = true;

                                    cout << "New best graph edges count: " << bestGraph->getEdgesCount()
                                         << ". Found by thread " << threadNum <<" | MPI rank: "<<mpisolver::getRank()<< endl;
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

}