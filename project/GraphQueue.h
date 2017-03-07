//
// Created by Martin on 08.03.2017.
//

#ifndef PROJECT_GRAPHQUEUE_H
#define PROJECT_GRAPHQUEUE_H


#include "SearchStructure.h"
#include <queue>

using namespace std;

class GraphQueue : public SearchStructure {
public:
    void put(Graph *graph) override;

    Graph *take() override;

    bool empty() override;
private:
    queue<Graph*> graphQueue;
};


#endif //PROJECT_GRAPHQUEUE_H
