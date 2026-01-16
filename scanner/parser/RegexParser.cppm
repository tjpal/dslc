module;

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

export module Scanner.RegexParser;

import Scanner.Regex;

namespace scanner {
    export class RegexParser final {
    public:
        std::shared_ptr<RegexNode> parse(const std::string& input) {
            expression = input;
            position = 0;

            if (expression.empty()) {
                throw std::runtime_error("Regular expression cannot be empty");
            }

            const auto result = parseExpression();

            if (position != expression.size()) {
                throw std::runtime_error("Unexpected character at position " + std::to_string(position));
            }

            return result;
        }

    private:
        static constexpr std::size_t ASCII_CHARACTER_COUNT = 128;

        std::string expression;
        std::size_t position = 0;

        bool hasNext() const {
            return position < expression.size();
        }

        char peek() const {
            if (!hasNext()) {
                throw std::runtime_error("Unexpected end of expression");
            }

            return expression[position];
        }

        char get() {
            if (!hasNext()) {
                throw std::runtime_error("Unexpected end of expression");
            }

            return expression[position++];
        }

        bool match(char expected) {
            if (hasNext() && expression[position] == expected) {
                ++position;
                return true;
            }

            return false;
        }

        std::shared_ptr<RegexNode> parseExpression() {
            auto node = parseTerm();

            while (match('|')) {
                const auto right = parseTerm();
                node = std::make_shared<Union>(node, right);
            }

            return node;
        }

        std::shared_ptr<RegexNode> parseTerm() {
            auto node = parseFactor();

            while (hasNext()) {
                const char next = peek();
                if (next == '|' || next == ')') {
                    break;
                }

                const auto rightHandSide = parseFactor();
                node = std::make_shared<Concatenation>(node, rightHandSide);
            }

            return node;
        }

        std::shared_ptr<RegexNode> parseFactor() {
            auto node = parseAtom();

            while (true) {
                if (match('*')) {
                    node = std::make_shared<Kleene>(node);
                    continue;
                }

                if (match('?')) {
                    node = std::make_shared<Optional>(node);
                    continue;
                }

                break;
            }

            return node;
        }

        std::shared_ptr<RegexNode> parseAtom() {
            if (!hasNext()) {
                throw std::runtime_error("Unexpected end of expression");
            }

            if (match('(')) {
                const auto nested = parseExpression();
                if (!match(')')) {
                    throw std::runtime_error("Unmatched '");
                }

                return nested;
            }

            if (match('[')) {
                const auto characters = parseCharacterClass();
                return createLeaf(characters);
            }

            if (match('\\')) {
                const char escaped = consumeEscapedCharacter();
                if (const auto predefined = getPredefinedCharacters(escaped); predefined.has_value()) {
                    return createLeaf(*predefined);
                }

                return createLeaf({escaped});
            }

            if (match('.')) {
                return createWildcardLeaf();
            }

            switch (const char literal = get()) {
            case '|':
            case ')':
            case '*':
            case '?':
            case ']':
                throw std::runtime_error(std::string("Unexpected symbol '") + literal + "'");
            default:
                return createLeaf({literal});
            }
        }

        char consumeEscapedCharacter() {
            if (!hasNext()) {
                throw std::runtime_error("Escape sequence is incomplete");
            }

            return get();
        }

        std::shared_ptr<RegexNode> createLeaf(const std::vector<char>& characters) const {
            if (characters.empty()) {
                throw std::runtime_error("Leaf requires at least one character");
            }

            auto leaf = std::make_shared<Leaf>();
            leaf->setCharacters(characters);
            return leaf;
        }

        std::shared_ptr<RegexNode> createWildcardLeaf() const {
            auto leaf = std::make_shared<Leaf>();
            leaf->setWildcard(true);
            return leaf;
        }

        std::vector<char> parseCharacterClass() {
            std::vector<char> characters;
            bool closed = false;

            auto appendCharacters = [&characters](const std::vector<char>& addition) {
                characters.insert(characters.end(), addition.begin(), addition.end());
            };

            const auto readRangeEndpoint = [this]() -> char {
                if (match('\\')) {
                    const char escaped = consumeEscapedCharacter();
                    if (isPredefinedCharacterClass(escaped)) {
                        throw std::runtime_error("Range endpoint cannot be a predefined character class");
                    }

                    return escaped;
                }

                if (!hasNext()) {
                    throw std::runtime_error("Unexpected end of character class");
                }

                const char value = get();
                if (value == ']') {
                    throw std::runtime_error("Unexpected ']' inside character class");
                }

                return value;
            };

            const auto processLiteralCharacter = [this, &characters, &readRangeEndpoint](char literal, bool escaped) {
                if (!escaped && literal == ']') {
                    throw std::runtime_error("Unexpected ']' inside character class");
                }

                if (hasNext() && peek() == '-' && (position + 1) < expression.size() && expression[position + 1] != ']') {
                    ++position;
                    const char rangeEnd = readRangeEndpoint();

                    const auto startValue = static_cast<unsigned char>(literal);
                    const auto endValue = static_cast<unsigned char>(rangeEnd);

                    if (startValue > endValue) {
                        throw std::runtime_error("Invalid character range");
                    }

                    for (unsigned int symbol = startValue; symbol <= endValue; ++symbol) {
                        characters.push_back(static_cast<char>(symbol));
                    }

                    return;
                }

                characters.push_back(literal);
            };

            while (hasNext()) {
                if (peek() == ']') {
                    get();
                    closed = true;
                    break;
                }

                if (match('\\')) {
                    const char escaped = consumeEscapedCharacter();
                    if (const auto predefined = getPredefinedCharacters(escaped); predefined.has_value()) {
                        appendCharacters(*predefined);
                        continue;
                    }

                    processLiteralCharacter(escaped, true);
                    continue;
                }

                const char character = get();
                processLiteralCharacter(character, false);
            }

            if (!closed) {
                throw std::runtime_error("Unterminated character class");
            }

            if (characters.empty()) {
                throw std::runtime_error("Character class must contain at least one character");
            }

            return characters;
        }

        std::optional<std::vector<char>> getPredefinedCharacters(char identifier) const {
            switch (identifier) {
            case 'd':
                return createCharacterRange('0', '9');
            case 'D':
                return createComplementCharacterSet(createCharacterRange('0', '9'));
            case 's':
                return createWhitespaceCharacters();
            case 'S':
                return createComplementCharacterSet(createWhitespaceCharacters());
            case 'w':
                return createWordCharacters();
            case 'W':
                return createComplementCharacterSet(createWordCharacters());
            default:
                return std::nullopt;
            }
        }

        static bool isPredefinedCharacterClass(char identifier) {
            switch (identifier) {
            case 'd':
            case 'D':
            case 's':
            case 'S':
            case 'w':
            case 'W':
                return true;
            default:
                return false;
            }
        }

        static std::vector<char> createCharacterRange(char start, char end) {
            std::vector<char> characters;
            const unsigned char startValue = static_cast<unsigned char>(start);
            const unsigned char endValue = static_cast<unsigned char>(end);

            if (startValue > endValue) {
                return characters;
            }

            characters.reserve(static_cast<std::size_t>(endValue - startValue + 1));
            for (unsigned int value = startValue; value <= endValue; ++value) {
                characters.push_back(static_cast<char>(value));
            }

            return characters;
        }

        static std::vector<char> createWhitespaceCharacters() {
            return {'\t', '\n', '\v', '\f', '\r', ' '};
        }

        static std::vector<char> createWordCharacters() {
            std::vector<char> characters;
            characters.reserve(26 + 26 + 10 + 1);

            for (char c = 'A'; c <= 'Z'; ++c) {
                characters.push_back(c);
            }

            for (char c = 'a'; c <= 'z'; ++c) {
                characters.push_back(c);
            }

            for (char c = '0'; c <= '9'; ++c) {
                characters.push_back(c);
            }

            characters.push_back('_');
            return characters;
        }

        static std::vector<char> createComplementCharacterSet(const std::vector<char>& characters) {
            std::array<bool, ASCII_CHARACTER_COUNT> excluded{};

            for (const char value : characters) {
                excluded[static_cast<unsigned char>(value)] = true;
            }

            std::vector<char> complement;
            complement.reserve(ASCII_CHARACTER_COUNT);

            for (std::size_t symbol = 0; symbol < ASCII_CHARACTER_COUNT; ++symbol) {
                if (!excluded[symbol]) {
                    complement.push_back(static_cast<char>(symbol));
                }
            }

            return complement;
        }
    };
}
