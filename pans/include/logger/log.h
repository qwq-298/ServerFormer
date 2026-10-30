#ifndef ZSEFDD
#define ZSEFDD

#include <cstdint>
#include <cstddef>
#include <iosfwd>
#include <string_view>

#include <logger/appender.h>
#include <logger/log_level.h>
#include <logger/logger.h>
#include <maroco.h>

namespace pans::detail{

    class PANS_API LogLine final{
        struct Impl;

        public:
        LogLine(Logger& logger,Loglevel::Level level,u32 line, std::string_view file_name);
        ~LogLine() noexcept;
        LogLine(const LogLine&) = delete;
        LogLine& operator=(const LogLine&) = delete;

        std::ostream& stream() noexcept;
        
        private:
        Impl& GetImpl() noexcept;

        static constexpr std::size_t LOG_LINE_IMPL_SIZE = 1024;
        alignas(std::max_align_t) std::byte m_implStorage[LOG_LINE_IMPL_SIZE];

    };

    PANS_API void LogPrintf(Logger& Logger,Loglevel::Level level,u32 line,std::string_view file_name,const char*format,...);

    #define PANS_LOG_LEVEL(logger,level)\
    if(auto pans_log_logger =(logger);!pans_log_logger){}\
    else if(const auto pans_log_level = (level);!pans_log_logger->shouldLog(pans_log_level)){}\
    else pans::detail::LogLine(*pans_log_logger,pans_log_level,__LINE__,__FILE__).stream()
    
    #define PANS_LOG_DEBUG(logger) PANS_LOG_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_DEBUG)
    #define PANS_LOG_INFO(logger)  PANS_LOG_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_INFO)
    #define PANS_LOG_WARN(logger)  PANS_LOG_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_WARN)
    #define PANS_LOG_ERROR(logger) PANS_LOG_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_ERROR)
    #define PANS_LOG_FATAL(logger) PANS_LOG_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_FATAL)

    #define PANS_LOG_FMT_LEVEL(logger,level,format,...)\
    if(auto pans_log_logger=(logger);!pans_log_logger){}\
    else if(const auto pans_log_level = (level);!pans_log_logger->shouldLog(pans_log_level)){}\
    else pans::detail::LogPrintf(*pans_log_logger,pans_log_level,__LINE__,__FILE__,(format)__VA_OPT__(,)__VA_ARGS__)

    #define PANS_LOG_FMT_DEBUG(logger,format,...) \
    PANS_LOG_FMT_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_DEBUG, format __VA_OPT__(,) __VA_ARGS__)

    #define PANS_LOG_FMT_INFO(logger,format,...) \
    PANS_LOG_FMT_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_INFO, format __VA_OPT__(,) __VA_ARGS__)

    #define PANS_LOG_FMT_WARN(logger,format,...) \
    PANS_LOG_FMT_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_WARN, format __VA_OPT__(,) __VA_ARGS__)

    #define PANS_LOG_FMT_ERROR(logger,format,...) \
    PANS_LOG_FMT_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_ERROR, format __VA_OPT__(,) __VA_ARGS__)

    #define PANS_LOG_FMT_FATAL(logger,format,...) \
    PANS_LOG_FMT_LEVEL((logger), pans::Loglevel::Level::LOG_LEVEL_FATAL, format __VA_OPT__(,) __VA_ARGS__)

    #define PANS_SET_ROOT pans::
    #define PANS_LOG_ROOT pans::GetRootLogger()
    #define PANS_LOG_NAME(name) pans::GetLogger((name))

    #define LOG_DEBUG PANS_LOG_DEBUG(PANS_LOG_ROOT)
    #define LOG_INFO  PANS_LOG_INFO(PANS_LOG_ROOT)
    #define LOG_WARN  PANS_LOG_WARN(PANS_LOG_ROOT)
    #define LOG_ERROR PANS_LOG_ERROR(PANS_LOG_ROOT)
    #define LOG_FATAL PANS_LOG_FATAL(PANS_LOG_ROOT)
}
#endif