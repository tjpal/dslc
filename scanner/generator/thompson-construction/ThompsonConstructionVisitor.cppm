module;

#include <memory>
#include <stack>
#include <vector>

export module Scanner.ThompsonConstructionVisitor;

import Scanner.NFA;
import Scanner.Regex;

namespace scanner {
    class ThompsonConstructionVisitor final : public RegexNodeVisitor {
    public:
        void visit(Leaf& leaf) override {
            auto start = NFANode();
            auto end = NFANode();

            start.addEdge(NFAEdge(end.getNodeID(), leaf.getCharacters()));

            nfaStack.emplace(NFA(start, {std::move(start), std::move(end)}, end));
        }

        void visit(Concatenation& concatenation) override {
            std::ignore = concatenation;

            auto left = popNFA();
            auto right = popNFA();

            left.getAcceptingNode().addEdge(NFAEdge::epsilon(right.getStartNodeID()));

            nfaStack.emplace(
                left.getStartNode(),
                mergeNodes(left, right, {}),
                right.getAcceptingNode()
            );
        }

        void visit(Kleene& kleene) override {
            std::ignore = kleene;

            auto child = popNFA();
            auto start = NFANode();
            auto end = NFANode();

            start.addEdge(NFAEdge::epsilon(end.getNodeID()));
            start.addEdge(NFAEdge::epsilon(child.getStartNodeID()));

            auto childEnd = child.getAcceptingNode();
            childEnd.addEdge(NFAEdge::epsilon(end.getNodeID()));
            childEnd.addEdge(NFAEdge::epsilon(start.getNodeID()));

            nfaStack.emplace(
                start,
                mergeNodes(child, NFA(), {start, end}),
                end
            );
        }

        void visit(Optional& optional) override {
            std::ignore = optional;
            NFA child = popNFA();

            auto start = child.getStartNode();
            start.addEdge(NFAEdge::epsilon(child.getAcceptingNodeID()));

            nfaStack.push(std::move(child));
        }

        void visit(Union& unionElement) override {
            std::ignore = unionElement;

            auto left = popNFA();
            auto right = popNFA();

            auto start = NFANode();
            start.addEdge(NFAEdge::epsilon(left.getStartNodeID()));
            start.addEdge(NFAEdge::epsilon(right.getStartNodeID()));

            auto end = NFANode();
            auto leftAcceptingNode = left.getAcceptingNode();
            auto rightAcceptingNode = right.getAcceptingNode();

            leftAcceptingNode.addEdge(NFAEdge::epsilon(end.getNodeID()));
            rightAcceptingNode.addEdge(NFAEdge::epsilon(end.getNodeID()));

            nfaStack.push(NFA(start, mergeNodes(left, right, {start, end}), {end}));
        }

        const NFA& getConstructedNFA() const {
            if (nfaStack.size() != 1) {
                throw std::runtime_error("No unique final NFA found");
            }

            return nfaStack.top();
        }

    private:
        NFA&& popNFA() {
            if (nfaStack.empty()) {
                throw std::runtime_error("NFA stack is empty");
            }

            NFA&& nfa = std::move(nfaStack.top());
            nfaStack.pop();

            return std::move(nfa);
        }

        static std::vector<NFANode> mergeNodes(const NFA& first, const NFA& second, const std::vector<NFANode>& other) {
            std::vector<NFANode> result;

            result.reserve(first.getNodes().size() + second.getNodes().size() + other.size());

            std::ranges::copy(first.getNodes(), std::back_inserter(result));
            std::ranges::copy(second.getNodes(), std::back_inserter(result));
            std::ranges::copy(other, std::back_inserter(result));

            return std::move(result);
        }

        std::stack<NFA> nfaStack;
    };

} // namespace scanner