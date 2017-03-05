//
// Created by Martin on 01.03.2017.
//

#include <iostream>
#include "Graph.h"
#include <queue>

using namespace std;

int Graph::copyConstrCalled = 0;

Graph::Graph(int nodes) {
    this->nodes = nodes;
    adjacency = new bool *[nodes];
    for (int i = 0; i < nodes; ++i) {
        adjacency[i] = new bool[nodes];
        for (int j = 0; j < nodes; ++j) {
            adjacency[i][j] = false;
        }
    }

    startI = 0;
    startJ = 0;
    edgesCount = 0;
}

Graph::Graph(const Graph &other) { // Copy constructor
    copyConstrCalled++;
    nodes = other.nodes;
    startI = other.startI;
    startJ = other.startJ;
    edgesCount = other.edgesCount;

    adjacency = new bool *[nodes];
    for (int i = 0; i < nodes; ++i) {
        adjacency[i] = new bool[nodes];
        copy(&other.adjacency[i][0], &other.adjacency[i][0] + other.nodes, &adjacency[i][0]);
    }
}

Graph::~Graph() {
    for (int i = 0; i < nodes; ++i) {
        delete[] adjacency[i];
    }
    delete[] adjacency;
}

void Graph::setAdjacency(int i, int j, bool value) {
    if (adjacency[i][j] != value) {
        adjacency[i][j] = value;
        adjacency[j][i] = value;
        if (value) edgesCount++;
        else edgesCount--;
    }
}

bool Graph::isAdjacent(int i, int j) {
    return adjacency[i][j];
}

int Graph::getEdgesCount() {
    return edgesCount;
}

void Graph::print(string prefix) const {
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
 *    - While there is a node in the queue, use it and check all of its neighbors:
 *      - If neighbor is not colored, color it to opposite color and add to queue
 *      - If neighbor is already colored, and the color is the same as current color, graph is not bipartite. Ret false.
 *
 * @return -1 ~ not connected
 *          0 ~ connected, not bipartite
 *          1 ~ connected, bipartite
 */
short Graph::isBipartiteOrConnected() {
    bool nodesSeen[nodes];
    char nodesColors[nodes];
    for (int i = 0; i < nodes; ++i) {
        nodesColors[i] = -1;
        nodesSeen[i] = false;
    }

    bool bipartite = true;

    queue<int> nodesQueue;

    nodesColors[0] = 0;
    nodesQueue.push(0);

    while (!nodesQueue.empty()) {
        int nodeIdx = nodesQueue.front();
        nodesQueue.pop();

        for (int j = 0; j < nodes; ++j) {
            // color all neighbors to the opposite color
            if (adjacency[nodeIdx][j] == 0)
                continue;
            else {
                if (!nodesSeen[j]) { // neighbor not seen yet -> color it
//                if (nodesColors[j] == -1) { // neighbor not colored yet -> color it
                    nodesColors[j] = getOppositeColor(nodesColors[nodeIdx]);
                    nodesQueue.push(j);
                } else if (nodesColors[j] == nodesColors[nodeIdx]) {
                    // if my color is the same as neighbors, graph is not bipartite
                    bipartite = false;
                }

                nodesSeen[j] = true;
            }

        }
    }

    // After the flooding, if any node is left not colored, it must be disjoint
    for (int k = 0; k < nodes; ++k) {
        if (!nodesSeen[k]) {
            return -1;
        }
    }


    return (bipartite ? 1 : 0);
}
