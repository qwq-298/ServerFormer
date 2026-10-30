#include <cstddef>
#include <cwchar>
#include <string>

#if defined(_WIN32)
#include <climits>
#include <windows.h>
#endif

#include <utils/string_utils.h>

namespace pans{

    namespace{
        constexpr std::size_t kInvalidSize = static_cast<std::size_t>(-1);
    }

    std::string StringUtils::WStringToString(std::wstring_view text) noexcept{
        if(text.empty()){
            return {};
        }
#if defined(_WIN32)
        if(text.size() > static_cast<std::size_t>(INT_MAX)){
            return {};
        }
        const int required = ::WideCharToMultiByte(CP_UTF8, 0, text.data(),
                                                   static_cast<int>(text.size()),
                                                   nullptr, 0, nullptr, nullptr);
        if(required <= 0){
            return {};
        }
        std::string result(static_cast<std::size_t>(required), '\0');
        ::WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                              result.data(), required, nullptr, nullptr);
        return result;
#else
        // wcsrtombs 需要以 '\0' 结尾的源串，string_view 不保证结尾，先拷贝。
        const std::wstring source(text);
        std::mbstate_t state{};
        const wchar_t* src = source.c_str();
        const std::size_t required = std::wcsrtombs(nullptr, &src, 0, &state);
        if(required == kInvalidSize){
            return {};
        }
        std::string result(required, '\0');
        src = source.c_str();
        state = std::mbstate_t{};
        std::wcsrtombs(result.data(), &src, required, &state);
        return result;
#endif
    }

    std::wstring StringUtils::StringToWString(std::string_view text) noexcept{
        if(text.empty()){
            return {};
        }
#if defined(_WIN32)
        if(text.size() > static_cast<std::size_t>(INT_MAX)){
            return {};
        }
        const int required = ::MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                                   static_cast<int>(text.size()),
                                                   nullptr, 0);
        if(required <= 0){
            return {};
        }
        std::wstring result(static_cast<std::size_t>(required), L'\0');
        ::MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                              result.data(), required);
        return result;
#else
        const std::string source(text);
        std::mbstate_t state{};
        const char* src = source.c_str();
        const std::size_t required = std::mbsrtowcs(nullptr, &src, 0, &state);
        if(required == kInvalidSize){
            return {};
        }
        std::wstring result(required, L'\0');
        src = source.c_str();
        state = std::mbstate_t{};
        std::mbsrtowcs(result.data(), &src, required, &state);
        return result;
#endif
    }

}