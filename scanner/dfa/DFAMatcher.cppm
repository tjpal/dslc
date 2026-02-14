module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <set>
#include <string>
#include <utility>

export module Scanner.DFAMatcher;

import Scanner.DFA;
import Scanner.SpecialSymbols;

namespace scanner {
    export class DFAMatcher final {
    public:
        explicit DFAMatcher(DFA inputDFA) : dfa(std::move(inputDFA)) {
            symbolToIndex.fill(INVALID_SYMBOL_INDEX);
            const auto& alphabet = dfa.getAlphabet();

            std::size_t anySymbolIndex = 0;
            bool hasAnySymbol = false;
            for (std::size_t index = 0; index < alphabet.size(); ++index) {
                if (alphabet[index] == AnySymbol) {
                    anySymbolIndex = index;
                    hasAnySymbol = true;
                    break;
                }
            }

            if (hasAnySymbol) {
                symbolToIndex.fill(static_cast<std::uint16_t>(anySymbolIndex));
            }

            for (std::size_t index = 0; index < alphabet.size(); ++index) {
                const char symbol = alphabet[index];
                if (symbol == AnySymbol) {
                    continue;
                }

                symbolToIndex[static_cast<unsigned char>(symbol)] = static_cast<std::uint16_t>(index);
            }
        }

        bool match(const std::string& input) const {
            if (dfa.getStateCount() == 0) {
                return false;
            }

            std::size_t currentState = 0;

            for (const char symbol : input) {
                const auto symbolIndex = symbolToIndex[static_cast<unsigned char>(symbol)];
                if (symbolIndex == INVALID_SYMBOL_INDEX) {
                    return false;
                }

                currentState = static_cast<std::size_t>(dfa.getNextState(currentState, symbolIndex));
            }

            return dfa.isAcceptingState(currentState);
        }

        std::set<std::uint32_t> getMatchingIDs(const std::string& input) const {
            std::set<std::uint32_t> acceptingIds;
            if (dfa.getStateCount() == 0) {
                return acceptingIds;
            }

            std::size_t currentState = 0;

            for (const char symbol : input) {
                const auto symbolIndex = symbolToIndex[static_cast<unsigned char>(symbol)];
                if (symbolIndex == INVALID_SYMBOL_INDEX) {
                    break;
                }

                currentState = static_cast<std::size_t>(dfa.getNextState(currentState, symbolIndex));

                if (dfa.isAcceptingState(currentState)) {
                    const auto& ids = dfa.getAcceptingIdsRef(currentState);
                    acceptingIds.insert(ids.begin(), ids.end());
                }
            }

            return acceptingIds;
        }

    private:
        static constexpr std::size_t SYMBOL_TABLE_SIZE = 256;
        static constexpr std::uint16_t INVALID_SYMBOL_INDEX = std::numeric_limits<std::uint16_t>::max();

        DFA dfa;
        std::array<std::uint16_t, SYMBOL_TABLE_SIZE> symbolToIndex{};
    };
} // namespace scanner
