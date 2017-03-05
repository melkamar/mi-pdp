//
// Created by Martin on 01.03.2017.
//

#ifndef PROJECT_GRAPH_H
#define PROJECT_GRAPH_H

#include <string>

class Graph {
public:
    Graph(int nodes);
    Graph(const Graph &other);
    virtual ~Graph();

    void setAdjacency(int i, int j, bool value);
    void print(std::string prefix) const;

    int getEdgesCount();
    short isBipartiteOrConnected();
    bool isAdjacent(int i, int j);

    int nodes;
    int startI, startJ;
    int edgesCount;

    static int copyConstrCalled;
private:
    bool **adjacency;
};


#endif //PROJECT_GRAPH_H
