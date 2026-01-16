module;

#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

export module Scanner.DFAMatcher;

import Scanner.DFA;

namespace scanner {
    export class DFAMatcher final {
    public:
        explicit DFAMatcher(DFA inputDFA) : dfa(std::move(inputDFA)) {
            const auto& alphabet = dfa.getAlphabet();
            symbolToIndex.reserve(alphabet.size());

            for (std::size_t index = 0; index < alphabet.size(); ++index) {
                symbolToIndex.emplace(alphabet[index], index);
            }
        }

        bool match(const std::string& input) const {
            if (dfa.getStateCount() == 0) {
                return false;
            }

            std::size_t currentState = 0;

            for (const char symbol : input) {
                const auto it = symbolToIndex.find(symbol);
                if (it == symbolToIndex.end()) {
                    return false;
                }

                currentState = static_cast<std::size_t>(dfa.getNextState(currentState, it->second));
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
                const auto it = symbolToIndex.find(symbol);
                if (it == symbolToIndex.end()) {
                    break;
                }

                currentState = static_cast<std::size_t>(dfa.getNextState(currentState, it->second));

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
    };
} // namespace scanner
