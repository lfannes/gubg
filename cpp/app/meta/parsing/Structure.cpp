#include "parsing/Structure.hpp"
using namespace meta;
using namespace std;

//#define L_ENABLE_DEBUG
#include "debug.hpp"

Structure::Structure(Code &code):
    code_(code) { } 

map<string, unsigned int> Structure::countPerName() const
{
    //Count per name
    map<string, unsigned int> cpn;

    for (auto token: tokens_)
    {
        auto name = dynamic_cast<Name*>(token);
        if (name)
            ++cpn[toCode(name->range_)];
    }

    return std::move(cpn);
}

#ifdef UnitTest
#include <iostream>
int main()
{
    Code code("#include <test.h>");
    Structure s(code);
    return 0;
}
#endif
