#include "gubg/msgpack/Factory.hpp"
#include "gubg/msgpack/Serializer.hpp"
#include "gubg/Testing.hpp"
#include "gubg/msgpack/test/Helper.hpp"
#include <iostream>

struct Ids
{
};

#define GUBG_MODULE_ "Work"
#include "gubg/log/begin.hpp"
struct Work
{
	enum {nonce_rid, msg_rid, nr_};
    std::string nonce;
    std::string msg;

    void stream(std::ostream &os) const
    {
        os << STREAM(nonce, msg) << std::endl;
    }
    template <typename Serializer>
        bool msgpack_serialize(Serializer &s)
        {
			MSS_BEGIN(bool);
			auto c = s.createComposer(nr_);
			MSS(c.ok());
			MSS(c.put(nonce_rid, nonce));
			MSS(c.put(msg_rid, msg));
			MSS(c.full());
			MSS_END();
        }

	typedef gubg::msgpack::RoleId RoleId;
	template <typename Wrapper>
		void msgpack_createObject(Wrapper &obj, RoleId rid)
		{
			S();L("Creating object " << STREAM(rid));
		}
    void msgpack_set(gubg::msgpack::RoleId rid, gubg::msgpack::Nil_tag)
    {
        S();L("Setting " << STREAM(rid) << " to nil");
        switch (rid)
        {
            case nonce_rid: nonce.clear(); break;
            case msg_rid: msg.clear(); break;
        }
    }
    void msgpack_set(gubg::msgpack::RoleId rid, const std::string &str)
    {
        S();L("Setting " << STREAM(rid) << " to str");
        switch (rid)
        {
            case nonce_rid: nonce.assign(&str[0], str.size()); break;
            case msg_rid: msg.assign(&str[0], str.size()); break;
        }
    }
    void msgpack_set(gubg::msgpack::RoleId rid, long v)
    {
        S();L("Setting " << STREAM(rid) << " to " << v);
    }
	void msgpack_createdObject(RoleId rid)
	{
		S();L("Created object " << STREAM(rid));
	}
};
Work work;
#include "gubg/log/end.hpp"

enum class ReturnCode {MSS_DEFAULT_CODES,};

#define GUBG_MODULE_ "test_Factory"
#include "gubg/log/begin.hpp"
class Factory: public gubg::msgpack::Factory_crtp<Factory, std::string, 15>
{
    public:
		enum {work_rid = 123};
        void msgpack_createObject(gubg::msgpack::Wrapper<std::string> &obj, RoleId rid)
        {
            SS(rid);
            switch (rid)
            {
                case work_rid: obj = wrap(work); break;
            }
        }
        void msgpack_createdObject(RoleId rid)
        {
            SS(rid);
        }
        void msgpack_set(RoleId rid, gubg::msgpack::Nil_tag) {S();L(rid << " nil");}
        void msgpack_set(RoleId rid, const std::string &str) {S();L(STREAM(rid, str));}
        void msgpack_set(RoleId rid, long l) {S();L(STREAM(rid, l));}
};
#include "gubg/log/end.hpp"

namespace data
{
    using namespace helper;
    auto msg_0 = str_({0x00});
    auto msg_nil = str_({0xc0});
    auto msg_ut = str_({0x81, 0x7b, 0x82, 0x00, 0xa3, 0x61, 0x62, 0x63, 0x01, 0xc0});
}

#define GUBG_MODULE_ "test"
#include "gubg/log/begin.hpp"
int main()
{
    TEST_TAG(main);
    Factory f;
    //    f.process(data::msg_0);
    //    f.process(data::msg_nil);
    f.process(data::msg_ut);
    work.stream(std::cout);

    std::string buffer;
    gubg::msgpack::Serializer<std::string, Ids, 10> serializer;
    serializer.serialize(buffer);
    std::cout << gubg::testing::toHex(buffer) << std::endl;
    return 0;
}
#include "gubg/log/end.hpp"
