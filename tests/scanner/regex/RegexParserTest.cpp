#include <algorithm>
#include <memory>
#include <vector>

#include <gmock/gmock.h>

import Scanner.RegexParser;
import Scanner.Regex;

namespace {

bool containsCharacter(const std::vector<char>& characters, char value) {
    return std::find(characters.begin(), characters.end(), value) != characters.end();
}

std::shared_ptr<scanner::Leaf> asLeaf(const std::shared_ptr<scanner::RegexNode>& node) {
    return std::dynamic_pointer_cast<scanner::Leaf>(node);
}

} // namespace

TEST(RegexParserPredefinedCharacterClasses, ParsesDigitClassIntoLeaf) {
    scanner::RegexParser parser;
    const auto node = parser.parse("\\d");
    const auto leaf = asLeaf(node);
    ASSERT_NE(nullptr, leaf);

    const auto& characters = leaf->getCharacters();
    EXPECT_EQ(10u, characters.size());

    for (char digit = '0'; digit <= '9'; ++digit) {
        EXPECT_TRUE(containsCharacter(characters, digit)) << "Digit " << digit << " is missing";
    }
}

TEST(RegexParserPredefinedCharacterClasses, ParsesNonDigitClassWithoutDigits) {
    scanner::RegexParser parser;
    const auto node = parser.parse("\\D");
    const auto leaf = asLeaf(node);
    ASSERT_NE(nullptr, leaf);

    const auto& characters = leaf->getCharacters();
    EXPECT_FALSE(containsCharacter(characters, '0'));
    EXPECT_FALSE(containsCharacter(characters, '5'));

    EXPECT_TRUE(containsCharacter(characters, 'A'));
    EXPECT_TRUE(containsCharacter(characters, '\n'));
}

TEST(RegexParserPredefinedCharacterClasses, ParsessWordClassInsideCharacterClass) {
    scanner::RegexParser parser;
    const auto node = parser.parse("[\\w]");
    const auto leaf = asLeaf(node);
    ASSERT_NE(nullptr, leaf);

    const auto& characters = leaf->getCharacters();
    EXPECT_TRUE(containsCharacter(characters, '0'));
    EXPECT_TRUE(containsCharacter(characters, 'A'));
    EXPECT_TRUE(containsCharacter(characters, '_'));
}
