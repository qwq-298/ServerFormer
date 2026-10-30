#ifndef OOOFGJH
#define OOOFGJH

#include <memory>
#include <string>

#include <export.h>
#include <logger/log_level.h>

namespace pans{

    namespace detail{
        class AppenderAccess;
    }

    class PANS_API Appender final{
        public:
        class Impl;
        ~Appender() = default;
        Appender (const Appender&) = delete;
        Appender& operator= (const Appender&) = delete;
        
        void setLevel(Loglevel::Level level) noexcept;

        Loglevel::Level getLevel() const noexcept;

        void flush();

        void sync();


        private:
        explicit Appender(std::unique_ptr<Impl> impl) noexcept;
        std::unique_ptr<Impl> m_impl;
        friend class detail::AppenderAccess;
    };

    using AppenderPtr = std::shared_ptr<Appender>;

    PANS_API AppenderPtr MakeStdoutAppender();
    PANS_API AppenderPtr MakeFileAppender(std::string file_name);

}
#endif