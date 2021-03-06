#ifndef HEADER_gubg_msgpack_Serializer_hpp_ALREADY_INCLUDED
#define HEADER_gubg_msgpack_Serializer_hpp_ALREADY_INCLUDED

#include "gubg/msgpack/Types.hpp"
#include "gubg/msgpack/Write.hpp"
#include "gubg/FixedVector.hpp"
#include "gubg/mss.hpp"

#define GUBG_MODULE "Serializer"
#include "gubg/log/begin.hpp"
namespace gubg { namespace msgpack {

    template <typename Buffer, size_t MaxDepth>
        class Serializer
    {
        public:
            class Composer
            {
                public:
                    typedef Serializer<Buffer, MaxDepth> Outer;

                    Composer(Outer &outer, size_t el_cnt):
                        outer_(outer)
                {
                    if (!MSS_IS_OK(write(outer_.buffer_, el_cnt, MapTL_tag())))
                        return;

                    {
                        auto &elementInfos = outer_.elementInfos_;
                        const auto s = elementInfos.size();
                        elementInfos.push_back(ElementInfo(el_cnt));
                        //Check that push_back() worked. For STL, this would be no problem, but on
                        //Arduino, this cannot be used, and FixedVector can fail to push_back()
                        //if the vector is full
                        if (elementInfos.size() != s+1)
                            return;
                    }

                    ok_ = true;
                }
                    //We only allow moving, no copying
                    Composer(Composer &&rhs): outer_(rhs.outer_), ok_(rhs.ok_) { }
                    Composer(const Composer &rhs) = delete;
                    Composer &operator=(const Composer &rhs) = delete;
                    Composer &operator=(Composer &&rhs) = delete;

                    ~Composer()
                    {
                        if (!ok())
                            return;
                        assert(full());
                        outer_.elementInfos_.pop_back();
                    }

                    template <typename T>
                        bool put(RoleId rid, const T &t)
                        {
							S();L(STREAM(rid));
                            assert(!full());
                            if (full())
								return false;
                            return outer_.put(rid, t);
                        }
                    template <typename T>
                        bool put(const T &t)
                        {
                            return put(T::TypeId, t);
                        }

                    bool ok() const {return ok_;}
                    bool full() const
                    {
                        if (!ok())
                            return false;
                        const auto &ei = outer_.elementInfos_.back();
                        return ei.ix == ei.nr;
                    }


                private:
                    Outer &outer_;
                    bool ok_ = false;
            };

            void clear()
            {
                buffer_.clear();
                elementInfos_.clear();
            }
            ReturnCode swap(Buffer &buffer)
            {
                MSS_BEGIN(ReturnCode);
                MSS(elementInfos_.empty());
                buffer_.swap(buffer);
                MSS_END();
            }
            const Buffer &buffer() const {return buffer_;}
            Buffer &buffer() {return buffer_;}

            template <typename T>
                bool put(RoleId rid, const T &t)
                {
                    MSS_BEGIN(bool);
                    MSS(write(buffer_, rid));
                    MSS(serialize(t));
                    auto &ei = elementInfos_.back();
                    ++ei.ix;
                    MSS(ei.ix <= ei.nr);
                    MSS_END();
                }

            ReturnCode serialize(bool v) { return write(buffer_, v); }
            ReturnCode serialize(char v) { return write(buffer_, v); }
            ReturnCode serialize(unsigned char v) { return write(buffer_, v); }
            ReturnCode serialize(int v) { return write(buffer_, v); }
            ReturnCode serialize(unsigned int v) { return write(buffer_, v); }
            ReturnCode serialize(long v) { return write(buffer_, v); }
            ReturnCode serialize(unsigned long v) { return write(buffer_, v); }
#ifndef ARDUINO
            ReturnCode serialize(std::nullptr_t) { return write(buffer_, Nil_tag()); }
            ReturnCode serialize(const std::string &str) { return write(buffer_, str); }
#endif
            template <typename T>
                ReturnCode serialize(const T &t)
                {
                    MSS_BEGIN(ReturnCode);
                    MSS(t.msgpack_serialize(*this));
                    MSS_END();
                }

            //Creates a frame of one object
            template <typename T>
                bool frame(const T &t)
                {
                    MSS_BEGIN(bool);
                    clear();
                    auto c = createComposer(1);
                    MSS(c.ok());
                    c.put(t);
                    MSS(c.full());
                    MSS_END();
                }

            Composer createComposer(size_t el_cnt)
            {
                return Composer(*this, el_cnt);
            }

        private:
            Buffer buffer_;
            struct ElementInfo
            {
                size_t nr = 0;
                size_t ix = 0;
                ElementInfo() {}
                ElementInfo(size_t nr): nr(nr), ix(0) {}
            };
            typedef FixedVector<ElementInfo, MaxDepth> ElementInfos;
            ElementInfos elementInfos_;
            bool ok_ = true;
    };

} }
#include "gubg/log/end.hpp"

#endif
