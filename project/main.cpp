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

    stack<Graph *> graphStack;
    stack<Graph *> tempStack; // used for rearranging the order of graphs in the "real" stack

    graphStack.push(&startGraph);

    cout << "::DFS:: starting." << endl;
    while (!graphStack.empty()) {
        Graph *graph = graphStack.top();
        graphStack.pop();
        cout << "::DFS::   Processing graph:" << endl;
        graph->print("::DFS::     ");
        cout << endl;

        int i = graph->startI;
        int j = graph->startJ;
        bool valid;
        do {
            valid = incrementEdgeIndex(i, j, graph->nodes);
            cout << "::DFS::      Trying edge index [" << i << "," << j << "]" << endl;
            if (valid && graph->adjacency[i][j]) { // the edge we are at is present -> create a new graph by removing it
                graphsCount++;

                Graph *newGraph = new Graph(*graph);
                newGraph->setAdjacency(i, j, false);
                newGraph->setAdjacency(j, i, false);
                newGraph->startI = i;
                newGraph->startJ = j;

//                graphStack.push(newGraph);
                tempStack.push(newGraph);

                cout << "::DFS::         Edge was present, creating a new graph." << endl;
//                newGraph->print("::DFS::       ");
            }
        } while (valid);

        delete graph;

        while (!tempStack.empty()){
            Graph * tempGraph = tempStack.top();
            graphStack.push(tempGraph);
            tempStack.pop();
        }

        cout << "::DFS::   iteration done. Adding graphs to stack." << endl;
    }
    cout << "::DFS:: complete. Graphs seen: "<<graphsCount;
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
    fn = "d:\\cvut-checkouted\\mi-pdp\\project\\input\\three";
    fn = "d:\\cvut-checkouted\\mi-pdp\\project\\input\\small";
    cout << "Loading input from file " << fn << endl;

    Graph graph = loadProblem(fn);
    doDFS(graph);

//    int i = 0;
//    int j = 0;
//    int count = 0;
//    while (true) {
//        cout << "Increment (" << i << ", " << j << ") ->";
//        bool res = incrementEdgeIndex(i, j, 5);
//        cout << res << "(" << i << ", " << j << ") - position " << ++count << endl;
//
//        if (!res) break;
//    }

    return 0;
}