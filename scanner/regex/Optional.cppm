module;
#include <memory>

export module Scanner.Regex:Optional;

import :RegexNode;
import :RegexNodeVisitor;

namespace scanner {
    export class Optional final : public RegexNode {
    public:
        explicit Optional(const std::shared_ptr<RegexNode>& optionalNode) : optionalNode(optionalNode) {}

        std::shared_ptr<RegexNode> getOptionalNode() { return optionalNode; }

        void accept(RegexNodeVisitor& visitor) override { visitor.visit(*this); }

    private:
        std::shared_ptr<RegexNode> optionalNode{};
    };
} // namespace scanner