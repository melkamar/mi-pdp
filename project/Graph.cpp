//
// Created by Martin on 01.03.2017.
//

#include <iostream>
#include "Graph.h"

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
        delete adjacency[i];
    }
    delete [] adjacency;
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