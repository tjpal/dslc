export module Scanner.Regex:RegexNode;

namespace scanner {
    class RegexNodeVisitor;

    export class RegexNode {
    public:
        virtual ~RegexNode() = default;

        virtual void accept(RegexNodeVisitor& visitor) = 0;
    };
} // namespace scanner