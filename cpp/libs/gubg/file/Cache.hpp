#ifndef sdf
#define sdf

#include "gubg/file/File.hpp"

namespace gubg
{
    namespace file
    {
        template <typename Receiver>
            class Cache_crtp
            {
                public:
			Cache_crtp(const File &store):
				store_(store){}

			template <typename SufficientData>
				bool get(const File &wanted, const SufficientData &sd)
				{
					std::ostringstream oss;
					oss << "TARGET:" << wanted.name() << std::endl << "SufficientData hash:" << sd.hash();
				}

                private:
			const File store_;
            };
    }
}

#endif
