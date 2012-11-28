#include "da/link/Linker.hpp"
#include <sstream>
#include <stdlib.h>
using namespace da;
using namespace da::compile;
using namespace std;

void Linker::addSetting(const Setting &setting)
{
    settings_.push_back(setting);
}
void Linker::addLibraryPath(const LibraryPath &libraryPath)
{
    libraryPaths_.push_back(libraryPath);
}

ReturnCode Linker::operator()(const ExeFile &exe, const ObjectFiles &objects)
{
    MSS_BEGIN(ReturnCode);

    //Prepare the command to be executed
    ostringstream cmd;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cmd << "g++ -std=c++0x -o " << exe.name();
        for (const auto &setting: settings_)
            cmd << " " << setting;
        for (const auto &libraryPath: libraryPaths_)
            cmd << " -L" << libraryPath.name();
        for (const auto &obj: objects)
            cmd << " " << obj.name();
    }

    //Execute the compilation command
    MSS(::system(cmd.str().c_str()) == 0, LinkingFailed);

    MSS_END();
}
