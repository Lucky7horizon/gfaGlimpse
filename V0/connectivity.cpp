#include "connectivity.h"


Connectivity::Connectivity(int vertexNumber) {
    nodeNumber = vertexNumber;
    vertexMap = std::vector <long long> (1,0);
}


Connectivity::~Connectivity() {}


void Connectivity::initGraph() {
    sccNumber = 0;
    cut = 0;
    dfsNumber = 0;
    graph = std::vector < std::set <int> > (nodeNumber + 1, std::set <int> ());
    isCut = std::vector <bool> (nodeNumber + 1, 0);
    visNum = std::vector <long long> (nodeNumber + 1, 0);
    low = std::vector <long long> (nodeNumber + 1, 0);
    sccNum = std::vector <long long> (nodeNumber + 1, 0);
    scc = std::stack <long long> ();
    sccList = std::vector < std::vector <int> > ();
}


void Connectivity::tarjan(int fiNode, bool mode) { //mode为0表示判断割点，为1表示判断割边
    visNum[fiNode] = low[fiNode] = ++ dfsNumber;
    scc.push(fiNode);

    // 手动模拟dfs栈
    std::stack <int> dfsStack = std::stack <int> ();
    dfsStack.push(fiNode);

    while (!dfsStack.empty()) {
        int nowNode = dfsStack.top();

        for (auto nxtNode: graph[nowNode]) {
            if (! visNum[nxtNode]) {
                visNum[nxtNode] = low[nxtNode] = ++ dfsNumber;
                scc.push(nxtNode);
                dfsStack.push(nxtNode);
                break;
            }
        }

        if (nowNode == dfsStack.top()) {
            for (auto nxtNode: graph[nowNode]) {
                if (visNum[nxtNode] > visNum[nowNode]) {
                    low[nowNode] = std::min(low[nowNode], low[nxtNode]);
                    if (mode == 0) {
                        if (low[nxtNode] >= low[nowNode] && nowNode != fiNode) {
                            isCut[nowNode] = 1;
                        }
                    } else {
                        if (low[nxtNode] > low[nowNode] && nowNode != fiNode) {
                            isCut[nowNode] = 1;
                        }
                    }
                    
                } else if (!sccNum[nxtNode]) {
                    low[nowNode] = std::min(low[nowNode], visNum[nxtNode]);
                }
            }

            if (visNum[nowNode] == low[nowNode]) {
                std::vector <int> tmp = std::vector <int> ();
                sccNumber ++;
                while (! scc.empty()) {
                    int sccNode = scc.top();
                    scc.pop();
                    sccNum[sccNode] = sccNumber;
                    tmp.push_back(sccNode);
                    if(sccNode == nowNode) break;
                }
                sccList.push_back(tmp);
            }
            dfsStack.pop();
        }
    }
}


void Connectivity::findComponent(const BiedgedGraph& biedgedGraph) {
    initGraph();
    for (int vID = 1; vID <= nodeNumber; vID ++) {
        for (auto e: biedgedGraph.edge[vID]) {
            int nxtNode = e.to;
            graph[vID].insert(nxtNode);
        }
    }

    for (int vID = 1; vID <= nodeNumber; vID ++) {
        if (! visNum[vID]) {
            tarjan(vID, 1);
        }
    }
}


void Connectivity::edgeCompress(const DiGraph& diGraph) {
    reVertexMap = std::vector <long long> ((nodeNumber << 1) + 1, 0);
    int newVertexID = 1;

    for (auto vertex: diGraph.vertexVal) {
        int vID = vertex.first;
        vertexMap.push_back(vID);
        reVertexMap[vID] = newVertexID ++;
    }
}


void Connectivity::findWCC(const DiGraph& diGraph) {
    initGraph();
    for (auto vertex: diGraph.vertexVal) {
        int vID = vertex.first;
        for (auto e: diGraph.edge[vID]) {
            graph[reVertexMap[vID]].insert(reVertexMap[e]);
            graph[reVertexMap[e]].insert(reVertexMap[vID]);
        }
    }

    for (int vID = 1; vID <= nodeNumber; vID ++) {
        if (! visNum[vID]) {
            tarjan(vID, 1);
        }
    }
}


void Connectivity::findSCC(const DiGraph& diGraph) {
    initGraph();
    for (auto vertex: diGraph.vertexVal) {
        int vID = vertex.first;
        for (auto e: diGraph.edge[vID]) {
            graph[reVertexMap[vID]].insert(reVertexMap[e]);
        }
    }

    for (int vID = 1; vID <= nodeNumber; vID ++) {
        if (! visNum[vID]) {
            tarjan(vID, 1);
        }
    }
}


void Connectivity::SCC2Subgraph(const DiGraph& diGraph, std::vector <DiGraph>& subgraphList) {
    for (auto SCC: sccList) {
        if (SCC.size() == 1) continue;

        DiGraph subGraph = DiGraph();
        subGraph.edge = std::vector <std::set <long long> > (SCC.size() + 1, std::set <long long>());
        std::map <int, int> nodeId = std::map <int, int>();
        int id = 0;

        for (auto node: SCC) {
            long long initNode = vertexMap[node];
            if (! diGraph.edge[initNode].empty()) {
                if (! nodeId.count(initNode)) {
                    nodeId[initNode] = ++ id;
                    subGraph.vertexVal[nodeId[initNode]] = diGraph.vertexVal.at(initNode);
                    subGraph.map2RawGraph[nodeId[initNode]] = initNode;
                }

                for (auto nxtNode: diGraph.edge[initNode]) {
                    long long nxtNodeMap = reVertexMap[nxtNode];
                    if (sccNum[nxtNodeMap] == sccNum[node]) {
                        if (! nodeId.count(nxtNode)) {
                            nodeId[nxtNode] = ++ id;
                            subGraph.vertexVal[nodeId[nxtNode]] = diGraph.vertexVal.at(nxtNode);
                            subGraph.map2RawGraph[nodeId[nxtNode]] = nxtNode;
                        }

                        subGraph.edge[nodeId[initNode]].insert(nodeId[nxtNode]);
                    }
                }
            }
        }
        subGraph.vertexNumber = id;
        subgraphList.push_back(subGraph);
    }
}


void Connectivity::SCC2Subgraph(const BiedgedGraph& biedgedGraph, std::vector <BiedgedGraph>& subgraphList) {
    for (auto SCC: sccList) {
        if (SCC.size() == 1) continue;

        BiedgedGraph subGraph = BiedgedGraph();
        subGraph.edge = std::vector <std::set <Biedge> > (SCC.size() + 1, std::set <Biedge>());
        std::map <int, int> nodeId = std::map <int, int>();
        int id = 0;

        for (auto node: SCC) {
            long long initNode = node;
            if (! biedgedGraph.edge[initNode].empty()) {
                if (! nodeId.count(initNode)) {
                    nodeId[initNode] = ++ id;
                    subGraph.map2RawGraph[nodeId[initNode]] = initNode;
                }

                for (auto nxt: biedgedGraph.edge[initNode]) {
                    int nxtNode = nxt.to;
                    if (sccNum[nxtNode] == sccNum[node]) {
                        if (! nodeId.count(nxtNode)) {
                            nodeId[nxtNode] = ++ id;
                            subGraph.map2RawGraph[nodeId[nxtNode]] = nxtNode;
                        }

                        Biedge newEdge = Biedge(nodeId[nxtNode], nxt.value);
                        subGraph.edge[nodeId[initNode]].insert(newEdge);
                    }
                }
            }
        }
        subGraph.vertexNumber = id;
        subgraphList.push_back(subGraph);
    }
}


void Connectivity::print2File(const std::string& outputFile) {
    // T. B. C.

}