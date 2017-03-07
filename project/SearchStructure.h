//
// Created by Martin on 07.03.2017.
//

#ifndef PROJECT_SEARCHSTRUCTURE_H
#define PROJECT_SEARCHSTRUCTURE_H


#include "Graph.h"

class SearchStructure {
public:
    virtual void put(Graph* graph) = 0;
    virtual Graph* take() = 0;
    virtual bool empty() = 0;
};


#endif //PROJECT_SEARCHSTRUCTURE_H
