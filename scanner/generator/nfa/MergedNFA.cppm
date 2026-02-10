module;

#include <vector>

export module Scanner.MergedNFA;

import Scanner.NFA;

namespace scanner {
    export class AcceptingState {
    public:
        AcceptingState(std::uint32_t nodeId, std::uint32_t nfaID) : nodeId(nodeId), nfaID(nfaID) {}

        std::uint32_t getNodeID() const {
            return nodeId;
        }

        std::uint32_t getNFAID() const {
            return nfaID;
        }

    private:
        std::uint32_t nodeId;
        std::uint32_t nfaID;
    };

    export class MergedNFA {
    public:
        MergedNFA(const std::vector<NFA>& nfas, NFANodeFactory& nodeFactory) {
            std::vector<NFANode> mergedNodes;
            mergeNodes(nfas, mergedNodes, nodeFactory);

            auto& rootNode = mergedNodes[0];

            std::uint32_t nfaIndex = 0;
            for (const auto& nfa: nfas) {
                rootNode.addEdge(NFAEdge::epsilon(nfa.getStartNodeID()));
                acceptingStates.emplace_back(nfa.getAcceptingNodeID(), nfaIndex);
                nfaIndex++;
            }

            mergedNFA = NFA(rootNode, std::move(mergedNodes), rootNode);
        }

        const NFA& getMergedNFA() const {
            return mergedNFA;
        }

        const std::vector<AcceptingState>& getAcceptingStates() const {
            return acceptingStates;
        }

    private:
        void mergeNodes(const std::vector<NFA>& nfas, std::vector<NFANode>& mergedNodes, NFANodeFactory& nodeFactory) {
            std::uint32_t numNodes = 0;
            for (const auto& nfa : nfas) {
                numNodes += nfa.getNodes().size();
            }

            mergedNodes.reserve(numNodes + 1); // +1 for the new root node

            mergedNodes.emplace_back(nodeFactory.createNode());

            for (const auto& nfa : nfas) {
                std::ranges::copy(nfa.getNodes(), std::back_inserter(mergedNodes));
            }
        }

    private:
        NFA mergedNFA;
        std::vector<AcceptingState> acceptingStates;
    };
}
