module;
#include <memory>

export module Scanner.Regex:Optional;

import :RegexNode;

namespace scanner {
    export class Optional {
      public:
        explicit Optional(const std::shared_ptr<RegexNode> &optionalNode) : optionalNode(optionalNode) {}

        std::shared_ptr<RegexNode> getOptionalNode() { return optionalNode; }

      private:
        std::shared_ptr<RegexNode> optionalNode{};
    };
} // namespace scanner