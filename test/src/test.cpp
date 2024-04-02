#include <gtest/gtest.h>

#include "library.h"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    EXPECT_EQ(hello(), 0);
}
