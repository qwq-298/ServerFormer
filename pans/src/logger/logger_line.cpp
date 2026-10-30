#include <array>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <new>
#include <string_view>
#include <vector>
#include <ostream>

#include <logger/log.h>
#include <utils/system_utils.h>
#include <utils/thread_utils.h>

#include "logger/buffer.h"
#include "logger/buffer_config.h"
#include "logger/logger_impl.h"
#include "logger/log_record.h"

namespace pans::detail{

    struct LogLine::Impl final{
        Impl(Logger& logger,Loglevel::Level level,u32 line,std::string_view file_name)
        :m_logger(logger),
        m_level(level),
        m_line(line),
        m_filename(file_name),
        m_timestamp(std::chrono::system_clock::now()),
        m_elapsed(GetElapsedTime()),
        m_threadId(GetThreadId()),
        m_fiberId(GetFiberId()),
        m_threadName(GetThreadName()),
        m_streamBuffer(m_inlinebuffer),
        m_stream(&m_streamBuffer){}


        LogRecordView getRecord() const noexcept{
            return{
                m_level,
                m_logger.getName(),
                m_inlinebuffer.view(),
                m_timestamp,
                m_elapsed,
                m_threadId,
                m_fiberId,
                m_filename,
                m_threadName,
                m_line
            };
        }

        
        Logger& m_logger;
        Loglevel::Level m_level;
        u32 m_line =0;
        std::string_view m_filename;
        std::chrono::system_clock::time_point m_timestamp;
        std::chrono::steady_clock::duration m_elapsed;
        u64 m_threadId =0;
        u64 m_fiberId =0;
        std::string_view m_threadName;
        Inline_buffer<LOG_MESSAGE_INLINE_SIZE> m_inlinebuffer;
        SmallStringBuffer<LOG_MESSAGE_INLINE_SIZE> m_streamBuffer;
        std::ostream m_stream;


    };

    LogLine::Impl& LogLine::GetImpl() noexcept{
        return *std::launder(reinterpret_cast<Impl*>(m_implStorage));
    }

    LogLine::LogLine(Logger& logger,Loglevel::Level level,u32 line, std::string_view file_name){
        static_assert(sizeof(Impl)<=LOG_LINE_IMPL_SIZE,"LogLine::Impl too large");
        static_assert(alignof(Impl)<=alignof(std::max_align_t),"LogLine::Impl over-aligned");
        std::construct_at(reinterpret_cast<Impl*>(m_implStorage),logger,level,line,file_name);
    }

    LogLine::~LogLine() noexcept{
        Impl& impl = GetImpl();
        LoggerAccess::Submit(impl.m_logger,impl.getRecord());
        std::destroy_at(std::addressof(impl));
    }

    std::ostream& LogLine::stream() noexcept{
        return GetImpl().m_stream;
    }

    void LogPrintf(Logger& logger,Loglevel::Level level,u32 line,std::string_view file_name,const char* format,...){
        LogLine log_line(logger,level,line,file_name);
        if(format==nullptr){
           log_line.stream() << "<null-format>";
           return ;
        }
        std::array<char,PRINT_FORMAT_INLINE_SIZE> inline_buffer;
        va_list arguements;
        va_list arguements_copy;
        va_start(arguements,format);
        va_copy(arguements_copy,arguements);
        const int required_size = std::vsnprintf(inline_buffer.data(),inline_buffer.size(),format,arguements);
        va_end(arguements);
        if(required_size<0){
            va_end(arguements_copy);
            log_line.stream()<<"<format-error>";
            return ;
        }
        const std::size_t message_size = static_cast<std::size_t>(required_size);
        if(message_size < inline_buffer.size()){
            va_end(arguements_copy);
            log_line.stream().write(inline_buffer.data(),static_cast<std::streamsize>(message_size));
            return;
        }
        std::vector<char> overflow_buffer(message_size+1);
        const int second_result = std::vsnprintf(overflow_buffer.data(),overflow_buffer.size(),format,arguements_copy);
        va_end(arguements_copy);
        if(second_result<0){
            log_line.stream() << "<format-error>";
            return ;
        }
        log_line.stream().write(overflow_buffer.data(),static_cast<std::streamsize>(message_size));
    }
}