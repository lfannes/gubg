#ifndef aoeuaou
#define aoeuaou

#include "garf/Metronome.hpp"
#include "gubg/msgpack/Serializer.hpp"
#include "gubg/OnlyOnce.hpp"

namespace garf
{
    namespace top
    {
        struct Info
        {
            unsigned long nrLoops;
            unsigned long maxElapse;

            Info():nrLoops(0), maxElapse(0){}

            enum {nrLoops_, maxElapse_, nr_};
            template <typename S>
                gubg::msgpack::ReturnCode msgpack_serialize(S &s) const
                {
                    MSS_BEGIN(gubg::msgpack::ReturnCode);
                    s.writeIdAndAttrCnt(0, nr_);
                    s.template writeAttribute<long>(nrLoops_, nrLoops);
                    s.template writeAttribute<long>(maxElapse_, maxElapse);
                    MSS_END();
                }
        };

        template <unsigned long Period>
            class Top: public Metronome_crtp<Top<Period>, Period>
        {
            private:
                typedef Metronome_crtp<Top<Period>, Period> Metronome;

            public:
                template <typename Elapse>
                void process(Elapse elapse)
                {
                    if (clearInfo_())
                        //We don't process this elapse data, it is probably too large due to sending our data
                        info_ = Info();
                    else
                    {
                        if (elapse > info_.maxElapse)
                            info_.maxElapse = elapse;
                        ++info_.nrLoops;
                    }
                    Metronome::process(elapse);
                }
                void metronome_tick()
                {
                    serializer_.clear();
                    serializer_.serialize(info_);
                    clearInfo_.reset();
                    Serial.write(serializer_.buffer().data(), serializer_.buffer().size());
                }
            private:
                gubg::OnlyOnce clearInfo_;
                Info info_;
                gubg::msgpack::Serializer<gubg::FixedVector<uint8_t, 20>, 2> serializer_;
        };
    }
}

#endif
