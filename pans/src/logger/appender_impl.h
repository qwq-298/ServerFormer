#ifndef GFHSL
#define GFHSL

#include <atomic>
#include <cstdio>
//#include <memory>
#include <mutex>
//#include <string>
#include <string_view>

#include <logger/appender.h>
#include <logger/log_level.h>

namespace pans{
class Appender::Impl
{
    private:
    std::atomic<Loglevel::Level> m_level{Loglevel::Level::LOG_LEVEL_DEBUG};
    public:
    virtual ~Impl()=default;
    void append(Loglevel::Level level,std::string_view formatted_record) noexcept;
    void setLevel(Loglevel::Level level) noexcept;
    Loglevel::Level getLevel() const noexcept;
    void flush();
    void sync();
    
    protected:
    virtual void writeUnlocked(std::string_view formatted_record) noexcept =0;
    virtual void flushUnlocked() noexcept=0;
    virtual void syncUnlocked() noexcept=0;
    std::mutex mutex;
};

namespace detail{
    class AppenderAccess final{
        public:
        static AppenderPtr MakeStdoutAppender();
        static AppenderPtr MakeFileAppender(std::string file_name);
        static void Append(const Appender& appender,Loglevel::Level level,std::string_view format);
    };
}

}

#endif