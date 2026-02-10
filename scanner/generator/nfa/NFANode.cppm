module;

#include <cstdint>
#include <vector>

export module Scanner.NFA:NFANode;

import :NFAEdge;

namespace scanner {
    export class NFANode {
    public:
        NFANode(NFANode&&) = default;
        NFANode(const NFANode&) = default;

        void setAcceptingStateID(std::uint32_t acceptingStateID) { this->acceptingStateID = acceptingStateID; }
        std::uint32_t getAcceptingStateID() { return acceptingStateID; }

        void addEdge(const NFAEdge& edge) { edges.push_back(edge); }
        const std::vector<NFAEdge>& getEdges() const { return edges; }

        std::uint32_t getNodeID() const { return nodeID; }

        NFANode& operator=(NFANode&&) = default;

    private:
        friend class NFANodeFactory;

        explicit NFANode(std::uint32_t nodeID) : nodeID(nodeID) {}

    private:
        std::uint32_t acceptingStateID = 0;
        std::vector<NFAEdge> edges;
        std::uint32_t nodeID = 0;
    };

} // namespace scanner
