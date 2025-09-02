module;

#include <cstdint>
#include <vector>

export module Scanner.NFA:NFAEdge;

namespace scanner {
    export class NFANode;

    export class NFAEdge {
    public:
        explicit NFAEdge(std::uint32_t endpointID) : endpointNodeID(endpointID) {}
        NFAEdge(std::uint32_t endpointNodeID, char character) : epsilonTransition(false), characters{character}, endpointNodeID(endpointNodeID) {}
        NFAEdge(std::uint32_t endpointNodeID, const std::vector<char>& characters)
            : epsilonTransition(false), characters(characters), endpointNodeID(endpointNodeID) {}

        bool isEpsilonTransition() const { return epsilonTransition; }
        std::vector<char> getCharacter() const { return characters; }
        std::uint32_t getEndpointID() const { return endpointNodeID; }

    private:
        bool epsilonTransition = true;
        std::vector<char> characters;
        std::uint32_t endpointNodeID = 0;
    };
} // namespace scanner