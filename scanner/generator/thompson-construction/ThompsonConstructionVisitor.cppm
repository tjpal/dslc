module;

#include <memory>
#include <stack>

export module Scanner.ThompsonConstructionVisitor;

import Scanner.NFA;
import Scanner.Regex;

namespace scanner {
    class ThompsonConstructionVisitor : public RegexNodeVisitor {
    public:
        void visit(Leaf& leaf) override {
            auto start = makeNode();
            auto end = makeNode();

            for (char c : leaf.getCharacters()) {
                start->addEdge(NFAEdge(end, c));
            }

            nfaStack.push(NFA(start, {start, end}, {end}));
        }

        void visit(Concatenation& concatenation) override {
            std::ignore = concatenation;

            auto left = popNFA();
            auto right = popNFA();

            uniqueAcceptingNode(left)->addEdge(NFAEdge(right.getStartNode()));

            nfaStack.push(NFA(left.getStartNode(), mergeNodes(left, right, {}), right.getAcceptingNodes()));
        }

        void visit(Kleene& kleene) override {
            std::ignore = kleene;

            auto child = popNFA();
            auto start = makeNode();
            auto end = makeNode();

            start->addEdge(NFAEdge(end));
            start->addEdge(NFAEdge(child.getStartNode()));

            auto childEnd = uniqueAcceptingNode(child);
            childEnd->addEdge(NFAEdge(end));
            childEnd->addEdge(NFAEdge(start));

            nfaStack.push(NFA(start, mergeNodes(child, NFA(), {start, end}), {end}));
        }

        void visit(Optional& optional) override {
            std::ignore = optional;
            NFA child = popNFA();

            auto start = child.getStartNode();
            auto end = uniqueAcceptingNode(child);

            start->addEdge(NFAEdge(end));

            nfaStack.push(std::move(child));
        }

        void visit(Union& unionElement) override {
            std::ignore = unionElement;

            auto left = popNFA();
            auto right = popNFA();

            auto start = makeNode();
            start->addEdge(NFAEdge(left.getStartNode()));
            start->addEdge(NFAEdge(right.getStartNode()));

            auto end = makeNode();
            auto leftAcceptingNode = uniqueAcceptingNode(left);
            auto rightAcceptingNode = uniqueAcceptingNode(right);

            leftAcceptingNode->addEdge(NFAEdge(end));
            rightAcceptingNode->addEdge(NFAEdge(end));

            nfaStack.push(NFA(start, mergeNodes(left, right, {start, end}), {end}));
        }

        const NFA& getConstructedNFA() const {
            if (nfaStack.size() != 1) {
                throw std::runtime_error("No unique final NFA found");
            }

            return nfaStack.top();
        }

    private:
        inline std::shared_ptr<NFANode> makeNode() { return std::make_shared<NFANode>(); }

        std::shared_ptr<NFANode> uniqueAcceptingNode(const NFA& nfa) {
            if (nfa.getAcceptingNodes().size() != 1) {
                throw std::runtime_error("Accepting node is not unique");
            }

            return nfa.getAcceptingNodes()[0];
        }

        NFA popNFA() {
            if (nfaStack.empty()) {
                throw std::runtime_error("NFA stack is empty");
            }

            NFA nfa = std::move(nfaStack.top());
            nfaStack.pop();

            return nfa;
        }

        std::vector<std::shared_ptr<NFANode>> mergeNodes(const NFA& first, const NFA& second,
                                                         const std::vector<std::shared_ptr<NFANode>>& other) {
            std::vector<std::shared_ptr<NFANode>> result;

            result.reserve(first.getNodes().size() + second.getNodes().size() + other.size());

            std::copy(first.getNodes().cbegin(), first.getNodes().cend(), std::back_inserter(result));
            std::copy(second.getNodes().cbegin(), second.getNodes().cend(), std::back_inserter(result));
            std::copy(other.cbegin(), other.cend(), std::back_inserter(result));

            return result;
        }

        std::stack<NFA> nfaStack;
    };

} // namespace scanner