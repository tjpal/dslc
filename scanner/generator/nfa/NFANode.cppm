module;

#include <atomic>
#include <cstdint>
#include <vector>

export module Scanner.NFA:NFANode;

import :NFAEdge;

namespace scanner {
    export class NFANode {
    public:
        NFANode() = default;
        NFANode(NFANode&&) = default;
        NFANode(const NFANode&) = default;

        void setAcceptingStateID(std::uint32_t acceptingStateID) { this->acceptingStateID = acceptingStateID; }
        std::uint32_t getAcceptingStateID() { return acceptingStateID; }

        void addEdge(const NFAEdge& edge) { edges.push_back(edge); }
        std::vector<NFAEdge> getEdges() const { return edges; }

        std::uint32_t getNodeID() const { return nodeID; }

        NFANode& operator=(NFANode&&) = default;

    private:
        static std::uint32_t generateNodeID() {
            return nextNodeID.fetch_add(1, std::memory_order_relaxed);
        }

    private:
        std::uint32_t acceptingStateID = 0;
        std::vector<NFAEdge> edges;
        std::uint32_t nodeID { generateNodeID() };

        static std::atomic<std::uint32_t> nextNodeID;
    };

    std::atomic<std::uint32_t> NFANode::nextNodeID = 0;
} // namespace scanner