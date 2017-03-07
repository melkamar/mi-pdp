//
// Created by Martin on 07.03.2017.
//

#ifndef PROJECT_GRAPHSTACK_H
#define PROJECT_GRAPHSTACK_H


#include "SearchStructure.h"
#include <stack>

using namespace std;

class GraphStack: public SearchStructure {
public:
    void put(Graph *graph) override;
    Graph *take() override;

    bool empty() override;

private:
    stack<Graph*> graphStack;
};


#endif //PROJECT_GRAPHSTACK_H
