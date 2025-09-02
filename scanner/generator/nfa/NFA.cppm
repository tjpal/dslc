module;

#include <ranges>
#include <vector>

export module Scanner.NFA;

export import :NFAEdge;
export import :NFANode;

namespace scanner {
    export class NFA {
    public:
        NFA() = default;
        NFA(NFA&& other) noexcept : nodes(std::move(other.nodes)) {
        }

        void addNode(NFANode&& node) {
            nodes.push_back(std::move(node));
        }

        NFANode& getNodeByID(std::uint32_t id) {
            const auto it = std::ranges::find_if(nodes, [id](const NFANode& node) { return node.getNodeID() == id; } );

            if (it == nodes.end())
                throw std::out_of_range("NFA node does not exist");

            return *it;
        }

        auto operator=(NFA&& other) noexcept -> NFA& {
            nodes = std::move(other.nodes);
            return *this;
        }

    private:
        std::vector<NFANode> nodes;
    };
} // namespace scanner