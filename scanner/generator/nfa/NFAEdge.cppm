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
        bool matchesAnySymbol() const { return matchesAnySymbolFlag; }
        std::uint32_t getEndpointID() const { return endpointNodeID; }

        static NFAEdge epsilon(std::uint32_t endpoint) {
            auto result = NFAEdge(endpoint);
            result.epsilonTransition = true;
            return result;
        }

        static NFAEdge wildcard(std::uint32_t endpoint) {
            auto result = NFAEdge(endpoint);
            result.epsilonTransition = false;
            result.matchesAnySymbolFlag = true;
            return result;
        }

    private:
        bool epsilonTransition = true;
        std::vector<char> characters;
        std::uint32_t endpointNodeID = 0;
        bool matchesAnySymbolFlag = false;
    };
} // namespace scanner
