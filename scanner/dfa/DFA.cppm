module;

#include <cstddef>
#include <cstdint>
#include <vector>

export module Scanner.DFA;

import Scanner.DFAAcceptingState;

namespace scanner {
    export class DFA {
    public:
        DFA() = default;
        DFA(DFA&& other) noexcept = default;
        DFA(const std::vector<std::vector<std::uint32_t>>& transitionTable,
            const std::vector<DFAAcceptingState>& acceptingStates,
            const std::vector<char>& alphabet) noexcept :
            transitionTable(transitionTable),
            acceptingStates(acceptingStates),
            alphabet(alphabet) {
        }

        std::uint32_t getStateCount() const {
            return transitionTable.size();
        }

        std::uint32_t getAlphabetSize() const {
            return alphabet.size();
        }

        const std::vector<char>& getAlphabet() const {
            return alphabet;
        }

        std::uint32_t getNextState(std::uint32_t state, std::uint32_t symbolIndex) const {
            return transitionTable[state][symbolIndex];
        }

        bool isAcceptingState(std::uint32_t state) const {
            return state < acceptingStates.size() && acceptingStates[state].getIsAccepting();
        }

        std::vector<std::uint32_t> getAcceptingIds(std::uint32_t state) const {
            if (state >= acceptingStates.size()) {
                return {};
            }

            return acceptingStates[state].getNfaIds();
        }

    private:
        std::vector<std::vector<std::uint32_t>> transitionTable;
        std::vector<DFAAcceptingState> acceptingStates;
        std::vector<char> alphabet;
    };
} // namespace scanner
