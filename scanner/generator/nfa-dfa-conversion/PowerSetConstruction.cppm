module;

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <set>
#include <unordered_map>
#include <vector>

export module Scanner.PowerSetConstruction;

import Scanner.DFA;
import Scanner.DFAAcceptingState;
import Scanner.GenerationStatistics;
import Scanner.MergedNFA;
import Scanner.NFA;
import Scanner.SpecialSymbols;
import Scanner.StateSet;
import Scanner.eClosure;

namespace scanner {
    export class PowerSetConstruction final {
    public:
        static DFA convert(const MergedNFA& mergedNFA, GenerationStatistics* statistics = nullptr) {
            std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> acceptingIdsByNodeID;
            acceptingIdsByNodeID.reserve(mergedNFA.getAcceptingStates().size());
            for (const AcceptingState& acceptingState : mergedNFA.getAcceptingStates()) {
                acceptingIdsByNodeID[acceptingState.getNodeID()].push_back(acceptingState.getNFAID());
            }

            const auto createAccepting = [&](const StateSet& subset) -> DFAAcceptingState {
                std::vector<std::uint32_t> nfaIds;
                for (const std::uint32_t stateID : subset.getLockedStates()) {
                    if (const auto it = acceptingIdsByNodeID.find(stateID); it != acceptingIdsByNodeID.end()) {
                        nfaIds.insert(nfaIds.end(), it->second.begin(), it->second.end());
                    }
                }

                return {!nfaIds.empty(), nfaIds};
            };

            return convert(mergedNFA.getMergedNFA(), createAccepting, statistics);
        }

        static DFA convert(const NFA& nfa, GenerationStatistics* statistics = nullptr) {
            const std::uint32_t acceptingNodeId = nfa.getAcceptingNodeID();

            const auto createAccepting = [&](const StateSet& subset) -> DFAAcceptingState {
                const bool isAccepting = subset.contains(acceptingNodeId);
                return {isAccepting, {0}};
            };

            return convert(nfa, createAccepting, statistics);
        }

    private:
        struct StateSubsetHash final {
            std::size_t operator()(const StateSet& subset) const {
                return static_cast<std::size_t>(subset.getHash());
            }
        };

        template <typename MakeAcceptingFn>
        static DFA convert(const NFA& nfa, MakeAcceptingFn&& makeAccepting, GenerationStatistics* statistics) {
            using Clock = std::chrono::steady_clock;
            const bool collectPowerSetStats = statistics != nullptr && statistics->isEnabled();

            nfa.lock();

            const auto alphabetCollectionStart = collectPowerSetStats ? Clock::now() : Clock::time_point{};
            const std::vector<char> alphabet = collectAlphabet(nfa);
            if (collectPowerSetStats) {
                statistics->setPowerSetAlphabetCollectionDuration(durationSince(alphabetCollectionStart));
            }

            std::vector<StateSet> subsets;
            std::unordered_map<StateSet, std::size_t, StateSubsetHash> subsetIndex;
            std::vector<std::vector<std::uint32_t>> transitionTable;
            std::vector<DFAAcceptingState> acceptingStates;
            std::deque<std::size_t> workQueue;
            subsets.reserve(128);
            subsetIndex.reserve(128);
            transitionTable.reserve(128);
            acceptingStates.reserve(128);

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

            const std::vector<std::uint32_t> startStates = {nfa.getStartNodeID()};
            const auto startClosure = eClosure::compute(nfa, startStates);
            addSubset(startClosure);

            std::vector<std::uint32_t> reachable;
            std::size_t transitionCount = 0;
            const auto mainLoopStart = collectPowerSetStats ? Clock::now() : Clock::time_point{};

            while (!workQueue.empty()) {
                const std::size_t current = workQueue.front();
                workQueue.pop_front();

                const std::size_t subsetSize = subsets[current].getLockedStates().size();
                if (reachable.capacity() < subsetSize) {
                    reachable.reserve(subsetSize);
                }

                for (std::size_t symbolIndex = 0; symbolIndex < alphabet.size(); ++symbolIndex) {
                    const char symbol = alphabet[symbolIndex];
                    moveOnSymbol(nfa, subsets[current], symbol, reachable);
                    const auto nextClosure = eClosure::compute(nfa, reachable);
                    const std::size_t target = addSubset(nextClosure);
                    transitionTable[current][symbolIndex] = static_cast<std::uint32_t>(target);
                    ++transitionCount;
                }
            }

            if (collectPowerSetStats) {
                statistics->setPowerSetMainLoopDuration(durationSince(mainLoopStart));
                statistics->setPowerSetSubsetCount(subsets.size());
                statistics->setPowerSetTransitionCount(transitionCount);
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

        static void moveOnSymbol(
            const NFA& nfa,
            const StateSet& subset,
            const char symbol,
            std::vector<std::uint32_t>& reachable
        ) {
            reachable.clear();

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
                        reachable.push_back(edge.getEndpointID());
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
        }

        static GenerationStatistics::Duration durationSince(const std::chrono::steady_clock::time_point start) {
            return std::chrono::duration_cast<GenerationStatistics::Duration>(std::chrono::steady_clock::now() - start);
        }
    };
} // namespace scanner
