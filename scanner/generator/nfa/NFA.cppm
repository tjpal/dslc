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
        NFA(NFA&& other) noexcept = default;
        NFA(const NFANode& startNode, std::vector<NFANode>&& nodes, const NFANode& acceptingNode) noexcept :
            nodes(nodes),
            startNodeID(startNode.getNodeID()),
            acceptingNodeID(acceptingNode.getNodeID()) {
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

        NFANode& getAcceptingNode() {
            return getNodeByID(acceptingNodeID);
        }

        std::uint32_t getAcceptingNodeID() const {
            return acceptingNodeID;
        }

        NFANode& getStartNode() {
            return getNodeByID(startNodeID);
        }

        std::uint32_t getStartNodeID() const {
            return startNodeID;
        }

        const std::vector<NFANode>& getNodes() const {
            return nodes;
        }

        NFA& operator=(NFA&& other) noexcept = default;

    private:
        std::vector<NFANode> nodes;

        std::uint32_t startNodeID = 0;
        std::uint32_t acceptingNodeID = 0;
    };
} // namespace scanner