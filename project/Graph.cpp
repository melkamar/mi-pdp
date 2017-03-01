//
// Created by Martin on 01.03.2017.
//

#include <iostream>
#include "Graph.h"
#include <queue>

using namespace std;

Graph::Graph(int nodes) {
    this->nodes = nodes;
    adjacency = new bool *[nodes];
    for (int i = 0; i < nodes; ++i) {
        adjacency[i] = new bool[nodes];
    }

    startI = 0;
    startJ = 0;
}

Graph::Graph(const Graph &other) { // Copy constructor
    nodes = other.nodes;
    startI = other.startI;
    startJ = other.startJ;

    adjacency = new bool *[nodes];
    for (int i = 0; i < nodes; ++i) {
        adjacency[i] = new bool[nodes];
        for (int j = 0; j < nodes; ++j) {
            adjacency[i][j] = other.adjacency[i][j];
        }
    }
}

Graph::~Graph() {
    for (int i = 0; i < nodes; ++i) {
        delete[] adjacency[i];
    }
    delete[] adjacency;
}

void Graph::setAdjacency(int i, int j, bool value) {
    adjacency[i][j] = value;
}

void Graph::print(string prefix) {
    // print header
    cout << prefix << "     ";
    for (int j = 0; j < nodes; ++j) {
        cout << "(" << j << ") ";
    }
    cout << endl;
    // done header

    for (int i = 0; i < nodes; ++i) {
        cout << prefix << "(" << i << ") |"; // left-column indices
        for (int j = 0; j < nodes; ++j) {
            cout << (adjacency[i][j] ? " 1" : " 0") << " |";
        }
        cout << endl;
    }
}

/**
 * Return opposite color: 0 -> 1, 1 -> 0. Any other value throws an exception.
 * @param color
 * @return
 */
char getOppositeColor(char color) {
    if (color == 0) return 1;
    else if (color == 1) return 0;
    else throw invalid_argument("getOppositeColor: Received color " + color);
}

/**
 * Check if graph is bipartite (i.e. colorable with using two colors)
 *
 * All nodes are assigned one of three values:
 *  -1: initial value, node has not been colored yet.
 *   0: first color.
 *   1: second color.
 *
 * Algorithm:
 *  - Loop through all nodes.
 *    - If node is already colored, let it be, continue next loop. (It was already processed)
 *    - If node is not colored, add it to a (empty) queue, color it to 0.
 *
 *    - While there is a node in the queue, use it and check all of its neighbors:
 *      - If neighbor is not colored, color it to opposite color and add to queue
 *      - If neighbor is already colored, and the color is the same as current color, graph is not bipartite. Ret false.
 *
 *
 * @return True if graph is bipartite, false if not.
 */
bool Graph::isBipartite() {
    char nodesColors[nodes];
    for (int i = 0; i < nodes; ++i) {
        nodesColors[i] = -1;
    }

    queue<int> nodesToColorQueue;
    for (int i = 0; i < nodes; ++i) { // This outer loop is necessary for disjoint graphs.
        if (nodesColors[i] != -1)
            continue; // this node is already colored, do not add it to queue.
        else {
            nodesToColorQueue.push(i);
            nodesColors[i] = 0; // Set the default color for the initial node.
        }

        while (!nodesToColorQueue.empty()) {
            int nodeIdx = nodesToColorQueue.front();
            nodesToColorQueue.pop();

            for (int j = 0; j < nodes; ++j) {
                // color all neighbors to the opposite color
                if (adjacency[nodeIdx][j] == 0)
                    continue;
                else {
                    if (nodesColors[j] == -1) { // neighbor not colored yet -> color it
                        nodesColors[j] = getOppositeColor(nodesColors[nodeIdx]);
                        nodesToColorQueue.push(j);
                    } else if (nodesColors[j] == nodesColors[nodeIdx]) {
                        // if my color is the same as neighbors, graph is not bipartite
                        return false;
                    }
                }

            }
        }
    }

    return true;
}

