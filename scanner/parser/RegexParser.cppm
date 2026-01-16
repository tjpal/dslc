module;

#include <cstddef>
#include <memory>
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
                return createLeaf({consumeEscapedCharacter()});
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

            const auto readClassChar = [this]() -> char {
                if (match('\\')) {
                    return consumeEscapedCharacter();
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

            while (hasNext()) {
                if (peek() == ']') {
                    get();
                    closed = true;
                    break;
                }

                const char start = readClassChar();

                if (hasNext() && peek() == '-' && (position + 1) < expression.size() && expression[position + 1] != ']') {
                    ++position;
                    const char end = readClassChar();

                    const auto startValue = static_cast<unsigned char>(start);
                    const auto endValue = static_cast<unsigned char>(end);

                    if (startValue > endValue) {
                        throw std::runtime_error("Invalid character range");
                    }

                    for (unsigned int symbol = startValue; symbol <= endValue; ++symbol) {
                        characters.push_back(static_cast<char>(symbol));
                    }

                    continue;
                }

                characters.push_back(start);
            }

            if (!closed) {
                throw std::runtime_error("Unterminated character class");
            }

            if (characters.empty()) {
                throw std::runtime_error("Character class must contain at least one character");
            }

            return characters;
        }
    };
}
