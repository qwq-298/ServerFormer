#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#if defined(__linux__)
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#endif

#include <utils/thread_utils.h>
#include <utils/string_utils.h>

namespace pans{

    namespace{

#if defined(_WIN32)
        struct LocalFreeDeleter{
            void operator()(wchar_t* value)const noexcept{
                if(value != nullptr){
                    (void)::LocalFree(value);
                }
            }
        };
#endif

        std::string GetNativeThreadName() noexcept{
            try{
#if defined(__linux__)
                std::array<char,16> native_name{};
                if(pthread_getname_np(pthread_self(), native_name.data(), native_name.size()) != 0){
                    return {};
                }
                return std::string(native_name.data());
#elif defined(_WIN32)
                PWSTR raw_name = nullptr;
                if(FAILED(::GetThreadDescription(::GetCurrentThread(), &raw_name))){
                    return {};
                }
                std::unique_ptr<wchar_t, LocalFreeDeleter> holder(raw_name);
                if(raw_name == nullptr){
                    return {};
                }
                return StringUtils::WStringToString(std::wstring_view(raw_name));
#else
                return {};
#endif
            }catch(...){
                return {};
            }
        }

        void SetNativeThreadName(std::string_view name) noexcept{
#if defined(__linux__)
            std::array<char,16> native_name{};
            const std::size_t name_size = std::min(name.size(), native_name.size() - 1);
            std::memcpy(native_name.data(), name.data(), name_size);
            native_name[name_size] = '\0';
            (void)pthread_setname_np(pthread_self(), native_name.data());
#elif defined(_WIN32)
            try{
                const std::wstring wide_name = StringUtils::StringToWString(name);
                (void)::SetThreadDescription(::GetCurrentThread(), wide_name.c_str());
            }catch(...){
                // 命名失败不影响日志流程
            }
#else
            (void)name;
#endif
        }

        constexpr std::string_view DEFAULT_THREAD_NAME = "UNKNOWN";

        thread_local std::string t_thread_name = GetNativeThreadName();

    } // namespace

    u64 GetThreadId() noexcept{
#if defined(__linux__)
        static thread_local const u64 THREAD_ID = static_cast<u64>(::syscall(SYS_gettid));
        return THREAD_ID;
#elif defined(_WIN32)
        return static_cast<u64>(::GetCurrentThreadId());
#else
        static thread_local const u64 THREAD_ID = static_cast<u64>(std::hash<std::thread::id>{}(std::this_thread::get_id()))
#endif
    }

    void SetThreadName(std::string name) noexcept{
        if(name.empty()){
            std::string native_name = GetNativeThreadName();
            t_thread_name = native_name.empty() ? std::string(DEFAULT_THREAD_NAME) : std::move(native_name);
            return;
        }
        t_thread_name = std::move(name);
        SetNativeThreadName(t_thread_name);
    }

    std::string_view GetThreadName() noexcept{
        if(t_thread_name.empty()){
            return DEFAULT_THREAD_NAME;
        }
        return t_thread_name;
    }

} // namespace pans
