add_library(libc
    ${CMAKE_CURRENT_SOURCE_DIR}/string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sprintf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/delay.c
)
add_library(libc::libc ALIAS libc)

target_include_directories(libc
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(libc compiler-rt ios::bsp)
