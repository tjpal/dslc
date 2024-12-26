module;

#include <cstdint>
#include <vector>

export module Scanner.NFA:NFANode;

import :NFAEdge;

namespace scanner {
    export class NFANode {
      public:
        void setAcceptingStateID(std::uint32_t acceptingStateID) { this->acceptingStateID = acceptingStateID; }
        std::uint32_t getAcceptingStateID() { return acceptingStateID; }

        void addEdge(const NFAEdge &edge) { edges.push_back(edge); }
        std::vector<NFAEdge> getEdges() const { return edges; }

      private:
        std::uint32_t acceptingStateID = 0;
        std::vector<NFAEdge> edges;
    };
} // namespace scanner