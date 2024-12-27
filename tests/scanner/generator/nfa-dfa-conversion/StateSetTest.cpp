#include <gtest/gtest.h>

import Scanner.StateSet;

TEST(StateSetTests, AddStateToLockedStateSetThrows) {
    scanner::StateSet stateSet{1};
    stateSet.lock();

    EXPECT_THROW(stateSet.addState(2), std::runtime_error);
}

TEST(StateSetTests, CompareLockedStateSetsEqual) {
    scanner::StateSet stateSet1{1, 3};
    stateSet1.lock();

    scanner::StateSet stateSet2{1, 3};
    stateSet2.lock();

    EXPECT_TRUE(stateSet1 == stateSet2);
}

TEST(StateSetTests, CompareLockedStateSetsNotEqual) {
    scanner::StateSet stateSet1{1, 3};
    stateSet1.lock();

    scanner::StateSet stateSet2{1, 3, 4};
    stateSet2.lock();

    EXPECT_FALSE(stateSet1 == stateSet2);
}

TEST(StateSetTests, CompareUnlockStateSetThrows) {
    scanner::StateSet stateSet1{1, 3};
    stateSet1.lock();

    scanner::StateSet stateSet2{1, 3, 4};

    EXPECT_THROW(stateSet1.operator==(stateSet2), std::runtime_error);
}