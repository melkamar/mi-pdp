//
// Created by Martin on 18.04.2017.
//

#ifndef PROJECT_MPISOLVER_H
#define PROJECT_MPISOLVER_H

#include "Graph.h"

namespace mpisolver {
    void runMPI(int argc, char **argv, Graph &graph, int graphsPerProcess);
    void logMPI(std::string text);
    void sendGraph(Graph *graph, int targetProcessId, int tag, int bestGraphEdgesCount);
    Graph *recvGraph(int source);
    void processMaster(Graph &startGraph, int processCount, int initialGraphsCount);
    void processSlave();
    void printBest(Graph *bestGraph);
    int getRank();
}

#endif //PROJECT_MPISOLVER_H
