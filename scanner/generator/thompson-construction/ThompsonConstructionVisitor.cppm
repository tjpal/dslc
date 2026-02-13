module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>
#include <stdexcept>

export module Scanner.ThompsonConstructionVisitor;

import Scanner.NFA;
import Scanner.Regex;
import Scanner.GenerationStatistics;

namespace scanner {
    export class ThompsonConstructionVisitor final : public RegexNodeVisitor {
    public:
        explicit ThompsonConstructionVisitor(NFANodeFactory& nodeFactory, GenerationStatistics* statistics = nullptr)
            : nodeFactory(nodeFactory), statistics(statistics) {}

        void visit(Leaf& leaf) override {
            auto start = nodeFactory.createNode();
            auto end = nodeFactory.createNode();

            if (leaf.isWildcard()) {
                addEdgeTimed(start, NFAEdge::wildcard(end.getNodeID()));
            } else {
                addEdgeTimed(start, NFAEdge(end.getNodeID(), leaf.getCharacters()));
            }

            nfaStack.emplace_back(NFA(start, {std::move(start), std::move(end)}, end));
        }

        void visit(Concatenation& concatenation) override {
            concatenation.getLeft()->accept(*this);
            concatenation.getRight()->accept(*this);

            auto right = popNFA();
            auto left = popNFA();

            addEdgeTimed(getAcceptingNodeTimed(left), NFAEdge::epsilon(right.getStartNodeID()));
            const std::uint32_t leftStartNodeID = left.getStartNodeID();
            const std::uint32_t rightAcceptingNodeID = right.getAcceptingNodeID();

            nfaStack.emplace_back(
                leftStartNodeID,
                mergeNodes(std::move(left), std::move(right)),
                rightAcceptingNodeID
            );
        }

        void visit(Kleene& kleene) override {
            kleene.getKleeneNode()->accept(*this);

            auto child = popNFA();
            auto start = nodeFactory.createNode();
            auto end = nodeFactory.createNode();
            const std::uint32_t startNodeID = start.getNodeID();
            const std::uint32_t endNodeID = end.getNodeID();

            addEdgeTimed(start, NFAEdge::epsilon(end.getNodeID()));
            addEdgeTimed(start, NFAEdge::epsilon(child.getStartNodeID()));

            auto& childEnd = getAcceptingNodeTimed(child);
            addEdgeTimed(childEnd, NFAEdge::epsilon(end.getNodeID()));
            addEdgeTimed(childEnd, NFAEdge::epsilon(start.getNodeID()));

            nfaStack.emplace_back(
                startNodeID,
                mergeNodes(std::move(child), NFA(), std::move(start), std::move(end)),
                endNodeID
            );
        }

        void visit(Plus& plus) override {
            plus.getPlusNode()->accept(*this);

            auto child = popNFA();
            auto start = nodeFactory.createNode();
            auto end = nodeFactory.createNode();
            const std::uint32_t startNodeID = start.getNodeID();
            const std::uint32_t endNodeID = end.getNodeID();

            addEdgeTimed(start, NFAEdge::epsilon(child.getStartNodeID()));

            auto& childEnd = getAcceptingNodeTimed(child);
            addEdgeTimed(childEnd, NFAEdge::epsilon(end.getNodeID()));
            addEdgeTimed(childEnd, NFAEdge::epsilon(start.getNodeID()));

            nfaStack.emplace_back(
                startNodeID,
                mergeNodes(std::move(child), NFA(), std::move(start), std::move(end)),
                endNodeID
            );
        }

        void visit(Optional& optional) override {
            optional.getOptionalNode()->accept(*this);
            NFA child = popNFA();

            auto& start = getStartNodeTimed(child);
            addEdgeTimed(start, NFAEdge::epsilon(child.getAcceptingNodeID()));

            nfaStack.push_back(std::move(child));
        }

        void visit(Union& unionElement) override {
            unionElement.getLeft()->accept(*this);
            unionElement.getRight()->accept(*this);

            auto right = popNFA();
            auto left = popNFA();

            auto start = nodeFactory.createNode();
            addEdgeTimed(start, NFAEdge::epsilon(left.getStartNodeID()));
            addEdgeTimed(start, NFAEdge::epsilon(right.getStartNodeID()));
            const std::uint32_t startNodeID = start.getNodeID();

            auto end = nodeFactory.createNode();
            const std::uint32_t endNodeID = end.getNodeID();
            auto& leftAcceptingNode = getAcceptingNodeTimed(left);
            auto& rightAcceptingNode = getAcceptingNodeTimed(right);

            addEdgeTimed(leftAcceptingNode, NFAEdge::epsilon(end.getNodeID()));
            addEdgeTimed(rightAcceptingNode, NFAEdge::epsilon(end.getNodeID()));

            nfaStack.emplace_back(
                startNodeID,
                mergeNodes(std::move(left), std::move(right), std::move(start), std::move(end)),
                endNodeID
            );
        }

        const NFA& getConstructedNFA() const {
            if (nfaStack.size() != 1) {
                throw std::runtime_error("No unique final NFA found");
            }

            return nfaStack.back();
        }

        NFA&& moveConstructedNFA() {
            if (nfaStack.size() != 1) {
                throw std::runtime_error("No unique final NFA found");
            }

            return std::move(nfaStack.back());
        }

    private:
        NFA popNFA() {
            if (nfaStack.empty()) {
                throw std::runtime_error("NFA stack is empty");
            }

            NFA result = std::move(nfaStack.back());
            nfaStack.pop_back();

            return result;
        }

        std::vector<NFANode> mergeNodes(NFA&& first, NFA&& second) {
            return mergeNodes(std::move(first), std::move(second), 0);
        }

        std::vector<NFANode> mergeNodes(NFA&& first, NFA&& second, NFANode&& extraFirst, NFANode&& extraSecond) {
            auto result = mergeNodes(std::move(first), std::move(second), 2);
            result.push_back(std::move(extraFirst));
            result.push_back(std::move(extraSecond));
            return result;
        }

        std::vector<NFANode> mergeNodes(NFA&& first, NFA&& second, const std::size_t additionalNodeCount) {
            if (statistics != nullptr) {
                statistics->startThompsonMergeNodes();
            }

            std::vector<NFANode> result = first.takeNodes();
            std::vector<NFANode> secondNodes = second.takeNodes();
            result.reserve(result.size() + secondNodes.size() + additionalNodeCount);
            std::ranges::move(secondNodes, std::back_inserter(result));

            if (statistics != nullptr) {
                statistics->endThompsonMergeNodes();
            }

            return result;
        }

        NFANodeFactory& nodeFactory;
        GenerationStatistics* statistics = nullptr;
        std::vector<NFA> nfaStack;

        void addEdgeTimed(NFANode& node, const NFAEdge& edge) const {
            if (statistics != nullptr) {
                statistics->startThompsonEdgeAdd();
            }

            node.addEdge(edge);

            if (statistics != nullptr) {
                statistics->endThompsonEdgeAdd();
            }
        }

        NFANode& getAcceptingNodeTimed(NFA& nfa) const {
            if (statistics != nullptr) {
                statistics->startThompsonNodeLookup();
            }

            auto& result = nfa.getAcceptingNode();

            if (statistics != nullptr) {
                statistics->endThompsonNodeLookup();
            }

            return result;
        }

        NFANode& getStartNodeTimed(NFA& nfa) const {
            if (statistics != nullptr) {
                statistics->startThompsonNodeLookup();
            }

            auto& result = nfa.getStartNode();

            if (statistics != nullptr) {
                statistics->endThompsonNodeLookup();
            }

            return result;
        }
    };

} // namespace scanner
