module;

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>
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
            std::vector<std::vector<std::vector<DFACaptureAction>>> transitionCaptureActions;
            std::vector<DFAAcceptingState> acceptingStates;
            std::deque<std::size_t> workQueue;
            subsets.reserve(128);
            subsetIndex.reserve(128);
            transitionTable.reserve(128);
            transitionCaptureActions.reserve(128);
            acceptingStates.reserve(128);

            const auto addSubset = [&](StateSet subset) -> std::size_t {
                if (const auto it = subsetIndex.find(subset); it != subsetIndex.end()) {
                    return it->second;
                }

                const std::size_t index = subsets.size();
                subsets.push_back(std::move(subset));
                subsetIndex.emplace(subsets.back(), index);
                transitionTable.emplace_back(alphabet.size());
                transitionCaptureActions.emplace_back(alphabet.size());
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
                    collectCaptureActions(
                        nfa,
                        subsets[current],
                        subsets[target],
                        symbol,
                        transitionCaptureActions[current][symbolIndex]
                    );
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

            return DFA(transitionTable, acceptingStates, alphabet, transitionCaptureActions);
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

        static void collectCaptureActions(
            const NFA& nfa,
            const StateSet& source,
            const StateSet& target,
            const char symbol,
            std::vector<DFACaptureAction>& captureActions
        ) {
            captureActions.clear();
            collectCaptureStarts(nfa, source, symbol, captureActions);
            collectCaptureEnds(nfa, target, captureActions);
        }

        static void collectCaptureStarts(
            const NFA& nfa,
            const StateSet& source,
            const char symbol,
            std::vector<DFACaptureAction>& captureActions
        ) {
            for (const std::uint32_t stateID : source.getLockedStates()) {
                for (const auto& edge : nfa.getNodeByID(stateID).getEdges()) {
                    if (!isCaptureAction(edge, NFAEdge::CaptureAction::Start)) {
                        continue;
                    }

                    if (!source.contains(edge.getEndpointID())) {
                        continue;
                    }

                    if (canReachSymbol(nfa, edge.getEndpointID(), symbol, source)) {
                        addCaptureActionOrThrow(captureActions, toDFACaptureAction(edge));
                    }
                }
            }
        }

        static void collectCaptureEnds(
            const NFA& nfa,
            const StateSet& target,
            std::vector<DFACaptureAction>& captureActions
        ) {
            for (const std::uint32_t stateID : target.getLockedStates()) {
                for (const auto& edge : nfa.getNodeByID(stateID).getEdges()) {
                    if (!isCaptureAction(edge, NFAEdge::CaptureAction::End)) {
                        continue;
                    }

                    if (target.contains(edge.getEndpointID())) {
                        addCaptureActionOrThrow(captureActions, toDFACaptureAction(edge));
                    }
                }
            }
        }

        static bool canReachSymbol(
            const NFA& nfa,
            std::uint32_t startStateID,
            const char symbol,
            const StateSet& subset
        ) {
            std::set<std::uint32_t> visited;
            std::vector<std::uint32_t> workList = {startStateID};

            while (!workList.empty()) {
                const std::uint32_t currentStateID = workList.back();
                workList.pop_back();

                if (!visited.insert(currentStateID).second) {
                    continue;
                }

                for (const auto& edge : nfa.getNodeByID(currentStateID).getEdges()) {
                    if (!edge.isEpsilonTransition()) {
                        if (edgeMatchesSymbol(edge, symbol)) {
                            return true;
                        }

                        continue;
                    }

                    if (subset.contains(edge.getEndpointID())) {
                        workList.push_back(edge.getEndpointID());
                    }
                }
            }

            return false;
        }

        static bool edgeMatchesSymbol(const NFAEdge& edge, const char symbol) {
            if (symbol == AnySymbol) {
                return edge.matchesAnySymbol();
            }

            if (edge.matchesAnySymbol()) {
                return true;
            }

            for (const char transitionCharacter : edge.getCharacter()) {
                if (transitionCharacter == symbol) {
                    return true;
                }
            }

            return false;
        }

        static bool isCaptureAction(const NFAEdge& edge, const NFAEdge::CaptureAction action) {
            return edge.isEpsilonTransition() && edge.hasCaptureAction() && edge.getCaptureAction() == action;
        }

        static DFACaptureAction toDFACaptureAction(const NFAEdge& edge) {
            switch (edge.getCaptureAction()) {
            case NFAEdge::CaptureAction::Start:
                return {DFACaptureAction::Type::Start, edge.getCaptureGroupID(), edge.getCaptureGroupName()};
            case NFAEdge::CaptureAction::End:
                return {DFACaptureAction::Type::End, edge.getCaptureGroupID(), edge.getCaptureGroupName()};
            case NFAEdge::CaptureAction::None:
                throw std::runtime_error("NFA edge does not have a capture action");
            }

            throw std::runtime_error("Unsupported capture action");
        }

        static void addCaptureActionOrThrow(
            std::vector<DFACaptureAction>& captureActions,
            const DFACaptureAction& captureAction
        ) {
            for (const auto& existing : captureActions) {
                if (existing.getType() != captureAction.getType()) {
                    continue;
                }

                if (existing.getGroupID() == captureAction.getGroupID() &&
                    existing.getName() == captureAction.getName()) {
                    return;
                }

                throw std::runtime_error("Conflicting capture actions in DFA transition");
            }

            captureActions.push_back(captureAction);
        }

        static GenerationStatistics::Duration durationSince(const std::chrono::steady_clock::time_point start) {
            return std::chrono::duration_cast<GenerationStatistics::Duration>(std::chrono::steady_clock::now() - start);
        }
    };
} // namespace scanner
