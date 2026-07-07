#include <gtest/gtest.h>

#include <initializer_list>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

import Scanner.DFA;
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

scanner::DFA BuildDFAFromRegex(const std::string& expression) {
    scanner::Generator generator;
    return generator.generateScanner(expression);
}

std::uint32_t GetSymbolIndex(const scanner::DFA& dfa, const char symbol) {
    const auto& alphabet = dfa.getAlphabet();
    for (std::uint32_t index = 0; index < alphabet.size(); ++index) {
        if (alphabet[index] == symbol) {
            return index;
        }
    }

    throw std::runtime_error("Symbol not found in DFA alphabet");
}

void ExpectMatches(const scanner::DFAMatcher& matcher, const std::initializer_list<std::string>& inputs) {
    for (const auto& input : inputs) {
        EXPECT_TRUE(matcher.match(input)) << "Expected to accept '" << input << "'";
    }
}

void ExpectMatchedIds(const scanner::DFAMatcher& matcher, const std::initializer_list<std::pair<std::string, std::set<std::uint32_t>>>& inputs) {
    for (const auto& input : inputs) {
        const auto result = matcher.getMatches(input.first);
        const auto& matchedIds = result.getMatchingIDs();

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

TEST(ScannerPipelineTests, NamedCapturingGroupIsTransparentForMatching) {
    auto matcher = BuildMatcherFromRegex("(?<prefix>abc|def)*123");

    ExpectMatches(matcher, {"123", "abc123", "defabc123"});
    ExpectRejections(matcher, {"ab", "abcde", "xyz"});
}

TEST(ScannerPipelineTests, NamedCapturingGroupStoresDfaTransitionActions) {
    auto dfa = BuildDFAFromRegex("(?<name>a)");
    const auto symbolIndex = GetSymbolIndex(dfa, 'a');
    const auto& actions = dfa.getCaptureActions(0, symbolIndex);

    ASSERT_EQ(2u, actions.size());
    EXPECT_EQ(scanner::DFACaptureAction::Type::Start, actions[0].getType());
    EXPECT_EQ(0u, actions[0].getRegexID());
    EXPECT_EQ(1u, actions[0].getGroupID());
    ASSERT_TRUE(actions[0].getName().has_value());
    EXPECT_EQ("name", *actions[0].getName());

    EXPECT_EQ(scanner::DFACaptureAction::Type::End, actions[1].getType());
    EXPECT_EQ(0u, actions[1].getRegexID());
    EXPECT_EQ(1u, actions[1].getGroupID());
    ASSERT_TRUE(actions[1].getName().has_value());
    EXPECT_EQ("name", *actions[1].getName());
}

TEST(ScannerPipelineTests, CapturesSingleNamedGroup) {
    auto matcher = BuildMatcherFromRegex("(?<name>a)");
    const auto result = matcher.getMatches("a", true);
    ASSERT_EQ(1u, result.getMatches().size());
    const auto& captures = result.getMatches()[0].getCaptures();

    ASSERT_TRUE(captures.contains("name"));
    ASSERT_EQ(1u, captures.at("name").size());
    EXPECT_EQ("a", captures.at("name")[0]);
}

TEST(ScannerPipelineTests, CapturesMultiCharacterNamedGroup) {
    auto matcher = BuildMatcherFromRegex("(?<name>ab)c");
    const auto result = matcher.getMatches("abc", true);
    ASSERT_EQ(1u, result.getMatches().size());
    const auto& captures = result.getMatches()[0].getCaptures();

    ASSERT_TRUE(captures.contains("name"));
    ASSERT_EQ(1u, captures.at("name").size());
    EXPECT_EQ("ab", captures.at("name")[0]);
}

TEST(ScannerPipelineTests, CapturesRepeatedNamedGroupValues) {
    auto matcher = BuildMatcherFromRegex("(?<item>ab)*");
    const auto result = matcher.getMatches("abab", true);
    ASSERT_EQ(1u, result.getMatches().size());
    const auto& captures = result.getMatches()[0].getCaptures();

    ASSERT_TRUE(captures.contains("item"));
    ASSERT_EQ(2u, captures.at("item").size());
    EXPECT_EQ("ab", captures.at("item")[0]);
    EXPECT_EQ("ab", captures.at("item")[1]);
}

TEST(ScannerPipelineTests, SkipsOptionalNamedGroupWhenItDoesNotMatch) {
    auto matcher = BuildMatcherFromRegex("(?<maybe>a)?b");
    const auto result = matcher.getMatches("b", true);
    ASSERT_EQ(1u, result.getMatches().size());
    const auto& captures = result.getMatches()[0].getCaptures();

    EXPECT_FALSE(captures.contains("maybe"));
}

TEST(ScannerPipelineTests, ReturnsNoCapturesWhenInputDoesNotMatch) {
    auto matcher = BuildMatcherFromRegex("(?<name>a)b");
    const auto result = matcher.getMatches("a", true);

    EXPECT_TRUE(result.getMatches().empty());
}

TEST(ScannerPipelineTests, CapturesStructuredApplicationLogFields) {
    auto matcher = BuildMatcherFromRegex(
        "(?<date>[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]) "
        "(?<time>[0-9][0-9]:[0-9][0-9]:[0-9][0-9]) "
        "(?<level>INFO) "
        "(?<service>[a-z][a-z][a-z]) "
        "request=(?<requestId>[a-z][0-9][a-z][0-9]) "
        "status=(?<status>[0-9][0-9][0-9])"
    );

    const auto result = matcher.getMatches("2026-07-07 14:35:02 INFO api request=a1b2 status=200", true);
    ASSERT_EQ(1u, result.getMatches().size());
    const auto& captures = result.getMatches()[0].getCaptures();

    ASSERT_EQ(1u, captures.at("date").size());
    EXPECT_EQ("2026-07-07", captures.at("date")[0]);
    EXPECT_EQ("14:35:02", captures.at("time")[0]);
    EXPECT_EQ("INFO", captures.at("level")[0]);
    EXPECT_EQ("api", captures.at("service")[0]);
    EXPECT_EQ("a1b2", captures.at("requestId")[0]);
    EXPECT_EQ("200", captures.at("status")[0]);
}

TEST(ScannerPipelineTests, CapturesHttpAccessLogFields) {
    auto matcher = BuildMatcherFromRegex(
        "(?<method>GET) "
        "(?<route>/api/users) "
        "(?<status>[0-9][0-9][0-9]) "
        "(?<latency>[0-9][0-9])ms"
    );

    const auto result = matcher.getMatches("GET /api/users 200 42ms", true);
    ASSERT_EQ(1u, result.getMatches().size());
    const auto& captures = result.getMatches()[0].getCaptures();

    EXPECT_EQ("GET", captures.at("method")[0]);
    EXPECT_EQ("/api/users", captures.at("route")[0]);
    EXPECT_EQ("200", captures.at("status")[0]);
    EXPECT_EQ("42", captures.at("latency")[0]);
}

TEST(ScannerPipelineTests, CapturesDatabaseSlowQueryLogFields) {
    auto matcher = BuildMatcherFromRegex(
        "slow-query "
        "duration=(?<duration>[0-9][0-9][0-9])ms "
        "table=(?<table>[a-z][a-z][a-z][a-z][a-z]) "
        "rows=(?<rows>[0-9][0-9])"
    );

    const auto result = matcher.getMatches("slow-query duration=123ms table=users rows=42", true);
    ASSERT_EQ(1u, result.getMatches().size());
    const auto& captures = result.getMatches()[0].getCaptures();

    EXPECT_EQ("123", captures.at("duration")[0]);
    EXPECT_EQ("users", captures.at("table")[0]);
    EXPECT_EQ("42", captures.at("rows")[0]);
}

TEST(ScannerPipelineTests, CapturesAreAssociatedWithMatchingRegexIds) {
    auto matcher = BuildMatcherFromRegexes({"(?<letter>a)", "(?<letter>b)"});
    const auto result = matcher.getMatches("a", true);

    EXPECT_EQ(std::set<std::uint32_t>{0}, result.getMatchingIDs());
    ASSERT_EQ(1u, result.getMatches().size());
    EXPECT_EQ(0u, result.getMatches()[0].getRegexID());
    ASSERT_TRUE(result.getMatches()[0].getCaptures().contains("letter"));
    EXPECT_EQ("a", result.getMatches()[0].getCaptures().at("letter")[0]);
}

TEST(ScannerPipelineTests, CapturesAreReturnedForMultipleMatchingRegexIds) {
    auto matcher = BuildMatcherFromRegexes({"(?<left>a)", "(?<right>a)"});
    const auto result = matcher.getMatches("a", true);

    EXPECT_EQ((std::set<std::uint32_t>{0, 1}), result.getMatchingIDs());
    ASSERT_EQ(2u, result.getMatches().size());

    EXPECT_EQ(0u, result.getMatches()[0].getRegexID());
    EXPECT_EQ("a", result.getMatches()[0].getCaptures().at("left")[0]);

    EXPECT_EQ(1u, result.getMatches()[1].getRegexID());
    EXPECT_EQ("a", result.getMatches()[1].getCaptures().at("right")[0]);
}

TEST(ScannerPipelineTests, ConflictingCaptureStartsThrowDuringGeneration) {
    scanner::Generator generator;
    EXPECT_THROW(generator.generateScanner("(?<left>a)|(?<right>a)"), std::runtime_error);
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
