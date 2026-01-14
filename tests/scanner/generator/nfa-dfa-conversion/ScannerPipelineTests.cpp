
#include <gtest/gtest.h>

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

import Scanner.Regex;
import Scanner.ThompsonConstructionVisitor;
import Scanner.PowerSetConstruction;
import Scanner.DFA;
import Scanner.DFAMatcher;
import Scanner.NFA;

namespace {

std::shared_ptr<scanner::Leaf> MakeLeaf(char symbol) {
    auto leaf = std::make_shared<scanner::Leaf>();
    leaf->setCharacters(std::vector<char>{symbol});
    return leaf;
}

std::shared_ptr<scanner::RegexNode> BuildSequence(const std::string& literal) {
    if (literal.empty()) {
        return nullptr;
    }

    std::shared_ptr<scanner::RegexNode> sequence = MakeLeaf(literal[0]);
    for (std::size_t index = 1; index < literal.size(); ++index) {
        sequence = std::make_shared<scanner::Concatenation>(sequence, MakeLeaf(literal[index]));
    }
    return sequence;
}

scanner::DFAMatcher BuildMatcher(const std::shared_ptr<scanner::RegexNode>& root) {
    scanner::ThompsonConstructionVisitor visitor;
    root->accept(visitor);

    const scanner::NFA& constructedNFA = visitor.getConstructedNFA();
    scanner::DFA dfa = scanner::PowerSetConstruction::convert(constructedNFA);
    return scanner::DFAMatcher(std::move(dfa));
}

void ExpectMatches(const scanner::DFAMatcher& matcher, const std::initializer_list<std::string>& inputs) {
    for (const auto& input : inputs) {
        EXPECT_TRUE(matcher.match(input)) << "Expected to accept '" << input << "'";
    }
}

void ExpectRejections(const scanner::DFAMatcher& matcher, const std::initializer_list<std::string>& inputs) {
    for (const auto& input : inputs) {
        EXPECT_FALSE(matcher.match(input)) << "Expected to reject '" << input << "'";
    }
}

} // namespace

TEST(ScannerPipelineTests, SingleLiteralAcceptsExactMatch) {
    auto matcher = BuildMatcher(BuildSequence("a"));

    ExpectMatches(matcher, {"a"});
    ExpectRejections(matcher, {"", "b", "aa"});
}

TEST(ScannerPipelineTests, UnionAcceptsEitherAlternative) {
    auto leftLeaf = MakeLeaf('a');
    auto rightLeaf = MakeLeaf('b');
    auto unionNode = std::make_shared<scanner::Union>(leftLeaf, rightLeaf);

    auto matcher = BuildMatcher(unionNode);

    ExpectMatches(matcher, {"a", "b"});
    ExpectRejections(matcher, {"", "c", "ab"});
}

TEST(ScannerPipelineTests, ConcatenationAcceptsSequence) {
    auto concatenation = std::make_shared<scanner::Concatenation>(MakeLeaf('a'), MakeLeaf('b'));
    auto matcher = BuildMatcher(concatenation);

    ExpectMatches(matcher, {"ab"});
    ExpectRejections(matcher, {"", "a", "b", "abc"});
}

TEST(ScannerPipelineTests, KleeneStarAcceptsZeroOrMoreOccurrences) {
    auto kleene = std::make_shared<scanner::Kleene>(MakeLeaf('a'));
    auto matcher = BuildMatcher(kleene);

    ExpectMatches(matcher, {"", "a", "aaaa"});
    ExpectRejections(matcher, {"b", "ab", "ba"});
}

TEST(ScannerPipelineTests, OptionalAcceptsZeroOrOneOccurrence) {
    auto optional = std::make_shared<scanner::Optional>(MakeLeaf('a'));
    auto matcher = BuildMatcher(optional);

    ExpectMatches(matcher, {"", "a"});
    ExpectRejections(matcher, {"aa", "b"});
}

TEST(ScannerPipelineTests, ComplexRegexHandlesAbcOrDefRepeatedly) {
    auto abcSequence = BuildSequence("abc");
    auto defSequence = BuildSequence("def");
    auto unionNode = std::make_shared<scanner::Union>(abcSequence, defSequence);
    auto repetition = std::make_shared<scanner::Kleene>(unionNode);

    auto matcher = BuildMatcher(repetition);

    ExpectMatches(matcher, {"", "abc", "defabc", "defabcdef"});
    ExpectRejections(matcher, {"ab", "abcde", "xyz", "abcdefg"});
}
