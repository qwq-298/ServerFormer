# 库源码（不含入口 main.cpp）
file(GLOB_RECURSE CHAT_LIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.c"
)
file(GLOB_RECURSE CHAT_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

set(CHAT_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/server/main.cpp")
list(REMOVE_ITEM CHAT_LIB_SOURCES "${CHAT_MAIN}")

add_library(chat STATIC ${CHAT_LIB_SOURCES} ${CHAT_HEADERS})

target_include_directories(chat
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(Threads REQUIRED)

target_link_libraries(chat
    PUBLIC
    pans
    muduo_net
    muduo_base
    nlohmann_json::nlohmann_json
    Threads::Threads
    mysqlclient
)

target_compile_options(chat PRIVATE -Werror)

# 可执行入口
add_executable(chat_server ${CHAT_MAIN})
target_link_libraries(chat_server PRIVATE chat)
set_target_properties(chat_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/chat"
)

add_executable(client_server "${CMAKE_CURRENT_SOURCE_DIR}/src/client/main.cpp")
target_link_libraries(client_server PRIVATE chat)
set_target_properties(client_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/chat"
)
