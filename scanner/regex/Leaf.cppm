module;
#include <vector>

export module Scanner.Regex:Leaf;

import :RegexNode;
import :RegexNodeVisitor;

namespace scanner {
    export class Leaf final : public RegexNode {
    public:
        void setCharacters(const std::vector<char>& characters) { this->characters = characters; }

        std::vector<char> getCharacters() { return characters; }

        void accept(RegexNodeVisitor& visitor) override { visitor.visit(*this); }

    private:
        std::vector<char> characters;
    };
} // namespace scanner