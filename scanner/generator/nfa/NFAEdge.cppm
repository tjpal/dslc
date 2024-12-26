module;

#include <memory>

export module Scanner.NFA:NFAEdge;

namespace scanner {
    export class NFANode;

    export class NFAEdge {
    public:
        explicit NFAEdge(const std::shared_ptr<NFANode> &endpoint) : epsilonTransition(true), endpoint(endpoint) {}
        NFAEdge(const std::shared_ptr<NFANode> &endpoint, char character) : endpoint(endpoint), character(character) {}

        bool isEpsilonTransition() const { return epsilonTransition; }
        char getCharacter() const { return character; }
        std::shared_ptr<NFANode> getEndpoint() const { return endpoint.lock(); }

    private:
        bool epsilonTransition = false;
        char character = '\0';

        std::weak_ptr<NFANode> endpoint;
    };
} // namespace scanner