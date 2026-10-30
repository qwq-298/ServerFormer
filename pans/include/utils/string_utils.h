#ifndef SFDGH
#define SFDGH

#include <cstring>
#include <cstdarg>
#include <string>
#include <string_view>

#include <export.h>

namespace pans{
    class PANS_API StringUtils final{
        public:
        StringUtils()=default;
        static std::string WStringToString(std::wstring_view text)noexcept;
        static std::wstring StringToWString(std::string_view text)noexcept;
    };
}

#endif