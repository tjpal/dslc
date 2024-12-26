module;
#include <vector>

export module Scanner.Regex:Leaf;

import :RegexNode;

namespace scanner {
    export class Leaf {
      public:
        void setCharacters(const std::vector<char> &characters) { this->characters = characters; }

        std::vector<char> getCharacters() { return characters; }

      private:
        std::vector<char> characters;
    };
} // namespace scanner