project(log
    LANGUAGES C
)

add_library(log
    ${CMAKE_CURRENT_SOURCE_DIR}/log.c
)
add_library(ios::log ALIAS log)

target_include_directories(log
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(log libc ios::api)
