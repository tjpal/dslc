module;

#include <chrono>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

export module Scanner.Generator;

import Scanner.RegexParser;
import Scanner.DFA;
import Scanner.ThompsonConstructionVisitor;
import Scanner.NFA;
import Scanner.PowerSetConstruction;
import Scanner.Regex;
import Scanner.MergedNFA;
import Scanner.GenerationStatistics;

namespace scanner {
    namespace {
        long long toMilliseconds(const GenerationStatistics::Duration duration) {
            return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        }

        long long toMicroseconds(const GenerationStatistics::Duration duration) {
            return duration.count();
        }
    } // namespace

    export class Generator {
    public:
        Generator() = default;

        void setProfilingEnabled(const bool enabled) {
            profilingEnabled = enabled;
            statistics.setEnabled(profilingEnabled);
        }

        [[nodiscard]] bool isProfilingEnabled() const {
            return profilingEnabled;
        }

        [[nodiscard]] const GenerationStatistics& getStatistics() const {
            return statistics;
        }

        void printStatistics(std::ostream& output) const {
            if (!profilingEnabled) {
                return;
            }

            output << "Generation profile:\n";
            output << "  Parse regexes: " << toMilliseconds(statistics.getParsingDuration()) << " ms\n";
            output << "    Parse calls (" << statistics.getParsedRegexCount() << "): "
                   << toMicroseconds(statistics.getParsingRegexDuration()) << " us";
            output << " [max " << toMicroseconds(statistics.getParsingLongestRegexDuration()) << " us]\n";
            output << "  Regex -> NFA: " << toMilliseconds(statistics.getRegexToNfaDuration()) << " ms\n";
            output << "    Thompson calls (" << statistics.getRegexToNfaCount() << "): "
                   << toMicroseconds(statistics.getRegexToNfaRegexDuration()) << " us";
            output << " [max " << toMicroseconds(statistics.getRegexToNfaLongestRegexDuration()) << " us]\n";
            output << "    NFA materialization: "
                   << toMicroseconds(statistics.getRegexToNfaMaterializationDuration()) << " us\n";
            output << "  NFA -> DFA: " << toMilliseconds(statistics.getNfaToDfaDuration()) << " ms\n";
            output << "    Merge NFAs: " << toMicroseconds(statistics.getMergeNfaDuration()) << " us\n";
            output << "    Powerset conversion: " << toMicroseconds(statistics.getPowerSetConversionDuration()) << " us\n";
        }

        DFA generateScanner(const std::string& regex) {
            statistics.reset();

            RegexParser parser;
            statistics.startParsing();
            statistics.startParsingRegex();
            const auto regexTree = parser.parse(regex);
            statistics.endParsingRegex();
            statistics.endParsing();

            statistics.startRegexToNfa();
            ThompsonConstructionVisitor visitor(nodeFactory);
            statistics.startRegexToNfaRegex();
            regexTree->accept(visitor);
            statistics.endRegexToNfaRegex();
            statistics.endRegexToNfa();

            const NFA& constructedNFA = visitor.getConstructedNFA();
            statistics.startNfaToDfa();
            statistics.startPowerSetConversion();
            DFA dfa = PowerSetConstruction::convert(constructedNFA);
            statistics.endPowerSetConversion();
            statistics.endNfaToDfa();
            return dfa;
        }

        DFA generateScanner(const std::vector<std::string>& regexes) {
            statistics.reset();

            std::vector<std::shared_ptr<RegexNode>> regexNodes;
            regexNodes.reserve(regexes.size());

            RegexParser parser;
            statistics.startParsing();
            for (const auto& regex: regexes) {
                statistics.startParsingRegex();
                regexNodes.emplace_back(parser.parse(regex));
                statistics.endParsingRegex();
            }
            statistics.endParsing();

            std::vector<NFA> nfas;
            statistics.startRegexToNfa();
            for (const auto& regexNode : regexNodes) {
                ThompsonConstructionVisitor visitor(nodeFactory);
                statistics.startRegexToNfaRegex();
                regexNode->accept(visitor);
                statistics.endRegexToNfaRegex();
                statistics.startRegexToNfaMaterialization();
                nfas.push_back(visitor.moveConstructedNFA());
                statistics.endRegexToNfaMaterialization();
            }
            statistics.endRegexToNfa();

            statistics.startNfaToDfa();
            statistics.startMergeNfa();
            MergedNFA mergedNFA(nfas, nodeFactory);
            statistics.endMergeNfa();
            statistics.startPowerSetConversion();
            DFA dfa = PowerSetConstruction::convert(mergedNFA);
            statistics.endPowerSetConversion();
            statistics.endNfaToDfa();

            return dfa;
        }

    private:
        NFANodeFactory nodeFactory;
        bool profilingEnabled = false;
        GenerationStatistics statistics;
    };
}
