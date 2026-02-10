#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

import Scanner.NFA;

namespace {

scanner::NFA BuildTwoNodeNFA() {
    scanner::NFANodeFactory nodeFactory;
    auto start = nodeFactory.createNode();
    auto accepting = nodeFactory.createNode();

    start.addEdge(scanner::NFAEdge::epsilon(accepting.getNodeID()));

    std::vector<scanner::NFANode> nodes = {start, accepting};
    return {start, std::move(nodes), accepting};
}

} // namespace

TEST(NFALockTests, LockBuildsLookupForConstNodeAccess) {
    scanner::NFA nfa = BuildTwoNodeNFA();
    const std::uint32_t startID = nfa.getStartNodeID();
    const std::uint32_t acceptingID = nfa.getAcceptingNodeID();

    nfa.lock();
    const scanner::NFA& lockedNFA = nfa;

    EXPECT_EQ(lockedNFA.getNodeByID(startID).getNodeID(), startID);
    EXPECT_EQ(lockedNFA.getNodeByID(acceptingID).getNodeID(), acceptingID);
}

TEST(NFALockTests, LockDisallowsMutationOperations) {
    scanner::NFA nfa = BuildTwoNodeNFA();
    scanner::NFANodeFactory nodeFactory;
    auto extraNode = nodeFactory.createNode();

    nfa.lock();

    EXPECT_THROW(nfa.addNode(std::move(extraNode)), std::runtime_error);
    EXPECT_THROW(nfa.getNodeByID(nfa.getStartNodeID()), std::runtime_error);
}
