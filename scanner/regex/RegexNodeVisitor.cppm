module;

export module Scanner.Regex:RegexNodeVisitor;

namespace scanner {
    export class Optional;
    export class Leaf;
    export class Concatenation;
    export class Kleene;
    export class Union;

    class RegexNodeVisitor {
      public:
        virtual ~RegexNodeVisitor() = default;

        virtual void visit(Optional &node) {}
        virtual void visit(Leaf &node) {}
        virtual void visit(Concatenation &node) {}
        virtual void visit(Kleene &node) {}
        virtual void visit(Union &node) {}
    };
} // namespace scanner