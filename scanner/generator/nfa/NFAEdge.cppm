module;

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

export module Scanner.NFA:NFAEdge;

namespace scanner {
    export class NFANode;

    export class NFAEdge {
    public:
        enum class CaptureAction {
            None,
            Start,
            End
        };

        explicit NFAEdge(std::uint32_t endpointID) : endpointNodeID(endpointID) {}
        NFAEdge(std::uint32_t endpointNodeID, char character) : epsilonTransition(false), characters{character}, endpointNodeID(endpointNodeID) {}
        NFAEdge(std::uint32_t endpointNodeID, const std::vector<char>& characters)
            : epsilonTransition(false), characters(characters), endpointNodeID(endpointNodeID) {}

        bool isEpsilonTransition() const { return epsilonTransition; }
        const std::vector<char>& getCharacter() const { return characters; }
        bool matchesAnySymbol() const { return matchesAnySymbolFlag; }
        std::uint32_t getEndpointID() const { return endpointNodeID; }
        bool hasCaptureAction() const { return captureAction != CaptureAction::None; }
        CaptureAction getCaptureAction() const { return captureAction; }
        std::uint32_t getCaptureGroupID() const { return captureGroupID; }
        const std::optional<std::string>& getCaptureGroupName() const { return captureGroupName; }

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

        static NFAEdge captureStart(std::uint32_t endpoint, std::uint32_t groupID, std::optional<std::string> name) {
            auto result = NFAEdge::epsilon(endpoint);
            result.captureAction = CaptureAction::Start;
            result.captureGroupID = groupID;
            result.captureGroupName = std::move(name);
            return result;
        }

        static NFAEdge captureEnd(std::uint32_t endpoint, std::uint32_t groupID, std::optional<std::string> name) {
            auto result = NFAEdge::epsilon(endpoint);
            result.captureAction = CaptureAction::End;
            result.captureGroupID = groupID;
            result.captureGroupName = std::move(name);
            return result;
        }

    private:
        bool epsilonTransition = true;
        std::vector<char> characters;
        std::uint32_t endpointNodeID = 0;
        bool matchesAnySymbolFlag = false;
        CaptureAction captureAction = CaptureAction::None;
        std::uint32_t captureGroupID = 0;
        std::optional<std::string> captureGroupName;
    };
} // namespace scanner
