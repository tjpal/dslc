module;
#include <memory>

export module Scanner.Regex:Plus;

import :RegexNode;
import :RegexNodeVisitor;

namespace scanner {
    export class Plus final : public RegexNode {
    public:
        explicit Plus(const std::shared_ptr<RegexNode>& plusNode) : plusNode(plusNode) {}

        std::shared_ptr<RegexNode> getPlusNode() { return plusNode; }

        void accept(RegexNodeVisitor& visitor) override { visitor.visit(*this); }

    private:
        std::shared_ptr<RegexNode> plusNode{};
    };
} // namespace scanner
