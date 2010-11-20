#include <gtest/gtest.h>
#include "environment.h"
#include "basicobjects.h"

using namespace std;


class EnvironmentTextFixture : public ::testing::Test 
{
public:
    virtual void SetUp() 
    {
        valnum1 = new BobNumber(1);
        valnum2 = new BobNumber(2);
        valsym1 = new BobSymbol("symbol1");
        valsym2 = new BobSymbol("symbol2");
    }

    BobObject *valnum1, *valnum2;
    BobObject *valsym1, *valsym2;
};


TEST_F(EnvironmentTextFixture, Basic) 
{
    BobEnvironment* env1 = new BobEnvironment(0);
    env1->define_var("jack", valnum1);

    EXPECT_EQ(valnum1, env1->lookup_var("jack"));
    EXPECT_EQ(0, env1->lookup_var("yvonne"));
}


TEST_F(EnvironmentTextFixture, Replacement) 
{
    BobEnvironment* env1 = new BobEnvironment(0);
    env1->define_var("foo", valsym1);
    env1->define_var("bar", valsym2);

    EXPECT_EQ(valsym1, env1->lookup_var("foo"));
    EXPECT_EQ(valsym2, env1->lookup_var("bar"));

    env1->set_var_value("bar", valnum1);

    EXPECT_EQ(valsym1, env1->lookup_var("foo"));
    EXPECT_EQ(valnum1, env1->lookup_var("bar"));
    EXPECT_EQ(0, env1->set_var_value("baz", valnum2));
    EXPECT_EQ(valnum2, env1->set_var_value("foo", valnum2));
    EXPECT_EQ(valnum2, env1->lookup_var("foo"));
}


TEST_F(EnvironmentTextFixture, Nested) 
{
    BobEnvironment* envparent = new BobEnvironment(0);
    BobEnvironment* envchild = new BobEnvironment(envparent);

    envparent->define_var("foo", valnum1);
    envchild->define_var("bar", valnum2);

    EXPECT_EQ(valnum2, envchild->lookup_var("bar"));
    EXPECT_EQ(valnum1, envchild->lookup_var("foo"));
    EXPECT_EQ(0, envparent->lookup_var("bar"));

    envchild->set_var_value("foo", valsym1);
    EXPECT_EQ(valsym1, envchild->lookup_var("foo"));
    EXPECT_EQ(valsym1, envparent->lookup_var("foo"));
    EXPECT_EQ(0, envchild->set_var_value("nosuchvar", valnum2));

    envchild->define_var("jason", valsym2);
    EXPECT_EQ(0, envparent->lookup_var("jason"));
}

