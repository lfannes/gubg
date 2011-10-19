#include "gubg/mss.hpp"
#include "gubg/testing/Testing.hpp"

enum class Compare {OK, Smaller, Larger};
Compare compare(int lhs, int rhs)
{
    MSS_BEGIN(Compare);
    MSS_T(lhs <= rhs, Larger);
    MSS_T(lhs >= rhs, Smaller);
    MSS_FAIL();
    MSS_RETURN();
}

int main()
{
    TEST_TAG(mss_main);
    TEST_EQ_TYPE(int, Compare::OK, compare(0, 0));
    TEST_EQ_TYPE(int, Compare::OK, compare(1, 1));
    TEST_EQ_TYPE(int, Compare::OK, compare(-2, -2));
    TEST_EQ_TYPE(int, Compare::Smaller, compare(0, 1));
    TEST_EQ_TYPE(int, Compare::Larger, compare(1, 0));
}
