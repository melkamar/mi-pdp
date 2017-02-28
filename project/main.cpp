#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <stack>

using namespace std;

class Graph {
public:
    Graph(int nodes) {
        this->nodes = nodes;
        adjacency = new bool *[nodes];
        for (int i = 0; i < nodes; ++i) {
            adjacency[i] = new bool[nodes];
        }

        startI = 0;
        startJ = 0;
    }

    Graph(const Graph &other) { // Copy constructor
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

    void setAdjacency(int i, int j, bool value) {
        adjacency[i][j] = value;
    }

    void print() {
        for (int i = 0; i < nodes; ++i) {
            for (int j = 0; j < nodes; ++j) {
                cout << (adjacency[i][j] ? "1" : "0");
            }
            cout << endl;
        }
    }

private:
    int nodes;
    bool **adjacency;
    int startI, startJ;
};

//class BFSSolver {
//public:
//
//private:
//};

//void BFSSolver::findOptimal() {
//
//}

void doDFS(Graph &startGraph) {
    stack<Graph*> graphStack;
    graphStack.push(&startGraph);

    while (!graphStack.empty()){
        Graph* graph = graphStack.top();
        graphStack.pop();
        cout << "::DFS:: Processing graph:"<<endl;
        graph->print();



        cout << "::DFS:: done.";
    }
}

Graph loadProblem(string filename) {
    ifstream f;
    f.open(filename);

    if (f.is_open()) {
        int nodesCount;
        f >> nodesCount;
        cout << "Nodes: " << nodesCount << endl;

        Graph graph(nodesCount);

        for (int i = 0; i < nodesCount; i++) {
            string line;
            f >> line;

            for (int j = 0; j < nodesCount; ++j) {
                if (line[j] == '0') {
                    graph.setAdjacency(i, j, false);
                } else {
                    graph.setAdjacency(i, j, true);
                }
            }
        }

        f.close();
        return graph;

    } else {
        cout << "Error opening the file." << endl;
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Enter input file name." << endl;
        return 1;
    }

    char *fn = argv[1];
    cout << "Loading input from file " << fn << endl;

    Graph graph = loadProblem(fn);
    doDFS(graph);

    return 0;
}