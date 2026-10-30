#ifndef PANS_INCLUDE_PANS_LOGLEVEL
#define PANS_INCLUDE_PANS_LOGLEVEL
#include <cstdint>
#include <string_view>

#include <export.h>

namespace pans{

class PANS_API Loglevel final
{
    public:
    enum class Level : std::uint8_t{
        LOG_LEVEL_DEBUG = 1,
        LOG_LEVEL_INFO =2,
        LOG_LEVEL_WARN =3,
        LOG_LEVEL_ERROR =4,
        LOG_LEVEL_FATAL =5,
        LOG_LEVEL_OFF =6,
    };
    //不要忽略返回值
    [[nodiscard]] static std::string_view ToString(Level level) noexcept;
    [[nodiscard]] static Loglevel::Level FromString(std::string_view value) noexcept;
};


}

#endif