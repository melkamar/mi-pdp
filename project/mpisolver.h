//
// Created by Martin on 18.04.2017.
//

#ifndef PROJECT_MPISOLVER_H
#define PROJECT_MPISOLVER_H

#include "Graph.h"

namespace mpisolver {
    void runMPI(int argc, char **argv, Graph &graph);
    void logMPI(std::string text);
    void myMPI_SendGraph(Graph *graph, int targetProcessId, int tag);
    Graph *myMPI_ReceiveGraph(int source);
    void MPI_ProcessMaster(Graph &startGraph, int processCount, int initialGraphsCount);
    void MPI_ProcessSlave();
    void printBest(Graph *bestGraph);
}

#endif //PROJECT_MPISOLVER_H
