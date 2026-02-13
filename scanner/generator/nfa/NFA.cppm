module;

#include <algorithm>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

export module Scanner.NFA;

export import :NFAEdge;
export import :NFANode;
export import :NFANodeFactory;

namespace scanner {
    export class NFA {
    public:
        NFA() = default;
        NFA(NFA&& other) noexcept = default;
        NFA(std::uint32_t startNodeID, std::vector<NFANode>&& nodes, std::uint32_t acceptingNodeID) noexcept :
            nodes(std::move(nodes)),
            startNodeID(startNodeID),
            acceptingNodeID(acceptingNodeID) {
        }

        NFA(const NFANode& startNode, std::vector<NFANode>&& nodes, const NFANode& acceptingNode) noexcept :
            nodes(std::move(nodes)),
            startNodeID(startNode.getNodeID()),
            acceptingNodeID(acceptingNode.getNodeID()) {
        }

        void addNode(NFANode&& node) {
            if (isLocked) {
                throw std::runtime_error("Cannot mutate locked NFA");
            }

            nodes.push_back(std::move(node));
        }

        void lock() const {
            if (isLocked) {
                return;
            }

            std::uint32_t maxNodeID = 0;
            bool hasNodes = false;
            for (const auto& node : nodes) {
                hasNodes = true;
                maxNodeID = std::max(maxNodeID, node.getNodeID());
            }

            nodeIndexLookup.clear();
            if (hasNodes) {
                nodeIndexLookup.assign(static_cast<std::size_t>(maxNodeID) + 1, InvalidNodeIndex);
            }

            for (std::size_t index = 0; index < nodes.size(); ++index) {
                const std::uint32_t nodeID = nodes[index].getNodeID();
                const std::size_t lookupIndex = static_cast<std::size_t>(nodeID);
                if (nodeIndexLookup[lookupIndex] != InvalidNodeIndex) {
                    throw std::runtime_error("Duplicate NFA node ID");
                }

                nodeIndexLookup[lookupIndex] = index;
            }

            isLocked = true;
        }

        NFANode& getNodeByID(std::uint32_t id) {
            if (isLocked) {
                throw std::runtime_error("Cannot mutate locked NFA");
            }

            const auto it = std::ranges::find_if(nodes, [id](const NFANode& node) { return node.getNodeID() == id; });

            if (it == nodes.end())
                throw std::out_of_range("NFA node does not exist");

            return *it;
        }

        const NFANode& getNodeByID(std::uint32_t id) const {
            if (isLocked) {
                const std::size_t lookupIndex = static_cast<std::size_t>(id);
                if (lookupIndex >= nodeIndexLookup.size()) {
                    throw std::out_of_range("NFA node does not exist");
                }

                const std::size_t nodeIndex = nodeIndexLookup[lookupIndex];
                if (nodeIndex == InvalidNodeIndex) {
                    throw std::out_of_range("NFA node does not exist");
                }

                return nodes[nodeIndex];
            }

            const auto it = std::ranges::find_if(nodes, [id](const NFANode& node) { return node.getNodeID() == id; });

            if (it == nodes.cend())
                throw std::out_of_range("NFA node does not exist");

            return *it;
        }

        NFANode& getAcceptingNode() {
            return getNodeByID(acceptingNodeID);
        }

        const NFANode& getAcceptingNode() const {
            return getNodeByID(acceptingNodeID);
        }

        std::uint32_t getAcceptingNodeID() const {
            return acceptingNodeID;
        }

        NFANode& getStartNode() {
            return getNodeByID(startNodeID);
        }

        const NFANode& getStartNode() const {
            return getNodeByID(startNodeID);
        }

        std::uint32_t getStartNodeID() const {
            return startNodeID;
        }

        const std::vector<NFANode>& getNodes() const {
            return nodes;
        }

        std::vector<NFANode> takeNodes() {
            if (isLocked) {
                throw std::runtime_error("Cannot mutate locked NFA");
            }

            return std::move(nodes);
        }

        NFA& operator=(NFA&& other) noexcept = default;

    private:
        static constexpr std::size_t InvalidNodeIndex = std::numeric_limits<std::size_t>::max();

        std::vector<NFANode> nodes;
        mutable std::vector<std::size_t> nodeIndexLookup;
        mutable bool isLocked = false;

        std::uint32_t startNodeID = 0;
        std::uint32_t acceptingNodeID = 0;
    };
} // namespace scanner
