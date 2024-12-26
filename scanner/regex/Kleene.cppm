module;
#include <memory>

export module Scanner.Regex:Kleene;

import :RegexNode;

namespace scanner {
    export class Kleene {
      public:
        explicit Kleene(const std::shared_ptr<RegexNode> &kleeneNode) : kleeneNode(kleeneNode) {}

        std::shared_ptr<RegexNode> getKleeneNode() { return kleeneNode; }

      private:
        std::shared_ptr<RegexNode> kleeneNode{};
    };
} // namespace scanner
