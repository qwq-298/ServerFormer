cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE FILEPATH "C++ compiler" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(pans VERSION 0.1.0 LANGUAGES CXX C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(PANS_BUILD_EXAMPLES "Build examples" ON)
option(PANS_BUILD_TESTS "Build tests" ON)
option(PANS_BUILD_TOOLS "Build tools" ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
       set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE )
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

cmake_host_system_information(RESULT CPU_NUMS QUERY NUMBER_OF_LOGICAL_CORES)
message(STATUS "cpu nums : ${CPU_NUMS}")
math(EXPR JOBS "${CPU_NUMS}+${CPU_NUMS}/2")
set(CMAKE_BUILD_PARALLEL_LEVEL ${JOBS} CACHE STRING "并行编程数：")
message(STATUS "Compiler default worker jobs: ${CMAKE_BUILD_PARALLEL_LEVEL}")

include(cmake/pansOptions.cmake)

# third-party dependencies
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/yaml-cpp)

# Boost: muduo only needs header-only libraries (boost/any, operators, circular_buffer).
# Download and extract the official release archive and expose the headers via
# BOOST_ROOT, without running boost's own super-build. The tree lives under
# third_party/ (not the build dir) so deleting build/ does not lose it.
set(BOOST_VERSION "1.86.0")
set(BOOST_ARCHIVE "boost_1_86_0.tar.gz")
set(BOOST_URL "https://archives.boost.io/release/${BOOST_VERSION}/source/${BOOST_ARCHIVE}")
set(BOOST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/third_party")
set(BOOST_ROOT "${BOOST_SOURCE_DIR}/boost_1_86_0" CACHE PATH "Boost header-only root" FORCE)
if(NOT EXISTS "${BOOST_ROOT}/boost/version.hpp")
    message(STATUS "Downloading Boost ${BOOST_VERSION} headers ...")
    set(_boost_archive "${BOOST_SOURCE_DIR}/${BOOST_ARCHIVE}")
    file(DOWNLOAD "${BOOST_URL}" "${_boost_archive}"
         STATUS boost_dl_status SHOW_PROGRESS)
    list(GET boost_dl_status 0 boost_dl_code)
    if(NOT boost_dl_code EQUAL 0)
        message(FATAL_ERROR "Failed to download Boost: ${boost_dl_status}")
    endif()
    file(ARCHIVE_EXTRACT
         INPUT "${_boost_archive}"
         DESTINATION "${BOOST_SOURCE_DIR}")
    file(REMOVE "${_boost_archive}")
endif()
set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "" FORCE)

# muduo's own CMakeLists requests 2.6, which CMake >= 4.0 rejects (< 3.5 removed).
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
add_subdirectory(third_party/muduo)

# Boost 1.86 headers use old-style casts that trip muduo's "-Wold-style-cast
# -Werror"; relax warnings-as-errors for the vendored muduo targets only.
# muduo sets its include dirs with directory-scoped include_directories() rather
# than target_include_directories(), so its targets do not propagate headers to
# dependents. Expose them here so users can #include <muduo/...>.
foreach(muduo_target muduo_base muduo_net muduo_http muduo_inspect)
    if(TARGET ${muduo_target})
        target_compile_options(${muduo_target} PRIVATE -Wno-error -Wno-old-style-cast)
        target_include_directories(${muduo_target} SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/muduo)
    endif()
endforeach()

add_subdirectory(third_party/rapidjson)

# nlohmann/json: header-only, exposes nlohmann_json::nlohmann_json
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/json)


add_subdirectory(pans)
if(PANS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(PANS_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

add_subdirectory(chat)
