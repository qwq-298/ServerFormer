#ifndef CVAES
#define CVAES

#include <memory>
#include <string>
#include <string_view>
#include <logger/appender.h>
#include <logger/log_level.h>
#include <export.h>
#include <vector>

namespace pans{
    namespace detail{
        class LoggerAccess;
    }
    
    class PANS_API Logger final{
        public:
        class Impl;
        explicit Logger(std::string name="root");
        ~Logger();
        Logger(const Logger& )=delete;
        Logger& operator=(const Logger&) = delete;
        Logger (Logger&&) = delete;
        Logger& operator=(const Logger&&) = delete;

        bool shouldLog(Loglevel::Level level)const noexcept;
        void setLevel(Loglevel::Level level) noexcept;
        Loglevel::Level getLevel()const noexcept;
        std::string_view getName()const noexcept;
        void setFromatter(std::string_view pattern);
        std::string getFromatterPattern()const;
        void addAppender(AppenderPtr apppender);
        void removeAppender(const Appender& appender);
        void clearAppender();
        void flush();
        void sync();

        private:
        std::unique_ptr<Impl> m_impl;

        friend class detail::LoggerAccess;
    };
    using LoggerPtr = std::shared_ptr<Logger>;
    PANS_API LoggerPtr GetRootLogger();
    PANS_API LoggerPtr GetLogger(std::string_view name);
    PANS_API std::vector<pans::LoggerPtr> ShowEveryLogger();
}
#endif