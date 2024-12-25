module;
#include <memory>

export module Scanner.Union;

import Scanner.RegexNode;

namespace scanner {
    export class Union {
      public:
        explicit Union(const std::shared_ptr<RegexNode> &left, const std::shared_ptr<RegexNode> &right)
            : left(left), right(right) {}

        std::shared_ptr<RegexNode> getLeft() { return left; }

        std::shared_ptr<RegexNode> getRight() { return right; }

      private:
        std::shared_ptr<RegexNode> left;
        std::shared_ptr<RegexNode> right;
    };
} // namespace scanner
