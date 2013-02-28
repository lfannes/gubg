#ifndef aoeuaoue
#define aoeuaoue

#include "gubg/msgpack/Write.hpp"

namespace gubg
{
    namespace msgpack
    {
        template <typename Buffer>
            class Array
            {
                public:
                    Array(Buffer &buffer, size_t nr):
                        buffer_(buffer), nr_(nr), ix_(0)
                {
                    write(buffer_, nr_, ArrayTL_tag());
                }

                    bool isComplete() const { return ix_ == nr_; }

                    bool push(long l)
                    {
                        if (ix_ >= nr_)
                            return false;
                        write(buffer_, l);
                        ++ix_;
                        return true;
                    }

                private:
                    Buffer &buffer_;
                    const size_t nr_;
                    size_t ix_;
            };

        template <typename Buffer>
            class Map
            {
                public:
                    Map(Buffer &buffer, size_t nr):
                        buffer_(buffer), nr_(nr), ix_(0)
                {
                    write(buffer_, nr_, MapTL_tag());
                }

                    bool isComplete() const { return ix_ == nr_; }

                    bool push(long id, long l)
                    {
                        if (ix_ >= nr_)
                            return false;
                        write(buffer_, id);
                        write(buffer_, l);
                        ++ix_;
                        return true;
                    }

                private:
                    Buffer &buffer_;
                    const size_t nr_;
                    size_t ix_;
            };
    }
}

#endif
