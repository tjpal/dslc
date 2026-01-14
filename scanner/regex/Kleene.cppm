module;
#include <memory>

export module Scanner.Regex:Kleene;

import :RegexNode;
import :RegexNodeVisitor;

namespace scanner {
    export class Kleene final : public RegexNode {
    public:
        explicit Kleene(const std::shared_ptr<RegexNode>& kleeneNode) : kleeneNode(kleeneNode) {}

        std::shared_ptr<RegexNode> getKleeneNode() { return kleeneNode; }

        void accept(RegexNodeVisitor& visitor) override { visitor.visit(*this); }

    private:
        std::shared_ptr<RegexNode> kleeneNode{};
    };
} // namespace scanner
