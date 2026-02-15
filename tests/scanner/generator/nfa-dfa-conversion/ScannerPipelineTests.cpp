#include <gtest/gtest.h>

#include <initializer_list>
#include <memory>
#include <set>
#include <string>
#include <vector>

import Scanner.DFAMatcher;
import Scanner.Generator;

namespace {

scanner::DFAMatcher BuildMatcherFromRegex(const std::string& expression) {
    scanner::Generator generator;
    return scanner::DFAMatcher(generator.generateScanner(expression));
}

scanner::DFAMatcher BuildMatcherFromRegexes(const std::vector<std::string>& expressions) {
    scanner::Generator generator;
    return scanner::DFAMatcher(generator.generateScanner(expressions));
}

void ExpectMatches(const scanner::DFAMatcher& matcher, const std::initializer_list<std::string>& inputs) {
    for (const auto& input : inputs) {
        EXPECT_TRUE(matcher.match(input)) << "Expected to accept '" << input << "'";
    }
}

void ExpectMatchedIds(const scanner::DFAMatcher& matcher, const std::initializer_list<std::pair<std::string, std::set<std::uint32_t>>>& inputs) {
    for (const auto& input : inputs) {
        auto matchedIds = matcher.getMatchingIDs(input.first);

        EXPECT_EQ(matchedIds.size(), input.second.size()) << "Expected to match IDs for '" << input.first << "'";
        EXPECT_EQ(matchedIds, input.second) << "Expected to match IDs for '" << input.first << "'";
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

TEST(ScannerPipelineTests, PlusAcceptsOneOrMoreOccurrences) {
    auto matcher = BuildMatcherFromRegex("a+");

    ExpectMatches(matcher, {"a", "aa", "aaaa"});
    ExpectRejections(matcher, {"", "b", "ab", "ba"});
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

TEST(ScannerPipelineTests, MultiRegexComplexRegexes) {
    auto matcher = BuildMatcherFromRegexes({"a(abc|def)*", "(ayz)?789"});

    ExpectMatchedIds(
        matcher,
        {
            { "a", { 0 }},
            { "aabc", { 0 }},
            { "adefabc", { 0 }},
            { "adefabcdef", { 0 }},
            { "789", { 1 }},
            { "ayz789", { 0, 1 }}
            }
        );
    ExpectRejections(matcher, {"abc123", "xyz123", "123", "defxyz789"});
}

TEST(ScannerPipelineTests, DotMatchesAnySingleCharacter) {
    auto matcher = BuildMatcherFromRegex(".");

    ExpectMatches(matcher, {"a", "Z", "1"});
    ExpectRejections(matcher, {"", "ab"});
}

TEST(ScannerPipelineTests, EscapedDotMatchesLiteral) {
    auto matcher = BuildMatcherFromRegex("\\.");

    ExpectMatches(matcher, {"."});
    ExpectRejections(matcher, {"", "a", ".."});
}

TEST(ScannerPipelineTests, DotMatchesCharactersMissingFromAlphabet) {
    auto matcher = BuildMatcherFromRegex("a.");

    ExpectMatches(matcher, {"ab", "ax"});
    ExpectRejections(matcher, {"a", "abc"});
}

TEST(ScannerPipelineTests, DotMatchesCharactersPresentInAlphabet) {
    auto matcher = BuildMatcherFromRegex("a.a");

    ExpectMatches(matcher, {"aaa", "aba"});
    ExpectRejections(matcher, {"aa", "aaaa"});
}

TEST(ScannerPipelineTests, MultipleDotsMatchCharacters) {
    auto matcher = BuildMatcherFromRegex("a.a.*a");

    ExpectMatches(matcher, {"abaa"});
    ExpectRejections(matcher, {"aaa", "ababac"});
}