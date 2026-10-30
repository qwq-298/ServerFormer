#ifndef YUIOP
#define YUIOP

#include <string>
#include <string_view>

#include <export.h>
#include <maroco.h>

namespace pans{
    PANS_API u64 GetThreadId() noexcept;

    PANS_API void SetThreadName(std::string name) noexcept;

    PANS_API std::string_view GetThreadName() noexcept;
}

#endif