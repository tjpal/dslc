module;

#include <cstdint>
#include <vector>

export module Scanner.eClosure;

import Scanner.NFA;
import Scanner.StateSet;

namespace scanner {
    export class eClosure final {
    public:
        static StateSet compute(const NFA& nfa, const std::vector<std::uint32_t>& initialStates) {
            StateSet closure;
            std::vector<std::uint32_t> workList;
            workList.reserve(initialStates.size());

            auto addState = [&](std::uint32_t stateID) {
                if (closure.contains(stateID)) {
                    return;
                }

                closure.addState(stateID);
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

                    const std::uint32_t endpointID = edge.getEndpointID();
                    if (closure.contains(endpointID)) {
                        continue;
                    }

                    addState(endpointID);
                }
            }

            closure.lock();
            return closure;
        }
    };
} // namespace scanner
