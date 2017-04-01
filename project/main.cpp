#include <iostream>
#include <fstream>
#include <iomanip>

#include <omp.h>
#include <queue>

#include "Graph.h"

using namespace std;

int MIN_EDGES_SOLUTION = 0;


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

void doSearchRecOMP(Graph *graph) {
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

                    int threshold = 3;
                    if (newGraph->edgesCount - MIN_EDGES_SOLUTION < threshold ||
                        (bestGraph && newGraph->edgesCount - bestGraph->getEdgesCount() < threshold)) {
                        doSearchRecOMP(newGraph);
                    } else {
                        #pragma omp task
                        doSearchRecOMP(newGraph);
                    }
                    break;
            }

            graph->setAdjacency(i, j, true);
        }
    } while (valid); // While there are edges to remove


    delete graph;
}

/**
 * Generic tree search algorithm. May be DFS or BFS based on searchStructure (stack or queue, respectively).
 *
 * @param startGraph Initial search state.
 * @param searchStructure Concrete implementation of SearchStructure container.
 */
queue<Graph *> *doBFS(Graph &startGraph, int requestedSize) {
    queue<Graph *> *resultQ = new queue<Graph *>();
    resultQ->push(new Graph(startGraph)); // make a copy of the graph so that it can be safely deleted during dfs

    short bip = startGraph.isBipartiteOrConnected();
    switch (bip) {
        case 1:
//            printBest(&startGraph);
            return resultQ;
        case -1:
            cout << "Given source graph is disjoint! I refuse to process it." << endl;
            return NULL;
        default:
            break;
    }

    while (!resultQ->empty() && resultQ->size() < requestedSize) {
        Graph *graph = resultQ->front();
        resultQ->pop();

//        cout << "Edges: " << setw(3) << graph->getEdgesCount() << endl;

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
//                            cout << "New best graph edges count: " << bestGraph->getEdgesCount() << endl;
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
                        cout << "Adding graph. Removing edge [" << i << "," << j << "]" << endl;
                        resultQ->push(newGraph);

                        // TODO this should be working, but not tested.
                        if (resultQ->size()+1 >= requestedSize) {  // already have enough initial states, stop generating more.
                            graph->setAdjacency(i, j, true);
                            graph->startI = i;
                            graph->startJ = j;
                            resultQ->push(graph);
                            breakloop = true;
                            break;
                        }
                }

                if (breakloop) break;

                graph->setAdjacency(i, j, true);
            }
        } while (valid); // While there are edges to remove


//        delete graph;
    } // No graphs left in stack

    return resultQ;

//    cout << "::DFS:: complete. Graphs seen: " << graphsCount;
    cout << endl;
}

/**
 * Generic tree search algorithm. May be DFS or BFS based on searchStructure (stack or queue, respectively).
 *
 * @param startGraph Initial search state.
 * @param searchStructure Concrete implementation of SearchStructure container.
 */
void doSearch(Graph &startGraph) {
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

    cout << "Initial: ["<<startGraph.startI<<", "<<startGraph.startJ<<"]"<<endl;
    startGraph.print("Initial");

    // BFS to obtain a certain number of tasks
    queue<Graph *> *initialGraphs = doBFS(startGraph, 4);

    cout << "Initial graphs size: " << initialGraphs->size() << endl;
    int size = initialGraphs->size();
    for (int i = 0; i < size; i++) {
        Graph* graph =initialGraphs->front();
        cout << "Initial: ["<<graph->startI<<", "<<graph->startJ<<"]"<<endl;
        graph->print(to_string(i)+"  ");

        initialGraphs->pop();
    }
//    #pragma omp parallel shared(bestGraph)
//    {
//        #pragma omp single
//        {
//            doSearchRecOMP(new Graph(startGraph));
//        }
//    }
//
//    printBest(bestGraph);
//    delete bestGraph;
}

Graph loadProblem(string filename) {
    ifstream f;
    f.open(filename);

    if (f.is_open()) {
        int nodesCount;
        f >> nodesCount;
        cout << "Nodes: " << nodesCount << endl;

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


int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Enter input file name." << endl;
        return 1;
    }

    char *fn = argv[1];
    cout << "Loading input from file " << fn << endl;

    Graph graph = loadProblem(fn);
    doSearch(graph);

    return 0;
}