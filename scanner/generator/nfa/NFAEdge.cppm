module;

#include <cstdint>
#include <memory>

export module Scanner.NFA:NFAEdge;

namespace scanner {
    export class NFANode;

    export class NFAEdge {
    public:
        explicit NFAEdge(std::uint32_t endpointID) : endpointNodeID(endpointID) {}
        NFAEdge(std::uint32_t endpointNodeID, char character) : endpointNodeID(endpointNodeID), character(character), epsilonTransition(false) {}

        bool isEpsilonTransition() const { return epsilonTransition; }
        char getCharacter() const { return character; }
        std::uint32_t getEndpointID() const { return endpointNodeID; }

    private:
        bool epsilonTransition = true;
        char character = '\0';
        std::uint32_t endpointNodeID = 0;
    };
} // namespace scanner