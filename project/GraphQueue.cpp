//
// Created by Martin on 08.03.2017.
//

#include "GraphQueue.h"

void GraphQueue::put(Graph *graph) {
    graphQueue.push(graph);
}

Graph *GraphQueue::take() {
    Graph *taken = graphQueue.front();
    graphQueue.pop();
    return taken;
}

bool GraphQueue::empty() {
    return graphQueue.empty();
}
