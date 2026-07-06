module;

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

export module Scanner.Regex:CapturingGroup;

import :RegexNode;
import :RegexNodeVisitor;

namespace scanner {
    export class CapturingGroup final : public RegexNode {
    public:
        CapturingGroup(const std::shared_ptr<RegexNode>& groupNode, std::uint32_t groupID)
            : groupNode(groupNode), groupID(groupID) {}

        CapturingGroup(const std::shared_ptr<RegexNode>& groupNode, std::uint32_t groupID, std::string name)
            : groupNode(groupNode), groupID(groupID), name(std::move(name)) {}

        std::shared_ptr<RegexNode> getGroupNode() { return groupNode; }
        std::uint32_t getGroupID() const { return groupID; }
        const std::optional<std::string>& getName() const { return name; }

        void accept(RegexNodeVisitor& visitor) override { visitor.visit(*this); }

    private:
        std::shared_ptr<RegexNode> groupNode{};
        std::uint32_t groupID = 0;
        std::optional<std::string> name;
    };
} // namespace scanner
