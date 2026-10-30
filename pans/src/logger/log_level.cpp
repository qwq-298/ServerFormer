#include "logger/log_level.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>

namespace pans{
    std::string_view Loglevel::ToString(Level level) noexcept{
        switch (level)
        {
        #define XX(name) case Level::LOG_LEVEL_##name : return #name;
                XX(DEBUG)
                XX(INFO)
                XX(WARN)
                XX(ERROR)
                XX(FATAL)
                XX(OFF)
        #undef XX        
        }
        return "UNKNOWN";
    }
    Loglevel::Level Loglevel::FromString(std::string_view value) noexcept{
        std::array<char,6>normalized{};
        std::transform(value.begin(),value.end(),normalized.begin(),
    [](unsigned char character){
        return static_cast<char>(std::toupper(character));
    }
    );//转大写
    const std::string_view upper_value(normalized.data(),value.size());
    #define XX(name) if(upper_value==#name) return Loglevel::Level::LOG_LEVEL_##name;
    XX(DEBUG)
    XX(INFO)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
    XX(OFF)
    #undef XX
    return Loglevel::Level::LOG_LEVEL_OFF;
    }
}
