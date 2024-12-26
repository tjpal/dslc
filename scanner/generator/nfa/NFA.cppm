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
        NFA(const std::shared_ptr<NFANode> &startNode, const std::vector<std::shared_ptr<NFANode>> &nodes,
            const std::vector<std::shared_ptr<NFANode>> &acceptingNodes)
            : startNode(startNode), nodes(nodes), acceptingNodes(acceptingNodes) {}

        NFA(NFA &&other) noexcept
            : startNode(std::move(other.startNode)), nodes(std::move(other.nodes)),
              acceptingNodes(std::move(other.acceptingNodes)) {}

        const std::shared_ptr<NFANode> &getStartNode() const { return startNode; }
        const std::vector<std::shared_ptr<NFANode>> &getNodes() const { return nodes; }
        const std::vector<std::shared_ptr<NFANode>> &getAcceptingNodes() const { return acceptingNodes; }

        auto operator=(NFA &&other) noexcept -> NFA& {
            startNode = std::move(other.startNode);
            nodes = std::move(other.nodes);
            acceptingNodes = std::move(other.acceptingNodes);

            return *this;
        }

      private:
        std::shared_ptr<NFANode> startNode;
        std::vector<std::shared_ptr<NFANode>> nodes;
        std::vector<std::shared_ptr<NFANode>> acceptingNodes;
    };
} // namespace scanner