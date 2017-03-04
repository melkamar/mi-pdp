#include <iostream>
#include <fstream>
#include <stack>

#include "Graph.h"

using namespace std;


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
        cout << "Best graph edges count: " << bestGraph->getEdgesCount() << endl;
//        bestGraph->print("BEST: ");
    } else {
        cout << "No best graph found. This should not happen!" << endl;
        delete bestGraph;
        exit(1);
    }
}

void doDFS(Graph &startGraph) {
    int graphsCount = 1;
    Graph *bestGraph = NULL;

    if (startGraph.isBipartiteOrConnected() == 1) {
        printBest(&startGraph);
        return;
    }

    stack<Graph *> graphStack;
    graphStack.push(new Graph(startGraph)); // make a copy of the graph so that it can be safely deleted during dfs

    while (!graphStack.empty()) {
        Graph *graph = graphStack.top();
        graphStack.pop();

        short bipOrConn = graph->isBipartiteOrConnected();
        if (bipOrConn == 1 && (!bestGraph || (graph->getEdgesCount() > bestGraph->getEdgesCount()))) {
            if (bestGraph) delete bestGraph;
            bestGraph = graph;
            continue;
        }

        if (bipOrConn == -1) {
            delete graph;
            continue;
        }

        if (graph->getEdgesCount() < graph->nodes ||
            (bestGraph && graph->getEdgesCount() <= bestGraph->getEdgesCount())) {
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
            if (valid && graph->isAdjacent(i,j)) {
                // if [i,j] are valid indices and the edge they point at
                // are at is present -> create a new graph by removing the edge
                graphsCount++;

                Graph *newGraph = new Graph(*graph);
                newGraph->setAdjacency(i, j, false);
                newGraph->setAdjacency(j, i, false);
                newGraph->startI = i;
                newGraph->startJ = j;

                // Uncomment following line if the walk-order does not matter
                if ((!bestGraph || newGraph->getEdgesCount() > bestGraph->getEdgesCount()) &&
                    newGraph->isBipartiteOrConnected() != -1 &&
                    newGraph->getEdgesCount() >= newGraph->nodes -1 ) {
                    graphStack.push(newGraph);
                } else {
                    delete newGraph;
                }
            }
        } while (valid);


        delete graph;
    }
//    cout << "::DFS:: complete. Graphs seen: " << graphsCount;
    cout << endl;

    printBest(bestGraph);

    delete bestGraph;
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
    if (argc != 2) {
        cout << "Enter input file name." << endl;
        return 1;
    }

    char *fn = argv[1];
//    fn = "d:\\cvut-checkouted\\mi-pdp\\project\\input\\three";
//    fn = "d:\\cvut-checkouted\\mi-pdp\\project\\input\\bipartite\\disjoint";
//    fn = "d:\\cvut-checkouted\\mi-pdp\\project\\input\\small";
//    fn = "d:\\cvut-checkouted\\mi-pdp\\project\\input\\7";
//    fn = "d:\\cvut-checkouted\\mi-pdp\\project\\input\\a";
    cout << "Loading input from file " << fn << endl;

    Graph graph = loadProblem(fn);
    doDFS(graph);

    return 0;
}