#include <gtest/gtest.h>
#include "basicobjects.h"


TEST(BasicObjects, Boolean) {
    BobObject* bobbool = new BobBoolean(true);
    EXPECT_EQ("true", bobbool->repr());
}



