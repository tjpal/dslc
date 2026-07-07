module;

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

export module Scanner.DFA;

import Scanner.DFAAcceptingState;

namespace scanner {
    export class DFACaptureAction final {
    public:
        enum class Type {
            Start,
            End
        };

        DFACaptureAction(
            Type type,
            std::uint32_t regexID,
            std::uint32_t groupID,
            std::optional<std::string> name
        ) : type(type), regexID(regexID), groupID(groupID), name(std::move(name)) {}

        Type getType() const {
            return type;
        }

        std::uint32_t getGroupID() const {
            return groupID;
        }

        std::uint32_t getRegexID() const {
            return regexID;
        }

        const std::optional<std::string>& getName() const {
            return name;
        }

    private:
        Type type;
        std::uint32_t regexID = 0;
        std::uint32_t groupID = 0;
        std::optional<std::string> name;
    };

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

        DFA(const std::vector<std::vector<std::uint32_t>>& transitionTable,
            const std::vector<DFAAcceptingState>& acceptingStates,
            const std::vector<char>& alphabet,
            const std::vector<std::vector<std::vector<DFACaptureAction>>>& transitionCaptureActions) noexcept :
            transitionTable(transitionTable),
            acceptingStates(acceptingStates),
            alphabet(alphabet),
            transitionCaptureActions(transitionCaptureActions) {
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

        const std::vector<std::uint32_t>& getAcceptingIdsRef(std::uint32_t state) const {
            static const std::vector<std::uint32_t> emptyIds;
            if (state >= acceptingStates.size()) {
                return emptyIds;
            }

            return acceptingStates[state].getNfaIds();
        }

        std::vector<std::uint32_t> getAcceptingIds(std::uint32_t state) const {
            return getAcceptingIdsRef(state);
        }

        const std::vector<DFACaptureAction>& getCaptureActions(
            std::uint32_t state,
            std::uint32_t symbolIndex
        ) const {
            static const std::vector<DFACaptureAction> emptyActions;
            if (state >= transitionCaptureActions.size() || symbolIndex >= transitionCaptureActions[state].size()) {
                return emptyActions;
            }

            return transitionCaptureActions[state][symbolIndex];
        }

    private:
        std::vector<std::vector<std::uint32_t>> transitionTable;
        std::vector<DFAAcceptingState> acceptingStates;
        std::vector<char> alphabet;
        std::vector<std::vector<std::vector<DFACaptureAction>>> transitionCaptureActions;
    };
} // namespace scanner
