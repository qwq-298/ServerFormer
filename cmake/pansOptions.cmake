add_library(pansOptions INTERFACE)
target_compile_options(
    pansOptions INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
    -Wall
    -Wextra
    -Wpedantic
    -fno-strict-aliasing
    >
)

target_compile_options(
    pansOptions INTERFACE
    $<$<AND:$<PLATFORM_ID:LINUX>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:CLang>>>:
       -fPIC 
    >
)

target_link_options(
    pansOptions INTERFACE
     $<$<AND:$<PLATFORM_ID:LINUX>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:CLang>>>:
       -rdymanic
    >
)

target_compile_definitions(
    pansOptions INTERFACE
    $<$<CONFIG:Debug>:PANS_DEBUG>
)

target_compile_options(
    pansOptions INTERFACE
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-ggdb>
)

target_compile_options(
    pansOptions INTERFACE
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-fno-omit-frame-pointer>

    $<$<CONFIG:RelWithDebInfo>:-DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O2>
    $<$<CONFIG:RelWithDebInfo>:-g>
    $<$<CONFIG:RelWithDebInfo>:-fno-omit-frame-pointer>
)

option(ENABLE_COVERAGE "enable code coverage" OFF)
if(ENABLE_COVERAGE)
   target_compile_options(
    pansOptions INTERFACE
    $<$<CONFIG:Debug>:--coverage>
   )
   target_link_options(
    pansOptions INTERFACE
    $<$<CONFIG:Debug>:--coverage>
   )
endif()