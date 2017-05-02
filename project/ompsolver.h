//
// Created by Martin on 18.04.2017.
//

#ifndef PROJECT_OMPSOLVER_H
#define PROJECT_OMPSOLVER_H

#include "Graph.h"
#include <deque>

namespace ompsolver {
    std::deque<Graph *> *generateInitialStates(Graph &startGraph, int requestedSize);

    Graph *doSearchOpenMP(Graph &startGraph, int threadCount);

    void doSearchDFS(Graph *graph);

    bool incrementEdgeIndex(int &i, int &j, int nodes);
}

#endif //PROJECT_OMPSOLVER_H
