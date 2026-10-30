#ifndef PLLKI
#define PLLKI

#include <atomic>
#include <logger/logger.h>
#include <vector>
#include <string>
#include <shared_mutex>
#include <logger/log_level.h>
#include "log_record.h"
#include "fomatter.h"

namespace pans{
    class Logger::Impl{
        public: 
        explicit Impl(std::string name);
        bool shouldLog(Loglevel::Level level)const noexcept;
        void submit(const detail::LogRecordView& record) noexcept;
        void setLevel(Loglevel::Level level) noexcept;
        Loglevel::Level getLevel()const noexcept;
        std::string_view getName()const noexcept;
        void setFormatter(std::shared_ptr<const detail::Formatter> fromatter);
        std::shared_ptr<const detail::Formatter> getFromatter()const noexcept;
        void addAppender(AppenderPtr appender);
        void removeAppender(const Appender& appender);
        void clearAppender();
        void flush();
        void sync();
        void setRoot(LoggerPtr root) noexcept;
        private:
        std::string m_name;
        std::atomic<Loglevel::Level> m_level{Loglevel::Level::LOG_LEVEL_DEBUG};
        mutable std::shared_mutex m_mutex;
        std::shared_ptr<const detail::Formatter> m_formatter;
        std::vector<AppenderPtr> m_appenders;
        LoggerPtr m_root;
    };
    namespace detail{
        class LoggerAccess final{
           public:
           static void Submit(Logger& logger,const LogRecordView& record) noexcept;
           static void SetRoot(Logger& logger,LoggerPtr root) noexcept;
        };
    }
}

#endif