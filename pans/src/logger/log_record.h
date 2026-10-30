#ifndef UUU
#define UUU

#include<chrono>
#include<cstdint>
//#include<source_location>
#include<string_view>
#include<logger/log_level.h>
#include<maroco.h>

namespace pans::detail{
     struct LogRecordView{
          Loglevel::Level m_level = Loglevel::Level::LOG_LEVEL_DEBUG;
          std::string_view m_loggername ="";
          std::string_view m_message="";
          std::chrono::system_clock::time_point m_timestamp;
          std::chrono::steady_clock::duration m_duration;
          u64 m_threadid =0;
          u64 m_fiberid =0;
          std::string_view m_file_name="";
          //std::string_view m_function_name="";
          std::string_view m_thread_name="";
          u32 m_line=0;
          //std::source_location m_location; //c20
     };
}
#endif