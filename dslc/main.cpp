#include <memory>
#include <string>
#include <iostream>

import Scanner.Regex;
import Scanner.RegexParser;
import Scanner.NFA;
import Scanner.ThompsonConstructionVisitor;
import Scanner.PowerSetConstruction;
import Scanner.DFA;
import Scanner.DFAMatcher;

scanner::DFAMatcher BuildMatcher(const std::shared_ptr<scanner::RegexNode>& root) {
    scanner::ThompsonConstructionVisitor visitor;
    root->accept(visitor);

    const scanner::NFA& constructedNFA = visitor.getConstructedNFA();
    scanner::DFA dfa = scanner::PowerSetConstruction::convert(constructedNFA);
    return scanner::DFAMatcher(std::move(dfa));
}

int main(const int argc, const char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: dslc <regex> <string_to_match>\n";
        return 1;
    }

    std::string regex = argv[1];
    std::string strToMatch = argv[2];

    scanner::RegexParser parser;
    auto regexTree = parser.parse(regex);
    auto matcher = BuildMatcher(regexTree);
    const bool isMatch = matcher.match(strToMatch);

    std::cout << isMatch << std::endl;

    return 0;
}