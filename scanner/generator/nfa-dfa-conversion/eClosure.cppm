module;

#include <cstddef>
#include <cstdint>
#include <vector>

export module Scanner.eClosure;

import Scanner.NFA;
import Scanner.StateSet;

namespace scanner {
    export class eClosure final {
    public:
        static StateSet compute(const NFA& nfa, const std::vector<std::uint32_t>& initialStates) {
            if (initialStates.empty()) {
                StateSet emptyClosure;
                emptyClosure.lock();
                return emptyClosure;
            }

            StateSet closure;
            closure.reserve(initialStates.size());
            std::vector<std::uint32_t> workList;
            workList.reserve(initialStates.size());

            auto addState = [&](std::uint32_t stateID) {
                if (!closure.tryAddState(stateID)) {
                    return;
                }

                workList.push_back(stateID);
            };

            for (const std::uint32_t state : initialStates) {
                addState(state);
            }

            while (!workList.empty()) {
                const std::uint32_t currentStateID = workList.back();
                workList.pop_back();

                const auto& node = nfa.getNodeByID(currentStateID);

                for (const auto& edge : node.getEdges()) {
                    if (!edge.isEpsilonTransition()) {
                        continue;
                    }

                    addState(edge.getEndpointID());
                }
            }

            closure.lock();
            return closure;
        }

        static StateSet compute(
            const std::vector<std::vector<std::uint32_t>>& epsilonTransitions,
            const std::vector<std::uint32_t>& initialStates
        ) {
            if (initialStates.empty()) {
                StateSet emptyClosure;
                emptyClosure.lock();
                return emptyClosure;
            }

            StateSet closure;
            closure.reserve(initialStates.size());
            std::vector<std::uint32_t> workList;
            workList.reserve(initialStates.size());

            auto addState = [&](std::uint32_t stateID) {
                if (!closure.tryAddState(stateID)) {
                    return;
                }

                workList.push_back(stateID);
            };

            for (const std::uint32_t state : initialStates) {
                addState(state);
            }

            while (!workList.empty()) {
                const std::uint32_t currentStateID = workList.back();
                workList.pop_back();

                const std::size_t transitionIndex = static_cast<std::size_t>(currentStateID);
                if (transitionIndex >= epsilonTransitions.size()) {
                    continue;
                }

                for (const std::uint32_t endpointID : epsilonTransitions[transitionIndex]) {
                    addState(endpointID);
                }
            }

            closure.lock();
            return closure;
        }
    };
} // namespace scanner
