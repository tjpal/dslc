module;

#include <cstdint>
#include <vector>

export module Scanner.DFAAcceptingState;

namespace scanner {
    export class DFAAcceptingState final {
    public:
        DFAAcceptingState(bool isAccepting, std::vector<std::uint32_t> nfaIds) :
            isAccepting(isAccepting), nfaIds(std::move(nfaIds)) {
        }

        bool getIsAccepting() const {
            return isAccepting;
        }

        const std::vector<std::uint32_t>& getNfaIds() const {
            return nfaIds;
        }

    private:

        bool isAccepting;
        std::vector<std::uint32_t> nfaIds;
    };
}