module;

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <unordered_set>
#include <vector>

export module Scanner.StateSet;

namespace scanner {
    export class StateSet {
    public:
        StateSet() = default;
        StateSet(const std::initializer_list<std::uint32_t>& states) : states(states) {}

        void addState(std::uint32_t state) {
            if (isLocked) {
                throw std::runtime_error("Cannot add state to locked StateSet");
            }

            states.insert(state);
        }

        bool contains(std::uint32_t state) const {
            if (isLocked) {
                return std::ranges::binary_search(lockedStates, state);
            }

            return states.contains(state);
        }

        const std::vector<std::uint32_t>& getLockedStates() const {
            if (!isLocked) {
                throw std::runtime_error("Cannot access locked states before calling lock");
            }

            return lockedStates;
        }

        std::uint32_t getHash() const {
            if (!isLocked) {
                throw std::runtime_error("Cannot access hash before calling lock");
            }

            return hash;
        }

        /*
         * Locks the StateSet, making it immutable and allowing for efficient comparison.
         * Both the hash and sorting the array helps with comparison efficiency.
         */
        void lock() {
            lockedStates = std::vector<std::uint32_t>(states.cbegin(), states.cend());
            std::sort(lockedStates.begin(), lockedStates.end());

            states.clear();
            hash = computeHash(lockedStates);

            isLocked = true;
        }

        bool operator==(const StateSet& other) const {
            if (!isLocked || !other.isLocked) {
                throw std::runtime_error("Cannot compare unlocked StateSets");
            }

            if (lockedStates.size() != other.lockedStates.size()) {
                return false;
            }

            if (hash != other.hash) {
                return false;
            }

            return lockedStates == other.lockedStates;
        }

    private:
        // Based on http://www.cse.yorku.ca/~oz/hash.html
        std::uint32_t computeHash(const std::vector<std::uint32_t>& states) const {
            std::uint32_t hash = 5381;

            for (const std::uint32_t state : states) {
                hash = (hash << 5) + hash + state;
            }

            return hash;
        }

    private:
        std::unordered_set<std::uint32_t> states;
        std::vector<std::uint32_t> lockedStates;
        std::uint32_t hash = 0;
        bool isLocked = false;
    };
} // namespace scanner
