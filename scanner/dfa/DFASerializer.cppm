module;

#include <array>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

export module Scanner.DFASerializer;

import Scanner.DFA;
import Scanner.DFAAcceptingState;

namespace scanner {
    export class DFASerializer final {
    public:
        static void serialize(const DFA& dfa, const std::filesystem::path& path) {
            std::ofstream output(path, std::ios::binary);
            if (!output) {
                throw std::runtime_error("Failed to open " + path.string() + " for writing");
            }

            output.write(Magic.data(), Magic.size());
            if (!output) {
                throw std::runtime_error("Failed to write DFA header");
            }

            write(output, Version);

            const auto& alphabet = dfa.getAlphabet();
            if (alphabet.size() > std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error("Alphabet size exceeds serializable limits");
            }

            write(output, static_cast<std::uint32_t>(alphabet.size()));
            if (!alphabet.empty()) {
                output.write(alphabet.data(), static_cast<std::streamsize>(alphabet.size()));
                if (!output) {
                    throw std::runtime_error("Failed to write DFA alphabet");
                }
            }

            const std::uint32_t stateCount = dfa.getStateCount();
            write(output, stateCount);

            const std::uint32_t alphabetSize = static_cast<std::uint32_t>(alphabet.size());
            for (std::uint32_t state = 0; state < stateCount; ++state) {
                for (std::uint32_t symbolIndex = 0; symbolIndex < alphabetSize; ++symbolIndex) {
                    write(output, dfa.getNextState(state, symbolIndex));
                }

                const bool isAccepting = dfa.isAcceptingState(state);
                write(output, static_cast<std::uint8_t>(isAccepting ? 1 : 0));

                if (isAccepting) {
                    const auto ids = dfa.getAcceptingIds(state);
                    if (ids.size() > std::numeric_limits<std::uint32_t>::max()) {
                        throw std::runtime_error("Accepting ID list too large");
                    }

                    write(output, static_cast<std::uint32_t>(ids.size()));
                    for (const std::uint32_t id : ids) {
                        write(output, id);
                    }
                }
            }
        }

    static DFA deserialize(const std::filesystem::path& path)  {
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            throw std::runtime_error("Failed to open " + path.string() + " for reading");
        }

        std::array<char, 4> header{};
        input.read(header.data(), header.size());
        if (!input || header != Magic) {
            throw std::runtime_error("Invalid DFA file header");
        }

        const std::uint32_t fileVersion = read<std::uint32_t>(input);
        if (fileVersion != Version) {
            throw std::runtime_error("Unsupported DFA file version");
        }

        const std::uint32_t alphabetSize = read<std::uint32_t>(input);
        std::vector<char> alphabet;
        alphabet.resize(alphabetSize);

        if (alphabetSize > 0) {
            input.read(alphabet.data(), static_cast<std::streamsize>(alphabet.size()));

            if (!input) {
                throw std::runtime_error("Failed to read DFA alphabet");
            }
        }

        const std::uint32_t stateCount = read<std::uint32_t>(input);
        std::vector<std::vector<std::uint32_t>> transitionTable;
        transitionTable.resize(stateCount, std::vector<std::uint32_t>(alphabetSize));

        std::vector<DFAAcceptingState> acceptingStates;
        acceptingStates.reserve(stateCount);

        for (std::uint32_t state = 0; state < stateCount; ++state) {
            for (std::uint32_t symbolIndex = 0; symbolIndex < alphabetSize; ++symbolIndex) {
                transitionTable[state][symbolIndex] = read<std::uint32_t>(input);
            }

            const std::uint8_t acceptingFlag = read<std::uint8_t>(input);
            const bool isAccepting = acceptingFlag != 0;
            std::vector<std::uint32_t> acceptingIds;

            if (isAccepting) {
                const std::uint32_t idCount = read<std::uint32_t>(input);
                acceptingIds.reserve(idCount);
                for (std::uint32_t i = 0; i < idCount; ++i) {
                    acceptingIds.push_back(read<std::uint32_t>(input));
                }
            }

            acceptingStates.emplace_back(isAccepting, std::move(acceptingIds));
        }

        return DFA(transitionTable, acceptingStates, alphabet);
    }

    private:
        template<typename T>
        static void write(std::ostream& output, T value) {
            using UnsignedType = std::make_unsigned_t<T>;
            UnsignedType normalized = static_cast<UnsignedType>(value);
            std::array<char, sizeof(UnsignedType)> buffer{};

            std::memcpy(buffer.data(), &normalized, buffer.size());
            output.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));

            if (!output) {
                throw std::runtime_error("Failed to write numeric data");
            }
        }

        template<typename T>
        static T read(std::istream& input) {
            using UnsignedType = std::make_unsigned_t<T>;
            std::array<char, sizeof(UnsignedType)> buffer{};

            input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            if (!input) {
                throw std::runtime_error("Failed to read numeric data");
            }

            UnsignedType normalized = 0;
            std::memcpy(&normalized, buffer.data(), buffer.size());

            return static_cast<T>(normalized);
        }

    private:
        static constexpr std::array<char, 4> Magic{'d', 's', 'l', 'c'};
        static constexpr std::uint32_t Version = 1;
    };
}
