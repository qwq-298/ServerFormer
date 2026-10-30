#ifndef GHFSDA
#define GHFSDA

#include <chrono>
#include <maroco.h>
#include <export.h>

namespace pans{

    PANS_API std::chrono::steady_clock::duration GetElapsedTime() noexcept;
    PANS_API u64 GetFiberId() noexcept;

}
#endif