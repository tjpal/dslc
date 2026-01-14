module;
#include <memory>

export module Scanner.Regex:Concatenation;

import :RegexNode;
import :RegexNodeVisitor;

namespace scanner {
    export class Concatenation final : public RegexNode {
    public:
        explicit Concatenation(const std::shared_ptr<RegexNode>& left, const std::shared_ptr<RegexNode>& right)
            : left(left), right(right) {}

        std::shared_ptr<RegexNode> getLeft() { return left; }
        std::shared_ptr<RegexNode> getRight() { return right; }

        void accept(RegexNodeVisitor& visitor) override { visitor.visit(*this); }

    private:
        std::shared_ptr<RegexNode> left;
        std::shared_ptr<RegexNode> right;
    };
} // namespace scanner
