#include <gtest/gtest.h>
#include "basicobjects.h"


TEST(BasicObjectsRepr, Boolean) {
    BobObject* bobbool = new BobBoolean(true);
    EXPECT_EQ("#t", bobbool->repr());
}


TEST(BasicObjectsRepr, Pair) {
    BobObject* num129 = new BobNumber(129);
    BobObject* num130 = new BobNumber(130);
    BobObject* null = new BobNull();

    BobObject* pair1 = new BobPair(num129, num130);
    EXPECT_EQ("(129 . 130)", pair1->repr());
    BobObject* pair2 = new BobPair(num130, null);
    EXPECT_EQ("(130)", pair2->repr());
    BobObject* pair3 = new BobPair(num129, pair2);
    EXPECT_EQ("(129 130)", pair3->repr());
    BobObject* pair4 = new BobPair(pair1, pair3);
    EXPECT_EQ("((129 . 130) 129 130)", pair4->repr());
}


