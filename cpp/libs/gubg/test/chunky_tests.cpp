#include "gubg/chunky.hpp"
#include "gubg/Testing.hpp"
#include "gubg/mss.hpp"
using namespace gubg;
using namespace std;

#define GUBG_MODULE "test"
#include "gubg/log/begin.hpp"
int main()
{
    MSS_BEGIN(int);
    TEST_TAG(Chunky);
    Chunky chunky(10);
    TEST_EQ(0, chunky.size());
    TEST_TRUE(chunky.empty());
    chunky.add('a');
    TEST_EQ(1, chunky.size());
    chunky.add("ja wadde, dees begint er al op te trekken");
    L("size: " << chunky.size());
    L("output: " << chunky);
    for (auto it = chunky.begin(); it != chunky.end(); ++it)
        L("it: " << *it);
    MSS_END();
}
#include "gubg/log/end.hpp"
