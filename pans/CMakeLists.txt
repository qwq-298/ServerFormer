file(GLOB_RECURSE PANS_SOURCES CONFIGURE_DEPENDS *.cc *.cpp *.c)
file(GLOB_RECURSE PANS_HEADERS CONFIGURE_DEPENDS *.h *.hpp)

add_library(pans STATIC ${PANS_SOURCES} ${PANS_HEADERS})

target_include_directories(pans
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CAMKE_INSTALL_INCLUDEDIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    pans
    PUBLIC 
    pansOptions
)

target_compile_options(
    pans PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
     -Werror
    >
)

install(
    TARGETS pans
    EXPORT pansTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CAMEK_INSTALL_BINDIR}
)

install(
    FILES ${PANS_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pans
)