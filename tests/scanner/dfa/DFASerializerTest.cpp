#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

import Scanner.DFA;
import Scanner.DFAAcceptingState;
import Scanner.DFAMatcher;
import Scanner.DFASerializer;
import Scanner.Generator;

using namespace scanner;

TEST(DFASerializerTest, RoundTripPreservesDFA) {
    const std::filesystem::path filePath = "/tmp/dfa_serializer_roundtrip.bin";
    std::error_code ec;
    std::filesystem::remove(filePath, ec);

    std::vector<char> alphabet{'a', 'b'};
    std::vector<std::vector<std::uint32_t>> transitions{
        {1, 0},
        {1, 1},
    };

    std::vector<DFAAcceptingState> acceptingStates;
    acceptingStates.emplace_back(false, std::vector<std::uint32_t>{});
    acceptingStates.emplace_back(true, std::vector<std::uint32_t>{42, 7});

    DFA dfa(transitions, acceptingStates, alphabet);

    DFASerializer::serialize(dfa, filePath);
    DFA deserialized = DFASerializer::deserialize(filePath);

    EXPECT_EQ(deserialized.getStateCount(), dfa.getStateCount());
    EXPECT_EQ(deserialized.getAlphabet(), dfa.getAlphabet());

    const std::uint32_t alphabetSize = static_cast<std::uint32_t>(alphabet.size());
    for (std::uint32_t state = 0; state < dfa.getStateCount(); ++state) {
        for (std::uint32_t symbolIndex = 0; symbolIndex < alphabetSize; ++symbolIndex) {
            EXPECT_EQ(deserialized.getNextState(state, symbolIndex),
                      dfa.getNextState(state, symbolIndex));
        }

        EXPECT_EQ(deserialized.isAcceptingState(state), dfa.isAcceptingState(state));
        EXPECT_EQ(deserialized.getAcceptingIds(state), dfa.getAcceptingIds(state));
    }

    std::filesystem::remove(filePath, ec);
}

TEST(DFASerializerTest, RoundTripPreservesCaptureActions) {
    const std::filesystem::path filePath = "/tmp/dfa_serializer_capture_roundtrip.bin";
    std::error_code ec;
    std::filesystem::remove(filePath, ec);

    Generator generator;
    DFA dfa = generator.generateScanner("(?<name>a)");

    DFASerializer::serialize(dfa, filePath);
    DFA deserialized = DFASerializer::deserialize(filePath);

    DFAMatcher matcher(std::move(deserialized));
    const auto result = matcher.getMatches("a", true);
    ASSERT_EQ(1u, result.getMatches().size());
    const auto& captures = result.getMatches()[0].getCaptures();

    ASSERT_TRUE(captures.contains("name"));
    ASSERT_EQ(1u, captures.at("name").size());
    EXPECT_EQ("a", captures.at("name")[0]);

    std::filesystem::remove(filePath, ec);
}

TEST(DFASerializerTest, DeserializeThrowsForInvalidHeader) {
    const std::filesystem::path filePath = "/tmp/dfa_serializer_invalid_header.bin";
    std::error_code ec;
    std::filesystem::remove(filePath, ec);

    {
        std::ofstream output(filePath, std::ios::binary);
        ASSERT_TRUE(output.is_open());
        output << "bad!";
    }

    EXPECT_THROW(DFASerializer::deserialize(filePath), std::runtime_error);

    std::filesystem::remove(filePath, ec);
}
