module;

#include <cstddef>
#include <cstdint>
#include <vector>

export module Scanner.DFA;

namespace scanner {
    export class DFA {
    public:
        DFA() = default;
        DFA(DFA&& other) noexcept = default;
        DFA(const std::vector<std::vector<std::uint32_t>>& transitionTable,
            const std::vector<bool>& acceptingStates,
            const std::vector<char>& alphabet) noexcept :
            transitionTable(transitionTable),
            acceptingStates(acceptingStates),
            alphabet(alphabet) {
        }

        std::size_t getStateCount() const {
            return transitionTable.size();
        }

        std::size_t getAlphabetSize() const {
            return alphabet.size();
        }

        const std::vector<char>& getAlphabet() const {
            return alphabet;
        }

        std::uint32_t getNextState(std::size_t state, std::size_t symbolIndex) const {
            return transitionTable[state][symbolIndex];
        }

        bool isAcceptingState(std::size_t state) const {
            return state < acceptingStates.size() && acceptingStates[state];
        }

    private:
        std::vector<std::vector<std::uint32_t>> transitionTable;
        std::vector<bool> acceptingStates;
        std::vector<char> alphabet;
    };
} // namespace scanner
