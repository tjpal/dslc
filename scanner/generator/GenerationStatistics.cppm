module;

#include <chrono>
#include <cstddef>
#include <optional>

export module Scanner.GenerationStatistics;

namespace scanner {
    export class GenerationStatistics {
    public:
        using Duration = std::chrono::microseconds;

        void setEnabled(const bool shouldCollect) {
            enabled = shouldCollect;
            if (!enabled) {
                reset();
            }
        }

        void reset() {
            parsingDuration = Duration::zero();
            regexToNfaDuration = Duration::zero();
            nfaToDfaDuration = Duration::zero();
            parsingRegexDuration = Duration::zero();
            parsingLongestRegexDuration = Duration::zero();
            regexToNfaRegexDuration = Duration::zero();
            regexToNfaLongestRegexDuration = Duration::zero();
            regexToNfaMaterializationDuration = Duration::zero();
            mergeNfaDuration = Duration::zero();
            powerSetConversionDuration = Duration::zero();
            parsedRegexCount = 0;
            regexToNfaCount = 0;
            parsingStart.reset();
            regexToNfaStart.reset();
            nfaToDfaStart.reset();
            parsingRegexStart.reset();
            regexToNfaRegexStart.reset();
            regexToNfaMaterializationStart.reset();
            mergeNfaStart.reset();
            powerSetConversionStart.reset();
        }

        void startParsing() {
            if (!enabled) {
                return;
            }

            parsingStart = Clock::now();
        }

        void endParsing() {
            if (!enabled || !parsingStart.has_value()) {
                return;
            }

            parsingDuration = elapsedDuration(parsingStart.value());
            parsingStart.reset();
        }

        void startParsingRegex() {
            if (!enabled) {
                return;
            }

            parsingRegexStart = Clock::now();
        }

        void endParsingRegex() {
            if (!enabled || !parsingRegexStart.has_value()) {
                return;
            }

            const Duration elapsed = elapsedDuration(parsingRegexStart.value());
            parsingRegexDuration += elapsed;
            if (elapsed > parsingLongestRegexDuration) {
                parsingLongestRegexDuration = elapsed;
            }

            ++parsedRegexCount;
            parsingRegexStart.reset();
        }

        void startRegexToNfa() {
            if (!enabled) {
                return;
            }

            regexToNfaStart = Clock::now();
        }

        void endRegexToNfa() {
            if (!enabled || !regexToNfaStart.has_value()) {
                return;
            }

            regexToNfaDuration = elapsedDuration(regexToNfaStart.value());
            regexToNfaStart.reset();
        }

        void startRegexToNfaRegex() {
            if (!enabled) {
                return;
            }

            regexToNfaRegexStart = Clock::now();
        }

        void endRegexToNfaRegex() {
            if (!enabled || !regexToNfaRegexStart.has_value()) {
                return;
            }

            const Duration elapsed = elapsedDuration(regexToNfaRegexStart.value());
            regexToNfaRegexDuration += elapsed;
            if (elapsed > regexToNfaLongestRegexDuration) {
                regexToNfaLongestRegexDuration = elapsed;
            }

            ++regexToNfaCount;
            regexToNfaRegexStart.reset();
        }

        void startNfaToDfa() {
            if (!enabled) {
                return;
            }

            nfaToDfaStart = Clock::now();
        }

        void endNfaToDfa() {
            if (!enabled || !nfaToDfaStart.has_value()) {
                return;
            }

            nfaToDfaDuration = elapsedDuration(nfaToDfaStart.value());
            nfaToDfaStart.reset();
        }

        void startRegexToNfaMaterialization() {
            if (!enabled) {
                return;
            }

            regexToNfaMaterializationStart = Clock::now();
        }

        void endRegexToNfaMaterialization() {
            if (!enabled || !regexToNfaMaterializationStart.has_value()) {
                return;
            }

            regexToNfaMaterializationDuration += elapsedDuration(regexToNfaMaterializationStart.value());
            regexToNfaMaterializationStart.reset();
        }

        void startMergeNfa() {
            if (!enabled) {
                return;
            }

            mergeNfaStart = Clock::now();
        }

        void endMergeNfa() {
            if (!enabled || !mergeNfaStart.has_value()) {
                return;
            }

            mergeNfaDuration = elapsedDuration(mergeNfaStart.value());
            mergeNfaStart.reset();
        }

        void startPowerSetConversion() {
            if (!enabled) {
                return;
            }

            powerSetConversionStart = Clock::now();
        }

        void endPowerSetConversion() {
            if (!enabled || !powerSetConversionStart.has_value()) {
                return;
            }

            powerSetConversionDuration = elapsedDuration(powerSetConversionStart.value());
            powerSetConversionStart.reset();
        }

        Duration getParsingDuration() const {
            return parsingDuration;
        }

        Duration getRegexToNfaDuration() const {
            return regexToNfaDuration;
        }

        Duration getNfaToDfaDuration() const {
            return nfaToDfaDuration;
        }

        Duration getParsingRegexDuration() const {
            return parsingRegexDuration;
        }

        Duration getParsingLongestRegexDuration() const {
            return parsingLongestRegexDuration;
        }

        std::size_t getParsedRegexCount() const {
            return parsedRegexCount;
        }

        Duration getRegexToNfaRegexDuration() const {
            return regexToNfaRegexDuration;
        }

        Duration getRegexToNfaLongestRegexDuration() const {
            return regexToNfaLongestRegexDuration;
        }

        std::size_t getRegexToNfaCount() const {
            return regexToNfaCount;
        }

        Duration getRegexToNfaMaterializationDuration() const {
            return regexToNfaMaterializationDuration;
        }

        Duration getMergeNfaDuration() const {
            return mergeNfaDuration;
        }

        Duration getPowerSetConversionDuration() const {
            return powerSetConversionDuration;
        }

    private:
        using Clock = std::chrono::steady_clock;

        static Duration elapsedDuration(const Clock::time_point start) {
            return std::chrono::duration_cast<Duration>(Clock::now() - start);
        }

        bool enabled = false;
        std::optional<Clock::time_point> parsingStart;
        std::optional<Clock::time_point> regexToNfaStart;
        std::optional<Clock::time_point> nfaToDfaStart;
        std::optional<Clock::time_point> parsingRegexStart;
        std::optional<Clock::time_point> regexToNfaRegexStart;
        std::optional<Clock::time_point> regexToNfaMaterializationStart;
        std::optional<Clock::time_point> mergeNfaStart;
        std::optional<Clock::time_point> powerSetConversionStart;
        Duration parsingDuration = Duration::zero();
        Duration regexToNfaDuration = Duration::zero();
        Duration nfaToDfaDuration = Duration::zero();
        Duration parsingRegexDuration = Duration::zero();
        Duration parsingLongestRegexDuration = Duration::zero();
        Duration regexToNfaRegexDuration = Duration::zero();
        Duration regexToNfaLongestRegexDuration = Duration::zero();
        Duration regexToNfaMaterializationDuration = Duration::zero();
        Duration mergeNfaDuration = Duration::zero();
        Duration powerSetConversionDuration = Duration::zero();
        std::size_t parsedRegexCount = 0;
        std::size_t regexToNfaCount = 0;
    };
}
