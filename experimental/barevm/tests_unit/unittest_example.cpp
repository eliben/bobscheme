// Sanity check - make sure gtest linked correctly and the include
// paths are set up
//
#include <gtest/gtest.h>
#include "bytecode.h"


TEST(SanityTest, Trivial) {
  EXPECT_EQ(1, 1);
}

