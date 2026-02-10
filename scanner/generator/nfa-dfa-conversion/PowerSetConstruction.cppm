module;

#include <algorithm>
#include <cstddef>
#include <deque>
#include <functional>
#include <set>
#include <unordered_map>
#include <vector>

export module Scanner.PowerSetConstruction;

import Scanner.DFA;
import Scanner.DFAAcceptingState;
import Scanner.NFA;
import Scanner.MergedNFA;
import Scanner.StateSet;
import Scanner.eClosure;
import Scanner.SpecialSymbols;

namespace scanner {
    export class PowerSetConstruction final {
    public:
        static DFA convert(const MergedNFA & mergedNFA) {
            const auto createAccepting = [&](const StateSet& subset) -> DFAAcceptingState {
                std::vector<std::uint32_t> nfaIds;
                for (const AcceptingState& acceptingState : mergedNFA.getAcceptingStates()) {
                    if (subset.contains(acceptingState.getNodeID())) {
                        nfaIds.emplace_back(acceptingState.getNFAID());
                    }
                }

                return {!nfaIds.empty(), nfaIds};
            };

            return convert(mergedNFA.getMergedNFA(), createAccepting);
        }

        static DFA convert(const NFA& nfa) {
            const std::uint32_t acceptingNodeId = nfa.getAcceptingNodeID();

            const auto createAccepting = [&](const StateSet& subset) -> DFAAcceptingState {
                bool isAccepting = subset.contains(acceptingNodeId);
                return { isAccepting, { 0 } };
            };

            return convert(nfa, createAccepting);
        }

    private:
        static DFA convert(const NFA& nfa, const std::function<DFAAcceptingState(const StateSet&)>& makeAccepting) {
            nfa.lock();

            const std::vector<char> alphabet = collectAlphabet(nfa);

            struct StateSubsetHash final {
                std::size_t operator()(const StateSet& subset) const {
                    return static_cast<std::size_t>(subset.getHash());
                }
            };

            std::vector<StateSet> subsets;
            std::unordered_map<StateSet, std::size_t, StateSubsetHash> subsetIndex;
            std::vector<std::vector<std::uint32_t>> transitionTable;
            std::vector<DFAAcceptingState> acceptingStates;
            std::deque<std::size_t> workQueue;

            const auto addSubset = [&](StateSet subset) -> std::size_t {
                if (const auto it = subsetIndex.find(subset); it != subsetIndex.end()) {
                    return it->second;
                }

                const std::size_t index = subsets.size();
                subsets.push_back(std::move(subset));
                subsetIndex.emplace(subsets.back(), index);

                transitionTable.emplace_back(alphabet.size());

                acceptingStates.push_back(makeAccepting(subsets.back()));

                workQueue.push_back(index);

                return index;
            };

            const std::vector<std::uint32_t> startStates = { nfa.getStartNodeID() };
            const auto startClosure = eClosure::compute(nfa, startStates);
            addSubset(startClosure);

            while (!workQueue.empty()) {
                const std::size_t current = workQueue.front();
                workQueue.pop_front();

                for (std::size_t symbolIndex = 0; symbolIndex < alphabet.size(); ++symbolIndex) {
                    const char symbol = alphabet[symbolIndex];
                    const std::vector<std::uint32_t> reachable = moveOnSymbol(nfa, subsets[current], symbol);

                    const auto nextClosure = eClosure::compute(nfa, reachable);

                    const std::size_t target = addSubset(nextClosure);
                    transitionTable[current][symbolIndex] = static_cast<std::uint32_t>(target);
                }
            }

            if (subsets.empty()) {
                return DFA({}, {}, alphabet);
            }

            return DFA(transitionTable, acceptingStates, alphabet);
        }

        static std::vector<char> collectAlphabet(const NFA& nfa) {
            std::set<char> alphabetSet;

            for (const auto& node : nfa.getNodes()) {
                for (const auto& edge : node.getEdges()) {
                    if (edge.isEpsilonTransition()) {
                        continue;
                    }

                    if (edge.matchesAnySymbol()) {
                        alphabetSet.insert(AnySymbol);
                        continue;
                    }

                    for (const char character : edge.getCharacter()) {
                        alphabetSet.insert(character);
                    }
                }
            }

            return {alphabetSet.begin(), alphabetSet.end()};
        }

        static std::vector<std::uint32_t> moveOnSymbol(const NFA& nfa, const StateSet& subset, char symbol) {
            std::vector<std::uint32_t> reachable;

            for (const std::uint32_t stateID : subset.getLockedStates()) {
                for (const auto& node = nfa.getNodeByID(stateID); const auto& edge : node.getEdges()) {
                    if (edge.isEpsilonTransition()) {
                        continue;
                    }

                    if (symbol == AnySymbol) {
                        if (edge.matchesAnySymbol()) {
                            reachable.push_back(edge.getEndpointID());
                        }

                        continue;
                    }

                    if (edge.matchesAnySymbol()) {
                        continue;
                    }

                    for (const char transitionCharacter : edge.getCharacter()) {
                        if (transitionCharacter == symbol) {
                            reachable.push_back(edge.getEndpointID());
                            break;
                        }
                    }
                }
            }

            return reachable;
        }
    };
} // namespace scanner
