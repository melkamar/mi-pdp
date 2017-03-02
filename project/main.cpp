#include <iostream>
#include <fstream>
#include <vector>
#include <list>
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

void doDFS(Graph &startGraph) {
    int graphsCount = 1;
    Graph *bestGraph = NULL;

    stack<Graph *> graphStack;
    stack<Graph *> tempStack; // used for rearranging the order of graphs in the "real" stack

    graphStack.push(new Graph(startGraph)); // make a copy of the graph so that it can be safely deleted during dfs

    cout << "::DFS:: starting." << endl;
    while (!graphStack.empty()) {
        Graph *graph = graphStack.top();
        graphStack.pop();

        if (graph->isBipartite() && (!bestGraph || graph->getEdgesCount() > bestGraph->getEdgesCount())) {
            if (bestGraph)
                delete bestGraph;
            bestGraph = new Graph(*graph);
        }

//        cout << "::DFS::   Processing graph:" << endl;
//        graph->print("::DFS::     ");
//        cout << endl;


        // Generate neighboring graphs by removing one edge from this one.
        // Start with the [startI, startJ] edge in the adjacency matrix.
        int i = graph->startI;
        int j = graph->startJ;
        bool valid; // If true, the obtained [i, j] indices point at a valid ID of edge to remove
        do {
            valid = incrementEdgeIndex(i, j, graph->nodes);
            cout << "::DFS::      Trying edge index [" << i << "," << j << "]" << endl;
            if (valid && graph->adjacency[i][j]) {
                // if [i,j] are valid indices and the edge they point at
                // are at is present -> create a new graph by removing the edge
                graphsCount++;

                Graph *newGraph = new Graph(*graph);
                newGraph->setAdjacency(i, j, false);
                newGraph->setAdjacency(j, i, false);
                newGraph->startI = i;
                newGraph->startJ = j;

                // Uncomment following line if the walk-order does not matter
                graphStack.push(newGraph);
//                tempStack.push(newGraph);

                cout << "::DFS::         Edge was present, creating a new graph." << endl;
//                newGraph->print("::DFS::       ");
            }
        } while (valid);

//        while (!tempStack.empty()) {
//            Graph *tempGraph = tempStack.top();
//            graphStack.push(tempGraph);
//            tempStack.pop();
//        }

        delete graph;
        cout << "::DFS::   iteration done. Adding graphs to stack." << endl;
    }
    cout << "::DFS:: complete. Graphs seen: " << graphsCount;
    cout << endl;

    if (bestGraph) {
        cout << "Best graph edges count: " << bestGraph->getEdgesCount() << endl;
        bestGraph->print("BEST: ");
    } else {
        cout << "No best graph found. This should not happen!" << endl;
        delete bestGraph;
        exit(1);
    }

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
    cout << "Loading input from file " << fn << endl;

    Graph graph = loadProblem(fn);
    doDFS(graph);

//    cout << "Is bipartite? " << graph.isBipartite() << endl;
//    cout << "Edges: " << graph.getEdgesCount() << endl;

    return 0;
}