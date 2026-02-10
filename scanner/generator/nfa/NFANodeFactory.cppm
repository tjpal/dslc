module;

#include <cstdint>

export module Scanner.NFA:NFANodeFactory;

import :NFANode;

namespace scanner {
    export class NFANodeFactory {
    public:
        NFANode createNode() {
            return NFANode(nextNodeID++);
        }

    private:
        std::uint32_t nextNodeID = 0;
    };
} // namespace scanner
