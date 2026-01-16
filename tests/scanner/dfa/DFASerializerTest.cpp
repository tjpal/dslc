#include <filesystem>
#include <fstream>
#include <system_error>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

import Scanner.DFA;
import Scanner.DFAAcceptingState;
import Scanner.DFASerializer;

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