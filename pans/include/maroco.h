#ifndef PANS_ASSERT_INCLUDE
#define PANS_ASSERT_INCLUDE

#include <iostream>
#include <cassert>
#include <cstdint>

#define PANS_ASSERT(x)\
if(!(x))[[unlikely]] {\
    std::cerr<<__FILE__ <<": "<< __LINE__ <<" ASSERT "<< #x <<" stacktrace: "<<std::endl;\
    assert(x);\
}

#define PANS_ASSERT_info(x,info)\
if(!(x))[[unlikely]]{\
    std::cerr<<__FILE__ <<": "<< __LINE__ <<" ASSERT "<< #x << "fail:"<< info <<" stacktrace: "<<std::endl;\
    assert(x);\
}

#define ASSERT_RETVAL(x,val)\
do{\
    if(x)[[likely]] break;\
    PANS_ASSERT(x);\
    return val;\
}while(0)

#define ASSERT_RETVAL_info(x,val,info)\
do{\
    if(x)[[likely]] break;\
    PANS_ASSERT_info(x,info);\
    return val;\
}while(0)

#define ASSERT_NONE(x)\
do{\
    if(x)[[likely]] break;\
    PANS_ASSERT(x);\
    return;\
}while(0)

#define ASSERT_NONE_info(x,info)\
do{\
    if(x)[[likely]] break;\
    PANS_ASSERT_info(x,info);\
    return;\
}while(0)

#define ASSERT_NOEFFECT(x)\
do{\
    if(x)[[likely]] break;\
    PANS_ASSERT(x);\
}while(0)

#define ASSERT_NOEFFECT_info(x,info)\
do{\
    if(x)[[likely]] break;\
    PANS_ASSERT_info(x,info);\
}while(0)

#define ASSERT_CONTINUE(x)\
if(!(x))[[unlikely]]{\
    PANS_ASSERT(x);\
    continue;\
}

#define ASSERT_CONTINUE_info(x,info)\
if(!(x))[[unlikely]]{\
    PANS_ASSERT_info(x,info);\
    continue;\
}

#define ASSERT_BREAK(x)\
if(!(x))[[unlikely]]{\
    PANS_ASSERT(x);\
    break;\
}

#define ASSERT_BREAK_info(x,info)\
if(!(x))[[unlikely]]{\
    PANS_ASSERT_info(x,info);\
    break;\
}

#define INVALID64 (~0ULL)
#define INVALID32 0xFFFFFFFF
#define INVALID16 0xFFFF
#define INVALID8  0xFF

#define MAX_U8 0xFF
#define MAX_U16 0xFFFF
#define MAX_U32 0xFFFFFFFF
#define MAX_U64 (~0ULL)

using u8 = uint8_t;
using s8 = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;

#endif