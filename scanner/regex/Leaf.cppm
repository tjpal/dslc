module;
#include <vector>

export module Scanner.Regex:Leaf;

import :RegexNode;
import :RegexNodeVisitor;

namespace scanner {
    export class Leaf final : public RegexNode {
    public:
        void setCharacters(const std::vector<char>& charactersValue) {
            characters = charactersValue;
            wildcard = false;
        }

        std::vector<char> getCharacters() const { return characters; }

        void setWildcard(bool wildcardState = true) {
            wildcard = wildcardState;
            if (wildcard) {
                characters.clear();
            }
        }

        bool isWildcard() const { return wildcard; }

        void accept(RegexNodeVisitor& visitor) override { visitor.visit(*this); }

    private:
        std::vector<char> characters;
        bool wildcard = false;
    };
} // namespace scanner
