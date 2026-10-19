// maroco_test.cpp
//
// Checks pans/include/maroco.h at compile time (static_assert) and at
// runtime (explicit value comparisons plus the PANS assertion macros).

#include "maroco.h"

#include <type_traits>

// ---------------------------------------------------------------------------
// Compile-time checks
// ---------------------------------------------------------------------------

// The fixed-width aliases resolve to the expected standard integer types.
static_assert(std::is_same_v<u8, std::uint8_t>, "u8 must be uint8_t");
static_assert(std::is_same_v<s8, std::int8_t>, "s8 must be int8_t");
static_assert(std::is_same_v<u16, std::uint16_t>, "u16 must be uint16_t");
static_assert(std::is_same_v<s16, std::int16_t>, "s16 must be int16_t");
static_assert(std::is_same_v<u32, std::uint32_t>, "u32 must be uint32_t");
static_assert(std::is_same_v<s32, std::int32_t>, "s32 must be int32_t");
static_assert(std::is_same_v<u64, std::uint64_t>, "u64 must be uint64_t");
static_assert(std::is_same_v<s64, std::int64_t>, "s64 must be int64_t");

// Signage of each alias.
static_assert(std::is_unsigned_v<u8>, "u8 must be unsigned");
static_assert(std::is_unsigned_v<u16>, "u16 must be unsigned");
static_assert(std::is_unsigned_v<u32>, "u32 must be unsigned");
static_assert(std::is_unsigned_v<u64>, "u64 must be unsigned");
static_assert(std::is_signed_v<s8>, "s8 must be signed");
static_assert(std::is_signed_v<s16>, "s16 must be signed");
static_assert(std::is_signed_v<s32>, "s32 must be signed");
static_assert(std::is_signed_v<s64>, "s64 must be signed");

// Widths in bytes.
static_assert(sizeof(u8) == 1, "u8 must be 1 byte");
static_assert(sizeof(s16) == 2, "s16 must be 2 bytes");
static_assert(sizeof(u32) == 4, "u32 must be 4 bytes");
static_assert(sizeof(u64) == 8, "u64 must be 8 bytes");

// Limit and sentinel constants.
static_assert(MAX_U8 == 0xFFu, "MAX_U8");
static_assert(MAX_U16 == 0xFFFFu, "MAX_U16");
static_assert(MAX_U32 == 0xFFFFFFFFu, "MAX_U32");
static_assert(MAX_U64 == ~0ULL, "MAX_U64");
static_assert(INVALID8 == 0xFFu, "INVALID8");
static_assert(INVALID16 == 0xFFFFu, "INVALID16");
static_assert(INVALID32 == 0xFFFFFFFFu, "INVALID32");
static_assert(INVALID64 == ~0ULL, "INVALID64");

// ---------------------------------------------------------------------------
// Runtime checks
// ---------------------------------------------------------------------------

// Exercises ASSERT_RETVAL: returns 7 when the condition holds.
static int retval_ok() {
    ASSERT_RETVAL(true, 1);
    return 7;
}

// Exercises ASSERT_NONE / ASSERT_NONE_info.
static void none_ok() {
    ASSERT_NONE(true);
    ASSERT_NONE_info(true, "none info");
}

// Exercises ASSERT_NOEFFECT / ASSERT_NOEFFECT_info.
static void noeffect_ok() {
    ASSERT_NOEFFECT(true);
    ASSERT_NOEFFECT_info(true, "noeffect info");
}

// Exercises ASSERT_CONTINUE / ASSERT_CONTINUE_info inside a loop. With a true
// condition the macro must not disturb control flow, so all iterations run.
// (The continue branch only fires on failure, which aborts via assert in
// Debug builds.)
static int continue_count() {
    int count = 0;
    for (int i = 0; i < 5; ++i) {
        ASSERT_CONTINUE(i >= 0);
        ASSERT_CONTINUE_info(i >= 0, "continue info");
        ++count;
    }
    return count;  // all 5 iterations complete
}

// Exercises ASSERT_BREAK / ASSERT_BREAK_info inside a loop. With a true
// condition the macro must not disturb control flow.
static int break_count() {
    int count = 0;
    for (int i = 0; i < 5; ++i) {
        ASSERT_BREAK(i >= 0);
        ASSERT_BREAK_info(i >= 0, "break info");
        ++count;
    }
    return count;  // all 5 iterations complete
}

int main() {
    #ifdef PANS_DEBUG
    std::cout<<"debug";
    #endif
    #ifdef NDEBUG
    std::cout<<"Release";
    #endif
    PANS_ASSERT(false);
    PANS_ASSERT_info(false, "runtime info");

    // Sentinel constants hold their advertised values at runtime.
    if (MAX_U8 != 0xFFu) return 1;
    if (MAX_U16 != 0xFFFFu) return 2;
    if (MAX_U32 != 0xFFFFFFFFu) return 3;
    if (MAX_U64 != ~0ULL) return 4;
    if (INVALID8 != 0xFFu) return 5;
    if (INVALID16 != 0xFFFFu) return 6;
    if (INVALID32 != 0xFFFFFFFFu) return 7;
    if (INVALID64 != ~0ULL) return 8;

    // Assertion-macro family behaviour.
    if (retval_ok() != 7) return 9;
    none_ok();
    noeffect_ok();
    if (continue_count() != 5) return 10;
    if (break_count() != 5) return 11;

    std::cout << "maroco_test: all compile-time and runtime checks passed\n";
    return 0;
}
