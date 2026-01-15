#include <gtest/gtest.h>

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

import Scanner.Regex;
import Scanner.RegexParser;
import Scanner.ThompsonConstructionVisitor;
import Scanner.PowerSetConstruction;
import Scanner.DFA;
import Scanner.DFAMatcher;
import Scanner.NFA;

namespace {

scanner::DFAMatcher BuildMatcher(const std::shared_ptr<scanner::RegexNode>& root) {
    scanner::ThompsonConstructionVisitor visitor;
    root->accept(visitor);

    const scanner::NFA& constructedNFA = visitor.getConstructedNFA();
    scanner::DFA dfa = scanner::PowerSetConstruction::convert(constructedNFA);
    return scanner::DFAMatcher(std::move(dfa));
}

scanner::DFAMatcher BuildMatcherFromRegex(const std::string& expression) {
    scanner::RegexParser parser;
    return BuildMatcher(parser.parse(expression));
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
    auto matcher = BuildMatcherFromRegex("a");

    ExpectMatches(matcher, {"a"});
    ExpectRejections(matcher, {"", "b", "aa"});
}

TEST(ScannerPipelineTests, UnionAcceptsEitherAlternative) {
    auto matcher = BuildMatcherFromRegex("a|b");

    ExpectMatches(matcher, {"a", "b"});
    ExpectRejections(matcher, {"", "c", "ab"});
}

TEST(ScannerPipelineTests, ConcatenationAcceptsSequence) {
    auto matcher = BuildMatcherFromRegex("ab");

    ExpectMatches(matcher, {"ab"});
    ExpectRejections(matcher, {"", "a", "b", "abc"});
}

TEST(ScannerPipelineTests, KleeneStarAcceptsZeroOrMoreOccurrences) {
    auto matcher = BuildMatcherFromRegex("a*");

    ExpectMatches(matcher, {"", "a", "aaaa"});
    ExpectRejections(matcher, {"b", "ab", "ba"});
}

TEST(ScannerPipelineTests, OptionalAcceptsZeroOrOneOccurrence) {
    auto matcher = BuildMatcherFromRegex("a?");

    ExpectMatches(matcher, {"", "a"});
    ExpectRejections(matcher, {"aa", "b"});
}

TEST(ScannerPipelineTests, ComplexRegexHandlesAbcOrDefRepeatedly) {
    auto matcher = BuildMatcherFromRegex("(abc|def)*");

    ExpectMatches(matcher, {"", "abc", "defabc", "defabcdef"});
    ExpectRejections(matcher, {"ab", "abcde", "xyz", "abcdefg"});
}


TEST(ScannerPipelineTests, Test2) {
    auto matcher = BuildMatcherFromRegex("(abc|def)*(x)?123");

    ExpectMatches(matcher, {"123", "abc123", "defabcdefx123", "defabcdef123"});
    ExpectRejections(matcher, {"ab", "abcde", "xyz", "abcdefg"});
}
