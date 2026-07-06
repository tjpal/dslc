#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

import Scanner.NFA;
import Scanner.RegexParser;
import Scanner.ThompsonConstructionVisitor;

namespace {

struct CaptureEdgeInfo {
    scanner::NFAEdge::CaptureAction action;
    std::uint32_t groupID;
    std::optional<std::string> name;
    bool isEpsilon;
};

scanner::NFA BuildNFA(const std::string& expression) {
    scanner::RegexParser parser;
    const auto regexTree = parser.parse(expression);

    scanner::NFANodeFactory nodeFactory;
    scanner::ThompsonConstructionVisitor visitor(nodeFactory);
    regexTree->accept(visitor);

    return visitor.moveConstructedNFA();
}

std::vector<CaptureEdgeInfo> CollectCaptureEdges(const scanner::NFA& nfa) {
    std::vector<CaptureEdgeInfo> captureEdges;

    for (const auto& node : nfa.getNodes()) {
        for (const auto& edge : node.getEdges()) {
            if (!edge.hasCaptureAction()) {
                continue;
            }

            captureEdges.push_back(
                {
                    edge.getCaptureAction(),
                    edge.getCaptureGroupID(),
                    edge.getCaptureGroupName(),
                    edge.isEpsilonTransition()
                }
            );
        }
    }

    return captureEdges;
}

const CaptureEdgeInfo* FindCaptureEdge(
    const std::vector<CaptureEdgeInfo>& captureEdges,
    scanner::NFAEdge::CaptureAction action
) {
    for (const auto& edge : captureEdges) {
        if (edge.action == action) {
            return &edge;
        }
    }

    return nullptr;
}

} // namespace

TEST(ThompsonConstructionVisitorCapturingGroups, EmitsCaptureEdgesForNamedGroup) {
    const auto nfa = BuildNFA("(?<name>a)");
    const auto captureEdges = CollectCaptureEdges(nfa);

    ASSERT_EQ(2u, captureEdges.size());

    const auto start = FindCaptureEdge(captureEdges, scanner::NFAEdge::CaptureAction::Start);
    ASSERT_NE(nullptr, start);
    EXPECT_TRUE(start->isEpsilon);
    EXPECT_EQ(1u, start->groupID);
    ASSERT_TRUE(start->name.has_value());
    EXPECT_EQ("name", *start->name);

    const auto end = FindCaptureEdge(captureEdges, scanner::NFAEdge::CaptureAction::End);
    ASSERT_NE(nullptr, end);
    EXPECT_TRUE(end->isEpsilon);
    EXPECT_EQ(1u, end->groupID);
    ASSERT_TRUE(end->name.has_value());
    EXPECT_EQ("name", *end->name);
}

TEST(ThompsonConstructionVisitorCapturingGroups, EmitsCaptureEdgesForUnnamedGroup) {
    const auto nfa = BuildNFA("(a)");
    const auto captureEdges = CollectCaptureEdges(nfa);

    ASSERT_EQ(2u, captureEdges.size());

    const auto start = FindCaptureEdge(captureEdges, scanner::NFAEdge::CaptureAction::Start);
    ASSERT_NE(nullptr, start);
    EXPECT_EQ(1u, start->groupID);
    EXPECT_FALSE(start->name.has_value());

    const auto end = FindCaptureEdge(captureEdges, scanner::NFAEdge::CaptureAction::End);
    ASSERT_NE(nullptr, end);
    EXPECT_EQ(1u, end->groupID);
    EXPECT_FALSE(end->name.has_value());
}
