module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

export module Scanner.DFAMatcher;

import Scanner.DFA;
import Scanner.SpecialSymbols;

namespace scanner {
    export using CaptureMap = std::unordered_map<std::string, std::vector<std::string>>;

    export class RegexMatchResult final {
    public:
        RegexMatchResult(std::uint32_t regexID, CaptureMap captures)
            : regexID(regexID), captures(std::move(captures)) {}

        std::uint32_t getRegexID() const {
            return regexID;
        }

        const CaptureMap& getCaptures() const {
            return captures;
        }

    private:
        std::uint32_t regexID = 0;
        CaptureMap captures;
    };

    export class MatchingResult final {
    public:
        explicit MatchingResult(std::set<std::uint32_t> matchingIDs) : matchingIDs(std::move(matchingIDs)) {}

        MatchingResult(std::set<std::uint32_t> matchingIDs, std::vector<RegexMatchResult> matches)
            : matchingIDs(std::move(matchingIDs)), matches(std::move(matches)) {}

        const std::set<std::uint32_t>& getMatchingIDs() const {
            return matchingIDs;
        }

        const std::vector<RegexMatchResult>& getMatches() const {
            return matches;
        }

    private:
        std::set<std::uint32_t> matchingIDs;
        std::vector<RegexMatchResult> matches;
    };

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

        MatchingResult getMatches(const std::string& input, bool includeCaptures = false) const {
            std::set<std::uint32_t> acceptingIds;
            if (dfa.getStateCount() == 0) {
                return MatchingResult(std::move(acceptingIds));
            }

            std::unordered_map<CaptureKey, ActiveCapture, CaptureKeyHash> activeCaptures;
            std::unordered_map<std::uint32_t, CaptureMap> currentCapturesByRegexID;
            std::unordered_map<std::uint32_t, CaptureMap> acceptedCapturesByRegexID;
            std::size_t currentState = 0;

            for (std::size_t position = 0; position < input.size(); ++position) {
                const char symbol = input[position];
                const auto symbolIndex = symbolToIndex[static_cast<unsigned char>(symbol)];
                if (symbolIndex == INVALID_SYMBOL_INDEX) {
                    break;
                }

                const auto& captureActions = dfa.getCaptureActions(currentState, symbolIndex);
                if (includeCaptures) {
                    startCaptures(captureActions, position, activeCaptures);
                }

                currentState = static_cast<std::size_t>(dfa.getNextState(currentState, symbolIndex));

                if (includeCaptures) {
                    endCaptures(captureActions, input, position + 1, activeCaptures, currentCapturesByRegexID);
                }

                if (dfa.isAcceptingState(currentState)) {
                    const auto& ids = dfa.getAcceptingIdsRef(currentState);
                    acceptingIds.insert(ids.begin(), ids.end());
                    if (includeCaptures) {
                        for (const std::uint32_t id : ids) {
                            acceptedCapturesByRegexID[id] = currentCapturesByRegexID[id];
                        }
                    }
                }
            }

            if (!includeCaptures) {
                return MatchingResult(std::move(acceptingIds));
            }

            std::vector<RegexMatchResult> matches;
            matches.reserve(acceptingIds.size());
            for (const std::uint32_t id : acceptingIds) {
                matches.emplace_back(id, acceptedCapturesByRegexID[id]);
            }

            return MatchingResult(std::move(acceptingIds), std::move(matches));
        }

    private:
        struct CaptureKey {
            std::uint32_t regexID = 0;
            std::uint32_t groupID = 0;

            bool operator==(const CaptureKey& other) const {
                return regexID == other.regexID && groupID == other.groupID;
            }
        };

        struct CaptureKeyHash final {
            std::size_t operator()(const CaptureKey& key) const {
                return (static_cast<std::size_t>(key.regexID) * 1315423911u) ^ key.groupID;
            }
        };

        struct ActiveCapture {
            std::size_t startPosition = 0;
            std::optional<std::string> name;
        };

        static constexpr std::size_t SYMBOL_TABLE_SIZE = 256;
        static constexpr std::uint16_t INVALID_SYMBOL_INDEX = std::numeric_limits<std::uint16_t>::max();

        static void startCaptures(
            const std::vector<DFACaptureAction>& captureActions,
            const std::size_t position,
            std::unordered_map<CaptureKey, ActiveCapture, CaptureKeyHash>& activeCaptures
        ) {
            for (const auto& captureAction : captureActions) {
                if (captureAction.getType() != DFACaptureAction::Type::Start) {
                    continue;
                }

                activeCaptures[{captureAction.getRegexID(), captureAction.getGroupID()}] = {
                    position,
                    captureAction.getName()
                };
            }
        }

        static void endCaptures(
            const std::vector<DFACaptureAction>& captureActions,
            const std::string& input,
            const std::size_t endPosition,
            std::unordered_map<CaptureKey, ActiveCapture, CaptureKeyHash>& activeCaptures,
            std::unordered_map<std::uint32_t, CaptureMap>& capturesByRegexID
        ) {
            for (const auto& captureAction : captureActions) {
                if (captureAction.getType() != DFACaptureAction::Type::End) {
                    continue;
                }

                const CaptureKey key{captureAction.getRegexID(), captureAction.getGroupID()};
                const auto active = activeCaptures.find(key);
                if (active == activeCaptures.end()) {
                    continue;
                }

                const auto& name = captureAction.getName();
                if (name.has_value()) {
                    capturesByRegexID[captureAction.getRegexID()][*name].push_back(
                        input.substr(active->second.startPosition, endPosition - active->second.startPosition)
                    );
                }

                activeCaptures.erase(active);
            }
        }

        DFA dfa;
        std::array<std::uint16_t, SYMBOL_TABLE_SIZE> symbolToIndex{};
    };
} // namespace scanner
