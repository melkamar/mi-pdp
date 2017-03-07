//
// Created by Martin on 07.03.2017.
//

#include "GraphStack.h"

void GraphStack::put(Graph *graph) {
    graphStack.push(graph);
}

Graph *GraphStack::take() {
    Graph* taken = graphStack.top();
    graphStack.pop();
    return taken;
}

bool GraphStack::empty() {
    return graphStack.empty();
}
