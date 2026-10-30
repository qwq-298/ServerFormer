#include "logger/logger_impl.h"

#include <algorithm>
#include <cstdio>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <maroco.h>

#include "logger/appender_impl.h"
#include "logger/buffer.h"

namespace pans{

    constexpr std::string_view DEFAULT_LOG_PATTERN =
    "%d{%Y-%m-%d %H:%M:%S}.%u%Tthread-%t%Tfiber=%F%T[%p]%T%f:%l%T%m%n";

    Logger::Logger(std::string name)
    :m_impl(std::make_unique<Impl>(std::move(name))){}

    Logger::~Logger()=default;

    bool Logger::shouldLog(Loglevel::Level level)const noexcept{
        return m_impl->shouldLog(level);
    }
    
    void Logger::setLevel(Loglevel::Level level) noexcept{
        return m_impl->setLevel(level);
    }

    Loglevel::Level Logger::getLevel() const noexcept{
        return m_impl->getLevel();
    }

    std::string_view Logger::getName()const noexcept{
        return m_impl->getName();
    }

    void Logger::setFromatter(std::string_view pattern){
         auto fromatter = std::make_shared<const detail::Formatter>(pattern);
         m_impl->setFormatter(fromatter);
    }

    std::string Logger::getFromatterPattern()const{
         const auto it = m_impl->getFromatter();
         return it == nullptr ? std::string() : it->getPattern();
    }

    void Logger::addAppender(AppenderPtr appender){
        m_impl->addAppender(appender);
    }
    void Logger::removeAppender(const Appender& appender){
        m_impl->removeAppender(appender);
    }
    void Logger::clearAppender(){
        m_impl->clearAppender();
    }
    void Logger::flush(){
        m_impl->flush();
    }
    void Logger::sync(){
        m_impl->sync();
    }

    Logger::Impl::Impl(std::string name)
    :m_name(move(name)){
        if(m_name.empty()){
            throw std::invalid_argument("name empty");
        }
        m_formatter = std::make_shared<const detail::Formatter> (DEFAULT_LOG_PATTERN);
    }

    bool Logger::Impl::shouldLog(Loglevel::Level level)const noexcept{
        if(static_cast<u8>(level)<static_cast<u8>(getLevel())){
            return false;
        }
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if(!m_appenders.empty()){
                return true;
            }
        }
        return m_root && m_root->shouldLog(level);
    }
    
    void Logger::Impl::submit(const detail::LogRecordView& record) noexcept{
        if(static_cast<u8>(record.m_level)<static_cast<u8>(getLevel())){
            return;
        }
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            if(m_appenders.empty()){
                lock.unlock();
                if(m_root!=nullptr){
                    m_root->m_impl->submit(record);
                }
                return;
            }
            try
            {
            detail::FormattedRecordBuffer fomatted_record;
            m_formatter->format(record,fomatted_record);
            for(const AppenderPtr& appender : m_appenders){
               detail::AppenderAccess::Append(*appender,record.m_level,fomatted_record.view());
            }
            }catch(...){
               constexpr std::string_view message = "formating fail";
               std::fwrite(message.data(),1,message.size(),stderr);
            }
        }
    }

    void Logger::Impl::setLevel(Loglevel::Level level)noexcept{
        m_level.store(level,std::memory_order_release);
    }

    Loglevel::Level Logger::Impl::getLevel() const noexcept{
        return m_level.load(std::memory_order_acquire);
    }

    std::string_view Logger::Impl::getName()const noexcept{
        return m_name;
    }

    void Logger::Impl::setFormatter(std::shared_ptr<const detail::Formatter> formatter){
        //formatter != null
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_formatter = formatter;
    }

    std::shared_ptr<const detail::Formatter> Logger::Impl::getFromatter() const noexcept{
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_formatter;
    }

    void Logger::Impl::addAppender(AppenderPtr appender){
         std::unique_lock<std::shared_mutex> lock(m_mutex);
         m_appenders.push_back(std::move(appender));
    }

    void Logger::Impl::removeAppender(const Appender& appender){
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_appenders.erase(
        std::remove_if(m_appenders.begin(), m_appenders.end(),
        [&](const AppenderPtr& p){ return p.get() == &appender; }),
        m_appenders.end()
        );
    }

    void Logger::Impl::clearAppender()
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_appenders.clear();
    }

    void Logger::Impl::flush(){
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        if(m_appenders.empty()){
            lock.unlock();
            if(m_root!=nullptr){
                m_root->flush();
            }
            return;
        }
        for(const AppenderPtr& appender : m_appenders){
            appender->flush();
        }
    }

    void Logger::Impl::sync(){
         std::shared_lock<std::shared_mutex> lock(m_mutex);
        if(m_appenders.empty()){
            lock.unlock();
            if(m_root!=nullptr){
                m_root->sync();
            }
            return;
        }
        for(const AppenderPtr& appender : m_appenders){
            appender->sync();
        }   
    }

    void Logger::Impl::setRoot(LoggerPtr root)noexcept{
        m_root = root;
    }

    void detail::LoggerAccess::Submit(Logger& logger,const LogRecordView& record) noexcept{
        logger.m_impl->submit(record);
    }

    void detail::LoggerAccess::SetRoot(Logger& logger,LoggerPtr root) noexcept{
        logger.m_impl->setRoot(root);
    }


    class LoggerManager final{
        public:
        LoggerManager()
        :m_root(std::make_shared<Logger>("root")){
            m_root->addAppender(MakeStdoutAppender());
            m_loggers.emplace("root",m_root);
        }
        LoggerPtr getRoot(){
            return m_root;
        }
        LoggerPtr getLogger(std::string_view name){
            std::lock_guard<std::mutex> lock(m_mutex);
            const auto it = m_loggers.find(std::string(name));
            if(it!=m_loggers.end()){
                return it->second;
            }else{
                auto logger = std::make_shared<Logger>(std::string(name));
                detail::LoggerAccess::SetRoot(*logger,m_root);
                m_loggers.emplace(logger->getName(),logger);
                return logger;
            }
        }
        private:
        mutable std::mutex m_mutex;
        std::unordered_map<std::string,LoggerPtr> m_loggers;
        LoggerPtr m_root;
        friend std::vector<LoggerPtr> ShowEveryLogger();
    };

    LoggerManager& GetLoggerManager(){
        static LoggerManager mgr;
        return mgr;
    }

    LoggerPtr GetRootLogger(){
        return GetLoggerManager().getRoot();
    }

    LoggerPtr GetLogger(std::string_view name){
        return GetLoggerManager().getLogger(name);
    }
    
    std::vector<LoggerPtr> ShowEveryLogger(){
        std::vector<LoggerPtr> vec;
        for(auto it = GetLoggerManager().m_loggers.begin();it!=GetLoggerManager().m_loggers.end();++it){
            vec.push_back(it->second);
        }
        return vec;
    }

}