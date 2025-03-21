module;

#include <cstdint>
#include <memory>

export module Scanner.NFA:NFAEdge;

namespace scanner {
    export class NFANode;

    export class NFAEdge {
    public:
        explicit NFAEdge(std::uint32_t endpointID) : endpointID(endpointID) {}
        NFAEdge(std::uint32_t endpointID, char character) : endpointID(endpointID), character(character) {}

        bool isEpsilonTransition() const { return epsilonTransition; }
        char getCharacter() const { return character; }
        std::uint32_t getEndpointID() const { return endpointID; }

    private:
        bool epsilonTransition = false;
        char character = '\0';
        std::uint32_t endpointID = 0;
    };
} // namespace scanner