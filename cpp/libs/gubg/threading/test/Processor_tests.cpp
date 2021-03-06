#include "gubg/threading/Processor.hpp"
#include "gubg/Testing.hpp"
#include <chrono>
using namespace std::chrono;

#define GUBG_MODULE "test"
#include "gubg/log/begin.hpp"
namespace 
{
    class Job
    {
        public:
            typedef std::shared_ptr<Job> Ptr;
            template <typename Duration>
                static Ptr create(Duration duration){return Ptr(new Job(duration));}
            template <typename Duration>
                Job(Duration duration):duration_(duration){}

            void execute()
            {
                MSS_BEGIN(void, execute);
                L("Starting job " << duration_.count());
                std::this_thread::sleep_for(duration_);
                L("         job " << duration_.count() << " is done");
                MSS_END();
            }

        private:
            milliseconds duration_;
    };
    typedef gubg::threading::Processor<Job> Processor;
}

int main()
{
    TEST_TAG(Processor);
    Processor processor(5);
    for (auto i = 0; i < 20; ++i)
        processor << Job::create(milliseconds(10*(i+1)));
    TEST_KO(processor.stop());
    TEST_OK(processor.start());
    TEST_KO(processor.start());

    std::this_thread::sleep_for(milliseconds(100));

    TEST_OK(processor.stop());
    TEST_KO(processor.stop());
    return 0;
}
#include "gubg/log/end.hpp"
