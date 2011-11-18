#ifndef gubg_mss_mss_hpp
#define gubg_mss_mss_hpp

//Main success scenario: enables easy working with return codes
//A general scope (e.g., a function body) looks like this:
//
//MSS_BEGIN(ReturnCodeT);
// => Initializes a return value "MSS_RC_VAR" of type ReturnCodeT to OK
// => Linear sequence of statements that are checked for being part of the main success scenario
//    As soon as one fails, we return from the function with the failing code
//    If nothing fails, MSS_END() will return from the function with a "OK" value
//MSS(some_function_that_returns_ReturnCodeT());
//MSS_T(some_function_that_returns_WhateverT(), code_upon_failure); => MSS_RC_VAR is set to code_upon_failure when some_function_that_returns_WhateverT() fails
//...
// => You can return or continue here
//
//MSS_END();
// => Ends the MSS block and returns from the function

#include "gubg/mss/info.hpp"

namespace gubg
{
    namespace mss
    {
        template <typename T>
            struct ReturnCodeWrapper
            {
                ReturnCodeWrapper():v_(T::OK){}
                T get(){return v_;}
                bool set(T v)
                {
                    return T::OK == (v_ = v);
                }
                bool set(bool b, T v = T::Error)
                {
                    return T::OK == (v_ = (b ? T::OK : v));
                }
                Level level() const {return getInfo<T>(v_).level;}
                std::string toString() const
                {
                    std::ostringstream oss;
                    auto info = getInfo(v_);
                    oss << info.type << ":" << info.code;
                    return oss.str();
                }
                T v_;
            };
        template <>
            struct ReturnCodeWrapper<void>
            {
                ReturnCodeWrapper():l_(Level::OK){}
                void get(){}
                template <typename OT>
                    bool set(OT v)
                    {
                        l_ = getInfo<OT>(v).level;
                        return OT::OK == v;
                    }
                bool set(bool b)
                {
                    v_ = (b ? "true" : "false");
                    return b;
                }
                Level level() const {return l_;}
                std::string toString() const {return v_;}
                std::string v_;
                Level l_;
            };
        template <typename ReturnCodeWrapper>
        struct SuccessChecker
        {
            public:
                SuccessChecker(const gubg::Location &location, ReturnCodeWrapper &rcw):
                    success_(false),
                    location_(location),
                    returnCodeWrapper_(rcw){}
                ~SuccessChecker()
                {
                    if (!success_)
                        std::cout << "MSS UNSUCCESSFUL::" << location_ << "::" << msg_ << "::" << returnCodeWrapper_.toString() << std::endl;
                }
                void setMessage(const std::string &msg){msg_ = msg;}
                void indicateSuccess(){success_ = true;}
            private:
                bool success_;
                std::string msg_;
                std::string returnCodeInfo_;
                gubg::Location location_;
                ReturnCodeWrapper &returnCodeWrapper_;
        };
    }
}

#define MSS_RC_VAR rc

#define MSS_BEGIN(type)      typedef type gubg_return_code_type; \
                             typedef gubg::mss::ReturnCodeWrapper<gubg_return_code_type> gubg_return_code_wrapper_type; \
                             gubg_return_code_wrapper_type MSS_RC_VAR
#define MSS_BEGIN_J()        gubg::mss::ReturnCodeWrapper<void> MSS_RC_VAR;

#define MSS_END()            return MSS_RC_VAR.get()
#define MSS_FAIL()           gubg_mss_fail_label:

#define MSS_BEGIN_(type, msg) \
    MSS_BEGIN(type); \
    { \
        gubg::mss::SuccessChecker<gubg_return_code_wrapper_type> l_gubg_success_checker(GUBG_HERE(), MSS_RC_VAR); \
        { \
            std::ostringstream l_gubg_success_checker_m; l_gubg_success_checker_m << msg; \
            l_gubg_success_checker.setMessage(l_gubg_success_checker_m.str()); \
        } \

#define MSS_END_() \
        l_gubg_success_checker.indicateSuccess(); \
        MSS_END(); \
    }

#define L_MSS_LOG(l, rc, msg) \
{ \
    auto level = (gubg::mss::Level::Unknown == gubg::mss::Level::l ? rc.level() : gubg::mss::Level::l); \
    std::cout << GUBG_HERE() << " " << level << "::" << rc.toString() << msg << std::endl; \
}

//Direct handling, v should be of the same type as specified in MSS_BEGIN(type)
#define MSS_(level, v, msg) \
    do { \
        if (!MSS_RC_VAR.set(v)) \
        { \
            L_MSS_LOG(level, MSS_RC_VAR, msg); \
            MSS_END(); \
        } \
    } while (false)
#define MSS(v) MSS_(Unknown, v, "")

//Direct return of a hardcoded value, c should be a value of the enum type specified in MSS_BEGIN(type)
#define MSS_L_(level, c, msg) MSS_(level, gubg_return_code_type::c, msg)
#define MSS_L(c) MSS_L_(Unknown, c, "")

//Transformation of _any_ type v in a hardcoded value nc. nc should be a value of the enum type specified in MSS_BEGIN(type)
#define MSS_T_(level, v, nc, msg) \
    do { \
        if (!MSS_RC_VAR.set(v, gubg_return_code_type::nc)) \
        { \
            L_MSS_LOG(level, MSS_RC_VAR, msg); \
            MSS_END(); \
        } \
    } while (false)
#define MSS_T(v, nc) MSS_T_(Unknown, v, nc, "")

//Allows integration with functions of incompatible return types, this is goto-based
#define MSS_J_(level, v, msg) \
    do { \
        if (!MSS_RC_VAR.set(v)) \
        { \
            L_MSS_LOG(level, MSS_RC_VAR, msg); \
            goto gubg_mss_fail_label; \
        } \
    } while (false)
#define MSS_J(v) MSS_J_(Unknown, v, "")

#define MSS_DEFAULT_CODES OK, InternalError, IllegalArgument, NotImplemented
#define MSS_CODE_BEGIN(type) \
    namespace { \
        typedef type l_gubg_mss_type; \
        const char *l_gubg_mss_type_as_string = #type; \
        MSS_CODE_(OK, OK); \
        MSS_CODE_(Fatal, NotImplemented); \
        MSS_CODE_(Critical, InternalError); \
        MSS_CODE_(Error, IllegalArgument);
#define MSS_CODE_(level, code) gubg::mss::InfoSetter<l_gubg_mss_type> l_gubg_mss_InfoSetter_ ## _ ## code(l_gubg_mss_type::code, gubg::mss::Level::level, l_gubg_mss_type_as_string, #code)
#define MSS_CODE(code) MSS_CODE_(Error, code)
#define MSS_CODE_END() }

#endif
