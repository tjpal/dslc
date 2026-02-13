module;

#include <chrono>
#include <cstddef>
#include <optional>

export module Scanner.GenerationStatistics;

namespace scanner {
    export class GenerationStatistics {
    public:
        using Duration = std::chrono::nanoseconds;

        void setEnabled(const bool shouldCollect) {
            enabled = shouldCollect;
            if (!enabled) {
                reset();
            }
        }

        [[nodiscard]] bool isEnabled() const {
            return enabled;
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
            thompsonMergeNodesDuration = Duration::zero();
            thompsonNodeLookupDuration = Duration::zero();
            thompsonEdgeAddDuration = Duration::zero();
            mergeNfaDuration = Duration::zero();
            powerSetConversionDuration = Duration::zero();
            powerSetAlphabetCollectionDuration = Duration::zero();
            powerSetMainLoopDuration = Duration::zero();
            parsedRegexCount = 0;
            regexToNfaCount = 0;
            thompsonMergeNodesCount = 0;
            thompsonNodeLookupCount = 0;
            thompsonEdgeAddCount = 0;
            powerSetSubsetCount = 0;
            powerSetTransitionCount = 0;
            parsingStart.reset();
            regexToNfaStart.reset();
            nfaToDfaStart.reset();
            parsingRegexStart.reset();
            regexToNfaRegexStart.reset();
            regexToNfaMaterializationStart.reset();
            thompsonMergeNodesStart.reset();
            thompsonNodeLookupStart.reset();
            thompsonEdgeAddStart.reset();
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

        void startThompsonMergeNodes() {
            if (!enabled) {
                return;
            }

            thompsonMergeNodesStart = Clock::now();
        }

        void endThompsonMergeNodes() {
            if (!enabled || !thompsonMergeNodesStart.has_value()) {
                return;
            }

            thompsonMergeNodesDuration += elapsedDuration(thompsonMergeNodesStart.value());
            ++thompsonMergeNodesCount;
            thompsonMergeNodesStart.reset();
        }

        void startThompsonNodeLookup() {
            if (!enabled) {
                return;
            }

            thompsonNodeLookupStart = Clock::now();
        }

        void endThompsonNodeLookup() {
            if (!enabled || !thompsonNodeLookupStart.has_value()) {
                return;
            }

            thompsonNodeLookupDuration += elapsedDuration(thompsonNodeLookupStart.value());
            ++thompsonNodeLookupCount;
            thompsonNodeLookupStart.reset();
        }

        void startThompsonEdgeAdd() {
            if (!enabled) {
                return;
            }

            thompsonEdgeAddStart = Clock::now();
        }

        void endThompsonEdgeAdd() {
            if (!enabled || !thompsonEdgeAddStart.has_value()) {
                return;
            }

            thompsonEdgeAddDuration += elapsedDuration(thompsonEdgeAddStart.value());
            ++thompsonEdgeAddCount;
            thompsonEdgeAddStart.reset();
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

        void setPowerSetAlphabetCollectionDuration(const Duration duration) {
            if (!enabled) {
                return;
            }

            powerSetAlphabetCollectionDuration = duration;
        }

        void setPowerSetMainLoopDuration(const Duration duration) {
            if (!enabled) {
                return;
            }

            powerSetMainLoopDuration = duration;
        }

        void setPowerSetSubsetCount(const std::size_t count) {
            if (!enabled) {
                return;
            }

            powerSetSubsetCount = count;
        }

        void setPowerSetTransitionCount(const std::size_t count) {
            if (!enabled) {
                return;
            }

            powerSetTransitionCount = count;
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

        Duration getThompsonMergeNodesDuration() const {
            return thompsonMergeNodesDuration;
        }

        std::size_t getThompsonMergeNodesCount() const {
            return thompsonMergeNodesCount;
        }

        Duration getThompsonNodeLookupDuration() const {
            return thompsonNodeLookupDuration;
        }

        std::size_t getThompsonNodeLookupCount() const {
            return thompsonNodeLookupCount;
        }

        Duration getThompsonEdgeAddDuration() const {
            return thompsonEdgeAddDuration;
        }

        std::size_t getThompsonEdgeAddCount() const {
            return thompsonEdgeAddCount;
        }

        Duration getMergeNfaDuration() const {
            return mergeNfaDuration;
        }

        Duration getPowerSetConversionDuration() const {
            return powerSetConversionDuration;
        }

        Duration getPowerSetAlphabetCollectionDuration() const {
            return powerSetAlphabetCollectionDuration;
        }

        Duration getPowerSetMainLoopDuration() const {
            return powerSetMainLoopDuration;
        }

        std::size_t getPowerSetSubsetCount() const {
            return powerSetSubsetCount;
        }

        std::size_t getPowerSetTransitionCount() const {
            return powerSetTransitionCount;
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
        std::optional<Clock::time_point> thompsonMergeNodesStart;
        std::optional<Clock::time_point> thompsonNodeLookupStart;
        std::optional<Clock::time_point> thompsonEdgeAddStart;
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
        Duration thompsonMergeNodesDuration = Duration::zero();
        Duration thompsonNodeLookupDuration = Duration::zero();
        Duration thompsonEdgeAddDuration = Duration::zero();
        Duration mergeNfaDuration = Duration::zero();
        Duration powerSetConversionDuration = Duration::zero();
        Duration powerSetAlphabetCollectionDuration = Duration::zero();
        Duration powerSetMainLoopDuration = Duration::zero();
        std::size_t parsedRegexCount = 0;
        std::size_t regexToNfaCount = 0;
        std::size_t thompsonMergeNodesCount = 0;
        std::size_t thompsonNodeLookupCount = 0;
        std::size_t thompsonEdgeAddCount = 0;
        std::size_t powerSetSubsetCount = 0;
        std::size_t powerSetTransitionCount = 0;
    };
}
