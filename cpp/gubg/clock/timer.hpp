#ifndef gubg_clock_timer_hpp
#define gubg_clock_timer_hpp

#include <sys/time.h>
#include <iostream>

namespace gubg
{
    enum class ResetType {Auto, NoAuto};
    struct Timer
    {
        Timer(ResetType resetType):
            resetType_(resetType)
        {
            reset();
        }

        double difference()
        {
            auto diff = getCurrentTime_() - startTime_;
            if (ResetType::Auto == resetType_)
                reset();
            return diff;
        }

        void reset()
        {
            startTime_ = getCurrentTime_();
        }

        private:
        static double getCurrentTime_()
        {
            struct timeval timeVal;
            if (gettimeofday(&timeVal, 0))
            {
                std::cerr << "Could not get the time" << std::endl;
                return false;
            }
            return (double)timeVal.tv_sec + 0.000001*timeVal.tv_usec;
        }
        double startTime_;
        ResetType resetType_;
    };
}

#endif
