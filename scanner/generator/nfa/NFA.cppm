module;

#include <memory>
#include <vector>

export module Scanner.NFA;

export import :NFAEdge;
export import :NFANode;

namespace scanner {
    export class NFA {
    public:
        NFA() = default;
        NFA(NFA&& other) noexcept :  nodes(std::move(other.nodes)) {
        }

        void addNode(NFANode&& node) {
            nodes.push_back(std::move(node));
        }

        void startStartNode(std::uint32_t startNodeIndex) {
            this->startNodeIndex = startNodeIndex;
        }

        NFANode& getNodeByID(std::uint32_t id) {
            if (id >= nodes.size()) {
                throw std::out_of_range("Node ID out of range");
            }
            return nodes[id];
        }

        auto operator=(NFA&& other) noexcept -> NFA& {
            nodes = std::move(other.nodes);
            return *this;
        }

    private:
        std::uint32_t startNodeIndex = 0;
        std::vector<NFANode> nodes;
    };
} // namespace scanner