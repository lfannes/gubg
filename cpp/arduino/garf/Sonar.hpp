#ifndef HEADER_garf_Sonar_hpp_ALREADY_INCLUDED
#define HEADER_garf_Sonar_hpp_ALREADY_INCLUDED

namespace garf
{
    class Sonar
    {
        private:
            enum State {Init, Idle, };
        public:
            Sonar():
                state_(Init){}
            void process(int elapse)
            {
                if (state_ == Init)
                    changeState_(Idle);
            }
        private:
            void changeState_(const State newState)
            {
            }
            State state_;
    };
}

#endif
