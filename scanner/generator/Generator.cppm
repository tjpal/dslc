module;

#include <memory>
#include <string>

export module Scanner.Generator;

import Scanner.RegexParser;
import Scanner.DFA;
import Scanner.ThompsonConstructionVisitor;
import Scanner.NFA;
import Scanner.PowerSetConstruction;
import Scanner.Regex;
import Scanner.MergedNFA;

namespace scanner {
    export class Generator {
    public:
        Generator() = default;

        DFA generateScanner(const std::string& regex) {
            RegexParser parser;
            const auto regexTree = parser.parse(regex);

            ThompsonConstructionVisitor visitor(nodeFactory);
            regexTree->accept(visitor);

            const NFA& constructedNFA = visitor.getConstructedNFA();
            DFA dfa = PowerSetConstruction::convert(constructedNFA);
            return dfa;
        }

        DFA generateScanner(const std::vector<std::string>& regexes) {
            std::vector<std::shared_ptr<RegexNode>> regexNodes;
            regexNodes.reserve(regexes.size());

            RegexParser parser;
            for (const auto& regex: regexes) {
                regexNodes.emplace_back(parser.parse(regex));
            }

            std::vector<NFA> nfas;
            for (const auto& regexNode : regexNodes) {
                ThompsonConstructionVisitor visitor(nodeFactory);
                regexNode->accept(visitor);
                nfas.push_back(visitor.moveConstructedNFA());
            }

            MergedNFA mergedNFA(nfas, nodeFactory);
            DFA dfa = PowerSetConstruction::convert(mergedNFA);

            return dfa;
        }

    private:
        NFANodeFactory nodeFactory;
    };
}
