module;

#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

export module Scanner.DFAMatcher;

import Scanner.DFA;
import Scanner.SpecialSymbols;

namespace scanner {
    export class DFAMatcher final {
    public:
        explicit DFAMatcher(DFA inputDFA) : dfa(std::move(inputDFA)) {
            const auto& alphabet = dfa.getAlphabet();
            symbolToIndex.reserve(alphabet.size());

            for (std::size_t index = 0; index < alphabet.size(); ++index) {
                const char symbol = alphabet[index];
                if (symbol == AnySymbol) {
                    anySymbolIndex = index;
                    continue;
                }

                symbolToIndex.emplace(symbol, index);
            }
        }

        bool match(const std::string& input) const {
            if (dfa.getStateCount() == 0) {
                return false;
            }

            std::size_t currentState = 0;

            for (const char symbol : input) {
                const auto it = symbolToIndex.find(symbol);
                if (it != symbolToIndex.end()) {
                    currentState = static_cast<std::size_t>(dfa.getNextState(currentState, it->second));
                    continue;
                }

                if (anySymbolIndex.has_value()) {
                    currentState = static_cast<std::size_t>(dfa.getNextState(currentState, *anySymbolIndex));
                    continue;
                }

                return false;
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
                std::optional<std::size_t> symbolIndex;
                const auto it = symbolToIndex.find(symbol);
                if (it != symbolToIndex.end()) {
                    symbolIndex = it->second;
                } else if (anySymbolIndex.has_value()) {
                    symbolIndex = anySymbolIndex;
                } else {
                    break;
                }

                currentState = static_cast<std::size_t>(dfa.getNextState(currentState, *symbolIndex));

                if (dfa.isAcceptingState(currentState)) {
                    std::vector<std::uint32_t> ids = dfa.getAcceptingIds(currentState);
                    acceptingIds.insert_range(ids);
                }
            }

            return acceptingIds;
        }

    private:
        DFA dfa;
        std::unordered_map<char, std::size_t> symbolToIndex;
        std::optional<std::size_t> anySymbolIndex;
    };
} // namespace scanner
