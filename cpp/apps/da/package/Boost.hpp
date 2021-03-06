#ifndef HEADER_da_package_Boost_hpp_ALREADY_INCLUDED
#define HEADER_da_package_Boost_hpp_ALREADY_INCLUDED

#include "da/package/Package.hpp"
#include "gubg/file/File.hpp"

namespace da
{
    namespace package
    {
        class Boost: public Package
        {
            public:
                template <typename T>
                static Ptr create(T t){return Ptr(new Boost(t));}

                //Package API
                virtual std::string name() const {return "boost";}
                virtual bool exists() const;
                virtual ReturnCode resolveHeader(File &resolvedHeader, SourceFiles &sisterFiles, const File &partial);

            private:
                Boost(const gubg::file::File &base);

                gubg::file::File base_;
                gubg::file::File libDir_;
        };
    }
}

#endif
